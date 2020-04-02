/*
 * i2c_driver.h
 *
 *  Created on: 14 de ago de 2017
 *      Author: allef.silva
 */

#ifndef DRIVERS_PERIPHERAL_DRIVERS_I2C_I2C_DRIVER_H_
#define DRIVERS_PERIPHERAL_DRIVERS_I2C_I2C_DRIVER_H_

extern void InitI2C5(void);
extern void I2C5Send(uint8_t slave_addr, uint8_t num_of_args, ...);
extern uint32_t I2C5Receive(uint32_t slave_addr, uint8_t reg);

extern void InitI2C2(void);

#endif /* DRIVERS_PERIPHERAL_DRIVERS_I2C_I2C_DRIVER_H_ */
