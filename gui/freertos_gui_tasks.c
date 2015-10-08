#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "i2c.h"

#include "GUI.h"
#include "BUTTON.h"
#include "TREEVIEW.h"
#include "DIALOG.h"
#include "MULTIPAGE.h"
#include "WM.h"

#include <string.h>


void StartGuiTask(void const * argument)
{
	//GUI_CURSOR_Select(&GUI_CursorArrowM);
	//GUI_CURSOR_Show();  

	//WM_SetCreateFlags(WM_CF_MEMDEV);      // Use memory devices on all windows to avoid flicker

    while(1)
    {
		GUI_Delay(200);
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
	//static int prev_state = -1;
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
				//GUI_PID_GetState(&State);
				uint16_t x = (((uint16_t)(I2C_RX_Buffer[i] & 0x0F)) << 8) | ((uint16_t)I2C_RX_Buffer[i+1]);
				uint16_t y = (((uint16_t)(I2C_RX_Buffer[i+2] & 0x0F)) << 8) | ((uint16_t)I2C_RX_Buffer[i+3]);
				State.x = x;
				State.y = y;
				State.Pressed = (ev == 0 || ev == 2) ? 1 : 0;
				State.Layer = 0;
				//if (prev_state != State.Pressed)
				//{
					//prev_state = State.Pressed;
					GUI_TOUCH_StoreStateEx(&State);
					//printf("state : %i\n", ev);
					//GUI_DrawCircle(x, y, 20);
				//}
			}
		}
		osDelay(10);
    }
}
