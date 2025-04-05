/*
 * i2c.h
 *
 *  Created on: Apr 5, 2025
 *      Author: pscsp
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_

#include "main.h"

//https://github.com/bitbank2/BitBang_I2C/blob/master/src/BitBang_I2C.h


//void I2CInit(BBI2C *pI2C, unsigned int iClock);
void I2C_init();

//void I2CScan(BBI2C *pI2C, unsigned char *pMap);
void I2C_scan();


//int I2CRead(BBI2C *pI2C, uint8_t iAddr, uint8_t *pData, int iLen);
void I2C_read();

//int I2CReadRegister(BBI2C *pI2C, unsigned char iAddr, unsigned char u8Register, unsigned char *pData, int iLen);
void I2C_read_register();

//int I2CWrite(BBI2C *pI2C, unsigned char iAddr, unsigned char *pData, int iLen);
void I2C_write();



// i2c read

// i2c read register

// i2c write

// i2c scan

// i2c init

#endif /* INC_I2C_H_ */
