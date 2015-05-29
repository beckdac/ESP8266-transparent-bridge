/*
 * serial.c
 *
 *  Created on: 19/05/2015
 *      Author: palha
 */

#include "driver/uart.h"
#include "c_types.h"
#include "os_type.h"
#include "server.h"
#include "task.h"

// UartDev is defined and initialized in rom code.
extern UartDevice    UartDev;
extern  serverConnData tcpuartConnData[MAX_CONN];

#define SIG_UART0_RX 	0
#define MAX_UARTBUFFER (MAX_TXBUFFER/4)
static uint8 uartbuffer[MAX_UARTBUFFER];
os_event_t		recvTaskQueue[recvTaskQueueLen];

static void ICACHE_FLASH_ATTR recvTask(os_event_t *events)
{
	uint8_t i;

	if (events->sig == SIG_UART0_RX) {
		while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
		{
			WRITE_PERI_REG(0X60000914, 0x73); //WTD
			uint16 length = 0;
			while ((READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S)) && (length<MAX_UARTBUFFER))
				uartbuffer[length++] = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
			for (i = 0; i < MAX_CONN; ++i)
				if (tcpuartConnData[i].conn)
					tcpuartespbuffsent(&tcpuartConnData[i], uartbuffer, length);
		}

		if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST))
		{
			WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
		}
		else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST))
		{
			WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
		}
		ETS_UART_INTR_ENABLE();
	}
}


void ICACHE_FLASH_ATTR serialInit(uint32_t baud, uint8_t uartconf0) {
	if (uartconf0 == 0) {
		UartDev.data_bits = EIGHT_BITS;
		UartDev.parity    = NONE_BITS;
		UartDev.stop_bits = ONE_STOP_BIT;
	} else {
		UartDev.data_bits = GETUART_DATABITS(uartconf0);
		UartDev.parity    = GETUART_PARITYMODE(uartconf0);
		UartDev.stop_bits = GETUART_STOPBITS(uartconf0);
	}

	uart_init(baud, BIT_RATE_115200);

	system_os_task(recvTask, recvTaskPrio, recvTaskQueue, recvTaskQueueLen);
}
