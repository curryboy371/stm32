

#include "dotmatrix.h"

#include "extern.h"

#include <stdio.h>

#include "spi.h"
#include "animation.h"
#if 1




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

unsigned char display_data[8];  // 최종 8x8 출력할 데이터
unsigned char scroll_buffer[50][8] = {0};  // 스코롤할 데이타가 들어있는 버퍼
int number_of_character = 3;  // 출력할 문자 갯수

uint8_t start_buffer = 0;
uint8_t max_count = 0;

uint8_t col[4]={0,0,0,0};

// 초기화 작업
// 1. display_data에 number_data[0]에 있는 내용 복사
// 2. number_data를 scroll_buffer에 복사
// 3. dotmatrix의 led를 off

void init_dotmatrix(uint8_t inCurMode)
{
	if(inCurMode == NAME_ANI) {

		number_of_character= 3;
		max_count = 8;
		start_buffer = 1;
		for (int i=0; i < 8; i++)
		{
			display_data[i] = name[i];
		}
		for (int i=1; i < number_of_character+1; i++)
		{
			for (int j=0; j < 8; j++) // scroll_buffer[0] = blank
			{
				scroll_buffer[i][j] = name[i-1][j];
			}
		}
	}
	else if(inCurMode == UP_ARROW_ANI) {

		number_of_character= 2;
		start_buffer = 1;
		max_count = 7;
		for (int i=0; i < 8; i++)
		{
			display_data[i] = up_arrow[i];
		}
		for (int i=1; i < number_of_character + 1; i++)
		{
			for (int j=0; j < 8; j++) // scroll_buffer[0] = blank
			{
				scroll_buffer[i][j] = up_arrow[i-1][j];
			}
		}
	}
	else if(inCurMode == DOWN_ARROW_ANI) {

		number_of_character= 2;
		max_count = 7;
		start_buffer = 1;
		for (int i=0; i < 8; i++)
		{
			display_data[i] = down_arrow[i];
		}
		for (int i=1; i < number_of_character + 1; i++)
		{
			for (int j=0; j < 8; j++) // scroll_buffer[0] = blank
			{
				scroll_buffer[i][j] = down_arrow[i-1][j];
			}
		}
	}

}

void render_dotmatrix(uint8_t inCurMode) {

	static int count=0;  // 컬럼 count
	static int index=0;  // scroll_buffer의 2차원 index값
	static uint32_t past_time=0;  // 이전 tick값 저장

	uint32_t now = HAL_GetTick();  // 1ms
	// 1.처음시작시 past_time=0; now: 500 --> past_time=500
	if (now - past_time >= 200) // 500ms scroll
	{
		past_time = now;

		int nextIdx = index+1;
		if(nextIdx > number_of_character) {
			nextIdx  = start_buffer;
		}

		if(inCurMode == NAME_ANI) {
			// 옆으로
			for (int i=0; i < 8; i++)
			{
				display_data[i] = (scroll_buffer[index][i] >> count) |
						(scroll_buffer[nextIdx][i] << 8 - count);
			}
		}
		else if(inCurMode == UP_ARROW_ANI) {
			// 위로
			for(int i = 0; i < 8-(count); ++i) {

				display_data[i] = scroll_buffer[index][count+i];
			}

			for(int i = 0; i < count; ++i) {
				display_data[7-i] = scroll_buffer[nextIdx][count-i];
			}
		}
		else if(inCurMode == DOWN_ARROW_ANI) {

			// 아래로
			for(int i = 0; i < count; ++i) {
				display_data[i] = scroll_buffer[nextIdx][7 -count + i];
			}

			for(int i = count; i < 8; ++i) {

				display_data[i] = scroll_buffer[index][i - count];
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


	// tx and latch
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

}

void shift_register_latch() {

	*GPIOB_ODR &=  ~GPIO_PIN_13;
	*GPIOB_ODR |= GPIO_PIN_13;

	//GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
	//GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up


	HAL_Delay(1);
}


void dotmatrix_main_test()
{
	static int count=0;  // 컬럼 count
	static int index=1;  // scroll_buffer의 2차원 index값
	static uint32_t past_time=0;  // 이전 tick값 저장

	spi_init();



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


#else





void dotmatrix_main_test();
void init_dotmatrix(void);
int dotmatrix_main(void);
int dotmatrix_main_func(void);

uint8_t allon[] = {			// allon 문자 정의
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111
};


uint8_t smile[] = {			// 스마일 문자 정의
	0b00111100,
	0b01000010,
	0b10010101,
	0b10100001,
	0b10100001,
	0b10010101,
	0b01000010,
	0b00111100 };

uint8_t hart[] = {		// hart
	0b00000000,    // hart
	0b01100110,
	0b11111111,
	0b11111111,
	0b11111111,
	0b01111110,
	0b00111100,
	0b00011000
};

uint8_t name[3][10] = {		// hart
	{0b00010010,
	0b00111010,
	0b00010011,
	0b00101010,
	0b00000000,
	0b00001000,
	0b00001111,
	0b00000000},


	{0b00010000,
	0b00101000,
	0b01010100,
	0b00010000,
	0b11111100,
	0b00000000,
	0b00000000,
	0b00010000},

	{
		0b00000000,    // hart
		0b01100110,
		0b11111111,
		0b11111111,
		0b11111111,
		0b01111110,
		0b00111100,
		0b00011000
	}
};

uint8_t one[] =
{0b00011000,
0b00111000,
0b00011000,
0b00011000,
0b00011000,
0b00011000,
0b01111110,
0b01111110};

uint8_t my_name[] =
{0B01111010,
0B00001010,
0B00001010,
0B11111010,
0B00100010,
0B10101110,
0B10000010,
0B11111110};



uint8_t col[4]={0,0,0,0};

void dotmatrix_main_test()
{
  //dotmatrix_main();

  while (1)
  {
        for (int i=0; i < 8; i++)
        {
			col[0] = ~(1 << i);  // 00000001  --> 11111110
			col[1] = hart[i];
			HAL_SPI_Transmit(&hspi2, col, 2, 1);
			GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
			GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up
			HAL_Delay(1);
        }
  }
}

uint8_t number_data[20][10] =
{
	{
		0b01110000,	//0
		0b10001000,
		0b10011000,
		0b10101000,
		0b11001000,
		0b10001000,
		0b01110000,
	    0b00000000
	},
	{
		0b01000000,	//1
		0b11000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b01000000,
		0b11100000,
	    6   // 점 0b00000110
	},
	{
		0b01110000,	//2
		0b10001000,
		0b00001000,
		0b00010000,
		0b00100000,
		0b01000000,
		0b11111000,
	    6
	},
	{
		0b11111000,	//3
	    0b00010000,
		0b00100000,
		0b00010000,
		0b00001000,
		0b10001000,
		0b01110000,
	    6
	},
	{
		0b00010000,	//4
		0b00110000,
		0b01010000,
		0b10010000,
		0b11111000,
		0b00010000,
		0b00010000,
	    6
	},
	{
		0b11111000,	//5
		0b10000000,
		0b11110000,
		0b00001000,
		0b00001000,
		0b10001000,
		0b01110000,
	    6
	},
	{
		0b00110000,	//6
		0b01000000,
		0b10000000,
		0b11110000,
		0b10001000,
		0b10001000,
		0b01110000,
	    6
	},
	{
		0b11111000,	//7
		0b10001000,
		0b00001000,
		0b00010000,
		0b00100000,
		0b00100000,
		0b00100000,
	    6
	},
	{
		0b01110000,	//8
		0b10001000,
		0b10001000,
		0b01110000,
		0b10001000,
		0b10001000,
		0b01110000,
	    6
	},
	{
		0b01111010,
		0b00001010,
		0b00001010,
		0b00110010,
		0b01000010,
		0b01111110,
		0b01000010,
		0b01111110
	},
	{
		0b00000000,    // hart
		0b01100110,
		0b11111111,
		0b11111111,
		0b11111111,
		0b01111110,
		0b00111100,
		0b00011000
	}
};

unsigned char display_data[8];  // 최종 8x8 출력할 데이터
unsigned char scroll_buffer[50][8] = {0};  // 스코롤할 데이타가 들어있는 버퍼
int number_of_character = 3;  // 출력할 문자 갯수

// 초기화 작업
// 1. display_data에 number_data[0]에 있는 내용 복사
// 2. number_data를 scroll_buffer에 복사
// 3. dotmatrix의 led를 off
void init_dotmatrix(void)
{
	for (int i=0; i < 8; i++)
	{
		display_data[i] = name[i];
	}
	for (int i=1; i < number_of_character+1; i++)
	{
		for (int j=0; j < 8; j++) // scroll_buffer[0] = blank
		{
			scroll_buffer[i][j] = name[i-1][j];
		}
	}
}

// scroll 문자 출력 프로그램
int dotmatrix_main(void)
{
	static int count=0;  // 컬럼 count
	static int index=0;  // scroll_buffer의 2차원 index값
	static uint32_t past_time=0;  // 이전 tick값 저장


	uint32_t now = HAL_GetTick();  // 1ms
	// 1.처음시작시 past_time=0; now: 500 --> past_time=500
	if (now - past_time >= 500) // 500ms scroll
	{
		past_time = now;
		for (int i=0; i < 8; i++)
		{

			display_data[i] = (scroll_buffer[index][i] >> count) |
					(scroll_buffer[index+1][i] << 8 - count);
		}
		if (++count == 8) // 8칼람을 다 처리 했으면 다음 scroll_buffer로 이동
		{
			count =0;
			index++;  // 다음 scroll_buffer로 이동
			if (index == number_of_character+1) index=0;
			// 11개의 문자를 다 처리 했으면 0번 scroll_buffer를 처리 하기위해 이동
		}
	}
/*
 		0b00000000,    // hart
		0b01100110,
		0b11111111,
		0b11111111,
		0b11111111,
		0b01111110,
		0b00111100,
		0b00011000
 */
	for (int i=0; i < 8; i++)
	{
		// 공통 양극 방식
		// column에는 0을 ROW에는 1을 출력해야 해당 LED가 on된다.
		col[0] = ~(1 << i);  // 00000001  --> 11111110
		col[1] = display_data[i];
		HAL_SPI_Transmit(&hspi2, col, 2, 1);
		GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
		GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up
		HAL_Delay(1);
	}

}

// scroll 문자 출력 프로그램
int dotmatrix_main_func(void)
{
	static int count=0;  // 컬럼 count
	static int index=0;  // scroll_buffer의 2차원 index값
	static uint32_t past_time=0;  // 이전 tick값 저장

	init_dotmatrix();

	while(1)
	{
		uint32_t now = HAL_GetTick();  // 1ms
		// 1.처음시작시 past_time=0; now: 500 --> past_time=500
		if (now - past_time >= 200) // 500ms scroll
		{
			past_time = now;
			for (int i=0; i < 8; i++)
			{

				display_data[i] = (scroll_buffer[index][i] >> count) |
						(scroll_buffer[index+1][i] << 8 - count);
			}
			if (++count == 8) // 8칼람을 다 처리 했으면 다음 scroll_buffer로 이동
			{
				count =0;
				index++;  // 다음 scroll_buffer로 이동
				if (index == number_of_character+1) index=0;
				// 11개의 문자를 다 처리 했으면 0번 scroll_buffer를 처리 하기위해 이동
			}
		}


		for (int i=0; i < 8; i++)
		{
			// 공통 양극 방식
			// column에는 0을 ROW에는 1을 출력해야 해당 LED가 on된다.
			col[0] = ~(1 << i);  // 00000001  --> 11111110
			col[1] = display_data[i];
			HAL_SPI_Transmit(&hspi2, col, 2, 1);
			GPIOB->ODR &= ~GPIO_PIN_13;   // latch핀을 pull-down
			GPIOB->ODR |= GPIO_PIN_13;   // latch핀을 pull-up
			HAL_Delay(1);
		}
	}
	return 0;
}


#endif

