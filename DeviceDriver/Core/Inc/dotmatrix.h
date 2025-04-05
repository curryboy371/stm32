/*
 * dotmatrix.c
 *
 *  Created on: Apr 3, 2025
 *      Author: microsoft
 */

#ifndef INC_DOTMATRIX_C_
#define INC_DOTMATRIX_C_

#include "main.h"

void init_dotmatrix(uint8_t inCurMode);

void render_dotmatrix(uint8_t inCurMode);


void shift_register_latch();
void dotmatrix_main_test();


#endif /* INC_DOTMATRIX_C_ */
