
/////////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "output.h"
#include "board_drivers/hardware_def.h"
#include "peripheral_drivers/gpio/gpio_driver.h"

/////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    unsigned char Enable;

}gpdo_t;

/////////////////////////////////////////////////////////////////////////////////////////////

static gpdo_t Gpdo1;
static gpdo_t Gpdo2;
static gpdo_t Gpdo3;
static gpdo_t Gpdo4;
static gpdo_t ReleAux;
static gpdo_t ReleExtItlk;

/////////////////////////////////////////////////////////////////////////////////////////////

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

    Gpdo1.Enable = 0;
    Gpdo2.Enable = 0;
    Gpdo3.Enable = 0;
    Gpdo4.Enable = 0;

    ReleAux.Enable = 0;

    ReleExtItlk.Enable = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo1Enable(void)
{
    Gpdo1.Enable = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo1Disable(void)
{
    Gpdo1.Enable = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo2Enable(void)
{
    Gpdo2.Enable = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo2Disable(void)
{
    Gpdo2.Enable = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo3Enable(void)
{
    Gpdo3.Enable = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo3Disable(void)
{
    Gpdo3.Enable = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo4Enable(void)
{
    Gpdo4.Enable = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo4Disable(void)
{
    Gpdo4.Enable = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ReleAuxEnable(void)
{
    ReleAux.Enable = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ReleAuxDisable(void)
{
    ReleAux.Enable = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ReleExtItlkEnable(void)
{
    ReleExtItlk.Enable = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ReleExtItlkDisable(void)
{
    ReleExtItlk.Enable = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo1TurnOn(void)
{
    if(Gpdo1.Enable) set_pin(GPDO_1_BASE, GPDO_1_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo1TurnOff(void)
{
    if(Gpdo1.Enable) clear_pin(GPDO_1_BASE, GPDO_1_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdo1Sts(void)
{
    if(Gpdo1.Enable) return read_pin(GPDO_1_BASE, GPDO_1_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo2TurnOn(void)
{
    if(Gpdo2.Enable) set_pin(GPDO_2_BASE, GPDO_2_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo2TurnOff(void)
{
    if(Gpdo2.Enable) clear_pin(GPDO_2_BASE, GPDO_2_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdo2Sts(void)
{
    if(Gpdo2.Enable) return read_pin(GPDO_2_BASE, GPDO_2_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo3TurnOn(void)
{
    if(Gpdo3.Enable) set_pin(GPDO_3_BASE, GPDO_3_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo3TurnOff(void)
{
    if(Gpdo3.Enable) clear_pin(GPDO_3_BASE, GPDO_3_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdo3Sts(void)
{
    if(Gpdo3.Enable) return read_pin(GPDO_3_BASE, GPDO_3_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo4TurnOn(void)
{
    if(Gpdo4.Enable) set_pin(GPDO_4_BASE, GPDO_4_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Gpdo4TurnOff(void)
{
    if(Gpdo4.Enable) clear_pin(GPDO_4_BASE, GPDO_4_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdo4Sts(void)
{
    if(Gpdo4.Enable) return read_pin(GPDO_4_BASE, GPDO_4_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ReleAuxTurnOn(void)
{
    if(ReleAux.Enable) set_pin(RELAY_1_BASE, RELAY_1_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ReleAuxTurnOff(void)
{
    if(ReleAux.Enable) clear_pin(RELAY_1_BASE, RELAY_1_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char ReleAuxSts(void)
{
    if(ReleAux.Enable) return read_pin(RELAY_1_BASE, RELAY_1_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ReleExtItlkTurnOn(void)
{
    if(ReleExtItlk.Enable) set_pin(RELAY_2_BASE, RELAY_2_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ReleExtItlkTurnOff(void)
{
    if(ReleExtItlk.Enable) clear_pin(RELAY_2_BASE, RELAY_2_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char ReleExtItlkSts(void)
{
    if(ReleExtItlk.Enable) return read_pin(RELAY_2_BASE, RELAY_2_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////


