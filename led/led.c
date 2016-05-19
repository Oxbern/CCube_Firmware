#include "stm32f4xx_hal.h"

#include <stdbool.h>
#include <inttypes.h>

#include "led.h"
#include "spi.h"

/**
 * buffer containing led data to be sent to sink drivers
 */
static uint16_t led_buffer[10][10] = {
	// 1th layer
    
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000000000001, // layer selection
	},
	// 2th layer
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000000000010, // layer selection
	},
	// 3th layer
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000000000100, // layer selection
	},

	// 4th layer
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000000001000, // layer selection
	},
	// 5th layer
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000000010000, // layer selection
	},
	// 6th layer
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000000100000, // layer selection
	},
	// 7th layer
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000001000000, // layer selection
	},
	// 8th layer
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000010000000, // layer selection
	},
	// 9th layer
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000100000000, // layer selection
	},
	{
		0b0000000000000000, // 9th row of columns
		0b0000000000000000, // 8th row of columns
		0b0000000000000000, // 7th row of columns
		0b0000000000000000, // 6th row of columns
		0b0000000000000000, // 5th row of columns
		0b0000000000000000, // 4th row of columns
		0b0000000000000000, // 3nd row of columns
		0b0000000000000000, // 2nd row of columns
		0b0000000000000000, // 1st row of columns
		0b0000000000000000, // layer selection
	}
};

void buffer_update(int i, int j, uint16_t newValue) {
    led_buffer[i][j] = newValue;
}


inline void led_set(uint8_t x, uint8_t y, uint8_t z)
{
	led_buffer[z][8-y] |= ( 1 << x );
}

inline void led_unset(uint8_t x, uint8_t y, uint8_t z)
{
	led_buffer[z][8-y] &= ~( 1 << x );
}


inline void led_toggle(uint8_t x, uint8_t y, uint8_t z)
{
	led_buffer[z][8-y] ^= ( 1 << x );
}


inline void led_set_state(uint8_t x, uint8_t y, uint8_t z, bool state)
{
	led_buffer[z][8-y] ^= (-state ^ led_buffer[z][8-y]) & ( 1 << x );
}


inline bool led_get(uint8_t x, uint8_t y, uint8_t z)
{
	return (bool) ( (led_buffer[z][8-y] >> x) & 1 );
}

extern SPI_HandleTypeDef hspi1;

bool led_update(int i)
{
    HAL_StatusTypeDef status;

	// populate the sink drivers
	status = HAL_SPI_Transmit(&hspi1, (uint8_t*)led_buffer[i], CUBE_WIDTH+1, SPI_TIMEOUT);
	// latch enable
	// TODO maybe have an other function do the latch enabling
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	return (status==HAL_OK);
}

void led_clear(void)
{
	for (uint8_t z = 0; z < CUBE_WIDTH; z++)
	{
		for (uint8_t y = 0; y < CUBE_WIDTH; y++)
		{
			for (uint8_t x = 0; x < CUBE_WIDTH; x++)
			{
				led_unset(x,y,z);
			}
		}
	}
}

void led_test1(void)
{
      
    for (uint8_t z = 0; z < CUBE_WIDTH; z++)
    {
	led_set(0, 0, 8-z); 
	
	led_update(8-z);
	for (int i = 0; i < 1000000; ++i) {}
    }

    led_clear();
}

void led_test2(void)
{
      
    for (uint8_t x = 0; x < CUBE_WIDTH; x++)
    {
	led_set(x, 0, 0); 

	led_update(0);
	for (int i = 0; i < 1000000; ++i) {}
    }

    led_clear();
}

void led_test3(void)
{
      
    for (uint8_t z = 0; z < CUBE_WIDTH; z++)
    {
	led_set(0, 0, z); 

	led_update(z);
	for (int i = 0; i < 1000000; ++i) {}
    }

    led_clear();
}
