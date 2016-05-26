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
#define CMD_INDEX 1
#define SIZE_INDEX 2
#define DATA_INDEX 4
#define CRC_INDEX 62

#define BEGINNING_DATA 0x01 

/* Size of ACK buffers */
#define ACK_SIZE 6
static uint8_t ACK[ACK_SIZE] = {0};

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
void StartCDCTransmissionTask(void const *argument);

/* Helpers */
static void Empty_UserRxBufferFS();
static bool Is_CMD_Known(uint8_t CMD);
static void Set_ACKSend_OK(uint8_t CMD, uint16_t size_buff, uint16_t crc);
static void Set_ACKSend_ERR(uint8_t CMD, uint16_t size_buff, uint16_t crc);
static void Set_ACKSend_NOK(uint8_t CMD, uint16_t size_buff, uint16_t crc);


USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = 
{
	CDC_Init_FS,
	CDC_DeInit_FS,
	CDC_Control_FS,  
	CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
void StartCDCReceptionTask(void const *argument) {
	uint8_t buff_RX[512];
	static uint8_t buff_TX[512];
	
	while (1) {
		/* Receive message send over reception queue */
		if (xQueueReceive(receptionQueue, &buff_RX[0], 10) != pdTRUE){
			/* Handle error */
		} else {

			for (int i = 0; i < 512; ++i) {
				buff_TX[i] = buff_RX[i];
			}
			/* buff_TX[0] = 'a'; */
			
			/* Send a message to the transmission queue */
			if (xQueueSend(transmissionQueue, &buff_TX[0], 10) != pdTRUE) {
				/* Handle error */
			}
		}
		osDelay(1);
	}
}

void StartCDCTransmissionTask(void const *argument) {
	uint16_t Len = 1;
	static uint8_t buff_TX[512];
	
	while (1) {
		/* Receive message send over transmission queue */
		if (xQueueReceive(transmissionQueue, &buff_TX[0], 10) != pdTRUE){
			/* Handle error */
		} else {
			/* Send the buffer over USB */
			USBD_CDC_SetTxBuffer(hUsbDevice_0, &buff_TX[0], Len);
			USBD_CDC_TransmitPacket(hUsbDevice_0);
		}

		osDelay(1);
	}
}


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


static uint8_t *CDC_Set_ACK(uint8_t *buff_RX) {
	static uint8_t Current_CMD = 0;
	static int16_t UserRxBufferFS_Expected_Size;
    
	uint16_t buff_RX_Index = DATA_INDEX;
	uint16_t size_left_buff = (buff_RX[SIZE_INDEX + 1]
	                           + (buff_RX[SIZE_INDEX] << 8));

	/* Checks if a buffer was lost */
	if (buff_RX[BEGINNING_INDEX] != BEGINNING_DATA) {
		if (size_left_buff != UserRxBufferFS_Expected_Size) {
			Set_ACKSend_NOK(Current_CMD,
			                UserRxBufferFS_Expected_Size, 0);
			return ACK;
		} else if (Current_CMD != buff_RX[CMD_INDEX]) {
			Set_ACKSend_NOK(Current_CMD,
			                UserRxBufferFS_Expected_Size, 0);
			return ACK;
		}
	}
    
	if (buff_RX[BEGINNING_INDEX] == BEGINNING_DATA) {
		if (UserRxBufferFS_Expected_Size > 0) {
			Set_ACKSend_NOK(Current_CMD,
			                size_left_buff, 0);
			return ACK;
		} else {
			Current_CMD = buff_RX[CMD_INDEX];
			Empty_UserRxBufferFS();
			UserRxBufferFS_Expected_Size = buff_RX[SIZE_INDEX + 1] + (buff_RX[SIZE_INDEX] << 8);
		}
	}
    
	while (buff_RX_Index < CRC_INDEX) {
		UserRxBufferFS[UserRxBufferFS_Current_Index++] = buff_RX[buff_RX_Index++];
		--UserRxBufferFS_Expected_Size;
	}
    
	Set_ACKSend_OK(Current_CMD, (buff_RX[SIZE_INDEX] << 8) + buff_RX[SIZE_INDEX], 0);

	if (UserRxBufferFS_Current_Index >= 198) {
		CDC_Control_FS(CDC_DISPLAY_CUBE, UserRxBufferFS, 200*sizeof(uint8_t));
	}

	return ACK;
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
	static uint8_t buff_RX[512];
	
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	/* Send the message to the queue */
	if (xQueueSendFromISR(receptionQueue, &buff_RX[0],
	                      &xHigherPriorityTaskWoken) != pdTRUE) {
		/* Handle error */
	}
	
	/* Switch on a led on top layer */
	led_test2();
	
	/* Wait for another buffer to be received */
	USBD_CDC_SetRxBuffer(hUsbDevice_0, &buff_RX[0]);
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
static void Empty_UserRxBufferFS() {
	for (int i = 0; i < APP_RX_DATA_SIZE; ++i) {
		UserRxBufferFS[i] = 0;
	}
	UserRxBufferFS_Current_Index = 0;
}

static bool Is_CMD_Known(uint8_t CMD) {
	if (CMD == CDC_DISPLAY_CUBE){
		return true;
	}

	return false;
}

static void Set_ACKSend_OK(uint8_t CMD, uint16_t size_buff, uint16_t crc) {
    
	ACK[0] = CDC_SEND_ACK_OK;
	ACK[1] = CMD;
	ACK[2] = size_buff >> 8;
	ACK[3] = size_buff & 0xFF;
	ACK[4] = crc >> 8;
	ACK[5] = crc & 0xFF;

}

static void Set_ACKSend_ERR(uint8_t CMD, uint16_t size_buff, uint16_t crc) {

	ACK[0] = CDC_SEND_ACK_ERR;
	ACK[1] = CMD;
	ACK[2] = size_buff >> 8;
	ACK[3] = size_buff & 0xFF;
	ACK[4] = crc >> 8;
	ACK[5] = crc & 0xFF;
    
}

static void Set_ACKSend_NOK(uint8_t CMD, uint16_t size_buff, uint16_t crc) {

	ACK[0] = CDC_SEND_ACK_NOK;
	ACK[1] = CMD;
	ACK[2] = size_buff >> 8;
	ACK[3] = size_buff & 0xFF;
	ACK[4] = crc >> 8;
	ACK[5] = crc & 0xFF;

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
