/*
 * devicedriver.c
 *
 *  Created on: Apr 7, 2025
 *      Author: microsoft
 */

#include "devicedriver.h"
#include "extern.h"
#include "def.h"
#include "bmp180.h"
#include "lcd1602.h"

#include <stdio.h>


void dd_init() {

	lcd1602_init();
	bmp180_init();


}

void dd_update() {

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	bmp180_update();
}

void dd_render() {

	static char buffer[16] = {0};

	if(bmp180_get_update_value()) {
		bmp180_set_update_value(FALSE);


		t_BMP180_info* pBMP_info = bmp180_get_info();
		if(pBMP_info) {
			lcd1602_move_cursor(0,0);
			sprintf(buffer, "t:%ld.%d", pBMP_info->temper, pBMP_info->temper_f);
			lcd1602_string(buffer);

			lcd1602_move_cursor(1,0);
			sprintf(buffer, "p:%ld.%d", pBMP_info->pressure, pBMP_info->pressure_f);
			lcd1602_string(buffer);
		}






	}

}


void dd_run() {


	while(true) {

		dd_update();

		dd_render();

	}

}





