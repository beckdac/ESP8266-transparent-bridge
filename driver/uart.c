/*
 * File	: uart.c
 * This file is part of Espressif's AT+ command set program.
 * Copyright (C) 2013 - 2016, Espressif Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ets_sys.h"
#include "osapi.h"
#include "driver/uart.h"
#include "driver/uart_register.h"
#include "task.h"
#include "driver/gpio16.h"

// UartDev is defined and initialized in rom code.
extern UartDevice    UartDev;

LOCAL void uart0_rx_intr_handler(void *para);

/******************************************************************************
 * FunctionName : uart_config
 * Description  : Internal used function
 *                UART0 used for data TX/RX, RX buffer size is 0x100, interrupt enabled
 *                UART1 just used for debug output
 * Parameters   : uart_no, use UART0 or UART1 defined ahead
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
uart_config(uint8 uart_no)
{
	if (uart_no == UART1)
	{
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
	}
	else
	{
		/* rcv_buff size if 0x100 */
		ETS_UART_INTR_ATTACH(uart0_rx_intr_handler,  &(UartDev.rcv_buff));
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
		PIN_PULLUP_DIS (PERIPHS_IO_MUX_U0RXD_U);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, 0);
	}

	uart_div_modify(uart_no, UART_CLK_FREQ / (UartDev.baut_rate));

	if (uart_no == UART1)  //UART 1 always 8 N 1
		WRITE_PERI_REG(UART_CONF0(uart_no), CALC_UARTMODE(EIGHT_BITS, NONE_BITS, ONE_STOP_BIT));
	else
		WRITE_PERI_REG(UART_CONF0(uart_no), CALC_UARTMODE(UartDev.data_bits, UartDev.parity, UartDev.stop_bits));

	//clear rx and tx fifo,not ready
	SET_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);
	CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST | UART_TXFIFO_RST);

	//set rx fifo trigger
	WRITE_PERI_REG(UART_CONF1(uart_no), UART_RX_TOUT_EN |
						(0x10 &UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S);
	//clear all interrupt
	WRITE_PERI_REG(UART_INT_CLR(uart_no), 0xffff);
	//enable rx_interrupt : rx fifo full and rx tout
	CLEAR_PERI_REG_MASK(UART_INT_ENA(uart_no), 0x1ff);
	SET_PERI_REG_MASK(UART_INT_ENA(uart_no),  UART_RXFIFO_TOUT_INT_ENA);
}

/******************************************************************************
 * FunctionName : uart1_tx_one_char
 * Description  : Internal used function
 *                Use uart1 interface to transfer one char
 * Parameters   : uint8 TxChar - character to tx
 * Returns      : OK
*******************************************************************************/
LOCAL STATUS
uart_tx_one_char(uint8 uart, uint8 TxChar)
{
	while (true)
	{
		uint32 fifo_cnt = READ_PERI_REG(UART_STATUS(uart)) & (UART_TXFIFO_CNT<<UART_TXFIFO_CNT_S);
		if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126) {
			break;
		}
	}

	WRITE_PERI_REG(UART_FIFO(uart) , TxChar);
	return OK;
}

/******************************************************************************
 * FunctionName : uart1_write_char
 * Description  : Internal used function
 *                Do some special deal while tx char is '\r' or '\n'
 * Parameters   : char c - character to tx
 * Returns      : NONE
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
uart1_write_char(char c)
{
	if (c == '\n')
	{
		uart_tx_one_char(UART1, '\r');
		uart_tx_one_char(UART1, '\n');
	}
	else if (c == '\r')
	{}
	else
	{
		uart_tx_one_char(UART1, c);
	}
}
/******************************************************************************
 * FunctionName : uart0_tx_buffer
 * Description  : use uart0 to transfer buffer
 * Parameters   : uint8 *buf - point to send buffer
 *                uint16 len - buffer len
 * Returns      :
*******************************************************************************/
void ICACHE_FLASH_ATTR
uart0_tx_buffer(uint8 *buf, uint16 len)
{
	uint16 i;

	for (i = 0; i < len; i++)
	{
		uart_tx_one_char(UART0, buf[i]);
	}
}

/******************************************************************************
 * FunctionName : uart0_sendStr
 * Description  : use uart0 to transfer buffer
 * Parameters   : uint8 *buf - point to send buffer
 *                uint16 len - buffer len
 * Returns      :
*******************************************************************************/
void ICACHE_FLASH_ATTR
uart0_sendStr(const char *str)
{
	while(*str)
	{
		uart_tx_one_char(UART0, *str++);
	}
}

/******************************************************************************
 * FunctionName : uart0_rx_intr_handler
 * Description  : Internal used function
 *                UART0 interrupt handler, add self handle code inside
 * Parameters   : void *para - point to ETS_UART_INTR_ATTACH's arg
 * Returns      : NONE
*******************************************************************************/
//extern void at_recvTask(void);

LOCAL void
uart0_rx_intr_handler(void *para)
{
	/* uart0 and uart1 intr combine together, when interrupt occur,
	* see reg 0x3ff20020, bit2, bit0 represents
	* uart1 and uart0 respectively
	*/
	uint8 RcvChar;
	uint8 uart_no = UART0;

	if(UART_FRM_ERR_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_FRM_ERR_INT_ST))
	{
		os_printf("FRM_ERR\r\n");
		WRITE_PERI_REG(UART_INT_CLR(uart_no), UART_FRM_ERR_INT_CLR);
	}

	if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_FULL_INT_ST))
	{
		//    os_printf("fifo full\r\n");
		ETS_UART_INTR_DISABLE();/////////

		system_os_post(recvTaskPrio, 0, 0);
	}
	else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(uart_no)) & UART_RXFIFO_TOUT_INT_ST))
	{
		ETS_UART_INTR_DISABLE();/////////

		//    os_printf("stat:%02X",*(uint8 *)UART_INT_ENA(uart_no));
		system_os_post(recvTaskPrio, 0, 0);
	}
}

/******************************************************************************
 * FunctionName : uart_init
 * Description  : user interface for init uart
 * Parameters   : UartBautRate uart0_br - uart0 bautrate
 *                UartBautRate uart1_br - uart1 bautrate
 * Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR
uart_init(UartBautRate uart0_br, UartBautRate uart1_br)
{
	// rom use 74880 baut_rate, here reinitialize
	UartDev.baut_rate = uart0_br;
	uart_config(UART0);
	UartDev.baut_rate = uart1_br;
	uart_config(UART1);
	ETS_UART_INTR_ENABLE();

	// install uart1 putc callback
	os_install_putc1((void *)uart1_write_char);
}

void ICACHE_FLASH_ATTR
uart_reattach()
{
	uart_init(BIT_RATE_74880, BIT_RATE_74880);
}
