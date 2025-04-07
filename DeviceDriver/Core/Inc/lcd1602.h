

#include "main.h"

#define CLEAR_DISPLAY    0x01 //
#define RETURN_HOME    0x03
#define DISPLAY_ON       0x0C


#define LCD_RS          GPIO_PIN_0
#define LCD_EN          GPIO_PIN_1
#define LCD_D4          GPIO_PIN_4
#define LCD_D5          GPIO_PIN_5
#define LCD_D6          GPIO_PIN_6
#define LCD_D7          GPIO_PIN_7

#define SET         	1
#define RESET          	0

void lcd1602_main(void);
void lcd1602_init(void);
void lcd1602_data(uint8_t data);
void Write_Data_to_DDRAM(uint8_t nibble);
void pulse_enable(void);
void Set_RS(uint8_t rs);
void lcd1602_move_cursor(uint8_t row, uint8_t column);

void lcd1602_string(char* pstr);
