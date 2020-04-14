
/////////////////////////////////////////////////////////////////////////////////////////////

/*
 * ntc_isolated_i2c.c
 *
 *  Created on: 18 de fev de 2020
 *      Author: rogerio.marcondeli
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "inc/hw_ssi.h"
#include "inc/hw_i2c.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "ntc_isolated_i2c.h"
#include "peripheral_drivers/timer/timer.h"
#include "peripheral_drivers/gpio/gpio_driver.h"
#include "peripheral_drivers/i2c/i2c_driver.h"
#include "board_drivers/hardware_def.h"

/////////////////////////////////////////////////////////////////////////////////////////////

#define Series_Resistance       10000.00 //Resistor de 10K Ohms

#define Vin                     3.3 //Tensão 3,3V de entrada do divisor resistivo

float Get_Temperature(float Vout_ADS1014);

/////////////////////////////////////////////////////////////////////////////////////////////

//******************************************************************************
// Declaração das variaveis globais
//******************************************************************************

// NTC 5k

static float A = 0.001003604774;
static float B = 0.000264014765;
static float C = 0.000000164677;

static unsigned char Temp_Ntc_Igbt1 = 0;
static unsigned char Temp_Ntc_Igbt2 = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

float Get_Temperature(float Vout_ADS1014)
{
    double NTC_Resistance;
    double Thermistor_temperature;
    float Temperature;

    NTC_Resistance = (((Series_Resistance * Vin) / Vout_ADS1014) - Series_Resistance); /* calculate the resistance */

    Thermistor_temperature = log(NTC_Resistance); /* calculate natural log of resistance */

    /*  Steinhart-Hart Thermistor Equation: */
    /*  Temperature in Kelvin = 1 / (A + B[ln(R)] + C[ln(R)]^3)   */

    Temperature = ( 1 / ( A + ( B * Thermistor_temperature ) + ( C * (pow(Thermistor_temperature,3) )) ) );  // temperatura em Kelvin

    Temperature = Temperature - 273.15; // temperatura em graus Celsius

    return Temperature;
}

/////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************************/
/*!
    You must provide this function.
*/
/**************************************************************************/
ADS1x1x_config_t ntc_igbt1;
ADS1x1x_config_t ntc_igbt2;

/////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************************/
/*!
    @brief  Writes 16 bits to the specified destination register.
*/
/**************************************************************************/
void ADS1x1x_write_register(uint8_t i2c_address, uint8_t reg, uint16_t value)
{

  I2CMasterSlaveAddrSet(I2C2_BASE, i2c_address, false);
  I2CMasterDataPut(I2C2_BASE, reg);
  I2CMasterControl(I2C2_BASE,I2C_MASTER_CMD_BURST_SEND_START);
  while(!(I2CMasterBusy(I2C2_BASE)));
  while(I2CMasterBusy(I2C2_BASE));

  I2CMasterDataPut(I2C2_BASE, (uint8_t)value>>8);
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
  while(!(I2CMasterBusy(I2C2_BASE)));
  while(I2CMasterBusy(I2C2_BASE));

  I2CMasterDataPut(I2C2_BASE, (uint8_t)value&0xff);
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
  while(!(I2CMasterBusy(I2C2_BASE)));
  while(I2CMasterBusy(I2C2_BASE));

  SysCtlDelay(30);

}

/////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************************/
/*!
    @brief  Read 16 bits from the specified destination register.
*/
/**************************************************************************/
uint16_t ADS1x1x_read_register(uint8_t i2c_address, uint8_t reg)
{

  uint16_t result = 0;
  uint8_t  ui8ConvRegHighByte = 0;
  uint8_t  ui8ConvRegLowByte = 0;

  I2CMasterSlaveAddrSet(I2C2_BASE, i2c_address, false);
  I2CMasterDataPut(I2C2_BASE, reg);
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_SEND_START );
  while(!(I2CMasterBusy(I2C2_BASE)));
  while(I2CMasterBusy(I2C2_BASE));

  I2CMasterSlaveAddrSet(I2C2_BASE, i2c_address, true);
  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
  while(!(I2CMasterBusy(I2C2_BASE)));
  while(I2CMasterBusy(I2C2_BASE));
  ui8ConvRegHighByte = I2CMasterDataGet(I2C2_BASE);

  I2CMasterControl(I2C2_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
  while(!(I2CMasterBusy(I2C2_BASE)));
  while(I2CMasterBusy(I2C2_BASE));
  ui8ConvRegLowByte = I2CMasterDataGet(I2C2_BASE);

  result = ((ui8ConvRegHighByte<<8)|ui8ConvRegLowByte);

  return result;

}

/////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************************/
/*!
    @brief  Initialise a user-provided ADS1X15 configuration structure.
            The user must provide a valid pointer to an empty
            ADS1x1x_config_t structure.
*/
/**************************************************************************/
uint8_t ADS1x1x_init(ADS1x1x_config_t *p_config, ADS1x1x_chip_t chip, uint8_t i2c_address, ADS1x1x_mux_t input, ADS1x1x_pga_t gain)
{
  uint8_t result = 0;

  if (p_config!=0)
  {
    // Set generic parameters.
    p_config->chip = chip;
    p_config->i2c_address = i2c_address;

    // Set configuration bits for default operation.
    p_config->config = 0;
    ADS1x1x_set_os(p_config,OS_SINGLE);
    ADS1x1x_set_multiplexer(p_config,input);
    ADS1x1x_set_pga(p_config,gain);
    ADS1x1x_set_mode(p_config,MODE_CONTINUOUS); // alterado MODE_SINGLE_SHOT
    if (p_config->chip==ADS1013 || p_config->chip==ADS1014 || p_config->chip==ADS1015)
    {
      ADS1x1x_set_data_rate(p_config,DATA_RATE_ADS101x_3300);
    }
    else
    {
      ADS1x1x_set_data_rate(p_config,DATA_RATE_ADS111x_128);
    }
    ADS1x1x_set_comparator_mode(p_config,COMPARATOR_MODE_TRADITIONAL);
    ADS1x1x_set_comparator_polarity(p_config,COMPARATOR_POLARITY_ACTIVE_LO);
    ADS1x1x_set_comparator_latching(p_config,COMPARATOR_NON_LATCHING);
    ADS1x1x_set_comparator_queue(p_config,COMPARATOR_QUEUE_NONE);

    result = 1;
  }

  return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************************/
/*!
    @brief  Start an ADC conversion cycle.
            The user must provide a valid pointer to a
            correctly filled ADS1x1x_config_t structure.
*/
/**************************************************************************/
void ADS1x1x_start_conversion(ADS1x1x_config_t *p_config)
{
  // Write configuration to the ADC.
  ADS1x1x_write_register(p_config->i2c_address,ADS1x1x_REG_POINTER_CONFIG,p_config->config);
}

/////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************************/
/*!
    @brief  Read the ADC conversion result.
            The user must provide a valid pointer to a
            correctly filled ADS1x1x_config_t structure.
*/
/**************************************************************************/
int16_t ADS1x1x_read(ADS1x1x_config_t *p_config)
{
  // Read the conversion result.
  int16_t result = (int16_t)ADS1x1x_read_register(p_config->i2c_address,ADS1x1x_REG_POINTER_CONVERSION);
  // Adjust for ADC resolution if needed.
  if (p_config->chip==ADS1013 || p_config->chip==ADS1014 || p_config->chip==ADS1015)
  {
    result >>= 4;
  }
  return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_threshold_lo(ADS1x1x_config_t *p_config, uint16_t value)
{
  if (p_config->chip==ADS1013 || p_config->chip==ADS1014 || p_config->chip==ADS1015)
  {
    value <<= 4;
  }
  ADS1x1x_write_register(p_config->i2c_address,ADS1x1x_REG_POINTER_LO_THRESH,value);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_threshold_hi(ADS1x1x_config_t *p_config, uint16_t value)
{
  if (p_config->chip==ADS1013 || p_config->chip==ADS1014 || p_config->chip==ADS1015)
  {
    value <<= 4;
  }
  ADS1x1x_write_register(p_config->i2c_address,ADS1x1x_REG_POINTER_HI_THRESH,value);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_config_bitfield(ADS1x1x_config_t *p_config, uint16_t value, uint16_t mask)
{
  p_config->config &= ~mask;
  p_config->config |= (value & mask);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_os(ADS1x1x_config_t *p_config, ADS1x1x_os_t value)
{
  ADS1x1x_set_config_bitfield(p_config,(uint16_t)value,ADS1x1x_REG_CONFIG_OS_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_multiplexer(ADS1x1x_config_t *p_config, ADS1x1x_mux_t value)
{
  ADS1x1x_set_config_bitfield(p_config,(uint16_t)value,ADS1x1x_REG_CONFIG_MULTIPLEXER_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_pga(ADS1x1x_config_t *p_config, ADS1x1x_pga_t value)
{
  ADS1x1x_set_config_bitfield(p_config,(uint16_t)value,ADS1x1x_REG_CONFIG_PGA_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_mode(ADS1x1x_config_t *p_config, ADS1x1x_mode_t value)
{
  ADS1x1x_set_config_bitfield(p_config,(uint16_t)value,ADS1x1x_REG_CONFIG_MODE_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_data_rate(ADS1x1x_config_t *p_config, ADS1x1x_data_rate_t value)
{
  ADS1x1x_set_config_bitfield(p_config,(uint16_t)value,ADS1x1x_REG_CONFIG_DATA_RATE_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_comparator_mode(ADS1x1x_config_t *p_config, ADS1x1x_comparator_mode_t value)
{
  ADS1x1x_set_config_bitfield(p_config,(uint16_t)value,ADS1x1x_REG_CONFIG_COMPARATOR_MODE_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_comparator_polarity(ADS1x1x_config_t *p_config, ADS1x1x_comparator_polarity_t value)
{
  ADS1x1x_set_config_bitfield(p_config,(uint16_t)value,ADS1x1x_REG_CONFIG_COMPARATOR_POLARITY_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_comparator_latching(ADS1x1x_config_t *p_config, ADS1x1x_comparator_latching_t value)
{
  ADS1x1x_set_config_bitfield(p_config,(uint16_t)value,ADS1x1x_REG_CONFIG_COMPARATOR_LATCHING_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void ADS1x1x_set_comparator_queue(ADS1x1x_config_t *p_config, ADS1x1x_comparator_queue_t value)
{
  ADS1x1x_set_config_bitfield(p_config,(uint16_t)value,ADS1x1x_REG_CONFIG_COMPARATOR_QUEUE_MASK);
}

/////////////////////////////////////////////////////////////////////////////////////////////

//******************************************************************************
//  ADS1014 with NTC 5K Igbt1 and Igbt2 initialization
//******************************************************************************
void Ntc_Igbt1_Igbt2_Init(void)
{
  // performs I2C initialization
  InitI2C2();

  // Initialise ADC object igbt1.
  ADS1x1x_init(&ntc_igbt1,ADS1014,ADS1x1x_I2C_ADDRESS_ADDR_TO_GND,MUX_SINGLE_0,PGA_6144);

  // Initialise ADC object igbt2.
  ADS1x1x_init(&ntc_igbt2,ADS1014,ADS1x1x_I2C_ADDRESS_ADDR_TO_VCC,MUX_SINGLE_0,PGA_6144);

  delay_ms(100);
}

/////////////////////////////////////////////////////////////////////////////////////////////

//******************************************************************************
// Read the ADS1014 with NTC 5K Igbt1 and Igbt2
//******************************************************************************
void Ntc_Igbt1_Igbt2_Start_Conversion(void)
{
  // Set input before starting conversion ntc igbt1.
  ADS1x1x_set_multiplexer(&ntc_igbt1,(ADS1x1x_mux_t)0x4000);
  ADS1x1x_start_conversion(&ntc_igbt1);

  // Default sample rate is 3300 samples/s, i.e. one sample every 1 ms.
  delay_ms(1);

  // Set input before starting conversion ntc igbt2.
  ADS1x1x_set_multiplexer(&ntc_igbt2,(ADS1x1x_mux_t)0x4000);
  ADS1x1x_start_conversion(&ntc_igbt2);

  // Default sample rate is 3300 samples/s, i.e. one sample every 1 ms.
  delay_ms(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

//******************************************************************************
// Start read the ADS1014 with NTC 5K Igbt1 and Igbt2
//******************************************************************************
void Ntc_Igbt1_Igbt2_Read(void)
{

    Temp_Ntc_Igbt1 = Get_Temperature((((float)ADS1x1x_read(&ntc_igbt1)*6.144)/2047.0));

    Temp_Ntc_Igbt2 = Get_Temperature((((float)ADS1x1x_read(&ntc_igbt2)*6.144)/2047.0));

    delay_ms(30);

}

/////////////////////////////////////////////////////////////////////////////////////////////

//******************************************************************************
// Read the ADS1014 with NTC 5K Igbt1 and return value
//******************************************************************************
unsigned char Temp_Igbt1_Read(void)
{
  return Temp_Ntc_Igbt1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

//******************************************************************************
// Read the ADS1014 with NTC 5K Igbt2 and return value
//******************************************************************************
unsigned char Temp_Igbt2_Read(void)
{
  return Temp_Ntc_Igbt2;
}
