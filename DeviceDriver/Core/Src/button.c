/*
 * buton.c
 *
 *  Created on: Mar 26, 2025
 *      Author: microsoft
 */


#include "button.h"
#include "def.h"
void button_init() {

	*GPIO_PUPDR |= (1 << BTN3 * 2) | (1 << BTN2 * 2) | (1 << BTN1 * 2) | (1 << BTN0 * 2);
}
void button_led_toggle_test(void){

	// button을 한번 눌렀다 떼면 led가 toggle되도록

	if(get_button(GPIOC, GPIO_PIN_0, BTN0) == BUTTON_PRESS) {

		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
	}
	if(get_button(GPIOC, GPIO_PIN_1, BTN1) == BUTTON_PRESS) {

		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
	}
	if(get_button(GPIOC, GPIO_PIN_2, BTN2) == BUTTON_PRESS) {

		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
	}
	if(get_button(GPIOC, GPIO_PIN_3, BTN3) == BUTTON_PRESS) {

		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
	}
	if(get_button(GPIOC, GPIO_PIN_13, BTN4) == BUTTON_PRESS) {
		// Demo b'd
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	}

}






// param1 BUTTON0 : s/w 번호
// param2 BUTTON0PIN : button pin num
// return 1 버튼을 눌렀다 뗌
//		  0 idle 상태
// 누르고 있으면?
int get_button(GPIO_TypeDef* GPIO, int GPIO_Pin, int button_num) {


	// 기본 버튼도 여기에 넣어야하는데...

	// 이 함수 내에서만 유지해서 사용하므로 전역변수 말고 static 사용
	static unsigned char button_status[5] =
	{
		BUTTON_RELEASE, BUTTON_RELEASE, BUTTON_RELEASE, BUTTON_RELEASE, BUTTON_RELEASE
	};

#if ACCESS_MODE == USE_HAL
	int currtn_state = HAL_GPIO_ReadPin(GPIO, GPIO_Pin);
#elif ACCESS_MODE == USE_CONST

	int currtn_state = (GPIO->IDR & GPIO_Pin) ? BUTTON_RELEASE : BUTTON_PRESS;

#elif ACCESS_MODE == USE_STRUCT
	int currtn_state = (GPIO->IDR & GPIO_Pin) ? BUTTON_RELEASE : BUTTON_PRESS;
#endif

	// 현재 버튼의 상태를 받음



	// 현재 버튼이 눌린경우
	// 처음 버튼이 눌린경우
	if(currtn_state == BUTTON_PRESS && button_status[button_num] == BUTTON_RELEASE) // 버튼이 처음 눌림 noise high
	{
		HAL_Delay(60);
		button_status[button_num] = BUTTON_PRESS; // noise가 지나간 상태의 High
		return BUTTON_RELEASE; // 아직은 완전히 눌렸다 떼어진 상태가 아님
	}
	else if(currtn_state == BUTTON_RELEASE && button_status[button_num] == BUTTON_PRESS) // 현재는 버튼을 떼고, 기존은 누르는 상태
	{
		HAL_Delay(60);
		button_status[button_num] = BUTTON_RELEASE; // 다음 버튼 체크를 위한 초기화
		return BUTTON_PRESS; // 완전히 1번 누르고 떼어진 상태
	}




	return BUTTON_RELEASE;

}
