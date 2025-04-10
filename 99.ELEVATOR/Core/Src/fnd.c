/*
 * fnd.c
 *
 *  Created on: Apr 9, 2025
 *      Author: microsoft
 */


#include "fnd.h"

#include "extern.h"
#include "def.h"
#include "spi.h"
#include "elevator.h"

uint16_t fnd_digitpin_arr[FND_DIGIT_MAX];
GPIO_TypeDef* fnd_digitport_arr[FND_DIGIT_MAX];


t_spi_gpio_setting fnd_spi_gpio;


void fnd_init() {

	//spi pin

	fnd_spi_gpio.gpio_port[SPI_PIN_DS] = FND_DS_GPIO_Port;
	fnd_spi_gpio.gpio_port[SPI_PIN_SH] = FND_SH_GPIO_Port;
	fnd_spi_gpio.gpio_port[SPI_PIN_ST] = FND_ST_GPIO_Port;

	fnd_spi_gpio.gpio_pin[SPI_PIN_DS] =  FND_DS_Pin;
	fnd_spi_gpio.gpio_pin[SPI_PIN_SH] =  FND_SH_Pin;
	fnd_spi_gpio.gpio_pin[SPI_PIN_ST] =  FND_ST_Pin;

	spi_init(&fnd_spi_gpio);


	fnd_digitpin_arr[FND_DIGIT_D4] = FND_DIGIT4_Pin;
	fnd_digitpin_arr[FND_DIGIT_D3] = FND_DIGIT3_Pin;
	fnd_digitpin_arr[FND_DIGIT_D2] = FND_DIGIT2_Pin;
	fnd_digitpin_arr[FND_DIGIT_D1] = FND_DIGIT1_Pin;

	fnd_digitport_arr[FND_DIGIT_D4] = FND_DIGIT4_GPIO_Port;
	fnd_digitport_arr[FND_DIGIT_D3] = FND_DIGIT3_GPIO_Port;
	fnd_digitport_arr[FND_DIGIT_D2] = FND_DIGIT2_GPIO_Port;
	fnd_digitport_arr[FND_DIGIT_D1] = FND_DIGIT1_GPIO_Port;


	// digit num port init
	for(int i = 0; i < FND_DIGIT_MAX; ++i) {

		HAL_GPIO_WritePin(fnd_digitport_arr[i], fnd_digitpin_arr[i], RESET);
	}

}

void fnd_set_digit_port(t_FND_digit select_digit) {


	FlagStatus digit_on = SET;
	FlagStatus digit_off = RESET;

#ifdef FND_COMMON_ANODE
	digit_on = SET;
	digit_off = RESET;

#else
	digit_on = RESET;
	digit_off = SET;
#endif
	for(int i = 0; i < FND_DIGIT_MAX; ++i) {

		HAL_GPIO_WritePin(fnd_digitport_arr[i], fnd_digitpin_arr[i], digit_off);
	}

	HAL_GPIO_WritePin(fnd_digitport_arr[select_digit], fnd_digitpin_arr[select_digit], digit_on);

}

// param inDigitNum 0~9 숫자 입력
// return fnd 모드에 따른 fnd number value 출력
uint8_t fnd_get_digit_num(uint8_t num)
{
	static const uint8_t fnd_font[10] = {
		FND_NUM_0, FND_NUM_1, FND_NUM_2, FND_NUM_3, FND_NUM_4,
		FND_NUM_5, FND_NUM_6, FND_NUM_7, FND_NUM_8, FND_NUM_9};

    if (num >= 10)
	{
	    return FND_NUM_0;
    }

	return fnd_font[num];
}

void fnd_render(uint8_t dir, uint8_t cur_floor) {

	static uint8_t is_clear = FALSE;

	is_clear = !is_clear;

	static t_FND_digit digit = FND_DIGIT_D4;

	if(is_clear) {
		fnd_null_render(digit);
	}
	else {

		digit = (digit + 1) % FND_DIGIT_MAX;

		if(dir == ELV_MOVE_DOWN) {
			fnd_move_render(digit, cur_floor);
		}
		else if(dir == ELV_MOVE_UP) {
			fnd_move_render(digit, cur_floor);
		}
		else {
			fnd_stop_render(digit);
		}
	}
}

void fnd_stop_render(t_FND_digit select_digit) {


	static uint8_t datas[4] = {FND_P_VAR, FND_O_VAR, FND_T_VAR, FND_S_VAR} ;

	fnd_set_digit_port(select_digit);
	spi_tx_data(&fnd_spi_gpio, datas[select_digit]);
	spi_letch(&fnd_spi_gpio);
}

void fnd_null_render(t_FND_digit select_digit) {

	fnd_set_digit_port(select_digit);
	spi_tx_data(&fnd_spi_gpio, FND_NULL);
	spi_letch(&fnd_spi_gpio);
}

void fnd_move_render(t_FND_digit select_digit, uint8_t cur_floor) {

	// 배열에 cycle을 모두 저장

	static int idx = 0;

	idx = (idx + 1) % 120;

	// 배열에 cycle을 모두 저장
	const static t_FND_Cycle FND_Cycle[12] = {

		//d1								d2								 d3										 d4
		{ FND_NULL,							FND_NULL,						FND_NULL,								FND_A,							},
		{ FND_NULL,							FND_NULL,						FND_A,									FND_A,							},
		{ FND_NULL,							FND_A,							FND_A,									FND_A,							},
		{ FND_A ,							FND_A,							FND_A,									FND_A,							},
		{ FND_A | FND_F	,					FND_A,							FND_A,									FND_A,							},
		{ FND_A | FND_F | FND_E,			FND_A,							FND_A,									FND_A,							},
		{ FND_A | FND_F | FND_E | FND_D,	FND_A,							FND_A,									FND_A,							},
		{ FND_A | FND_F | FND_E | FND_D,	FND_A | FND_D,					FND_A,									FND_A,							},
		{ FND_A | FND_F | FND_E | FND_D,	FND_A | FND_D,					FND_A| FND_D,							FND_A,							},
		{ FND_A | FND_F | FND_E | FND_D,	FND_A | FND_D,					FND_A| FND_D,							FND_A | FND_D,					},
		{ FND_A | FND_F | FND_E | FND_D,	FND_A | FND_D,					FND_A| FND_D,							FND_A | FND_D | FND_C,			},
		{ FND_A | FND_F | FND_E | FND_D,	FND_A | FND_D,					FND_A| FND_D,							FND_A | FND_D | FND_C | FND_D,	},
	};


	fnd_set_digit_port(select_digit);


	int cycle_idx = idx* 0.1;

	switch(select_digit) {

	case FND_DIGIT_D4:
		spi_tx_data(&fnd_spi_gpio, FND_Cycle[cycle_idx].d4);
		break;
	case FND_DIGIT_D3:
		spi_tx_data(&fnd_spi_gpio, FND_Cycle[cycle_idx].d3);
		break;
	case FND_DIGIT_D2:
		spi_tx_data(&fnd_spi_gpio, FND_Cycle[cycle_idx].d2);
		break;
	case FND_DIGIT_D1:
		spi_tx_data(&fnd_spi_gpio, FND_Cycle[cycle_idx].d1);

		break;

	default:
		break;
	}

	spi_letch(&fnd_spi_gpio);


}


void fnd_render_digit(t_FND_digit select_digit, uint8_t num) {


	fnd_set_digit_port(select_digit);

	uint8_t data = fnd_get_digit_num(num);
	spi_tx_data(&fnd_spi_gpio, data);
	spi_letch(&fnd_spi_gpio);
}


