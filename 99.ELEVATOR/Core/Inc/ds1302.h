/*
 * ds1302.h
 *
 *  Created on: Mar 28, 2025
 *      Author: microsoft
 */

#ifndef INC_DS1302_H_
#define INC_DS1302_H_

#include "main.h"
#include "def.h"


// RTC command를 define 해서 사용
// read는 write +1이므로 write만 define 함

#define ADDR_SECONDS  		0x80
#define ADDR_MINUTES  		0x82

#define ADDR_HOURS  		0x84
#define ADDR_DATES  		0x86
#define ADDR_MONTHS  		0x88
#define ADDR_DAYSOFWEEK 	0x8A
#define ADDR_YEARS 	 		0x8C
#define ADDR_WITEPROTRECTED 0x8E


#define PRINT_ON  1
#define PRINT_OFF 0

// time info struct of ds1302
typedef struct s_ds1302 {

	uint8_t year;
	uint8_t month;
	uint8_t date;
	uint8_t dayofweek;			// 1 :sun 2: mon...
	uint8_t hours;
	uint8_t minutes; 			//
	uint8_t seconds; 			// sec

	uint8_t ampm; 				// 1:pm 0:am
	uint8_t hourmode; 			// 0:24h 1:12h
}t_ds1302;



void ds1302_main();

void init_ds1302();

void init_date_time(t_ds1302* pds1302);

void init_gpio_ds1302();

void ds1302_update();

void set_rtc(char* data_time);
void set_print_mode(uint8_t inMode);


uint8_t check_alarm();


void read_time_ds1302(t_ds1302* pds1302);
void read_date_ds1302(t_ds1302* pds1302);

uint8_t read_ds1302(uint8_t addr);

const t_ds1302* ds1302_get_dsinfo();


void write_ds1302(uint8_t addr, uint8_t data);


void rx_ds1302(unsigned char* pdata);
void tx_ds1302(uint8_t value);
void clock_ds1302();


uint8_t bcd2dec(uint8_t bcd);
uint8_t dec2bcd(uint8_t dec);


void set_io_dataline_ds1302();
void output_dataline_ds1302();
void input_dataline_ds1302();

#endif /* INC_DS1302_H_ */
