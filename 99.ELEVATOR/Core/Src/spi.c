/*
 * spi.c
 *
 *  Created on: Apr 3, 2025
 *      Author: microsoft
 */

#include "spi.h"
#include "def.h"
#include "extern.h"
#include "timer.h"

void spi_init(t_spi_gpio_setting* pspi_gpio) {

	if(!pspi_gpio) {
		return;
	}

	for(int i = 0; i < SPI_PIN_MAX; ++i) {
		pspi_gpio->gpio_port[i]->ODR &= ~pspi_gpio->gpio_pin[i];
	}

	HAL_Delay(10);
}

void spi_clock(t_spi_gpio_setting* pspi_gpio) {

	if(!pspi_gpio) {
		return;
	}

	pspi_gpio->gpio_port[SPI_PIN_SH]->ODR |= pspi_gpio->gpio_pin[SPI_PIN_SH];
	delay_us(5);
	pspi_gpio->gpio_port[SPI_PIN_SH]->ODR &= ~(pspi_gpio->gpio_pin[SPI_PIN_SH]);
	delay_us(5);
}

void spi_tx_data(t_spi_gpio_setting* pspi_gpio, uint8_t inData) {

	if(!pspi_gpio) {
		return;
	}


	for(int i = 7; i >= 0; --i) {

		// msb부터
		if(inData & 1 << i) {

			pspi_gpio->gpio_port[SPI_PIN_DS]->ODR |= pspi_gpio->gpio_pin[SPI_PIN_DS];
		}
		else {
			pspi_gpio->gpio_port[SPI_PIN_DS]->ODR &= ~(pspi_gpio->gpio_pin[SPI_PIN_DS]);
		}

		// clock
		spi_clock(pspi_gpio);
	}

}

void spi_letch(t_spi_gpio_setting* pspi_gpio) {

	if(!pspi_gpio) {
		return;
	}

	delay_us(5);
	pspi_gpio->gpio_port[SPI_PIN_ST]->ODR &= ~(pspi_gpio->gpio_pin[SPI_PIN_ST]);
	pspi_gpio->gpio_port[SPI_PIN_ST]->ODR |= (pspi_gpio->gpio_pin[SPI_PIN_ST]);

	delay_us(5);
	//HAL_Delay(1);

}


