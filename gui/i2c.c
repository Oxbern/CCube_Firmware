#include "stm32f4xx_hal.h"
#include "i2c.h"

I2C_HandleTypeDef I2cHandle;

void I2C_Init(void)
{
  I2Cx_CLK_ENABLE();
  I2cHandle.Instance             = I2Cx;
  
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.ClockSpeed      = 400000;
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE_16_9;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
  I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
  I2cHandle.Init.OwnAddress2     = 0xFE;
  
  HAL_I2C_Init(&I2cHandle);

  HAL_GPIO_WritePin(I2Cx_WAKEUP_GPIO_PORT, I2Cx_WAKEUP_PIN, GPIO_PIN_RESET);
  HAL_Delay(5);
  HAL_GPIO_WritePin(I2Cx_WAKEUP_GPIO_PORT, I2Cx_WAKEUP_PIN, GPIO_PIN_SET);
  HAL_Delay(400);
}

void I2C_Reset(void)
{
  HAL_GPIO_WritePin(I2Cx_WAKEUP_GPIO_PORT, I2Cx_WAKEUP_PIN, GPIO_PIN_RESET);

  HAL_I2C_DeInit(&I2cHandle);

  HAL_I2C_Init(&I2cHandle);

  HAL_Delay(5);
  HAL_GPIO_WritePin(I2Cx_WAKEUP_GPIO_PORT, I2Cx_WAKEUP_PIN, GPIO_PIN_SET);
  HAL_Delay(400);
}
