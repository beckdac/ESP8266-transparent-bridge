#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

#include "config.h"

#ifdef CONFIG_STATIC

void config_execute(void) {
	uint8_t mode;
	struct station_config sta_conf;
	struct softap_config ap_conf;
	uint8_t macaddr[6];

	// make sure the device is in AP and STA combined mode
	mode = wifi_get_mode();
	if (mode != STATIONAP_MODE) {
		wifi_set_opmode(STATIONAP_MODE);
		system_restart();
	}

	// connect to our station
	os_strncpy(sta_conf.ssid, STA_SSID, sizeof(sta_conf.ssid));
	os_strncpy(sta_conf.password, STA_PASSWORD, sizeof(sta_conf.password));
	wifi_station_disconnect();
	ETS_UART_INTR_DISABLE();
	wifi_station_set_config(&sta_conf);		
	ETS_UART_INTR_ENABLE();
	wifi_station_connect();

	// setup the soft AP
	wifi_get_macaddr(SOFTAP_IF, macaddr);
	os_strncpy(ap_conf.ssid, AP_SSID, sizeof(ap_conf.ssid));
	os_strncpy(ap_conf.password, AP_PASSWORD, sizeof(ap_conf.password));
	os_snprintf(&ap_conf.password[strlen(AP_PASSWORD)], sizeof(ap_conf.password), "%02X%02X%02X", macaddr[3], macaddr[4], macaddr[5]);
	ap_conf.authmode = AUTH_WPA_PSK;
	ETS_UART_INTR_DISABLE(); 
	wifi_softap_set_config(&ap_conf);
	ETS_UART_INTR_ENABLE();
}

#endif

#ifdef CONFIG_DYNAMIC

#define MSG_OK "OK\r\n"
#define MSG_ERROR "ERROR\r\n"
#define MSG_INVALID_CMD "INVALID COMMAND\r\n"

void config_parse_ssid_password(char *buf, char **ssid, char **password) {
	for (; *buf == ' ' || *buf == '\t'; ++buf); // absorb spaces
	if (*buf != '\0')
		*ssid = buf;
	else
		*ssid = NULL;
	for (; *buf != ' ' && *buf != '\t' && *buf != '\n' && *buf != '\r'; ++buf);
	if (*buf != '\0')
		buf = '\0';
	else
		*password = NULL;
	for (; *buf == ' ' || *buf == '\t'; ++buf); // absorb spaces
	if (*buf != '\0')
		*password = buf;
	for (; *buf != ' ' && *buf != '\t' && *buf != '\n' && *buf != '\r'; ++buf);
	if (*buf != '\0')
		buf = '\0';
}

void config_parse(struct espconn *conn, char *buf, int len) {
	char *lbuf = (char *)os_zalloc(len + 1), *cmd;
	// we need a '\0' end of the string
	os_memcpy(lbuf, buf, len);
	lbuf[len] = '\0';

	espconn_sent(conn, lbuf, len);

	if (os_strncmp(lbuf, "+++AT", 5) != 0) {
		return;
	}
	cmd=&lbuf[5];
	for (; *cmd == ' ' || *cmd == '\t'; ++cmd); // absorb spaces
	if (os_strncmp(cmd, "STA", 3) == 0) {
		char *ssid, *password;
		struct station_config sta_conf;
		config_parse_ssid_password(cmd, &ssid, &password);
		os_strncpy(sta_conf.ssid, ssid, sizeof(sta_conf.ssid));
		os_strncpy(sta_conf.password, password, sizeof(sta_conf.password));
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
		wifi_station_disconnect();
		ETS_UART_INTR_DISABLE(); 
		wifi_station_set_config(&sta_conf);		
		ETS_UART_INTR_ENABLE(); 
		wifi_station_connect();
	} else if (os_strncmp(cmd, "AP", 2) == 0) {
		char *ssid, *password;
		struct softap_config ap_conf;
		config_parse_ssid_password(cmd, &ssid, &password);
		os_strncpy(ap_conf.ssid, ssid, sizeof(ap_conf.ssid));
		os_strncpy(ap_conf.password, password, sizeof(ap_conf.password));
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
		ap_conf.authmode = AUTH_WPA_PSK;
		ap_conf.channel = 6;
		ETS_UART_INTR_DISABLE();
		wifi_softap_set_config(&ap_conf);
		ETS_UART_INTR_ENABLE();
	} else if (os_strncmp(cmd, "MODE", 4) == 0) {
		uint8_t mode;
		char *endptr;
		cmd+=4;
		for (; *cmd == ' ' || *cmd == '\t'; ++cmd); // absorb spaces
		mode = strtoul(cmd, &endptr, 10);
		if (cmd != endptr && mode >= 0 && mode <= 3) {
			if (wifi_get_opmode() != mode) {
				ETS_UART_INTR_DISABLE();
				wifi_set_opmode(mode);
				ETS_UART_INTR_ENABLE();
				espconn_sent(conn, MSG_OK, strlen(MSG_OK));
				os_free(lbuf);
				system_restart();
			} else
				espconn_sent(conn, MSG_OK, strlen(MSG_OK));
		} else {
			espconn_sent(conn, MSG_ERROR, strlen(MSG_ERROR));
		}
	} else if (os_strncmp(cmd, "RESET", 5) == 0) {
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
		system_restart();
	} else if (*cmd == '\n' || *cmd == '\r') {
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
	} else {
		espconn_sent(conn, MSG_INVALID_CMD, strlen(MSG_INVALID_CMD));
	}
	os_free(lbuf);
}

#endif
