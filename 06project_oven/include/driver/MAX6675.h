/*
 * MAX6675.h
 *
 *  Created on: 2017��1��7��
 *      Author: fxsl01455
 */

#ifndef EXAMPLES_06PROJECT_OVEN_INCLUDE_DRIVER_MAX6675_H_
#define EXAMPLES_06PROJECT_OVEN_INCLUDE_DRIVER_MAX6675_H_

#include "espressif/c_types.h"
#include "gpio.h"
#include "espressif/esp8266/eagle_soc.h"
#include "espressif/esp8266/gpio_register.h"
#include "espressif/esp8266/pin_mux_register.h"

//Define SPI hardware modules
#define MAX6675_CS   15
#define MAX6675_CK   14
#define MAX6675_SO	 13


#define HIGH 1
#define LOW 0

void MAX6675_init(void);
uint16 MAX6675_Read(void);
uint16 MAX6675_Temp(uint16 TC_Num);

#endif /* EXAMPLES_06PROJECT_OVEN_INCLUDE_DRIVER_MAX6675_H_ */
