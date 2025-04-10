/*
 * stepmotor.h
 *
 *  Created on: Apr 8, 2025
 *      Author: microsoft
 */





#ifndef INC_STEPMOTOR_H_
#define INC_STEPMOTOR_H_

#include "main.h"
#include "extern.h"


#define IDLE_DIR 0
#define FOWARD_DIR 1
#define BACKWARD_DIR -1


#define MAX_STEP 4


void stepmotor_main();

// step motor demo
void stepmotor_demo();

void stepmotor_set_state(t_setep_motor_state new_state);

uint8_t stepmotor_is_idle();

void stepmotor_run();


// rpm 값을 세팅 : 1~13
void set_rpm(int rpm);

void stepmotor_drive(int direction);






#endif







