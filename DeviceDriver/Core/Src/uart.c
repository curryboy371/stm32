/*
 * uart.c
 *
 *  Created on: Mar 31, 2025
 *      Author: microsoft
 */


#include "uart.h"
#include "def.h"
#include "extern.h"
#include <string.h>
#include <stdio.h>

#include "led.h"
#include "ds1302.h"



volatile uint8_t rx_buff[COMMAND_NUMBER][COMMAND_LENGTH];	// 2차원 arr로 변경

volatile uint32_t rear; // input index : HAL_UART_RxCpltCallback에서 집어 넣는 index
volatile uint32_t front; // output index :

t_print_option prt_option;

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @move from Drivers/STM32F4xx_HAL_driver/Src/stm32f4xx_hal_uart.c
  * ex) comportmaster로 부터 1char를 수신하면 HAL_UART_RxCpltCallback으로 진입
  * 9600   bps인 경우 HAL_UART_ReCpltCallback을 수행 후  1ms이내 빠져나가야함
  * 115200 bps인 경우 							    86us이내에 빠져나가야함
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
/*   Prevent unused argument(s) compilation warning
  UNUSED(huart);
   NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback could be implemented in the user file
   */


	// INT안에서 사용하는 것은 volatile 사용해줘야함
	static volatile uint8_t idx = 0;

	// uart2번인 경우에만 printf 하도록
	// address cmp
	if(huart == &huart2) {

		// new line 체크
		if(rx_data == '\n' || rx_data == '\r') {

			rx_buff[rear++][idx] = '\0';
			rear %= COMMAND_NUMBER;
			idx = 0;

			// check queue full
		}
		else {
			// check command length
			rx_buff[rear][idx++] = rx_data;
		}

		// 반드시 실행해야 다음 INT가 발생함
		HAL_UART_Receive_IT(&huart2, &rx_data, 1);
	}
}

void pc_command_processing() {

	// todo enum...


	static const char* ledStr[] = {
		"led_all_on",
		"led_all_off",
		"shift_left_ledon",
		"shift_right_ledon",
		"shift_left_keep_led_on",
		"shift_right_keep_led_on",
		"flower_ledon",
		"flower_ledoff",
	};


	static const void (*ledFunc[]) () = {
		led_all_on,
		led_all_off,
		shift_left_ledon,
		shift_right_ledon,
		shift_left_keep_led_on,
		shift_right_keep_led_on,
		flower_ledon,
		flower_ledoff,
	};


	if(front != rear) { // rx_buffer에 data 존재

		printf("%s\n", (char*)rx_buff[front]); // rx_buffer
		size_t rx_len = strlen((const char*)rx_buff[front]);
		size_t command_len = 0;
		for(int i = 0; i < 8; ++i) {
			command_len = strlen(ledStr[i]);
			if(rx_len == command_len) {

				if(strncmp((const char*)rx_buff[front], ledStr[i], rx_len) == 0){

					ledFunc[i]();
					break;
				}
			}
		}


		if(strncmp((const char*)rx_buff[front], "setrtc", strlen("setrtc")) == 0) {

			command_len = strlen("setrtc");
			if(rx_len >= command_len + 12/*yymmddhhmmss*/) {

				char* rtc = (char*)&rx_buff[front][command_len];
				set_rtc(rtc);

			}
		}
		else if(strncmp((const char*)rx_buff[front], "help", strlen("help")) == 0) {

			command_len = strlen("help");
			if(rx_len >= command_len) {
				show_command();

			}
		}

		else if(strncmp((const char*)rx_buff[front], "print_rtc", strlen("print_rtc")) == 0) {

			command_len = strlen("print_rtc");
			if(rx_len >= command_len) {
				prt_option.prt_rtc = 1;
				printf("print_rtc on\r\n");
			}
		}
		else if(strncmp((const char*)rx_buff[front], "printoff_rtc", strlen("printoff_rtc")) == 0) {

			command_len = strlen("printoff_rtc");
			if(rx_len >= command_len) {
				prt_option.prt_rtc = 0;
				printf("print_rtc off\r\n");
			}
		}

		++front;
		front %= COMMAND_NUMBER;

		// check queue full

	}


}




void show_command() {

	char *command[] = {
		"setrtc",
		"print_rtc",
		"printoff_rtc",
		"help",
	};

	for(int i = 0; i < 4; ++i) {
		printf("\r\n==help==\r\n%s\r\n", command[i]);
	}

}



