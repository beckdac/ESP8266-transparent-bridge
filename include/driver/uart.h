/*
 * File	: uart.h
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
#ifndef UART_APP_H
#define UART_APP_H

#include "uart_register.h"
#include "eagle_soc.h"
#include "c_types.h"

#define RX_BUFF_SIZE    256
#define TX_BUFF_SIZE    100
#define UART0   0
#define UART1   1

//macros for UART_CONF0 register

//calc bit 0..5 for  UART_CONF0 register
#define CALC_UARTMODE(data_bits,parity,stop_bits) \
	(((parity == NONE_BITS) ? 0x0 : (UART_PARITY_EN | (parity & UART_PARITY))) | \
	((stop_bits & UART_STOP_BIT_NUM) << UART_STOP_BIT_NUM_S) | \
	((data_bits & UART_BIT_NUM) << UART_BIT_NUM_S))
//get data_bits from UART_CONF0 register
#define GETUART_DATABITS(uartmode) ((uartmode >> UART_BIT_NUM_S) & UART_BIT_NUM)
//get stop_bits from UART_CONF0 register
#define GETUART_STOPBITS(uartmode) ((uartmode >> UART_STOP_BIT_NUM_S) & UART_STOP_BIT_NUM)
//get parity from UART_CONF0 register
#define GETUART_PARITYMODE(uartmode) ((uartmode & UART_PARITY_EN) ? uartmode & UART_PARITY : NONE_BITS)


typedef enum {
	FIVE_BITS = 0x0,
	SIX_BITS = 0x1,
	SEVEN_BITS = 0x2,
	EIGHT_BITS = 0x3
} UartBitsNum4Char;

//FROM  Uart_reg_release_141118.xls
// stop_bit_num 	 [5:4]   	 2'd1 	 R/W 	Set stop bit: 1:1bit  2:1.5bits  3:2bits
// bit_num 	 [3:2]   	 2'd3 	 R/W 	Set bit num:  0:5bits 1:6bits 2:7bits 3:8bits
// parity_en 	 [1]   	 1'b0 	 R/W 	Set this bit to enable uart parity check
// parity		[0]   	 1'b0 	 R/W 	Set parity check:  0:even 1:odd

typedef enum {
	ONE_STOP_BIT = 0x1,
	ONE_HALF_STOP_BIT = 0x2,
	TWO_STOP_BIT = 0x3
} UartStopBitsNum;

typedef enum {
	NONE_BITS = 0x2,
	ODD_BITS = 1,
	EVEN_BITS = 0
} UartParityMode;

typedef enum {
	PARITY_DISABLE = 0,
	PARITY_ENABLE = UART_PARITY_EN
} UartExistParity;


typedef enum {
	BIT_RATE_300 = 300,
	BIT_RATE_600 = 600,
	BIT_RATE_1200 = 1200,
	BIT_RATE_2400 = 2400,
	BIT_RATE_4800 = 4800,
	BIT_RATE_9600 = 9600,
	BIT_RATE_19200 = 19200,
	BIT_RATE_38400 = 38400,
	BIT_RATE_57600 = 57600,
	BIT_RATE_74880 = 74880,
	BIT_RATE_115200 = 115200,
	BIT_RATE_230400 = 230400,
	BIT_RATE_256000 = 256000,
	BIT_RATE_460800 = 460800,
	BIT_RATE_921600 = 921600
} UartBautRate;

typedef enum {
    NONE_CTRL,
    HARDWARE_CTRL,
    XON_XOFF_CTRL
} UartFlowCtrl;

typedef enum {
    EMPTY,
    UNDER_WRITE,
    WRITE_OVER
} RcvMsgBuffState;

typedef struct {
    uint32     RcvBuffSize;
    uint8     *pRcvMsgBuff;
    uint8     *pWritePos;
    uint8     *pReadPos;
    uint8      TrigLvl; //JLU: may need to pad
    RcvMsgBuffState  BuffState;
} RcvMsgBuff;

typedef struct {
    uint32   TrxBuffSize;
    uint8   *pTrxBuff;
} TrxMsgBuff;

typedef enum {
    BAUD_RATE_DET,
    WAIT_SYNC_FRM,
    SRCH_MSG_HEAD,
    RCV_MSG_BODY,
    RCV_ESC_CHAR,
} RcvMsgState;

typedef struct {
    UartBautRate 	     baut_rate;
    UartBitsNum4Char  data_bits;
    UartExistParity      exist_parity;
    UartParityMode 	    parity;    
    UartStopBitsNum   stop_bits;
    UartFlowCtrl         flow_ctrl;
    RcvMsgBuff          rcv_buff;
    TrxMsgBuff           trx_buff;
    RcvMsgState        rcv_state;
    int                      received;
    int                      buff_uart_no;  //indicate which uart use tx/rx buffer
} UartDevice;

void uart_init(UartBautRate uart0_br, UartBautRate uart1_br);
void uart0_sendStr(const char *str);

#endif

