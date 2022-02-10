#ifndef __CUSTOM_H
#define __CUSTOM_H

#include "stm32h7xx_hal.h"

void customLoop(void);

void customSetup(ADC_HandleTypeDef handler1, I2S_HandleTypeDef handler2);

#endif /* __CUSTOM_H */
