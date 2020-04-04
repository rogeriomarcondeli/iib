
// Sensor: Si7005-B-FM1


#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"

#include "BoardTempHum.h"
#include "peripheral_drivers/timer/timer.h"
#include "peripheral_drivers/gpio/gpio_driver.h"
#include "peripheral_drivers/i2c/i2c_driver.h"

#include "board_drivers/hardware_def.h"
//#include "peripheral_drivers/gpio/gpio_driver.h"


#define SlaveAddress 0x40
#define RegisterAddress0 0x00
#define RegisterAddress1 0x01
#define RegisterAddress2 0x02
#define RegisterAddress3 0x03
#define RegisterAddress11 0x11

#define a0 (-4.7844)
#define a1  0.4008
#define a2 (-0.00393)
#define q0  0.1973
#define q1  0.00237


static unsigned char RelativeHumidity = 0;
static unsigned char Temperature = 0;

static unsigned char Start1 = 0x01;
static unsigned char Start2 = 0x11;

static unsigned char TempAlarm = 0;
static unsigned char TempTrip = 0;
static unsigned char TempAlarmLevel = 60;
static unsigned char TempTripLevel = 70;

static unsigned char RhAlarm = 0;
static unsigned char RhTrip = 0;
static unsigned char RhAlarmLevel = 80;
static unsigned char RhTripLevel = 90;


//******************************************************************************
// /* Read the temperature */
//******************************************************************************

void BoardTemperatureStartConversion(void)
{

  toggle_pin(TP_1_BASE, TP_1_PIN);

  I2C5Send(SlaveAddress, 2, RegisterAddress3, Start2);

  toggle_pin(TP_1_BASE, TP_1_PIN);

}

void BoardTemperatureRead(void)
{
    uint8_t Status=1;
    uint8_t TemperatureH;
    uint8_t TemperatureL;
    uint16_t Tempa;
    uint16_t Tempb;
    uint16_t Tempc;

    toggle_pin(TP_1_BASE, TP_1_PIN);

    while(Status==1)
    {

         Status = I2C5Receive(SlaveAddress, RegisterAddress0);

    }

    toggle_pin(TP_1_BASE, TP_1_PIN);

    TemperatureH = I2C5Receive(SlaveAddress, RegisterAddress1);

    toggle_pin(TP_1_BASE, TP_1_PIN);

    TemperatureL = I2C5Receive(SlaveAddress, RegisterAddress2);

    toggle_pin(TP_1_BASE, TP_1_PIN);

    Tempa = TemperatureH;
    Tempb = Tempa<<8;

    Tempb += TemperatureL;
    Tempc = Tempb>>2;

    Temperature = (Tempc/32) - 50;

    if(Temperature > TempAlarmLevel) TempAlarm = 1;
    if(Temperature > TempTripLevel) TempTrip = 1;
}

//******************************************************************************
// /* Read humidity. The 'compensate' variable contains the current temperature used to calculate the temperature compensation.*/
//******************************************************************************

void RelativeHumidityStartConversion(void)
{

  toggle_pin(TP_2_BASE, TP_2_PIN);

  I2C5Send(SlaveAddress, 2, RegisterAddress3, Start1);

  toggle_pin(TP_2_BASE, TP_2_PIN);

}

void RelativeHumidityRead(void)
{
    unsigned char Status=1;
    unsigned char RelativeHumidityH;
    unsigned char RelativeHumidityL;
    unsigned int RelHuma;
    unsigned int RelHumb;
    unsigned int RelHumc;

    float curve;
    float rawHumidity;
    float linearHumidity;

    while(Status==1)
    {

         Status = I2C5Receive(SlaveAddress, RegisterAddress0);

    }

    toggle_pin(TP_2_BASE, TP_2_PIN);

    RelativeHumidityH = I2C5Receive(SlaveAddress, RegisterAddress1);

    toggle_pin(TP_2_BASE, TP_2_PIN);

    RelativeHumidityL = I2C5Receive(SlaveAddress, RegisterAddress2);

    toggle_pin(TP_2_BASE, TP_2_PIN);

    RelHuma = RelativeHumidityH;
    RelHumb = RelHuma<<8;

    RelHumb += RelativeHumidityL;
    RelHumc = RelHumb>>4;

    rawHumidity = RelHumc;

    curve = (rawHumidity/16.0)-24.0 ;

    linearHumidity = curve - ( (curve * curve) * a2 + curve * a1 + a0);
    linearHumidity = linearHumidity + ( Temperature - 30.0 ) * ( linearHumidity * q1 + q0 );

    RelativeHumidity = linearHumidity;

    if(RelativeHumidity > RhAlarmLevel) RhAlarm = 1;
    if(RelativeHumidity > RhTripLevel) RhTrip = 1;
}


//******************************************************************************
// Init Si7005-B SENSOR
//******************************************************************************

void RhTempSenseInit(void)
{

    set_gpio_as_output(TP_1_BASE, TP_1_PIN);
    set_gpio_as_output(TP_2_BASE, TP_2_PIN);



   // performs I2C initialization
    InitI2C5();

   // configura CS como output
   set_gpio_as_output(GPIO_PORTB_BASE, GPIO_PIN_2);
   clear_pin(GPIO_PORTB_BASE, GPIO_PIN_2);

   delay_ms(100);

}

unsigned char RhRead(void)
{
   return RelativeHumidity;
}

unsigned char BoardTempRead(void)
{
   return Temperature;
}

void TempBoardAlarmLimitSet(unsigned char TempLimit)
{
   if(TempLimit > 100) TempLimit = 100;
   TempAlarmLevel = TempLimit;
}

void TempBoardTripLimitSet(unsigned char TempLimit)
{
   if(TempLimit > 100) TempLimit = 100;
   TempTripLevel = TempLimit;
}

unsigned char TempAlarmStatusRead(void)
{
   return TempAlarm;
}

unsigned char TempTripStatusRead(void)
{
   return TempTrip;
}

void RhAlarmLimitSet(unsigned char RhLimit)
{
   if(RhLimit > 100) RhLimit = 100;
   RhAlarmLevel = RhLimit;
}

void RhTripLimitSet(unsigned char RhLimit)
{
   if(RhLimit > 100) RhLimit = 100;
   RhTripLevel = RhLimit;
}

unsigned char RhAlarmStatusRead(void)
{
   return RhAlarm;
}

unsigned char RhTripStatusRead(void)
{
   return RhTrip;
}

void RhTempClearAlarmTrip(void)
{
   
   TempAlarm = 0;
   TempTrip = 0;
   
   RhAlarm = 0;
   RhTrip =  0;
   
}
