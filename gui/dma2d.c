#include "stm32f4xx_hal.h"
#include "dma2d.h"

DMA2D_HandleTypeDef hdma2d;

void DMA2D_Init(void)
{
	hdma2d.Instance = DMA2D;
	hdma2d.Init.Mode = DMA2D_M2M;
	hdma2d.Init.ColorMode = DMA2D_ARGB8888;
	hdma2d.Init.OutputOffset = 0;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.LayerCfg[1].InputColorMode = CM_ARGB8888;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].InputAlpha = 0;
	HAL_DMA2D_Init(&hdma2d);

	HAL_DMA2D_ConfigLayer(&hdma2d, 1);
}
