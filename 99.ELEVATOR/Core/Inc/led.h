/*
 * led.h
 *
 *  Created on: Mar 26, 2025
 *      Author: microsoft
 */

#ifndef INC_LED_H_
#define INC_LED_H_




#define LED_PIN_ALL 	(GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7)




#include "main.h" // HAL driver GPIO 정보가 들어있기 때문에 include


void ledbar_init();
void led_all_on();

void led_all_off();

void led_test();

void shift_left_ledon(void);
void shift_right_ledon(void);

void shift_left_keep_led_on(void);
void shift_right_keep_led_on(void);

void flower_ledon(void);
void flower_ledoff(void);

void led_main();

void ledbar_render(uint8_t inData);


#endif /* INC_LED_H_ */
