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

static ADC_HandleTypeDef hadc1;
static I2S_HandleTypeDef hi2s1;

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

// UI variables
static uint8_t menu_option = 0;
char menu[8][16] = {
		"ch. 1 volume:   ",
		"ch. 2 volume:   ",
		"ch. 1 waveform: ",
		"ch. 2 waveform: ",
		"ch. 1 harmonics:",
		"ch. 2 harmonics:",
		"ch. 1 octave:   ",
		"ch. 2 octave:   ",
};
char octave_menu[6][6] = {
		"1 - 2",
		"2 - 3",
		"3 - 4",
		"4 - 5",
		"5 - 6",
		"6 - 7",
};
char wave_menu[4][8] = {
		"  sine  ",
		" square ",
		"triangle",
		"sawtooth",
};
static uint8_t max[8] = {
		15, 15, 3, 3, 7, 7, 5, 5
};
static uint16_t adc_old = 0;

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

static void lcd_enable(void){
    lcd_e(0);
    HAL_Delay(1);
    lcd_e(1);
    HAL_Delay(1);
    lcd_e(0);
    HAL_Delay(1);
}

static void lcd_config(uint8_t data){
    lcd_rs(0);
    lcd_data(data);
    lcd_enable();
    lcd_clear();
}

static void lcd_write(uint8_t data){
    lcd_rs(1);
    lcd_data(data);
    lcd_enable();
    lcd_clear();
}

static void lcd_pos(uint8_t line, uint8_t pos){
    if(line)            // display second line
        pos |= 0x40;    // pos 0 of second line is mem pos 0x40
    pos |= 0x80;        // config bit set
    lcd_config(pos);
}

static void lcd_flush(void){
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

static void lcd_string(const char *pointer, uint32_t line, uint32_t pos){
	uint32_t counter = 0;

	lcd_pos(line, pos);
	while((counter++ < 16) && (*pointer != '\0'))
		lcd_write(*(pointer++));
}

//////////////////////////////////////////////////////////////////////// hal abstraction

static void delay(uint32_t time){
	HAL_Delay(time);
}

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

	return value;
}

static void i2s(uint16_t right, uint16_t left){
	uint16_t buffer[2];
	//HAL_StatusTypeDef bob;

	buffer[0] = right;
	buffer[1] = left;
	HAL_I2S_Transmit(&hi2s1, buffer, 2, 0);

}

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

//////////////////////////////////////////////////////////////////////// main functions

void customButtonInterrupt(void){
	static uint32_t state = 0;

	HAL_GPIO_WritePin(LED, state);
	state ^= 1;

}

void customTimerInterrupt(void){
	static uint32_t counter1 = 0;
	static uint32_t counter2 = 0;

	i2s(LUTsine[counter1], LUTsquare[counter2]);

	counter1++;
	if(counter1 == LUT_SIZE)
		counter1 = 0;
	counter2++;
	if(counter2 == LUT_SIZE)
		counter2 = 0;
}

void customSetup(ADC_HandleTypeDef handler1, I2S_HandleTypeDef handler2){
	//uint8_t aux = 0;

	hadc1 = handler1;
	hi2s1 = handler2;
	lcd_init();

	sound_config.vol1 = 7;
	sound_config.vol2 = 7;
	sound_config.shape1 = sine;
	sound_config.shape2 = sine;
	sound_config.harmonics1 = 0;
	sound_config.harmonics2 = 0;
	sound_config.octave1 = range12;
	sound_config.octave2 = range12;

	delay(10);

	// UI first option
	lcd_string(menu[menu_option], 0, 0);
	lcd_number(sound_config.vol1, 2, 1, 6);
}

void customLoop(void){
	// input variables
	enum _buttons button_pressed = button_check();
	uint16_t adc_value = adc();

	// detects UI input and adjust interface
	if(button_pressed != 0){
		lcd_string(menu[menu_option], 0, 0);
		lcd_string("                ", 1, 0);
		switch(menu_option){
		case 0:
			switch(button_pressed){
			case up:
				break;
			case down:
				break;
			case right:
				break;
			case left:
				break;
			default:
				break;
			}
			lcd_number(sound_config.vol1, 2, 1, 6);
			break;
		case 1:
			switch(button_pressed){
			case up:
				break;
			case down:
				break;
			case right:
				break;
			case left:
				break;
			default:
				break;
			}
			lcd_number(sound_config.vol2, 2, 1, 6);
			break;
		case 2:
			switch(button_pressed){
			case up:
				break;
			case down:
				break;
			case right:
				break;
			case left:
				break;
			default:
				break;
			}
			lcd_string(wave_menu[sound_config.shape1], 1, 4);
			break;
		case 3:
			switch(button_pressed){
			case up:
				break;
			case down:
				break;
			case right:
				break;
			case left:
				break;
			default:
				break;
			}
			lcd_string(wave_menu[sound_config.shape2], 1, 4);
			break;
		case 4:
			switch(button_pressed){
			case up:
				break;
			case down:
				break;
			case right:
				break;
			case left:
				break;
			default:
				break;
			}
			lcd_number(sound_config.harmonics1, 1, 1, 7);
			break;
		case 5:
			switch(button_pressed){
			case up:
				break;
			case down:
				break;
			case right:
				break;
			case left:
				break;
			default:
				break;
			}
			lcd_number(sound_config.harmonics2, 1, 1, 7);
			break;
		case 6:
			switch(button_pressed){
			case up:
				break;
			case down:
				break;
			case right:
				break;
			case left:
				break;
			default:
				break;
			}
			lcd_string(octave_menu[sound_config.octave1], 1, 5);
			break;
		case 7:
			switch(button_pressed){
			case up:
				break;
			case down:
				break;
			case right:
				break;
			case left:
				break;
			default:
				break;
			}
			lcd_string(octave_menu[sound_config.octave2], 1, 5);
			break;
		}
	}

	// detect ribbon sensor input and activate voices
	if(adc_value != adc_old){
		adc_old = adc_value;
		// ativar ou mudar voz
	}

	// interface loop time interval
	delay(100);
}





