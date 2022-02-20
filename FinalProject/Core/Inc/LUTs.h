#ifndef __LUTS_H
#define __LUTS_H

#include "stm32h7xx_hal.h"

#define LUT_SIZE	2000

// 88 keys of a piano
uint32_t tone[88];

// sine LUT
uint16_t LUTsine[LUT_SIZE];

// square LUT
uint16_t LUTsquare[LUT_SIZE];

// sawtooth LUT
uint16_t LUTsaw[LUT_SIZE];


// sawtooth LUT
uint16_t LUTtri[LUT_SIZE];

#endif /* __LUTS_H */
