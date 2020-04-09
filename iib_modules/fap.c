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
 * @file fap.c
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 20/10/2018
 *
 */

#include <iib_modules/fap.h>
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

/**
 * TODO: Put here your defines. Just what is local. If you don't
 * need to access it from other module, consider use a constant (const)
 */

// Linhas de transporte
//#define FAP_INPUT_OVERVOLTAGE_ALM_LIM           50.0
//#define FAP_INPUT_OVERVOLTAGE_ITLK_LIM          55.0
//#define FAP_OUTPUT_OVERVOLTAGE_ALM_LIM          35.0
//#define FAP_OUTPUT_OVERVOLTAGE_ITLK_LIM         40.0
//#define FAP_OUTPUT_OVERCURRENT_1_ALM_LIM        105.0
//#define FAP_OUTPUT_OVERCURRENT_1_ITLK_LIM       115.0
//#define FAP_OUTPUT_OVERCURRENT_2_ALM_LIM        105.0
//#define FAP_OUTPUT_OVERCURRENT_2_ITLK_LIM       115.0

// Potencia nominal
#define FAP_INPUT_OVERVOLTAGE_ALM_LIM         555.0
#define FAP_INPUT_OVERVOLTAGE_ITLK_LIM        560.0
#define FAP_OUTPUT_OVERVOLTAGE_ALM_LIM        260.0
#define FAP_OUTPUT_OVERVOLTAGE_ITLK_LIM       270.0
#define FAP_OUTPUT_OVERCURRENT_1_ALM_LIM      115.0
#define FAP_OUTPUT_OVERCURRENT_1_ITLK_LIM     120.0
#define FAP_OUTPUT_OVERCURRENT_2_ALM_LIM      115.0
#define FAP_OUTPUT_OVERCURRENT_2_ITLK_LIM     120.0

// Fonte 750A
//#define FAP_INPUT_OVERVOLTAGE_ALM_LIM                    55.0
//#define FAP_INPUT_OVERVOLTAGE_ITLK_LIM                   60.0
//#define FAP_OUTPUT_OVERVOLTAGE_ALM_LIM                   40.0
//#define FAP_OUTPUT_OVERVOLTAGE_ITLK_LIM                  45.0
//#define FAP_OUTPUT_OVERCURRENT_1_ALM_LIM                 100.0
//#define FAP_OUTPUT_OVERCURRENT_1_ITLK_LIM                105.0
//#define FAP_OUTPUT_OVERCURRENT_2_ALM_LIM                 100.0
//#define FAP_OUTPUT_OVERCURRENT_2_ITLK_LIM                105.0

#define FAP_GROUND_LEAKAGE_ALM_LIM              40.0
#define FAP_GROUND_LEAKAGE_ITLK_LIM             45.0

#define FAP_IGBT1_OVERTEMP_ALM_LIM              00.0    // Not in use
#define FAP_IGBT1_OVERTEMP_ITLK_LIM             00.0    // Not in use
#define FAP_IGBT2_OVERTEMP_ALM_LIM              00.0    // Not in use
#define FAP_IGBT2_OVERTEMP_ITLK_LIM             00.0    // Not in use
#define FAP_DRIVER_OVERVOLTAGE_ALM_LIM_LIM      16.0
#define FAP_DRIVER_OVERVOLTAGE_ITLK_LIM         17.0
#define FAP_DRIVER1_OVERCURRENT_ALM_LIM         1.15
#define FAP_DRIVER1_OVERCURRENT_ITLK_LIM        2.4
#define FAP_DRIVER2_OVERCURRENT_ALM_LIM         1.15
#define FAP_DRIVER2_OVERCURRENT_ITLK_LIM        2.4
#define FAP_INDUC_OVERTEMP_ALM_LIM              50.0
#define FAP_INDUC_OVERTEMP_ITLK_LIM             60.0
#define FAP_HS_OVERTEMP_ALM_LIM                 60
#define FAP_HS_OVERTEMP_ITLK_LIM                80
#define FAP_RH_ALM_LIM                          80
#define FAP_RH_ITLK_LIM                         90
#define FAP_BOARD_TEMP_ALM_LIM                  80
#define FAP_BOARD_TEMP_ITLK_LIM                 90

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
    bool TempIGBT1HwrItlk;
    bool TempIGBT1HwrItlkSts;

    union {
        float   f;
        uint8_t u8[4];
    } TempIGBT2;

    bool TempIGBT2AlarmSts;
    bool TempIGBT2ItlkSts;
    bool TempIGBT2HwrItlk;
    bool TempIGBT2HwrItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } DriveVoltage;

    bool DriveVoltageAlarmSts; // Rogerio adicionou
    bool DriveVoltageItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } Drive1Current;

    bool Drive1CurrentAlarmSts; // Rogerio adicionou
    bool Drive1CurrentItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } Drive2Current;

    bool Drive2CurrentAlarmSts; // Rogerio adicionou
    bool Drive2CurrentItlkSts;

    bool Driver1Error;
    bool Driver1ErrorItlk;
    bool Driver2Error;
    bool Driver2ErrorItlk;

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

    bool BoardTemperatureAlarmSts; // Rogerio adicionou
    bool BoardTemperatureItlkSts;

    union {
        float   f;
        uint8_t u[4];
    } RelativeHumidity;

    bool RelativeHumidityAlarmSts; // Rogerio adicionou
    bool RelativeHumidityItlkSts;

    bool Relay;
    bool ExternalItlk;
    bool ExternalItlkSts;
    bool Rack;
    bool RackSts;

} fap_t;

fap_t fap;

uint32_t fap_interlocks_indication   = 0;
uint32_t fap_alarms_indication       = 0;

static uint32_t itlk_id;
static uint32_t alarm_id;

static void get_itlks_id();
static void get_alarms_id();
static void map_vars();
static void config_module();

void init_fap()
{
    config_module();
}

void clear_fap_interlocks()
{
    fap.VinItlkSts               = 0;
    fap.VoutItlkSts              = 0;
    fap.IoutA1ItlkSts            = 0;
    fap.IoutA2ItlkSts            = 0;
    fap.TempIGBT1ItlkSts         = 0;
    fap.TempIGBT1HwrItlkSts      = 0;
    fap.TempIGBT2ItlkSts         = 0;
    fap.TempIGBT2HwrItlkSts      = 0;
    fap.Driver1ErrorItlk         = 0;
    fap.Driver2ErrorItlk         = 0;
    fap.TempLItlkSts             = 0;
    fap.TempHeatSinkItlkSts      = 0;
    fap.ExternalItlkSts          = 0;
    fap.RackSts                  = 0;
    fap.GroundLeakageItlkSts     = 0;
    fap.DriveVoltageItlkSts      = 0; // Rogerio adicionou
    fap.Drive1CurrentItlkSts     = 0; // Rogerio adicionou
    fap.Drive2CurrentItlkSts     = 0; // Rogerio adicionou
    fap.BoardTemperatureItlkSts  = 0;
    fap.RelativeHumidityItlkSts  = 0;

    itlk_id = 0;
}

uint8_t check_fap_interlocks()
{
    uint8_t test = 0;

    test |= fap.VinItlkSts;
    test |= fap.VoutItlkSts;
    test |= fap.IoutA1ItlkSts;
    test |= fap.IoutA2ItlkSts;
    test |= fap.TempIGBT1ItlkSts;
    test |= fap.TempIGBT1HwrItlkSts;
    test |= fap.TempIGBT2ItlkSts;
    test |= fap.TempIGBT2HwrItlkSts;
    test |= fap.Driver1ErrorItlk;
    test |= fap.Driver2ErrorItlk;
    test |= fap.TempLItlkSts;
    test |= fap.TempHeatSinkItlkSts;
    test |= fap.ExternalItlkSts;
    test |= fap.RackSts;
    test |= fap.GroundLeakageItlkSts;
    test |= fap.DriveVoltageItlkSts;  // Rogerio adicionou
    test |= fap.Drive1CurrentItlkSts; // Rogerio adicionou
    test |= fap.Drive2CurrentItlkSts; // Rogerio adicionou
    test |= fap.BoardTemperatureItlkSts;
    test |= fap.RelativeHumidityItlkSts;

    return test;
}

void clear_fap_alarms()
{
    fap.VinAlarmSts               = 0;
    fap.VoutAlarmSts              = 0;
    fap.IoutA1AlarmSts            = 0;
    fap.IoutA2AlarmSts            = 0;
    fap.TempIGBT1AlarmSts         = 0;
    fap.TempIGBT2AlarmSts         = 0;
    fap.TempLAlarmSts             = 0;
    fap.TempHeatSinkAlarmSts      = 0;
    fap.GroundLeakageAlarmSts     = 0;
    fap.DriveVoltageAlarmSts      = 0; // Rogerio adicionou
    fap.Drive1CurrentAlarmSts     = 0; // Rogerio adicionou
    fap.Drive2CurrentAlarmSts     = 0; // Rogerio adicionou
    fap.BoardTemperatureAlarmSts  = 0;
    fap.RelativeHumidityAlarmSts  = 0;

    alarm_id = 0;
}

uint8_t check_fap_alarms()
{
    uint8_t test = 0;

    test |= fap.VinAlarmSts;
    test |= fap.VoutAlarmSts;
    test |= fap.IoutA1AlarmSts;
    test |= fap.IoutA2AlarmSts;
    test |= fap.TempIGBT1AlarmSts;
    test |= fap.TempIGBT2AlarmSts;
    test |= fap.TempLAlarmSts;
    test |= fap.TempHeatSinkAlarmSts;
    test |= fap.GroundLeakageAlarmSts;
    test |= fap.DriveVoltageAlarmSts;  // Rogerio adicionou
    test |= fap.Drive1CurrentAlarmSts; // Rogerio adicionou
    test |= fap.Drive2CurrentAlarmSts; // Rogerio adicionou
    test |= fap.BoardTemperatureAlarmSts;
    test |= fap.RelativeHumidityAlarmSts;

    return test;
}

void check_fap_indication_leds()
{
    // Output over voltage
    if(fap.VoutItlkSts) Led2TurnOff();
    else if(fap.VoutAlarmSts) Led2Toggle();
    else Led2TurnOn();

    // Input over voltage
    if(fap.VinItlkSts) Led3TurnOff();
    else if(fap.VinAlarmSts) Led3Toggle();
    else Led3TurnOn();

    // Output over current
    if (fap.IoutA1ItlkSts || fap.IoutA2ItlkSts) Led4TurnOff();
    else if(fap.IoutA1AlarmSts || fap.IoutA2AlarmSts) Led4Toggle();
    else Led4TurnOn();

    // Over temperature
    if(fap.TempIGBT1ItlkSts || fap.TempIGBT2ItlkSts ||  fap.TempLItlkSts || fap.TempHeatSinkItlkSts || fap.TempIGBT1HwrItlkSts || fap.TempIGBT2HwrItlkSts) Led5TurnOff();
    else if(fap.TempIGBT1AlarmSts || fap.TempIGBT2AlarmSts ||  fap.TempLAlarmSts || fap.TempHeatSinkAlarmSts) Led5Toggle();
    else Led5TurnOn();

    if(fap.ExternalItlkSts) Led6TurnOff();
    else Led6TurnOn();

    if(fap.GroundLeakageItlkSts) Led7TurnOff();
    else if(fap.GroundLeakageAlarmSts) Led7Toggle();
    else Led7TurnOn();

    if(fap.RackSts) Led8TurnOff();
    else Led8TurnOn();

    if(fap.Driver1ErrorItlk || fap.Driver2ErrorItlk || fap.DriveVoltageItlkSts || fap.Drive1CurrentItlkSts || fap.Drive2CurrentItlkSts) Led9TurnOff(); // Rogerio alterou
    else if(fap.DriveVoltageAlarmSts || fap.Drive1CurrentAlarmSts || fap.Drive2CurrentAlarmSts) Led9Toggle(); // Rogerio alterou
    else Led9TurnOn();

    //if(InterlockRead()) Led10TurnOff(); // Rogerio modificou
    //else Led10TurnOn();
    if(InterlockRead()) Led10TurnOn();
    else Led10TurnOn();
}

void fap_application_readings()
{
    fap.TempHeatSink.f = (float) Pt100ReadCh1();//PT100 CH1
    fap.TempHeatSinkAlarmSts = Pt100ReadCh1AlarmSts();
    if(!fap.TempHeatSinkItlkSts)fap.TempHeatSinkItlkSts         = Pt100ReadCh1TripSts();

    fap.TempL.f = (float) Pt100ReadCh2();//PT100 CH2
    fap.TempLAlarmSts = Pt100ReadCh2AlarmSts();
    if(!fap.TempLItlkSts)fap.TempLItlkSts                       = Pt100ReadCh2TripSts();

    fap.TempIGBT1.f = (float) Temp_Igbt1_Read();
    fap.TempIGBT1AlarmSts = 0;
    fap.TempIGBT1ItlkSts = 0;

    if(!fap.TempIGBT1HwrItlkSts) fap.TempIGBT1HwrItlkSts        = Driver1OverTempRead();

    fap.TempIGBT2.f = (float) Temp_Igbt2_Read();
    fap.TempIGBT2AlarmSts = 0;
    fap.TempIGBT2ItlkSts = 0;

    if(!fap.TempIGBT2HwrItlkSts) fap.TempIGBT2HwrItlkSts        = Driver2OverTempRead();

    //Temperatura PCB IIB leitura adicionado Rogerio

    fap.BoardTemperature.f = (float) BoardTempRead();
    fap.BoardTemperatureAlarmSts = 0;
    fap.BoardTemperatureItlkSts = 0;

    // Umidade leitura adicionado Rogerio

    fap.RelativeHumidity.f = (float) RhRead();
    fap.RelativeHumidityAlarmSts = 0;
    fap.RelativeHumidityItlkSts = 0;

    //DriverVotage leitura adicionado Rogerio

    fap.DriveVoltage.f = DriverVoltageRead();
    fap.DriveVoltageAlarmSts = DriverVoltageAlarmStatusRead();
    if(!fap.DriveVoltageItlkSts)fap.DriveVoltageItlkSts         = DriverVolatgeTripStatusRead();

    /////////////////////////////////////////////////////////////////////////////////////////////

    //Drive1Current leitura adicionado Rogerio

    fap.Drive1Current.f = Driver1CurrentRead();
    fap.Drive1CurrentAlarmSts = Driver1CurrentAlarmStatusRead();
    if(!fap.Drive1CurrentItlkSts)fap.Drive1CurrentItlkSts       = Driver1CurrentTripStatusRead();

    /////////////////////////////////////////////////////////////////////////////////////////////

    //Drive2Current leitura adicionado Rogerio

    fap.Drive2Current.f = Driver2CurrentRead();
    fap.Drive2CurrentAlarmSts = Driver2CurrentAlarmStatusRead();
    if(!fap.Drive2CurrentItlkSts)fap.Drive2CurrentItlkSts       = Driver2CurrentTripStatusRead();

    /////////////////////////////////////////////////////////////////////////////////////////////


    fap.IoutA1.f = CurrentCh1Read();//HALL CH1
    fap.IoutA1AlarmSts = CurrentCh1AlarmStatusRead();
    if(!fap.IoutA1ItlkSts)fap.IoutA1ItlkSts                     = CurrentCh1TripStatusRead();

    fap.IoutA2.f = CurrentCh2Read();//HALL CH2
    fap.IoutA2AlarmSts = CurrentCh2AlarmStatusRead();
    if(!fap.IoutA2ItlkSts)fap.IoutA2ItlkSts                     = CurrentCh2TripStatusRead();

    fap.Vin.f = LvCurrentCh1Read();
    fap.VinAlarmSts = LvCurrentCh1AlarmStatusRead();
    if(!fap.VinItlkSts)fap.VinItlkSts                           = LvCurrentCh1TripStatusRead();

    fap.Vout.f = LvCurrentCh2Read();
    fap.VoutAlarmSts = LvCurrentCh2AlarmStatusRead();
    if(!fap.VoutItlkSts) fap.VoutItlkSts                        = LvCurrentCh2TripStatusRead();

    fap.GroundLeakage.f = LvCurrentCh3Read();
    fap.GroundLeakageAlarmSts = LvCurrentCh3AlarmStatusRead();
    if(!fap.GroundLeakageItlkSts) fap.GroundLeakageItlkSts      = LvCurrentCh3TripStatusRead();

    fap.ExternalItlk = Gpdi5Read();
    if(!fap.ExternalItlkSts) fap.ExternalItlkSts               = Gpdi5Read();
    //fap.ExternalItlk = Gpdi1Read();
    //if(!fap.ExternalItlkSts) fap.ExternalItlkSts               = Gpdi1Read();

    fap.Rack = Gpdi6Read();
    if(!fap.RackSts) fap.RackSts                               = Gpdi6Read();
    //fap.Rack = Gpdi2Read();
    //if(!fap.RackSts) fap.RackSts                               = Gpdi2Read();

    fap.Relay = Gpdi7Read();
    //fap.Relay = Gpdi3Read();

    //fap.LeakageCurrent = Gpdi6Read();
    //if(!fap.LeakageCurrentSts) fap.LeakageCurrentSts           = Gpdi6Read();
    //fap.LeakageCurrent = Gpdi2Read();
    //if(!fap.LeakageCurrentSts) fap.LeakageCurrentSts           = Gpdi2Read();

    fap.Driver1Error = Driver1TopErrRead();
    if(!fap.Driver1ErrorItlk) fap.Driver1ErrorItlk             = Driver1TopErrRead();

    fap.Driver2Error = Driver2TopErrRead();
    if(!fap.Driver2ErrorItlk) fap.Driver2ErrorItlk             = Driver2TopErrRead();

    if(fap.ExternalItlkSts || fap.Driver2ErrorItlk || fap.Driver2ErrorItlk) InterlockSet(); // If no signal over the port, then set Interlock action

    map_vars();
    get_itlks_id();
    get_alarms_id();
}


void fap_power_on_check()
{
    if (fap.Relay) {
        Led1TurnOff();
        ReleItlkTurnOff();
    }
    else {
        Led1TurnOn();
        ReleItlkTurnOn();
    }
}


static void map_vars()
{
    g_controller_iib.iib_signals[0].u32     = fap_interlocks_indication;
    g_controller_iib.iib_signals[1].u32     = fap_alarms_indication;
    g_controller_iib.iib_signals[2].f       = fap.Vin.f;
    g_controller_iib.iib_signals[3].f       = fap.Vout.f;
    g_controller_iib.iib_signals[4].f       = fap.IoutA1.f;
    g_controller_iib.iib_signals[5].f       = fap.IoutA2.f;
    g_controller_iib.iib_signals[6].f       = fap.TempIGBT1.f;
    g_controller_iib.iib_signals[7].f       = fap.TempIGBT2.f;
    g_controller_iib.iib_signals[8].f       = fap.DriveVoltage.f;
    g_controller_iib.iib_signals[9].f       = fap.Drive1Current.f;
    g_controller_iib.iib_signals[10].f      = fap.Drive2Current.f;
    g_controller_iib.iib_signals[11].f      = fap.TempL.f;
    g_controller_iib.iib_signals[12].f      = fap.TempHeatSink.f;
    g_controller_iib.iib_signals[13].f      = fap.GroundLeakage.f;
    g_controller_iib.iib_signals[14].f      = fap.BoardTemperature.f;
    g_controller_iib.iib_signals[15].f      = fap.RelativeHumidity.f;
}

void send_fap_data()
{
    //uint8_t i;
    //for (i = 2; i < 15; i++) send_data_message(i);

    static uint8_t i = 2;

    send_data_message(i);

    i++;

    if (i > 15) i = 2;
}

static void get_itlks_id()
{
    if (fap.VinItlkSts)                itlk_id |= FAP_INPUT_OVERVOLTAGE_ITLK;
    if (fap.VoutItlkSts)               itlk_id |= FAP_OUTPUT_OVERVOLTAGE_ITLK;
    if (fap.IoutA1ItlkSts)             itlk_id |= FAP_OUTPUT_OVERCURRENT_1_ITLK;
    if (fap.IoutA2ItlkSts)             itlk_id |= FAP_OUTPUT_OVERCURRENT_2_ITLK;
    if (fap.TempIGBT1ItlkSts)          itlk_id |= FAP_IGBT1_OVERTEMP_ITLK;
    if (fap.TempIGBT2ItlkSts)          itlk_id |= FAP_IGBT2_OVERTEMP_ITLK;
    if (fap.Driver1ErrorItlk)          itlk_id |= FAP_DRIVER1_ERROR_ITLK;
    if (fap.Driver2ErrorItlk)          itlk_id |= FAP_DRIVER2_ERROR_ITLK;
    if (fap.TempLItlkSts)              itlk_id |= FAP_INDUC_OVERTEMP_ITLK;
    if (fap.TempHeatSinkItlkSts)       itlk_id |= FAP_HS_OVERTEMP_ITLK;
    if (fap.Relay)                     itlk_id |= FAP_RELAY_ITLK;
    if (fap.ExternalItlkSts)           itlk_id |= FAP_EXTERNAL_ITLK;
    if (fap.RackSts)                   itlk_id |= FAP_RACK_ITLK;
    if (fap.GroundLeakageItlkSts)      itlk_id |= FAP_GROUND_LKG_ITLK;
    if (fap.DriveVoltageItlkSts)       itlk_id |= FAP_DRIVER_OVERVOLTAGE_ITLK;
    if (fap.Drive1CurrentItlkSts)      itlk_id |= FAP_DRIVER1_OVERCURRENT_ITLK;
    if (fap.Drive2CurrentItlkSts)      itlk_id |= FAP_DRIVER2_OVERCURRENT_ITLK;
    if (fap.BoardTemperatureItlkSts)   itlk_id |= FAP_BOARD_IIB_OVERTEMP_ITLK;
    if (fap.RelativeHumidityItlkSts)   itlk_id |= FAP_BOARD_IIB_OVERHUMIDITY_ITLK;
}

static void get_alarms_id()
{
    if (fap.VinAlarmSts)               alarm_id |= FAP_INPUT_OVERVOLTAGE_ALM;
    if (fap.VoutAlarmSts)              alarm_id |= FAP_OUTPUT_OVERVOLTAGE_ALM;
    if (fap.IoutA1AlarmSts)            alarm_id |= FAP_OUTPUT_OVERCURRENT_1_ALM;
    if (fap.IoutA2AlarmSts)            alarm_id |= FAP_OUTPUT_OVERCURRENT_2_ALM;
    if (fap.TempIGBT1AlarmSts)         alarm_id |= FAP_IGBT1_OVERTEMP_ALM;
    if (fap.TempIGBT2AlarmSts)         alarm_id |= FAP_IGBT2_OVERTEMP_ALM;
    if (fap.TempLAlarmSts)             alarm_id |= FAP_INDUC_OVERTEMP_ALM;
    if (fap.TempHeatSinkAlarmSts)      alarm_id |= FAP_HS_OVERTEMP_ALM;
    if (fap.GroundLeakageAlarmSts)     alarm_id |= FAP_GROUND_LKG_ALM;
    if (fap.DriveVoltageAlarmSts)      alarm_id |= FAP_DRIVER_OVERVOLTAGE_ALM;
    if (fap.Drive1CurrentAlarmSts)     alarm_id |= FAP_DRIVER1_OVERCURRENT_ALM;
    if (fap.Drive2CurrentAlarmSts)     alarm_id |= FAP_DRIVER2_OVERCURRENT_ALM;
    if (fap.BoardTemperatureAlarmSts)  alarm_id |= FAP_BOARD_IIB_OVERTEMP_ALM;
    if (fap.RelativeHumidityAlarmSts)  alarm_id |= FAP_BOARD_IIB_OVERHUMIDITY_ALM;
}

void send_fap_itlk_msg()
{
    //send_interlock_message(itlk_id);
    send_data_message(0);
}

static void config_module()
{
    //Set current range FAP 150 A
    CurrentCh1Init(130.0, 0.130, 50.0, 3); // Corrente bra�o1: Sensor Hall
    CurrentCh2Init(130.0, 0.130, 50.0, 3); // Corrente bra�o2: LEM LA 130-P

    //Set protection limits FAP 150 A
    //     These interlocks are bypassed due to the fact that their ADC's
    //     will most probably saturate during operation at 300 A. These
    //     measures are also performed by UDC, which guarantees these
    //     protections
    CurrentCh1AlarmLevelSet(FAP_OUTPUT_OVERCURRENT_1_ALM_LIM);  // Corrente bra�o1
    CurrentCh1TripLevelSet(FAP_OUTPUT_OVERCURRENT_1_ITLK_LIM);  // Corrente bra�o1
    CurrentCh2AlarmLevelSet(FAP_OUTPUT_OVERCURRENT_2_ALM_LIM);  // Corrente bra�o2
    CurrentCh2TripLevelSet(FAP_OUTPUT_OVERCURRENT_2_ITLK_LIM);  // Corrente bra�o2

    // NTC contiguration type
    //ConfigNtcType(SEMIX);

    //Leitura de tens�o isolada
    LvCurrentCh1Init(720, 0.025, 120.0, 100);
    LvCurrentCh2Init(300, 0.025, 120.0, 100);

    // TODO: Check this values
    LvCurrentCh3Init(50.0, 0.025, 120.0, 3); // Ground Leakage

    LvCurrentCh1AlarmLevelSet(FAP_INPUT_OVERVOLTAGE_ALM_LIM);   // Tens�o de entrada Alarme
    LvCurrentCh1TripLevelSet(FAP_INPUT_OVERVOLTAGE_ITLK_LIM);   // Tens�o de entrada Interlock
    LvCurrentCh2AlarmLevelSet(FAP_OUTPUT_OVERVOLTAGE_ALM_LIM);  // Tens�o de sa�da Alarme
    LvCurrentCh2TripLevelSet(FAP_OUTPUT_OVERVOLTAGE_ITLK_LIM);  // Tens�o de sa�da Interlock

    // Ground Leakage
    // TODO: Check this
    LvCurrentCh3AlarmLevelSet(FAP_GROUND_LEAKAGE_ALM_LIM);  // fuga para o terra alarme
    LvCurrentCh3TripLevelSet(FAP_GROUND_LEAKAGE_ITLK_LIM);  // fuga para o terra interlock

    // PT100 configuration limits
    Pt100SetCh1AlarmLevel(FAP_HS_OVERTEMP_ALM_LIM);     // Temperatura Dissipador
    Pt100SetCh1TripLevel(FAP_HS_OVERTEMP_ITLK_LIM);     // Temperatura Dissipador
    Pt100SetCh2AlarmLevel(FAP_INDUC_OVERTEMP_ALM_LIM);  // Temperatura L
    Pt100SetCh2TripLevel(FAP_INDUC_OVERTEMP_ITLK_LIM);  // Temperatura L

    // Delay 4 seconds
    Pt100SetCh1Delay(4);
    // Delay 4 seconds
    Pt100SetCh2Delay(4);

    // PT100 channel enable
    Pt100Ch1Enable(); // Temperatura Dissipador
    Pt100Ch2Enable(); // Temperatura L
    Pt100Ch3Disable();
    Pt100Ch4Disable();

    // Rh configuration limits
    RhAlarmLimitSet(FAP_RH_ALM_LIM);
    RhTripLimitSet(FAP_RH_ITLK_LIM);

    // Temp board configuration limits
    TempBoardAlarmLimitSet(FAP_BOARD_TEMP_ALM_LIM);
    TempBoardTripLimitSet(FAP_BOARD_TEMP_ITLK_LIM);

    Driver1ErrEnable();
    Driver2ErrEnable();

    DriverVoltageInit(3); // Inserir valor de delay

    // Leitura da tensao dos drivers
    DriverVoltageAlarmLevelSet(FAP_DRIVER_OVERVOLTAGE_ALM_LIM_LIM); // Rogerio adicionou
    DriverVoltageTripLevelSet(FAP_DRIVER_OVERVOLTAGE_ITLK_LIM);     // Rogerio adicionou

    DriverCurrentInit(3); // Inserir valor de delay

    // Leitura da corrente dos drivers
    Driver1CurrentAlarmLevelSet(FAP_DRIVER1_OVERCURRENT_ALM_LIM ); // Rogerio adicionou
    Driver1CurrentTripLevelSet(FAP_DRIVER1_OVERCURRENT_ITLK_LIM);  // Rogerio adicionou

    Driver2CurrentAlarmLevelSet(FAP_DRIVER2_OVERCURRENT_ALM_LIM);  // Rogerio adicionou
    Driver2CurrentTripLevelSet(FAP_DRIVER2_OVERCURRENT_ITLK_LIM);  // Rogerio adicionou


    // Init Variables
    fap.Vin.f                    = 0.0;
    fap.VinAlarmSts              = 0;
    fap.VinItlkSts               = 0;
    fap.Vout.f                   = 0.0;
    fap.VoutAlarmSts             = 0;
    fap.VoutItlkSts              = 0;
    fap.IoutA1.f                 = 0.0;
    fap.IoutA1AlarmSts           = 0;
    fap.IoutA1ItlkSts            = 0;
    fap.IoutA2.f                 = 0.0;
    fap.IoutA2AlarmSts           = 0;
    fap.IoutA2ItlkSts            = 0;
    fap.TempIGBT1.f              = 0.0;
    fap.TempIGBT1AlarmSts        = 0;
    fap.TempIGBT1ItlkSts         = 0;
    fap.TempIGBT1HwrItlk         = 0;
    fap.TempIGBT1HwrItlkSts      = 0;
    fap.TempIGBT2.f              = 0.0;
    fap.TempIGBT2AlarmSts        = 0;
    fap.TempIGBT2ItlkSts         = 0;
    fap.TempIGBT2HwrItlk         = 0;
    fap.TempIGBT2HwrItlkSts      = 0;
    fap.DriveVoltage.f           = 0.0;
    fap.DriveVoltageAlarmSts     = 0;
    fap.DriveVoltageItlkSts      = 0;
    fap.Drive1Current.f          = 0.0;
    fap.Drive1CurrentAlarmSts    = 0;
    fap.Drive1CurrentItlkSts     = 0;
    fap.Drive2Current.f          = 0.0;
    fap.Drive2CurrentAlarmSts    = 0;
    fap.Drive2CurrentItlkSts     = 0;
    fap.Driver1Error             = 0;
    fap.Driver1ErrorItlk         = 0;
    fap.Driver2Error             = 0;
    fap.Driver2ErrorItlk         = 0;
    fap.TempL.f                  = 0;
    fap.TempLAlarmSts            = 0;
    fap.TempLItlkSts             = 0;
    fap.TempHeatSink.f           = 0;
    fap.TempHeatSinkAlarmSts     = 0;
    fap.TempHeatSinkItlkSts      = 0;
    fap.Relay                    = 0;
    fap.ExternalItlk             = 0;
    fap.ExternalItlkSts          = 0;
    fap.Rack                     = 0;
    fap.RackSts                  = 0;
    fap.GroundLeakage.f          = 0;
    fap.GroundLeakageAlarmSts    = 0;
    fap.GroundLeakageItlkSts     = 0;
    fap.BoardTemperature.f       = 0.0;
    fap.BoardTemperatureAlarmSts = 0;
    fap.BoardTemperatureItlkSts  = 0;
    fap.RelativeHumidity.f       = 0.0;
    fap.RelativeHumidityAlarmSts = 0;
    fap.RelativeHumidityItlkSts  = 0;
}
