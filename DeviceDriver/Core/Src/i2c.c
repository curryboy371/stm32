/*
 * i2c.c
 *
 *  Created on: Apr 5, 2025
 *      Author: pscsp
 */

#include "i2c.h"

#include <stdio.h>

#include "def.h"
#include "extern.h"
#include "timer.h"

// init i2c
void I2C_init(t_I2C_settings* pI2C_setting) {

	if(!pI2C_setting) {
		return;
	}

	// scl sda 포트, 핀 기본 설정
	for(int i = 0; i < I2C_MAX_LINE; ++i) {

		// pull 설정
		I2C_set_pull_config(&pI2C_setting->gpio_infos[i]);

		// mode 설정
		I2C_set_gpio_mode(&pI2C_setting->gpio_infos[i]);
	}

}


void I2C_set_pull_config(t_I2C_gpio_info* pI2C_gpio) {

	if(!pI2C_gpio) {
		return;
	}

	volatile uint32_t* reg_pupdr = (uint32_t*)((uint32_t)pI2C_gpio->gpio_port + DMA_PUPDR);

	uint8_t pin_pos =I2C_get_pin_num(pI2C_gpio->gpio_pin);
	uint32_t mask = (0x3 << (pin_pos * 2));

	switch(pI2C_gpio->pull_config) {

		case I2C_PULL_NONE: // none 00
			*reg_pupdr &= ~mask;
		break;

		case I2C_PULL_UP: // pull_up 01
			*reg_pupdr &= ~mask;
			*reg_pupdr |= (0x01 << (pin_pos*2));

			I2C_set_gpio_odr(pI2C_gpio, HIGH);
		break;


		case I2C_PULL_DOWN: // pull_down 10
			*reg_pupdr &= ~mask;
			*reg_pupdr |= (0x02 << (pin_pos*2));

			I2C_set_gpio_odr(pI2C_gpio, LOW);
		break;


		case I2C_PULL_RESERVED:
		default:
		break;

	}


}

void I2C_set_gpio_mode(t_I2C_gpio_info* pI2C_gpio) {

	if(!pI2C_gpio) {
		return;
	}

	volatile uint32_t* reg_mode = (uint32_t*)((uint32_t)pI2C_gpio->gpio_port + DMA_MODE);

	uint8_t pin_pos =I2C_get_pin_num(pI2C_gpio->gpio_pin);
	uint32_t mask = (0x3 << (pin_pos * 2));

	// gpip mode 설정
	switch(pI2C_gpio->gpio_mode) {
		case I2C_MODE_INPUT:  // input 00
			*reg_mode &= ~mask;

			// input mode로 설정하는 경우 idle 상태 초기화
			if(pI2C_gpio->pull_config == I2C_PULL_UP) {
				I2C_set_gpio_odr(pI2C_gpio, HIGH);
			}
			else if(pI2C_gpio->pull_config == I2C_PULL_DOWN) {
				I2C_set_gpio_odr(pI2C_gpio, LOW);
			}

			break;

		case I2C_MODE_OUTPUT: // output 01
			*reg_mode &= ~mask;
			*reg_mode |= (0x1 << (pin_pos * 2));
			break;

		default:
			break;
	}

}


void I2C_set_gpio_odr(t_I2C_gpio_info* pI2C_gpio, uint8_t state) {

    volatile uint32_t* reg_odr = (uint32_t*)((uint32_t)pI2C_gpio->gpio_port + DMA_ODR);

    if(state == HIGH) {

    	*reg_odr |= pI2C_gpio->gpio_pin;
    }
    else {
    	*reg_odr &= ~(pI2C_gpio->gpio_pin);
    }
}

uint8_t I2C_get_gpio_idr(t_I2C_gpio_info* pI2C_gpio) {

	if(!pI2C_gpio) {
		return LOW;
	}

    volatile uint32_t* reg_idr = (uint32_t*)((uint32_t)pI2C_gpio->gpio_port + DMA_IDR);

    uint8_t state = (*reg_idr & pI2C_gpio->gpio_pin) ? HIGH : LOW;

    return state;
}


uint8_t I2C_get_pin_num(uint16_t gpio_pin) {

	uint8_t pin_pos = 0;
	while (((gpio_pin >> pin_pos) & 0x1) == 0 && pin_pos < 16) {
		++pin_pos;
	}

	return pin_pos;
};

// slave addr에 응답이 있는지 scan하는 함수
t_I2C_COMM_state I2C_scan(t_I2C_settings* pI2C_setting, uint8_t addr) {

	if(!pI2C_setting) {
		return I2C_COMM_STATE_ERROR;
	}

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	I2C_start(pI2C_setting);

	state = I2C_transmit_byte(pI2C_setting, addr);
	if(state != I2C_COMM_STATE_OK) {

        I2C_stop(pI2C_setting);
		return state;
	}

	state = I2C_ack_slave(pI2C_setting);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	I2C_stop(pI2C_setting);

	printf("find address %x\r\n", addr);
	return state;

}


// receive와 ackm을 한번에 수행하는 함수
// read는 이미 연결된 상태
t_I2C_COMM_state I2C_read(t_I2C_settings* pI2C_setting, uint8_t* out_rx_data, uint8_t ack_setting, t_I2C_command command) {

	if(!pI2C_setting) {
		return I2C_COMM_STATE_ERROR;
	}

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	// read는 start 안해도 됨
	// command 무시
	if(command & I2C_START) {
		//I2C_start(pI2C_setting);
	}

	state = I2C_receive_byte(pI2C_setting, out_rx_data);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	state = I2C_ack_master(pI2C_setting, ack_setting);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	if(command & I2C_STOP) {
		I2C_stop(pI2C_setting);
	}

	return state;
}


// transmit과 acks을 한번에 수행하는 함수
t_I2C_COMM_state I2C_write(t_I2C_settings* pI2C_setting, uint8_t tx_data, t_I2C_command command) {

	if(!pI2C_setting) {
		return I2C_COMM_STATE_ERROR;
	}

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	// start
	if(command & I2C_START) {
		I2C_start(pI2C_setting);
	}

	state = I2C_transmit_byte(pI2C_setting, tx_data);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	state = I2C_ack_slave(pI2C_setting);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	if(command & I2C_STOP) {
		I2C_stop(pI2C_setting);
	}

	return state;
}

// I2C addr의 register add까지 read 후 빠져나옴
// 정상적인 상황시 통신이 완전히 종료되지 않은 상황에서 빠져나오므로 주의!
t_I2C_COMM_state I2C_connect_register(t_I2C_settings* pI2C_setting, uint8_t addr, uint8_t reg_addr) {

	if(!pI2C_setting) {
		return I2C_COMM_STATE_ERROR;
	}

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	// start
	I2C_start(pI2C_setting);


	// slave write 전송
	state = I2C_transmit_byte(pI2C_setting, addr);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	// acks
	state = I2C_ack_slave(pI2C_setting);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	// register address 전송
	state = I2C_transmit_byte(pI2C_setting, reg_addr);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	// acks
	state = I2C_ack_slave(pI2C_setting);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	// 정상적인 종료시 register까지 연결하고 빠져나옴
	// 아직 연결 중인 상태임을 명시
	return state;

}

// I2C addr의 register에서 out_data로 데이터를 data_len개 읽어오는 함수
t_I2C_COMM_state I2C_read_register(t_I2C_settings* pI2C_setting, uint8_t addr, uint8_t* out_data, uint32_t data_len) {

	if(!pI2C_setting) {
		return I2C_COMM_STATE_ERROR;
	}

	t_I2C_COMM_state state = I2C_COMM_STATE_OK;

	// start
	I2C_start(pI2C_setting);

	// slave read 전송
	state = I2C_transmit_byte(pI2C_setting, addr);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	// acks
	state = I2C_ack_slave(pI2C_setting);
	if(state != I2C_COMM_STATE_OK) {
		I2C_stop(pI2C_setting);
		return state;
	}

	// burst 방식으로 한번에 받아옴
	for(int i = 0; i < data_len; ++i) {

		state = I2C_receive_byte(pI2C_setting, out_data);
		if(state != I2C_COMM_STATE_OK) {
			I2C_stop(pI2C_setting);
			return state;
		}

		state = I2C_ack_master(pI2C_setting, pI2C_setting->ack);
		if(state != I2C_COMM_STATE_OK) {
			I2C_stop(pI2C_setting);
			return state;
		}

		++out_data;
	}

	I2C_stop(pI2C_setting);

	return state;
}


// I2C start signal
// 기기별로 시그널이 달라서 구분이 필요할 수도 있음
// 그 경우에는 type을 추가하여 조건문 처리하도록 하자
void I2C_start(t_I2C_settings* pI2C_setting) {

	pI2C_setting->comm_step = I2C_COMM_STEP_START;

	t_I2C_gpio_info* pI2C_sda = &pI2C_setting->gpio_infos[I2C_SDA];
	t_I2C_gpio_info* pI2C_scl = &pI2C_setting->gpio_infos[I2C_SCL];

	// sda output mode로 변경
	pI2C_sda->gpio_mode = I2C_MODE_OUTPUT;
	I2C_set_gpio_mode(pI2C_sda);

	//	SCL 이 HIGH 상태일 때 SDA가 하강하며 start
    I2C_set_gpio_odr(pI2C_sda, HIGH);
    I2C_set_gpio_odr(pI2C_scl, HIGH);

    I2C_set_gpio_odr(pI2C_sda, LOW);

	// SCL이 상승에서 데이터를 받도록 클럭은 하강 상태에서 시작
    I2C_set_gpio_odr(pI2C_scl, LOW);

}

// I2C start stop signal
// 기기별로 시그널이 달라서 구분이 필요할 수도 있음
// 그 경우에는 type을 추가하여 조건문 처리하도록 하자
void I2C_stop(t_I2C_settings* pI2C_setting) {

	if(!pI2C_setting) {
		return;
	}

	pI2C_setting->comm_step = I2C_COMM_STEP_STOP;

	t_I2C_gpio_info* pI2C_sda = &pI2C_setting->gpio_infos[I2C_SDA];
	t_I2C_gpio_info* pI2C_scl = &pI2C_setting->gpio_infos[I2C_SCL];

	// sda output mode로 변경
	pI2C_sda->gpio_mode = I2C_MODE_OUTPUT;
	I2C_set_gpio_mode(pI2C_sda);


	// sda를 확실히 high 상태로 두기위해 low처리
    I2C_set_gpio_odr(pI2C_sda, LOW);

	//	SCL 이 HIGH 상태일 때 SDA를 HIGH로 변경하여 STOP
    I2C_set_gpio_odr(pI2C_scl, HIGH);
    I2C_set_gpio_odr(pI2C_sda, HIGH);

}

t_I2C_COMM_state I2C_transmit_byte(t_I2C_settings* pI2C_setting, uint8_t tx_data) {

	if(!pI2C_setting) {
		return I2C_COMM_STATE_ERROR;
	}

	pI2C_setting->comm_step = I2C_COMM_STEP_TX;

	t_I2C_gpio_info* pI2C_sda = &pI2C_setting->gpio_infos[I2C_SDA];

	// sda output mode로 변경
	pI2C_sda->gpio_mode = I2C_MODE_OUTPUT;
	I2C_set_gpio_mode(pI2C_sda);

	// bit order에 맞춰 I2C data write
	if(pI2C_setting->bit_order == I2C_MSB_FIRST) {

		// MSB to LSB
		for(uint8_t i = 0; i < 8; ++i) {

			uint8_t v = (tx_data & 0x80) >> 7;
			I2C_set_gpio_odr(pI2C_sda, v);
			tx_data <<= 0x01;

			I2C_clock_tick(&pI2C_setting->gpio_infos[I2C_SCL], pI2C_setting->sampling_type);
		}
	}
	else {
		// LSB to MSB
		for(uint8_t i = 0; i < 8; ++i) {

			uint8_t v = tx_data & 0x01;
			I2C_set_gpio_odr(pI2C_sda, v);
			tx_data >>= 0x01;

			I2C_clock_tick(&pI2C_setting->gpio_infos[I2C_SCL], pI2C_setting->sampling_type);
		}

	}

	return I2C_COMM_STATE_OK;
}

t_I2C_COMM_state I2C_receive_byte(t_I2C_settings* pI2C_setting, uint8_t* out_rx_data) {

	if(!pI2C_setting) {
		return I2C_COMM_STATE_ERROR;
	}

	pI2C_setting->comm_step = I2C_COMM_STEP_RX;

	t_I2C_gpio_info* pI2C_sda = &pI2C_setting->gpio_infos[I2C_SDA];

	// sda input mode로 변경
	pI2C_sda->gpio_mode = I2C_MODE_INPUT;
	I2C_set_gpio_mode(pI2C_sda);

	*out_rx_data = 0;

	// bit order에 맞춰 I2C data read
	if(pI2C_setting->bit_order == I2C_MSB_FIRST) {

		for(int i = 7; i >= 0; --i){

			if(I2C_get_gpio_idr(pI2C_sda)) {

				*out_rx_data |= 1 << i;
			}
			I2C_clock_tick(&pI2C_setting->gpio_infos[I2C_SCL], pI2C_setting->sampling_type);
		}

	}
	else {

		for(int i = 0; i < 8; ++i){

			if(I2C_get_gpio_idr(pI2C_sda)) {

				*out_rx_data |= 1 << i;
			}
			I2C_clock_tick(&pI2C_setting->gpio_infos[I2C_SCL], pI2C_setting->sampling_type);
		}
	}

	return I2C_COMM_STATE_OK;
}

// ack of master
t_I2C_COMM_state I2C_ack_master(t_I2C_settings* pI2C_setting, uint8_t ack_setting) {

	if(!pI2C_setting) {
		return I2C_COMM_STATE_ERROR;
	}

	pI2C_setting->comm_step =I2C_COMM_STEP_ACKM;


	t_I2C_gpio_info* pI2C_sda = &pI2C_setting->gpio_infos[I2C_SDA];

	// sda output mode로 변경
	pI2C_sda->gpio_mode = I2C_MODE_OUTPUT;
	I2C_set_gpio_mode(pI2C_sda);


	// ack or nack
	I2C_set_gpio_odr(pI2C_sda, ack_setting);

	// clock tick
	I2C_clock_tick(&pI2C_setting->gpio_infos[I2C_SCL], pI2C_setting->sampling_type);


	return I2C_COMM_STATE_OK;
}

// ack of slave
t_I2C_COMM_state I2C_ack_slave(t_I2C_settings* pI2C_setting) {

	if(!pI2C_setting) {
		return I2C_COMM_STATE_ERROR;
	}

	pI2C_setting->comm_step =I2C_COMM_STEP_ACKS;

	// sda input mode로 변경
	pI2C_setting->gpio_infos[I2C_SDA].gpio_mode = I2C_MODE_INPUT;
	I2C_set_gpio_mode(&pI2C_setting->gpio_infos[I2C_SDA]);

	// acks시 clock을 여기서 바로 tick해버리면 문제가 발생하였음
	// rising sampling을 기준으로 clock을 올렸다가 내리면 그 사이에 sda에 acks가 정상적으로 올 줄 알았는데
	// 충분한 시간을 기다린 후 clock을 내리더라도 read하는 시점에 clock이 내려져 있으면 acks를 정상적으로 읽을 수 없었음

	// clock tick (tick)
	I2C_set_gpio_odr(&pI2C_setting->gpio_infos[I2C_SCL], (uint8_t)pI2C_setting->sampling_type);


	// sda가 slave에 의해 ack가 될 때까지 wait
	uint32_t start = get_tick();
	while(I2C_get_gpio_idr(&pI2C_setting->gpio_infos[I2C_SDA]) != pI2C_setting->ack) {

		delay_us(10);
		uint32_t now = get_tick();
		if( now > start + pI2C_setting->timeout_ms) {
			return I2C_COMM_STATE_TIMEOUT;
		}
	}

	// clock tick (tock)
	I2C_set_gpio_odr(&pI2C_setting->gpio_infos[I2C_SCL], (uint8_t)!(pI2C_setting->sampling_type));

	return I2C_COMM_STATE_OK;

}


// clock을 토글하는 함수
void I2C_clock_tick(t_I2C_gpio_info* pI2C_gpio, t_I2C_sampling_type sampling_type) {

	if(!pI2C_gpio) {
		return;
	}
	// sampling_type == I2C_RISING_EDGE_SAMPLING (0)
	// high to low로 클럭 tick
	// sampling_type == I2C_FALLING_EDGE_SAMPLING (1)
	// low to high로 클럭 tick

	I2C_set_gpio_odr(pI2C_gpio, (uint8_t)sampling_type);
	I2C_set_gpio_odr(pI2C_gpio, (uint8_t)!sampling_type);

}








