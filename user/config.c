#ifndef CONFIG_PARSE_TEST_UNIT

// this is the normal build target ESP include set
#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

#else

// test unit target for config_parse
// gcc -g -o config_test config.c -D CONFIG_PARSE_TEST_UNIT
// ./config_test < config_test.cmd
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct espconn {
	int dummy;
};

struct station_config {
	uint8_t ssid[32];
	uint8_t password[64];
	uint8_t bssid_set;
	uint8_t bssid[6];
};

struct softap_config {
	uint8_t ssid[32];
	uint8_t password[64];
	uint8_t ssid_len;
	uint8_t channel;
	uint8_t authmode;
	uint8_t ssid_hidden;
	uint8_t max_connection;
};

#define os_sprintf	sprintf
#define os_malloc	malloc
#define os_strncpy	strncpy
#define os_strncmp	strncmp
#define os_free		free
#define os_bzero	bzero
#define os_memcpy	memcpy
#define os_memset	memset

#define espconn_sent(conn, buf, len)	printf(buf)

#define AUTH_OPEN 0
#define AUTH_WPA_PSK 2
#define AUTH_WPA2_PSK 3

#define wifi_get_opmode() (printf("wifi_get_opmode()\n") ? 2 : 2)
#define wifi_set_opmode(mode) printf("wifi_set_opmode(%d)\n", mode)
#define wifi_station_disconnect() printf("wifi_station_disconnect()\n")
#define wifi_station_get_config(conf) { strncpy((conf)->ssid, "dummystassid", 32); strncpy((conf)->password, "dummystapassword", 64); }
#define wifi_station_set_config(conf) printf("wifi_station_set_config(%s, %s)\n", (conf)->ssid, (conf)->password)
#define wifi_station_connect() printf("wifi_station_connect()\n");
#define wifi_get_macaddr(if, result) printf("wifi_get_mac_addr(SOFTAP_IF, macaddr)\n")
#define wifi_softap_get_config(conf) { strncpy((conf)->ssid, "dummyapssid", 32); strncpy((conf)->password, "dummyappassword", 64); (conf)->authmode=AUTH_WPA_PSK; (conf)->channel=3; }
#define wifi_softap_set_config(conf) printf("wifi_softap_set_config(%s, %s, %d, %d)\n", (conf)->ssid, (conf)->password, (conf)->authmode, (conf)->channel)
#define system_restart() printf("system_restart()\n");
#define ETS_UART_INTR_DISABLE()	printf("ETS_UART_INTR_DISABLE()\n");
#define ETS_UART_INTR_ENABLE()	printf("ETS_UART_INTR_ENABLE()\n");

#endif

#include "config.h"

#ifdef CONFIG_STATIC

void config_execute(void) {
	uint8_t mode;
	struct station_config sta_conf;
	struct softap_config ap_conf;
	uint8_t macaddr[6] = { 0, 0, 0, 0, 0, 0 };

	// make sure the device is in AP and STA combined mode
	mode = wifi_get_opmode();
	if (mode != STATIONAP_MODE) {
		wifi_set_opmode(STATIONAP_MODE);
		system_restart();
	}

	// connect to our station
	os_bzero(&sta_conf, sizeof(struct station_config));
	wifi_station_get_config(&sta_conf);
	os_strncpy(sta_conf.ssid, STA_SSID, sizeof(sta_conf.ssid));
	os_strncpy(sta_conf.password, STA_PASSWORD, sizeof(sta_conf.password));
	wifi_station_disconnect();
	ETS_UART_INTR_DISABLE();
	wifi_station_set_config(&sta_conf);		
	ETS_UART_INTR_ENABLE();
	wifi_station_connect();

	// setup the soft AP
	os_bzero(&ap_conf, sizeof(struct softap_config));
	wifi_softap_get_config(&ap_conf);
	wifi_get_macaddr(SOFTAP_IF, macaddr);
	os_strncpy(ap_conf.ssid, AP_SSID, sizeof(ap_conf.ssid));
	ap_conf.ssid_len = strlen(AP_SSID);
	os_strncpy(ap_conf.password, AP_PASSWORD, sizeof(ap_conf.password));
	//os_snprintf(&ap_conf.password[strlen(AP_PASSWORD)], sizeof(ap_conf.password) - strlen(AP_PASSWORD), "_%02X%02X%02X", macaddr[3], macaddr[4], macaddr[5]);
	os_sprintf(ap_conf.password[strlen(AP_PASSWORD)], "_%02X%02X%02X", macaddr[3], macaddr[4], macaddr[5]);
	ap_conf.authmode = AUTH_WPA_PSK;
	ap_conf.channel = 6;
	ETS_UART_INTR_DISABLE(); 
	wifi_softap_set_config(&ap_conf);
	ETS_UART_INTR_ENABLE();
}

#endif

#ifdef CONFIG_DYNAMIC

#define MSG_OK "OK\r\n"
#define MSG_ERROR "ERROR\r\n"
#define MSG_INVALID_CMD "UNKNOWN COMMAND\r\n"

#define MAX_ARGS 12
#define MSG_BUF_LEN 128

#ifdef CONFIG_PARSE_TEST_UNIT
#endif

char *my_strdup(char *str) {
	size_t len;
	char *copy;

	len = strlen(str) + 1;
	if (!(copy = os_malloc((u_int)len)))
		return (NULL);
	os_memcpy(copy, str, len);
	return (copy);
}

char **config_parse_args(char *buf, uint8_t *argc) {
	const char delim[] = " \t";
	char *save, *tok;
	char **argv = (char **)os_malloc(sizeof(char *) * MAX_ARGS);	// note fixed length
	os_memset(argv, 0, sizeof(char *) * MAX_ARGS);

	*argc = 0;
	for (; *buf == ' ' || *buf == '\t'; ++buf); // absorb leading spaces
	for (tok = strtok_r(buf, delim, &save); tok; tok = strtok_r(NULL, delim, &save)) {
		argv[*argc] = my_strdup(tok);
		(*argc)++;
		if (*argc == MAX_ARGS) {
			break;
		}
	}
	return argv;
}

void config_parse_args_free(uint8_t argc, char *argv[]) {
	uint8_t i;
	for (i = 0; i <= argc; ++i) {
		if (argv[i])
			os_free(argv[i]);
	}
	os_free(argv);
}

void config_cmd_reset(struct espconn *conn, uint8_t argc, char *argv[]) {
	espconn_sent(conn, MSG_OK, strlen(MSG_OK));
	system_restart();
}

void config_cmd_mode(struct espconn *conn, uint8_t argc, char *argv[]) {
	uint8_t mode;

	if (argc == 0) {
		char buf[MSG_BUF_LEN];
		uint8_t len;
		len = os_sprintf(buf, "MODE=%d\n", wifi_get_opmode());
		espconn_sent(conn, buf, len);
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
	} else if (argc != 1) {
		espconn_sent(conn, MSG_ERROR, strlen(MSG_ERROR));
	} else {
		mode = atoi(argv[1]);
		if (mode >= 0 && mode <= 3) {
			if (wifi_get_opmode() != mode) {
				ETS_UART_INTR_DISABLE();
				wifi_set_opmode(mode);
				ETS_UART_INTR_ENABLE();
				espconn_sent(conn, MSG_OK, strlen(MSG_OK));
				system_restart();
			} else {
				espconn_sent(conn, MSG_OK, strlen(MSG_OK));
			}
		} else {
			espconn_sent(conn, MSG_ERROR, strlen(MSG_ERROR));
		}
	}
}

// spaces are not supported in the ssid or password
void config_cmd_sta(struct espconn *conn, uint8_t argc, char *argv[]) {
	char *ssid = argv[1], *password = argv[2];
	struct station_config sta_conf;

	os_bzero(&sta_conf, sizeof(struct station_config));
	wifi_station_get_config(&sta_conf);

	if (argc == 0) {
		char buf[MSG_BUF_LEN];
		uint8_t len;
		len = os_sprintf(buf, "SSID=%s PASSWORD=%s\n", sta_conf.ssid, sta_conf.password);
		espconn_sent(conn, buf, len);
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
	} else if (argc != 2) {
		espconn_sent(conn, MSG_ERROR, strlen(MSG_ERROR));
	} else {
		os_strncpy(sta_conf.ssid, ssid, sizeof(sta_conf.ssid));
		os_strncpy(sta_conf.password, password, sizeof(sta_conf.password));
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
		wifi_station_disconnect();
		ETS_UART_INTR_DISABLE(); 
		wifi_station_set_config(&sta_conf);		
		ETS_UART_INTR_ENABLE(); 
		wifi_station_connect();
	}
}

// spaces are not supported in the ssid or password
void config_cmd_ap(struct espconn *conn, uint8_t argc, char *argv[]) {
	char *ssid = argv[1], *password = argv[2];
	struct softap_config ap_conf;

	os_bzero(&ap_conf, sizeof(struct softap_config));
	wifi_softap_get_config(&ap_conf);

	if (argc == 0) {
		char buf[MSG_BUF_LEN];
		uint8_t len;
		len = os_sprintf(buf, "SSID=%s PASSWORD=%s AUTHMODE=%d CHANNEL=%d\n", ap_conf.ssid, ap_conf.password, ap_conf.authmode, ap_conf.channel);
		espconn_sent(conn, buf, len);
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
	} else if (argc == 1) {
		os_strncpy(ap_conf.ssid, ssid, sizeof(ap_conf.ssid));
		os_bzero(ap_conf.password, sizeof(ap_conf.password));
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
		ap_conf.authmode = AUTH_OPEN;
		ap_conf.channel = 6;
		ETS_UART_INTR_DISABLE();
		wifi_softap_set_config(&ap_conf);
		ETS_UART_INTR_ENABLE();
	} else if (argc == 2) {
		os_strncpy(ap_conf.ssid, ssid, sizeof(ap_conf.ssid));
		os_strncpy(ap_conf.password, password, sizeof(ap_conf.password));
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
		ap_conf.authmode = AUTH_WPA_PSK;
		ap_conf.channel = 6;
		ETS_UART_INTR_DISABLE();
		wifi_softap_set_config(&ap_conf);
		ETS_UART_INTR_ENABLE();
	} else {
		espconn_sent(conn, MSG_ERROR, strlen(MSG_ERROR));
	}
}

const config_commands_t config_commands[] = { 
		{ "RESET", &config_cmd_reset }, 
		{ "MODE", &config_cmd_mode },
		{ "STA", &config_cmd_sta },
		{ "AP", &config_cmd_ap },
		{ NULL, NULL }
	};

void config_parse(struct espconn *conn, char *buf, int len) {
	char *lbuf = (char *)os_malloc(len + 1), **argv;
	uint8_t i, argc;
	// we need a '\0' end of the string
	os_memcpy(lbuf, buf, len);
	lbuf[len] = '\0';
	
	// command echo
	//espconn_sent(conn, lbuf, len);

	// remove any CR / LF
	for (i = 0; i < len; ++i)
		if (lbuf[i] == '\n' || lbuf[i] == '\r')
			lbuf[i] = '\0';

	// verify the command prefix
	if (os_strncmp(lbuf, "+++AT", 5) != 0) {
		return;
	}
	// parse out buffer into arguments
	argv = config_parse_args(&lbuf[5], &argc);
#if 0
// debugging
	{
		uint8_t i;
		size_t len;
		char buf[MSG_BUF_LEN];
		for (i = 0; i < argc; ++i) {
			//len = os_snprintf(buf, MSG_BUF_LEN, "argument %d: '%s'\r\n", i, argv[i]);
			len = os_sprintf(buf, "argument %d: '%s'\r\n", i, argv[i]);
			espconn_sent(conn, buf, len);
		}
	}
// end debugging
#endif
	if (argc == 0) {
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
	} else {
		argc--;	// to mimic C main() argc argv
		for (i = 0; config_commands[i].command; ++i) {
			if (os_strncmp(argv[0], config_commands[i].command, strlen(argv[0])) == 0) {
				config_commands[i].function(conn, argc, argv);
				break;
			}
		}
		if (!config_commands[i].command)
			espconn_sent(conn, MSG_INVALID_CMD, strlen(MSG_INVALID_CMD));
	}
	config_parse_args_free(argc, argv);
	os_free(lbuf);
}

#ifdef CONFIG_PARSE_TEST_UNIT
const int max_line = 255;
int main(int argc, char *argv[]) {
	char line[max_line];

	// read lines and feed them to config_parse
	while (fgets(line, max_line, stdin) != NULL) {
		uint8_t len = strlen(line);
		config_parse(NULL, line, len);
	}
}
#endif

#endif
