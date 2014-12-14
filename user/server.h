#ifndef __SERVER_H__
#define __SERVER_H__

#include <ip_addr.h>
#include <c_types.h>
#include <espconn.h>

#define MAX_CONN 5
#define SERVER_TIMEOUT 28799

typedef struct serverConnData serverConnData;

struct serverConnData {
        struct espconn *conn;
        char *buff;
};

void ICACHE_FLASH_ATTR serverInit(int port);

#endif /* __SERVER_H__ */
