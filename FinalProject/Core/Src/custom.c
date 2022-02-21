#include "custom.h"
#include "LUTs.h"

#define	ADC_SP	GPIOA,GPIO_PIN_6	// PA06 = ADC1_INP16 for the soft-pot
#define	LED		GPIOA,GPIO_PIN_1	// PA01 = board built-in LED

#define	I2S_WS	GPIOA,GPIO_PIN_4	// PA04 = I2S1_WS	(LCK)
#define	I2S_CK	GPIOA,GPIO_PIN_5	// PA05 = I2S1_CK	(BCK)
#define	I2S_SDO	GPIOA,GPIO_PIN_7	// PA06 = I2S1_SDO	(DIN)

#define	LCD_D0	GPIOD,GPIO_PIN_0	// PD00 = LCD data pin 0
#define	LCD_D1	GPIOD,GPIO_PIN_1	// PD01 = LCD data pin 1
#define	LCD_D2	GPIOD,GPIO_PIN_2	// PD02 = LCD data pin 2
#define	LCD_D3	GPIOD,GPIO_PIN_3	// PD03 = LCD data pin 3
#define	LCD_D4	GPIOD,GPIO_PIN_4	// PD04 = LCD data pin 4
#define	LCD_D5	GPIOD,GPIO_PIN_5	// PD05 = LCD data pin 5
#define	LCD_D6	GPIOD,GPIO_PIN_6	// PD06 = LCD data pin 6
#define	LCD_D7	GPIOD,GPIO_PIN_7	// PD07 = LCD data pin 7

#define	LCD_RS	GPIOE,GPIO_PIN_0	// PE00 = LCD RS pin
#define	LCD_EN	GPIOE,GPIO_PIN_1	// PE01 = LCD E pin

#define	BUT_UP	GPIOE,GPIO_PIN_2	// PE02 = UP button
#define	BUT_DWN	GPIOE,GPIO_PIN_3	// PE03 = DOWN button
#define	BUT_RGT	GPIOE,GPIO_PIN_4	// PE04 = RIGHT button
#define	BUT_LFT	GPIOE,GPIO_PIN_5	// PE05 = LEFT button

#define BUT_K2	GPIOC,GPIO_PIN_5	// PC05 = K2 onboard button
#define INT_OUT	GPIOB,GPIO_PIN_5	// PC05 = K2 onboard button

#define MAX_HARM 3					// maximum number of harmonics plus the fundamental

static ADC_HandleTypeDef hadc1;
static I2S_HandleTypeDef hi2s1;

// DDS parameters
static struct _dds{
	uint16_t		vol1;
	uint16_t		vol2;
	enum _waveforms	shape1;
	enum _waveforms	shape2;
	uint16_t		harmonics1;
	uint16_t		harmonics2;
	enum _octave	octave1;
	enum _octave	octave2;
} sound_config;
static uint32_t increment1[MAX_HARM + 1];
static uint32_t increment2[MAX_HARM + 1];
static uint32_t counter1[MAX_HARM + 1];
static uint32_t counter2[MAX_HARM + 1];
static uint32_t bigmax = LUT_SIZE << 10;

// UI variables
static uint8_t menu_option = 0;
static uint8_t max[8] = {
		16, 16, // max volume 0 - 16
		 3,  3, // max shape 0 - 3
		 MAX_HARM,  MAX_HARM,  // max harmonics
		 5,  5	// max octaves
};
static uint16_t adc_old = 0;
static uint16_t needs_update = 0;

//////////////////////////////////////////////////////////////////////// LCD "ll" functions

static void lcd_e(uint8_t status){
    if(status)
        HAL_GPIO_WritePin(LCD_EN, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(LCD_EN, GPIO_PIN_RESET);
}

static void lcd_rs(uint8_t status){
    if(status)
    	HAL_GPIO_WritePin(LCD_RS, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_RS, GPIO_PIN_RESET);
}

static void lcd_data(uint8_t data){
    if(data & (1 << 0))
    	HAL_GPIO_WritePin(LCD_D0, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_D0, GPIO_PIN_RESET);

    if(data & (1 << 1))
    	HAL_GPIO_WritePin(LCD_D1, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_D1, GPIO_PIN_RESET);

    if(data & (1 << 2))
    	HAL_GPIO_WritePin(LCD_D2, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_D2, GPIO_PIN_RESET);

    if(data & (1 << 3))
    	HAL_GPIO_WritePin(LCD_D3, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_D3, GPIO_PIN_RESET);

    if(data & (1 << 4))
    	HAL_GPIO_WritePin(LCD_D4, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_D4, GPIO_PIN_RESET);

    if(data & (1 << 5))
    	HAL_GPIO_WritePin(LCD_D5, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_D5, GPIO_PIN_RESET);

    if(data & (1 << 6))
    	HAL_GPIO_WritePin(LCD_D6, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_D6, GPIO_PIN_RESET);

    if(data & (1 << 7))
    	HAL_GPIO_WritePin(LCD_D7, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_D7, GPIO_PIN_RESET);
}

static void lcd_clear(void){
	HAL_GPIO_WritePin(LCD_RS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_EN, GPIO_PIN_RESET);
    lcd_data(0);
}

static void lcd_enable(void){ // generate E pin pulse
    lcd_e(0);
    HAL_Delay(1);
    lcd_e(1);
    HAL_Delay(1);
    lcd_e(0);
    HAL_Delay(1);
}

static void lcd_config(uint8_t data){ // send config data to lcd
    lcd_rs(0);
    lcd_data(data);
    lcd_enable();
    lcd_clear();
}

static void lcd_write(uint8_t data){ // send characters to lcd
    lcd_rs(1);
    lcd_data(data);
    lcd_enable();
    lcd_clear();
}

static void lcd_pos(uint8_t line, uint8_t pos){ // set cursor position
    if(line)            // display second line
        pos |= 0x40;    // pos 0 of second line is mem pos 0x40
    pos |= 0x80;        // config bit set
    lcd_config(pos);
}

static void lcd_flush(void){ // lcd reset
    lcd_rs(0);
    lcd_data(1);    // data = 0x01
    lcd_enable();
    HAL_Delay(2);
    lcd_clear();
}

static void lcd_init(void){
    lcd_config(0x06);   // display automatic cursor increment
    lcd_config(0x0C);   // active display with hidden cursor
    lcd_config(0x38);   // bit and pixel format
    lcd_flush();
    lcd_pos(0, 0);
}

//////////////////////////////////////////////////////////////////////// LCD "hal" functions

// display number in LCD
static void lcd_number(uint32_t number, uint32_t size, uint32_t line, uint32_t pos){
	uint32_t character = 0;
	uint32_t ten = 0;
	uint32_t i, j;

/*
instead of doing
		size--;
	and having a signed i, with a for like
		for(i = size; i >= 0; i--)
	with a negative threshold and a int32_t i i'll do a
		number *= 10
	and a for like
		for(i = size; i > 0; i--)
	with a uint32_t i
*/

	number *= 10;
	lcd_pos(line, pos);
	for(i = size; i > 0; i--){
		ten = 1;

		for(j = 1; j <= i; j++)
			ten *= 10;

		if(i < size)
			character = ((number / ten) % 10) + 48;
		else
			character = (number / ten) + 48;

		lcd_write(character);
	}
}

// display string in LCD
static void lcd_string(const char *pointer, uint32_t line, uint32_t pos){
	uint32_t counter = 0;

	lcd_pos(line, pos);
	while((counter++ < 16) && (*pointer != '\0'))
		lcd_write(*(pointer++));
}

//////////////////////////////////////////////////////////////////////// hal abstraction

// somehow I've managed to forget the HAL function's name everytime
static void delay(uint32_t time){
	HAL_Delay(time);
}

// measure the ADC value, condition it to return 25 values
static uint32_t adc(){
	uint32_t average = 50;
	uint32_t counter = 0;
	int32_t value = 0;

	for(counter = 0; counter < average; counter++){
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 1);
		value += HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
	}

	value /= (average * 2420);	// value ranges from 27 to 2
	value -= 2;					// value ranges from 25 to 0

	if(value < 0)
		value = 0;

	return value;
}

static void i2s(uint16_t right, uint16_t left){
	uint16_t buffer[2];
	//HAL_StatusTypeDef bob;

	buffer[0] = right;
	buffer[1] = left;
	HAL_I2S_Transmit(&hi2s1, buffer, 2, 0);

}

// UI buttons abstraction
static enum _buttons button_check(void){
	enum _buttons output = 0;
	uint32_t count = 0;

	if(HAL_GPIO_ReadPin(BUT_UP)){
		count++;
		output = up;
	}
	if(HAL_GPIO_ReadPin(BUT_DWN)){
		count++;
		output = down;
	}
	if(HAL_GPIO_ReadPin(BUT_RGT)){
		count++;
		output = right;
	}
	if(HAL_GPIO_ReadPin(BUT_LFT)){
		count++;
		output = left;
	}

	if(count > 1)
		output = invalid;

	return output;
}

//////////////////////////////////////////////////////////////////////// HL functions

// refreshes the UI with updated info
static void refresh(void){
	lcd_string(menu[menu_option], 0, 0);
	lcd_string("                ", 1, 0);
	lcd_number(menu_option, 1, 1, 0);

	switch(menu_option){
	case 0:
		lcd_number(sound_config.vol1, 2, 1, 6);
		break;
	case 1:
		lcd_number(sound_config.vol2, 2, 1, 6);
		break;
	case 2:
		lcd_string(wave_menu[sound_config.shape1], 1, 4);
		break;
	case 3:
		lcd_string(wave_menu[sound_config.shape2], 1, 4);
		break;
	case 4:
		lcd_number(sound_config.harmonics1, 1, 1, 7);
		break;
	case 5:
		lcd_number(sound_config.harmonics2, 1, 1, 7);
		break;
	case 6:
		lcd_string(octave_menu[sound_config.octave1], 1, 5);
		break;
	case 7:
		lcd_string(octave_menu[sound_config.octave2], 1, 5);
		break;
	}
}

// adds a voice to the DDS
static void add_voice(uint16_t frequency, uint16_t channel, uint16_t harmonic){
	switch(channel){
	case 1:
		increment1[harmonic] = frequency * 512 / 5;
		break;
	case 2:
		increment2[harmonic] = frequency * 512 / 5;
		break;
	default:
		break;
	}
}

// removes a voice from the DDS
static void remove_voice(uint16_t channel, uint16_t harmonic){
	switch(channel){
	case 1:
		increment1[harmonic] = 0;
		counter1[harmonic] = 0;
		break;
	case 2:
		increment2[harmonic] = 0;
		counter2[harmonic] = 0;
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////// main functions

// button interrupt that resets the DDS
void customButtonInterrupt(void){
	// dds initial configuration
	sound_config.vol1 = 7;
	sound_config.vol2 = 7;
	sound_config.shape1 = sine;
	sound_config.shape2 = sine;
	sound_config.harmonics1 = 0;
	sound_config.harmonics2 = 0;
	sound_config.octave1 = range12;
	sound_config.octave2 = range12;
	needs_update = 1;
}

// timer interrupt running at sampling rate
void customTimerInterrupt(void){
	volatile uint16_t out_l = 0;
	volatile uint16_t out_r = 0;
	uint16_t harmonic = 0;

	HAL_GPIO_WritePin(INT_OUT, GPIO_PIN_SET);

	// left channel
	for(harmonic = 0; harmonic <= sound_config.harmonics1; harmonic++){
		counter1[harmonic] += increment1[harmonic];
		if(counter1[harmonic] >= bigmax)
			counter1[harmonic] -= bigmax;
		switch(sound_config.shape1){
		case sine:
			out_l += (uint16_t)((int16_t)LUTsine[counter1[harmonic] >> 10] * sound_config.vol1 / (16 * (harmonic + 1)));
			break;
		case square:
			out_l += (uint16_t)((int16_t)LUTsquare[counter1[harmonic] >> 10] * sound_config.vol1 / (16 * (harmonic + 1)));
			break;
		case triangle:
			out_l += (uint16_t)((int16_t)LUTtri[counter1[harmonic] >> 10] * sound_config.vol1 / (16 * (harmonic + 1)));
			break;
		case sawtooth:
			out_l += (uint16_t)((int16_t)LUTsaw[counter1[harmonic] >> 10] * sound_config.vol1 / (16 * (harmonic + 1)));
			break;
		default:
			break;
		}
	}

	// right channel
	for(harmonic = 0; harmonic <= sound_config.harmonics2; harmonic++){
		counter2[harmonic] += increment2[harmonic];
		if(counter2[harmonic] >= bigmax)
			counter2[harmonic] -= bigmax;
		switch(sound_config.shape2){
		case sine:
			out_r += (uint16_t)((int16_t)LUTsine[counter2[harmonic] >> 10] * sound_config.vol2 / (16 * (harmonic + 1)));
			break;
		case square:
			out_r += (uint16_t)((int16_t)LUTsquare[counter2[harmonic] >> 10] * sound_config.vol2 / (16 * (harmonic + 1)));
			break;
		case triangle:
			out_r += (uint16_t)((int16_t)LUTtri[counter2[harmonic] >> 10] * sound_config.vol2 / (16 * (harmonic + 1)));
			break;
		case sawtooth:
			out_r += (uint16_t)((int16_t)LUTsaw[counter2[harmonic] >> 10] * sound_config.vol2 / (16 * (harmonic + 1)));
			break;
		default:
			break;
		}
	}

	i2s(out_l, out_r);
	HAL_GPIO_WritePin(INT_OUT, GPIO_PIN_RESET);
}

// function that runs once before entering the main loop
void customSetup(ADC_HandleTypeDef handler1, I2S_HandleTypeDef handler2){
	uint16_t aux = 0;

	hadc1 = handler1;
	hi2s1 = handler2;
	lcd_init();

	// dds initial configuration
	sound_config.vol1 = 7;
	sound_config.vol2 = 7;
	sound_config.shape1 = sine;
	sound_config.shape2 = sine;
	sound_config.harmonics1 = 0;
	sound_config.harmonics2 = 0;
	sound_config.octave1 = range12;
	sound_config.octave2 = range12;
	for(aux = 0; aux <= MAX_HARM; aux++){
		counter1[aux] = 0;
		counter2[aux] = 0;
		increment1[aux] = 0;
		increment2[aux] = 0;
	}

	delay(10);
	// UI first option
	refresh();
}

// main loop
void customLoop(void){
	// input variables
	enum _buttons button_pressed = button_check();
	uint16_t adc_value = adc();
	uint16_t aux = 0;

	// detects UI input and adjust interface
	if((button_pressed != 0) || (needs_update == 1)){
		switch(menu_option){
		case 0:	// channel 1 volume
			switch(button_pressed){
			case up:
				break;
			case down:
				menu_option++;
				break;
			case right:
				if(sound_config.vol1 < max[menu_option])
					sound_config.vol1++;
				break;
			case left:
				if(sound_config.vol1 > 0)
					sound_config.vol1--;
				break;
			default:
				break;
			}
			break;
		case 1:	// channel 2 volume
			switch(button_pressed){
			case up:
				menu_option--;
				break;
			case down:
				menu_option++;
				break;
			case right:
				if(sound_config.vol2 < max[menu_option])
					sound_config.vol2++;
				break;
			case left:
				if(sound_config.vol2 > 0)
					sound_config.vol2--;
				break;
			default:
				break;
			}
			break;
		case 2:	// channel 1 waveform
			switch(button_pressed){
			case up:
				menu_option--;
				break;
			case down:
				menu_option++;
				break;
			case right:
				if(sound_config.shape1 < max[menu_option])
					sound_config.shape1++;
				break;
			case left:
				if(sound_config.shape1 > 0)
					sound_config.shape1--;
				break;
			default:
				break;
			}
			break;
		case 3:	// channel 2 waveform
			switch(button_pressed){
			case up:
				menu_option--;
				break;
			case down:
				menu_option++;
				break;
			case right:
				if(sound_config.shape2 < max[menu_option])
					sound_config.shape2++;
				break;
			case left:
				if(sound_config.shape2 > 0)
					sound_config.shape2--;
				break;
			default:
				break;
			}
			break;
		case 4:	// channel 1 harmonics
			switch(button_pressed){
			case up:
				menu_option--;
				break;
			case down:
				menu_option++;
				break;
			case right:
				if(sound_config.harmonics1 < max[menu_option])
					sound_config.harmonics1++;
				break;
			case left:
				if(sound_config.harmonics1 > 0)
					sound_config.harmonics1--;
				break;
			default:
				break;
			}
			refresh();
			break;
		case 5:	// channel 2 harmonics
			switch(button_pressed){
			case up:
				menu_option--;
				break;
			case down:
				menu_option++;
				break;
			case right:
				if(sound_config.harmonics2 < max[menu_option])
					sound_config.harmonics2++;
				break;
			case left:
				if(sound_config.harmonics2 > 0)
					sound_config.harmonics2--;
				break;
			default:
				break;
			}
			refresh();
			break;
		case 6:	// channel 1 octave
			switch(button_pressed){
			case up:
				menu_option--;
				break;
			case down:
				menu_option++;
				break;
			case right:
				if(sound_config.octave1 < max[menu_option])
					sound_config.octave1++;
				break;
			case left:
				if(sound_config.octave1 > 0)
					sound_config.octave1--;
				break;
			default:
				break;
			}
			break;
		case 7:	// channel 2 octave
			switch(button_pressed){
			case up:
				menu_option--;
				break;
			case down:
				break;
			case right:
				if(sound_config.octave2 < max[menu_option])
					sound_config.octave2++;
				break;
			case left:
				if(sound_config.octave2 > 0)
					sound_config.octave2--;
				break;
			default:
				break;
			}
			break;
		}
		refresh();
		needs_update = 0;
	}

	// detect ribbon sensor input and activate voices
	if(adc_value != adc_old){
		adc_old = adc_value;
		if(adc_value == 0){ // ribbon sensor released
			HAL_GPIO_WritePin(LED, GPIO_PIN_SET);
			for(aux = 0; aux <= sound_config.harmonics1; aux++)
				remove_voice(1, aux);
			for(aux = 0; aux <= sound_config.harmonics2; aux++)
				remove_voice(2, aux);
		}
		else{ // needs to update the frequency
			HAL_GPIO_WritePin(LED, GPIO_PIN_RESET);
			// will add voices according to the number of harmonics
			for(aux = 0; aux <= sound_config.harmonics1; aux++)
				if(adc_value > 12)
					add_voice(tone[1 + aux][adc_value - 12], 1, aux);
				else
					add_voice(tone[0 + aux][adc_value], 1, aux);
			for(aux = 0; aux <= sound_config.harmonics2; aux++)
				if(adc_value > 12)
					add_voice(tone[1 + aux][adc_value - 12], 2, aux);
				else
					add_voice(tone[0 + aux][adc_value], 2, aux);
		}
	}

	// interface loop time interval
	delay(100);
}








































