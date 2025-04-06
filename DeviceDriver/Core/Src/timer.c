/*
 * timer.c
 *
 *  Created on: Mar 31, 2025
 *      Author: microsoft
 */


#include "timer.h"
#include "extern.h"

volatile uint32_t TIM11_1ms_tick;

// 1MHz의 주파수가 TIM2에 공급
// T=1/f 1/1,000,000Hz ==> 0.000001sec ( 1us) 1개의 pulse 소요 시간
// 1us * 1000 ==> 1ms
void delay_us(int us) {

	// 근데 이 함수가 왜 필요할까?
	// 이렇게 딜레이하면 인터럽트는 계속 발생하겠네?


	// 1. timer 2번의 counter reset
	__HAL_TIM_SET_COUNTER(&htim2, 0);


	// 2. 사용자가 지정한 시간만큼 wait
	while(__HAL_TIM_GET_COUNTER(&htim2) < us) {
		;
	}


}

void delay_ms(int ms) {

	// 근데 이 함수가 왜 필요할까?
	// 이렇게 딜레이하면 인터럽트는 계속 발생하겠네?


	// 1. timer 2번의 counter reset
	__HAL_TIM_SET_COUNTER(&htim2, 0);

	// 2. 사용자가 지정한 시간만큼 wait
	while(__HAL_TIM_GET_COUNTER(&htim2) < ms * 1000) {
		;
	}
}

uint32_t get_tick() {

	return TIM11_1ms_tick;
}

