#include "custom.h"

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

static uint32_t buttons = 0;

//////////////////////////////////////////////////////////////////////// LCD "ll" functions

void lcd_e(uint8_t status){
    if(status)
        HAL_GPIO_WritePin(LCD_EN, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(LCD_EN, GPIO_PIN_RESET);
}

void lcd_rs(uint8_t status){
    if(status)
    	HAL_GPIO_WritePin(LCD_RS, GPIO_PIN_SET);
    else
    	HAL_GPIO_WritePin(LCD_RS, GPIO_PIN_RESET);
}

void lcd_data(uint8_t data){
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

void lcd_clear(void){
	HAL_GPIO_WritePin(LCD_RS, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_EN, GPIO_PIN_RESET);
    lcd_data(0);
}

void lcd_enable(void){
    lcd_e(0);
    HAL_Delay(1);
    lcd_e(1);
    HAL_Delay(1);
    lcd_e(0);
    HAL_Delay(1);
}

void lcd_config(uint8_t data){
    lcd_rs(0);
    lcd_data(data);
    lcd_enable();
    lcd_clear();
}

void lcd_write(uint8_t data){
    lcd_rs(1);
    lcd_data(data);
    lcd_enable();
    lcd_clear();
}

void lcd_pos(uint8_t line, uint8_t pos){
    if(line)            // display second line
        pos |= 0x40;    // pos 0 of second line is mem pos 0x40
    pos |= 0x80;        // config bit set
    lcd_config(pos);
}

void lcd_flush(void){
    lcd_rs(0);
    lcd_data(1);    // data = 0x01
    lcd_enable();
    HAL_Delay(2);
    lcd_clear();
}

void lcd_init(void){
    lcd_config(0x06);   // display automatic cursor increment
    lcd_config(0x0C);   // active display with hidden cursor
    lcd_config(0x38);   // bit and pixel format
    lcd_flush();
    lcd_pos(0, 0);
}

//////////////////////////////////////////////////////////////////////// LCD "hal" functions

void lcd_number(uint32_t number, uint32_t size, uint32_t line, uint32_t pos){
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

void lcd_string(const char *pointer, uint32_t line, uint32_t pos){
	uint32_t counter = 0;

	lcd_pos(line, pos);
	while((counter++ < 16) && (*pointer != '\0'))
		lcd_write(*(pointer++));
}

//////////////////////////////////////////////////////////////////////// main functions

void customSetup(void){
	//uint8_t aux = 0;

	lcd_init();
	HAL_Delay(500);

}

void customLoop(void){
	buttons = 0;

	if(HAL_GPIO_ReadPin(BUT_UP))
		buttons++;
	if(HAL_GPIO_ReadPin(BUT_DWN))
		buttons++;
	if(HAL_GPIO_ReadPin(BUT_LFT))
		buttons++;
	if(HAL_GPIO_ReadPin(BUT_RGT))
		buttons++;

	if(buttons == 1){
		if(HAL_GPIO_ReadPin(BUT_UP))
			lcd_string("UP             ", 0, 0);
		if(HAL_GPIO_ReadPin(BUT_DWN))
			lcd_string("DOWN           ", 0, 0);
		if(HAL_GPIO_ReadPin(BUT_LFT))
			lcd_string("LEFT           ", 0, 0);
		if(HAL_GPIO_ReadPin(BUT_RGT))
			lcd_string("RIGHT          ", 0, 0);
	}
	else
		if(buttons == 0)
			lcd_string("me aperta vai", 0, 0);
		else
			lcd_string("para com isso", 0, 0);


	HAL_Delay(100);
}





