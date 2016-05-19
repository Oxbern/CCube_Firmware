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
#include <stdio.h>

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
#define APP_RX_DATA_SIZE  30
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

uint8_t UserRxBufferFS_Current_Index;
uint8_t UserRxBufferFS_Expected_Size;

#define BEGINNING_DATA 0x01 

/* USB handler declaration */
/* Handle for USB Full Speed IP */
USBD_HandleTypeDef  *hUsbDevice_0;

extern USBD_HandleTypeDef hUsbDeviceFS;

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

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = 
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,  
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
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
	
	for (int i = 0; i < CUBE_WIDTH; ++i) {
	    for (int j = 0; j < CUBE_WIDTH; ++j) {
		buffer_update(i, j, (pbuf[k] << 8) + pbuf[k+1]);
		k += 2;
	    }
	}


	for (int l = 0; l < CUBE_WIDTH; ++l)
	    led_update(l);
	
	break;
    
    default:
	break;
    }

    return (USBD_OK);
    /* USER CODE END 5 */
}


/* Helper */
void Empty_UserRxBufferFS() {
    for (int i = 0; i < APP_RX_DATA_SIZE; ++i) {
	UserRxBufferFS[i] = 0;
    }
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
    uint8_t Current_CMD = 0;
    static uint8_t buff_RX[512];
    static uint8_t buff_TX[512];
        
    for (int i = 0; i < *Len; i++)
    {
    	buff_TX[i] = buff_RX[i];
    }

    /* if (buff_TX[0]=='\r' || buff_TX[0]=='\n') */
    /* { */
    /* 	printf("\n>>> "); */
    /* } else { */
    /* 	buff_TX[*Len] = '\0'; */
    /* 	printf("%s",(const char *)buff_TX); */
    /* } */

    if (buff_TX[0] == BEGINNING_DATA) {
    	Empty_UserRxBufferFS();
    	UserRxBufferFS_Current_Index = 0;
    	UserRxBufferFS_Expected_Size = buff_TX[3] + (buff_TX[2] << 8);
    	Current_CMD = buff_TX[1];
    	led_test1();
    } else {
    	led_test3();
    }

    /* for (int i = 4; i < 62; ++i) { */
    /* 	UserRxBufferFS[UserRxBufferFS_Current_Index] = buff_TX[i]; */
    /* 	UserRxBufferFS_Current_Index++; */
    /* } */

    /* if (UserRxBufferFS_Current_Index == UserRxBufferFS_Expected_Size) { */
    /* 	led_test1(); */
    /* 	CDC_Control_FS(Current_CMD, UserRxBufferFS, UserRxBufferFS_Current_Index*sizeof(uint8_t)); */
    /* } */

    USBD_CDC_SetTxBuffer(hUsbDevice_0, &buff_TX[0], *Len);
    USBD_CDC_TransmitPacket(hUsbDevice_0);
	
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

