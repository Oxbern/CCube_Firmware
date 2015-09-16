/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "GUI.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern SD_HandleTypeDef hsd;

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
	osSystickHandler();

	HAL_IncTick();

}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(unsigned int * hardfault_args)
{
	  unsigned int stacked_r0;
	  unsigned int stacked_r1;
	  unsigned int stacked_r2;
	  unsigned int stacked_r3;
	  unsigned int stacked_r12;
	  unsigned int stacked_lr;
	  unsigned int stacked_pc;
	  unsigned int stacked_psr;

	  stacked_r0 = ((unsigned long) hardfault_args[0]);
	  stacked_r1 = ((unsigned long) hardfault_args[1]);
	  stacked_r2 = ((unsigned long) hardfault_args[2]);
	  stacked_r3 = ((unsigned long) hardfault_args[3]);

	  stacked_r12 = ((unsigned long) hardfault_args[4]);
	  stacked_lr = ((unsigned long) hardfault_args[5]);
	  stacked_pc = ((unsigned long) hardfault_args[6]);
	  stacked_psr = ((unsigned long) hardfault_args[7]);

	  printf ("\n\n[Hard fault handler - all numbers in hex]\n");
	  printf ("R0 = %x\n", stacked_r0);
	  printf ("R1 = %x\n", stacked_r1);
	  printf ("R2 = %x\n", stacked_r2);
	  printf ("R3 = %x\n", stacked_r3);
	  printf ("R12 = %x\n", stacked_r12);
	  printf ("LR [R14] = %x  subroutine call return address\n", stacked_lr);
	  printf ("PC [R15] = %x  program counter\n", stacked_pc);
	  printf ("PSR = %x\n", stacked_psr);
	  printf ("BFAR = %lx\n", (*((volatile unsigned long *)(0xE000ED38))));
	  printf ("CFSR = %lx\n", (*((volatile unsigned long *)(0xE000ED28))));
	  printf ("HFSR = %lx\n", (*((volatile unsigned long *)(0xE000ED2C))));
	  printf ("DFSR = %lx\n", (*((volatile unsigned long *)(0xE000ED30))));
	  printf ("AFSR = %lx\n", (*((volatile unsigned long *)(0xE000ED3C))));
	  printf ("SCB_SHCSR = %lx\n", SCB->SHCSR);

	  while (1);
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/


/**
* @brief This function handles USB On The Go FS global interrupt.
*/
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

void DMA2_Stream6_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hsd.hdmatx);
}
void DMA2_Stream3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hsd.hdmarx);
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
