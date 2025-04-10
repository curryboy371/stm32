/*
 * stepmotor.c
 *
 * RPM ( Revolutions per minutes : 분당 회전 수
 * 1분 : 60sec
 * 1초 : 1sec(1,000ms 1,000,000us)
 * 60초: 1,000,000us * 60 = 60,000,000us
 *
 * 1바퀴 회전 : 4096 step 필요
 * 4096 / 8 ==> 512 sequence : 360도 회전
 * 1 sequence (8 step ) : 0.70312도
 * 0.70312 * 512 sequence : 360도 회전
 *
 * ------- RPM 조절 --------
 * ex) 1분에 13회전이 최대 속도
 * 13회전 : 60,000,000us(1minute) / 4096 / 13 ==> step과 step간의 간격 time : 1126us
 * 1126 us * 4096 ( 1회전 하는데 필요 스텝 ) = 4,615,384(us) = 4624ms(4.6초)
 *
 * 60sec / 4.6초(1회전 하는데 소요되는 시간 ) 13회전
 *
 *
 *  Created on: Apr 8, 2025
 *      Author: microsoft
 */

#include "stepmotor.h"
#include "extern.h"
#include "def.h"
#include "timer.h"
#include <stdio.h>

#include "button.h"


t_setep_motor_state stepmotor_state = SM_STATE_IDLE;

void stepmotor_main() {

	switch(stepmotor_state) {

	case SM_STATE_IDLE:
		if(get_button(GPIOC, GPIO_PIN_0, BTN0) == BUTTON_PRESS) {
			//stepmotor_state = SM_STATE_FORWARD;
		}
		break;

	case SM_STATE_FORWARD:
		stepmotor_drive(FOWARD_DIR);
		set_rpm(13);

		if(get_button(GPIOC, GPIO_PIN_1, BTN1) == BUTTON_PRESS) {
			//stepmotor_state = SM_STATE_BACKWARD;
		}

		if(get_button(GPIOC, GPIO_PIN_0, BTN0) == BUTTON_PRESS) {
			//stepmotor_state = SM_STATE_IDLE;
		}


		break;

	case SM_STATE_BACKWARD:

		stepmotor_drive(BACKWARD_DIR);
		set_rpm(13);

		if(get_button(GPIOC, GPIO_PIN_1, BTN1) == BUTTON_PRESS) {
			//stepmotor_state = SM_STATE_FORWARD;
		}

		if(get_button(GPIOC, GPIO_PIN_0, BTN0) == BUTTON_PRESS) {
			//stepmotor_state = SM_STATE_IDLE;
		}

		break;

	default:
		break;
	}

}

void stepmotor_demo() {

	while(true) {


		for(int i = 0; i < 512; ++i) { // 시계방향 1회전

			for(int j =0; j < 8; ++j) { // 1sequence : 8 step

				stepmotor_drive(j);
				set_rpm(13); // 최대 속도 ( 1126 us 동안 wait )

			}
		}

		HAL_Delay(500);

		for(int i = 0; i < 512; ++i) { // 반시계방향 1회전

			for(int j =7; j >= 0; --j) { // 1sequence : 8 step

				stepmotor_drive(j);
				set_rpm(13); // 최대 속도 ( 1126 us 동안 wait )
			}
		}

	}


}

void stepmotor_set_state(t_setep_motor_state new_state) {

	if(stepmotor_state == new_state) {
		return;
	}

	stepmotor_state = new_state;

}

uint8_t stepmotor_is_idle() {

	return stepmotor_state == SM_STATE_IDLE;
}

void stepmotor_run() {


	if(TIM11_1ms_delay_counter[MODULE_STEPMOTOR] > 0) {
		return;
	}

	if(stepmotor_state == SM_STATE_IDLE) {
		return;
	}

	// todo 여기서 조건문 없애야함..

	// todo 이부분은 rtos 고려
	if(stepmotor_state == SM_STATE_FORWARD) {

		stepmotor_drive(FOWARD_DIR);
		set_rpm(13);
	}
	else {
		stepmotor_drive(BACKWARD_DIR);
		set_rpm(13);
	}

}

void set_rpm(int rpm) {

	// delay를 주는 만큼 스피드가 설정됨
	//delay_us(60'000'000 / 4096 / rpm);
	delay_us(60000000 / 4096 / rpm);
	//최대 speed기준 13 : 1126


}

void stepmotor_drive(int direction) {

	static int8_t step = 0;

	// 이렇게 쓸거면 둘은 같은 포트에서 써야함
	// 배열 메모리를 좀 더 써서 switch 조건문을 없앰
/*
	const static uint16_t arr[8] = {IN1_Pin,
									IN1_Pin|IN2_Pin,
									IN2_Pin,
									IN2_Pin|IN3_Pin,
									IN3_Pin,
									IN3_Pin|IN4_Pin,
									IN3_Pin,
									IN4_Pin|IN1_Pin,

	};
*/

	const static uint16_t arr[MAX_STEP] = {	IN1_Pin,
											IN1_Pin | IN2_Pin,
											IN2_Pin | IN3_Pin,
											IN3_Pin | IN4_Pin,
	};


	if(direction == FOWARD_DIR) {
		step = (step + 1) % MAX_STEP;
	}
	else {
		step = (step -1);
		if(step < 0) {
			step += MAX_STEP;
		}
	}


	GPIOC->ODR &= ~(IN1_Pin | IN2_Pin | IN3_Pin | IN4_Pin);
	GPIOC->ODR |= arr[step];


}




