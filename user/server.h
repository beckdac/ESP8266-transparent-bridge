#ifndef __SERVER_H__
#define __SERVER_H__

#include <ip_addr.h>
#include <c_types.h>
#include <espconn.h>

#define MAX_CONN 5
#define SERVER_TIMEOUT 28799

//Max send buffer len
#define MAX_TXBUFFER 1024

typedef struct serverConnData serverConnData;

struct serverConnData {
        struct espconn *conn;
		char *txbuffer; //the buffer for the data to send
		uint16  txbufferlen; //the length  of data in txbuffer
 		bool readytosend; //true, if txbuffer can send by espconn_sent
};

void ICACHE_FLASH_ATTR serverInit(int port);
sint8  ICACHE_FLASH_ATTR espbuffsent(serverConnData *conn, const char *data, uint16 len);
sint8  ICACHE_FLASH_ATTR espbuffsentstring(serverConnData *conn, const char *data);
sint8  ICACHE_FLASH_ATTR espbuffsentprintf(serverConnData *conn, const char *format, ...);

#endif /* __SERVER_H__ */
