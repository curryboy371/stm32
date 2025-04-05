/*
 * animation.h
 *
 *  Created on: Apr 3, 2025
 *      Author: microsoft
 */

#ifndef INC_ANIMATION_H_
#define INC_ANIMATION_H_
#include "main.h"

#define NAME_ANI  		0
#define UP_ARROW_ANI	1
#define DOWN_ARROW_ANI	2

#define MAX_PLAY_SEC   30

void init_animaion();

void run_animation();

void tick_animation();

void update_animation();

void render_animation();

void input_animation();

#endif /* INC_ANIMATION_H_ */
