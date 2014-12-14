/*
 * File	: user_main.c
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
#include "os_type.h"
#include "osapi.h"
#include "driver/uart.h"
#include "task.h"

#include "server.h"

os_event_t    recvTaskQueue[recvTaskQueueLen];
extern serverConnData connData[MAX_CONN];

static void ICACHE_FLASH_ATTR recvTask(os_event_t *events)
{
  uint8_t c, i;

  //uart0_sendStr("\r\nrecTask called\r\n");

  while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
  {
    WRITE_PERI_REG(0X60000914, 0x73); //WTD
    c = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;

    for (i = 0; i < MAX_CONN; ++i) {
      if (connData[i].conn) {
        espconn_sent(connData[i].conn, &c, 1);
      }
    }
//  echo    
//  uart_tx_one_char(c);
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

void user_init(void)
{
  uint8_t i;

  uart_init(BIT_RATE_9600, BIT_RATE_9600);

  serverInit(23);

  for (i = 0; i < 16; ++i)
  	uart0_sendStr("\r\nready\r\n");

  system_os_task(recvTask, recvTaskPrio, recvTaskQueue, recvTaskQueueLen);
}
