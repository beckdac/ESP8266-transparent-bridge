#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

#include "server.h"
#include "config.h"

static struct espconn serverConn;
static esp_tcp serverTcp;

//Connection pool
static char txbuffer[MAX_CONN][MAX_TXBUFFER];
serverConnData connData[MAX_CONN];

static serverConnData ICACHE_FLASH_ATTR *serverFindConnData(void *arg) {
	int i;
	for (i=0; i<MAX_CONN; i++) {
		if (connData[i].conn==(struct espconn *)arg)
			return &connData[i];
	}
	//os_printf("FindConnData: Huh? Couldn't find connection for %p\n", arg);
	return NULL; //WtF?
}

//send all data in conn->txbuffer
//returns result from espconn_sent if data in buffer or ESPCONN_OK (0)
//only internal used by espbuffsent, serverSentCb
static sint8 ICACHE_FLASH_ATTR sendtxbuffer(serverConnData *conn) {
	sint8 result = ESPCONN_OK;
	if (conn->txbufferlen != 0) {
		conn->readytosend = false;
		result= espconn_sent(conn->conn, (uint8_t*)conn->txbuffer, conn->txbufferlen);
		conn->txbufferlen = 0;
		if (result != ESPCONN_OK)
			os_printf("sendtxbuffer: espconn_sent error %d on conn %p\n", result, conn);
	}
	return result;
}

//send formatted output to transmit buffer and call sendtxbuffer, if ready (previous espconn_sent is completed)
sint8 ICACHE_FLASH_ATTR espbuffsentprintf(serverConnData *conn, const char *format, ...) {
	uint16 len;
	va_list al;
	va_start(al, format);
	len = ets_vsnprintf(conn->txbuffer + conn->txbufferlen, MAX_TXBUFFER - conn->txbufferlen - 1, format, al);
	va_end(al);
	if (len <0) {
		os_printf("espbuffsentprintf: txbuffer full on conn %p\n", conn);
		return len;
	}
	conn->txbufferlen += len;
	if (conn->readytosend)
		return sendtxbuffer(conn);
	return ESPCONN_OK;
}

//send string through espbuffsent
sint8 ICACHE_FLASH_ATTR espbuffsentstring(serverConnData *conn, const char *data) {
	return espbuffsent(conn, data, strlen(data));
}

//use espbuffsent instead of espconn_sent
//It solve problem: the next espconn_sent must after espconn_sent_callback of the pre-packet.
//Add data to the send buffer and send if previous send was completed it call sendtxbuffer and espconn_sent
//Returns ESPCONN_OK (0) for success, -128 if buffer is full or error from espconn_sent
sint8 ICACHE_FLASH_ATTR espbuffsent(serverConnData *conn, const char *data, uint16 len) {
	if (conn->txbufferlen + len > MAX_TXBUFFER) {
		os_printf("espbuffsent: txbuffer full on conn %p\n", conn);
		return -128;
	}
	os_memcpy(conn->txbuffer + conn->txbufferlen, data, len);
	conn->txbufferlen += len;
	if (conn->readytosend) 
		return sendtxbuffer(conn);
	return ESPCONN_OK;
}

//callback after the data are sent
static void ICACHE_FLASH_ATTR serverSentCb(void *arg) {
	serverConnData *conn = serverFindConnData(arg);
	//os_printf("Sent callback on conn %p\n", conn);
	if (conn==NULL) return;
	conn->readytosend = true;
	sendtxbuffer(conn); // send possible new data in txbuffer
}

static void ICACHE_FLASH_ATTR serverRecvCb(void *arg, char *data, unsigned short len) {
	int x;
	char *p, *e;
	serverConnData *conn = serverFindConnData(arg);
	//os_printf("Receive callback on conn %p\n", conn);
	if (conn == NULL) return;

#ifdef CONFIG_DYNAMIC
	if (len >= 5 && data[0] == '+' && data[1] == '+' && data[2] == '+' && data[3] == 'A' && data[4] == 'T') {
		config_parse(conn, data, len);
	} else
#endif
		uart0_tx_buffer(data, len);
}

static void ICACHE_FLASH_ATTR serverReconCb(void *arg, sint8 err) {
	serverConnData *conn=serverFindConnData(arg);
	if (conn==NULL) return;
	//Yeah... No idea what to do here. ToDo: figure something out.
}

static void ICACHE_FLASH_ATTR serverDisconCb(void *arg) {
	//Just look at all the sockets and kill the slot if needed.
	int i;
	for (i=0; i<MAX_CONN; i++) {
		if (connData[i].conn!=NULL) {
			if (connData[i].conn->state==ESPCONN_NONE || connData[i].conn->state==ESPCONN_CLOSE) {
				connData[i].conn=NULL;
			}
		}
	}
}

static void ICACHE_FLASH_ATTR serverConnectCb(void *arg) {
	struct espconn *conn = arg;
	int i;
	//Find empty conndata in pool
	for (i=0; i<MAX_CONN; i++) if (connData[i].conn==NULL) break;
	os_printf("Con req, conn=%p, pool slot %d\n", conn, i);

	if (i==MAX_CONN) {
		os_printf("Aiee, conn pool overflow!\n");
		espconn_disconnect(conn);
		return;
	}
	connData[i].conn=conn;
	connData[i].txbufferlen = 0;
	connData[i].readytosend = true;

	espconn_regist_recvcb(conn, serverRecvCb);
	espconn_regist_reconcb(conn, serverReconCb);
	espconn_regist_disconcb(conn, serverDisconCb);
	espconn_regist_sentcb(conn, serverSentCb);
}

void ICACHE_FLASH_ATTR serverInit(int port) {
	int i;
	for (i = 0; i < MAX_CONN; i++) {
		connData[i].conn = NULL;
		connData[i].txbuffer = txbuffer[i];
		connData[i].txbufferlen = 0;
		connData[i].readytosend = true;
	}
	serverConn.type=ESPCONN_TCP;
	serverConn.state=ESPCONN_NONE;
	serverTcp.local_port=port;
	serverConn.proto.tcp=&serverTcp;

	espconn_regist_connectcb(&serverConn, serverConnectCb);
	espconn_accept(&serverConn);
	espconn_regist_time(&serverConn, SERVER_TIMEOUT, 0);
}
