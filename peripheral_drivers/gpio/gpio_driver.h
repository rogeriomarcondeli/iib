/*
 * gpio_driver.h
 *
 *  Created on: 14 de ago de 2017
 *      Author: allef.silva
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DRIVERS_PERIPHERAL_DRIVERS_GPIO_DRIVER_H_
#define DRIVERS_PERIPHERAL_DRIVERS_GPIO_DRIVER_H_

/////////////////////////////////////////////////////////////////////////////////////////////

void set_gpio_as_output(uint32_t, uint8_t);
void set_gpio_as_input(uint32_t, uint8_t);
void toggle_pin(uint32_t, uint8_t);
void set_pin(uint32_t, uint8_t);
bool read_pin(uint32_t, uint8_t);
void clear_pin(uint32_t, uint8_t);

/////////////////////////////////////////////////////////////////////////////////////////////

#endif /* DRIVERS_PERIPHERAL_DRIVERS_GPIO_DRIVER_H_ */

/////////////////////////////////////////////////////////////////////////////////////////////
