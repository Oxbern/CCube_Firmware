#ifndef __I2C_H_
#define __I2C_H_

#define I2Cx                             I2C1
#define I2Cx_CLK_ENABLE()                __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()               __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()             __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SCL_AF                     GPIO_AF4_I2C1
#define I2Cx_SDA_PIN                    GPIO_PIN_7
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_AF                     GPIO_AF4_I2C1
#define I2Cx_IT_PIN						GPIO_PIN_5
#define I2Cx_IT_GPIO_PORT				GPIOB
#define I2Cx_WAKEUP_PIN					GPIO_PIN_4
#define I2Cx_WAKEUP_GPIO_PORT			GPIOB

#define I2C_ADDRESS 0x70

#define I2C_TIMEOUT 500

void I2C_Init(void);

void I2C_Reset(void);

#endif
