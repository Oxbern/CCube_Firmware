#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "GUI.h"

#include <stdio.h>

osThreadId touchTaskHandle;
void StartTouchTask(void const * argument);

osThreadId watchTaskHandle;
void StartWatchTask(void const * argument);

void FREERTOS_Init(void)
{
	osThreadDef(touchTask, StartTouchTask, osPriorityNormal, 0, 1024);
	touchTaskHandle = osThreadCreate(osThread(touchTask), NULL);

	osThreadDef(watchTask, StartWatchTask, osPriorityNormal, 0, 1024);
	watchTaskHandle = osThreadCreate(osThread(watchTask), NULL);
}

void StartWatchTask(void const * argument)
{
	printf("watch task started\n");
	while(1)
	{
		printf("poke\n");
		osDelay(500);
	}
}

extern I2C_HandleTypeDef I2cHandle;

void StartTouchTask(void const * argument)
{
	uint8_t I2C_RX_Buffer[0x1F];
	printf("touch task started\n");
    while(1)
    {
		if (HAL_GPIO_ReadPin(I2Cx_IT_GPIO_PORT, I2Cx_IT_PIN) == GPIO_PIN_RESET)
		{
			if (HAL_I2C_Master_Receive(&I2cHandle, (uint16_t)I2C_ADDRESS, (uint8_t*)I2C_RX_Buffer, 0x1F, I2C_TIMEOUT) != HAL_OK)
			{
				printf("I2C Com Problem\n");
			}
			uint32_t i = 3;
			uint16_t x = (((uint16_t)(I2C_RX_Buffer[i] & 0x0F)) << 8) | ((uint16_t)I2C_RX_Buffer[i+1]);
			uint16_t y = (((uint16_t)(I2C_RX_Buffer[i+2] & 0x0F)) << 8) | ((uint16_t)I2C_RX_Buffer[i+3]);
			GUI_DrawCircle(x, y, 20);
		}
		osDelay(5);
    }
}
