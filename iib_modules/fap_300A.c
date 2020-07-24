/******************************************************************************
 * Copyright (C) 2017 by LNLS - Brazilian Synchrotron Light Laboratory
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. LNLS and
 * the Brazilian Center for Research in Energy and Materials (CNPEM) are not
 * liable for any misuse of this material.
 *
 *****************************************************************************/

/**
 * @file fap_300A.c
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 20 de out de 2018
 *
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#include <iib_modules/fap_300A.h>
#include "iib_data.h"

#include "adc_internal.h"
#include "application.h"

#include "BoardTempHum.h"
#include "ntc_isolated_i2c.h"
#include "pt100.h"
#include "output.h"
#include "leds.h"
#include "can_bus.h"
#include "input.h"

#include <stdbool.h>
#include <stdint.h>

#include "peripheral_drivers/timer/timer.h"
#include "inc/hw_ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"

/////////////////////////////////////////////////////////////////////////////////////////////

/**
 * TODO: Put here your defines. Just what is local. If you don't
 * need to access it from other module, consider use a constant (const)
 */

#define FAP_300A_INPUT_OVERVOLTAGE_ALM_LIM       55.0
#define FAP_300A_INPUT_OVERVOLTAGE_ITLK_LIM      60.0

#define FAP_300A_OUTPUT_OVERVOLTAGE_ALM_LIM      17.0
#define FAP_300A_OUTPUT_OVERVOLTAGE_ITLK_LIM     20.0

#define FAP_300A_OUTPUT_OVERCURRENT_1_ALM_LIM    151.0
#define FAP_300A_OUTPUT_OVERCURRENT_1_ITLK_LIM   152.0

#define FAP_300A_OUTPUT_OVERCURRENT_2_ALM_LIM    151.0
#define FAP_300A_OUTPUT_OVERCURRENT_2_ITLK_LIM   152.0

#define FAP_300A_GROUND_LEAKAGE_ALM_LIM          40.0
#define FAP_300A_GROUND_LEAKAGE_ITLK_LIM         45.0

#define FAP_300A_IGBT1_OVERTEMP_ALM_LIM          60
#define FAP_300A_IGBT1_OVERTEMP_ITLK_LIM         80

#define FAP_300A_IGBT2_OVERTEMP_ALM_LIM          60
#define FAP_300A_IGBT2_OVERTEMP_ITLK_LIM         80

#define FAP_300A_DRIVER_OVERVOLTAGE_ALM_LIM      16.0
#define FAP_300A_DRIVER_OVERVOLTAGE_ITLK_LIM     17.0

#define FAP_300A_DRIVER1_OVERCURRENT_ALM_LIM     2.0
#define FAP_300A_DRIVER1_OVERCURRENT_ITLK_LIM    2.4

#define FAP_300A_DRIVER2_OVERCURRENT_ALM_LIM     2.0
#define FAP_300A_DRIVER2_OVERCURRENT_ITLK_LIM    2.4

#define FAP_300A_INDUC_OVERTEMP_ALM_LIM          50
#define FAP_300A_INDUC_OVERTEMP_ITLK_LIM         60

#define FAP_300A_HS_OVERTEMP_ALM_LIM             50
#define FAP_300A_HS_OVERTEMP_ITLK_LIM            60

#define FAP_300A_RH_ALM_LIM                      50
#define FAP_300A_RH_ITLK_LIM                     90

#define FAP_300A_BOARD_TEMP_ALM_LIM              40
#define FAP_300A_BOARD_TEMP_ITLK_LIM             50

/////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    union {
        float   f;
        uint8_t u8[4];
    } Vin;

    bool VinAlarmSts;
    bool VinItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } Vout;

    bool VoutAlarmSts;
    bool VoutItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } IoutA1;

    bool IoutA1AlarmSts;
    bool IoutA1ItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } IoutA2;

    bool IoutA2AlarmSts;
    bool IoutA2ItlkSts;

    union {
        float f;
        uint8_t u8[4];
    } GroundLeakage;

    bool GroundLeakageItlkSts;
    bool GroundLeakageAlarmSts;

    union {
        float   f;
        uint8_t u8[4];
    } TempIGBT1;

    bool TempIGBT1AlarmSts;
    bool TempIGBT1ItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } TempIGBT2;

    bool TempIGBT2AlarmSts;
    bool TempIGBT2ItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } DriveVoltage;

    bool DriveVoltageAlarmSts;
    bool DriveVoltageItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } Drive1Current;

    bool Drive1CurrentAlarmSts;
    bool Drive1CurrentItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } Drive2Current;

    bool Drive2CurrentAlarmSts;
    bool Drive2CurrentItlkSts;

    bool Driver1Error;
    bool Driver1ErrorItlkSts;
    bool Driver2Error;
    bool Driver2ErrorItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } TempL;

    bool TempLAlarmSts;
    bool TempLItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } TempHeatSink;

    bool TempHeatSinkAlarmSts;
    bool TempHeatSinkItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } BoardTemperature;

    bool BoardTemperatureAlarmSts;
    bool BoardTemperatureItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } RelativeHumidity;

    bool RelativeHumidityAlarmSts;
    bool RelativeHumidityItlkSts;

    bool Relay;
    bool ExternalItlk;
    bool ExternalItlkSts;
    bool Rack;
    bool RackItlkSts;

    bool ReleAuxItlkSts;
    bool ReleExtItlkSts;
    bool RelayOpenItlkSts;
    bool RelayContactStickingItlkSts;
    bool FlagAux;

} fap_300A_t;

/////////////////////////////////////////////////////////////////////////////////////////////

fap_300A_t fap_300A;

static uint32_t fap_300A_interlocks_indication;
static uint32_t fap_300A_alarms_indication;

static uint32_t ResetInterlocksRegister = 0;
static uint32_t ResetAlarmsRegister = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t itlk_id;
static uint32_t alarm_id;

/////////////////////////////////////////////////////////////////////////////////////////////

static void get_itlks_id();
static void get_alarms_id();
static void map_vars();
static void config_module();

/////////////////////////////////////////////////////////////////////////////////////////////

void init_fap_300A()
{
    config_module();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fap_300A_interlocks()
{
    fap_300A.RelayOpenItlkSts = 0;
    fap_300A.RelayContactStickingItlkSts = 0;
    fap_300A.ReleAuxItlkSts = 0;
    fap_300A.ReleExtItlkSts = 0;
    fap_300A.FlagAux = 0;

////////////////////////////////////////

    fap_300A.VinItlkSts               = 0;
    fap_300A.VoutItlkSts              = 0;
    fap_300A.IoutA1ItlkSts            = 0;
    fap_300A.IoutA2ItlkSts            = 0;
    fap_300A.TempIGBT1ItlkSts         = 0;
    fap_300A.TempIGBT2ItlkSts         = 0;
    fap_300A.Driver1ErrorItlkSts      = 0;
    fap_300A.Driver2ErrorItlkSts      = 0;
    fap_300A.TempLItlkSts             = 0;
    fap_300A.TempHeatSinkItlkSts      = 0;
    fap_300A.ExternalItlkSts          = 0;
    fap_300A.RackItlkSts              = 0;
    fap_300A.GroundLeakageItlkSts     = 0;
    fap_300A.DriveVoltageItlkSts      = 0;
    fap_300A.Drive1CurrentItlkSts     = 0;
    fap_300A.Drive2CurrentItlkSts     = 0;
    fap_300A.BoardTemperatureItlkSts  = 0;
    fap_300A.RelativeHumidityItlkSts  = 0;

    itlk_id = 0;

    send_itlk_message(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fap_300A_interlocks()
{
    uint8_t test = 0;

    test |= fap_300A.VinItlkSts;
    test |= fap_300A.VoutItlkSts;
    test |= fap_300A.IoutA1ItlkSts;
    test |= fap_300A.IoutA2ItlkSts;
    test |= fap_300A.TempIGBT1ItlkSts;
    test |= fap_300A.TempIGBT2ItlkSts;
    test |= fap_300A.Driver1ErrorItlkSts;
    test |= fap_300A.Driver2ErrorItlkSts;
    test |= fap_300A.TempLItlkSts;
    test |= fap_300A.TempHeatSinkItlkSts;
    test |= fap_300A.ExternalItlkSts;
    test |= fap_300A.RackItlkSts;
    test |= fap_300A.GroundLeakageItlkSts;
    test |= fap_300A.DriveVoltageItlkSts;
    test |= fap_300A.Drive1CurrentItlkSts;
    test |= fap_300A.Drive2CurrentItlkSts;
    test |= fap_300A.BoardTemperatureItlkSts;
    test |= fap_300A.RelativeHumidityItlkSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fap_300A_alarms()
{
    fap_300A.VinAlarmSts               = 0;
    fap_300A.VoutAlarmSts              = 0;
    fap_300A.IoutA1AlarmSts            = 0;
    fap_300A.IoutA2AlarmSts            = 0;
    fap_300A.TempIGBT1AlarmSts         = 0;
    fap_300A.TempIGBT2AlarmSts         = 0;
    fap_300A.TempLAlarmSts             = 0;
    fap_300A.TempHeatSinkAlarmSts      = 0;
    fap_300A.GroundLeakageAlarmSts     = 0;
    fap_300A.DriveVoltageAlarmSts      = 0;
    fap_300A.Drive1CurrentAlarmSts     = 0;
    fap_300A.Drive2CurrentAlarmSts     = 0;
    fap_300A.BoardTemperatureAlarmSts  = 0;
    fap_300A.RelativeHumidityAlarmSts  = 0;

    alarm_id = 0;

    send_alarm_message(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fap_300A_alarms()
{
    uint8_t test = 0;

    test |= fap_300A.VinAlarmSts;
    test |= fap_300A.VoutAlarmSts;
    test |= fap_300A.IoutA1AlarmSts;
    test |= fap_300A.IoutA2AlarmSts;
    test |= fap_300A.TempIGBT1AlarmSts;
    test |= fap_300A.TempIGBT2AlarmSts;
    test |= fap_300A.TempLAlarmSts;
    test |= fap_300A.TempHeatSinkAlarmSts;
    test |= fap_300A.GroundLeakageAlarmSts;
    test |= fap_300A.DriveVoltageAlarmSts;
    test |= fap_300A.Drive1CurrentAlarmSts;
    test |= fap_300A.Drive2CurrentAlarmSts;
    test |= fap_300A.BoardTemperatureAlarmSts;
    test |= fap_300A.RelativeHumidityAlarmSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void check_fap_300A_indication_leds()
{
    // Output over voltage
    if(fap_300A.VoutItlkSts) Led2TurnOff();
    else if(fap_300A.VoutAlarmSts) Led2Toggle();
    else Led2TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Input over voltage
    if(fap_300A.VinItlkSts) Led3TurnOff();
    else if(fap_300A.VinAlarmSts) Led3Toggle();
    else Led3TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Output over current
    if (fap_300A.IoutA1ItlkSts || fap_300A.IoutA2ItlkSts) Led4TurnOff();
    else if(fap_300A.IoutA1AlarmSts || fap_300A.IoutA2AlarmSts) Led4Toggle();
    else Led4TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Over temperature
    if(fap_300A.TempIGBT1ItlkSts || fap_300A.TempIGBT2ItlkSts ||  fap_300A.TempLItlkSts || fap_300A.TempHeatSinkItlkSts) Led5TurnOff();
    else if(fap_300A.TempIGBT1AlarmSts || fap_300A.TempIGBT2AlarmSts ||  fap_300A.TempLAlarmSts || fap_300A.TempHeatSinkAlarmSts) Led5Toggle();
    else Led5TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Externo
    if(fap_300A.ExternalItlkSts) Led6TurnOff();
    else Led6TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Fuga para o Terra
    if(fap_300A.GroundLeakageItlkSts) Led7TurnOff();
    else if(fap_300A.GroundLeakageAlarmSts) Led7Toggle();
    else Led7TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock do Rack
    if(fap_300A.RackItlkSts) Led8TurnOff();
    else Led8TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks dos Drivers
    if(fap_300A.Driver1ErrorItlkSts || fap_300A.Driver2ErrorItlkSts || fap_300A.DriveVoltageItlkSts || fap_300A.Drive1CurrentItlkSts || fap_300A.Drive2CurrentItlkSts) Led9TurnOff();
    else if(fap_300A.DriveVoltageAlarmSts || fap_300A.Drive1CurrentAlarmSts || fap_300A.Drive2CurrentAlarmSts) Led9Toggle();
    else Led9TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Temperatura PCB e Umidade Relativa
    if(fap_300A.BoardTemperatureItlkSts || fap_300A.RelativeHumidityItlkSts) Led10TurnOff();
    else if(fap_300A.BoardTemperatureAlarmSts || fap_300A.RelativeHumidityAlarmSts) Led10Toggle();
    else Led10TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fap_300A_application_readings()
{
    //PT100 CH1 Dissipador
    fap_300A.TempHeatSink.f = (float) Pt100ReadCh1();
    fap_300A.TempHeatSinkAlarmSts = Pt100ReadCh1AlarmSts();
    if(!fap_300A.TempHeatSinkItlkSts)fap_300A.TempHeatSinkItlkSts = Pt100ReadCh1TripSts();

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 CH2 Indutor
    fap_300A.TempL.f = (float) Pt100ReadCh2();
    fap_300A.TempLAlarmSts = Pt100ReadCh2AlarmSts();
    if(!fap_300A.TempLItlkSts)fap_300A.TempLItlkSts = Pt100ReadCh2TripSts();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT1
    fap_300A.TempIGBT1.f = (float) TempIgbt1Read();
    fap_300A.TempIGBT1AlarmSts = TempIgbt1AlarmStatusRead();
    if(!fap_300A.TempIGBT1ItlkSts)fap_300A.TempIGBT1ItlkSts = TempIgbt1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT2
    fap_300A.TempIGBT2.f = (float) TempIgbt2Read();
    fap_300A.TempIGBT2AlarmSts = TempIgbt2AlarmStatusRead();
    if(!fap_300A.TempIGBT2ItlkSts)fap_300A.TempIGBT2ItlkSts = TempIgbt2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura PCB IIB
    fap_300A.BoardTemperature.f = (float) BoardTempRead();
    fap_300A.BoardTemperatureAlarmSts = BoardTempAlarmStatusRead();
    if(!fap_300A.BoardTemperatureItlkSts)fap_300A.BoardTemperatureItlkSts = BoardTempTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Umidade Relativa
    fap_300A.RelativeHumidity.f = (float) RhRead();
    fap_300A.RelativeHumidityAlarmSts = RhAlarmStatusRead();
    if(!fap_300A.RelativeHumidityItlkSts)fap_300A.RelativeHumidityItlkSts = RhTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //DriverVotage
    fap_300A.DriveVoltage.f = DriverVoltageRead();
    fap_300A.DriveVoltageAlarmSts = DriverVoltageAlarmStatusRead();
    if(!fap_300A.DriveVoltageItlkSts)fap_300A.DriveVoltageItlkSts = DriverVolatgeTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Drive1Current
    fap_300A.Drive1Current.f = Driver1CurrentRead();
    fap_300A.Drive1CurrentAlarmSts = Driver1CurrentAlarmStatusRead();
    if(!fap_300A.Drive1CurrentItlkSts)fap_300A.Drive1CurrentItlkSts = Driver1CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Drive2Current
    fap_300A.Drive2Current.f = Driver2CurrentRead();
    fap_300A.Drive2CurrentAlarmSts = Driver2CurrentAlarmStatusRead();
    if(!fap_300A.Drive2CurrentItlkSts)fap_300A.Drive2CurrentItlkSts = Driver2CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Corrente de Saida IGBT1
    fap_300A.IoutA1.f = CurrentCh1Read();//HALL CH1
    fap_300A.IoutA1AlarmSts = CurrentCh1AlarmStatusRead();
    if(!fap_300A.IoutA1ItlkSts)fap_300A.IoutA1ItlkSts = CurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Corrente de Saida IGBT2
    fap_300A.IoutA2.f = CurrentCh2Read();//HALL CH2
    fap_300A.IoutA2AlarmSts = CurrentCh2AlarmStatusRead();
    if(!fap_300A.IoutA2ItlkSts)fap_300A.IoutA2ItlkSts = CurrentCh2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Tensao de Entrada
    fap_300A.Vin.f = LvCurrentCh1Read();
    fap_300A.VinAlarmSts = LvCurrentCh1AlarmStatusRead();
    if(!fap_300A.VinItlkSts)fap_300A.VinItlkSts = LvCurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Tensao de Saida
    fap_300A.Vout.f = LvCurrentCh2Read();
    fap_300A.VoutAlarmSts = LvCurrentCh2AlarmStatusRead();
    if(!fap_300A.VoutItlkSts)fap_300A.VoutItlkSts = LvCurrentCh2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Medida de Fuga para o Terra
    fap_300A.GroundLeakage.f = LvCurrentCh3Read();
    fap_300A.GroundLeakageAlarmSts = LvCurrentCh3AlarmStatusRead();
    if(!fap_300A.GroundLeakageItlkSts)fap_300A.GroundLeakageItlkSts = LvCurrentCh3TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock externo
    fap_300A.ExternalItlk = Gpdi5Read();//Variavel usada para debug
    if(!fap_300A.ExternalItlkSts)fap_300A.ExternalItlkSts = Gpdi5Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock do Rack
    fap_300A.Rack = Gpdi6Read();//Variavel usada para debug
    if(!fap_300A.RackItlkSts)fap_300A.RackItlkSts = Gpdi6Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Status do Contato do Rele
    fap_300A.Relay = Gpdi7Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 1
    fap_300A.Driver1Error = Driver1TopErrorRead();//Variavel usada para debug
    if(!fap_300A.Driver1ErrorItlkSts)fap_300A.Driver1ErrorItlkSts = Driver1TopErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 2
    fap_300A.Driver2Error = Driver2TopErrorRead();//Variavel usada para debug
    if(!fap_300A.Driver2ErrorItlkSts)fap_300A.Driver2ErrorItlkSts = Driver2TopErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    fap_300A.ReleAuxItlkSts = ReleAuxSts();
    fap_300A.ReleExtItlkSts = ReleExtItlkSts();

    if(fap_300A.ReleAuxItlkSts == 1 && fap_300A.ReleExtItlkSts == 0)
    {
        fap_300A.FlagAux = 1;
        fap_300A.RelayOpenItlkSts = 0;
        fap_300A.RelayContactStickingItlkSts = 0;
    }

    delay_ms(2);

    if(fap_300A.FlagAux == 1 && fap_300A.ReleAuxItlkSts == 0 && fap_300A.ReleExtItlkSts == 0)
    {
        fap_300A.RelayOpenItlkSts = 1;
        fap_300A.RelayContactStickingItlkSts = 0;
    }

    delay_ms(2);

    if(fap_300A.FlagAux == 1 && fap_300A.ReleAuxItlkSts == 0 && fap_300A.ReleExtItlkSts == 1)
    {
        fap_300A.RelayContactStickingItlkSts = 1;
        fap_300A.RelayOpenItlkSts = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    //Se nao houver sinal na entrada digital dos 4 sinais, defina a acao como Interlock.
    if(fap_300A.ExternalItlkSts || fap_300A.RackItlkSts || fap_300A.Driver1ErrorItlkSts || fap_300A.Driver2ErrorItlkSts) InterlockSet();

/////////////////////////////////////////////////////////////////////////////////////////////

    map_vars();
    get_itlks_id();
    get_alarms_id();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fap_300A_power_on_check()
{
    if (fap_300A.Relay) {
        Led1TurnOff();
        ReleExtItlkTurnOff();
    }
    else {
        Led1TurnOn();
        ReleExtItlkTurnOn();
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////

static void map_vars()
{
    fap_300A_interlocks_indication = itlk_id;
    fap_300A_alarms_indication = alarm_id;

    g_controller_iib.iib_itlk[0].u32        = fap_300A_interlocks_indication;
    g_controller_iib.iib_itlk[1].u32        = ResetInterlocksRegister;

    g_controller_iib.iib_alarm[0].u32       = fap_300A_alarms_indication;
    g_controller_iib.iib_alarm[1].u32       = ResetAlarmsRegister;

    g_controller_iib.iib_signals[0].f       = fap_300A.Vin.f;
    g_controller_iib.iib_signals[1].f       = fap_300A.Vout.f;
    g_controller_iib.iib_signals[2].f       = fap_300A.IoutA1.f;
    g_controller_iib.iib_signals[3].f       = fap_300A.IoutA2.f;
    g_controller_iib.iib_signals[4].f       = fap_300A.TempIGBT1.f;
    g_controller_iib.iib_signals[5].f       = fap_300A.TempIGBT2.f;
    g_controller_iib.iib_signals[6].f       = fap_300A.DriveVoltage.f;
    g_controller_iib.iib_signals[7].f       = fap_300A.Drive1Current.f;
    g_controller_iib.iib_signals[8].f       = fap_300A.Drive2Current.f;
    g_controller_iib.iib_signals[9].f       = fap_300A.TempL.f;
    g_controller_iib.iib_signals[10].f      = fap_300A.TempHeatSink.f;
    g_controller_iib.iib_signals[11].f      = fap_300A.GroundLeakage.f;
    g_controller_iib.iib_signals[12].f      = fap_300A.BoardTemperature.f;
    g_controller_iib.iib_signals[13].f      = fap_300A.RelativeHumidity.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_fap_300A_data()
{
    static uint8_t i = 0;

    send_data_message(i);

    i++;

    if (i > 13) i = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void get_itlks_id()
{
    if (fap_300A.VinItlkSts)                     itlk_id |= FAP_300A_INPUT_OVERVOLTAGE_ITLK;
    if (fap_300A.VoutItlkSts)                    itlk_id |= FAP_300A_OUTPUT_OVERVOLTAGE_ITLK;
    if (fap_300A.IoutA1ItlkSts)                  itlk_id |= FAP_300A_OUTPUT_OVERCURRENT_1_ITLK;
    if (fap_300A.IoutA2ItlkSts)                  itlk_id |= FAP_300A_OUTPUT_OVERCURRENT_2_ITLK;
    if (fap_300A.TempIGBT1ItlkSts)               itlk_id |= FAP_300A_IGBT1_OVERTEMP_ITLK;
    if (fap_300A.TempIGBT2ItlkSts)               itlk_id |= FAP_300A_IGBT2_OVERTEMP_ITLK;
    if (fap_300A.DriveVoltageItlkSts)            itlk_id |= FAP_300A_DRIVER_OVERVOLTAGE_ITLK;
    if (fap_300A.Drive1CurrentItlkSts)           itlk_id |= FAP_300A_DRIVER1_OVERCURRENT_ITLK;
    if (fap_300A.Drive2CurrentItlkSts)           itlk_id |= FAP_300A_DRIVER2_OVERCURRENT_ITLK;
    if (fap_300A.Driver1ErrorItlkSts)            itlk_id |= FAP_300A_DRIVER1_ERROR_ITLK;
    if (fap_300A.Driver2ErrorItlkSts)            itlk_id |= FAP_300A_DRIVER2_ERROR_ITLK;
    if (fap_300A.TempLItlkSts)                   itlk_id |= FAP_300A_INDUC_OVERTEMP_ITLK;
    if (fap_300A.TempHeatSinkItlkSts)            itlk_id |= FAP_300A_HS_OVERTEMP_ITLK;
    if (fap_300A.RelayOpenItlkSts)               itlk_id |= FAP_300A_RELAY_ITLK;
    if (fap_300A.RelayContactStickingItlkSts)    itlk_id |= FAP_300A_RELAY_CONTACT_STICKING_ITLK;
    if (fap_300A.ExternalItlkSts)                itlk_id |= FAP_300A_EXTERNAL_ITLK;
    if (fap_300A.RackItlkSts)                    itlk_id |= FAP_300A_RACK_ITLK;
    if (fap_300A.GroundLeakageItlkSts)           itlk_id |= FAP_300A_GROUND_LKG_ITLK;
    if (fap_300A.BoardTemperatureItlkSts)        itlk_id |= FAP_300A_BOARD_IIB_OVERTEMP_ITLK;
    if (fap_300A.RelativeHumidityItlkSts)        itlk_id |= FAP_300A_BOARD_IIB_OVERHUMIDITY_ITLK;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_fap_300A_itlk_msg()
{
    send_itlk_message(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void get_alarms_id()
{
    if (fap_300A.VinAlarmSts)               alarm_id |= FAP_300A_INPUT_OVERVOLTAGE_ALM;
    if (fap_300A.VoutAlarmSts)              alarm_id |= FAP_300A_OUTPUT_OVERVOLTAGE_ALM;
    if (fap_300A.IoutA1AlarmSts)            alarm_id |= FAP_300A_OUTPUT_OVERCURRENT_1_ALM;
    if (fap_300A.IoutA2AlarmSts)            alarm_id |= FAP_300A_OUTPUT_OVERCURRENT_2_ALM;
    if (fap_300A.TempIGBT1AlarmSts)         alarm_id |= FAP_300A_IGBT1_OVERTEMP_ALM;
    if (fap_300A.TempIGBT2AlarmSts)         alarm_id |= FAP_300A_IGBT2_OVERTEMP_ALM;
    if (fap_300A.TempLAlarmSts)             alarm_id |= FAP_300A_INDUC_OVERTEMP_ALM;
    if (fap_300A.TempHeatSinkAlarmSts)      alarm_id |= FAP_300A_HS_OVERTEMP_ALM;
    if (fap_300A.GroundLeakageAlarmSts)     alarm_id |= FAP_300A_GROUND_LKG_ALM;
    if (fap_300A.DriveVoltageAlarmSts)      alarm_id |= FAP_300A_DRIVER_OVERVOLTAGE_ALM;
    if (fap_300A.Drive1CurrentAlarmSts)     alarm_id |= FAP_300A_DRIVER1_OVERCURRENT_ALM;
    if (fap_300A.Drive2CurrentAlarmSts)     alarm_id |= FAP_300A_DRIVER2_OVERCURRENT_ALM;
    if (fap_300A.BoardTemperatureAlarmSts)  alarm_id |= FAP_300A_BOARD_IIB_OVERTEMP_ALM;
    if (fap_300A.RelativeHumidityAlarmSts)  alarm_id |= FAP_300A_BOARD_IIB_OVERHUMIDITY_ALM;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void config_module()
{
    //Set current range FAP 150 A
    CurrentCh1Init(150.0, 0.150, 50.0, 3); // Corrente braço1: Sensor Hall
    CurrentCh2Init(150.0, 0.150, 50.0, 3); // Corrente braço2: LEM LA 130-P

    CurrentCh1Enable();  //CurrentCh1 enable
    CurrentCh2Enable();  //CurrentCh2 enable
    CurrentCh3Disable(); //CurrentCh3 disable
    CurrentCh4Disable(); //CurrentCh4 disable


    //Set protection limits FAP 150 A
    //     These interlocks are bypassed due to the fact that their ADC's
    //     will most probably saturate during operation at 300 A. These
    //     measures are also performed by UDC, which guarantees these
    //     protections
    CurrentCh1AlarmLevelSet(FAP_300A_OUTPUT_OVERCURRENT_1_ALM_LIM);  //Corrente braço1
    CurrentCh1TripLevelSet(FAP_300A_OUTPUT_OVERCURRENT_1_ITLK_LIM);  //Corrente braço1
    CurrentCh2AlarmLevelSet(FAP_300A_OUTPUT_OVERCURRENT_2_ALM_LIM);  //Corrente braço2
    CurrentCh2TripLevelSet(FAP_300A_OUTPUT_OVERCURRENT_2_ITLK_LIM);  //Corrente braço2

/////////////////////////////////////////////////////////////////////////////////////////////

    //Leitura de tensão isolada
    LvCurrentCh1Init(722.5, 0.025, 120.0, 100); // Vin
    LvCurrentCh2Init(302.5, 0.025, 120.0, 100); // Vout
    LvCurrentCh3Init(52.5, 0.025, 120.0, 3); // Ground Leakage

    LvCurrentCh1Enable(); //LvCurrentCh1 enable
    LvCurrentCh2Enable(); //LvCurrentCh2 enable
    LvCurrentCh3Enable(); //LvCurrentCh3 enable

    LvCurrentCh1AlarmLevelSet(FAP_300A_INPUT_OVERVOLTAGE_ALM_LIM);  //Tensão de entrada Alarme
    LvCurrentCh1TripLevelSet(FAP_300A_INPUT_OVERVOLTAGE_ITLK_LIM);  //Tensão de entrada Interlock
    LvCurrentCh2AlarmLevelSet(FAP_300A_OUTPUT_OVERVOLTAGE_ALM_LIM); //Tensão de saída Alarme
    LvCurrentCh2TripLevelSet(FAP_300A_OUTPUT_OVERVOLTAGE_ITLK_LIM); //Tensão de saída Interlock
    LvCurrentCh3AlarmLevelSet(FAP_300A_GROUND_LEAKAGE_ALM_LIM);     //Fuga para o terra alarme
    LvCurrentCh3TripLevelSet(FAP_300A_GROUND_LEAKAGE_ITLK_LIM);     //Fuga para o terra interlock

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 configuration
    //Delay 4 seconds
    Pt100SetCh1Delay(4);
    Pt100SetCh2Delay(4);

    //PT100 channel enable
    Pt100Ch1Enable(); //Temperatura Dissipador
    Pt100Ch2Enable(); //Temperatura L
    Pt100Ch3Disable();
    Pt100Ch4Disable();

    //PT100 configuration limits
    Pt100SetCh1AlarmLevel(FAP_300A_HS_OVERTEMP_ALM_LIM);     //Temperatura Dissipador
    Pt100SetCh1TripLevel(FAP_300A_HS_OVERTEMP_ITLK_LIM);     //Temperatura Dissipador
    Pt100SetCh2AlarmLevel(FAP_300A_INDUC_OVERTEMP_ALM_LIM);  //Temperatura L
    Pt100SetCh2TripLevel(FAP_300A_INDUC_OVERTEMP_ITLK_LIM);  //Temperatura L

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature igbt1 configuration
    TempIgbt1Delay(3); //Inserir valor de delay

    TempIgbt1Enable(); //TempIgbt1 enable

    //Temp Igbt1 configuration limits
    TempIgbt1AlarmLevelSet(FAP_300A_IGBT1_OVERTEMP_ALM_LIM);
    TempIgbt1TripLevelSet(FAP_300A_IGBT1_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature igbt2 configuration
    TempIgbt2Delay(3); //Inserir valor de delay

    TempIgbt2Enable(); //TempIgbt2 enable

    //Temp Igbt2 configuration limits
    TempIgbt2AlarmLevelSet(FAP_300A_IGBT2_OVERTEMP_ALM_LIM);
    TempIgbt2TripLevelSet(FAP_300A_IGBT2_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature Board configuration
    BoardTempDelay(3); //Inserir valor de delay

    BoardTempEnable(); //BoardTemp enable

    //Temp board configuration limits
    BoardTempAlarmLevelSet(FAP_300A_BOARD_TEMP_ALM_LIM);
    BoardTempTripLevelSet(FAP_300A_BOARD_TEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Humidity Board configuration
    RhDelay(3); //Inserir valor de delay

    RhEnable(); //Rh enable

    //Rh configuration limits
    RhAlarmLevelSet(FAP_300A_RH_ALM_LIM);
    RhTripLevelSet(FAP_300A_RH_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Driver1 error configuration
    Driver1TopErrorEnable(); //Habilitado driver error 1 Top
    Driver1BotErrorDisable(); //Desabilitado driver error 1 Bot
    Driver1OverTempDisable(); //Desabilitado Temperatura por Hardware do modulo 1

    //Driver2 error configuration
    Driver2TopErrorEnable(); //Habilitado driver error 2 Top
    Driver2BotErrorDisable(); //Desabilitado driver error 2 Bot
    Driver2OverTempDisable(); //Desabilitado Temperatura por Hardware do modulo 2

/////////////////////////////////////////////////////////////////////////////////////////////

    //Driver Voltage configuration
    DriverVoltageInit();

    DriverVoltageDelay(3); //Inserir valor de delay

    DriverVoltageEnable(); //DriverVoltage enable

    //Limite de alarme e interlock da tensao dos drivers
    DriverVoltageAlarmLevelSet(FAP_300A_DRIVER_OVERVOLTAGE_ALM_LIM);
    DriverVoltageTripLevelSet(FAP_300A_DRIVER_OVERVOLTAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Driver Current configuration
    DriverCurrentInit();

    DriverCurrentDelay(3); //Inserir valor de delay

    Driver1CurrentEnable(); //Driver1Current enable
    Driver2CurrentEnable(); //Driver2Current enable

    //Limite de alarme e interlock da corrente do driver 1
    Driver1CurrentAlarmLevelSet(FAP_300A_DRIVER1_OVERCURRENT_ALM_LIM);
    Driver1CurrentTripLevelSet(FAP_300A_DRIVER1_OVERCURRENT_ITLK_LIM);

    //Limite de alarme e interlock da corrente do driver 2
    Driver2CurrentAlarmLevelSet(FAP_300A_DRIVER2_OVERCURRENT_ALM_LIM);
    Driver2CurrentTripLevelSet(FAP_300A_DRIVER2_OVERCURRENT_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Voltage configuration
    VoltageCh1Disable(); //VoltageCh1 disable
    VoltageCh2Disable(); //VoltageCh2 disable
    VoltageCh3Disable(); //VoltageCh3 disable
    VoltageCh4Disable(); //VoltageCh4 disable

/////////////////////////////////////////////////////////////////////////////////////////////

    //Gpdi configuration
    Gpdi1Disable();  //Gpdi1 disable
    Gpdi2Disable();  //Gpdi2 disable
    Gpdi3Disable();  //Gpdi3 disable
    Gpdi4Disable();  //Gpdi4 disable
    Gpdi5Enable();   //Gpdi5 enable ExternalITLK
    Gpdi6Enable();   //Gpdi6 enable RackITLK
    Gpdi7Enable();   //Gpdi7 enable RelayStatus
    Gpdi8Disable();  //Gpdi8 disable
    Gpdi9Disable();  //Gpdi9 disable
    Gpdi10Disable(); //Gpdi10 disable
    Gpdi11Disable(); //Gpdi11 disable
    Gpdi12Disable(); //Gpdi12 disable

    //Gpdo configuration
    Gpdo1Disable();  //Gpdo1 disable
    Gpdo2Disable();  //Gpdo2 disable
    Gpdo3Disable();  //Gpdo3 disable
    Gpdo4Disable();  //Gpdo4 disable

    //ReleAux and ReleExtItlk configuration
    ReleAuxEnable(); //ReleAux enable
    ReleExtItlkEnable(); //ReleExtItlk enable

/////////////////////////////////////////////////////////////////////////////////////////////

    //Init Variables
    fap_300A.Vin.f                        = 0.0;
    fap_300A.VinAlarmSts                  = 0;
    fap_300A.VinItlkSts                   = 0;
    fap_300A.Vout.f                       = 0.0;
    fap_300A.VoutAlarmSts                 = 0;
    fap_300A.VoutItlkSts                  = 0;
    fap_300A.IoutA1.f                     = 0.0;
    fap_300A.IoutA1AlarmSts               = 0;
    fap_300A.IoutA1ItlkSts                = 0;
    fap_300A.IoutA2.f                     = 0.0;
    fap_300A.IoutA2AlarmSts               = 0;
    fap_300A.IoutA2ItlkSts                = 0;
    fap_300A.TempIGBT1.f                  = 0.0;
    fap_300A.TempIGBT1AlarmSts            = 0;
    fap_300A.TempIGBT1ItlkSts             = 0;
    fap_300A.TempIGBT2.f                  = 0.0;
    fap_300A.TempIGBT2AlarmSts            = 0;
    fap_300A.TempIGBT2ItlkSts             = 0;
    fap_300A.DriveVoltage.f               = 0.0;
    fap_300A.DriveVoltageAlarmSts         = 0;
    fap_300A.DriveVoltageItlkSts          = 0;
    fap_300A.Drive1Current.f              = 0.0;
    fap_300A.Drive1CurrentAlarmSts        = 0;
    fap_300A.Drive1CurrentItlkSts         = 0;
    fap_300A.Drive2Current.f              = 0.0;
    fap_300A.Drive2CurrentAlarmSts        = 0;
    fap_300A.Drive2CurrentItlkSts         = 0;
    fap_300A.Driver1Error                 = 0;
    fap_300A.Driver1ErrorItlkSts          = 0;
    fap_300A.Driver2Error                 = 0;
    fap_300A.Driver2ErrorItlkSts          = 0;
    fap_300A.TempL.f                      = 0.0;
    fap_300A.TempLAlarmSts                = 0;
    fap_300A.TempLItlkSts                 = 0;
    fap_300A.TempHeatSink.f               = 0.0;
    fap_300A.TempHeatSinkAlarmSts         = 0;
    fap_300A.TempHeatSinkItlkSts          = 0;
    fap_300A.Relay                        = 0;
    fap_300A.ExternalItlk                 = 0;
    fap_300A.ExternalItlkSts              = 0;
    fap_300A.Rack                         = 0;
    fap_300A.RackItlkSts                  = 0;
    fap_300A.GroundLeakage.f              = 0.0;
    fap_300A.GroundLeakageAlarmSts        = 0;
    fap_300A.GroundLeakageItlkSts         = 0;
    fap_300A.BoardTemperature.f           = 0.0;
    fap_300A.BoardTemperatureAlarmSts     = 0;
    fap_300A.BoardTemperatureItlkSts      = 0;
    fap_300A.RelativeHumidity.f           = 0.0;
    fap_300A.RelativeHumidityAlarmSts     = 0;
    fap_300A.RelativeHumidityItlkSts      = 0;
    fap_300A.ReleAuxItlkSts               = 0;
    fap_300A.ReleExtItlkSts               = 0;
    fap_300A.RelayOpenItlkSts             = 0;
    fap_300A.RelayContactStickingItlkSts  = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////





