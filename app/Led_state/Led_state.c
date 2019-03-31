/*
 * Led_state.c
 *
 *  Created on: 2019Äê3ÔÂ25ÈÕ
 *      Author: Feng
 */

#include "esp_common.h"
#include "esp8266/ets_sys.h"
#include "lwip/lwip/mem.h"
#include "esp8266/eagle_soc.h"
#include "driver/gpio.h"
#include "Led_state/Led_state.h"


static void ICACHE_FLASH_ATTR
Led_state_gpio_init(void)
{
	GPIO_ConfigTypeDef  GPIOConfig;

	GPIOConfig.GPIO_Pin = LED1_IO_FUNC | LED2_IO_FUNC | LED3_IO_FUNC;
	GPIOConfig.GPIO_Mode = GPIO_Mode_Output;
	GPIOConfig.GPIO_Pullup = GPIO_PullUp_EN;
	GPIOConfig.GPIO_IntrType = GPIO_PIN_INTR_DISABLE;
	gpio_config( &GPIOConfig );
}

static void ICACHE_FLASH_ATTR
Led_state_task(void *pvParameters)
{
	Led_state_gpio_init();

	for( ; ; )
	{
		GPIO_OUTPUT_SET(LED1_IO_NUM, 1);
		GPIO_OUTPUT_SET(LED2_IO_NUM, 1);
		GPIO_OUTPUT_SET(LED3_IO_NUM, 0);
		vTaskDelay( 1000/portTICK_RATE_MS );

		GPIO_OUTPUT_SET(LED1_IO_NUM, 1);
		GPIO_OUTPUT_SET(LED2_IO_NUM, 1);
		GPIO_OUTPUT_SET(LED3_IO_NUM, 1);
		vTaskDelay( 1000/portTICK_RATE_MS );
	}

	vTaskDelete(NULL);
}


void ICACHE_FLASH_ATTR
Led_state_init(void)
{
	xTaskCreate(Led_state_task, "Led_state_task", 256, NULL, 3, NULL);
}

