/*
 * bmp180.h
 *
 *  Created on: Apr 4, 2025
 *      Author: pscsp
 */

#ifndef INC_BMP180_H_
#define INC_BMP180_H_

#include "main.h"
#include "def.h"
#include "i2c.h"



// slave address
#define BMP180_ADDR  0x77
#define BMP_ADDR_R   ((BMP180_ADDR << 1) | 0x01)
#define BMP_ADDR_W   ((BMP180_ADDR << 1) | 0x00)

// register
#define OUT_XLSB	0xF8	// ro
#define OUT_LSB 	0xF7	// ro
#define OUT_MSB 	0xF6	// ro
#define CTRL_MEAS 	0xF4	// rw
#define SOFT_RESET 	0xE0	// rw
#define ID			0xD0	// ro



#define BMP_MSB 	0
#define BMP_LSB 	1
#define BMP_XLSB 	2

#define BMP_MAX_SB 	3

#define MAX_OSS			4
#define BMP_CALIB_LEN 	11
#define BMP_RETRY_COUNT	5
#define BMP_UPDATE_TURM 2000 //ms

#define BMP_TEMP 	0
#define BMP_PRRESS	1


typedef enum BMP180_comm_state {

	BMP_STATE_IDLE,
	BMP_STATE_SCAN,
	BMP_STATE_READ_TABLE,
	BMP_STATE_READ_TEMPERATURE,
	BMP_STATE_READ_PRESSURE,
	BMP_STATE_READ_TEMPERATURE_WAIT,
	BMP_STATE_READ_PRESSURE_WAIT,
	BMP_STATE_READ_TEMPERATURE_RECV,
	BMP_STATE_READ_PRESSURE_RECV,

}t_BMP180_commu_state;

typedef struct BMP180_calib_table {

	int16_t AC1;
	int16_t AC2;
	int16_t AC3;

	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;

	int16_t B1;
	int16_t B2;
	int16_t MB;
	int16_t MC;
	int16_t MD;

}t_BMP180_calib_table;

typedef struct BMP180_info {

	 uint8_t oss;
	 long B5; 			// 온도 보정 공식 중간 값 ( 압력 계산시 필요)

	 long temper;
	 uint8_t temper_f;
	 long pressure;
	 uint8_t pressure_f;

	 uint8_t step;
	 uint8_t binit;
	 uint8_t update_value;

}t_BMP180_info;

void bmp180_init();
void bmp180_update();

t_I2C_COMM_state bmp180_read_calib_table();
void bmp180_make_calib_table(uint8_t calib_datas[]);

t_I2C_COMM_state bmp180_read_temperature();
t_I2C_COMM_state bmp180_tx_temperature();
t_I2C_COMM_state bmp180_rx_temperature();

t_I2C_COMM_state bmp180_read_pressure();
t_I2C_COMM_state bmp180_tx_pressure();
t_I2C_COMM_state bmp180_rx_pressure();

long calc_temperature(long ut);
long calc_pressure(long up);
;

void bmp180_print_msg(const char* msg, t_I2C_COMM_state res_state);

uint8_t bmp180_get_update_value();
void bmp180_set_update_value(uint8_t value);


const t_BMP180_info* bmp180_get_info();


uint8_t bmp180_wait();

#endif /* INC_BMP180_H_ */
