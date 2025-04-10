/*
 * ds1302.c
 *
 *  Created on: Mar 28, 2025
 *      Author: microsoft
 */

#include "ds1302.h"

#include <stdio.h>

#include "def.h"

#include "extern.h"
#include "uart.h"
#include "timer.h"

#include "i2c_lcd.h"

t_ds1302 ds1302;

t_ds1302* g_pds1302;

uint8_t isPrint = PRINT_ON;

void ds1302_main() {

	char buffer[12] = {0};


	g_pds1302 = &ds1302;



	init_ds1302(&ds1302);


	int writeCount = 0;
	while(1) {

		read_time_ds1302(&ds1302);
		read_date_ds1302(&ds1302);
		pc_command_processing();

		if(TIM11_1ms_counter >= 1000) {

			TIM11_1ms_counter = 0;
			++writeCount;

			//if(prt_option.prt_rtc) {
				// 날짜와 시각을 출력

/*				printf("20%d-%02d-%02d %02d:%02d:%02d\r\n",
						ds1302.year,
						ds1302.month,
						ds1302.date,
						ds1302.hours,
						ds1302.minutes,
						ds1302.seconds);*/
			//}

			sprintf(buffer, "20%d-%02d-%02d", ds1302.year, ds1302.month, ds1302.date);
			move_cursor(0,0);
			lcd_string((char*)buffer);

			sprintf(buffer, "%02d:%02d:%02d", ds1302.hours, ds1302.minutes, ds1302.seconds);
			move_cursor(1,0);
			lcd_string((char*)buffer);

		}

		if(writeCount > 30) {

			writeCount = 0;

			//flash_write_time((uint8_t*)(&ds1302), (uint32_t)sizeof(t_ds1302));

		}

	}

}


void init_ds1302() {

	g_pds1302 = &ds1302;
	init_gpio_ds1302();
	init_date_time(g_pds1302);


	write_ds1302(ADDR_SECONDS, g_pds1302->seconds);
	write_ds1302(ADDR_MINUTES, g_pds1302->minutes);
	write_ds1302(ADDR_HOURS, g_pds1302->hours);

	write_ds1302(ADDR_DATES, g_pds1302->date);
	write_ds1302(ADDR_MONTHS, g_pds1302->month);
	write_ds1302(ADDR_DAYSOFWEEK, g_pds1302->dayofweek);
	write_ds1302(ADDR_YEARS, g_pds1302->year);


}

void init_date_time(t_ds1302* pds1302) {

	pds1302->year = 25;
	pds1302->month = 4;
	pds1302->date = 10;

	pds1302->dayofweek = 6;

	pds1302->hourmode = 0;
	pds1302->ampm =0;

	pds1302->hours= 11;
	pds1302->minutes = 30;
	pds1302->seconds = 30;


	//flash_set_time((uint8_t*)pds1302, (uint32_t)sizeof(t_ds1302));

}

void init_gpio_ds1302() {

	// 모든 gpio를 low로
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 0);
	HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 0);
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, 0);

}

void ds1302_update() {

	if(TIM11_1ms_delay_counter[MODULE_DS1302] > 0) {
		return;
	}


	read_time_ds1302(g_pds1302);
	read_date_ds1302(g_pds1302);


	TIM11_1ms_delay_counter[MODULE_DS1302] = 1000;


}


void set_rtc(char* data_time) {

	//YYMMDDHHMMSS
	//char* iter = data_time;

	int  index = 0;

	int ten, one;
	ten = data_time[index++] - '0';
	one = data_time[index++] - '0';
	g_pds1302->year = ten * 10 + one;

	ten = data_time[index++] - '0';
	one = data_time[index++] - '0';
	g_pds1302->month = ten * 10 + one;

	ten = data_time[index++] - '0';
	one = data_time[index++] - '0';
	g_pds1302->date = ten * 10 + one;

	ten = data_time[index++] - '0';
	one = data_time[index++] - '0';
	g_pds1302->hours = ten * 10 + one;

	ten = data_time[index++] - '0';
	one = data_time[index++] - '0';
	g_pds1302->minutes = ten * 10 + one;

	ten = data_time[index++] - '0';
	one = data_time[index++] - '0';
	g_pds1302->seconds = ten * 10 + one;


/*	g_pds1302->year = ((*iter++) - '0') * 10 + (*iter++) - '0';
	g_pds1302->month = ((*iter++) - '0') * 10 + (*iter++) - '0';
	g_pds1302->date = ((*iter++) - '0') * 10 + (*iter++) - '0';
	g_pds1302->hours = ((*iter++) - '0') * 10 + (*iter++) - '0';
	g_pds1302->minutes = ((*iter++) - '0') * 10 + (*iter++) - '0';
	g_pds1302->seconds = ((*iter++) - '0') * 10 + (*iter++) - '0';
*/

	init_ds1302(g_pds1302);
}

void set_print_mode(uint8_t inMode) {

	isPrint = inMode;

}

void read_time_ds1302(t_ds1302* pds1302) {

	pds1302->seconds = read_ds1302(ADDR_SECONDS);
	pds1302->minutes = read_ds1302(ADDR_MINUTES);
	pds1302->hours = read_ds1302(ADDR_HOURS);


}


void read_date_ds1302(t_ds1302* pds1302) {

	pds1302->date = read_ds1302(ADDR_DATES);
	pds1302->month = read_ds1302(ADDR_MONTHS);
	pds1302->dayofweek = read_ds1302(ADDR_DAYSOFWEEK);
	pds1302->year = read_ds1302(ADDR_YEARS);
}

uint8_t read_ds1302(uint8_t addr) {

	unsigned char data8bits=  0; // 1bit씩 넘어온 것을 담을 그릇

	// 1. CE enable l to h
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 1);

	// 2. addr 전송 ( 내부에서 data port output으로 변경)
	tx_ds1302(addr + 1);


	// 3 data read ( 내부에서 data port input으로 변경)
	rx_ds1302(&data8bits);

	// 4. CE disable h to l
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 0);


	// 5. bcd to dec convert
	// bcd ? BCD (Binary-Coded Decimal) 10진수 각 자리를 2진수로 표현한거임
	return bcd2dec(data8bits);

}

const t_ds1302* ds1302_get_dsinfo() {

	return g_pds1302;
}


void write_ds1302(uint8_t addr, uint8_t data) {


	// 1 .CE enable l to h
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 1);
	//CE_DS1302_GPIO_Port->ODR |= (1 << CE_DS1302_Pin);

	// 2. addr send
	tx_ds1302(addr);

	// 3. data send ( data - dec to bcd )
	tx_ds1302(dec2bcd(data));

	// 4. CE disable h to l
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 0);
	//CE_DS1302_GPIO_Port->ODR &= ~(1 << CE_DS1302_Pin);

}

void rx_ds1302(unsigned char* pdata) {

	unsigned char temp = 0;

	// gpio data pin ddr inputmode로 설정
	input_dataline_ds1302();


	// DS1302로부터 들어온 bit를 LSB부터 8bit를 받음
	// 데이터를 읽는 경우 마지막 bit는 클럭을 치지 않아도 됨 ( tx에서의 마지막 하강 클럭으로 첫번째 rx data를 읽기 때문
	for(int i = 0 ; i < 8; ++i) {

		// 1bit를 읽음
		// 0번째 자리부터 |연산으로 temp에 입력하므로 들어오는 데이터를 거꾸로 저장함
		if(HAL_GPIO_ReadPin(IO_DS1302_GPIO_Port, IO_DS1302_Pin)) {

			// 1 조건만 처리
			temp |= 1 << i;
		}

		// 읽는경우 마지막 rx bit에서는 클럭을 치지 않아도 됨
		// tx의 마지막 클럭 하강 구간에서 rx  첫번째 bit를 읽기 때문
		if(i != 7) {
			clock_ds1302();
		}
	}

	*pdata = temp;

}

void tx_ds1302(uint8_t value) {

	// gpio output 설정 필요 ( 수시로 바뀌기 때문에 항상 전송 전에 ddr설정해주기 )
	output_dataline_ds1302();

	// ex) addr 초를 write
	// 80h M         L

	// write는 lsb부터 전송되어야함
	//     1000 0000 ( 실제값)
	//	   0000 0001 ( 전송할 값)

	// & 마스킹 연산자로 처리
	// 1000 0000 (실제 값) & mask
 	// 0000 0001 (mask) = 0
	// 0000 0010 (mask) = 0
	// 0000 0100 (mask) = 0
	// 1000 0000 (mask) = 1000 0000 => 1로


	for(int i = 0; i < 8; ++i) {

		if(value & (1 << i)) {
			HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 1);
		}
		else {
			HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 0);
		}

		//클락을 직접 쳐주네?
		clock_ds1302();
	}


}

void clock_ds1302() {

	// 클락 시간이 얼마나 머물러 있고 이런건 없음
	// 그냥 h - to l로 직접 쳐주기만 하면 됨
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, 1);
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, 0);

}

uint8_t bcd2dec(uint8_t bcd) {

	// bcd 값을 dec으로 변경하는 함수

	// 입력 bcd

	// ex 25년
	// 7654 3210
	// 0010 0101  (2 5)

	uint8_t high, low;

	low = bcd & 0x0f;
	high = (bcd >> 4) * 10; // 00100101 >>4 = 0010 (2)

	return high+low;

}

uint8_t dec2bcd(uint8_t dec) {

	// dec을 bcd로 변경하는 함수

	// 입력 ded

	// ex 25년
	//     7654 3210
	// dec 0001 1001  (2 5)
	// bcd 0010 0101  (2 | 5)


	uint8_t high = (dec / 10) << 4; // 100의자리까지 오지 않으니 %는 안하는듯
	uint8_t low = dec % 10;

	return high + low;

}


void set_io_dataline_ds1302() {



}

void output_dataline_ds1302() {

	// ddr을 output 모드로 만들기
	GPIO_InitTypeDef GPIO_init = {0};

	GPIO_init.Pin = IO_DS1302_Pin;
	GPIO_init.Mode = GPIO_MODE_OUTPUT_PP; // output mode

	GPIO_init.Pull = GPIO_NOPULL; // pull up no
	GPIO_init.Speed = GPIO_SPEED_FREQ_HIGH; // low:2Mhz high:25~100Mhz
	HAL_GPIO_Init(IO_DS1302_GPIO_Port, &GPIO_init);

}

void input_dataline_ds1302() {
	// ddr을 output 모드로 만들기
	GPIO_InitTypeDef GPIO_init = {0};

	GPIO_init.Pin = IO_DS1302_Pin;
	GPIO_init.Mode = GPIO_MODE_INPUT; // input mode
	GPIO_init.Pull = GPIO_NOPULL; // pull up no
	// input은 speed 없어도 됨
	//GPIO_init.Speed = GPIO_SPEED_FREQ_HIGHHIGH; // low:2Mhz high:25~100Mhz
	HAL_GPIO_Init(IO_DS1302_GPIO_Port, &GPIO_init);


}

