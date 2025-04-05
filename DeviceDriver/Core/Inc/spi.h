/*
 * spi.h
 *
 *  Created on: Apr 3, 2025
 *      Author: microsoft
 */


#ifndef INC_SPI_H_

#define INC_SPI_H_

#include "main.h"

void spi_init();
void spi_clock();
void spi_tx_data(uint8_t inData);


#endif // INC_SPI_H_
