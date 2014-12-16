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
	//os_snprintf(&ap_conf.password[strlen(AP_PASSWORD)], sizeof(ap_conf.password) - strlen(AP_PASSWORD), "_%02X%02X%02X", macaddr[3], macaddr[4], macaddr[5]);
	os_sprintf(&ap_conf.password[strlen(AP_PASSWORD)], "_%02X%02X%02X", macaddr[3], macaddr[4], macaddr[5]);
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
		*argc++;
		if (*argc == MAX_ARGS) {
			break;
		}
	}
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

	if (argc < 1)
		espconn_sent(conn, MSG_ERROR, strlen(MSG_ERROR));
	else {
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

void config_cmd_sta(struct espconn *conn, uint8_t argc, char *argv[]) {
	char *ssid = argv[1], *password = argv[2];
	struct station_config sta_conf;

	if (argc != 2) {
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

void config_cmd_ap(struct espconn *conn, uint8_t argc, char *argv[]) {
	char *ssid = argv[1], *password = argv[2];
	struct softap_config ap_conf;

	if (argc != 2) {
		char buf[MSG_BUF_LEN];
		espconn_sent(conn, MSG_ERROR, strlen(MSG_ERROR));
	} else {
		os_strncpy(ap_conf.ssid, ssid, sizeof(ap_conf.ssid));
		os_strncpy(ap_conf.password, password, sizeof(ap_conf.password));
		espconn_sent(conn, MSG_OK, strlen(MSG_OK));
		ap_conf.authmode = AUTH_WPA_PSK;
		ap_conf.channel = 6;
		ETS_UART_INTR_DISABLE();
		wifi_softap_set_config(&ap_conf);
		ETS_UART_INTR_ENABLE();
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

	espconn_sent(conn, lbuf, len);

	// remove any CR / LF
	for (i = 0; i < len; ++i)
		if (lbuf[i] == '\n' || lbuf[i] == '\r')
			lbuf[i] = '\0';

	// verify the command prefix
	if (os_strncmp(lbuf, "+++AT", 5) != 0) {
		return;
	}
	// parse out buffer into arguments
	argv = config_parse_args(lbuf, &argc);
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

#endif
