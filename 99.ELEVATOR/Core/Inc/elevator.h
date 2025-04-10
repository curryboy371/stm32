/*
 * elevator.h
 *
 *  Created on: Apr 9, 2025
 *      Author: microsoft
 */

#ifndef INC_ELEVATOR_H_
#define INC_ELEVATOR_H_

#include "main.h"


typedef enum {

	ELV_MOVE_UP,
	ELV_MOVE_DOWN,
	ELV_MOVE_IDLE

} t_elv_dir;

typedef enum {
	ELV_NONE = 0x00,
	ELV_1F = 0x01, 	// 0001
	ELV_2F = 0x02, 	// 0010
	ELV_3F = 0x04, 	// 0100
	ELV_4F = 0x08, 	// 1000
	ELV_ALL_F = ELV_1F | ELV_2F | ELV_3F | ELV_4F
} t_elv_floor;

typedef struct {

	t_elv_dir 	cur_move_dir; // 현재 dir
	t_elv_floor pre_floor; // 이전 floor
	t_elv_floor cur_floor; // 현재 floor
	t_elv_floor obj_floor; // 목표 floor

	t_elv_floor cur_input_state; // 버튼 input state

}t_elv_info;

#define MAX_ELV_FLOOR 4
#define MIN_ELV_FLOOR 1


void elevator_init();

t_elv_floor elevator_input();
void elevator_set_floor_state(t_elv_floor input_floor, uint8_t force_off);
void elevator_update();
void elevator_tick();
void elevator_run();

void elevator_print();

void elevator_update_floor();

t_elv_floor elevator_get_close_floor(t_elv_info* elv_info);


void elevator_set_dir(t_elv_floor new_dir);
void elevator_set_floor(t_elv_floor new_floor);

uint8_t elevator_get_floor2int(t_elv_floor cur_floor);
void elevator_get_state2arr(uint8_t out_states[], t_elv_floor floor_state);

#endif /* INC_ELEVATOR_H_ */



