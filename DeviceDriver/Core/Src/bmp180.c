/*
 * bmp180.c
 *
 *  Created on: Apr 4, 2025
 *      Author: pscsp
 */

#include "bmp180.h"

#include "extern.h"
#include "def.h"
#include "timer.h"

#include <stdio.h>

t_BMP180_calib_table calib_table_info;

t_BMP180_info BMP_info;

t_BMP180_commu_state BMP_state;

uint8_t press_wait_time[MAX_OSS] = { 5, 7, 15, 26 }; // 4.5, 7.5 13.5 25.5 ms

t_I2C_settings I2C_setting;


volatile uint32_t bmp_timer;

void bmp180_init() {

	HAL_Delay(100);

	t_I2C_gpio_info* scl_info = &I2C_setting.gpio_infos[I2C_SCL];
	t_I2C_gpio_info* sda_info = &I2C_setting.gpio_infos[I2C_SDA];

	scl_info->gpio_port = (uint32_t*)DMA_GPIOA_BASE;
	scl_info->gpio_pin = GPIO_PIN_8;
	scl_info->gpio_mode = I2C_MODE_OUTPUT;
	scl_info->pull_config = I2C_PULL_UP;


	sda_info->gpio_port = (uint32_t*)DMA_GPIOA_BASE;
	sda_info->gpio_pin = GPIO_PIN_7;
	sda_info->gpio_mode = I2C_MODE_OUTPUT;
	sda_info->pull_config = I2C_PULL_UP;

	I2C_setting.timeout_ms = 100;
	I2C_setting.sampling_type = I2C_RISING_EDGE_SAMPLING; 	// sampling은 rising 시점에
	I2C_setting.ack = LOW;									// ack 기준은 low
	I2C_setting.bit_order = I2C_MSB_FIRST; 					// MSB 먼저 처리



	I2C_setting.comm_step = I2C_COMM_STEP_WAIT;

	I2C_init(&I2C_setting);


	BMP_state = BMP_STATE_IDLE;

	BMP_info.oss = 0; //

	BMP_info.step = BMP_TEMP;

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	for(int i = 0; i < BMP_RETRY_COUNT; ++i) {

		BMP_state = BMP_STATE_SCAN;
		state = I2C_scan(&I2C_setting, BMP_ADDR_W);
		if(state == I2C_COMM_STATE_OK){
			break;
		}

		bmp180_print_msg("scan err", state);

		printf("retry scan %d\r\n", i+1);
		HAL_Delay(10);
	}

	for(int i = 0; i < BMP_RETRY_COUNT; ++i) {

		state = bmp180_read_calib_table();
		if(state == I2C_COMM_STATE_OK){
			BMP_info.binit = TRUE;
			break;
		}

		bmp180_print_msg("read calib err", state);
		printf("retry read calib table %d\r\n", i+1);
		HAL_Delay(10);
	}
}

void bmp180_update() {

	if(!BMP_info.binit) {
		return;
	}

	if(bmp_timer > 0 ){
		return;
	}

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	state = bmp180_rx_temperature();
	if(state != I2C_COMM_STATE_OK) {
		bmp180_print_msg("rx temp err", state);
	}

	state = bmp180_rx_pressure();
	if(state != I2C_COMM_STATE_OK) {
		bmp180_print_msg("rx press err", state);
	}

	if(BMP_info.step == BMP_TEMP) {
		state = bmp180_read_temperature();
		if(state != I2C_COMM_STATE_OK) {
			bmp180_print_msg("read temp err", state);
		}
	}
	else {
		state = bmp180_read_pressure();
		if(state != I2C_COMM_STATE_OK) {
			bmp180_print_msg("read press err", state);
		}
	}

}

t_I2C_COMM_state bmp180_read_calib_table() {

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	BMP_state = BMP_STATE_READ_TABLE;

	// calib talbe regster와 connect
	// 연속적 데이터이므로 시작주소와 연결 후 한번에 받음
	uint8_t reg_addr = 0xAA; // calib reg start address
	state = I2C_connect_register(&I2C_setting, BMP_ADDR_W, reg_addr);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	// calib table data 한번에 받기
	uint8_t calib_datas[BMP_CALIB_LEN*2] = {0};
	state = I2C_read_register(&I2C_setting, BMP_ADDR_R, calib_datas, BMP_CALIB_LEN*2);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	// make data
	bmp180_make_calib_table(calib_datas);


	return state;
}

void bmp180_make_calib_table(uint8_t calib_datas[]) {

    int16_t* table[] = {
        &calib_table_info.AC1, &calib_table_info.AC2, &calib_table_info.AC3,
        (int16_t*)&calib_table_info.AC4, (int16_t*)&calib_table_info.AC5, (int16_t*)&calib_table_info.AC6,
        &calib_table_info.B1, &calib_table_info.B2,
        &calib_table_info.MB, &calib_table_info.MC, &calib_table_info.MD
    };

    int size = sizeof(table) / sizeof(table[0]);


    for (int i = 0; i < size; i++) {

        *table[i] = (int16_t)(calib_datas[2 * i] << 8 | calib_datas[2 * i + 1]);
    }

    printf("make clib table\r\n");
}

t_I2C_COMM_state bmp180_read_temperature() {

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;
	if(bmp_timer > 0) {
		return state;
	}

	if(bmp180_wait() == FALSE) {
		return bmp180_tx_temperature();
	}
	return state;
}

t_I2C_COMM_state bmp180_tx_temperature() {

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;
	if(bmp_timer > 0) {
		return state;
	}

	BMP_state = BMP_STATE_READ_TEMPERATURE;

	state = I2C_connect_register(&I2C_setting, BMP_ADDR_W, CTRL_MEAS);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	state = I2C_write(&I2C_setting, 0x2e, I2C_STOP);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	// 4.5ms wait
	bmp_timer = 5;
	//HAL_Delay(5);
	BMP_state = BMP_STATE_READ_TEMPERATURE_WAIT;

	return state;
}

t_I2C_COMM_state bmp180_rx_temperature() {

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	if(BMP_state != BMP_STATE_READ_TEMPERATURE_WAIT) {
		return state;
	}

	if(bmp_timer > 0) {
		return state;
	}

	BMP_state = BMP_STATE_READ_TEMPERATURE_RECV;

	uint8_t output_datas[2] = {0};

	state = I2C_connect_register(&I2C_setting, BMP_ADDR_W, OUT_MSB);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}


	// slave read 전송
	state = I2C_write(&I2C_setting, BMP_ADDR_R, I2C_START);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	// temper의 msb, lsb을 read
	// 마지막 read에서는 nack을 ackm으로 응답해야함
	state = I2C_read(&I2C_setting, &output_datas[BMP_MSB], I2C_setting.ack, I2C_NONE);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	// 마지막 data read 후 내부에서 stop
	state = I2C_read(&I2C_setting, &output_datas[BMP_LSB], !I2C_setting.ack, I2C_STOP);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	uint16_t uncomp_temperature = output_datas[BMP_MSB] << 8 | output_datas[BMP_LSB];

	long temperature = calc_temperature(uncomp_temperature);


	BMP_info.temper = (long)temperature / 10;
	BMP_info.temper_f = temperature % 10;
	BMP_info.step = BMP_PRRESS;
	printf("ut:%ld t:%ld.%d \r\n", uncomp_temperature, BMP_info.temper, BMP_info.temper_f);

	bmp_timer = BMP_UPDATE_TURM;
	return state;

}

t_I2C_COMM_state bmp180_read_pressure() {

	t_I2C_COMM_state state  = I2C_COMM_STATE_OK;

	if(bmp_timer > 0) {
		return state;
	}

	if(bmp180_wait() == FALSE) {
		return bmp180_tx_pressure();
	}

	return state;
}

t_I2C_COMM_state bmp180_tx_pressure() {

	t_I2C_COMM_state state  = I2C_COMM_STATE_OK;

	if(bmp_timer > 0) {
		return state;
	}


	BMP_state = BMP_STATE_READ_PRESSURE;

	state = I2C_connect_register(&I2C_setting, BMP_ADDR_W, CTRL_MEAS);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	state = I2C_write(&I2C_setting, 0x34 | BMP_info.oss << 6, I2C_STOP);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	// wait
	bmp_timer = press_wait_time[BMP_info.oss];
	//HAL_Delay();
	BMP_state = BMP_STATE_READ_PRESSURE_WAIT;

	return state;

}

t_I2C_COMM_state bmp180_rx_pressure() {

	t_I2C_COMM_state state  = I2C_COMM_STATE_OK;

	if(BMP_state != BMP_STATE_READ_PRESSURE_WAIT) {
		return state;
	}

	if(bmp_timer > 0) {
		return state;
	}
	BMP_state = BMP_STATE_READ_PRESSURE_RECV;

	uint8_t output_datas[3] = {0};

	state = I2C_connect_register(&I2C_setting, BMP_ADDR_W, OUT_MSB);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	// slave read 전송
	state = I2C_write(&I2C_setting, BMP_ADDR_R, I2C_START);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	// temper의 msb, lsb을 read
	// 마지막 read에서는 nack을 ackm으로 응답해야함
	state = I2C_read(&I2C_setting, &output_datas[BMP_MSB], I2C_setting.ack, I2C_NONE);
	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

	if(BMP_info.oss == 0) {

		// 마지막 data read 후 내부에서 stop
		state = I2C_read(&I2C_setting, &output_datas[BMP_LSB], !I2C_setting.ack, I2C_STOP);
		if(state != I2C_COMM_STATE_OK) {
			return state;
		}
	}
	else {
		// oss 1부터는 xlsb 사용
		state = I2C_read(&I2C_setting, &output_datas[BMP_LSB], I2C_setting.ack, I2C_NONE);
		if(state != I2C_COMM_STATE_OK) {
			return state;
		}

		// 마지막 data read 후 내부에서 stop
		state = I2C_read(&I2C_setting, &output_datas[BMP_XLSB], !I2C_setting.ack, I2C_STOP);
		if(state != I2C_COMM_STATE_OK) {
			return state;
		}
	}


	// xlsb는 oss의 단계에 따라 상위 비트만 사용
	long uncomp_pressure = output_datas[BMP_MSB] << 16 | output_datas[BMP_LSB] << 8 | output_datas[BMP_XLSB] >> (8 - BMP_info.oss);
	long pressure = calc_pressure(uncomp_pressure);

	BMP_info.step = BMP_TEMP;
	BMP_info.pressure = (long)pressure / 100;
	BMP_info.pressure_f = pressure % 100;
	printf("up:%ld p:%ld.%d \r\n", uncomp_pressure, BMP_info.pressure, BMP_info.pressure_f);

	bmp_timer = BMP_UPDATE_TURM;

	BMP_info.update_value = true;
	return state;

}

long calc_temperature(long ut) {

	// X1 = ( UT - AC6) * AC5 / 2^15
	long x1 = (ut- calib_table_info.AC6) * calib_table_info.AC5 / (1 << 15);

	// x2 = mc* 2^11 / (x1 + md)
	long x2 = calib_table_info.MC * (1 << 11) / (x1 + calib_table_info.MD);

	// b5 = x1 + x2
	BMP_info.B5 = x1 + x2;

	// t = (b5+ 8) / 2^4
	long t = (BMP_info.B5+8) >> 4;

	return t;
}


long calc_pressure(long up) {

	// B6 = B5 -4000
	long b6 = BMP_info.B5 - 4000;

	// x1 = (b2 * (b6 * b6 / 2^12)) / 2^11
	long x1 = (calib_table_info.B2 * (b6*b6 >> 12)) >> 11;

	// x2 = AC2* B6 / 2^11
	long x2 = calib_table_info.AC2 * b6 >> 11;

	/// x3 = x1 + x2
	long x3 = x1 + x2;

	// b3 = (((AC1* 4 + x3) << oss) +2) /4
	long b3 = (((calib_table_info.AC1 * 4 + x3) << BMP_info.oss) + 2) >> 4;

	// x1 = AC3 * b6 / 2^13
	x1 = calib_table_info.AC3 * b6  >> 13;

	// x2 = (B1 * ( B6 * B6 / 2^12)) / 2^16
	x2 = (calib_table_info.B1 * (b6 * b6 >> 12)) >> 16;

	// x3 = ((X1 + X2) +2) / 2^2
	x3 = ((x1 + x2) + 2) >> 2;

	// B4 = AC4 * (ulong)(x3 + 32768) / 2^15
	unsigned long b4 = calib_table_info.AC4 * (unsigned long)(x3 + 32768) >> 15;

	// B7 = ((unsigned long)UP - B3) * (50000 >> oss)
	unsigned long b7 = ((unsigned long)up - b3) * (50000 >> BMP_info.oss);


	// if (B7 < 0x80000000) { p = (B7*2) / B4 }
	// else { p= (B7 / B4) *2}
	long p = 0;
	if(b7 < 0x80000000) {
		p = (b7*2) / b4;
	}
	else {
		p = (b7/b4) * 2;
	}

	// x1 = (p/2^8) * (p/2^8)
	x1 = (p >> 8) * (p >> 8);

	// x1 = (x1 * 3038) / 2^16
	x1 = (x1 * 3038) >> 16;

	// x2 = (-7357*p) / 2^16
	x2 = (-7357*p) >> 16;

	// p = p + (x1 + x2 + 3791) / 2^4
	p = p + ((x1 + x2 + 3791) >> 4);

	return p;
}


void bmp180_run() {

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;
	while(true) {


		state = bmp180_rx_temperature();
		if(state != I2C_COMM_STATE_OK) {
			bmp180_print_msg("rx temp err", state);
		}

		state = bmp180_rx_pressure();
		if(state != I2C_COMM_STATE_OK) {
			bmp180_print_msg("rx press err", state);
		}


		if(TIM11_1ms_counter > 1500) {
			TIM11_1ms_counter = 0;

			if(BMP_info.step == BMP_TEMP) {
				state = bmp180_read_temperature();
				if(state != I2C_COMM_STATE_OK) {
					bmp180_print_msg("read temp err", state);
				}
			}
			else {
				state = bmp180_read_pressure();
				if(state != I2C_COMM_STATE_OK) {
					bmp180_print_msg("read press err", state);
				}
			}
		}
	}
}




void bmp180_print_msg(const char* msg, t_I2C_COMM_state res_state) {

	printf("%d %s bmp:%d i2c:%d\r\n", res_state, msg, BMP_state, I2C_setting.comm_step);
}



uint8_t bmp180_wait() {

	return (BMP_state == BMP_STATE_READ_TEMPERATURE_WAIT ||  BMP_state == BMP_STATE_READ_TEMPERATURE_WAIT);
}

uint8_t bmp180_get_update_value() {

	return BMP_info.update_value;
}

void bmp180_set_update_value(uint8_t value) {

	BMP_info.update_value = value;
}

const t_BMP180_info* bmp180_get_info() {

	return &BMP_info;
}



