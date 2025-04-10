/*
 * i2c_lcd.c
 *
 *  Created on: 2019. 9. 4.
 *      Author: k
 */

#include "i2c_lcd.h"
#include "extern.h"

#include "def.h"

//#include <stm32f4xx_hal.h>
#include <string.h>
#include <stdio.h>



t_I2C_settings I2C_lcd_settings;


void i2c_lcd_display_line0() {

	static int value = 0;
	static int sec = 0;
	if(TIM11_line0 >= 100) {
		value++;
		if(value > 9) {
			value = 0;
			++sec;

		}
		TIM11_line0= 0;
	}


	move_cursor(0,0);

	char buffer[12] = {0};

	sprintf(buffer, "s:%d %d", sec, value);
	lcd_string(buffer);


}

void i2c_lcd_display_line1() {

	static int value = 0;

	static int sec = 0;

	if(TIM11_line1 >= 100) {
		value++;
		if(value > 9) {
			value = 0;
			++sec;
		}
		TIM11_line1= 0;
	}

	move_cursor(1,0);

	char buffer[12] = {0};

	sprintf(buffer, "s:%d %d", sec, value);
	lcd_string(buffer);

}


void i2c_lcd_main(void)
{

	uint8_t value=0;
	i2c_lcd_init();

	while(1)
	{
		move_cursor(0,0);
		lcd_string("Hello World!!!");
		move_cursor(1,0);
		lcd_data(value + '0');
		value++;
		if(value>9)value=0;
		HAL_Delay(500);
	}

}

t_I2C_COMM_state lcd_command(uint8_t command){

	uint8_t high_nibble, low_nibble;
	uint8_t i2c_buffer[5];
	high_nibble = command & 0xf0;
	low_nibble = (command<<4) & 0xf0;

	i2c_buffer[0] = I2C_LCD_W;
	i2c_buffer[1] = high_nibble | 0x04 | 0x08; //en=1, rs=0, rw=0, backlight=1
	i2c_buffer[2] = high_nibble | 0x00 | 0x08; //en=0, rs=0, rw=0, backlight=1
	i2c_buffer[3] = low_nibble  | 0x04 | 0x08; //en=1, rs=0, rw=0, backlight=1
	i2c_buffer[4] = low_nibble  | 0x00 | 0x08; //en=0, rs=0, rw=0, backlight=1

	t_I2C_COMM_state state = I2C_write(&I2C_lcd_settings, i2c_buffer, 5, I2C_START_AND_STOP);

	return state;
}
t_I2C_COMM_state lcd_data(uint8_t data){

	uint8_t high_nibble, low_nibble;
	uint8_t i2c_buffer[5];
	high_nibble = data & 0xf0;
	low_nibble = (data<<4) & 0xf0;

	i2c_buffer[0] = I2C_LCD_W;
	i2c_buffer[1] = high_nibble | 0x05 | 0x08; //en=1, rs=1, rw=0, backlight=1
	i2c_buffer[2] = high_nibble | 0x01 | 0x08; //en=0, rs=1, rw=0, backlight=1
	i2c_buffer[3] = low_nibble  | 0x05 | 0x08; //en=1, rs=1, rw=0, backlight=1
	i2c_buffer[4] = low_nibble  | 0x01 | 0x08; //en=0, rs=1, rw=0, backlight=1


	t_I2C_COMM_state state = I2C_write(&I2C_lcd_settings, i2c_buffer, 5, I2C_START_AND_STOP);

	return state;
}
void i2c_lcd_init(void){

	t_I2C_gpio_info* scl_info = &I2C_lcd_settings.gpio_infos[I2C_SCL];
	t_I2C_gpio_info* sda_info = &I2C_lcd_settings.gpio_infos[I2C_SDA];

	scl_info->gpio_port = (uint32_t*)DMA_GPIOB_BASE;
	scl_info->gpio_pin = GPIO_PIN_8;
	scl_info->gpio_mode = I2C_MODE_OUTPUT;
	scl_info->pull_config = I2C_PULL_UP;


	sda_info->gpio_port = (uint32_t*)DMA_GPIOB_BASE;
	sda_info->gpio_pin = GPIO_PIN_9;
	sda_info->gpio_mode = I2C_MODE_OUTPUT;
	sda_info->pull_config = I2C_PULL_UP;

	I2C_lcd_settings.timeout_ms = I2C_TIMEOUT;
	I2C_lcd_settings.sampling_type = I2C_RISING_EDGE_SAMPLING; 	// sampling은 rising 시점에
	I2C_lcd_settings.ack = LOW;									// ack 기준은 low
	I2C_lcd_settings.bit_order = I2C_MSB_FIRST; 				// MSB 먼저 처리


	I2C_init(&I2C_lcd_settings);

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;
	state = lcd_command(0x33);
	if(state != I2C_COMM_STATE_OK) {

		printf("1 err %d", state);
		return;
	}

	state = lcd_command(0x32);
	if(state != I2C_COMM_STATE_OK) {

		printf("lcd command %x %d", 0x32, state);
		return;
	}

	//lcd_command(0x28);	//Function Set 4-bit mode
	state = lcd_command(0x28);
	if(state != I2C_COMM_STATE_OK) {

		printf("lcd command  err %d", state);
		return;
	}

	state = lcd_command(DISPLAY_ON);
	if(state != I2C_COMM_STATE_OK) {

		printf("4 err %d", state);
		return;
	}
	state = lcd_command(0x06);	//Entry mode set
	if(state != I2C_COMM_STATE_OK) {

		printf("5 err %d", state);
		return;
	}
	state = lcd_command(CLEAR_DISPLAY);
	if(state != I2C_COMM_STATE_OK) {

		printf("6 err %d", state);
		return;
	}
	HAL_Delay(2);
}
void lcd_string(char* inputStr){

	while(*inputStr)	{
		lcd_data(*inputStr++);
	}
}
void move_cursor(uint8_t row, uint8_t column){
	lcd_command(0x80 | row<<6 | column);
	return;
}











