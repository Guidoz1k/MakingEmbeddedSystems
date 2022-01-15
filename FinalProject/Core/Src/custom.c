#include "custom.h"

void customSetup(void){

}

void customLoop(void){
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
	HAL_Delay(100);
}
