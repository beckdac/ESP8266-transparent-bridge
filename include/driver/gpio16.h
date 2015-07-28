/*
    Driver for GPIO
    Official repository: https://github.com/CHERTS/esp8266-gpio16

    Copyright (C) 2015 Mikhail Grigorev (CHERTS)

    Pin number:
    -----------
    Pin 0 = GPIO16
    Pin 1 = GPIO5
    Pin 2 = GPIO4
    Pin 3 = GPIO0
    Pin 4 = GPIO2
    Pin 5 = GPIO14
    Pin 6 = GPIO12
    Pin 7 = GPIO13
    Pin 8 = GPIO15
    Pin 9 = GPIO3
    Pin 10 = GPIO1
    Pin 11 = GPIO9
    Pin 12 = GPIO10

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __GPIO16_H__
#define __GPIO16_H__

#include "gpio.h"

// ESP-12-E -
// http://www.esp8266.com/wiki/lib/exe/fetch.php?w=600&tok=babab2&media=esp-12_pindef.png
#define GPIO_0_PIN  3 // GPIO0
#define GPIO_2_PIN  4 // GPIO2
#define GPIO_4_PIN  2 // GPIO4
#define GPIO_5_PIN  1 // GPIO5
#define GPIO_12_PIN 6 // GPIO12
#define GPIO_13_PIN 7 // GPIO13
#define GPIO_14_PIN 5 // GPIO14
#define GPIO_15_PIN 8 // GPIO15
#define GPIO_16_PIN 0 // GPIO16

#define GPIO_PIN_NUM 13
#define GPIO_INTERRUPT_ENABLE 1

#define GPIO_FLOAT 0
#define GPIO_PULLUP 1
#define GPIO_PULLDOWN 2

#define GPIO_INPUT 0
#define GPIO_OUTPUT 1
#define GPIO_INT 2

/* GPIO interrupt handler */
#ifdef GPIO_INTERRUPT_ENABLE
typedef void (* gpio_intr_handler)(unsigned pin, unsigned level);
#endif

void gpio16_output_conf(void);
void gpio16_output_set(uint8 value);
void gpio16_input_conf(void);
uint8 gpio16_input_get(void);
int set_gpio_mode(unsigned pin, unsigned mode, unsigned pull);
int gpio_write(unsigned pin, unsigned level);
int gpio_pulse(unsigned pin, unsigned long delay);
int gpio_read(unsigned pin);
#ifdef GPIO_INTERRUPT_ENABLE
void gpio_intr_attach(gpio_intr_handler cb);
int gpio_intr_deattach(unsigned pin);
int gpio_intr_init(unsigned pin, GPIO_INT_TYPE type);
#endif

#endif
