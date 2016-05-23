#ifndef CRC_H
#define CRC_H

#include <stdint.h>
#define CRC16 0x1021

uint16_t computeCRC(uint8_t *data, uint16_t size);

#endif
