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
#define APP_RX_DATA_SIZE  4
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


#define MyID 1

/* Received Buffer Index */
#define BEGINNING_INDEX 0
#define ID_INDEX (BEGINNING_INDEX + 1)
#define CMD_INDEX (ID_INDEX + 1)
#define SIZE_INDEX (CMD_INDEX + 1)
#define DATA_INDEX (SIZE_INDEX + 2)
#define CRC_INDEX 62

#define BEGINNING_DATA 0x01
#define RETRANSMIT_BUFFER 0x02
/* ACK opCode */
#define ACK_OK 0x01
#define ACK_ERR 0x02
#define ACK_NOK 0x03

/* User size */
#define ACK_SIZE 10
#define CRC_SIZE 2
#define ENCAPSULATION_SIZE 7
#define MAX_RESPONSE_SIZE 10

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

static void StoreDataUntilHandling(uint8_t *buff_RX);
static uint8_t *Set_ACK(uint8_t Ack_Type, uint8_t Current_CMD,
                        uint16_t Current_Size_Left);

static bool CMD_Require_Imm_Response(uint8_t CMD);
static uint16_t Response_Size(uint8_t CMD);
static uint8_t *Set_Imm_Response(uint8_t CMD);

void StartCDCReceptionTask(void const *argument);

void StartCDCControlTask(void const *argument);



/* Helpers */
static bool Is_CMD_Known(uint8_t CMD);
static uint16_t Get_Buffer_Size_From_CMD(uint8_t CMD);
static uint16_t Get_Data_Size_From_CMD(uint8_t CMD);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
    CDC_Init_FS,
    CDC_DeInit_FS,
    CDC_Control_FS,
    CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  SaveBufferUntilHandle
 *         Recovers data and store them into a localBuffer
 *         When full, sends data to the controller
 * @param buff_RX: Buffer received over USB
 */
static void StoreDataUntilHandling(uint8_t *buff_RX)
{
    /* Store up to 10 buffers */
    static uint8_t localBuffer[CDC_MAX_DATA_SIZE];
    static uint16_t localBuffer_Current_Index;
    static uint16_t localBuffer_Bytes_To_Be_Received;

    uint16_t buff_RX_Index = DATA_INDEX;


    if (buff_RX[BEGINNING_INDEX] == BEGINNING_DATA) {

        /* Clear local buffer */
        for (int i = 0; i < CDC_MAX_DATA_SIZE; ++i) {
            localBuffer[i] = 0;
        }

        /* Set the index of local buffer to 0 */
        localBuffer_Current_Index = 0;

        /* Current CMD is stored */
        localBuffer[localBuffer_Current_Index++] = buff_RX[CMD_INDEX];

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
        xQueueSend(controlQueue, &localBuffer[0], 0);

        /* Set the index back to 0 */
        localBuffer_Current_Index = 0;

        /* Wait for another buffer before sending a message to update task */
        localBuffer_Bytes_To_Be_Received = 1;
    }

} /* End of reception task */


/**
 * @brief  StartCDCReceptionTransmissionTask
 *         Recovers data over reception queue (sent by CDC_Receive_FS)
 *         And sends ack back over USB connection
 * @param  argument: default argument for task (NULL here)
 */
void StartCDCReceptionTask(void const *argument)
{
    uint8_t Current_CMD = 0;
    int16_t Current_Size_Left = 0;
    static uint8_t ackBuffer[ACK_SIZE];
    static uint8_t responseBuffer[MAX_RESPONSE_SIZE];


    while (1) {

        /* Local buffer to store buffer found in queue */
        uint8_t transmitBuffer[CDC_BUFFER_SIZE];

        /* Receive buffer send over ack queue */
        if (xQueueReceive(receptionQueue, &transmitBuffer[0], 0) != pdTRUE){
            /* Handle error */
        } else {

            /* Check if CMD is known */
            if (Is_CMD_Known(transmitBuffer[CMD_INDEX]) &&
                transmitBuffer[ID_INDEX] == MyID) {

                /* Buffer is the beginning of a message */
                if (transmitBuffer[BEGINNING_INDEX] == BEGINNING_DATA) {

                    /* If buffer ask to reset the reception then reset */
                    if (transmitBuffer[CMD_INDEX] == CDC_RESET_RECEPTION) {
                        Current_CMD = 0;
                        Current_Size_Left = 0;

                        /* /\* /\\* Set ACK OK *\\/ *\/ */
                        /* ackBuffer = Set_ACK(ACK_OK, Current_CMD, */
                        /*                     Current_Size_Left); */

                        /* send the ACK over USB */
                        while (CDC_Transmit_FS(&ackBuffer[0], ACK_SIZE) != USBD_OK);
                        /* Wait for another buffer */
                        continue;
                    }

                    /* Check if buffer are missing */
                    if (Current_Size_Left) {

                        /* /\* Set ACK NOK *\/ */
                        /* ackBuffer = Set_ACK(ACK_NOK, Current_CMD, */
                        /*                     Current_Size_Left); */

                        /* send the ACK over USB */
                        while (CDC_Transmit_FS(&ackBuffer[0], ACK_SIZE) != USBD_OK);
                        /* Wait for another buffer */
                        continue;
                    }

                    /* New message: Current _Size_Left == 0 */
                    Current_CMD = transmitBuffer[CMD_INDEX];
                    Current_Size_Left = (transmitBuffer[SIZE_INDEX] << 8)
                        + transmitBuffer[SIZE_INDEX + 1];

                } else if (transmitBuffer[BEGINNING_INDEX] == RETRANSMIT_BUFFER) {

                    uint16_t size_buff = (transmitBuffer[SIZE_INDEX] << 8)
                        + transmitBuffer[SIZE_INDEX + 1];

                    /* /\* Set ACK NOK *\/ */
                    /* ackBuffer = Set_ACK(ACK_NOK, Current_CMD, */
                    /*                     size_buff); */

                    /* send the ACK over USB */
                    while (CDC_Transmit_FS(&ackBuffer[0], ACK_SIZE) != USBD_OK);
                    /* Wait for another buffer */
                    continue;
                }


                /* Check if CRCs match */
                uint16_t computedCRC = computeCRC(&transmitBuffer[0],
                                                  (Get_Buffer_Size_From_CMD(
                                                      transmitBuffer[CMD_INDEX])
                                                   - CRC_SIZE)
                                                  *sizeof(uint8_t));

                uint16_t retrievedCRC = (transmitBuffer[CRC_INDEX] << 8)
                    + transmitBuffer[CRC_INDEX + 1];

                if (computedCRC != retrievedCRC) {

                    /* /\* Set ACK ERR *\/ */
                    /* ackBuffer = Set_ACK(ACK_ERR, Current_CMD, */
                    /*                     Current_Size_Left); */

                    /* send the ACK over USB */
                    while (CDC_Transmit_FS(&ackBuffer[0], ACK_SIZE) != USBD_OK);
                    /* Wait for another buffer */
                    continue;
                }
                /* CRCs match */

                /* Check if CMDs and Size_Lefts match  */
                if (Current_CMD != transmitBuffer[CMD_INDEX] ||
                    Current_Size_Left != (transmitBuffer[SIZE_INDEX] << 8)
                    + transmitBuffer[SIZE_INDEX + 1]) {

                    /* /\* /\\* Set ACK NOK *\\/ *\/ */
                    /* ackBuffer = Set_ACK(ACK_NOK, Current_CMD, */
                    /*                     Current_Size_Left); */

                    /* send the ACK over USB */
                    while (CDC_Transmit_FS(&ackBuffer[0], ACK_SIZE) != USBD_OK);
                    /* Wait for another buffer */
                    continue;
                }
                /* CMDs and Sizes match */

                if (CMD_Require_Imm_Response(transmitBuffer[CMD_INDEX])) {

                    memcpy(responseBuffer,
                           Set_Imm_Response(transmitBuffer[CMD_INDEX]),
                           MAX_RESPONSE_SIZE);

                    while (CDC_Transmit_FS(&responseBuffer[0],
                                           Response_Size(transmitBuffer[CMD_INDEX])
                                           != USBD_OK));

                } else {

                    /* Set ACK OK */
                    memcpy(&ackBuffer[0],
                           Set_ACK(ACK_OK, Current_CMD,
                                   Current_Size_Left), ACK_SIZE);

                    /* send the ACK over USB */
                    while (CDC_Transmit_FS(&ackBuffer[0], ACK_SIZE) != USBD_OK);

                    /* All good -> store data until message is complete */
                    StoreDataUntilHandling(&transmitBuffer[0]);
                }

                /* Update Current_Size_Left */
                Current_Size_Left =
                    MAX(0, (Current_Size_Left -
                           Get_Buffer_Size_From_CMD(transmitBuffer[CMD_INDEX])
                            + ENCAPSULATION_SIZE));


            } else {

                /* Simple echo */
                while (CDC_Transmit_FS(&transmitBuffer[0], 1) != USBD_OK);
            }

        } /* End of else statement (buffer received) */

    } /* End of infinite loop */

} /* End of ACK transmission task */



/**
 * @brief  StartCDCControlTask
 *         Recovers data and execute what CMD ask to do
 *
 * @param  argument: default argument for task (NULL here)
 */
void StartCDCControlTask(void const *argument)
{
    /* Infinite loop */
    while (1) {
        uint8_t localBuffer[CDC_MAX_DATA_SIZE] = {0};

        /* Receive message send over transmission queue */
        if (xQueueReceive(controlQueue, &localBuffer[0], 0) != pdTRUE){
            /* Handle error */
        } else {

            CDC_Control_FS(localBuffer[0], &localBuffer[1],
                           Get_Data_Size_From_CMD(localBuffer[0]));

        } /* End of else statement (Buffer received) */

    } /* End of infinite loop */

} /* End of display task */


/**
 * @brief  CDC_Init_FS
 *         Initializes the CDC media low layer over the FS USB IP
 * @param  None
 *
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
    bool actualBit;
    uint8_t x = 0, y = 0, z = 0;

    switch (cmd)
    {
    case CDC_DISPLAY_CUBE:

        for (int i = 0; i < length; ++i) {
            for (int j = 0; j < 8; ++j) {
                actualBit = pbuf[i] & (1 << (7 - j));
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

        /* Update all layers to print the LEDs */
        for (int l = 0; l < CUBE_WIDTH; ++l){
            led_update(l);
        }

        break;

    case CDC_FIRMWARE_UPDATE:
        /* The CCube_Firmware.bin can be found in pbuf[1..length] */
        /* TODO: Place .bin into bootloader to actually make the upgrade */
        break;

    case CDC_GET_LED_STATUS:
        break;

    case CDC_PRINT_MSG:
        break;

    case CDC_SET_LUMINOSITY:
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

    uint8_t buff_RX[CDC_BUFFER_SIZE];
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Copy data received in a local variable */
    memcpy(buff_RX, Buf, CDC_BUFFER_SIZE);

    /* Send the message to the queue */
    if (xQueueSendFromISR(receptionQueue, buff_RX,
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

/**
 * @brief  Is_CMD_Known
 *         Checks if a CMD is known
 *
 * @param  CMD: CMD to check
 * @retval Result of the operation: true if the CMD is known, false otherwise
 */
static bool Is_CMD_Known(uint8_t CMD)
{
    if (CMD == CDC_DISPLAY_CUBE
        || CMD == CDC_RESET_RECEPTION){
        return true;
    }

    return false;
}


/**
 * @brief  Get_Buffer_Size_From_CMD
 *         Gets buffer size for a specific command
 *
 * @param  CMD: specific command
 * @retval Result of the operation: buffer size if CMD is known, 0 otherwise
 */
static uint16_t Get_Buffer_Size_From_CMD(uint8_t CMD) {

    switch(CMD) {
    case CDC_DISPLAY_CUBE:
        return 64;
    case CDC_RESET_RECEPTION:
        return 7;
    default:
        return 0;
    }
}



/**
 * @brief  Get_Data_Size_From_CMD
 *         Gets data size for a specific command
 *
 * @param  CMD: specific command
 * @retval Result of the operation: data size if CMD is known, 0 otherwise
 */
static uint16_t Get_Data_Size_From_CMD(uint8_t CMD) {

    switch(CMD) {
    case CDC_DISPLAY_CUBE:
        return 92;
    default:
        return 0;
    }
}


static uint8_t *Set_ACK(uint8_t Ack_Type, uint8_t Current_CMD,
                        uint16_t Current_Size_Left)
{
    static uint8_t ackBuffer[ACK_SIZE];

    ackBuffer[0] = 1;
    ackBuffer[1] = MyID;
    ackBuffer[2] = Ack_Type;
    ackBuffer[3] = 0;
    ackBuffer[4] = 3;
    ackBuffer[5] = Current_CMD;
    ackBuffer[6] = Current_Size_Left >> 8;
    ackBuffer[7] = Current_Size_Left & 0xFF;

    uint16_t crc = computeCRC(ackBuffer, 8*sizeof(uint8_t));
    ackBuffer[8] = crc >> 8;
    ackBuffer[9] = crc & 0xFF;

    return &ackBuffer[0];
}


static bool CMD_Require_Imm_Response(uint8_t CMD)
{
    if (CMD == CDC_GET_DEVICE_ID ||
        CMD == CDC_GET_LED_STATUS ||
        CMD == CDC_GET_LUMINOSITY ||
        CMD == CDC_GET_SCREEN_SIZE ||
        CMD == CDC_GET_FIRMWARE_VERSION ||
        CMD == CDC_GET_INFO) {

        return true;
    }

    return false;
}

static uint16_t Response_Size(uint8_t CMD)
{
    switch (CMD) {
    case CDC_GET_DEVICE_ID:
        return 8;
    case CDC_GET_LED_STATUS:
        return 7;
    case CDC_GET_LUMINOSITY:
        return 8;
    case CDC_GET_SCREEN_SIZE:
        return 10;
    case CDC_GET_FIRMWARE_VERSION:
        return 8;
    case CDC_GET_INFO:
        return 10;
    }

    return 0;
}

static uint8_t *Set_Imm_Response(uint8_t CMD)
{
    static uint8_t response[MAX_RESPONSE_SIZE];

    switch (CMD) {
    case CDC_GET_DEVICE_ID:
        return response;
    case CDC_GET_LED_STATUS:
        return response;
    case CDC_GET_LUMINOSITY:
        return response;
    case CDC_GET_SCREEN_SIZE:
        return response;
    case CDC_GET_FIRMWARE_VERSION:
        return response;
    case CDC_GET_INFO:
        return response;
    }

    return NULL;
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
