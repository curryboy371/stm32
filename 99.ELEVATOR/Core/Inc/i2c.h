/*
 * i2c.h
 *
 *  Created on: Apr 5, 2025
 *      Author: pscsp
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_

#include "main.h"

//https://github.com/bitbank2/BitBang_I2C/blob/master/src/BitBang_I2C.h


// i2c 통신 단계
typedef enum I2C_COMM_step {

	I2C_COMM_STEP_WAIT,
	I2C_COMM_STEP_START,
	I2C_COMM_STEP_RESTART,
	I2C_COMM_STEP_TX,
	I2C_COMM_STEP_RX,

	I2C_COMM_STEP_ACKS,
	I2C_COMM_STEP_ACKM,
	I2C_COMM_STEP_STOP,

}t_I2C_COMM_step;


// i2c 통신 상태
typedef enum I2C_COMM_state {

	I2C_COMM_STATE_OK,
	I2C_COMM_STATE_TIMEOUT,

	I2C_COMM_STATE_ERROR,

}t_I2C_COMM_state;

typedef enum {
    I2C_MSB_FIRST,  // MSB부터 전송
    I2C_LSB_FIRST   // LSB부터 전송
} I2C_bit_order;

typedef enum {
	I2C_NONE = 0x00,  							// none start, stop
    I2C_START = 0x01,  							// only start
	I2C_STOP = 0x02,  							// only stop
    I2C_START_AND_STOP = I2C_START | I2C_STOP   	// start and stop
} t_I2C_command;


typedef enum I2C_sampling_type {

	I2C_FALLING_EDGE_SAMPLING,		// 하강 구간에서 샘플링 ( clock은 l to h )
	I2C_RISING_EDGE_SAMPLING,		// 상승 구간에서 샘플링 ( clock은 h to l )

}t_I2C_sampling_type;

typedef enum I2C_line{

	I2C_SCL,
	I2C_SDA,
	I2C_MAX_LINE,

} t_I2C_line;

typedef enum I2C_gpio_mode{

	I2C_MODE_OUTPUT,	// 출력 모드
	I2C_MODE_INPUT		// 입력 모드

} t_I2C_gpio_mode;

typedef enum I2C_gpio_pull{

	I2C_PULL_NONE,    // 풀업/풀다운 없음
	I2C_PULL_UP,      // 풀업
	I2C_PULL_DOWN,     // 풀다운
	I2C_PULL_RESERVED, //

} t_I2C_gpio_pull;

typedef struct I2C_gpio_info{

	uint32_t* gpio_port;
	uint16_t gpio_pin;
	t_I2C_gpio_mode gpio_mode;
	t_I2C_gpio_pull pull_config;

}t_I2C_gpio_info;

typedef struct I2C_settings{

	t_I2C_gpio_info gpio_infos[I2C_MAX_LINE];


	t_I2C_COMM_step comm_step;
	t_I2C_sampling_type sampling_type;
	uint32_t timeout_ms;

	I2C_bit_order bit_order;
	uint8_t ack;

}t_I2C_settings;



void I2C_init(t_I2C_settings* pI2C_setting);

void I2C_set_pull_config(t_I2C_gpio_info* pI2C_gpio);

void I2C_set_gpio_mode(t_I2C_gpio_info* pI2C_gpio);

void I2C_set_gpio_odr(t_I2C_gpio_info* pI2C_gpio, uint8_t state);
uint8_t I2C_get_gpio_idr(t_I2C_gpio_info* pI2C_gpio);

uint8_t I2C_get_pin_num(uint16_t gpio_pin);



t_I2C_COMM_state I2C_scan(t_I2C_settings* pI2C_setting, uint8_t addr);

t_I2C_COMM_state I2C_read(t_I2C_settings* pI2C_setting, uint8_t* out_rx_data, uint8_t ack_setting, t_I2C_command command);
t_I2C_COMM_state I2C_write(t_I2C_settings* pI2C_setting, uint8_t* tx_datas, uint8_t data_len, t_I2C_command command);

t_I2C_COMM_state I2C_connect_register(t_I2C_settings* pI2C_setting, uint8_t addr, uint8_t reg_addr);
t_I2C_COMM_state I2C_read_register(t_I2C_settings* pI2C_setting, uint8_t addr, uint8_t* out_data, uint32_t data_len);



void I2C_start(t_I2C_settings* pI2C_setting);
void I2C_stop(t_I2C_settings* pI2C_setting);



t_I2C_COMM_state I2C_transmit_byte(t_I2C_settings* pI2C_setting, uint8_t tx_data);
t_I2C_COMM_state I2C_receive_byte(t_I2C_settings* pI2C_setting, uint8_t* out_rx_data);

t_I2C_COMM_state I2C_ack_master(t_I2C_settings* pI2C_setting, uint8_t ack_setting);
t_I2C_COMM_state I2C_ack_slave(t_I2C_settings* pI2C_setting);

void I2C_clock_tick(t_I2C_gpio_info* pI2C_gpio, t_I2C_sampling_type sampling_type);



#endif /* INC_I2C_H_ */
