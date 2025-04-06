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

t_BMP180_commu_step BMP_step;
t_BMP180_commu_state BMP_state;

uint8_t press_wait_time[MAX_OSS] = { 5, 7, 15, 26 }; // 4.5, 7.5 13.5 25.5 ms

t_I2C_settings I2C_setting;


void bmp180_init() {

#if ACCESS_MODE == USE_DMA

	t_I2C_gpio_info* scl_info = &I2C_setting.gpio_infos[I2C_SCL];
	t_I2C_gpio_info* sda_info = &I2C_setting.gpio_infos[I2C_SDA];

	scl_info->gpio_port = (uint32_t*)DMA_GPIOA_BASE;
	scl_info->gpio_pin = GPIO_PIN_8;
	scl_info->gpio_mode = I2C_MODE_OUTPUT;
	scl_info->pull_config = I2C_PULL_UP;


	sda_info->gpio_port = (uint32_t*)DMA_GPIOB_BASE;
	sda_info->gpio_pin = GPIO_PIN_4;
	sda_info->gpio_mode = I2C_MODE_OUTPUT;
	sda_info->pull_config = I2C_PULL_UP;

	I2C_setting.timeout_ms = 100;
	I2C_setting.sampling_type = I2C_RISING_EDGE_SAMPLING; 	// sampling은 rising 시점에
	I2C_setting.ack = LOW;									// ack 기준은 low
	I2C_setting.bit_order = I2C_MSB_FIRST; 					// MSB 먼저 처리



	I2C_setting.comm_step = I2C_COMM_STEP_WAIT;

	I2C_init(&I2C_setting);
#else
	HAL_GPIO_WritePin(BMP_SCL_PORT, BMP_SCL_PIN, SET);
	HAL_GPIO_WritePin(BMP_SDA_PORT, BMP_SDA_PIN, SET);
#endif


	BMP_step = BMP_STEP_WAIT;
	BMP_state = BMP_STATE_IDLE;




	BMP_info.oss = 0; //

	bmp180_scan();



	bmp180_read_calib_table();

}


t_commu_state bmp180_scan() {
	printf("scan\r\n");
	t_I2C_COMM_state state = I2C_COMM_STATE_OK;
	BMP_state = BMP_STATE_SCAN;

#if ACCESS_MODE == USE_DMA
	state = I2C_scan(&I2C_setting, BMP_ADDR_W);

	if(state != I2C_COMM_STATE_OK) {
		return state;
	}

#else
	bmp180_start();

	commu_state = bmp180_tx(BMP_ADDR_W);
    if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
    	return commu_state;
    }

    bmp180_stop();
	printf("finish scan \r\n");

	return commu_state;
#endif

	printf("finish scan \r\n");

}


t_commu_state bmp180_read_calib_table() {

	t_commu_state commu_state = COMMU_OK;

	BMP_state = BMP_STATE_READ_TABLE;


	// calib reg start address
	uint8_t reg_addr = 0xAA;

	// start
	bmp180_start();

	// slave write 전송
	commu_state = bmp180_tx(BMP_ADDR_W);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// register address 전송
	commu_state = bmp180_tx(reg_addr);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// restart
	bmp180_start();

	// slave read 전송
	commu_state = bmp180_tx(BMP_ADDR_R);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}



	// burst 방식으로 한번에 받아옴
	uint8_t calib_datas[22] = {0};
	for(int i = 0; i < 22; ++i) {

		commu_state = bmp180_rx(&calib_datas[i], I2C_ACK);
		if(commu_state != COMMU_OK) {
			printf("timeout\r\n");
			bmp180_stop();
			return commu_state;
		}
	}


	bmp180_stop();

	// make data
	bmp180_make_calib_table(calib_datas);


	return commu_state;
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


/*	calib_table_info.AC1 = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.AC2 = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.AC3 = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.AC4 = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.AC5 = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.AC6 = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.B1 = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.B2 = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.MB = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.MC = calib_datas[idx++] << 8 | calib_datas[idx++];
	calib_table_info.MD = calib_datas[idx++] << 8 | calib_datas[idx++];*/

}

t_commu_state bmp180_read_temperature() {

	t_commu_state commu_state = COMMU_OK;


	BMP_state = BMP_STATE_READ_PRESSURE;


	// start
	bmp180_start();


	// slave write 전송
	commu_state = bmp180_tx(BMP_ADDR_W);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// 0x2e write into reg 0xf4

	// write reg address
	commu_state = bmp180_tx(CTRL_MEAS);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// write 0x2e (
	commu_state = bmp180_tx(0x2e);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	bmp180_stop();


	// 4.5ms wait
	HAL_Delay(5);

	// start
	bmp180_start();

	// slave write 전송
	commu_state = bmp180_tx(BMP_ADDR_W);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// read reg MSB, LSB
	uint8_t output_datas[2] = {0};
	commu_state = bmp180_tx(OUT_MSB);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// restart
	bmp180_start();

	// slave read 전송
	commu_state = bmp180_tx(BMP_ADDR_R);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}



	// msb, lsb read
	// ack, nack로 master 응답
	commu_state = bmp180_rx(&output_datas[BMP_MSB], I2C_ACK);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	commu_state = bmp180_rx(&output_datas[BMP_LSB], I2C_NACK);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	bmp180_stop();


	uint16_t uncomp_temperature = output_datas[BMP_MSB] << 8 | output_datas[BMP_LSB];

	long temperature = calc_temperature(uncomp_temperature);

	printf("ut:%ld t:%ld.%d \r\n", uncomp_temperature, (long)(temperature / 10), temperature % 10);
	return commu_state;
}


t_commu_state bmp180_read_pressure() {

	t_commu_state commu_state = COMMU_OK;


	BMP_state = BMP_STATE_READ_PRESSURE;


	// start
	bmp180_start();


	// slave write 전송
	commu_state = bmp180_tx(BMP_ADDR_W);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// 0x2e write into reg 0xf4


	// write reg address
	commu_state = bmp180_tx(CTRL_MEAS);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// write 0x34 + oss << 6
	commu_state = bmp180_tx(0x34 | BMP_info.oss << 6);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	bmp180_stop();


	// wait
	HAL_Delay(press_wait_time[BMP_info.oss]);

	// start
	bmp180_start();

	// slave write 전송
	commu_state = bmp180_tx(BMP_ADDR_W);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// read reg MSB, LSB
	uint8_t output_datas[BMP_MAX_SB] = {0};
	commu_state = bmp180_tx(OUT_MSB);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}

	// restart
	bmp180_start();

	// slave read 전송
	commu_state = bmp180_tx(BMP_ADDR_R);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}



	// msb, lsb read
	// ack, nack로 master 응답
	commu_state = bmp180_rx(&output_datas[BMP_MSB], I2C_ACK);
	if(commu_state != COMMU_OK) {
		bmp180_stop();
		printf("timeout\r\n");
		return commu_state;
	}




	if(BMP_info.oss == 0) {

		// oss 0은 msb lsb만 사용
		commu_state = bmp180_rx(&output_datas[BMP_LSB], I2C_NACK);
		if(commu_state != COMMU_OK) {
			bmp180_stop();
			printf("timeout\r\n");
			return commu_state;
		}
	}
	else {
		// oss 1부터는 xlsb 사용
		commu_state = bmp180_rx(&output_datas[BMP_LSB], I2C_ACK);
		if(commu_state != COMMU_OK) {
			bmp180_stop();
			printf("timeout\r\n");
			return commu_state;
		}

		commu_state = bmp180_rx(&output_datas[BMP_XLSB], I2C_NACK);
		if(commu_state != COMMU_OK) {
			bmp180_stop();
			printf("timeout\r\n");
			return commu_state;
		}
	}

	bmp180_stop();


	// xlsb는 oss의 단계에 따라 상위 비트만 사용

	long uncomp_pressure = output_datas[BMP_MSB] << 16 | output_datas[BMP_LSB] << 8 | output_datas[BMP_XLSB] >> (8 - BMP_info.oss);

	long pressure = calc_pressure(uncomp_pressure);
	printf("up:%ld p:%ld.%ld \r\n", uncomp_pressure, pressure);

	return commu_state;

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


	while(true) {

		if(TIM11_1ms_counter > 1500) {
			TIM11_1ms_counter = 0;

			bmp180_read_temperature();
			bmp180_read_pressure();

		}
	}
}


void bmp180_start() {

	BMP_step = BMP_STEP_START;

#if ACCESS_MODE == USE_DMA
	I2C_start(&I2C_setting);

#else

	bmp180_set_sda_mode(DEF_MODE_OUTPUT);

	//	SCL 이 HIGH 상태일 때
	HAL_GPIO_WritePin(BMP_SCL_PORT, BMP_SCL_PIN, SET);
	HAL_GPIO_WritePin(BMP_SDA_PORT, BMP_SDA_PIN, SET);

	//	SDA가 하강 하면서 시작
	HAL_GPIO_WritePin(BMP_SDA_PORT, BMP_SDA_PIN, RESET);

	// SCL이 상승에서 데이터를 받도록 클럭은 하강 상태에서 시작
	HAL_GPIO_WritePin(BMP_SCL_PORT, BMP_SCL_PIN, RESET);

#endif

}


void bmp180_stop() {

	BMP_step = BMP_STEP_STOP;

#if ACCESS_MODE == USE_DMA
	I2C_stop(&I2C_setting);

#else

	bmp180_set_sda_mode(DEF_MODE_OUTPUT);

	HAL_GPIO_WritePin(BMP_SDA_PORT, BMP_SDA_PIN, RESET);

	//	SCL 이 HIGH 상태일 때
	HAL_GPIO_WritePin(BMP_SCL_PORT, BMP_SCL_PIN, SET);

	//	SDA가 HIGH로 하여 STOP
	HAL_GPIO_WritePin(BMP_SDA_PORT, BMP_SDA_PIN, SET);

#endif




}

t_I2C_COMM_state bmp180_tx(uint8_t value) {

#if ACCESS_MODE == USE_DMA

	t_I2C_COMM_state comm_state = I2C_COMM_STATE_OK;

	comm_state = I2C_transmit_byte(&I2C_setting, value);
	if(comm_state != I2C_COMM_STATE_OK) {
		return comm_state;
	}

	comm_state = I2C_ack_slave(&I2C_setting);
	if(comm_state != I2C_COMM_STATE_OK) {
		return comm_state;
	}



#else
	// output mode 설정
	bmp180_set_sda_mode(DEF_MODE_OUTPUT);

	// send data
	// msb to lsb
	for(int i = 7; i >= 0; --i){

		if(value & (1 << i)) {
			HAL_GPIO_WritePin(BMP_SDA_PORT, BMP_SDA_PIN, SET);
		}
		else {
			HAL_GPIO_WritePin(BMP_SDA_PORT, BMP_SDA_PIN, RESET);
		}

		bmp180_clock();
	}

	return bmp180_acks();
#endif


}



t_I2C_COMM_state bmp180_rx(uint8_t* out_value, uint8_t in_ackm) {

#if ACCESS_MODE == USE_DMA
	t_I2C_COMM_state comm_state = I2C_COMM_STATE_OK;

	comm_state = I2C_receive_byte(&I2C_setting, out_value);
	if(comm_state != I2C_COMM_STATE_OK) {
		return comm_state;
	}


	comm_state = I2C_ack_master(&I2C_setting, in_ackm);
	if(comm_state != I2C_COMM_STATE_OK) {
		return comm_state;
	}

	return comm_state;
#else
	bmp180_set_sda_mode(DEF_MODE_INPUT);

	// send data
	// msb ? lsb?

	*out_value = 0;

	for(int i = 7; i >= 0; --i){

		if(HAL_GPIO_ReadPin(BMP_SDA_PORT, BMP_SDA_PIN)) {

			*out_value |= 1 << i;
		}

		bmp180_clock();
	}

	// check data

	// ack m

	bmp180_ackm(in_ackm);
#endif


	return I2C_COMM_STATE_OK;
}

// ack of slave
t_I2C_COMM_state bmp180_acks() {

#if ACCESS_MODE == USE_DMA

	return I2C_ack_slave(&I2C_setting);


#else
	bmp180_set_sda_mode(DEF_MODE_INPUT);

	// 이렇게 클럭을 치고 받으면 안됨
	//bmp180_clock();

	HAL_GPIO_WritePin(BMP_SCL_PORT, BMP_SCL_PIN, SET);

	// ack ( 0)
	uint32_t start = HAL_GetTick();
	while(HAL_GPIO_ReadPin(BMP_SDA_PORT, BMP_SDA_PIN)) {

		delay_us(10);
		uint32_t now = HAL_GetTick();
		if(now - start > I2C_TIMEOUT) {
			return I2C_COMM_STATE_TIMEOUT;
		}
	}

	HAL_GPIO_WritePin(BMP_SCL_PORT, BMP_SCL_PIN, RESET);
#endif






	return I2C_COMM_STATE_OK;

}

// ack of master
void bmp180_ackm(uint8_t in_ackm) {

#if ACCESS_MODE == USE_DMA
	I2C_ack_master(&I2C_setting, in_ackm);

#else
	bmp180_set_sda_mode(DEF_MODE_OUTPUT);

	ITStatus pin_state = in_ackm ? SET : RESET;
	HAL_GPIO_WritePin(BMP_SDA_PORT, BMP_SDA_PIN, pin_state);
	bmp180_clock();
#endif



}

void bmp180_clock() {

	// start에서 clock이 하강 상태에서 시작하므로
	// h to l
#if ACCESS_MODE == USE_DMA
	I2C_clock_tick(&I2C_setting.gpio_infos[I2C_SCL], I2C_setting.sampling_type);

#else
	HAL_GPIO_WritePin(BMP_SCL_PORT, BMP_SCL_PIN, SET);
	HAL_GPIO_WritePin(BMP_SCL_PORT, BMP_SCL_PIN, RESET);
#endif

}

void bmp180_set_sda_mode(t_I2C_gpio_mode gpio_mode) {

	// input으로 설정시 내부적으로 pull config에 따라 odr을 변경함
#if ACCESS_MODE == USE_DMA
	I2C_setting.gpio_infos[I2C_SDA].gpio_mode = gpio_mode;
	I2C_set_gpio_mode(&I2C_setting.gpio_infos[I2C_SDA]);

#else

	GPIO_InitTypeDef GPIO_InitStruct = {0};  // 초기화 구조체
	__HAL_RCC_GPIOB_CLK_ENABLE();  // 포트 클럭 인가 (필수)

	GPIO_InitStruct.Pin = BMP_SDA_PIN;

	if(gpio_mode == I2C_MODE_OUTPUT) {
		// output 설정
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 출력, Push-Pull
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	}
	else {

		// input 설정
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
	}

	HAL_GPIO_Init(BMP_SDA_PORT, &GPIO_InitStruct);

#endif


}




