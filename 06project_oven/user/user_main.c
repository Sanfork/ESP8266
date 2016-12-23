/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "esp_common.h"
#include "user_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "gpio.h"


#define HIGH 1
#define LOW 0
#define KEY_QUEUE_LEN 4
uint8 keyValue;
struct TTP226keyCheck{
	BOOL TTP226CkStatus;
	uint8 checkbit;
}keyPinCheck;
uint times;

xQueueHandle keyTaskQueue;
/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


/******************************************************************************
 * FunctionName : ttp226_GetKeyValue
 * Description  : get ttp226 key value
 * Parameters   : none
 * Returns      : uint8
*******************************************************************************/
uint8 ttp226_GetKeyValue(void){
	uint8 i;
	uint8 keyValue = 0;

	os_delay_us(17);
	GPIO_OUTPUT_SET(TTP226_CK,LOW);
	GPIO_OUTPUT_SET(TTP226_RST,HIGH);
	os_delay_us(62);
	GPIO_OUTPUT_SET(TTP226_RST,LOW);
	//keyPinCheck.TTP226CkStatus=false;
	//keyPinCheck.checkbit=0;
	//os_timer_arm_us(keyTimer, 62 , 1);
	for(i=0;i<8;i++){
		keyValue = keyValue >>1;
		os_delay_us(62);
		GPIO_OUTPUT_SET(TTP226_CK,HIGH);
		if(GPIO_INPUT_GET(TTP226_D0) == 0){
			keyValue = keyValue & 0x7f;
		} else {
			keyValue = keyValue | 0x80;
		}
		os_delay_us(62);
		GPIO_OUTPUT_SET(TTP226_CK,LOW);
	}
	return keyValue;
}

/******************************************************************************
 * FunctionName : key_gpio_intertupt_handler
 * Description  : key
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void key_gpio_intertupt_handler(void *para){
	uint16 gpioStatus = 0;
	uint8 keyValue = 0xAB;
	portBASE_TYPE xStatus;

	/*clear interrupt*/
	gpioStatus = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpioStatus);
	printf("Receive Interrupt message 0x%x\r\n", gpioStatus);
	times = 0;
	if(gpioStatus = GPIO_ID_PIN(TTP226_DV)){
		xStatus = xQueueSendToBack( keyTaskQueue, &keyValue, 0 );
	}
}
/******************************************************************************
 * FunctionName : key_task
 * Description  : key task function
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void key_task(void *pvParameters){
	uint8 keyValueBuff;
	portBASE_TYPE xStatus;
	static BOOL level=false;
	while(1){
		xStatus = xQueueReceive( keyTaskQueue, &keyValueBuff, portMAX_DELAY );
		if (keyValueBuff == 0xAA ) {
			times++;
			printf("Receive Cycle message %d\r\n",times);
			if(level){
				GPIO_OUTPUT_SET(TTP226_RST,LOW);
				level = false;
			} else {
				GPIO_OUTPUT_SET(TTP226_RST,HIGH);
				level = true;
			}

		}
		if (keyValueBuff == 0xAB ) {
			keyValue    = ttp226_GetKeyValue();
			printf("Receive Interrupt message 0x%x\r\n",keyValue);

		}

	}
	vTaskDelete( NULL );
}

void cycle_task(void *pvParameters){

	uint8 keyValue = 0xAA;

	portBASE_TYPE xStatus;

	while(1){
		xStatus = xQueueSendToBack( keyTaskQueue, &keyValue, 0 );
		vTaskDelay( 100 );
	}
}
/******************************************************************************
 * FunctionName : keyInit
 * Description  : the  key initializing
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void key_init(){

	keyTaskQueue = xQueueCreate( 5, sizeof( uint8 ) );


	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15);

	//GPIO_OUTPUT_SET(TTP226_CK,LOW);
	//GPIO_OUTPUT_SET(TTP226_RST,HIGH);
	//os_delay_us(100);
	GPIO_OUTPUT_SET(TTP226_RST,LOW);
	portENTER_CRITICAL();
	GPIO_DIS_OUTPUT(TTP226_D0);
	GPIO_DIS_OUTPUT(TTP226_DV);
	gpio_pin_intr_state_set(GPIO_ID_PIN(TTP226_DV),GPIO_PIN_INTR_POSEDGE);
	_xt_isr_attach(ETS_GPIO_INUM, (_xt_isr)key_gpio_intertupt_handler,NULL);
	_xt_isr_unmask(1<<ETS_GPIO_INUM);
	portENABLE_INTERRUPTS();
	portEXIT_CRITICAL();

	xTaskCreate(key_task,"keyTask",256,NULL,2,NULL);
	xTaskCreate(cycle_task,"keyTask",256,NULL,2,NULL);

}



/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	uart_init_new();
	printf("SDK version:%s\n", system_get_sdk_version());
	key_init();
	oled_i2c_master_gpio_init();
    wifi_set_opmode(STATIONAP_MODE);


}

