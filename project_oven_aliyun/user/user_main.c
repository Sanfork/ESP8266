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
#include "espconn.h"
#include "lwip/mem.h"
#include "semphr.h"

#include <stdio.h>

#include "aliyun_iot_mqtt_common.h"
#include "aliyun_iot_mqtt_client.h"
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#include "aliyun_iot_common_log.h"
#include "aliyun_iot_auth.h"
#include "FreeRTOS.h"
#include "task.h"

#define HOST_NAME       "iot.channel.aliyun.com"
#define PRODUCT_KEY    	"1000118719"
#define PRODUCT_SECRET 	"kQ5ZNciHxMwe4b1y"
#define DEVICE_NAME    	"oven_d"
#define DEVICE_SECRET  	"1Kk6VAUXQUi1m3g7"
#define TOPIC          	"/1000118719/oven_d/upload"

#define MSG_LEN_MAX 100


/**Number of publish thread*/
#define MAX_PUBLISH_THREAD_COUNT 2

#define DNS_ENABLE
#define packet_size (2*1024)
LOCAL os_timer_t test_timer;
LOCAL struct espconn user_tcp_conn;
LOCAL struct _esp_tcp user_tcp;
ip_addr_t tcp_server_ip;

//callback of publish
static void messageArrived(MessageData *md)
{
    char msg[MSG_LEN_MAX] = {0};

    MQTTMessage *message = md->message;
    if(message->payloadlen > MSG_LEN_MAX - 1)
    {
        printf("process part of receive message\n");
        message->payloadlen = MSG_LEN_MAX - 1;
    }

	memcpy(msg,message->payload,message->payloadlen);

	printf("Message : %s\n", msg);
}

static void publishComplete(void* context, unsigned int msgId)
{
    printf("publish message is arrived,id = %d\n",msgId);
}

static void subAckTimeout(SUBSCRIBE_INFO_S *subInfo)
{
    printf("msgId = %d,sub ack is timeout\n",subInfo->msgId);
}

void  pubThread(void*param)
{
    char buf[MSG_LEN_MAX] = { 0 };
    static int num = 0;
    int rc = 0 ;
    Client * client = (Client*)param;
    MQTTMessage message;

    int msgId[5] = {0};

    static int threadID = 0;
    int id = threadID++;

    for(;;)
    {
        int i = 0;
        for(i = 0; i < 5; i++)
        {
            memset(&message,0x0,sizeof(message));
            sprintf(buf, "Hello World! threadId = %d, num = %d\n",id,num++);
            message.qos = QOS1;
            message.retained = FALSE_IOT;
            message.dup = FALSE_IOT;
            message.payload = (void *) buf;
            message.payloadlen = strlen(buf);
            message.id = 0;
            rc = aliyun_iot_mqtt_publish(client, TOPIC, &message);
            if (0 != rc)
            {
                printf("ali_iot_mqtt_publish failed ret = %d\n", rc);
            }
            else
            {
                msgId[i] = message.id;
            }

            aliyun_iot_pthread_taskdelay(5000);
        }

        aliyun_iot_pthread_taskdelay(1000);
    }

    return ;
}

int multiThreadDemo(unsigned char *msg_buf,unsigned char *msg_readbuf)
{
    int rc = 0;
    memset(msg_buf,0x0,MSG_LEN_MAX);
    memset(msg_readbuf,0x0,MSG_LEN_MAX);

    IOT_DEVICEINFO_SHADOW_S deviceInfo;
    memset(&deviceInfo, 0x0, sizeof(deviceInfo));

    deviceInfo.productKey = PRODUCT_KEY;
    deviceInfo.productSecret = PRODUCT_SECRET;
    deviceInfo.deviceName = DEVICE_NAME;
    deviceInfo.deviceSecret = DEVICE_SECRET;
    deviceInfo.hostName = HOST_NAME;
    if (0 != aliyun_iot_set_device_info(&deviceInfo))
    {
        printf("run aliyun_iot_set_device_info() error!\n");
        return -1;
    }

    if (0 != aliyun_iot_auth(HMAC_MD5_SIGN_TYPE, IOT_VALUE_FALSE))
    {
        printf("run aliyun_iot_auth() error!\n");
        return -1;
    }

    Client client;
    memset(&client,0x0,sizeof(client));
    IOT_CLIENT_INIT_PARAMS initParams;
    memset(&initParams,0x0,sizeof(initParams));

    initParams.mqttCommandTimeout_ms = 2000;
    initParams.pReadBuf = msg_readbuf;
    initParams.readBufSize = MSG_LEN_MAX;
    initParams.pWriteBuf = msg_buf;
    initParams.writeBufSize = MSG_LEN_MAX;
    initParams.disconnectHandler = NULL;
    initParams.disconnectHandlerData = (void*) &client;
    initParams.deliveryCompleteFun = publishComplete;
    initParams.subAckTimeOutFun = subAckTimeout;
    rc = aliyun_iot_mqtt_init(&client, &initParams);
    if (0 != rc)
    {
        printf("ali_iot_mqtt_init failed ret = %d\n", rc);
        return rc;
    }

    MQTTPacket_connectData connectParam;
    memset(&connectParam,0x0,sizeof(connectParam));
    connectParam.cleansession = 1;
    connectParam.MQTTVersion = 4;
    connectParam.keepAliveInterval = 180;
    connectParam.willFlag = 0;
    rc = aliyun_iot_mqtt_connect(&client, &connectParam);
    if (0 != rc)
    {
        printf("ali_iot_mqtt_connect failed ret = %d\n", rc);
        return rc;
    }

    rc = aliyun_iot_mqtt_subscribe(&client, TOPIC, QOS1, messageArrived);
    if (0 != rc)
    {
        printf("ali_iot_mqtt_subscribe failed ret = %d\n", rc);
        return rc;
    }

    do
    {
        aliyun_iot_pthread_taskdelay(100);
        rc = aliyun_iot_mqtt_suback_sync(&client, TOPIC, messageArrived);
    }while(rc != SUCCESS_RETURN);

	ALIYUN_IOT_PTHREAD_S publishThread[MAX_PUBLISH_THREAD_COUNT];
	unsigned iter = 0;
	for (iter = 0; iter < MAX_PUBLISH_THREAD_COUNT; iter++)
	{
		rc = xTaskCreate(pubThread, "pub_thread", 512, &client, 1, &publishThread[iter].threadID);
		if(1 == rc)
		{
			printf("create publish thread success ");
		}
		else
		{
			printf("create publish thread success failed");
		}
	}

    INT32 ch;
    do
    {
        ch = getchar();
        aliyun_iot_pthread_taskdelay(100);
    } while (ch != 'Q' && ch != 'q');

	int i = 0;
	for (i = 0; i < MAX_PUBLISH_THREAD_COUNT; i++)
	{
		vTaskDelete(publishThread[i].threadID);
	}

    aliyun_iot_mqtt_release(&client);

    return 0;
}

int singleThreadDemo(unsigned char *msg_buf,unsigned char *msg_readbuf)
{
    int rc = 0;
    char buf[MSG_LEN_MAX] = { 0 };

    IOT_DEVICEINFO_SHADOW_S deviceInfo;
    memset(&deviceInfo, 0x0, sizeof(deviceInfo));

    deviceInfo.productKey = PRODUCT_KEY;
    deviceInfo.productSecret = PRODUCT_SECRET;
    deviceInfo.deviceName = DEVICE_NAME;
    deviceInfo.deviceSecret = DEVICE_SECRET;
    deviceInfo.hostName = HOST_NAME;
    if (0 != aliyun_iot_set_device_info(&deviceInfo))
    {
        printf("run aliyun_iot_set_device_info() error!\n");
        return -1;
    }
    printf(" hiram 1 \n");

    if (0 != aliyun_iot_auth(HMAC_MD5_SIGN_TYPE, IOT_VALUE_FALSE))
    {
        printf("run aliyun_iot_auth() error!\n");
        return -1;
    }
    printf(" hiram 0 \n");
    Client client;
    memset(&client,0x0,sizeof(client));
    IOT_CLIENT_INIT_PARAMS initParams;
    memset(&initParams,0x0,sizeof(initParams));

    initParams.mqttCommandTimeout_ms = 2000;
    initParams.pReadBuf = msg_readbuf;
    initParams.readBufSize = MSG_LEN_MAX;
    initParams.pWriteBuf = msg_buf;
    initParams.writeBufSize = MSG_LEN_MAX;
    initParams.disconnectHandler = NULL;
    initParams.disconnectHandlerData = (void*) &client;
    initParams.deliveryCompleteFun = publishComplete;
    initParams.subAckTimeOutFun = subAckTimeout;

    rc = aliyun_iot_mqtt_init(&client, &initParams);
    if (0 != rc)
    {
        printf("aliyun_iot_mqtt_init failed ret = %d\n", rc);
        return rc;
    }
    printf(" hiram 2 \n");
    MQTTPacket_connectData connectParam;
    memset(&connectParam,0x0,sizeof(connectParam));
    connectParam.cleansession = 1;
    connectParam.MQTTVersion = 4;
    connectParam.keepAliveInterval = 180;
    connectParam.willFlag = 0;

    rc = aliyun_iot_mqtt_connect(&client, &connectParam);
    if (0 != rc)
    {
        aliyun_iot_mqtt_release(&client);
        printf("ali_iot_mqtt_connect failed ret = %d\n", rc);
        return rc;
    }
    printf(" hiram 3 \n");
    rc = aliyun_iot_mqtt_subscribe(&client, TOPIC, QOS1, messageArrived);
    if (0 != rc)
    {
        aliyun_iot_mqtt_release(&client);
        printf("ali_iot_mqtt_subscribe failed ret = %d\n", rc);
        return rc;
    }
    printf(" hiram 4 \n");
    do
    {
        aliyun_iot_pthread_taskdelay(100);
        rc = aliyun_iot_mqtt_suback_sync(&client, TOPIC, messageArrived);
    }while(rc != SUCCESS_RETURN);

    MQTTMessage message;
    memset(&message,0x0,sizeof(message));
    sprintf(buf, "{\"message\":\"Hello World\"}");
    message.qos = QOS1;
    message.retained = FALSE_IOT;
    message.dup = FALSE_IOT;
    message.payload = (void *) buf;
    message.payloadlen = strlen(buf);
    message.id = 0;
    rc = aliyun_iot_mqtt_publish(&client, TOPIC, &message);
    if (0 != rc)
    {
        aliyun_iot_mqtt_release(&client);
        printf("ali_iot_mqtt_publish failed ret = %d\n", rc);
        return rc;
    }

    INT32 ch;
    do
    {
        ch = getchar();
        aliyun_iot_pthread_taskdelay(100);
    } while (ch != 'Q' && ch != 'q');

    aliyun_iot_mqtt_release(&client);

    aliyun_iot_pthread_taskdelay(10000);
    return 0;
}

int mqtt_client_demo()
{
    printf("start demo!\n");

    unsigned char *msg_buf = (unsigned char *)malloc(MSG_LEN_MAX);
    unsigned char *msg_readbuf = (unsigned char *)malloc(MSG_LEN_MAX);

    if (0 != aliyun_iot_auth_init())
    {
        printf("run aliyun_iot_auth_init error!\n");
        return -1;
    }

   // singleThreadDemo(msg_buf,msg_readbuf);
    multiThreadDemo(msg_buf,msg_readbuf);

    free(msg_buf);
    free(msg_readbuf);

    (void) aliyun_iot_auth_release();

    printf("out of demo!\n");

    return 0;
}



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
/********************************************************************************************************************/
#define NET_DOMAIN "iot.eclipse.org"
#define pheadbuffer "GET / HTTP/1.1\r\nUser-Agent: curl/7.37.0\r\nHost: %s\r\nAccept: */*\r\n\r\n"

xSemaphoreHandle xSemaphore = NULL;
/******************************************************************************
   * FunctionName : user_tcp_recv_cb
   * Description  : receive callback.
   * Parameters   : arg -- Additional argument to pass to the callback function
   * Returns      : none
 *******************************************************************************/
    LOCAL void ICACHE_FLASH_ATTR
user_tcp_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
    //received some data from tcp connection
    printf("tcp recv !!! %s \r\n", pusrdata);
}
/******************************************************************************
   * FunctionName : user_tcp_sent_cb
   * Description  : data sent callback.
   * Parameters   : arg -- Additional argument to pass to the callback function
   * Returns      : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_sent_cb(void *arg)
{
    //data sent successfully
    printf("tcp sent succeed !!! \r\n");
}
/******************************************************************************
   * FunctionName : user_tcp_discon_cb
   * Description  : disconnect callback.
   * Parameters   : arg -- Additional argument to pass to the callback function
   * Returns      : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_discon_cb(void *arg)
{
    //tcp disconnect successfully
    printf("tcp disconnect succeed !!! \r\n");
}
/******************************************************************************
   * FunctionName : user_esp_platform_sent
   * Description  : Processing the application data and sending it to the host
   * Parameters   : pespconn -- the espconn used to connetion with the host
   * Returns      : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_sent_data(struct espconn *pespconn)
{
    char* pbuf=(char*)os_zalloc(packet_size);
    sprintf(pbuf, pheadbuffer, NET_DOMAIN);
    espconn_sent(pespconn, pbuf, strlen(pbuf));
    free(pbuf);
}
/******************************************************************************
   * FunctionName : user_tcp_connect_cb
   * Description  : A new incoming tcp connection has been connected.
   * Parameters   : arg -- Additional argument to pass to the callback function
   * Returns      : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_connect_cb(void *arg)
{
    struct espconn *pespconn = arg;
    printf("connect succeed !!! \r\n");
    espconn_regist_recvcb(pespconn, user_tcp_recv_cb);
    espconn_regist_sentcb(pespconn, user_tcp_sent_cb);
    espconn_regist_disconcb(pespconn, user_tcp_discon_cb);
    user_sent_data(pespconn);
}
/******************************************************************************
   * FunctionName : user_tcp_recon_cb
   * Description  : reconnect callback, error occured in TCP connection.
   * Parameters   : arg -- Additional argument to pass to the callback function
   * Returns      : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_tcp_recon_cb(void *arg, sint8 err)
{
    //error occured , tcp connection broke. user can try to reconnect here.
    printf("reconnect callback, error code %d !!! \r\n",err);
}
#ifdef DNS_ENABLE
/******************************************************************************
   * FunctionName : user_dns_found
   * Description  : dns found callback
   * Parameters   : name -- pointer to the name that was looked up.
   *                ipaddr -- pointer to an ip_addr_t containing the IP address of
   *                the hostname, or NULL if the name could not be found (or on any
   *                other error).
   *                callback_arg -- a user-specified callback argument passed to
   *                dns_gethostbyname
   * Returns      : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;
    if (ipaddr == NULL)
    {
        printf("user_dns_found NULL \r\n");
        return;
    }
    //dns got ip
    printf("user_dns_found %d.%d.%d.%d \r\n",
              *((uint8 *)&ipaddr->addr), *((uint8 *)&ipaddr->addr + 1),
              *((uint8 *)&ipaddr->addr + 2), *((uint8 *)&ipaddr->addr + 3));
    if (tcp_server_ip.addr == 0 && ipaddr->addr != 0)
    {
        // dns succeed, create tcp connection
        os_timer_disarm(&test_timer);
        tcp_server_ip.addr = ipaddr->addr;
        memcpy(pespconn->proto.tcp->remote_ip, &ipaddr->addr, 4); // remote ip of tcp server which get by dns
        pespconn->proto.tcp->remote_port = 80; // remote port of tcp server
        pespconn->proto.tcp->local_port = espconn_port(); //local port of ESP8266
        espconn_regist_connectcb(pespconn, user_tcp_connect_cb); // register connect callback
        espconn_regist_reconcb(pespconn, user_tcp_recon_cb); // register reconnect callback as error handler
        espconn_connect(pespconn); // tcp connect
    }
}
/******************************************************************************
   * FunctionName : user_esp_platform_dns_check_cb
   * Description  : 1s time callback to check dns found
   * Parameters   : arg -- Additional argument to pass to the callback function
   * Returns      : none
 *******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
user_dns_check_cb(void *arg)
{
    struct espconn *pespconn = arg;
    espconn_gethostbyname(pespconn, NET_DOMAIN, &tcp_server_ip, user_dns_found); // recall DNS function
    os_timer_arm(&test_timer, 1000, 0);
}
#endif
/******************************************************************************
   * FunctionName : user_check_ip
   * Description  : check whether get ip addr or not
   * Parameters   : none
   * Returns      : none
 *******************************************************************************/

void ICACHE_FLASH_ATTR
user_check_ip(void)
{
    struct ip_info ipconfig;
    //disarm timer first
    os_timer_disarm(&test_timer);
    //get ip info of ESP8266 station
    wifi_get_ip_info(STATION_IF, &ipconfig);
    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0)
    {
        printf("got ip !!! \r\n");
        // Connect to tcp server as NET_DOMAIN
        user_tcp_conn.proto.tcp = &user_tcp;
        user_tcp_conn.type = ESPCONN_TCP;
        user_tcp_conn.state = ESPCONN_NONE;
#ifdef DNS_ENABLE
        tcp_server_ip.addr = 0;
        espconn_gethostbyname(&user_tcp_conn, NET_DOMAIN, &tcp_server_ip, user_dns_found); // DNS function

        os_timer_setfn(&test_timer, (os_timer_func_t *)user_dns_check_cb, &user_tcp_conn);
        os_timer_arm(&test_timer, 1000, 0);
#else
        const char esp_tcp_server_ip[4] = {192,168,1,100}; // remote IP of TCP server
        memcpy(user_tcp_conn.proto.tcp->remote_ip, esp_tcp_server_ip, 4);
        user_tcp_conn.proto.tcp->remote_port = 1000;// remote port
        user_tcp_conn.proto.tcp->local_port = espconn_port();//localportofESP8266
        espconn_regist_connectcb(&user_tcp_conn, user_tcp_connect_cb); // register connect callback
        espconn_regist_reconcb(&user_tcp_conn, user_tcp_recon_cb); // register reconnect callback as error handler
        espconn_connect(&user_tcp_conn);
#endif
    }
    else
    {
        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
             wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
             wifi_station_get_connect_status() == STATION_CONNECT_FAIL))
        {
            printf("connect fail !!! \r\n");
        }
        else
        {
            //re-arm timer to check ip
            os_timer_setfn(&test_timer, (os_timer_func_t *)user_check_ip, NULL);
            os_timer_arm(&test_timer, 100, 0);
        }
    }
}
/******************************************************************************
   * FunctionName : user_set_station_config
   * Description  : set the router info which ESP8266 station will connect to
   * Parameters   : none
   * Returns      : none
 *******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
    // Wifi configuration
    char ssid[32] = "9152ND";
    char password[64] = "hiram666";
    struct station_config stationConf;
    memset(stationConf.ssid, 0, 32);
    memset(stationConf.password, 0, 64);
    //need not mac address
    stationConf.bssid_set = 0;
    //Set ap settings
    memcpy(&stationConf.ssid, ssid, 32);
    memcpy(&stationConf.password, password, 64);
    wifi_station_set_config(&stationConf);
    //set a timer to check whether got ip from router succeed or not.
    os_timer_disarm(&test_timer);
    os_timer_setfn(&test_timer, (os_timer_func_t *)user_check_ip, NULL);
    os_timer_arm(&test_timer, 100, 0);
}


xTaskHandle xHandle1 = NULL;
xTaskHandle xHandle2 = NULL;

/* Task to be created. */
void connect_router( void * pvParameters )
{
    printf("set station mode\n");
    wifi_set_opmode(STATIONAP_MODE);
    user_set_station_config();
    vTaskDelete(xHandle1);
}

/* Task to be created. */
void connect_mqttserver( void * pvParameters )
{
    while(user_tcp_conn.state != ESPCONN_CONNECT)
    {
        printf("user_tcp_conn.state = %d\n",user_tcp_conn.state);
    }
    printf("begin mqtt_client_demo...\n");
    mqtt_client_demo();
    vTaskDelete(xHandle2);
}
/******************************************************************************
   * FunctionName : user_init
   * Description  : entry of user application, init user function here
   * Parameters   : none
   * Returns      : none
 *******************************************************************************/
void user_init(void)
{
	uart_init_new();
	printf("SDK version:%s\n", system_get_sdk_version());

    portBASE_TYPE xReturned1;
    portBASE_TYPE xReturned2;
    /* Create the task, storing the handle. */

#if 0
    xSemaphore = xSemaphoreCreateMutex();
#endif

    xReturned1 = xTaskCreate(
                            connect_router,       /* Function that implements the task. */
                            "connet to router",          /* Text name for the task. */
                            512,      /* Stack size in words, not bytes. */
                            ( void * ) 1,    /* Parameter passed into the task. */
                            tskIDLE_PRIORITY,/* Priority at which the task is created. */
                            &xHandle1 );      /* Used to pass out the created task's handle. */


#if 1
    xReturned2 = xTaskCreate(
                            connect_mqttserver,       /* Function that implements the task. */
                            "connet to mqttserver",          /* Text name for the task. */
                            2048,      /* Stack size in words, not bytes. */
                            ( void * ) 1,    /* Parameter passed into the task. */
                            tskIDLE_PRIORITY,/* Priority at which the task is created. */
                            &xHandle2 );      /* Used to pass out the created task's handle. */
#endif

}

