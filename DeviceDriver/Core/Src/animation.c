/*
 * animation.c
 *
 *  Created on: Apr 3, 2025
 *      Author: microsoft
 */


#include "animation.h"
#include "extern.h"
#include "def.h"
#include "dotmatrix.h"
#include "button.h"
#include "spi.h"

uint32_t play_ani_sec = 0;
uint8_t ani_mode = NAME_ANI;
uint8_t pre_ani_mode = NAME_ANI;
void init_animaion() {

	spi_init();
	button_init();


	init_dotmatrix(ani_mode);
}

void run_animation() {

	init_animaion();

	while(true) {

		tick_animation();
		update_animation();

		render_animation();

	}
}

void tick_animation() {

	input_animation();

	if(TIM11_1ms_counter > 1000) {

		TIM11_1ms_counter = 0;

		if(ani_mode != NAME_ANI) {

			play_ani_sec++;
		}

	}
}

void update_animation() {

	uint8_t chage_mode = !(ani_mode == pre_ani_mode);

	if(ani_mode != NAME_ANI) {

		if(play_ani_sec >= MAX_PLAY_SEC) {
			play_ani_sec = 0;
			ani_mode = NAME_ANI;
			chage_mode = TRUE;
		}
	}

	if(!chage_mode) {
		return;
	}


	init_dotmatrix(ani_mode);
	pre_ani_mode = ani_mode;
}

void render_animation() {

	render_dotmatrix(ani_mode);
}

void input_animation() {

	if(get_button(GPIOC, GPIO_PIN_0, BTN0) == BUTTON_PRESS) {

		if(ani_mode == NAME_ANI) {
			ani_mode = DOWN_ARROW_ANI;
		}
		else {
			ani_mode = (ani_mode == DOWN_ARROW_ANI) ? UP_ARROW_ANI : DOWN_ARROW_ANI;
			play_ani_sec = 0;
		}
	}
}
