/*
 * key.c
 *
 *  Created on: 2016/12/27
 *      Author:hiram
 */
#include "esp_common.h"
#include "user_config.h"
#include "gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/key.h"

struct key_mgr{
	xQueueHandle aimQueue;
	xQueueHandle keyEventQueue;
	uint8 keyValue;
	EVENT_MESG keyEvent;
}keyMgrData;


/******************************************************************************
 * FunctionName : getKeyValue
 * Description  : get ttp226 key value
 * Parameters   : none
 * Returns      : uint8
*******************************************************************************/
uint8 getKeyValue(){
	return keyMgrData.keyValue;
}

/******************************************************************************
 * FunctionName : ttp226_GetKeyValue
 * Description  : get ttp226 key value
 * Parameters   : none
 * Returns      : uint8
*******************************************************************************/
static uint8 ttp226_GetKeyValue(void){
	uint8 i;
	uint8 keyValue = 0;

	os_delay_us(17);
	GPIO_OUTPUT_SET(TTP226_CK,LOW);
	GPIO_OUTPUT_SET(TTP226_RST,HIGH);
	os_delay_us(62);
	GPIO_OUTPUT_SET(TTP226_RST,LOW);

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

 uint8 ttp226_GetKeyValueAgain(void){
	uint8 i;
	uint8 keyValue = 0;

//	os_delay_us(17);
//	GPIO_OUTPUT_SET(TTP226_CK,LOW);
//	GPIO_OUTPUT_SET(TTP226_RST,HIGH);
//	os_delay_us(62);
//	GPIO_OUTPUT_SET(TTP226_RST,LOW);

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
static void key_gpio_intertupt_handler(void *para){
	uint16 gpioStatus = 0;
	uint8 keyEvent = keyMgrData.keyEvent;
	portBASE_TYPE xStatus;

	/*clear interrupt*/
	gpioStatus = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpioStatus);
	printf("Receive Interrupt message 0x%x\r\n", gpioStatus);
	if(gpioStatus = GPIO_ID_PIN(TTP226_DV)){
		xStatus = xQueueSendToBack( keyMgrData.keyEventQueue, &keyMgrData.keyEvent, 0 );
	}
}

/******************************************************************************
 * FunctionName : sysMgr_task
 * Description  : system manage task
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
static void key_task(void *pvParameters){
	uint8 keyValueBuff;
	portBASE_TYPE xStatus;
	while(1){
		xStatus = xQueueReceive( keyMgrData.keyEventQueue, &keyValueBuff, portMAX_DELAY );

		if (keyValueBuff == keyMgrData.keyEvent ) {
			keyMgrData.keyValue    = ttp226_GetKeyValue();
			printf("Receive Interrupt message 0x%x\r\n",keyMgrData.keyValue);
			xStatus = xQueueSendToBack( keyMgrData.aimQueue , &keyMgrData.keyEvent, 0 );
		}

	}
	vTaskDelete( NULL );
}

/******************************************************************************
 * FunctionName : keyInit
 * Description  : the  key initializing
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void key_init(xQueueHandle queue){

/*	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U,FUNC_GPIO0);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U,FUNC_GPIO4);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO15);*/
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15);

	GPIO_OUTPUT_SET(TTP226_RST,LOW);
	portENTER_CRITICAL();
	GPIO_DIS_OUTPUT(TTP226_D0);
	GPIO_DIS_OUTPUT(TTP226_DV);
	gpio_pin_intr_state_set(GPIO_ID_PIN(TTP226_DV),GPIO_PIN_INTR_POSEDGE);
	_xt_isr_attach(ETS_GPIO_INUM, (_xt_isr)key_gpio_intertupt_handler,NULL);
	_xt_isr_unmask(1<<ETS_GPIO_INUM);
	portENABLE_INTERRUPTS();
	portEXIT_CRITICAL();

	keyMgrData.keyEvent = KEY_EVENT;
	keyMgrData.keyEventQueue = xQueueCreate( 5, sizeof( uint8 ) );
	keyMgrData.aimQueue = queue;
	xTaskCreate(key_task,"keyTask",256,NULL,3,NULL);
}

