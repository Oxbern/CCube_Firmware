/**
******************************************************************************
* @file           : usbd_cdc_if.c
* @brief          :
******************************************************************************
* COPYRIGHT(c) 2015 STMicroelectronics
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
* 3. Neither the name of STMicroelectronics nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"
#include "./../../led/led.h"
#include "crc.h"
#include "cmsis_os.h"
#include "string.h"

#include "stdio.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @{
 */

/** @defgroup USBD_CDC 
 * @brief usbd core module
 * @{
 */ 

/** @defgroup USBD_CDC_Private_TypesDefinitions
 * @{
 */ 
/* USER CODE BEGIN 0 */ 
/* USER CODE END 0 */ 
/**
 * @}
 */ 

/** @defgroup USBD_CDC_Private_Defines
 * @{
 */ 
/* USER CODE BEGIN 1 */
/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE  512
#define APP_TX_DATA_SIZE  4
/* USER CODE END 1 */
/**
 * @}
 */ 

/** @defgroup USBD_CDC_Private_Macros
 * @{
 */ 
/* USER CODE BEGIN 2 */ 
/* USER CODE END 2 */
/**
 * @}
 */ 
  
/** @defgroup USBD_CDC_Private_Variables
 * @{
 */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/* Received Data over USB are stored in this buffer       */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/* Send Data over USB CDC are stored in this buffer       */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

uint16_t UserRxBufferFS_Current_Index;


/* Received Buffer Index */
#define BEGINNING_INDEX 0 
#define ID_INDEX (BEGINNING_INDEX + 1)
#define CMD_INDEX (ID_INDEX + 1)
#define SIZE_INDEX (CMD_INDEX + 1)
#define DATA_INDEX (SIZE_INDEX + 2)
#define CRC_INDEX 62

#define BEGINNING_DATA 0x01 

/* ACK opCode */
#define ACK_OK 0x20
#define ACK_ERR 0x21
#define ACK_NOK 0x22

/* Macro to define ACK (OK, ERR or NOK) */
#define ACK(IdDevice, AckType, CmdBuff, SizeBuff)    \
	{1, (IdDevice), (AckType), 0, 3, CmdBuff,                  \
			(SizeBuff >> 8), (SizeBuff & 0xFF)}

/* USB handler declaration */
/* Handle for USB Full Speed IP */
USBD_HandleTypeDef  *hUsbDevice_0;

extern USBD_HandleTypeDef hUsbDeviceFS;

#define DEBUG 0

/**
 * @}
 */ 
  
/** @defgroup USBD_CDC_Private_FunctionPrototypes
 * @{
 */
static int8_t CDC_Init_FS     (void);
static int8_t CDC_DeInit_FS   (void);
static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS  (uint8_t* pbuf, uint32_t *Len);
void StartCDCReceptionTask(void const *argument);
void StartCDCAckTransmissionTask(void const *argument);
void StartCDCDisplayTask(void const *argument);



/* Helpers */
static bool Is_CMD_Known(uint8_t CMD);


USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = 
{
	CDC_Init_FS,
	CDC_DeInit_FS,
	CDC_Control_FS,  
	CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  StartCDCReceptionTask
 *         Recovers data and store them into a localBuffer
 *         When full, sends data to the controller
 * @param argument: Default argument for task (NULL here)
 */

void StartCDCReceptionTask(void const *argument) {
	
	uint8_t localBuffer[512];
	uint16_t localBuffer_Current_Index;
	uint16_t localBuffer_Bytes_To_Be_Received;

	/* uint8_t Current_CMD; */
	
	while (1) {

		uint8_t buff_RX[512] = {0};
		uint16_t buff_RX_Index = DATA_INDEX;
		
		/* Receive message send over reception queue */
		if (xQueueReceive(receptionQueue, &buff_RX[0], 10) != pdTRUE){
			/* Handle error */
		} else {

 			/* Handle formatted buffer only */
			if (Is_CMD_Known(buff_RX[CMD_INDEX])) {

				if (buff_RX[BEGINNING_INDEX] == BEGINNING_DATA) {

					/* Clear local buffer */
					for (int i = 0; i < 512; ++i) {
						localBuffer[i] = 0;
					}

					/* Set the index of local buffer to 0 */
					localBuffer_Current_Index = 0;

					/* Retrieve number of bytes to be received */
					localBuffer_Bytes_To_Be_Received = buff_RX[SIZE_INDEX + 1]
						+ (buff_RX[SIZE_INDEX] << 8);
				}
				
				
				while (buff_RX_Index < CRC_INDEX
				       && localBuffer_Bytes_To_Be_Received > 0) {
					/* Copy value in local buffer */
					localBuffer[localBuffer_Current_Index++] = buff_RX[buff_RX_Index++];

					/* Update control variable */
					localBuffer_Bytes_To_Be_Received--;
				}

				if (localBuffer_Bytes_To_Be_Received == 0
				    && localBuffer_Current_Index > 0) {

					/* Send the data into the transmission queue */
					xQueueSend(displayQueue, &localBuffer[0], 10);

					/* Set the index back to 0 */
					localBuffer_Current_Index = 0;
					
					/* Wait for another buffer before sending a message to update task */
					localBuffer_Bytes_To_Be_Received = 1;
				} 
				
			} /* End of if statement (Is_CMD_Known) */
			
		} /* End of else statement (buffer was received) */
		
	} /* End of infinite loop */
	
} /* End of reception task */

/**
 * @brief  StartCDCAckTransmissionTask
 *         Recovers data over ack queue (sent by CDC_Receive_FS)
 *         And sends ack back over USB connection
 * @param  argument: default argument for task (NULL here)
 */
void StartCDCAckTransmissionTask(void const *argument) {

	while (1) {
		uint8_t transmitBuffer[512];

		uint8_t Current_CMD = 0;
		uint16_t Current_Size_Left = 0;
		
		/* Receive message send over ack queue */
		if (xQueueReceive(ackQueue, &transmitBuffer[0], 10) != pdTRUE){
			/* Handle error */
		} else {			

			if (Is_CMD_Known(transmitBuffer[CMD_INDEX])) {

				if (transmitBuffer[BEGINNING_INDEX] == BEGINNING_DATA) {
					if (Current_Size_Left) {
						uint8_t ackBuffer[ACK_SIZE] = ACK(1, ACK_NOK,
						                                  Current_CMD, Current_Size_Left);

						/* send the ACK over USB */
						USBD_CDC_SetTxBuffer(hUsbDevice_0, &ackBuffer[0], ACK_SIZE);
						USBD_CDC_TransmitPacket(hUsbDevice_0);
						/* Wait for another buffer */
						continue;
					
					} else {
						Current_CMD = transmitBuffer[CMD_INDEX];
						Current_Size_Left = (transmitBuffer[SIZE_INDEX] << 8)
							+ transmitBuffer[SIZE_INDEX];
					}
				}
			
				if (xQueueSend(receptionQueue, &transmitBuffer[0], 10) != pdTRUE) {
					/* Handle error */
				}

				uint8_t ackBuffer[ACK_SIZE] = {0};

				/* send the ACK over USB */
				USBD_CDC_SetTxBuffer(hUsbDevice_0, &ackBuffer[0], ACK_SIZE);
				USBD_CDC_TransmitPacket(hUsbDevice_0);

			} else {
				/* Simple echo */
				USBD_CDC_SetTxBuffer(hUsbDevice_0, &transmitBuffer[0], 1);
				USBD_CDC_TransmitPacket(hUsbDevice_0);
			}
			
		} /* End of else statement (buffer received) */

	} /* End of infinite loop */
	
} /* End of ACK transmission task */

/**
 * @brief  StartCDCDisplayTask
 *         Converts data received over USB in shapes on the cube
 * @param  argument: default argument for task (NULL here)
 */
void StartCDCDisplayTask(void const *argument) {
	
	while (1) {
		uint8_t localBuffer[512] = {0};
	
		/* Receive message send over transmission queue */
		if (xQueueReceive(displayQueue, &localBuffer[0], 10) != pdTRUE){
			/* Handle error */
		} else {

			bool actualBit;
			uint8_t x = 0, y = 0, z = 0;
			
			/* Local buffer has 92B of data (usefull)*/
			for (int i = 0; i < 92; ++i) {
				for (int j = 0; j < 8; ++j) {
					actualBit = localBuffer[i] & (1 << (7 - j));
					led_set_state(x, y, z, actualBit);

					z = (z + 1) % 9;
					if (z == 0) {
						y = (y + 1) % 9;
						if (y == 0) {
							x = (x + 1) % 9;
							if (x == 0) {
								i = 92;
								j = 8;
							}
						}
					}
				}
			}
			
			for (int l = 0; l < CUBE_WIDTH; ++l){
				led_update(l);
			}
		}

	}	/* End of infinite loop */

}		/* End of display task */


/**
 * @brief  CDC_Init_FS
 *         Initializes the CDC media low layer over the FS USB IP
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Init_FS(void)
{
	hUsbDevice_0 = &hUsbDeviceFS;
	/* USER CODE BEGIN 3 */ 
	/* Set Application Buffers */
	USBD_CDC_SetTxBuffer(hUsbDevice_0, UserTxBufferFS, 0);
	USBD_CDC_SetRxBuffer(hUsbDevice_0, UserRxBufferFS);
	return (USBD_OK);
	/* USER CODE END 3 */ 
}

/**
 * @brief  CDC_DeInit_FS
 *         DeInitializes the CDC media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_DeInit_FS(void)
{
	/* USER CODE BEGIN 4 */ 
	return (USBD_OK);
	/* USER CODE END 4 */ 
}

/**
 * @brief  CDC_Control_FS
 *         Manage the CDC class requests
 * @param  cmd: Command code            
 * @param  pbuf: Buffer containing command data (request parameters)
 * @param  length: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
	/* USER CODE BEGIN 5 */
	int k = 0;
	switch (cmd)
	{
	case CDC_DISPLAY_CUBE:
		for (int i = 0; i < CUBE_WIDTH+1; ++i) {
			for (int j = 0; j < CUBE_WIDTH+1; ++j) {
				buffer_update(i, j, (pbuf[k] << 8) + pbuf[k+1]);
				k += 2;
			}
		}
	
		for (int l = 0; l < CUBE_WIDTH; ++l){
			led_update(l);
		}
		break;
    
	default:
		break;
	}

	return (USBD_OK);
	/* USER CODE END 5 */
}		



/**
 * @brief  CDC_Receive_FS
 *         Data received over USB OUT endpoint are sent over CDC interface 
 *         through this function.
 *           
 *         @note
 *         This function will block any OUT packet reception on USB endpoint 
 *         untill exiting this function. If you exit this function before transfer
 *         is complete on CDC interface (ie. using DMA controller) it will result 
 *         in receiving more data while previous ones are still not sent.
 *                 
 * @param  Buf: Buffer of data to be received
 * @param  Len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CDC_Receive_FS (uint8_t* Buf, uint32_t *Len)
{
	/* USER CODE BEGIN 6 */
	uint8_t result = USBD_OK;
	uint8_t buff_RX[512];
	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	memcpy(&buff_RX, Buf, 512);
		
	/* Send the message to the queue */
	if (xQueueSendFromISR(ackQueue, &buff_RX,
	                      &xHigherPriorityTaskWoken) != pdTRUE) {
		/* Handle error */
	}
	
	/* Wait for another buffer to be received */
	USBD_CDC_ReceivePacket(hUsbDevice_0);
	
	return (result);
	/* USER CODE END 6 */ 
}


/**
 * @brief  CDC_Transmit_FS
 *         Data send over USB IN endpoint are sent over CDC interface 
 *         through this function.           
 *         @note
 *         
 *                 
 * @param  Buf: Buffer of data to be send
 * @param  Len: Number of data to be send (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
 */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
	uint8_t result = USBD_OK;
	/* USER CODE BEGIN 7 */ 
	USBD_CDC_SetTxBuffer(hUsbDevice_0, Buf, Len);   
	result = USBD_CDC_TransmitPacket(hUsbDevice_0);
	/* USER CODE END 7 */ 
	return result;
}


/* Helper */
static bool Is_CMD_Known(uint8_t CMD) {
	if (CMD == CDC_DISPLAY_CUBE){
		return true;
	}

	return false;
}


/**
 * @}
 */ 

/**
 * @}
 */ 

/**
 * @}
 */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
