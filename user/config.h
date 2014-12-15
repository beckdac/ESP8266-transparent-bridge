#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define CONFIG_STATIC
#ifdef CONFIG_STATIC

#define STA_SSID	"ssid2join"
#define STA_PASSWORD	"password"
#define AP_SSID		"myssid"
#define AP_PASSWORD	"password"
// over ride in this file
#include "config_wifi.h"

void config_execute(void);

#endif

//#define CONFIG_DYNAMIC
#ifdef CONFIG_DYNAMIC

void config_parse(struct espconn *conn, char *buf, int len);

#endif

#endif /* __CONFIG_H__ */
