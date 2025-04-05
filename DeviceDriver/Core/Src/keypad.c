#include "keypad.h"

GPIO_TypeDef* keypadRowPort[4] = {GPIOC, GPIOC, GPIOC, GPIOC}; //R1~R4
GPIO_TypeDef* keypadColPort[4] = {GPIOC, GPIOC, GPIOC, GPIOC}; //C1~C4
uint16_t keypadRowPin[4] = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7}; //R1~R4 GPIO Input & Pull-up으로 설정을 해야 한다.
uint16_t keypadColPin[4] = {GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11}; //C1~C4  GPIO Output으로만 설정 한다.

void keypadInit()
{
	for(uint8_t col = 0; col < 4; col++)
	{
		HAL_GPIO_WritePin(keypadColPort[col], keypadColPin[col], SET); //초기 값 1로 셋팅
	}
}

uint8_t getKeypadState(uint8_t col, uint8_t row)
{
#if 1
	uint8_t keypadChar[4][4] = {
			{'/', '3', '2', '1'},
			{'*', '6', '5', '4'},
			{'-', '9', '8', '7'},
			{'+', '=', '0', ' '},
	};

#else
	uint8_t keypadChar[4][4] = {
			{'1', '2', '3', '/'},
			{'4', '5', '6', '*'},
			{'7', '8', '9', '-'},
			{' ', '0', '=', '+'},
	};
#endif
	static uint8_t prevState[4][4] = {
			{1, 1, 1, 1},
			{1, 1, 1, 1},
			{1, 1, 1, 1},
			{1, 1, 1, 1},
	};
	uint8_t curState = 1;

	// 이 컬럼의 row를 읽기 위해서 이번 컬럼을 reset
	//
	HAL_GPIO_WritePin(keypadColPort[col], keypadColPin[col], RESET);

	// 읽을 컬럼을 on 했으므로 row를 읽을 수 있음
	// row는 read만 처리
	curState = HAL_GPIO_ReadPin(keypadRowPort[row], keypadRowPin[row]);

	// 다 읽었으므로 colum 되돌림
	HAL_GPIO_WritePin(keypadColPort[col], keypadColPin[col], SET);

	// 이부분은 getbutton과 동일
	// getbutton은 노이즈 처리를 위해 delay가 있었는데..
	// delay가 없음. 이유는 이 함수를 60ms마다 호출하기 때문에 딜레이가 없어도 동작함
	if(curState == PUSHED && prevState[col][row] == RELEASED)
	{
		// noise software 처리
		prevState[col][row] = curState;
		return 0;
	}
	else if (curState == RELEASED && prevState[col][row] == PUSHED)
	{
		// 버튼 눌림을 처리
		// 눌린 버튼에 저장된 char 값을 반환
		prevState[col][row] = curState;
		return keypadChar[col][row];
	}
	return 0;
}

uint8_t keypadScan()
{
	uint8_t data;

	for(uint8_t col=0; col<4; col++)
	{
		for(uint8_t row=0; row<4; row++)
		{
			data = getKeypadState(col, row);
			if(data != 0)
			{
				return data;
			}
		}
	}
	return 0;
}
