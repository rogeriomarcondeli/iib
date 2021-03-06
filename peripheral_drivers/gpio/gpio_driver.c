
/////////////////////////////////////////////////////////////////////////////////////////////

/*
 * gpio_driver.c
 *
 *  Created on: 14 de ago de 2017
 *      Author: allef.silva
 */
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "gpio_driver.h"

/////////////////////////////////////////////////////////////////////////////////////////////

#define ON  ~0
#define OFF 0

/////////////////////////////////////////////////////////////////////////////////////////////

void set_gpio_as_output(uint32_t base, uint8_t pins)
{
    GPIOPinTypeGPIOOutput(base, pins);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void set_gpio_as_input(uint32_t base, uint8_t pins)
{
    GPIOPinTypeGPIOInput(base, pins);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void toggle_pin(uint32_t base, uint8_t pins)
{
    uint32_t value = 0;

    value = GPIOPinRead(base, pins);
    value ^= pins;
    GPIOPinWrite(base, pins, value);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void set_pin(uint32_t base, uint8_t pins)
{
    GPIOPinWrite(base, pins, ON);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_pin(uint32_t base, uint8_t pins)
{
    GPIOPinWrite(base, pins, OFF);
}

/////////////////////////////////////////////////////////////////////////////////////////////

bool read_pin(uint32_t base, uint8_t pins)
{
    uint32_t value;
    value = GPIOPinRead(base, pins);
    if (value == pins) return true;
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////



