#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "stm32f4xx_hal.h"
#include "GUI.h"
#include "usbd_cdc.h"
#include "usb_device.h"
#include "ltdc.h"
#include "sdram.h"
#include "dma2d.h"
#include "i2c.h"
#include "crc.h"
#include "sd.h"
#include "spi.h"
#include "tim.h"
#include "adc.h"
#include "fatfs.h"
#include "json.h"
#include "json-builder.h"
#include "led.h"
#include "console.h"

#include <stdio.h>
#include <string.h>

/*
 * Firmware Tasks
 */

osThreadId initTaskHandle;
void StartInitTask(void const * argument);

osThreadId touchTaskHandle;
void StartTouchTask(void const * argument);

osThreadId pwmTaskHandle;
void StartPwmTask(void const * argument);

osThreadId ledTaskHandle;
void StartLedTask(void const * argument);

osThreadId fsTaskHandle;
void StartFsTask(void const * argument);


/**
 * FreeRTOS Initialisation function
 */
void FREERTOS_Init(void)
{
	osThreadDef(initTask, StartInitTask, osPriorityHigh, 0, 8192);
	initTaskHandle = osThreadCreate(osThread(initTask), NULL);

}

static __IO uint32_t uwTick;

void HAL_IncTick(void)
{
  uwTick++;
}

void HAL_Delay(volatile uint32_t millis)
{

  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
	//osDelay(millis);
	vTaskDelay(millis);
  } else {
	uint32_t t = uwTick;
	while((HAL_GetTick() - t) < millis)
	{
	}
  }
}

void vApplicationTickHook(void)
{
	HAL_IncTick();
}

uint32_t HAL_GetTick(void)
{
	/*
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
		return osKernelSysTick();
	} else {
		return ticks;
	}	
	*/
	uint32_t temp = SCB->ICSR & SCB_ICSR_VECTPENDING_Msk;
	if (temp == 0xf000UL)
	{
		SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
		uwTick++;
	}
	return uwTick;
}

void StartInitTask(void const * argument)
{
	

	SPI_Init();
	TIM_Init();
	ADC_Init();
	I2C_Init();
	USB_DEVICE_Init();
	


	osThreadDef(touchTask, StartTouchTask, osPriorityNormal, 0, 8192);
	touchTaskHandle = osThreadCreate(osThread(touchTask), NULL);

	osThreadDef(pwmTask, StartPwmTask, osPriorityNormal, 0, 8192);
	pwmTaskHandle = osThreadCreate(osThread(pwmTask), NULL);

	osThreadDef(ledTask, StartLedTask, osPriorityNormal, 0, 8192);
	ledTaskHandle = osThreadCreate(osThread(ledTask), NULL);

	osThreadDef(fsTask, StartFsTask, osPriorityNormal, 0, 16384);
	fsTaskHandle = osThreadCreate(osThread(fsTask), NULL);

	// vTaskDelete(initTaskHandle);
    while(1)
    {
		osDelay(5000);
    }
}

void StartFsTask(void const * argument)
{
	printf("Fs task started\n");
	FIL my_file;
	char* str;
	FRESULT res1 = f_open(&my_file, "soutenance.ccdb", FA_READ);
	if (res1 != FR_OK)
	{
		printf("f_open error\n");
	} else {
		// kwerky way to get clean file size
		uint32_t file_size = f_size(&my_file);
		str = malloc(file_size);

		// read the file in a buffer
		f_lseek(&my_file, 0);
		uint32_t bytesread; // TODO check bytesead agains lenght
		taskENTER_CRITICAL();
		FRESULT res = f_read(&my_file, str, file_size, (UINT*)&bytesread);
		taskEXIT_CRITICAL();
		str[file_size-1] = '\0';
		f_close(&my_file);
		if (res != FR_OK)
		{
			printf("f_read error\n");
		} else {
			//printf("%s\n", str);
			
			json_settings settings = {};
			settings.value_extra = json_builder_extra;  /* space for json-builder state */

			char error[128];
			json_value * arr = json_parse_ex(&settings, str, strlen(str), error);

			/* Now serialize it again.
			 
			char * buf = malloc(json_measure(arr));
			json_serialize(buf, arr);

			printf("%s\n", buf);
			*/

			int nb_points = arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.length;

			printf("%i\n", nb_points);

			for (int i = 0; i < nb_points; i++)
			{
				printf("[%i,%i,%i]\n",
						(int) arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.values[i]->u.array.values[0]->u.integer,
						(int) arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.values[i]->u.array.values[1]->u.integer,
						(int) arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.values[i]->u.array.values[2]->u.integer
				);
				led_set((int) arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.values[i]->u.array.values[0]->u.integer,
						(int) arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.values[i]->u.array.values[1]->u.integer,
						(int) arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.values[i]->u.array.values[2]->u.integer
				);
			}
			
		}
	}



    while(1)
    {
		osDelay(5000);
    }
}

extern uint16_t **led_buffer;
extern SPI_HandleTypeDef hspi1;

void StartLedTask(void const * argument)
{

	int i = 0;
    while(1)
    {

		if (!led_update(i))
		{
			printf("Error Trying to update\n");
		}
		i = (i+1)%9;
		//console_disp();
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
		osDelay(500);
	}

}


/*
 * Touch Screen handling
 */
extern I2C_HandleTypeDef I2cHandle;

const char *b2b(int x)
{
	static char b[9];
	b[0] = '\0';
	int z;
	for (z=128; z > 0; z>>=1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}
	return b;
}

void StartTouchTask(void const * argument)
{
	uint8_t I2C_RX_Buffer[0x1F];
	printf("Touch task started\n");
	GUI_PID_STATE State;
    while(1)
    {
		if (HAL_GPIO_ReadPin(I2Cx_IT_GPIO_PORT, I2Cx_IT_PIN) == GPIO_PIN_RESET)
		{
			if (HAL_I2C_Master_Receive(&I2cHandle, (uint16_t)I2C_ADDRESS, (uint8_t*)I2C_RX_Buffer, 0x1F, I2C_TIMEOUT) != HAL_OK)
			{
				printf("I2C Com Problem, Reseting...\n");
				I2C_Reset();
				printf("I2C Reset complete\n");
			} else {
				uint32_t i = 3;
				uint8_t ev = ((uint16_t)I2C_RX_Buffer[i] >> 6) & 0b00000011;
				/*
				if (ev == 0)
					printf("Put Down\n");
				if (ev == 1)
					printf("Put Up\n");
				*/
				GUI_PID_GetState(&State);
				uint16_t x = (((uint16_t)(I2C_RX_Buffer[i] & 0x0F)) << 8) | ((uint16_t)I2C_RX_Buffer[i+1]);
				uint16_t y = (((uint16_t)(I2C_RX_Buffer[i+2] & 0x0F)) << 8) | ((uint16_t)I2C_RX_Buffer[i+3]);
				State.x = x;
				State.y = y;
				State.Pressed = (ev == 0 || ev == 2) ? 1 : 0;
				GUI_PID_StoreState(&State);
				GUI_DrawCircle(x, y, 20);
			}
		}
		osDelay(5);
    }
}
