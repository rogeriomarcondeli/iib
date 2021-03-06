/*
 * i2c_driver.h
 *
 *  Created on: 05 de jun de 2020
 *      Author: Rogerio Jose Marcondeli
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DRIVERS_PERIPHERAL_DRIVERS_I2C_I2C_DRIVER_H_
#define DRIVERS_PERIPHERAL_DRIVERS_I2C_I2C_DRIVER_H_

/////////////////////////////////////////////////////////////////////////////////////////////

extern void InitI2C5(void);
extern void I2C5Send(uint8_t slave_addr, uint8_t num_of_args, ...);
extern uint32_t I2C5Receive(uint32_t slave_addr, uint8_t reg);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void InitI2C2(void);
extern void I2C2Send16Bits(uint8_t slave_addr, uint8_t num_of_args, ...);
extern uint16_t I2C2Receive16Bits(uint32_t slave_addr, uint8_t reg);

/////////////////////////////////////////////////////////////////////////////////////////////

#endif /* DRIVERS_PERIPHERAL_DRIVERS_I2C_I2C_DRIVER_H_ */

/////////////////////////////////////////////////////////////////////////////////////////////






