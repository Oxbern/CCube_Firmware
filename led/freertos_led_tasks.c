#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "spi.h"
#include "tim.h"
#include "adc.h"
#include "led.h"

#include <stdio.h>

extern uint16_t **led_buffer;
extern SPI_HandleTypeDef hspi1;

void StartLedTask(void const * argument)
{

	int i = 0;

	/*
	led_set(1,1,0);
	led_set(1,1,1);
	led_set(1,1,8);
	led_set(1,1,7);
	*/

	led_update(9);

	osDelay(500);

	led_set(0,0,0);
	
	led_update(0);	

    while(1)
    {
		/*
		if (!led_update(i))
		{
			printf("Error Trying to update\n");
		}
		i = (i+1)%9;
		*/
		osDelay(1);
    }
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
	char p_str[100];


	while(1)
	{
		if (HAL_ADC_Start(&hadc1) != HAL_OK)
		{
			printf("Error Starting ADC\n");
		}
		if (HAL_ADC_PollForConversion(&hadc1, 1000) != HAL_OK)
		{
			printf("Error polling ADC convertion\n");
		}
		conval = HAL_ADC_GetValue(&hadc1);
		if (HAL_ADC_Stop(&hadc1) != HAL_OK)
		{
			printf("Error Stopping ADC\n");
		}
 		prescale = map(conval, 0, 0x0FFF, 0, 333);

		sprintf(p_str, "map(%x) = %u", (unsigned)conval, (unsigned)prescale);

		//GUI_DispStringAt(p_str, 500, 0);

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
		osDelay(20);
	}

}
