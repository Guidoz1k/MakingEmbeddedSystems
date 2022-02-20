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

char menu[8][16];

char octave_menu[6][6];

char wave_menu[4][9];

enum _buttons{
		invalid = 0,
		up = 1,
		down = 2,
		right = 3,
		left = 4
};

enum _waveforms{
		sine = 0,
		square = 1,
		triangle = 2,
		sawtooth = 3
};

enum _octave{
	range12 = 0,
	range23 = 1,
	range34 = 2,
	range45 = 3,
	range56 = 4,
	range67 = 5
};

#endif /* __LUTS_H */
