#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

#include "tcpuart.h"
#include "config.h"

static struct espconn tcpuartConn;
static esp_tcp tcpuartTcp;

//Connection pool
static char tcpuart_txbuffer[MAX_CONN_TCP_UART][MAX_TXBUFFER];
serverConnData tcpuartConnData[MAX_CONN_TCP_UART];



static serverConnData ICACHE_FLASH_ATTR *tcpuartFindConnData(void *arg) {
	int i;
	for (i=0; i<MAX_CONN_TCP_UART; i++) {
		if (tcpuartConnData[i].conn==(struct espconn *)arg)
			return &tcpuartConnData[i];
	}
	//os_printf("FindConnData: Huh? Couldn't find connection for %p\n", arg);
	return NULL; //WtF?
}


//send all data in conn->txbuffer
//returns result from espconn_sent if data in buffer or ESPCONN_OK (0)
//only internal used by espbuffsent, serverSentCb
static sint8  ICACHE_FLASH_ATTR sendtxbuffer(serverConnData *conn) {
	sint8 result = ESPCONN_OK;
	if (conn->txbufferlen != 0)	{
		conn->readytosend = false;
		result= espconn_sent(conn->conn, (uint8_t*)conn->txbuffer, conn->txbufferlen);
		conn->txbufferlen = 0;	
		if (result != ESPCONN_OK)
			os_printf("sendtxbuffer: espconn_sent error %d on conn %p\n", result, conn);
	}
	return result;
}

//use espbuffsent instead of espconn_sent
//It solve problem: the next espconn_sent must after espconn_sent_callback of the pre-packet.
//Add data to the send buffer and send if previous send was completed it call sendtxbuffer and  espconn_sent
//Returns ESPCONN_OK (0) for success, -128 if buffer is full or error from  espconn_sent
sint8 ICACHE_FLASH_ATTR tcpuartespbuffsent(serverConnData *conn, const char *data, uint16 len) {
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
static void ICACHE_FLASH_ATTR tcpuartSentCb(void *arg) {
	serverConnData *conn = tcpuartFindConnData(arg);
	//os_printf("Sent callback on conn %p\n", conn);
	if (conn==NULL) return;
	conn->readytosend = true;
	sendtxbuffer(conn); // send possible new data in txbuffer
}

static void ICACHE_FLASH_ATTR tcpuartRecvCb(void *arg, char *data, unsigned short len) {
	int x;
	char *p, *e;
	serverConnData *conn = tcpuartFindConnData(arg);
	//os_printf("Receive callback on conn %p\n", conn);
	if (conn == NULL) return;

	uart0_tx_buffer(data, len);
}

static void ICACHE_FLASH_ATTR tcpuartReconCb(void *arg, sint8 err) {
	serverConnData *conn=tcpuartFindConnData(arg);
	if (conn==NULL) return;
	//Yeah... No idea what to do here. ToDo: figure something out.
}

static void ICACHE_FLASH_ATTR tcpuartDisconCb(void *arg) {
	//Just look at all the sockets and kill the slot if needed.
	int i;
	for (i=0; i<MAX_CONN_TCP_UART; i++) {
		if (tcpuartConnData[i].conn!=NULL) {
			if (tcpuartConnData[i].conn->state==ESPCONN_NONE || tcpuartConnData[i].conn->state==ESPCONN_CLOSE) {
				tcpuartConnData[i].conn=NULL;
			}
		}
	}
}

static void ICACHE_FLASH_ATTR tcpuartConnectCb(void *arg) {
	struct espconn *conn = arg;
	int i;
	//Find empty conndata in pool
	for (i=0; i<MAX_CONN_TCP_UART; i++) if (tcpuartConnData[i].conn==NULL) break;
	os_printf("Con req, conn=%p, pool slot %d\n", conn, i);

	if (i==MAX_CONN_TCP_UART) {
		os_printf("Aiee, conn pool overflow!\n");
		espconn_disconnect(conn);
		return;
	}
	tcpuartConnData[i].conn=conn;
	tcpuartConnData[i].txbufferlen = 0;
	tcpuartConnData[i].readytosend = true;

	espconn_regist_recvcb(conn, tcpuartRecvCb);
	espconn_regist_reconcb(conn, tcpuartReconCb);
	espconn_regist_disconcb(conn, tcpuartDisconCb);
	espconn_regist_sentcb(conn, tcpuartSentCb);
}

void ICACHE_FLASH_ATTR tcpuartInit(int port) {
	int i;
	for (i = 0; i < MAX_CONN_TCP_UART; i++) {
		tcpuartConnData[i].conn = NULL;
		tcpuartConnData[i].txbuffer = tcpuart_txbuffer[i];
		tcpuartConnData[i].txbufferlen = 0;
		tcpuartConnData[i].readytosend = true;
	}
	tcpuartConn.type=ESPCONN_TCP;
	tcpuartConn.state=ESPCONN_NONE;
	tcpuartTcp.local_port=port;
	tcpuartConn.proto.tcp=&tcpuartTcp;

	espconn_regist_connectcb(&tcpuartConn, tcpuartConnectCb);
	espconn_accept(&tcpuartConn);
	espconn_regist_time(&tcpuartConn, SERVER_TIMEOUT, 0);
}
