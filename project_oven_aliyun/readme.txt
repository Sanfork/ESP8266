Porting Step

1.Creat project_oven same to project_template
2.change sample_lib to aliyun_lib
3.change project_oven Makefile  line26 sample-lib to aliyun_lib
                                line50 sample_lib/libsample.a  to aliyun_lib/libaliyun.a
4.copy common document to aliyun_lib and rename aliyun_common.
  copy the makefile of folder1 to aliyun_common,change libfolder1.a to libaliyun_common.a
  move all the files of src to aliyun_common document.
5.copy the freerotos document of platform to aliyun_lib and rename aliyun_platform.
  copy the makefile of folder1 to aliyun_platform.,change libfolder1.a to libaliyun_platform.a
  move all the files of src to aliyun_platform. document.
6.copy the MQTTPacket document of src/mqtt to aliyun_lib .
  copy the makefile of folder1 to MQTTPacket.,change libfolder1.a to libMQTTPacket.a
  move all the files of src to MQTTPacket document.
7.copy the MQTTClient-C document of src/mqtt to aliyun_lib .
  copy the makefile of folder1 to MQTTClient_C.,change libfolder1.a to libMQTTClient_C.a
  move all the files of src to MQTTClient_C document. 
  I: tcp module :use the file of nettype\tcp repalce the same name file

/**(RAM is not enough, cancel)***/
  II: tsl module :use the file of nettype\tls repalce the same name file  
      copy the c and h files of  \public_libs\porting\mbedtls to MQTTClient_C document
      project_oven\Makefile line 85 add 	-lmbedtls	\
						-lopenssl	\
/********************************/

8 copy include/mqtt  to include 
 /******not to do*****/ 
 replace the config.h of include\mbedtls by the config.h of public_libs\mbedtls\configs\aliyun_iot\mbedtls
 /***************/
10 modify Makefile of aliyun_lib.
	GEN_LIBS = libaliyun.a
	COMPONENTS_libaliyun = aliyun_common/libaliyun_common.a \
					   aliyun_platform/libaliyun_platform.a \
					   aliyun_public_libs/mbedtls.a \
					   MQTTClient_C/libMQTTClient_C.a \
					   MQTTPacket/libMQTTPacket.a
11copy the whole code of demon.c tp user.c
12 modify Makefile of project_oven
  add: INCLUDES += -I $(PDIR)/include/mqtt
	INCLUDES += -I $(PDIR)/aliyun_lib/aliyun_common/inc
	INCLUDES += -I $(PDIR)/aliyun_lib/aliyun_platform/inc
	INCLUDES += -I $(PDIR)/aliyun_lib/MQTTClient_C
	INCLUDES += -I $(PDIR)/aliyun_lib/MQTTPacket
	INCLUDES += -I $(SDK_PATH)/include/freertos
	INCLUDES += -I $(SDK_PATH)/driver_lib/include
	INCLUDES += -I $(SDK_PATH)/include/espressif
	INCLUDES += -I $(SDK_PATH)/include/lwip
	INCLUDES += -I $(SDK_PATH)/include/lwip/ipv4
	INCLUDES += -I $(SDK_PATH)/include/lwip/ipv6
	INCLUDES += -I $(SDK_PATH)/include/lwip/posix
	INCLUDES += -I $(SDK_PATH)/include/nopoll
	INCLUDES += -I $(SDK_PATH)/include/spiffs
	INCLUDES += -I $(SDK_PATH)/include/ssl
	INCLUDES += -I $(SDK_PATH)/include/json
	INCLUDES += -I $(SDK_PATH)/include/lwip/lwip
	INCLUDES += -I $(SDK_PATH)/include

13 Modify SemaphoreHandle_t  to xSemaphoreHandle
   Modify TaskHandle_t to xTaskHandle
   delete the BOOL define in aliyun_iot_common_datatype.h
14 aliyun_iot_platform_threadsync.c line 5
   Modify xSemaphoreCreateBinary to  xSemaphoreCreateMutex

15 in  aliyun_iot_platform_network.c
   add #include "netdb.h"
   in include/lwip/lwip of RTOS_SDK, 
   add below code to netdb.h
   /* Flag values for getaddrinfo function. */
	#define AI_PASSIVE      0x1	/* Intend socket address for bind. */
	#define AI_CANONNAME    0x2	/* Return canonical node name. */
	#define AI_NUMERICHOST  0x4	/* Input is address, don't resolve. */
	#define AI_NUMERICSERV  0x8	/* Input is port number, don't resolve. */
	#define AI_ALL          0x100	/* Return v4-mapped and v6 addresses. */
	#define AI_ADDRCONFIG   0x400	/* Only available on Vista.  Unchangable default
				   on older systems. */
	#define AI_V4MAPPED     0x800
	/* Glibc extensions. We use numerical values taken by winsock-specific
	   extensions. */
	#define AI_IDN          0x4000	/* Encode IDN input from current local to
					   punycode per RFC 3490. */
	#define AI_CANONIDN     0x8000	/* Convert ai_canonname from punycode to IDN
					   in current locale. */
	#define AI_IDN_ALLOW_UNASSIGNED 0x10000	    /* Allow unassigned code points in
						       input string.  */
	#define AI_IDN_USE_STD3_ASCII_RULES 0x20000 /* Filter ASCII chars according to
						       STD3 rules.  */
16 in aliyun_iot_auth.c line 333
  Modify errCode[0] to  errCode
	
	
17 aliyun_iot_auth.c line 337
 Modify  if(NULL != info->errCode)  to  if(strlen(info->errCode))
 
18 aliyun_iot_platform_pthread.c in aliyun_iot_pthread_create
 Modify 2048 to 512
  
19 aliyun_iot_common_json.c
   in order to void  multiple definition of `cJSON_ParseWithOpts' with third_party/josn/cJSON.c
   modify  cJSON_ParseWithOpts to cJSON_ParseWithOpts_I



