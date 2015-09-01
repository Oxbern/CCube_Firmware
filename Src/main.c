/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
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
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */

#define LCD_WIDTH   800
#define LCD_HEIGHT  480
#define HFP     		40
#define HSYNC     		48
#define HBP     		88
#define VFP     		13
#define VSYNC     		3
#define VBP				32
#define ACTIVE_W 		(HSYNC + LCD_WIDTH + HBP - 1)
#define ACTIVE_H 		(VSYNC + LCD_HEIGHT + VBP - 1)
#define TOTAL_WIDTH (HSYNC + HBP + LCD_WIDTH + HFP - 1)
#define TOTAL_HEIGHT (VSYNC + VBP + LCD_HEIGHT + VFP - 1) 

#include "GUI.h"
#include "font16.h"
#include "usbd_cdc.h"
#include "fatfs.h"
#include "string.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
DMA2D_HandleTypeDef hdma2d;

LTDC_HandleTypeDef hltdc;

SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypedef SDCardInfo;
//uint32_t tab[200*100];

SDRAM_HandleTypeDef hsdram;

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;
uint16_t font_px_w = 11;
uint16_t font_px_h = 16;
uint16_t font_tb_offset = 2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA2D_Init(void);
static void MX_FMC_Init(void);
static void MX_LTDC_Init(void);
static void MX_SDIO_SD_Init(void);

/* USER CODE BEGIN PFP */

/**
 * @brief Draws a pixel in the screen framebuffer
 * @param Xpos the pixel's x position
 * @param Ypos the pixel's y position
 * @param RGB_Code the pixel's color
 */
void Put_Pixel(uint16_t Xpos, uint16_t Ypos, uint32_t ARGB_Code)
{
  *(__IO uint32_t*) (0xC0000000 + (4*(Ypos*800 + Xpos))) = ARGB_Code;
}

/**
 * @brief Renders a glyph from a given array of uint16_t
 * @param glyph pointer to start of the glyph
 * @param width width of the glyph by uint16_t
 * @param height height of the glyph by pixel
 * @param x x position where to put the glyph
 * @param y y position where to put the glyph
 * @param color the glyph's color
 */
void Render_Glyph(const uint8_t *glyph, uint16_t width, uint16_t height,
					uint16_t x, uint16_t y, uint32_t ct, uint32_t cf)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				if (glyph[i*width+j] & (0x80 >> k))
				{
					Put_Pixel(x+8*j+k, y+i, ct);
				} else {
					Put_Pixel(x+8*j+k, y+i, cf);
				}
			}
		}
	}
}

/**
 * Put a character at the given coordinates
 * @param lig the line
 * @param col the column
 * @param c the character to draw
 * @param cf the background color
 * @param ct the text color
 * @param cl the blinking flag
 */
void putc_at(uint32_t lig, uint32_t col, char c, uint32_t ct, uint32_t cf)
{
	Render_Glyph(&Font16_Table[(c-' ')*font_px_h*font_tb_offset],
					font_tb_offset, font_px_h,
					col*font_px_w, lig*font_px_h,
					ct, cf);
}

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

FMC_SDRAM_CommandTypeDef command;
FMC_SDRAM_TimingTypeDef SDRAM_Timing;

#define SDRAM_BANK_ADDR                 ((uint32_t)0xC0000000)

/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_8 */
#define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_16

/* #define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2 */
#define SDCLOCK_PERIOD                FMC_SDRAM_CLOCK_PERIOD_3

#define SDRAM_TIMEOUT     ((uint32_t)0xFFFF) 

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000) 
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200) 

#define REFRESH_COUNT       ((uint32_t)0x0569)   /* SDRAM refresh counter (90MHz SD clock) */


static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
{
  __IO uint32_t tmpmrd =0;
  /* Step 3:  Configure a clock configuration enable command */
  Command->CommandMode 			 = FMC_SDRAM_CMD_CLK_ENABLE;
  Command->CommandTarget 		 = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber 	 = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);

  /* Step 4: Insert 100 ms delay */
  HAL_Delay(100);
    
  /* Step 5: Configure a PALL (precharge all) command */ 
  Command->CommandMode 			 = FMC_SDRAM_CMD_PALL;
  Command->CommandTarget 	     = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber 	 = 1;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);  
  
  /* Step 6 : Configure a Auto-Refresh command */ 
  Command->CommandMode 			 = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command->CommandTarget 		 = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber 	 = 4;
  Command->ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);
  
  /* Step 7: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                     SDRAM_MODEREG_CAS_LATENCY_3           |
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  
  Command->CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  Command->CommandTarget 		 = FMC_SDRAM_CMD_TARGET_BANK1;
  Command->AutoRefreshNumber 	 = 1;
  Command->ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(hsdram, Command, 0x1000);
  
  /* Step 8: Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
  HAL_SDRAM_ProgramRefreshRate(hsdram, REFRESH_COUNT); 
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_LTDC_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
		
  /* USER CODE BEGIN 2 */
  
  /* 
    Before initializing the GUI the CRC mode in RCC peripheral clock
    enable registser should be enabled.
    (DM00089670.pdf page 14)
  */
  __HAL_RCC_CRC_CLK_ENABLE();
	
  SDRAM_Initialization_Sequence(&hsdram, &command);
	
  for (unsigned i=0 ; i<800*480*4 ; i+=4)
  {
    *(__IO uint32_t*)(SDRAM_BANK_ADDR+i) = 0x00000000;
  }	
	
  //HAL_LTDC_SetWindowPosition(&hltdc, 0, 0, 0);

  GUI_Init();
  GUI_SetColor(GUI_WHITE);
  GUI_SetFont(GUI_FONT_24_ASCII);
  GUI_SelectLayer(0);
  GUI_DispString("CCube version \"suck my weiner you piece of poop\" Crystallography \n");
  //GUI_DispStringHCenterAt("FUCKING FINALLY YOU PIECE OF SHIT", 400, 216);
  //GUI_DrawRoundedFrame(200,216,600,240,5,2);

	  FIL MyFile;
	if (f_open(&MyFile, "test", FA_READ) != FR_OK)
	{
		GUI_DispString("Failed to open file test\n");
	} else {
		uint8_t rtext[100];
		uint32_t bytesread;
		f_read(&MyFile, rtext, sizeof(rtext), &bytesread);
		GUI_DispString(rtext);
	}

	char *my_string = (char*) malloc(sizeof(char)*6);
	strcpy(my_string, "hello");
	GUI_DispString(my_string);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  HAL_Delay(500);

  }
  /* USER CODE END 3 */

}

/** 
 * System Clock Configuration
*/
void SystemClock_Config(void)
{
	
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /*##-1- System Clock Configuration #########################################*/  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;//5;
  RCC_OscInitStruct.PLL.PLLN = 336;//210;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;//4;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Activate the Over-Drive mode */
  //HAL_PWREx_EnableOverDrive();
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  /*##-2- LTDC Clock Configuration ###########################################*/  
  /* LCD clock configuration */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 MHz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 MHz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 MHz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDIVR_8 = 48/8 = 6 MHz */

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 240;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;//4;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

}


/* DMA2D init function */
void MX_DMA2D_Init(void)
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

/* SDIO init function */
void MX_SDIO_SD_Init(void)
{

  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 2;
  HAL_SD_Init(&hsd, &SDCardInfo);

  HAL_SD_WideBusOperation_Config(&hsd, SDIO_BUS_WIDE_4B);

}

/* LTDC init function */
void MX_LTDC_Init(void)
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
  pLayerCfg1.FBStartAdress = 0xC0177000;
  pLayerCfg1.ImageWidth = LCD_WIDTH;
  pLayerCfg1.ImageHeight = LCD_HEIGHT;
  pLayerCfg1.Backcolor.Blue = 0;
  pLayerCfg1.Backcolor.Green = 0;
  pLayerCfg1.Backcolor.Red = 0;
  HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1);

}
/* FMC initialization function */
void MX_FMC_Init(void)
{
  /* SDRAM device configuration */ 
  hsdram.Instance = FMC_SDRAM_DEVICE;
  
  /*// Timing configuration for 90 MHz of SD clock frequency (180MHz/2)
  // TMRD: 2 Clock cycles 
  SDRAM_Timing.LoadToActiveDelay    = 2;
  // TXSR: min=70ns (6x11.90ns)
  SDRAM_Timing.ExitSelfRefreshDelay = 6;
  // TRAS: min=42ns (4x11.90ns) max=120k (ns) 
  SDRAM_Timing.SelfRefreshTime      = 4;
  // TRC:  min=63 (6x11.90ns)        
  SDRAM_Timing.RowCycleDelay        = 6;
  // TWR:  2 Clock cycles 
  SDRAM_Timing.WriteRecoveryTime    = 2;
  // TRP:  15ns => 2x11.90ns 
  SDRAM_Timing.RPDelay              = 2;
  // TRCD: 15ns => 2x11.90ns 
  SDRAM_Timing.RCDDelay             = 2;*/
	
  // Timing configuration for 84 MHz of SD clock frequency (168MHz/2) => period = 11.90ns
  // TMRD: 2 Clock cycles 
  SDRAM_Timing.LoadToActiveDelay    = 2;
  // TXSR: min=63+1.5ns = 64.5 (6x11.90ns)
  SDRAM_Timing.ExitSelfRefreshDelay = 6;
  // TRAS: min=42ns (4x11.90ns) max=120k (ns) 
  SDRAM_Timing.SelfRefreshTime      = 4;
  // TRC:  min=63ns (6x11.90ns)        
  SDRAM_Timing.RowCycleDelay        = 6;
  // TWR:  2 Clock cycles 
  SDRAM_Timing.WriteRecoveryTime    = 2;
  // TRP:  21ns => 2x11.90ns 
  SDRAM_Timing.RPDelay              = 2;
  // TRCD: 21ns => 2x11.90ns 
  SDRAM_Timing.RCDDelay             = 2;

  hsdram.Init.SDBank             = FMC_SDRAM_BANK1;
  hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
  hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
  hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram.Init.SDClockPeriod      = SDCLOCK_PERIOD;
  hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_DISABLE;
  hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;

  /* Initialize the SDRAM controller */
  HAL_SDRAM_Init(&hsdram, &SDRAM_Timing);
}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOE_CLK_ENABLE();
  __GPIOI_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOH_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();

  /*Configure GPIO pin : PD5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
