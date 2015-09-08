#include "stm32f4xx_hal.h"
#include "crc.h"

CRC_HandleTypeDef hcrc;

void CRC_Init(void)
{
	hcrc.Instance = CRC;
	HAL_CRC_Init(&hcrc);
}

