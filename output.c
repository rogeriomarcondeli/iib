
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#include "output.h"
#include "board_drivers/hardware_def.h"
#include "peripheral_drivers/gpio/gpio_driver.h"


void OutputInit(void)
{
    
    set_gpio_as_output(RELAY_1_BASE, RELAY_1_PIN);
    set_gpio_as_output(RELAY_2_BASE, RELAY_2_PIN);

    set_gpio_as_output(GPDO_1_BASE, GPDO_1_PIN);
    set_gpio_as_output(GPDO_2_BASE, GPDO_2_PIN);
    set_gpio_as_output(GPDO_3_BASE, GPDO_3_PIN);
    set_gpio_as_output(GPDO_4_BASE, GPDO_4_PIN);

    set_gpio_as_output(TP_1_BASE, TP_1_PIN);
    set_gpio_as_output(TP_2_BASE, TP_2_PIN);
    set_gpio_as_output(TP_3_BASE, TP_3_PIN);

    set_gpio_as_output(STATUS_LED_BASE, STATUS_LED_PIN);

    clear_pin(RELAY_1_BASE, RELAY_1_PIN);
    clear_pin(RELAY_2_BASE, RELAY_2_PIN);
    
    clear_pin(GPDO_1_BASE, GPDO_1_PIN);
    clear_pin(GPDO_2_BASE, GPDO_2_PIN);
    clear_pin(GPDO_3_BASE, GPDO_3_PIN);
    clear_pin(GPDO_4_BASE, GPDO_4_PIN);
    
    clear_pin(TP_1_BASE, TP_1_PIN);
    clear_pin(TP_2_BASE, TP_2_PIN);
    clear_pin(TP_3_BASE, TP_3_PIN);

    clear_pin(STATUS_LED_BASE, STATUS_LED_PIN);

}

void Gpdo1TurnOn(void)
{
    set_pin(GPDO_1_BASE, GPDO_1_PIN);
}

void Gpdo1TurnOff(void)
{
    clear_pin(GPDO_1_BASE, GPDO_1_PIN);
}

unsigned char Gpdo1Sts(void)
{
    return read_pin(GPDO_1_BASE, GPDO_1_PIN);
}

void Gpdo2TurnOn(void)
{
    set_pin(GPDO_2_BASE, GPDO_2_PIN);
}

void Gpdo2TurnOff(void)
{
    clear_pin(GPDO_2_BASE, GPDO_2_PIN);
}

unsigned char Gpdo2Sts(void)
{
    return read_pin(GPDO_2_BASE, GPDO_2_PIN);
}

void Gpdo3TurnOn(void)
{
    set_pin(GPDO_3_BASE, GPDO_3_PIN);
}

void Gpdo3TurnOff(void)
{
    clear_pin(GPDO_3_BASE, GPDO_3_PIN);
}

unsigned char Gpdo3Sts(void)
{
    return read_pin(GPDO_3_BASE, GPDO_3_PIN);
}

void Gpdo4TurnOn(void)
{
    set_pin(GPDO_4_BASE, GPDO_4_PIN);
}

void Gpdo4TurnOff(void)
{
    clear_pin(GPDO_4_BASE, GPDO_4_PIN);
}

unsigned char Gpdo4Sts(void)
{
    return read_pin(GPDO_4_BASE, GPDO_4_PIN);
}

void ReleAuxTurnOn(void)
{
    set_pin(RELAY_1_BASE, RELAY_1_PIN);
}

void ReleAuxTurnOff(void)
{
    clear_pin(RELAY_1_BASE, RELAY_1_PIN);
}

unsigned char ReleAuxSts(void)
{
    return read_pin(RELAY_1_BASE, RELAY_1_PIN);
}

void ReleItlkTurnOn(void)
{
    set_pin(RELAY_2_BASE, RELAY_2_PIN);
}

void ReleItlkTurnOff(void)
{
    clear_pin(RELAY_2_BASE, RELAY_2_PIN);
}

unsigned char ReleItlkSts(void)
{
    return read_pin(RELAY_2_BASE, RELAY_2_PIN);
}

