/*
 * dh11.h
 *
 *  Created on: Mar 31, 2025
 *      Author: microsoft
 */

#ifndef INC_DH11_H_
#define INC_DH11_H_

#include "main.h"

typedef enum state_define {

	OK,
	TIMEOUT,
	VALUE_ERROR,
	TRANS_ERROR
}t_state_define;


void dh11_main();
void init_dht11();
void get_dht11_data(uint8_t outData[]);

void start_signal();

t_state_define check_dht_handshake();

t_state_define check_response_timeout(uint8_t inTimeOutCnt, uint8_t inReadValue);

t_state_define receive_dhc_data(uint8_t outData[]);


#endif /* INC_DH11_H_ */
