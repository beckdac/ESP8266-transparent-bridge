#ifndef __TCPUART_H__
#define __TCPUART_H__

#include "server.h"

#define MAX_CONN_TCP_UART 1

void ICACHE_FLASH_ATTR tcpuartInit(int port);
sint8  ICACHE_FLASH_ATTR tcpuartespbuffsent(serverConnData *conn, const char *data, uint16 len);

#endif /* __TCPUART_H__ */
