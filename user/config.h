#ifndef __CONFIG_H__
#define __CONFIG_H__

#define STA_SSID	"ssid2join"
#define STA_PASSWORD	"password"
#define AP_SSID		"myssid"
#define AP_PASSWORD	"password"
// over ride in this file
#include "config_wifi.h"

void config_execute(void);

#endif /* __CONFIG_H__ */
