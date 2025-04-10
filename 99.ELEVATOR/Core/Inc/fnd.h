/*
 * fnd.h
 *
 *  Created on: Apr 9, 2025
 *      Author: microsoft
 */

#ifndef INC_FND_H_
#define INC_FND_H_

#include "main.h"

typedef enum FND_digit{
	FND_DIGIT_D4 = 0,		// [1][ ][ ][ ]
	FND_DIGIT_D3,			// [ ][1][ ][ ]
	FND_DIGIT_D2,			// [ ][ ][1][ ]
	FND_DIGIT_D1,			// [ ][ ][ ][1]
	FND_DIGIT_MAX
} t_FND_digit;

typedef struct {

	unsigned char d1;
	unsigned char d2;
	unsigned char d3;
	unsigned char d4;

}t_FND_Cycle;


#ifdef FND_COMMON_ANODE
#define FND_NUM_DOT		(uint8_t)0x7f
#define FND_NUM_0		(uint8_t)0xc0
#define FND_NUM_1		(uint8_t)0xf9
#define FND_NUM_2		(uint8_t)0xa4
#define FND_NUM_3		(uint8_t)0xb0
#define FND_NUM_4		(uint8_t)0x99
#define FND_NUM_5		(uint8_t)0x92
#define FND_NUM_6		(uint8_t)0x82
#define FND_NUM_7		(uint8_t)0xd8
#define FND_NUM_8		(uint8_t)0x80
#define FND_NUM_9		(uint8_t)0x90

#define FND_NULL		(uint8_t)~0b00000000
#define FND_A			(uint8_t)~0b00000001
#define FND_B			(uint8_t)~0b00000010
#define FND_C			(uint8_t)~0b00000100
#define FND_D			(uint8_t)~0b00001000
#define FND_E			(uint8_t)~0b00010000
#define FND_F			(uint8_t)~0b00100000
#define FND_G			(uint8_t)~0b01000000
#else
#define FND_NUM_DOT		(uint8_t)~0x7f
#define FND_NUM_0		(uint8_t)~0xc0
#define FND_NUM_1		(uint8_t)~0xf9
#define FND_NUM_2		(uint8_t)~0xa4
#define FND_NUM_3		(uint8_t)~0xb0
#define FND_NUM_4		(uint8_t)~0x99
#define FND_NUM_5		(uint8_t)~0x92
#define FND_NUM_6		(uint8_t)~0x82
#define FND_NUM_7		(uint8_t)~0xd8
#define FND_NUM_8		(uint8_t)~0x80
#define FND_NUM_9		(uint8_t)~0x90

#define FND_NULL		(uint8_t)0b00000000
#define FND_A			(uint8_t)0b00000001
#define FND_B			(uint8_t)0b00000010
#define FND_C			(uint8_t)0b00000100
#define FND_D			(uint8_t)0b00001000
#define FND_E			(uint8_t)0b00010000
#define FND_F			(uint8_t)0b00100000
#define FND_G			(uint8_t)0b01000000
#endif

#define FND_S_VAR	FND_A | FND_F | FND_G | FND_C | FND_D
#define FND_T_VAR	FND_A | FND_B | FND_C
#define FND_O_VAR	FND_NUM_0
#define FND_P_VAR	FND_A | FND_B | FND_G | FND_F | FND_E


#define FND_A_VAR  	FND_A | FND_F | FND_B | FND_E | FND_C | FND_G
#define FND_B_VAR  	FND_NUM_8



void fnd_init();


void fnd_set_digit_port(t_FND_digit select_digit);

uint8_t fnd_get_digit_num(uint8_t num);

void fnd_render(uint8_t dir, uint8_t cur_floor);

void fnd_stop_render(t_FND_digit select_digit);
void fnd_move_render(t_FND_digit select_digit, uint8_t cur_floor);

void fnd_null_render(t_FND_digit select_digit);
void fnd_render_digit(t_FND_digit select_digit, uint8_t num);


#endif /* INC_FND_H_ */
