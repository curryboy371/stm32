/*
 * dotmatrix.c
 *
 *  Created on: Apr 3, 2025
 *      Author: microsoft
 */

#ifndef INC_DOTMATRIX_C_
#define INC_DOTMATRIX_C_

#include "main.h"

typedef struct {

	uint8_t cur_index;
	uint8_t next_index;

}t_dotmatrix_info;


#define IDLE_ANI  	0
#define UP_ANI		1
#define DOWN_ANI	2


void init_dotmatrix_pin();
void init_dotmatrix(uint8_t inCurMode);

void render_dotmatrix();

void dotmatrix_refresh_idx(uint8_t cur_idx, uint8_t next_idx);

void dotmatrix_main_test();


#endif /* INC_DOTMATRIX_C_ */
