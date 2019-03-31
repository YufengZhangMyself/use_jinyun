/*
 * connect_wifi.c
 *
 *  Created on: 2019年3月25日
 *      Author: Feng
 */


#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "espressif/smartconfig.h"



static bool smartconfig_run_flag = false;
bool ICACHE_FLASH_ATTR get_smartconfig_run_flag(void)	{	return smartconfig_run_flag;	}
void ICACHE_FLASH_ATTR set_smartconfig_run_flag(void)	{	smartconfig_run_flag = true;	}
void ICACHE_FLASH_ATTR reset_smartconfig_run_flag(void)	{	smartconfig_run_flag = false;	}

/*
 * smartconfig 响应函数
 */
static void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status, void *pdata)
{
    switch(status) {
        case SC_STATUS_WAIT:
            printf("SC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            printf("SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            printf("SC_STATUS_GETTING_SSID_PSWD\n");
            sc_type *type = pdata;
            if (*type == SC_TYPE_ESPTOUCH) {
                printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
            } else {
                printf("SC_TYPE:SC_TYPE_AIRKISS\n");
            }
            break;
        case SC_STATUS_LINK:
            printf("SC_STATUS_LINK\n");
            struct station_config *sta_conf = pdata;

	        wifi_station_set_config(sta_conf);
	        wifi_station_disconnect();
	        wifi_station_connect();
            break;
        case SC_STATUS_LINK_OVER:
            printf("SC_STATUS_LINK_OVER\n");
            if (pdata != NULL) {
				//SC_TYPE_ESPTOUCH
                uint8 phone_ip[4] = {0};

                memcpy(phone_ip, (uint8*)pdata, 4);
                printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            } else {
            	//SC_TYPE_AIRKISS - support airkiss v2.0
//				airkiss_start_discover();
			}
            smartconfig_stop();
            reset_smartconfig_run_flag();
            break;
    }
}

/*
 * smartconfig 任务函数
 */
static void ICACHE_FLASH_ATTR
smartconfig_task(void *pvParameters)
{
	bool ret = false;
	u8 count = 0;

	printf("smartconfig version:%s\n", smartconfig_get_version());
	set_smartconfig_run_flag();						//smartcofig运行标志位置位

	count = 0;
	while((wifi_get_opmode() != STATION_MODE) && (++count < 5))//判断WiFi的工作模式是否是STA模式
	{
	    wifi_set_opmode(STATION_MODE);				//设置WiFi的工作模式为STA
	    vTaskDelay( 1000/portTICK_RATE_MS );
	}
	if(count >= 5)									//count大于等于5，WiFi设置模式失败
	{
		os_printf("set wifi opmode fail ! \n");
		reset_smartconfig_run_flag();				//smartcofig运行标志位复位
		vTaskDelete(NULL);							//删除任务
		return;
	}

	esptouch_set_timeout(90);						//设置smartconfig的超时时间
	smartconfig_set_type(SC_TYPE_ESPTOUCH);			//设置smartconfig的工作模式

	count = 0;
	do{
		ret = smartconfig_start(smartconfig_done);	//开启smartconfig模式
		vTaskDelay( 1000/portTICK_RATE_MS );
	}while((ret == false) && (++count < 5));		//判断是否开启成功
	if(count >= 5)									//count大于等于5，WiFi设置模式失败
	{
		os_printf("open smartconfig fail ! \n");
		reset_smartconfig_run_flag();				//smartcofig运行标志位复位
		smartconfig_stop();							//关闭smartconfig模式
		vTaskDelete(NULL);							//删除任务
		return;
	}

	vTaskDelay( 90*1000/portTICK_RATE_MS );			//设置smartconfig模式的开启时间为90秒

	if(get_smartconfig_run_flag() == true)			//判断smartconfig模式是否关闭，即连接WiFi失败
	{
		smartconfig_stop();							//关闭smartconfig模式
		reset_smartconfig_run_flag();				//smartcofig运行标志位复位
	}
    vTaskDelete(NULL);								//删除任务
}


void ICACHE_FLASH_ATTR
smartconfig_init(void)
{
	xTaskCreate(smartconfig_task, "smartconfig_task", 256, NULL, 2, NULL);
}


