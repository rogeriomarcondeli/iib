
/////////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "input.h"
#include "board_drivers/hardware_def.h"
#include "peripheral_drivers/gpio/gpio_driver.h"

/////////////////////////////////////////////////////////////////////////////////////////////

static unsigned char BoardAddress = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

static unsigned char ErrorDriver1Config = 0;
static unsigned char ErrorDriver2Config = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char BoardAddressRead(void)
{
     BoardAddress = 0;

     if(read_pin(CAN_ADD_0_BASE, CAN_ADD_0_PIN)) BoardAddress |= 0b00000001;
     if(read_pin(CAN_ADD_1_BASE, CAN_ADD_1_PIN)) BoardAddress |= 0b00000010;
     if(read_pin(CAN_ADD_2_BASE, CAN_ADD_2_PIN)) BoardAddress |= 0b00000100;
     if(read_pin(CAN_ADD_3_BASE, CAN_ADD_3_PIN)) BoardAddress |= 0b00001000;
     if(read_pin(CAN_ADD_4_BASE, CAN_ADD_4_PIN)) BoardAddress |= 0b00010000;

     BoardAddress ^= 0b11111;
     BoardAddress &= 0b00011111;

     return BoardAddress;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void InputInit(void)
{
    
    // GPDI
    set_gpio_as_input(GPDI_1_BASE, GPDI_1_PIN);
    set_gpio_as_input(GPDI_2_BASE, GPDI_2_PIN);
    set_gpio_as_input(GPDI_3_BASE, GPDI_3_PIN);
    set_gpio_as_input(GPDI_4_BASE, GPDI_4_PIN);
    set_gpio_as_input(GPDI_5_BASE, GPDI_5_PIN);
    set_gpio_as_input(GPDI_6_BASE, GPDI_6_PIN);
    set_gpio_as_input(GPDI_7_BASE, GPDI_7_PIN);
    set_gpio_as_input(GPDI_8_BASE, GPDI_8_PIN);
    set_gpio_as_input(GPDI_9_BASE, GPDI_9_PIN);
    set_gpio_as_input(GPDI_10_BASE, GPDI_10_PIN);
    set_gpio_as_input(GPDI_11_BASE, GPDI_11_PIN);
    set_gpio_as_input(GPDI_12_BASE, GPDI_12_PIN);
    
    //Driver Status
    set_gpio_as_input(ERROR_DRIVER_1_TOP_BASE, ERROR_DRIVER_1_TOP_PIN);
    set_gpio_as_input(ERROR_DRIVER_1_BOT_BASE, ERROR_DRIVER_1_BOT_PIN);
    set_gpio_as_input(MODULE_1_OVER_TEMP_BASE, MODULE_1_OVER_TEMP_PIN);
    set_gpio_as_input(ERROR_DRIVER_2_TOP_BASE, ERROR_DRIVER_2_TOP_PIN);
    set_gpio_as_input(ERROR_DRIVER_2_BOT_BASE, ERROR_DRIVER_2_BOT_PIN);
    set_gpio_as_input(MODULE_2_OVER_TEMP_BASE, MODULE_2_OVER_TEMP_PIN);

    //CAN Address
    set_gpio_as_input(CAN_ADD_0_BASE, CAN_ADD_0_PIN);
    set_gpio_as_input(CAN_ADD_1_BASE, CAN_ADD_1_PIN);
    set_gpio_as_input(CAN_ADD_2_BASE, CAN_ADD_2_PIN);
    set_gpio_as_input(CAN_ADD_3_BASE, CAN_ADD_3_PIN);
    set_gpio_as_input(CAN_ADD_4_BASE, CAN_ADD_4_PIN);

}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi1Read(void)
{
    return read_pin(GPDI_1_BASE, GPDI_1_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi2Read(void)
{
    return read_pin(GPDI_2_BASE, GPDI_2_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi3Read(void)
{
    return read_pin(GPDI_3_BASE, GPDI_3_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi4Read(void)
{
    return read_pin(GPDI_4_BASE, GPDI_4_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi5Read(void)
{
    return read_pin(GPDI_5_BASE, GPDI_5_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi6Read(void)
{
    return read_pin(GPDI_6_BASE, GPDI_6_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi7Read(void)
{
    return read_pin(GPDI_7_BASE, GPDI_7_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi8Read(void)
{
    return read_pin(GPDI_8_BASE, GPDI_8_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////
unsigned char Gpdi9Read(void)
{
    return read_pin(GPDI_9_BASE, GPDI_9_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi10Read(void)
{
    return read_pin(GPDI_10_BASE, GPDI_10_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi11Read(void)
{
    return read_pin(GPDI_11_BASE, GPDI_11_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Gpdi12Read(void)
{
    return read_pin(GPDI_12_BASE, GPDI_12_PIN);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Driver1ErrEnable(void)
{
    ErrorDriver1Config = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Driver1ErrDisable(void)
{
    ErrorDriver1Config = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Driver2ErrEnable(void)
{
    ErrorDriver2Config = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Driver2ErrDisable(void)
{
    ErrorDriver2Config = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Driver1TopErrRead(void)
{
    if(ErrorDriver1Config) return read_pin(ERROR_DRIVER_1_TOP_BASE, ERROR_DRIVER_1_TOP_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Driver1BotErrRead(void)
{
    if(ErrorDriver1Config) return read_pin(ERROR_DRIVER_1_BOT_BASE, ERROR_DRIVER_1_BOT_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Driver1OverTempRead(void)
{
    if(ErrorDriver1Config) return !read_pin(MODULE_1_OVER_TEMP_BASE, MODULE_1_OVER_TEMP_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Driver2TopErrRead(void)
{
    if(ErrorDriver2Config)return read_pin(ERROR_DRIVER_2_TOP_BASE, ERROR_DRIVER_2_TOP_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Driver2BotErrRead(void)
{
    if(ErrorDriver2Config)return read_pin(ERROR_DRIVER_2_BOT_BASE, ERROR_DRIVER_2_BOT_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char Driver2OverTempRead(void)
{
    if(ErrorDriver2Config) return !read_pin(MODULE_2_OVER_TEMP_BASE, MODULE_2_OVER_TEMP_PIN);
    else return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////
