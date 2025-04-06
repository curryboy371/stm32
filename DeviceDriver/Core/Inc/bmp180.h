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


#define BMP_SCL_PORT	GPIOA
#define BMP_SDA_PORT	GPIOB
#define BMP_SDA_PIN 	GPIO_PIN_4
#define BMP_SCL_PIN		GPIO_PIN_8


#define BMP_MSB 	0
#define BMP_LSB 	1
#define BMP_XLSB 	2

#define BMP_MAX_SB 	3

#define MAX_OSS		4

#define I2C_ACK		0
#define I2C_NACK	1

typedef enum BMP180_comm_state {

	BMP_STATE_IDLE,
	BMP_STATE_SCAN,

	BMP_STATE_READ_TABLE,
	BMP_STATE_READ_TEMPERATURE,
	BMP_STATE_READ_PRESSURE,

}t_BMP180_commu_state;

typedef enum BMP180_comm_step {


	BMP_STEP_WAIT,
	BMP_STEP_START,
	BMP_STEP_RESTART,
	BMP_STEP_TX,
	BMP_STEP_RX,
	BMP_STEP_STOP,


}t_BMP180_commu_step;


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

}t_BMP180_info;

void bmp180_init();
t_commu_state bmp180_scan();
t_commu_state bmp180_read_calib_table();
void bmp180_make_calib_table(uint8_t calib_datas[]);

t_commu_state bmp180_read_temperature();
t_commu_state bmp180_read_pressure();

long calc_temperature(long ut);
long calc_pressure(long up);

void bmp180_run();

void bmp180_start();
void bmp180_stop();

t_I2C_COMM_state bmp180_tx(uint8_t value);
t_I2C_COMM_state bmp180_rx(uint8_t* out_value, uint8_t in_ackm);

t_I2C_COMM_state bmp180_acks();

void bmp180_ackm(uint8_t in_ackm);

void bmp180_clock();

void bmp180_set_sda_mode(t_I2C_gpio_mode gpio_mode);


#endif /* INC_BMP180_H_ */
