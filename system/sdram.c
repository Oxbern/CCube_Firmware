#include "stm32f4xx_hal.h"
#include "sdram.h"

SDRAM_HandleTypeDef hsdram;
FMC_SDRAM_CommandTypeDef command;
FMC_SDRAM_TimingTypeDef SDRAM_Timing;

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

static void FMC_Init(void)
{
	/* SDRAM device configuration */ 
	hsdram.Instance = FMC_SDRAM_DEVICE;

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
	hsdram.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_16;
	hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
	hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
	hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
	hsdram.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_3;
	hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_DISABLE;
	hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;

	/* Initialize the SDRAM controller */
	HAL_SDRAM_Init(&hsdram, &SDRAM_Timing);
}

void SDRAM_Init(void)
{
	FMC_Init();
	SDRAM_Initialization_Sequence(&hsdram, &command);
}
