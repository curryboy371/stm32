/*
 * elevator.c
 *
 *  Created on: Apr 9, 2025
 *      Author: microsoft
 */

#include "elevator.h"

#include "extern.h"
#include "def.h"
#include "timer.h"

#include "stepmotor.h"
#include "button.h"

#include "i2c_lcd.h"
#include "led.h"
#include "fnd.h"
//#include "animation.h"
#include "dotmatrix.h"
#include "buzzer.h"
#include "ds1302.h"

#include <stdio.h>
#include <string.h>

t_elv_info elv_info;

extern t_dotmatrix_info dotmatrix_info;



void elevator_init() {

	elv_info.cur_floor = ELV_1F;
	elv_info.obj_floor = ELV_NONE;

	elv_info.cur_input_state = ELV_NONE;
	elv_info.cur_move_dir = ELV_MOVE_IDLE;

}


t_elv_floor elevator_input() {

	t_elv_floor input_floor = ELV_NONE;

	// input check
	if(get_button(GPIOC, GPIO_PIN_0, BTN0) == BUTTON_PRESS) {
		input_floor = ELV_1F;

	}
	if(get_button(GPIOC, GPIO_PIN_1, BTN1) == BUTTON_PRESS) {
		input_floor = ELV_2F;
	}

	if(get_button(GPIOC, GPIO_PIN_2, BTN2) == BUTTON_PRESS) {
		input_floor = ELV_3F;
	}

	if(get_button(GPIOC, GPIO_PIN_3, BTN3) == BUTTON_PRESS) {
		input_floor = ELV_4F;
	}

	// 아래로
/*
	if(get_button(GPIOA, GPIO_PIN_13, BTN4) == BUTTON_PRESS) {
		stepmotor_set_state(SM_STATE_BACKWARD);
	}
*/

	return input_floor;



}

void elevator_set_floor_state(t_elv_floor input_floor, uint8_t force_off) {

	if(input_floor == ELV_NONE) {
		return;
	}

	if(force_off == TRUE) {
		elv_info.cur_input_state &= ~input_floor;
	}
	else {
		// 버튼이 눌려있으면 제거, 그렇지 않으면 누름
		if(elv_info.cur_input_state & input_floor) {
			elv_info.cur_input_state &= ~input_floor;

			buzzer_off_button();

			printf("btn off %x \r\n", input_floor);
		}
		else {

			// 첫번째 버튼일때 방향 설정
			if(elv_info.cur_input_state == ELV_NONE) {

				if(input_floor > elv_info.cur_floor) {
					elevator_set_dir(ELV_MOVE_UP);
				}
				else if(input_floor < elv_info.cur_floor) {
					elevator_set_dir(ELV_MOVE_DOWN);
				}
				else {
					elevator_set_dir(ELV_MOVE_IDLE);
				}
			}

			elv_info.cur_input_state |= input_floor;
			buzzer_on_button();
			printf("btn on %x \r\n", input_floor);

		}
	}
}


void elevator_tick() {

	elevator_set_floor_state(elevator_input(), FALSE);
}

void elevator_update() {

	elevator_tick();
	buzzer_update();
	ds1302_update();
	elevator_update_floor();

}

void elevator_run() {

	stepmotor_run();

	ledbar_render(elv_info.cur_input_state);


	fnd_render((uint8_t)elv_info.cur_move_dir, elevator_get_floor2int(elv_info.cur_floor));

}


void elevator_print() {


	uint8_t input_state[MAX_ELV_FLOOR] = {0};
	elevator_get_state2arr(input_state, elv_info.cur_input_state);


	static char buffer[40] = {0};

	const t_ds1302* pds1302 = ds1302_get_dsinfo();

	if(pds1302) {

		sprintf(buffer, "20%d-%02d-%02d", pds1302->year, pds1302->month, pds1302->date);
		move_cursor(0,0);
		lcd_string((char*)buffer);

		sprintf(buffer, "%02d:%02d:%02d", pds1302->hours, pds1302->minutes, pds1302->seconds);
		move_cursor(1,0);
		lcd_string((char*)buffer);
	}
}

void elevator_update_floor() {

	if(elv_info.pre_floor != elv_info.cur_floor) {

		uint8_t cur_floor_idx = elevator_get_floor2int(elv_info.cur_floor) -1;

		if(elv_info.cur_move_dir == ELV_MOVE_UP) {
			dotmatrix_refresh_idx(cur_floor_idx, cur_floor_idx+1);
		}
		else if(elv_info.cur_move_dir == ELV_MOVE_IDLE){
			dotmatrix_refresh_idx(cur_floor_idx, cur_floor_idx+1);
		}
		else {
			dotmatrix_refresh_idx(cur_floor_idx+1, cur_floor_idx);
		}


		elv_info.pre_floor = elv_info.cur_floor;
	}

	t_elv_floor obj_floor = elevator_get_close_floor(&elv_info);

	// 입력이 없음
	if(obj_floor == ELV_NONE) {
		if(elv_info.obj_floor != ELV_NONE) {
			elv_info.obj_floor = obj_floor;
			stepmotor_set_state(SM_STATE_IDLE);

			elevator_set_floor_state(ELV_ALL_F, TRUE);
			printf("set idle\r\n");
		}
		return;
	}

	// 새로운 input인 경우
	elv_info.obj_floor = obj_floor;

	// 정지 상태의 경우에만 방향 설정 가능
	if(stepmotor_is_idle() == TRUE) {
		// 상승 하강 체크

		if(elv_info.obj_floor > elv_info.cur_floor) {
			elevator_set_dir(ELV_MOVE_UP);
		}
		else if(elv_info.obj_floor < elv_info.cur_floor) {
			elevator_set_dir(ELV_MOVE_DOWN);
		}
		else {
			elevator_set_dir(ELV_MOVE_IDLE);
		}

	}

	// 도착 여부
	if(elv_info.cur_floor & elv_info.obj_floor) {
		// 강제로 off
		elevator_set_floor_state(elv_info.cur_floor, TRUE);
		TIM11_1ms_delay_counter[MODULE_STEPMOTOR] = 1000;

		// 도착노래
		buzzer_welcom_play();


		// 버튼 눌린게 없으면 stop
		if(elv_info.cur_input_state == ELV_NONE) {

			elevator_set_dir(ELV_MOVE_IDLE);
		}
	}
}

t_elv_floor elevator_get_close_floor(t_elv_info* elv_info) {

	if(!elv_info) {
		return ELV_NONE;
	}

	// 입력이 없음
	if(elv_info->cur_input_state == ELV_NONE) {
		return ELV_NONE;
	}

	t_elv_floor floor = elv_info->cur_floor;
	t_elv_floor obj_floor = ELV_NONE;

	// 현재층 정보가 없으면 방향에 맞춰 min, max로
	if(floor == ELV_NONE) {
		floor = (elv_info->cur_move_dir == ELV_MOVE_UP) ? ELV_1F : ELV_4F;
	}



	int startfloor = elevator_get_floor2int(floor);

	// 올라가는 상황
	if(elv_info->cur_move_dir == ELV_MOVE_UP) {

	    for (int i = startfloor; i <= MAX_ELV_FLOOR; ++i) {
	        uint8_t mask = 1 << (i - 1); // 현재 층에 해당하는 비트 마스크

	        if (elv_info->cur_input_state & mask) {
	            obj_floor = mask;
	            break;
	        }
	    }
	}
	else { // 내려가는 상황

	    for (int i = startfloor; i >= MIN_ELV_FLOOR; --i) {
	        uint8_t mask = 1 << (i - 1); // 현재 층에 해당하는 비트 마스크

	        if (elv_info->cur_input_state & mask) {
	            obj_floor = mask; // 해당 층이 요청되었으면 목적층으로 설정
	            break;
	        }
	    }
	}
	return obj_floor;
}

void elevator_set_dir(t_elv_floor new_dir) {

	uint8_t cur_floor_idx = elevator_get_floor2int(elv_info.cur_floor) -1;

	switch(new_dir) {

	case ELV_MOVE_UP:
		elv_info.cur_move_dir = new_dir;

		stepmotor_set_state(SM_STATE_FORWARD);
		init_dotmatrix(UP_ANI);
		dotmatrix_refresh_idx(cur_floor_idx, cur_floor_idx+1);
		break;

	case ELV_MOVE_DOWN:
		elv_info.cur_move_dir = new_dir;
		init_dotmatrix(DOWN_ANI);
		stepmotor_set_state(SM_STATE_BACKWARD);
		dotmatrix_refresh_idx(cur_floor_idx+1, cur_floor_idx);
		break;


	case ELV_MOVE_IDLE:
		elv_info.cur_move_dir = new_dir;
		stepmotor_set_state(SM_STATE_IDLE);
		dotmatrix_refresh_idx(cur_floor_idx, cur_floor_idx+1);
		init_dotmatrix(IDLE_ANI);
		break;

	default:
		break;
	}
}

void elevator_set_floor(t_elv_floor new_floor) {

	elv_info.cur_floor = new_floor;

}

uint8_t elevator_get_floor2int(t_elv_floor cur_floor) {

	uint8_t ifloor = 0;


	t_elv_floor floor = ELV_1F;
	for(int i = 0; i < MAX_ELV_FLOOR; ++i) {

		if(cur_floor & floor << i) {
			ifloor = i+1;
		}
	}
	return ifloor;
}

void elevator_get_state2arr(uint8_t out_states[], t_elv_floor floor_state) {

	t_elv_floor floor = ELV_1F;
	for(int i = 0; i < MAX_ELV_FLOOR; ++i) {

		if(floor_state & floor << i) {
			out_states[i] = 1;
		}

	}
}
