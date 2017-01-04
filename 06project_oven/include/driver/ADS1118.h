/*
 * Sanfork code
*/

#ifndef SPI_APP_H
#define SPI_APP_H

#include "espressif/c_types.h"
#include "gpio.h"
#include "espressif/esp8266/eagle_soc.h"
#include "espressif/esp8266/gpio_register.h"
#include "espressif/esp8266/pin_mux_register.h"

//Define SPI hardware modules
#define SPI_CS   15
#define SPI_CK   14
#define SPI_IN	 13
#define SPI_OUT  12

#define HIGH 1
#define LOW 0

uint8 ADS1118_Read(uint8);
uint16 ADS1118_Get_Voltage(void);

#endif

