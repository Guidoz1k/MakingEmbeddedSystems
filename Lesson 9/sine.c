#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <math.h>

#define LUTSIZE		2000
#define LUTRANGE	65535

float LUTf[LUTSIZE];
uint32_t LUTi[LUTSIZE];

float disc_sine(uint32_t range){
	float value = 0;

	value = (((range / (999.5)) * M_PI) - M_PI);
	value = sin(value);

	return value;
}

void init_lut(void){
	uint32_t counter = 0;

	for(counter = 0; counter < LUTSIZE; counter++){
		LUTf[counter] = disc_sine(counter);
		LUTi[counter] = (uint32_t)(LUTRANGE*(disc_sine(counter) + 1));
	}
}

float sine_taylor(float x){
	float y = x;

	y += - (x*x*x) / (3*2);
	y += + (x*x*x*x*x) / (5*4*3*2);
	y += - (x*x*x*x*x*x*x) / (7*6*5*4*3*2);
	y += + (x*x*x*x*x*x*x*x*x) / (9*8*7*6*5*4*3*2);
	y += - (x*x*x*x*x*x*x*x*x*x*x) / (11*10*9*8*7*6*5*4*3*2);

	return y;
}


int main(){
	init_lut();

	printf("sine taylor at pi/2: \t\t%f \n", sine_taylor(M_PI/2));
	printf("float LUT at pi/2 : \t\t%f \n", LUTf[1500]);
	printf("int LUT converted at pi/2: \t%f", (float)LUTi[1500]/LUTRANGE - 1);
}