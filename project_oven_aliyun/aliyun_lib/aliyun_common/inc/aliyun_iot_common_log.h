/*********************************************************************************
 * 鏂囦欢鍚嶇О: aliyun_iot_common_log.h
 * 浣�       鑰�:
 * 鐗�       鏈�:
 * 鏃�       鏈�: 2016-05-30
 * 鎻�       杩�:
 * 鍏�       瀹�:
 * 鍘�       鍙�:
 **********************************************************************************/
#ifndef ALIYUN_IOT_COMMON_LOG_H
#define ALIYUN_IOT_COMMON_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include "aliyun_iot_common_datatype.h"
#include "aliyun_iot_common_error.h"
#define STD_OUT

//鏃ュ織绾у埆鍜岀被鍨�
typedef enum IOT_LOG_LEVEL
{
    DEBUG_IOT_LOG = 0,
    INFO_IOT_LOG,
    NOTICE_IOT_LOG,
    WARNING_IOT_LOG,
    ERROR_IOT_LOG,
    OFF_IOT_LOG,        //鏃ュ織鍏抽棴鏍囧織锛屾斁缃湪鏋氫妇绫诲瀷鏈�鍚�
}IOT_LOG_LEVEL_E;

//鍏ㄥ眬鏃ュ織绾у埆鏍囧織锛屽綋姝ゅ彉閲忓皬浜庣瓑浜庤杈撳嚭鏃ュ織绾у埆鏃舵棩蹇楄緭鍑烘湁鏁�
extern IOT_LOG_LEVEL_E g_iotLogLevel;

void aliyun_iot_common_log_init();
void aliyun_iot_common_log_release();
void aliyun_iot_common_set_log_level(IOT_LOG_LEVEL_E iotLogLevel);
IOT_LOG_LEVEL_E aliyun_iot_common_get_log_level();
void sdkLog(char* format,char* level,const char* file,int line,const char*function,...);

#ifdef STD_OUT
#define WRITE_IOT_DEBUG_LOG(format, ...) \
{\
    if(g_iotLogLevel <= DEBUG_IOT_LOG)\
    {\
        printf("[debug] %s:%d %s()| "format"\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}

#define WRITE_IOT_INFO_LOG(format, ...) \
{\
    if(g_iotLogLevel <= INFO_IOT_LOG)\
    {\
        printf("[info] %s:%d %s()| "format"\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}

#define WRITE_IOT_NOTICE_LOG(format, ...) \
{\
    if(g_iotLogLevel <= NOTICE_IOT_LOG)\
    {\
        printf("[notice] %s:%d %s()| "format"\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}

#define WRITE_IOT_WARNING_LOG(format, ...) \
{\
    if(g_iotLogLevel <= WARNING_IOT_LOG)\
    {\
        printf("[warning] %s:%d %s()| "format"\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}

#define WRITE_IOT_ERROR_LOG(format,...) \
{\
    if(g_iotLogLevel <= ERROR_IOT_LOG)\
    {\
        printf("[error] %s:%d %s()| "format"\n",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}

#else
#define WRITE_IOT_DEBUG_LOG(format, ...) \
{\
    if(g_iotLogLevel <= DEBUG_IOT_LOG)\
    {\
        sdkLog(format,"debug",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}

#define WRITE_IOT_INFO_LOG(format, ...) \
{\
    if(g_iotLogLevel <= INFO_IOT_LOG)\
    {\
        sdkLog(format,"info",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}

#define WRITE_IOT_NOTICE_LOG(format, ...) \
{\
    if(g_iotLogLevel <= NOTICE_IOT_LOG)\
    {\
        sdkLog(format,"notice",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}

#define WRITE_IOT_WARNING_LOG(format, ...) \
{\
    if(g_iotLogLevel <= WARNING_IOT_LOG)\
    {\
        sdkLog(format,"warning",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}

#define WRITE_IOT_ERROR_LOG(format, ...) \
{\
    if(g_iotLogLevel <= ERROR_IOT_LOG)\
    {\
        sdkLog(format,"error",__FILE__, __LINE__, __FUNCTION__,##__VA_ARGS__);\
    }\
}
#endif

#endif
