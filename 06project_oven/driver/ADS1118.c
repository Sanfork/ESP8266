/*
 * Sanfork code
*/

#include "../include/driver/ADS1118.h"


/*
 * Functionname:ADS1118_Read
 * Describe: Send and read data
 * parameter: the data sent to ADS1118
 * return:read data from ADS1118
*/
uint8 ADS1118_Read(uint8 data){
	uint8 i,temp,Din;
	temp=data;
	Din=0;
	for(i=0;i<8;i++)
	{
		Din = Din<<1;
		if(0x80&temp)
		   GPIO_OUTPUT_SET(SPI_OUT,HIGH);
		else
		   GPIO_OUTPUT_SET(SPI_OUT,LOW);
		os_delay_us(1);
		   GPIO_OUTPUT_SET(SPI_CK,HIGH);
		os_delay_us(1);
		if(GPIO_INPUT_GET(SPI_IN))
			Din |= 0x01;
		os_delay_us(1);
		GPIO_OUTPUT_SET(SPI_CK,LOW);
		os_delay_us(1);
		temp = (temp<<1);
	}
	return Din;
}

/*
 * Functionname:ADS1118_Get_Voltage
 * Describe: get the whole data from ADS1118
 * parameter: NONE
 * return:read data from ADS1118
*/
uint16 ADS1118_Get_Voltage(void)
{
	unsigned int i=0;
	unsigned char Data_REG_H,Data_REG_L;
	unsigned int Data_REG;

	os_delay_us(1);
	// while((GPIO_INPUT_GET(SPI_IN))&&(i<1000)) i++;
	// ADS1118_Read(0x05);//(unsigned char)((Control_Regist_MSB>>8)));
	//ADS1118_Read(0x9B);//(unsigned char)Control_Regist_LSB);

	Data_REG_H=ADS1118_Read(0x04);//((unsigned char)(Control_Regist_MSB>>8)));
	Data_REG_L=ADS1118_Read(0x99);//(unsigned char)Control_Regist_LSB);
	Data_REG=(Data_REG_H<<8)+Data_REG_L;
	/*
	if(Data_REG>=0x8000)
	{
	Data_REG=0xFFFF-Data_REG;
	ADS1118_Voltage=(-1.0)*((Data_REG*FS/0x8000));
	}
	else
	 ADS1118_Voltage=(1.0)*((Data_REG*FS/32768)); */
	return Data_REG;
}

void ADS1118_init(void){

        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U,FUNC_GPIO15);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U,FUNC_GPIO14);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U,FUNC_GPIO13);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U,FUNC_GPIO12);

        GPIO_OUTPUT_SET(SPI_CS,LOW);    //pull up cs
        GPIO_OUTPUT_SET(SPI_CK,LOW);    //pull down ck
        GPIO_OUTPUT_SET(SPI_OUT,LOW);   //pull down out
        GPIO_DIS_OUTPUT(SPI_IN);                //set si to input

	ADS1118_Read(0x04);
	ADS1118_Read(0x9B);
}


