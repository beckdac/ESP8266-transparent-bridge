#include "c_types.h"
#include "osapi.h"
#include "ets_sys.h"
#include "io.h"
#include "driver/gpio16.h"

static ETSTimer resetBtntimer;

#define MSG_ERROR "ERROR\r\n"

void config_cmd_gpio2(serverConnData *conn, uint8_t argc, char *argv[]) {
	int laststate = 1;
	int i;

	if (argc == 0) {
		laststate = gpio_read(GPIO_2_PIN);
		espbuffsentprintf(conn, "GPIO2=%d\r\n", laststate);
	}
	else {
		uint16_t gpiodelay = 100;
		if (argc == 2) {
			gpiodelay = atoi(argv[2]);
		}
		uint8_t value = atoi(argv[1]);
		if (value < 3) {
			if (value == 0) {
				gpio_write(GPIO_2_PIN, value);
				espbuffsentstring(conn, "LOW\r\n");
			}
			if (value == 1) {
				gpio_write(GPIO_2_PIN, value);
				espbuffsentstring(conn, "HIGH\r\n");
			}
			if (value == 2) {
				gpio_pulse(GPIO_2_PIN, gpiodelay*1000);
				espbuffsentstring(conn, "RESET\r\n");
			}
		} else {
			espbuffsentstring(conn, MSG_ERROR);
		}
	}
}

void config_cmd_gpio4(serverConnData *conn, uint8_t argc, char *argv[]) {
	if (argc == 0)
		espbuffsentprintf(conn, "Args: 0=low, 1=high, 2 <delay in ms>=reset (delay optional).\r\n");
	else {
		uint16_t gpiodelay = 100;
		if (argc == 2) {
			gpiodelay = atoi(argv[2]);
		}
		uint8_t value = atoi(argv[1]);
		if (value < 3) {
			if (value == 0) {
				gpio_write(GPIO_4_PIN, value);
				espbuffsentstring(conn, "LOW\r\n");
			}
			if (value == 1) {
				gpio_write(GPIO_4_PIN, value);
				espbuffsentstring(conn, "HIGH\r\n");
			}
			if (value == 2) {
				gpio_pulse(GPIO_4_PIN, gpiodelay*1000);
				espbuffsentstring(conn, "RESET\r\n");
			}
		} else {
			espbuffsentstring(conn, MSG_ERROR);
		}
	}
}

void config_cmd_gpio5(serverConnData *conn, uint8_t argc, char *argv[]) {
	if (argc == 0)
		espbuffsentprintf(conn, "Args: 0=low, 1=high, 2 <delay in ms>=reset (delay optional).\r\n");
	else {
		uint16_t gpiodelay = 100;
		if (argc == 2) {
			gpiodelay = atoi(argv[2]);
		}
		uint8_t value = atoi(argv[1]);
		if (value < 3) {
			if (value == 0) {
				gpio_write(GPIO_5_PIN, value);
				espbuffsentstring(conn, "LOW\r\n");
			}
			if (value == 1) {
				gpio_write(GPIO_5_PIN, value);
				espbuffsentstring(conn, "HIGH\r\n");
			}
			if (value == 2) {
				gpio_pulse(GPIO_5_PIN, gpiodelay*1000);
				espbuffsentstring(conn, "RESET\r\n");
			}
		} else {
			espbuffsentstring(conn, MSG_ERROR);
		}
	}
}

void config_cmd_gpio12(serverConnData *conn, uint8_t argc, char *argv[]) {
	if (argc == 0)
		espbuffsentprintf(conn, "Args: 0=low, 1=high, 2 <delay in ms>=reset (delay optional).\r\n");
	else {
		uint16_t gpiodelay = 100;
		if (argc == 2) {
			gpiodelay = atoi(argv[2]);
		}
		uint8_t value = atoi(argv[1]);
		if (value < 3) {
			if (value == 0) {
				gpio_write(GPIO_12_PIN, value);
				espbuffsentstring(conn, "LOW\r\n");
			}
			if (value == 1) {
				gpio_write(GPIO_12_PIN, value);
				espbuffsentstring(conn, "HIGH\r\n");
			}
			if (value == 2) {
				gpio_pulse(GPIO_12_PIN, gpiodelay*1000);
				espbuffsentstring(conn, "RESET\r\n");
			}
		} else {
			espbuffsentstring(conn, MSG_ERROR);
		}
	}
}

void config_cmd_gpio13(serverConnData *conn, uint8_t argc, char *argv[]) {
	if (argc == 0)
		espbuffsentprintf(conn, "Args: 0=low, 1=high, 2 <delay in ms>=reset (delay optional).\r\n");
	else {
		uint16_t gpiodelay = 100;
		if (argc == 2) {
			gpiodelay = atoi(argv[2]);
		}
		uint8_t value = atoi(argv[1]);
		if (value < 3) {
			if (value == 0) {
				gpio_write(GPIO_13_PIN, value);
				espbuffsentstring(conn, "LOW\r\n");
			}
			if (value == 1) {
				gpio_write(GPIO_13_PIN, value);
				espbuffsentstring(conn, "HIGH\r\n");
			}
			if (value == 2) {
				gpio_pulse(GPIO_13_PIN, gpiodelay*1000);
				espbuffsentprintf(conn, "RESET %d ms\r\n",gpiodelay);
			}
		} else {
			espbuffsentstring(conn, MSG_ERROR);
		}
	}
}

void config_cmd_gpio14(serverConnData *conn, uint8_t argc, char *argv[]) {
	int laststate = 1;

	if (argc == 0) {
		laststate = gpio_read(GPIO_14_PIN);
		espbuffsentprintf(conn, "GPIO14=%d\r\n", laststate);
	} else {
		uint16_t gpiodelay = 1;
		if (argc == 2) {
			gpiodelay = atoi(argv[2]);
		}
		uint8_t value = atoi(argv[1]);
		if (value < 3) {
			if (value == 0) {
				gpio_write(GPIO_14_PIN, value);
				espbuffsentstring(conn, "LOW\r\n");
			}
			if (value == 1) {
				gpio_write(GPIO_14_PIN, value);
				espbuffsentstring(conn, "HIGH\r\n");
			}
			if (value == 2) {
				gpio_pulse(GPIO_14_PIN, gpiodelay*1000);
				espbuffsentstring(conn, "RESET\r\n");
			}
		} else {
			espbuffsentstring(conn, MSG_ERROR);
		}
	}
}

void config_cmd_gpio15(serverConnData *conn, uint8_t argc, char *argv[]) {
	if (argc == 0)
		espbuffsentprintf(conn, "Args: 0=low, 1=high, 2 <delay in ms>=reset (delay optional).\r\n");
	else {
		uint16_t gpiodelay = 100;
		if (argc == 2) {
			gpiodelay = atoi(argv[2]);
		}
		uint8_t value = atoi(argv[1]);
		if (value < 3) {
			if (value == 0) {
				gpio_write(GPIO_15_PIN, value);
				espbuffsentstring(conn, "LOW\r\n");
			}
			if (value == 1) {
				gpio_write(GPIO_15_PIN, value);
				espbuffsentstring(conn, "HIGH\r\n");
			}
			if (value == 2) {
				gpio_pulse(GPIO_15_PIN, gpiodelay*1000);
				espbuffsentstring(conn, "RESET\r\n");
			}
		} else {
			espbuffsentstring(conn, MSG_ERROR);
		}
	}
}

void config_cmd_gpio16(serverConnData *conn, uint8_t argc, char *argv[]) {
	if (argc == 0)
		espbuffsentprintf(conn, "Args: 0=low, 1=high, 2 <delay in ms>=reset (delay optional).\r\n");
	else {
		uint16_t gpiodelay = 100;
		if (argc == 2) {
			gpiodelay = atoi(argv[2]);
		}
		uint8_t value = atoi(argv[1]);
		if (value < 3) {
			if (value == 0) {
				gpio_write(GPIO_16_PIN, value);
				espbuffsentstring(conn, "LOW\r\n");
			}
			if (value == 1) {
				gpio_write(GPIO_16_PIN, value);
				espbuffsentstring(conn, "HIGH\r\n");
			}
			if (value == 2) {
				gpio_pulse(GPIO_16_PIN, gpiodelay*1000);
				espbuffsentstring(conn, "RESET\r\n");
			}
		} else {
			espbuffsentstring(conn, MSG_ERROR);
		}
	}
}

void ICACHE_FLASH_ATTR intr_callback(unsigned pin, unsigned level) {
	switch (pin) {
	case GPIO_14_PIN:
		gpio_write(GPIO_2_PIN, level);
		break;
	default:
		while (pin) {
			gpio_write(GPIO_2_PIN, 1);
			gpio_write(GPIO_2_PIN, 0);
			pin--;
		}
		break;
	}
}

static void ICACHE_FLASH_ATTR pulseTimer(void *arg) {
	gpio_write(GPIO_2_PIN, 1);
	os_delay_us(50);
	gpio_write(GPIO_2_PIN, 0);
}

static void ICACHE_FLASH_ATTR enableInterrupt(unsigned pin, unsigned gpio_type, gpio_intr_handler icb) {
	set_gpio_mode(pin, GPIO_INT, GPIO_FLOAT);
	gpio_intr_init(pin, gpio_type);
	gpio_intr_attach(icb);
}

void ioInit() {

	// GPIO2 output
	set_gpio_mode(GPIO_2_PIN, GPIO_OUTPUT, GPIO_FLOAT);
	gpio_write(GPIO_2_PIN, 0);

	// GPIO4 output
	set_gpio_mode(GPIO_4_PIN, GPIO_OUTPUT, GPIO_FLOAT);
	gpio_write(GPIO_4_PIN, 0);

	// GPIO5 output
	set_gpio_mode(GPIO_5_PIN, GPIO_OUTPUT, GPIO_FLOAT);
	gpio_write(GPIO_5_PIN, 0);

	// GPIO12 output
	set_gpio_mode(GPIO_12_PIN, GPIO_OUTPUT, GPIO_FLOAT);
	gpio_write(GPIO_12_PIN, 0);

	// GPIO13 output
	set_gpio_mode(GPIO_13_PIN, GPIO_OUTPUT, GPIO_FLOAT);
	gpio_write(GPIO_13_PIN, 0);

	// GPIO14 interrupt enable
	enableInterrupt(GPIO_14_PIN, GPIO_PIN_INTR_ANYEDGE, intr_callback);

	// GPIO15 output
	set_gpio_mode(GPIO_15_PIN, GPIO_OUTPUT, GPIO_FLOAT);
	gpio_write(GPIO_15_PIN, 0);

/*
	os_timer_disarm(&resetBtntimer);
	os_timer_setfn(&resetBtntimer, pulseTimer, NULL);
	os_timer_arm(&resetBtntimer, 100, 1);
*/
}

