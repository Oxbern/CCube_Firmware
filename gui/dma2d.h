#ifndef __DMA2D_H_
#define __DMA2D_H_

#include <stdint.h>

#define BUFFER_SIZE 0x200000

void DMA2D_Init(void);

void DMA2D_copy_buffer(uint32_t src, uint32_t dst);

#endif
