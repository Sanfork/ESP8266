/*
 * oled.h
 *
 *  Created on: 2016Äê12ÔÂ23ÈÕ
 *      Author: fxsl01455
 */

#ifndef EXAMPLES_06PROJECT_OVEN_INCLUDE_DRIVER_OLED_H_
#define EXAMPLES_06PROJECT_OVEN_INCLUDE_DRIVER_OLED_H_

#include "i2c_master.h"

/*i2c gpio initializing*/
void oled_i2c_master_gpio_init(void);
void ICACHE_FLASH_ATTR Initial_M096128x64_ssd1306();
void ICACHE_FLASH_ATTR oled_display_On(void);
void ICACHE_FLASH_ATTR oled_display_Off(void);
void ICACHE_FLASH_ATTR oled_clear(void);
void ICACHE_FLASH_ATTR oled_drawPoint(u8 x,u8 y,u8 t);
void ICACHE_FLASH_ATTR oled_showChar(u8 x,u8 y,u8 chr,u8 size,u8 mode);
void ICACHE_FLASH_ATTR oled_showString(u8 x,u8 y,const u8 *p);

#endif /* EXAMPLES_06PROJECT_OVEN_INCLUDE_DRIVER_OLED_H_ */
