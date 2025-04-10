/*
 * spi.h
 *
 *  Created on: Apr 3, 2025
 *      Author: microsoft
 */


#ifndef INC_SPI_H_

#define INC_SPI_H_

#include "main.h"

typedef enum {

	SPI_PIN_DS, // data
	SPI_PIN_SH, // clock
	SPI_PIN_ST,  // latch
	SPI_PIN_MAX

}t_spi_pin_type;

typedef struct {

	GPIO_TypeDef* gpio_port[SPI_PIN_MAX];
	uint16_t gpio_pin[SPI_PIN_MAX];

}t_spi_gpio_setting;

void spi_init(t_spi_gpio_setting* pspi_gpio);
void spi_clock(t_spi_gpio_setting* pspi_gpio);
void spi_tx_data(t_spi_gpio_setting* pspi_gpio, uint8_t inData);
void spi_letch(t_spi_gpio_setting* pspi_gpio);

#endif // INC_SPI_H_
