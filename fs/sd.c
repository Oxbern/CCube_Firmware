#include "stm32f4xx_hal.h"
#include "sd.h"

#include "fatfs.h"

SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypedef SDCardInfo;

/* SDIO init function */
static void SDIO_SD_Init(void)
{
	hsd.Instance = SDIO;
	hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
	hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
	hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
	hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
	hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
	hsd.Init.ClockDiv = 4;
	HAL_SD_Init(&hsd, &SDCardInfo);

	HAL_SD_WideBusOperation_Config(&hsd, SDIO_BUS_WIDE_4B);
}

void SD_Init(void)
{
  SDIO_SD_Init();
  FATFS_Init();
}
