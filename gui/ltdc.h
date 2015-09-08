#ifndef __LTDC_H_
#define __LTDC_H_

#define LCD_WIDTH   	800
#define LCD_HEIGHT  	480
#define HFP     		40
#define HSYNC     		48
#define HBP     		88
#define VFP     		13
#define VSYNC     		3
#define VBP				32
#define ACTIVE_W 		(HSYNC + LCD_WIDTH + HBP - 1)
#define ACTIVE_H 		(VSYNC + LCD_HEIGHT + VBP - 1)
#define TOTAL_WIDTH  	(HSYNC + HBP + LCD_WIDTH + HFP - 1)
#define TOTAL_HEIGHT 	(VSYNC + VBP + LCD_HEIGHT + VFP - 1)

void LTDC_Init(void);

#endif
