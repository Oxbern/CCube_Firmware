/*********************************************************************
*          Portions COPYRIGHT 2013 STMicroelectronics                *
*          Portions SEGGER Microcontroller GmbH & Co. KG             *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2013  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.22 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The  software has  been licensed  to STMicroelectronics International
N.V. a Dutch company with a Swiss branch and its headquarters in Plan-
les-Ouates, Geneva, 39 Chemin du Champ des Filles, Switzerland for the
purposes of creating libraries for ARM Cortex-M-based 32-bit microcon_
troller products commercialized by Licensee only, sublicensed and dis_
tributed under the terms and conditions of the End User License Agree_
ment supplied by STMicroelectronics International N.V.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

/**
  ******************************************************************************
  * @attention
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#include "stm32f4xx_hal.h"
#include "GUI.h"
#include "GUIDRV_Lin.h"
#include "dma2d.h"
#include "ltdc.h"

#include "LCDConf.h"
#include "GUI_Private.h"

/*********************************************************************
*
*       Layer configuration (to be modified)
*
**********************************************************************
*/
//
// Physical display size
//
#define XSIZE_PHYS 800
#define YSIZE_PHYS 480


#define BK_COLOR GUI_DARKBLUE

#undef GUI_NUM_LAYERS
#define GUI_NUM_LAYERS 2
//
// Color conversion
//
#define COLOR_CONVERSION_0 GUICC_M8888I

#if (GUI_NUM_LAYERS > 1)
	#define COLOR_CONVERSION_1 GUICC_M1555I

#endif

//
// Display driver
//
#define DISPLAY_DRIVER_0 GUIDRV_LIN_32

#if (GUI_NUM_LAYERS > 1)
	#define DISPLAY_DRIVER_1 GUIDRV_LIN_16

#endif
//
// Buffers / VScreens
//
#define NUM_BUFFERS  3 // Number of multiple buffers to be used
#define NUM_VSCREENS 1 // Number of virtual screens to be used
/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#define LCD_LAYER0_FRAME_BUFFER ((int)0xC0000000)
#define LCD_LAYER1_FRAME_BUFFER ((int)0xC0200000)


static LCD_LayerPropTypedef layerprop[GUI_NUM_LAYERS]

static const LCD_API_COLOR_CONV * apColorConvAPI[] = 
{
	COLOR_CONVERSION_0,
#if GUI_NUM_LAYERS > 1
	COLOR_CONVERSION_1,
#endif
};

//#ifndef   VRAM_ADDR
//  #define VRAM_ADDR 0xC0000000 // TBD by customer: This has to be the frame buffer start address
//#endif
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION_0
  #error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER_0
  #error No display driver defined!
#endif
#ifndef   NUM_VSCREENS
  #define NUM_VSCREENS 1
#else
  #if (NUM_VSCREENS <= 0)
    #error At least one screeen needs to be defined!
  #endif
#endif
#if (NUM_VSCREENS > 1) && (NUM_BUFFERS > 1)
  #error Virtual screens and multiple buffers are not allowed!
#endif

/*********************************************************************
*
*      Private code
*
*********************************************************************/

static void     DMA2D_CopyBuffer(U32 LayerIndex, void * pSrc, void * pDst, U32 xSize, U32 ySize, U32 OffLineSrc, U32 OffLineDst);
static void     DMA2D_FillBuffer(U32 LayerIndex, void * pDst, U32 xSize, U32 ySize, U32 OffLine, U32 ColorIndex);

static void     LCD_LL_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst);
static void     LCD_LL_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize);
static void     LCD_LL_FillRect(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex);
static void     LCD_LL_DrawBitmap32bpp(int LayerIndex, int x, int y, U8 const * p,  int xSize, int ySize, int BytesPerLine);

static inline U32 LCD_LL_GetPixelformat(U32 LayerIndex)
{
  if (LayerIndex == 0)
  {
    return LTDC_PIXEL_FORMAT_ARGB8888;
  } 
  else
  {
    return LTDC_PIXEL_FORMAT_ARGB1555;
  } 
}

static void _CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst)
{
	unsigned long AddrSrc, AddrDst;

	AddrSrc = VRAM_ADDR + BUFFER_SIZE * IndexSrc;
	AddrDst = VRAM_ADDR + BUFFER_SIZE * IndexDst;

	DMA2D_copy_buffer((uint32_t)AddrSrc, (uint32_t)AddrDst);
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*   
*/
void LCD_X_Config(void) {

  uint32_t i;

  //
  // At first initialize use of multiple buffers on demand
  //
#if (NUM_BUFFERS > 1)
	for (uint32_t i = 0; i < GUI_NUM_LAYERS; i++)
	{
		GUI_MULTIBUF_ConfigEx(i, NUM_BUFFERS);
	}
#endif
  //
  // Set display driver and color conversion for 1st layer
  //
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_0, COLOR_CONVERSION_0, 0, 0);
  //
  // Display driver configuration, required for Lin-driver
  //
  if (LCD_GetSwapXY()) {
    LCD_SetSizeEx (0, YSIZE_PHYS, XSIZE_PHYS);
    LCD_SetVSizeEx(0, YSIZE_PHYS * NUM_VSCREENS, XSIZE_PHYS);
  } else {
    LCD_SetSizeEx (0, XSIZE_PHYS, YSIZE_PHYS);
    LCD_SetVSizeEx(0, XSIZE_PHYS, YSIZE_PHYS * NUM_VSCREENS);
  }

#if (GUI_NUM_LAYERS > 1) 

    /* Set display driver and color conversion for 2nd layer */
    GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER_1, COLOR_CONVERSION_1, 0, 1);
    
    /* Set size of 2nd layer */
    if (LCD_GetSwapXYEx(1)) {
      LCD_SetSizeEx (1, YSIZE_PHYS, XSIZE_PHYS);
      LCD_SetVSizeEx(1, YSIZE_PHYS * NUM_VSCREENS, XSIZE_PHYS);
    } else {
      LCD_SetSizeEx (1, XSIZE_PHYS, YSIZE_PHYS);
      LCD_SetVSizeEx(1, XSIZE_PHYS, YSIZE_PHYS * NUM_VSCREENS);
    }
#endif

    /*Initialize GUI Layer structure */
    layer_prop[0].address = LCD_LAYER0_FRAME_BUFFER;

#if (GUI_NUM_LAYERS > 1)    
    layer_prop[1].address = LCD_LAYER1_FRAME_BUFFER; 
#endif
   /* Setting up VRam address and LCD_LL functions for CopyBuffer-, CopyRect- and FillRect operations */
  for (i = 0; i < GUI_NUM_LAYERS; i++) 
  {

    layer_prop[i].pColorConvAPI = (LCD_API_COLOR_CONV *)apColorConvAPI[i];
     
    layer_prop[i].pending_buffer = -1;

    /* Set VRAM address */
    LCD_SetVRAMAddrEx(i, (void *)(layer_prop[i].address));

    /* Remember color depth for further operations */
    layer_prop[i].BytesPerPixel = LCD_GetBitsPerPixelEx(i) >> 3;

    /* Set LCD_LL functions for several operations */
    LCD_SetDevFunc(i, LCD_DEVFUNC_COPYBUFFER, (void(*)(void))LCD_LL_CopyBuffer);
    LCD_SetDevFunc(i, LCD_DEVFUNC_COPYRECT,   (void(*)(void))LCD_LL_CopyRect);
    LCD_SetDevFunc(i, LCD_DEVFUNC_FILLRECT, (void(*)(void))LCD_LL_FillRect);

    /* Set up drawing routine for 32bpp bitmap using DMA2D */
    if (LCD_LL_GetPixelformat(i) == LTDC_PIXEL_FORMAT_ARGB8888) {
     LCD_SetDevFunc(i, LCD_DEVFUNC_DRAWBMP_32BPP, (void(*)(void))LCD_LL_DrawBitmap32bpp);     /* Set up drawing routine for 32bpp bitmap using DMA2D. Makes only sense with ARGB8888 */
    }    
  }
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if 
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;
  unsigned long Addr;

  switch (Cmd) {
  case LCD_X_INITCONTROLLER: {
    //
    // Called during the initialization process in order to set up the
    // display controller and put it into operation. If the display
    // controller is not initialized by any external routine this needs
    // to be adapted by the customer...
    //
    // ...
    return 0;
  }
  case LCD_X_SETVRAMADDR: {
    //
    // Required for setting the address of the video RAM for drivers
    // with memory mapped video RAM which is passed in the 'pVRAM' element of p
    //
    // LCD_X_SETVRAMADDR_INFO * p;
    // p = (LCD_X_SETVRAMADDR_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_SETORG: {
    //
    // Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
    //
    // LCD_X_SETORG_INFO * p;
    // p = (LCD_X_SETORG_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_SHOWBUFFER: {
    //
    // Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
    //
    LCD_X_SHOWBUFFER_INFO * p;
    p = (LCD_X_SHOWBUFFER_INFO *)pData;
    Addr = LCD_LAYER0_FRAME_BUFFER + BUFFER_SIZE * p->Index;
	HAL_LTDC_SetAddress(&hltdc, (uint32_t)Addr, 0);
	GUI_MULTIBUF_Confirm(p->Index);
    return 0;
  }
  case LCD_X_SETLUTENTRY: {
    //
    // Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
    //
    // LCD_X_SETLUTENTRY_INFO * p;
    // p = (LCD_X_SETLUTENTRY_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_ON: {
    //
    // Required if the display controller should support switching on and off
    //
    return 0;
  }
  case LCD_X_OFF: {
    //
    // Required if the display controller should support switching on and off
    //
    // ...
    return 0;
  }
  default:
    r = -1;
  }
  return r;
}

/*************************** End of file ****************************/
