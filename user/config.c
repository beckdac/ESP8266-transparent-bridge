#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "osapi.h"

#include "config.h"

void config_execute(void) {
	uint8_t mode;
	struct station_config sta_conf;
	struct softap_config ap_conf;
	uint8_t macaddr[6];

	// make sure the device is in AP and STA combined mode
	mode = wifi_get_mode();
	if (mode != STATIONAP_MODE) {
		wifi_set_mode(STATIONAP_MODE);
		system_restart();
	}

	// connect to our station
	os_strncpy(sta_conf.ssid, STA_SSID, sizeof(sta_conf.ssid));
	os_strncpy(sta_conf.password, STA_PASSWORD, sizeof(sta_conf.password));
	wifi_station_set_config(&sta_conf);		
	wifi_station_disconnect();	// probably not necessary
	wifi_station_connect();

	// setup the soft AP
	wifi_get_macaddr(SOFTAP_IF, macaddr);
	os_strncpy(ap_conf.ssid, AP_SSID, sizeof(ap_conf.ssid));
	os_strncpy(ap_conf.password, AP_PASSWORD, sizeof(ap_conf.password));
	ap_conf.authmode = AUTH_WPA_PSK;
	wifi_softap_set_config(&ap_conf);
}
