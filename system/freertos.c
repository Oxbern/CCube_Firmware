#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "tim.h"
#include "adc.h"
#include "GUI.h"

#include <stdio.h>

/*
 * Firmware Tasks
 */

osThreadId touchTaskHandle;
void StartTouchTask(void const * argument);

osThreadId pwmTaskHandle;
void StartPwmTask(void const * argument);


/**
 * FreeRTOS Initialisation function
 */
void FREERTOS_Init(void)
{
	osThreadDef(touchTask, StartTouchTask, osPriorityNormal, 0, 1024);
	touchTaskHandle = osThreadCreate(osThread(touchTask), NULL);

	osThreadDef(pwmTask, StartPwmTask, osPriorityNormal, 0, 1024);
	pwmTaskHandle = osThreadCreate(osThread(pwmTask), NULL);
}

/*
 * LED Light Level handling
 */

static inline int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void StartPwmTask(void const * argument)
{
	if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3) != HAL_OK)
	{
		printf("Error Starting PWM\n");
	}

	TIM_OC_InitTypeDef sConfigOC;
	uint32_t p = INI_PULSE_LENGTH;
	uint32_t conval, prescale;


	while(1)
	{
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 1000);
		conval = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
 		prescale = map(conval, 0, 0x0FFF, 0, 100);
		if (HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3)!= HAL_OK)
		{
			printf("Error Stoping PWM\n");
		}
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = p;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
		if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3)!= HAL_OK)
		{
			printf("Error Configuring PWM Channel\n");
		}
		if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3)!= HAL_OK)
		{
			printf("Error Starting PWM\n");
		}
		p = prescale?((TIM_PERIOD + 1) * prescale) / 100 - 1 : 0;
		// TODO implémentation d'un seuil variable pour la vérification
		osDelay(500);
	}
}


/*
 * Touch Screen handling
 */
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
