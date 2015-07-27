/*
 * File	: user_main.c
 * This file is part of Espressif's AT+ command set program.
 * Copyright (C) 2013 - 2016, Espressif Systems
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.	If not, see <http://www.gnu.org/licenses/>.
 */
#include "c_types.h"

#include "config.h"
#include "flash_param.h"
void user_rf_pre_init(void)
{
}

void user_init(void)
{
	uint8_t i;

	//wifi_set_opmode(3); //STA+AP

#ifdef CONFIG_DYNAMIC
	flash_param_t *flash_param;
	flash_param_init();
	flash_param = flash_param_get();
	serialInit(flash_param->baud, flash_param->uartconf0);
#else
	serialInit(115200, 0);
#endif
//	os_printf("size flash_param_t %d\n", sizeof(flash_param_t));


#ifdef CONFIG_STATIC
	// refresh wifi config
	config_execute();
#endif

#ifdef CONFIG_DYNAMIC
	serverInit(21); // only AT commands
	tcpuartInit(flash_param->port); // only bridge (TCP <-> UART)
#else
	serverInit(21); // only AT commands
	tcpuartInit(23); // only bridge (TCP <-> UART)
#endif

#ifdef CONFIG_GPIO
	ioInit();
#endif

}
