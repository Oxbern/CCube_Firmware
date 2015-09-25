#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "stm32f4xx_hal.h"

#include "GUI.h"
#include "BUTTON.h"
#include "TREEVIEW.h"
#include "WM.h"

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
#include <stdlib.h>
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

osThreadId buttonTaskHandle;
void StartButtonTask(void const * argument);


/**
 * FreeRTOS Initialisation function
 */
void FREERTOS_Init(void)
{
	osThreadDef(initTask, StartInitTask, osPriorityHigh, 0, 8192);
	initTaskHandle = osThreadCreate(osThread(initTask), NULL);

}

/*
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
	
	//if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	//{
	//	return osKernelSysTick();
	//} else {
	//	return ticks;
	//}	
	//
	uint32_t temp = SCB->ICSR & SCB_ICSR_VECTPENDING_Msk;
	if (temp == 0xf000UL)
	{
		SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;
		uwTick++;
	}
	return uwTick;
}
*/

void StartInitTask(void const * argument)
{
	osThreadDef(touchTask, StartTouchTask, osPriorityHigh, 0, 8192);
	touchTaskHandle = osThreadCreate(osThread(touchTask), NULL);

	osThreadDef(pwmTask, StartPwmTask, osPriorityNormal, 0, 8192);
	pwmTaskHandle = osThreadCreate(osThread(pwmTask), NULL);

	osThreadDef(ledTask, StartLedTask, osPriorityNormal, 0, 8192);
	ledTaskHandle = osThreadCreate(osThread(ledTask), NULL);

	osThreadDef(fsTask, StartFsTask, osPriorityNormal, 0, 16384);
	fsTaskHandle = osThreadCreate(osThread(fsTask), NULL);

	osThreadDef(buttonTask, StartButtonTask, osPriorityNormal, 0, 8192);
	buttonTaskHandle = osThreadCreate(osThread(buttonTask), NULL);

	// vTaskDelete(initTaskHandle);
    while(1)
    {
		osDelay(5000);
    }
}

void StartButtonTask(void const * argument)
{
	//BUTTON_Handle hButton0;
	//BUTTON_Handle hButton1;
	//BUTTON_Handle hButton2;
	//BUTTON_Handle hButton3;

/*
	TREEVIEW_Handle selection;
	TREEVIEW_Handle selection1;
	TREEVIEW_ITEM_Handle node1;
	TREEVIEW_ITEM_Handle node2;
	TREEVIEW_ITEM_Handle node3;
	TREEVIEW_ITEM_Handle leaf11;
	TREEVIEW_ITEM_Handle leaf12;
	TREEVIEW_ITEM_Handle leaf13;
	TREEVIEW_ITEM_Handle leaf21;
	TREEVIEW_ITEM_Handle leaf22;
	TREEVIEW_ITEM_Handle leaf23;
	TREEVIEW_ITEM_Handle leaf31;
	TREEVIEW_ITEM_Handle leaf32;
*/

	WM_MESSAGE Msg;
	WM_SetCreateFlags(WM_CF_MEMDEV);
	WM_EnableMemdev(WM_HBKWIN);

/*
	hButton0 = BUTTON_Create(700, 100, 100, 380, GUI_ID_OK, WM_CF_SHOW);
  	BUTTON_SetText(hButton0, ">");

	hButton1 = BUTTON_Create(700, 0, 100, 100, GUI_ID_OK, WM_CF_SHOW);
  	BUTTON_SetText(hButton1, ">");

	hButton2 = BUTTON_Create(0, 100, 100, 380, GUI_ID_OK, WM_CF_SHOW);
  	BUTTON_SetText(hButton2, "<");

	hButton3 = BUTTON_Create(0, 0, 100, 100, GUI_ID_OK, WM_CF_SHOW);
  	BUTTON_SetText(hButton3, "<");
*/

/*
	selection = TREEVIEW_CreateEx(	600, 0,
									200, 480,
									0, WM_CF_SHOW,
									TREEVIEW_CF_AUTOSCROLLBAR_V |
									TREEVIEW_CF_ROWSELECT,
									0
									);


	selection1 = TREEVIEW_CreateEx(	0, 0,
									0, 0,
									0, 0,
									0,
									0
									);

	TREEVIEW_ITEM_Handle tree =  TREEVIEW_InsertItem(	selection1,
														TREEVIEW_ITEM_TYPE_NODE,
														0,
														0,
														(const char *) "child_1"
														);

	TREEVIEW_ITEM_Handle child11 = TREEVIEW_InsertItem(	selection1,
														TREEVIEW_ITEM_TYPE_NODE,
														tree,
														TREEVIEW_INSERT_FIRST_CHILD,
														(const char *) "child_1_1"
														);

	TREEVIEW_ITEM_Handle child12 = TREEVIEW_InsertItem(	selection1,
														TREEVIEW_ITEM_TYPE_NODE,
														child11,
														TREEVIEW_INSERT_BELOW,
														(const char *) "child_1_2"
														);

	node1 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_NODE,
									(const char *) "node_1",
									0);
	node2 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_NODE,
									(const char *) "node_2",
									0);
	node3 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_NODE,
									(const char *) "node_3",
									0);
	leaf11 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_LEAF,
									(const char *) "leaf_1_1",
									0);
	leaf12 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_LEAF,
									(const char *) "leaf_1_2",
									0);
	leaf13 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_LEAF,
									(const char *) "leaf_1_3",
									0);
	leaf21 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_LEAF,
									(const char *) "leaf_2_1",
									0);
	leaf22 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_LEAF,
									(const char *) "leaf_2_2",
									0);
	leaf23 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_LEAF,
									(const char *) "leaf_2_3",
									0);
	leaf31 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_LEAF,
									(const char *) "leaf_3_1",
									0);
	leaf32 = TREEVIEW_ITEM_Create(	TREEVIEW_ITEM_TYPE_LEAF,
									(const char *) "leaf_3_2",
									0);

	TREEVIEW_AttachItem(	selection,
							node1,
							0,
							0);

	TREEVIEW_AttachItem(	selection,
							node2,
							node1,
							TREEVIEW_INSERT_BELOW);

	TREEVIEW_AttachItem(	selection,
							node3,
							node2,
							TREEVIEW_INSERT_BELOW);
	
	TREEVIEW_AttachItem(	selection,
							tree,
							node3,
							TREEVIEW_INSERT_BELOW);

	TREEVIEW_AttachItem(	selection,
							node3,
							node2,
							TREEVIEW_INSERT_BELOW);
	
	TREEVIEW_AttachItem(	selection,
							node3,
							node2,
							TREEVIEW_INSERT_BELOW);
	
	TREEVIEW_AttachItem(	selection,
							leaf11,
							node1,
							TREEVIEW_INSERT_FIRST_CHILD);

	TREEVIEW_AttachItem(	selection,
							leaf12,
							leaf11,
							TREEVIEW_INSERT_BELOW);

	TREEVIEW_AttachItem(	selection,
							leaf13,
							leaf12,
							TREEVIEW_INSERT_BELOW);

	TREEVIEW_AttachItem(	selection,
							leaf21,
							node2,
							TREEVIEW_INSERT_FIRST_CHILD);

	TREEVIEW_AttachItem(	selection,
							leaf13,
							leaf12,
							TREEVIEW_INSERT_BELOW);


	TREEVIEW_SetFont(selection, GUI_FONT_24_ASCII);
*/

/*
	BUTTON_Delete(hButton);
 	GUI_ClearRect(700, 0, 480, 800);
*/
    while(1)
    {
		WM_Exec();
		console_disp();
		osDelay(100);
    }
}


FRESULT scan_files (
	char* path,        /* Start node to be scanned (also used as work area) */
	TREEVIEW_Handle *hpath
)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;   /* This function assumes non-Unicode configuration */
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof lfn;


    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
		// Create directory in treeview
		TREEVIEW_ITEM_Handle node =  TREEVIEW_InsertItem(	*hpath,
															TREEVIEW_ITEM_TYPE_NODE,
															0,
															0,
															(const char *)path
														);
		int nb_child = 0;
		TREEVIEW_ITEM_Handle last_item = NULL;
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fname[0] == '.') 
			{
					continue;             /* Ignore dot entry */
			}
            fn = *fno.lfname ? fno.lfname : fno.fname;
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
				int j = strlen(fn);
				char * dirname = malloc(j);
				strcpy(dirname, fn);
				printf("%s\n", dirname);
				char newpath[i+j]; // "/0" du i renplacé par "/" plus bas
				newpath[0] = '\0';
				strcat(newpath, path);
				strcat(newpath, "/");
				strcat(newpath, fn);
				TREEVIEW_Handle child_tree = TREEVIEW_CreateEx(0,0,0,0,0,0,0,0);
                res = scan_files(newpath, &child_tree);
				printf("after scan_files %s\n", dirname);
				TREEVIEW_ITEM_Handle child_tree_first_node = TREEVIEW_GetItem(	child_tree,
																				0,
																				TREEVIEW_GET_FIRST
																				);
				if (child_tree_first_node != 0)
				{
					child_tree_first_node = TREEVIEW_ITEM_SetText(	child_tree_first_node,
																	dirname);
					printf("%s\n", newpath);
					TREEVIEW_AttachItem(	*hpath,
											child_tree_first_node,
											nb_child ? last_item : node,
											nb_child ? TREEVIEW_INSERT_BELOW : TREEVIEW_INSERT_FIRST_CHILD
										);
					last_item = child_tree_first_node;
				}
				free(dirname);
                if (res != FR_OK) break;
				nb_child++;
            } else {                                       /* It is a file. */
				last_item = TREEVIEW_InsertItem(	*hpath,
													TREEVIEW_ITEM_TYPE_LEAF,
													nb_child ? last_item : node,
													nb_child ? TREEVIEW_INSERT_BELOW : TREEVIEW_INSERT_FIRST_CHILD,
													(const char *) fn
												);
				nb_child++;
                //printf("%s/%s\n", path, fn);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

void StartFsTask(void const * argument)
{
	printf("Fs task started\n");
	taskENTER_CRITICAL();
	TREEVIEW_Handle filesystem  = TREEVIEW_CreateEx(	500, 0,
														300, 480,
														0, WM_CF_SHOW,
														TREEVIEW_CF_AUTOSCROLLBAR_V |
														TREEVIEW_CF_AUTOSCROLLBAR_H |
														TREEVIEW_CF_ROWSELECT,
														0
													);
	TREEVIEW_SetFont(filesystem, GUI_FONT_24_ASCII);
	scan_files("", &filesystem);
	TREEVIEW_ITEM_ExpandAll(TREEVIEW_GetItem(filesystem, 0, TREEVIEW_GET_FIRST));
	taskEXIT_CRITICAL();
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
		//taskENTER_CRITICAL();
		FRESULT res = f_read(&my_file, str, file_size, (UINT*)&bytesread);
		//taskEXIT_CRITICAL();
		str[file_size-1] = '\0';
		f_close(&my_file);
		if (res != FR_OK)
		{
			printf("f_read error\n");
		} else {
			//printf("%s\n", str);
			
			json_settings settings = {};
			settings.value_extra = json_builder_extra;

			char error[128];
			json_value * arr = json_parse_ex(&settings, str, strlen(str), error);

			/* Now serialize it again.
			 
			char * buf = malloc(json_measure(arr));
			json_serialize(buf, arr);

			printf("%s\n", buf);
			*/

			int nb_points = arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.length;

			//printf("%i\n", nb_points);

			for (int i = 0; i < nb_points; i++)
			{
				/*
				printf("[%i,%i,%i]\n",
						(int) arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.values[i]->u.array.values[0]->u.integer,
						(int) arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.values[i]->u.array.values[1]->u.integer,
						(int) arr->u.object.values[0].value->u.array.values[0]->u.object.values[4].value->u.array.values[i]->u.array.values[2]->u.integer
				);
				*/
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
