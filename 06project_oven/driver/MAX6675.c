/*
 * MAX6675.c
 *
 *  Created on: 2017��1��7��
 *      Author: fxsl01455
 */
#include "driver/MAX6675.h"

/*
 * FunctionName: MAX6675_init
 * Description: initialize GPIO
 * parameter: NONE
 * return: void
*/
void MAX6675_init(void){

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15);

	GPIO_OUTPUT_SET(MAX6675_CS,HIGH);  	//pull up cs
	GPIO_OUTPUT_SET(MAX6675_CK,LOW);	//pull down ck
	GPIO_DIS_OUTPUT(MAX6675_SO);		//set SO to input
}

/*
 * FunctionName: MAX6675_Read
 * Description: Read MAX6675
 * parameter: NONE
 * return: Read data
*/
uint16 MAX6675_Read(void){
	uint16 Dat_Out=0;
	uint8 Cyc=0; //cycle times

	GPIO_OUTPUT_SET(MAX6675_CS,LOW);  //Set CS LOW
	os_delay_us(1);
	for(Cyc=0;Cyc<16;Cyc++){
		if(Cyc!=0){//The first bit is not used,so CK can not be pull up and down
			GPIO_OUTPUT_SET(MAX6675_CK,HIGH);
			os_delay_us(1);
			GPIO_OUTPUT_SET(MAX6675_CK,LOW);
		}
		if(GPIO_INPUT_GET(MAX6675_SO)){
			Dat_Out++;
		}
		if(Cyc!=15){
			Dat_Out<<=1;
		}
	}
	GPIO_OUTPUT_SET(MAX6675_CS,HIGH);;
	return Dat_Out;
}

/*
 * FunctionName: MAX6675_Temp
 * Description: temper converter
 * parameter: input data
 * return: FFFF open
 *         others temperature
*/
uint16 MAX6675_Temp(uint16 TC_Num){
	u16 Temp;
	if(TC_Num &4 )	return 0xFFFF;
	Temp=(((TC_Num&0x7fff)>>3)*25)/100;//get D14-D3
	return Temp;
}

