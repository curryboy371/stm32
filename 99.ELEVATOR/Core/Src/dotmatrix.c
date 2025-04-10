

#include "dotmatrix.h"

#include "extern.h"

#include <stdio.h>

#include "spi.h"




uint8_t number_data[][8] =
{
	{
		0b00000000,
		0b00011000,
		0b00111000,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00111100,
		0b00000000
	},
	{0b00000000,
	0b00011000,
	0b00101100,
	0b00101100,
	0b00001000,
	0b00010000,
	0b00111100,
	0b00000000},

	{0b00000000,
	0b00111100,
	0b00000100,
	0b00111100,
	0b00000100,
	0b00000100,
	0b00111100,
	0b00000000},

	{0b00000000,
	0b00100100,
	0b00100100,
	0b00100100,
	0b00100100,
	0b00111110,
	0b00000100,
	0b00000000},


};

uint8_t name[3][8] = {

	{
	0b01010100,
	0b01110110,
	0b01010100,
	0b01110100,
	0b00000000,
	0b01111100,
	0b00000100,
	0b00000100},

	{
	0b00100100,
	0b01110110,
	0b00100100,
	0b01010100,
	0b00000000,
	0b00100000,
	0b00111100,
	0b00000000},

	{
	0b00010000,
	0b00101000,
	0b01000100,
	0b00010000,
	0b01111100,
	0b00000000,
	0b00111000,
	0b00010000
	}
};

uint8_t up_arrow[2][8] = {

	{
		0b00000000,
		0b00011000,
		0b00111100,
		0b01011010,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00000000
	},

	{
		0b00000000,
		0b00011000,
		0b00111100,
		0b01011010,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00000000
	},
};

uint8_t down_arrow[2][8] = {

	{
	0b00000000,
	0b00011000,
	0b00011000,
	0b01011010,
	0b01111110,
	0b00111100,
	0b00011000,
	0b00000000
	},

	{
	0b00000000,
	0b00011000,
	0b00011000,
	0b01011010,
	0b01111110,
	0b00111100,
	0b00011000,
	0b00000000
	}
};

t_dotmatrix_info dotmatrix_info;


unsigned char display_data[8];  // 최종 8x8 출력할 데이터
unsigned char scroll_buffer[50][8] = {0};  // 스코롤할 데이타가 들어있는 버퍼
int number_of_character = 3;  // 출력할 문자 갯수

uint8_t start_buffer = 0;
uint8_t max_count = 0;
uint16_t ani_speed = 0;

uint8_t col[4]={0,0,0,0};



t_spi_gpio_setting dotmatrix_spi_gpio;


uint8_t dotmatrix_mode = 0;



void init_dotmatrix_pin() {


	dotmatrix_spi_gpio.gpio_port[SPI_PIN_DS] = SER_74HC595_GPIO_Port;
	dotmatrix_spi_gpio.gpio_port[SPI_PIN_SH] = CLK_74HC595_GPIO_Port;
	dotmatrix_spi_gpio.gpio_port[SPI_PIN_ST] = LATCH_74HC595_GPIO_Port;

	dotmatrix_spi_gpio.gpio_pin[SPI_PIN_DS] =  SER_74HC595_Pin;
	dotmatrix_spi_gpio.gpio_pin[SPI_PIN_SH] =  CLK_74HC595_Pin;
	dotmatrix_spi_gpio.gpio_pin[SPI_PIN_ST] =  LATCH_74HC595_Pin;

	spi_init(&dotmatrix_spi_gpio);


	init_dotmatrix(0);
}

// 초기화 작업
// 1. display_data에 number_data[0]에 있는 내용 복사
// 2. number_data를 scroll_buffer에 복사
// 3. dotmatrix의 led를 off

void init_dotmatrix(uint8_t inCurMode)
{
	if(inCurMode == IDLE_ANI) {

		number_of_character= 4;
		max_count = 8;
		start_buffer = 1;
		ani_speed = 300;

/*		for (int i=0; i < 8; i++)
		{
			display_data[i] = number_data[i];
		}
		for (int i=1; i < number_of_character+1; i++)
		{
			for (int j=0; j < 8; j++) // scroll_buffer[0] = blank
			{
				scroll_buffer[i][j] = number_data[i-1][j];
			}
		}*/
	}
	else if(inCurMode == UP_ANI) {

		number_of_character= 2;
		start_buffer = 1;
		max_count = 7;
		ani_speed = 400;
/*		for (int i=0; i < 8; i++)
		{
			display_data[i] = up_arrow[i];
		}
		for (int i=1; i < number_of_character + 1; i++)
		{
			for (int j=0; j < 8; j++) // scroll_buffer[0] = blank
			{
				scroll_buffer[i][j] = up_arrow[i-1][j];
			}
		}*/
	}
	else if(inCurMode == DOWN_ANI) {

		number_of_character= 2;
		max_count = 7;
		start_buffer = 1;
		ani_speed = 500;
/*		for (int i=0; i < 8; i++)
		{
			display_data[i] = down_arrow[i];
		}
		for (int i=1; i < number_of_character + 1; i++)
		{
			for (int j=0; j < 8; j++) // scroll_buffer[0] = blank
			{
				scroll_buffer[i][j] = down_arrow[i-1][j];
			}
		}*/
	}

	dotmatrix_mode = inCurMode;

}

void render_dotmatrix() {

	static int render_index = 0;

	static int count=0;  // 컬럼 count
	static int index=0;  // scroll_buffer의 2차원 index값
	static uint32_t past_time=0;  // 이전 tick값 저장

	uint32_t now = HAL_GetTick();  // 1ms
	// 1.처음시작시 past_time=0; now: 500 --> past_time=500
	if (now - past_time >= ani_speed) // 500ms scroll
	{
		past_time = now;

		int nextIdx = index+1;
		if(nextIdx > number_of_character) {
			nextIdx  = start_buffer;
		}

		if(dotmatrix_mode == IDLE_ANI) {


			// 옆으로
			for (int i=0; i < 8; i++)
			{
				display_data[i] = number_data[dotmatrix_info.cur_index][i];

/*
				display_data[i] = (scroll_buffer[index][i] >> count) |
						(scroll_buffer[nextIdx][i] << 8 - count);*/
			}
		}
		else if(dotmatrix_mode == UP_ANI) {

			// 위로
			for(int i = 0; i < 8-(count); ++i) {

				display_data[i] = number_data[dotmatrix_info.cur_index][count+i];
				//display_data[i] = scroll_buffer[index][count+i];
			}

			for(int i = 0; i < count; ++i) {

				display_data[7-i] = number_data[dotmatrix_info.next_index][count-i];
				//display_data[7-i] = scroll_buffer[nextIdx][count-i];
			}

		}
		else if(dotmatrix_mode == DOWN_ANI) {

			// 아래로
			for(int i = 0; i < count; ++i) {

				display_data[i] = number_data[dotmatrix_info.next_index][7 -count + i];
				//display_data[i] = scroll_buffer[nextIdx][7 -count + i];
			}

			for(int i = count; i < 8; ++i) {

				display_data[i] = number_data[dotmatrix_info.cur_index][i - count];
				//display_data[i] = scroll_buffer[index][i - count];
			}
		}


		if (++count == max_count) // 8칼람을 다 처리 했으면 다음 scroll_buffer로 이동
		{
			count =0;
			index++;  // 다음 scroll_buffer로 이동
			if (index > number_of_character) index=start_buffer;
			// 11개의 문자를 다 처리 했으면 0번 scroll_buffer를 처리 하기위해 이동
		}

	}



	col[0] = ~(1 << render_index);  // 00000001  --> 11111110
	col[1] = display_data[render_index];

	for(int j = 0; j < 2; j++) {
		spi_tx_data(&dotmatrix_spi_gpio, col[j]);
	}


	// latch
	spi_letch(&dotmatrix_spi_gpio);

	++render_index;
	render_index %= 8;

}

void dotmatrix_refresh_idx(uint8_t cur_idx, uint8_t next_idx) {


	dotmatrix_info.cur_index = cur_idx;
	dotmatrix_info.next_index = next_idx;


}

void dotmatrix_main_test()
{
	static int count=0;  // 컬럼 count
	static int index=1;  // scroll_buffer의 2차원 index값
	static uint32_t past_time=0;  // 이전 tick값 저장

	//spi_init();


	while(1){
		render_dotmatrix(0);
	}



/*	initMode = UP_ARROW_ANI;
	init_dotmatrix();*/

/*	while (1)
	{
		uint32_t now = HAL_GetTick();  // 1ms
		// 1.처음시작시 past_time=0; now: 500 --> past_time=500
		if (now - past_time >= 100) // 500ms scroll
		{
			past_time = now;


			if(initMode == NAME_ANI) {

				// 옆으로
				for (int i=0; i < 8; i++)
				{
					display_data[i] = (scroll_buffer[index][i] >> count) |
							(scroll_buffer[index+1][i] << 8 - count);
				}
			}
			else if(initMode == UP_ARROW_ANI) {

				// 위로
				for(int i = 0; i < 8-count; ++i) {

					display_data[i] = scroll_buffer[index][count+i];
				}

				for(int i = 0; i <= count; ++i) {
					display_data[7-i] = scroll_buffer[index+1][count-i];
				}

			}
			else if(initMode == DOWN_ARROW_ANI) {

				// 아래로
				for(int i = 0; i < count; ++i) {
					display_data[i] = scroll_buffer[index+1][7 -count + i];
				}

				for(int i = count; i < 8; ++i) {

					display_data[i] = scroll_buffer[index][i - count];
				}
			}


			if (++count == 8) // 8칼람을 다 처리 했으면 다음 scroll_buffer로 이동
			{
				count =0;
				index++;  // 다음 scroll_buffer로 이동
				if (index >= number_of_character) index=1;
				// 11개의 문자를 다 처리 했으면 0번 scroll_buffer를 처리 하기위해 이동
			}
		}

		for (int i=0; i < 8; i++)
		{
			col[0] = ~(1 << i);  // 00000001  --> 11111110
			col[1] = display_data[i];

			for(int j = 0; j < 2; j++) {
				spi_tx_data(col[j]);
			}

			// latch
			shift_register_latch();
		}

	}*/
}
