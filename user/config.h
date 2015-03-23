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

#define CONFIG_DYNAMIC
#ifdef CONFIG_DYNAMIC

typedef struct config_commands {
	char *command;
	void(*function)(serverConnData *conn, uint8_t argc, char *argv[]);
} config_commands_t;


void config_parse(serverConnData *conn, char *buf, int len);

#endif

#define CONFIG_GPIO
#ifdef CONFIG_GPIO
#include "gpio.h"

void config_gpio(void);

#endif

#endif /* __CONFIG_H__ */
