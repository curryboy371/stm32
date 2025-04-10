/*
 * def.h
 *
 *  Created on: Mar 31, 2025
 *      Author: microsoft
 */

#ifndef INC_DEF_H_
#define INC_DEF_H_




#define TRUE 1
#define FALSE 0
#define true 1
#define false 0

#define LOW 0
#define HIGH 1


typedef enum {

	MODULE_LCD,

	MODULE_DTOMATRIX,
	MODULE_STEPMOTOR,
	MODULE_DS1302,
	MODULE_BUZZER,
	MODULE_BUTTON,

	MODULE_MAX,


}t_module_id;

typedef enum {

	SM_STATE_IDLE,
	SM_STATE_FORWARD,
	SM_STATE_BACKWARD,

}t_setep_motor_state;


typedef struct print_option {

	uint8_t prt_rtc;
	uint8_t prt_led;
}t_print_option;

typedef enum {

	EDIR_0,
	EDIR_90,
	EDIR_180,
	EDIR_MAX,

} t_edirection;


#define I2C_TIMEOUT 100 // msec

#define DMA_GPIOA_BASE  0x40020000
#define DMA_GPIOB_BASE  0x40020400
#define DMA_GPIOC_BASE  0x40020800

#define DMA_IDR 0x10
#define DMA_ODR 0x14
#define DMA_PUPDR 0x0c
#define DMA_MODE 0x00

#define GPIOA_MODE  	(unsigned int*)(DMA_GPIOA_BASE + DMA_MODE)
#define GPIOA_IDR  		(unsigned int*)(DMA_GPIOA_BASE + DMA_IDR)
#define GPIOA_ODR  		(unsigned int*)(DMA_GPIOA_BASE + DMA_ODR)
#define GPIOA_PUPDR  	(unsigned int*)(DMA_GPIOA_BASE + DMA_PUPDR)

#define GPIOB_IDR  		(unsigned int*)(DMA_GPIOB_BASE + DMA_IDR)
#define GPIOB_ODR  		(unsigned int*)(DMA_GPIOB_BASE + DMA_ODR)


#define GPIOC_MODE  	(unsigned int*)(DMA_GPIOC_BASE + DMA_MODE)
#define GPIOC_IDR  		(unsigned int*)(DMA_GPIOC_BASE + DMA_IDR)
#define GPIOC_ODR  		(unsigned int*)(DMA_GPIOC_BASE + DMA_ODR)
#define GPIOC_PUPDR  	(unsigned int*)(DMA_GPIOC_BASE + DMA_PUPDR)



#endif /* INC_DEF_H_ */
