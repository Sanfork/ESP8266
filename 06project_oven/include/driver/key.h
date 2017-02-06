/*
 * key.h
 *
 *  Created on: 2016/12/27
 *      Author: hiram
 */

#ifndef EXAMPLES_06PROJECT_OVEN_INCLUDE_DRIVER_KEY_H_
#define EXAMPLES_06PROJECT_OVEN_INCLUDE_DRIVER_KEY_H_
#include "freertos/queue.h"

uint8 getKeyValue(void);
void key_init(xQueueHandle queue);
uint8 ttp226_GetKeyValueAgain(void);

#endif /* EXAMPLES_06PROJECT_OVEN_INCLUDE_DRIVER_KEY_H_ */
