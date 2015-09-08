#include "stm32f4xx_hal.h"
#include "ltdc.h"

LTDC_HandleTypeDef hltdc;

void LTDC_Init(void)
{
	LTDC_LayerCfgTypeDef pLayerCfg;
	LTDC_LayerCfgTypeDef pLayerCfg1;

	hltdc.Instance = LTDC;
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	hltdc.Init.HorizontalSync = HSYNC-1;
	hltdc.Init.VerticalSync = VSYNC-1;
	hltdc.Init.AccumulatedHBP = HSYNC + HBP - 1;
	hltdc.Init.AccumulatedVBP = VSYNC + VBP - 1; 
	hltdc.Init.AccumulatedActiveW = ACTIVE_W;
	hltdc.Init.AccumulatedActiveH = ACTIVE_H;
	hltdc.Init.TotalWidth = TOTAL_WIDTH;
	hltdc.Init.TotalHeigh = TOTAL_HEIGHT;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;
	hltdc.Init.Backcolor.Red = 0;
	HAL_LTDC_Init(&hltdc);

	pLayerCfg.WindowX0 = 0;
	pLayerCfg.WindowX1 = LCD_WIDTH;
	pLayerCfg.WindowY0 = 0;
	pLayerCfg.WindowY1 = LCD_HEIGHT;
	pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	pLayerCfg.FBStartAdress = 0xC0000000;
	pLayerCfg.ImageWidth = LCD_WIDTH;
	pLayerCfg.ImageHeight = LCD_HEIGHT;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0);

	pLayerCfg1.WindowX0 = 0;
	pLayerCfg1.WindowX1 = LCD_WIDTH;
	pLayerCfg1.WindowY0 = 0;
	pLayerCfg1.WindowY1 = LCD_HEIGHT;
	pLayerCfg1.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
	pLayerCfg1.Alpha = 0;
	pLayerCfg1.Alpha0 = 0;
	pLayerCfg1.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg1.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	pLayerCfg1.FBStartAdress = 0;
	pLayerCfg1.ImageWidth = LCD_WIDTH;
	pLayerCfg1.ImageHeight = LCD_HEIGHT;
	pLayerCfg1.Backcolor.Blue = 0;
	pLayerCfg1.Backcolor.Green = 0;
	pLayerCfg1.Backcolor.Red = 0;
	HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1);
}
