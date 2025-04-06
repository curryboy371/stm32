#include "lcd1602.h"

#include "extern.h"
#include "timer.h"
void lcd1602_main(void);
void lcd1602_init(void);
void lcd1602_data(uint8_t data);
void Write_Data_to_DDRAM(uint8_t nibble);
void pulse_enable(void);
void Set_RS(uint8_t rs);
void lcd1602_move_cursor(uint8_t row, uint8_t column);

void lcd1602_string(char* pstr);

void lcd1602_main(void)
{

   lcd1602_init();

   while(1)
   {
      lcd1602_move_cursor(0,0);
      lcd1602_string("kh!");

      HAL_Delay(500);
   }
}


void lcd1602_command(uint8_t command)
{
   uint8_t high_nibble, low_nibble;
   high_nibble = command >> 4;
   low_nibble = (command & 0x0f);

   Set_RS(0);
   Write_Data_to_DDRAM(high_nibble);
   Write_Data_to_DDRAM(low_nibble);

   delay_us(37); // 최소 시간
}

void lcd1602_data(uint8_t data){ // 1byte를 출력

   uint8_t high_nibble, low_nibble;
   high_nibble = data >> 4;
   low_nibble = (data & 0x0f);

   Set_RS(1);
   Write_Data_to_DDRAM(high_nibble);
   Write_Data_to_DDRAM(low_nibble);

   delay_us(37); // 최소 시간
}


void lcd1602_init(void) //초기화
{
   HAL_Delay(50);

   Write_Data_to_DDRAM(RETURN_HOME);//lcd신호를 보내서 깨움
   HAL_Delay(5);
   Write_Data_to_DDRAM(RETURN_HOME);
   HAL_Delay(5);
   Write_Data_to_DDRAM(RETURN_HOME);
   HAL_Delay(1);
   Write_Data_to_DDRAM(0x02); //4비트 전송모드로 진입


   lcd1602_command(0x28); //Function Set 4-bit mode , font set
   delay_us(40); // Function Set 최소 시간
   lcd1602_command(DISPLAY_ON); //0x0c
   delay_us(40); // Display On 최소 시간
   lcd1602_command(0x06);   //Entry mode set 글자 방향설정 왼쪽에서 오른쪽 커서이동도 자동적으로 해줄 수 있게끔해줌
   delay_us(40); // Entry mode set 최소 시간
   lcd1602_command(CLEAR_DISPLAY); //0x01
   HAL_Delay(2); // Clear display 최소 시간
}

void lcd1602_string(char* pstr){ //string값 찍는 함수 , null을 만날 때 까지 출력

	while(*pstr) {
	   lcd1602_data(*pstr++);
   }

}

void lcd1602_move_cursor(uint8_t row, uint8_t column){ // 커서 이동 함수
   lcd1602_command(0x80 | row<<6 | column); //첫번째 행은 0x80이고 두번째 행은 0x80+0x40
   return;
}

void Write_Data_to_DDRAM(uint8_t nibble)
{
   HAL_GPIO_WritePin(GPIOB, LCD_D4, (nibble & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOB, LCD_D5, (nibble & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOB, LCD_D6, (nibble & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
   HAL_GPIO_WritePin(GPIOB, LCD_D7, (nibble & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
   pulse_enable();
}

void pulse_enable(void)
{
    // EN 핀 제어: EN 핀에 펄스 생성
    HAL_GPIO_WritePin(GPIOB, LCD_EN, SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, LCD_EN, RESET);
    HAL_Delay(1);

}

void Set_RS(uint8_t rs)//rs는 register select
{
   HAL_GPIO_WritePin(GPIOB, LCD_RS, (rs ? SET : RESET));//0과 1일때 모드가 달라진다? 매커니즘이 달라진다
}

