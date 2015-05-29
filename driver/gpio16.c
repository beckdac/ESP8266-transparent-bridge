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

#include "ets_sys.h"
#include "osapi.h"
#include "driver/gpio16.h"

uint8_t pin_num[GPIO_PIN_NUM];
uint8_t pin_func[GPIO_PIN_NUM];
uint32_t pin_mux[GPIO_PIN_NUM];
uint32_t pin_mux[GPIO_PIN_NUM] = {PAD_XPD_DCDC_CONF,  PERIPHS_IO_MUX_GPIO5_U,  PERIPHS_IO_MUX_GPIO4_U, 	 PERIPHS_IO_MUX_GPIO0_U,
								PERIPHS_IO_MUX_GPIO2_U, PERIPHS_IO_MUX_MTMS_U, PERIPHS_IO_MUX_MTDI_U, PERIPHS_IO_MUX_MTCK_U,
								PERIPHS_IO_MUX_MTDO_U, PERIPHS_IO_MUX_U0RXD_U, PERIPHS_IO_MUX_U0TXD_U, PERIPHS_IO_MUX_SD_DATA2_U,
								PERIPHS_IO_MUX_SD_DATA3_U };
uint8_t pin_num[GPIO_PIN_NUM] = {16, 5, 4, 0,
								2,  14,  12, 13,
								15,  3,  1, 9,
								10};
uint8_t pin_func[GPIO_PIN_NUM] = {0, FUNC_GPIO5, FUNC_GPIO4, FUNC_GPIO0,
								FUNC_GPIO2,  FUNC_GPIO14,  FUNC_GPIO12,  FUNC_GPIO13,
								FUNC_GPIO15,  FUNC_GPIO3,  FUNC_GPIO1, FUNC_GPIO9,
								FUNC_GPIO10};
#ifdef GPIO_INTERRUPT_ENABLE
GPIO_INT_TYPE pin_int_type[GPIO_PIN_NUM] = {
								GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE,
								GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE,
								GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE, GPIO_PIN_INTR_DISABLE,
								GPIO_PIN_INTR_DISABLE};
#endif

void ICACHE_FLASH_ATTR gpio16_output_conf(void)
{
	WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
			(READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 	// mux configuration for XPD_DCDC to output rtc_gpio0

	WRITE_PERI_REG(RTC_GPIO_CONF,
			(READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable

	WRITE_PERI_REG(RTC_GPIO_ENABLE,
			(READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe) | (uint32)0x1);	//out enable
}

void ICACHE_FLASH_ATTR gpio16_output_set(uint8 value)
{
	WRITE_PERI_REG(RTC_GPIO_OUT,
			(READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xfffffffe) | (uint32)(value & 1));
}

void ICACHE_FLASH_ATTR gpio16_input_conf(void)
{
	WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
			(READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 	// mux configuration for XPD_DCDC and rtc_gpio0 connection

	WRITE_PERI_REG(RTC_GPIO_CONF,
			(READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable

	WRITE_PERI_REG(RTC_GPIO_ENABLE,
			READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe);	//out disable
}

uint8 ICACHE_FLASH_ATTR gpio16_input_get(void)
{
	return (uint8)(READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
}

int ICACHE_FLASH_ATTR set_gpio_mode(unsigned pin, unsigned mode, unsigned pull)
{
	if (pin >= GPIO_PIN_NUM)
		return -1;
	if(pin == 0) {
		if(mode == GPIO_INPUT)
			gpio16_input_conf();
		else
			gpio16_output_conf();
		return 1;
	}

	switch(pull) {
		case GPIO_PULLUP:
			PIN_PULLDWN_DIS(pin_mux[pin]);
			PIN_PULLUP_EN(pin_mux[pin]);
			break;
		case GPIO_PULLDOWN:
			PIN_PULLUP_DIS(pin_mux[pin]);
			PIN_PULLDWN_EN(pin_mux[pin]);
			break;
		case GPIO_FLOAT:
			PIN_PULLUP_DIS(pin_mux[pin]);
			PIN_PULLDWN_DIS(pin_mux[pin]);
			break;
		default:
			PIN_PULLUP_DIS(pin_mux[pin]);
			PIN_PULLDWN_DIS(pin_mux[pin]);
			break;
	}

	switch(mode) {
		case GPIO_INPUT:
			GPIO_DIS_OUTPUT(pin_num[pin]);
			break;
		case GPIO_OUTPUT:
			ETS_GPIO_INTR_DISABLE();
#ifdef GPIO_INTERRUPT_ENABLE
			pin_int_type[pin] = GPIO_PIN_INTR_DISABLE;
#endif
			PIN_FUNC_SELECT(pin_mux[pin], pin_func[pin]);
			//disable interrupt
			gpio_pin_intr_state_set(GPIO_ID_PIN(pin_num[pin]), GPIO_PIN_INTR_DISABLE);
			//clear interrupt status
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(pin_num[pin]));
			GPIO_REG_WRITE(GPIO_PIN_ADDR(GPIO_ID_PIN(pin_num[pin])), GPIO_REG_READ(GPIO_PIN_ADDR(GPIO_ID_PIN(pin_num[pin]))) & (~ GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_ENABLE))); //disable open drain;
			ETS_GPIO_INTR_ENABLE();
			break;
#ifdef GPIO_INTERRUPT_ENABLE
		case GPIO_INT:
			ETS_GPIO_INTR_DISABLE();
			PIN_FUNC_SELECT(pin_mux[pin], pin_func[pin]);
			GPIO_DIS_OUTPUT(pin_num[pin]);
			gpio_register_set(GPIO_PIN_ADDR(GPIO_ID_PIN(pin_num[pin])), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
					| GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
					| GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));
			ETS_GPIO_INTR_ENABLE();
			break;
#endif
		default:
			break;
	}
	return 1;
}

int ICACHE_FLASH_ATTR gpio_write(unsigned pin, unsigned level)
{
	if (pin >= GPIO_PIN_NUM)
		return -1;
	if(pin == 0){
		gpio16_output_conf();
		gpio16_output_set(level);
		return 1;
	}
	GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), level);
	return 1;
}

int ICACHE_FLASH_ATTR gpio_pulse(unsigned pin, unsigned long delay) {
	if (pin >= GPIO_PIN_NUM)
			return -1;
	if(pin == 0){
		gpio16_output_conf();
		gpio16_output_set(1);
		os_delay_us(delay);
		gpio16_output_set(0);
		return 1;
	}
	GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 1);
	os_delay_us(delay);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(pin_num[pin]), 0);
	return 1;
}

int ICACHE_FLASH_ATTR gpio_read(unsigned pin)
{
	if (pin >= GPIO_PIN_NUM)
		return -1;
	if(pin == 0){
		// gpio16_input_conf();
		return 0x1 & gpio16_input_get();
	}
	// GPIO_DIS_OUTPUT(pin_num[pin]);
	return 0x1 & GPIO_INPUT_GET(GPIO_ID_PIN(pin_num[pin]));
}

#ifdef GPIO_INTERRUPT_ENABLE
void ICACHE_FLASH_ATTR gpio_intr_dispatcher(gpio_intr_handler cb)
{
	uint8 i, level;
	uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	for (i = 0; i < GPIO_PIN_NUM; i++) {
		if (pin_int_type[i] && (gpio_status & BIT(pin_num[i])) ) {
			//disable global interrupt
			ETS_GPIO_INTR_DISABLE();
			//disable interrupt
			gpio_pin_intr_state_set(GPIO_ID_PIN(pin_num[i]), GPIO_PIN_INTR_DISABLE);
			//clear interrupt status
			GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(pin_num[i]));
			level = 0x1 & GPIO_INPUT_GET(GPIO_ID_PIN(pin_num[i]));
			if(cb){
				cb(i, level);
			}
			//enable interrupt
			gpio_pin_intr_state_set(GPIO_ID_PIN(pin_num[i]), pin_int_type[i]);
			//enable global interrupt
			ETS_GPIO_INTR_ENABLE();
		}
	}
}

void ICACHE_FLASH_ATTR gpio_intr_attach(gpio_intr_handler cb)
{
	ETS_GPIO_INTR_ATTACH(gpio_intr_dispatcher, cb);
}

int ICACHE_FLASH_ATTR gpio_intr_deattach(unsigned pin)
{
	if (pin >= GPIO_PIN_NUM)
		return -1;
	//disable global interrupt
	ETS_GPIO_INTR_DISABLE();
	//clear interrupt status
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(pin_num[pin]));
	pin_int_type[pin] = GPIO_PIN_INTR_DISABLE;
	//disable interrupt
	gpio_pin_intr_state_set(GPIO_ID_PIN(pin_num[pin]), pin_int_type[pin]);
	//enable global interrupt
	ETS_GPIO_INTR_ENABLE();
	return 1;
}

int ICACHE_FLASH_ATTR gpio_intr_init(unsigned pin, GPIO_INT_TYPE type)
{
	if (pin >= GPIO_PIN_NUM)
		return -1;
	//disable global interrupt
	ETS_GPIO_INTR_DISABLE();
	//clear interrupt status
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(pin_num[pin]));
	pin_int_type[pin] = type;
	//enable interrupt
	gpio_pin_intr_state_set(GPIO_ID_PIN(pin_num[pin]), type);
	//enable global interrupt
	ETS_GPIO_INTR_ENABLE();
	return 1;
}
#endif
