/*
 * i2c_lcd.h
 *
 *  Created on: 2019. 9. 4.
 *      Author: k
 */

#ifndef SRC_I2C_LCD_H_
#define SRC_I2C_LCD_H_

#include "main.h"
#include "i2c.h"

#define I2C_LCD_ADDRESS 0x27

#define I2C_LCD_W (I2C_LCD_ADDRESS << 1 | 0x00)
#define I2C_LCD_R (I2C_LCD_ADDRESS << 1 | 0x01)

#define BACKLIGHT_ON 0x08

/* LCD command   */
#define DISPLAY_ON 0x0C
#define DISPLAY_OFF 0x08
#define CLEAR_DISPLAY 0x01  //Delay 2msec
#define RETURN_HOME 0x02

void i2c_lcd_init(void);
void i2c_lcd_main();
void i2c_lcd_display_line0();
void i2c_lcd_display_line1();

t_I2C_COMM_state lcd_command(uint8_t command);
t_I2C_COMM_state lcd_data(uint8_t data);

void lcd_string(char *str);
void move_cursor(uint8_t row, uint8_t column);

#endif /* SRC_I2C_LCD_H_ */
