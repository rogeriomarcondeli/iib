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
 * @file fac_os.c
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 28 de ago de 2018
 *
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#include <iib_modules/fac_os.h>
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

#define FAC_OS_INPUT_OVERVOLTAGE_ALM_LIM            290.0
#define FAC_OS_INPUT_OVERVOLTAGE_ITLK_LIM           295.0

#define FAC_OS_INPUT_OVERCURRENT_ALM_LIM            430.0
#define FAC_OS_INPUT_OVERCURRENT_ITLK_LIM           440.0

#define FAC_OS_OUTPUT_OVERCURRENT_ALM_LIM           570.0
#define FAC_OS_OUTPUT_OVERCURRENT_ITLK_LIM          590.0

#define FAC_OS_IGBT1_OVERTEMP_ALM_LIM               60
#define FAC_OS_IGBT1_OVERTEMP_ITLK_LIM              80

#define FAC_OS_IGBT2_OVERTEMP_ALM_LIM               60
#define FAC_OS_IGBT2_OVERTEMP_ITLK_LIM              80

#define FAC_OS_DRIVER_OVERVOLTAGE_ALM_LIM           16.0
#define FAC_OS_DRIVER_OVERVOLTAGE_ITLK_LIM          17.0

#define FAC_OS_DRIVER1_OVERCURRENT_ALM_LIM          2.0
#define FAC_OS_DRIVER1_OVERCURRENT_ITLK_LIM         2.4

#define FAC_OS_DRIVER2_OVERCURRENT_ALM_LIM          2.0
#define FAC_OS_DRIVER2_OVERCURRENT_ITLK_LIM         2.4

#define FAC_OS_INDUC_OVERTEMP_ALM_LIM               40
#define FAC_OS_INDUC_OVERTEMP_ITLK_LIM              45

#define FAC_OS_HS_OVERTEMP_ALM_LIM                  40
#define FAC_OS_HS_OVERTEMP_ITLK_LIM                 45

#define FAC_OS_RH_OVERHUMIDITY_ALM_LIM              50
#define FAC_OS_RH_OVERHUMIDITY_ITLK_LIM             90

#define FAC_OS_BOARD_OVERTEMP_ALM_LIM               50
#define FAC_OS_BOARD_OVERTEMP_ITLK_LIM              60

/////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    union {
        float       f;
        uint8_t     u8[4];
    } Iin;

    bool IinAlarmSts;
    bool IinItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } Iout;

    bool IoutAlarmSts;
    bool IoutItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } VdcLink;

    bool VdcLinkAlarmSts;
    bool VdcLinkItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } TempIGBT1;

    bool TempIGBT1AlarmSts;
    bool TempIGBT1ItlkSts;
    bool TempIGBT1HwrItlk;
    bool TempIGBT1HwrItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } TempIGBT2;

    bool TempIGBT2AlarmSts;
    bool TempIGBT2ItlkSts;
    bool TempIGBT2HwrItlk;
    bool TempIGBT2HwrItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } DriveVoltage;

    bool DriveVoltageAlarmSts;
    bool DriveVoltageItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } Drive1Current;

    bool Drive1CurrentAlarmSts;
    bool Drive1CurrentItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } Drive2Current;

    bool Drive2CurrentAlarmSts;
    bool Drive2CurrentItlkSts;

    bool Driver1ErrorTop;
    bool Driver1ErrorTopItlkSts;

    bool Driver1ErrorBot;
    bool Driver1ErrorBotItlkSts;

    bool Driver2ErrorTop;
    bool Driver2ErrorTopItlkSts;

    bool Driver2ErrorBot;
    bool Driver2ErrorBotItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } TempL;

    bool TempLAlarmSts;
    bool TempLItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } TempHeatSink;

    bool TempHeatSinkAlarmSts;
    bool TempHeatSinkItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } BoardTemperature;

    bool BoardTemperatureAlarmSts;
    bool BoardTemperatureItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } RelativeHumidity;

    bool RelativeHumidityAlarmSts;
    bool RelativeHumidityItlkSts;

} fac_os_t;

/////////////////////////////////////////////////////////////////////////////////////////////

fac_os_t fac_os;

static uint32_t fac_os_interlocks_indication;
static uint32_t fac_os_alarms_indication;

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

void init_fac_os()
{
    config_module();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fac_os_interlocks()
{
    fac_os.VdcLinkItlkSts           = 0;
    fac_os.IinItlkSts               = 0;
    fac_os.IoutItlkSts              = 0;
    fac_os.TempIGBT1ItlkSts         = 0;
    fac_os.TempIGBT1HwrItlkSts      = 0;
    fac_os.TempIGBT2ItlkSts         = 0;
    fac_os.TempIGBT2HwrItlkSts      = 0;
    fac_os.Driver1ErrorTopItlkSts   = 0;
    fac_os.Driver1ErrorBotItlkSts   = 0;
    fac_os.Driver2ErrorTopItlkSts   = 0;
    fac_os.Driver2ErrorBotItlkSts   = 0;
    fac_os.TempLItlkSts             = 0;
    fac_os.TempHeatSinkItlkSts      = 0;
    fac_os.DriveVoltageItlkSts      = 0;
    fac_os.Drive1CurrentItlkSts     = 0;
    fac_os.Drive2CurrentItlkSts     = 0;
    fac_os.BoardTemperatureItlkSts  = 0;
    fac_os.RelativeHumidityItlkSts  = 0;

    itlk_id = 0;

    send_itlk_message(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fac_os_interlocks()
{
    uint8_t test = 0;

    test |= fac_os.VdcLinkItlkSts;
    test |= fac_os.IinItlkSts;
    test |= fac_os.IoutItlkSts;
    test |= fac_os.TempIGBT1ItlkSts;
    test |= fac_os.TempIGBT1HwrItlkSts;
    test |= fac_os.TempIGBT2ItlkSts;
    test |= fac_os.TempIGBT2HwrItlkSts;
    test |= fac_os.Driver1ErrorTopItlkSts;
    test |= fac_os.Driver1ErrorBotItlkSts;
    test |= fac_os.Driver2ErrorTopItlkSts;
    test |= fac_os.Driver2ErrorBotItlkSts;
    test |= fac_os.TempLItlkSts;
    test |= fac_os.TempHeatSinkItlkSts;
    test |= fac_os.DriveVoltageItlkSts;
    test |= fac_os.Drive1CurrentItlkSts;
    test |= fac_os.Drive2CurrentItlkSts;
    test |= fac_os.BoardTemperatureItlkSts;
    test |= fac_os.RelativeHumidityItlkSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fac_os_alarms()
{
    fac_os.VdcLinkAlarmSts           = 0;
    fac_os.IinAlarmSts               = 0;
    fac_os.IoutAlarmSts              = 0;
    fac_os.TempIGBT1AlarmSts         = 0;
    fac_os.TempIGBT2AlarmSts         = 0;
    fac_os.TempLAlarmSts             = 0;
    fac_os.TempHeatSinkAlarmSts      = 0;
    fac_os.DriveVoltageAlarmSts      = 0;
    fac_os.Drive1CurrentAlarmSts     = 0;
    fac_os.Drive2CurrentAlarmSts     = 0;
    fac_os.BoardTemperatureAlarmSts  = 0;
    fac_os.RelativeHumidityAlarmSts  = 0;

    alarm_id = 0;

    send_alarm_message(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fac_os_alarms()
{
    uint8_t test = 0;

    test |= fac_os.VdcLinkAlarmSts;
    test |= fac_os.IinAlarmSts;
    test |= fac_os.IoutAlarmSts;
    test |= fac_os.TempIGBT1AlarmSts;
    test |= fac_os.TempIGBT2AlarmSts;
    test |= fac_os.TempLAlarmSts;
    test |= fac_os.TempHeatSinkAlarmSts;
    test |= fac_os.DriveVoltageAlarmSts;
    test |= fac_os.Drive1CurrentAlarmSts;
    test |= fac_os.Drive2CurrentAlarmSts;
    test |= fac_os.BoardTemperatureAlarmSts;
    test |= fac_os.RelativeHumidityAlarmSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void check_fac_os_indication_leds()
{
    //Input over voltage
    if(fac_os.VdcLinkItlkSts) Led2TurnOff();
    else if(fac_os.VdcLinkAlarmSts) Led2Toggle();
    else Led2TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Input over current
    if(fac_os.IinItlkSts) Led3TurnOff();
    else if(fac_os.IinAlarmSts) Led3Toggle();
    else Led3TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Output over current
    if(fac_os.IoutItlkSts) Led4TurnOff();
    else if(fac_os.IoutAlarmSts) Led4Toggle();
    else Led4TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks dos Drivers
    if(fac_os.Driver1ErrorTopItlkSts || fac_os.Driver1ErrorBotItlkSts || fac_os.Driver2ErrorTopItlkSts || fac_os.Driver2ErrorBotItlkSts) Led5TurnOff();
    else Led5TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Heatsink Over temperature
    if(fac_os.TempHeatSinkItlkSts) Led6TurnOff();
    else if(fac_os.TempHeatSinkAlarmSts) Led6Toggle();
    else Led6TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Inductor Over temperature
    if(fac_os.TempLItlkSts) Led7TurnOff();
    else if(fac_os.TempLAlarmSts) Led7Toggle();
    else Led7TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Over temperature igbt1 and igbt2
    if(fac_os.TempIGBT1ItlkSts || fac_os.TempIGBT1HwrItlkSts || fac_os.TempIGBT2ItlkSts || fac_os.TempIGBT2HwrItlkSts) Led8TurnOff();
    else if(fac_os.TempIGBT1AlarmSts || fac_os.TempIGBT2AlarmSts) Led8Toggle();
    else Led8TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks dos Drivers
    if(fac_os.DriveVoltageItlkSts || fac_os.Drive1CurrentItlkSts || fac_os.Drive2CurrentItlkSts) Led9TurnOff();
    else if(fac_os.DriveVoltageAlarmSts || fac_os.Drive1CurrentAlarmSts || fac_os.Drive2CurrentAlarmSts) Led9Toggle();
    else Led9TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Temperatura PCB e Umidade Relativa
    if(fac_os.BoardTemperatureItlkSts || fac_os.RelativeHumidityItlkSts) Led10TurnOff();
    else if(fac_os.BoardTemperatureAlarmSts || fac_os.RelativeHumidityAlarmSts) Led10Toggle();
    else Led10TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fac_os_application_readings()
{
    //PT100 CH1 Dissipador
    fac_os.TempHeatSink.f = (float) Pt100ReadCh1();
    fac_os.TempHeatSinkAlarmSts = Pt100ReadCh1AlarmSts();
    if(!fac_os.TempHeatSinkItlkSts)fac_os.TempHeatSinkItlkSts = Pt100ReadCh1TripSts();

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 CH2 Indutor
    fac_os.TempL.f = (float) Pt100ReadCh2();
    fac_os.TempLAlarmSts = Pt100ReadCh2AlarmSts();
    if(!fac_os.TempLItlkSts)fac_os.TempLItlkSts = Pt100ReadCh2TripSts();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT1
    fac_os.TempIGBT1.f = (float) TempIgbt1Read();
    fac_os.TempIGBT1AlarmSts = TempIgbt1AlarmStatusRead();
    if(!fac_os.TempIGBT1ItlkSts)fac_os.TempIGBT1ItlkSts = TempIgbt1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT1 Hardware
    fac_os.TempIGBT1HwrItlk = Driver1OverTempRead();//Variavel usada para debug
    if(!fac_os.TempIGBT1HwrItlkSts)fac_os.TempIGBT1HwrItlkSts = Driver1OverTempRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT2
    fac_os.TempIGBT2.f = (float) TempIgbt2Read();
    fac_os.TempIGBT2AlarmSts = TempIgbt2AlarmStatusRead();
    if(!fac_os.TempIGBT2ItlkSts)fac_os.TempIGBT2ItlkSts = TempIgbt2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT2 Hardware
    fac_os.TempIGBT2HwrItlk = Driver2OverTempRead();//Variavel usada para debug
    if(!fac_os.TempIGBT2HwrItlkSts)fac_os.TempIGBT2HwrItlkSts = Driver2OverTempRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura PCB IIB
    fac_os.BoardTemperature.f = (float) BoardTempRead();
    fac_os.BoardTemperatureAlarmSts = BoardTempAlarmStatusRead();
    if(!fac_os.BoardTemperatureItlkSts)fac_os.BoardTemperatureItlkSts = BoardTempTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Umidade Relativa
    fac_os.RelativeHumidity.f = (float) RhRead();
    fac_os.RelativeHumidityAlarmSts = RhAlarmStatusRead();
    if(!fac_os.RelativeHumidityItlkSts)fac_os.RelativeHumidityItlkSts = RhTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //DriverVotage
    fac_os.DriveVoltage.f = DriverVoltageRead();
    fac_os.DriveVoltageAlarmSts = DriverVoltageAlarmStatusRead();
    if(!fac_os.DriveVoltageItlkSts)fac_os.DriveVoltageItlkSts = DriverVolatgeTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Drive1Current
    fac_os.Drive1Current.f = Driver1CurrentRead();
    fac_os.Drive1CurrentAlarmSts = Driver1CurrentAlarmStatusRead();
    if(!fac_os.Drive1CurrentItlkSts)fac_os.Drive1CurrentItlkSts = Driver1CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Drive2Current
    fac_os.Drive2Current.f = Driver2CurrentRead();
    fac_os.Drive2CurrentAlarmSts = Driver2CurrentAlarmStatusRead();
    if(!fac_os.Drive2CurrentItlkSts)fac_os.Drive2CurrentItlkSts = Driver2CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    fac_os.VdcLink.f = LvCurrentCh1Read();
    fac_os.VdcLinkAlarmSts = LvCurrentCh1AlarmStatusRead();
    if(!fac_os.VdcLinkItlkSts)fac_os.VdcLinkItlkSts = LvCurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    fac_os.Iin.f = CurrentCh1Read();
    fac_os.IinAlarmSts = CurrentCh1AlarmStatusRead();
    if(!fac_os.IinItlkSts)fac_os.IinItlkSts = CurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    fac_os.Iout.f = CurrentCh2Read();
    fac_os.IoutAlarmSts = CurrentCh2AlarmStatusRead();
    if(!fac_os.IoutItlkSts)fac_os.IoutItlkSts = CurrentCh2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 1 Top
    fac_os.Driver1ErrorTop = Driver1TopErrorRead();//Variavel usada para debug
    if(!fac_os.Driver1ErrorTopItlkSts)fac_os.Driver1ErrorTopItlkSts = Driver1TopErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 1 Bot
    fac_os.Driver1ErrorBot = Driver1BotErrorRead();//Variavel usada para debug
    if(!fac_os.Driver1ErrorBotItlkSts)fac_os.Driver1ErrorBotItlkSts = Driver1BotErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 2 Top
    fac_os.Driver2ErrorTop = Driver2TopErrorRead();//Variavel usada para debug
    if(!fac_os.Driver2ErrorTopItlkSts)fac_os.Driver2ErrorTopItlkSts = Driver2TopErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 2 Bot
    fac_os.Driver2ErrorBot = Driver2BotErrorRead();//Variavel usada para debug
    if(!fac_os.Driver2ErrorBotItlkSts)fac_os.Driver2ErrorBotItlkSts = Driver2BotErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Se nao houver sinal na entrada digital dos 6 sinais, defina a acao como Interlock.
    if(fac_os.Driver1ErrorTopItlkSts || fac_os.Driver1ErrorBotItlkSts || fac_os.Driver2ErrorTopItlkSts || fac_os.Driver2ErrorBotItlkSts
       || fac_os.TempIGBT1HwrItlkSts || fac_os.TempIGBT2HwrItlkSts) InterlockSet();

/////////////////////////////////////////////////////////////////////////////////////////////

    map_vars();
    get_itlks_id();
    get_alarms_id();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fac_os_power_on_check()
{
    Led1TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void map_vars()
{
    fac_os_interlocks_indication = itlk_id;
    fac_os_alarms_indication = alarm_id;

    g_controller_iib.iib_itlk[0].u32        = fac_os_interlocks_indication;
    g_controller_iib.iib_itlk[1].u32        = ResetInterlocksRegister;

    g_controller_iib.iib_alarm[0].u32       = fac_os_alarms_indication;
    g_controller_iib.iib_alarm[1].u32       = ResetAlarmsRegister;

    g_controller_iib.iib_signals[0].f       = fac_os.VdcLink.f;
    g_controller_iib.iib_signals[1].f       = fac_os.Iin.f;
    g_controller_iib.iib_signals[2].f       = fac_os.Iout.f;
    g_controller_iib.iib_signals[3].f       = fac_os.TempIGBT1.f;
    g_controller_iib.iib_signals[4].f       = fac_os.TempIGBT2.f;
    g_controller_iib.iib_signals[5].f       = fac_os.DriveVoltage.f;
    g_controller_iib.iib_signals[6].f       = fac_os.Drive1Current.f;
    g_controller_iib.iib_signals[7].f       = fac_os.Drive2Current.f;
    g_controller_iib.iib_signals[8].f       = fac_os.TempL.f;
    g_controller_iib.iib_signals[9].f       = fac_os.TempHeatSink.f;
    g_controller_iib.iib_signals[10].f      = fac_os.BoardTemperature.f;
    g_controller_iib.iib_signals[11].f      = fac_os.RelativeHumidity.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_fac_os_data()
{
    static uint8_t i = 0;

    send_data_message(i);

    i++;

    if (i > 11) i = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void get_itlks_id()
{
    if (fac_os.VdcLinkItlkSts)              itlk_id |= FAC_OS_INPUT_OVERVOLTAGE_ITLK;
    if (fac_os.IinItlkSts)                  itlk_id |= FAC_OS_INPUT_OVERCURRENT_ITLK;
    if (fac_os.IoutItlkSts)                 itlk_id |= FAC_OS_OUTPUT_OVERCURRENT_ITLK;
    if (fac_os.TempIGBT1ItlkSts)            itlk_id |= FAC_OS_IGBT1_OVERTEMP_ITLK;
    if (fac_os.TempIGBT1HwrItlkSts)         itlk_id |= FAC_OS_IGBT1_HWR_OVERTEMP_ITLK;
    if (fac_os.TempIGBT2ItlkSts)            itlk_id |= FAC_OS_IGBT2_OVERTEMP_ITLK;
    if (fac_os.TempIGBT2HwrItlkSts)         itlk_id |= FAC_OS_IGBT2_HWR_OVERTEMP_ITLK;
    if (fac_os.DriveVoltageItlkSts)         itlk_id |= FAC_OS_DRIVER_OVERVOLTAGE_ITLK;
    if (fac_os.Drive1CurrentItlkSts)        itlk_id |= FAC_OS_DRIVER1_OVERCURRENT_ITLK;
    if (fac_os.Drive2CurrentItlkSts)        itlk_id |= FAC_OS_DRIVER2_OVERCURRENT_ITLK;
    if (fac_os.Driver1ErrorTopItlkSts)      itlk_id |= FAC_OS_DRIVER1_ERROR_TOP_ITLK;
    if (fac_os.Driver1ErrorBotItlkSts)      itlk_id |= FAC_OS_DRIVER1_ERROR_BOT_ITLK;
    if (fac_os.Driver2ErrorTopItlkSts)      itlk_id |= FAC_OS_DRIVER2_ERROR_TOP_ITLK;
    if (fac_os.Driver2ErrorBotItlkSts)      itlk_id |= FAC_OS_DRIVER2_ERROR_BOT_ITLK;
    if (fac_os.TempLItlkSts)                itlk_id |= FAC_OS_INDUC_OVERTEMP_ITLK;
    if (fac_os.TempHeatSinkItlkSts)         itlk_id |= FAC_OS_HS_OVERTEMP_ITLK;
    if (fac_os.BoardTemperatureItlkSts)     itlk_id |= FAC_OS_BOARD_IIB_OVERTEMP_ITLK;
    if (fac_os.RelativeHumidityItlkSts)     itlk_id |= FAC_OS_BOARD_IIB_OVERHUMIDITY_ITLK;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_output_fac_os_itlk_msg()
{
    send_itlk_message(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void get_alarms_id()
{
    if (fac_os.VdcLinkAlarmSts)             alarm_id |= FAC_OS_INPUT_OVERVOLTAGE_ALM;
    if (fac_os.IinAlarmSts)                 alarm_id |= FAC_OS_INPUT_OVERCURRENT_ALM;
    if (fac_os.IoutAlarmSts)                alarm_id |= FAC_OS_OUTPUT_OVERCURRENT_ALM;
    if (fac_os.TempIGBT1AlarmSts)           alarm_id |= FAC_OS_IGBT1_OVERTEMP_ALM;
    if (fac_os.TempIGBT2AlarmSts)           alarm_id |= FAC_OS_IGBT2_OVERTEMP_ALM;
    if (fac_os.DriveVoltageAlarmSts)        alarm_id |= FAC_OS_DRIVER_OVERVOLTAGE_ALM;
    if (fac_os.Drive1CurrentAlarmSts)       alarm_id |= FAC_OS_DRIVER1_OVERCURRENT_ALM;
    if (fac_os.Drive2CurrentAlarmSts)       alarm_id |= FAC_OS_DRIVER2_OVERCURRENT_ALM;
    if (fac_os.TempLAlarmSts)               alarm_id |= FAC_OS_INDUC_OVERTEMP_ALM;
    if (fac_os.TempHeatSinkAlarmSts)        alarm_id |= FAC_OS_HS_OVERTEMP_ALM;
    if (fac_os.BoardTemperatureAlarmSts)    alarm_id |= FAC_OS_BOARD_IIB_OVERTEMP_ALM;
    if (fac_os.RelativeHumidityAlarmSts)    alarm_id |= FAC_OS_BOARD_IIB_OVERHUMIDITY_ALM;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void config_module()
{
    /* Set current range */
    CurrentCh1Init(300.0, 0.150, 50.0, 0); /* Input */
    CurrentCh2Init(500.0, 0.100, 50.0, 0); /* Output */

    CurrentCh1Enable();  //CurrentCh1 enable
    CurrentCh2Enable();  //CurrentCh2 enable
    CurrentCh3Disable(); //CurrentCh3 disable
    CurrentCh4Disable(); //CurrentCh4 disable

    /* Protection Limits */
    CurrentCh1AlarmLevelSet(FAC_OS_INPUT_OVERCURRENT_ALM_LIM);
    CurrentCh1TripLevelSet(FAC_OS_INPUT_OVERCURRENT_ITLK_LIM);
    CurrentCh2AlarmLevelSet(FAC_OS_OUTPUT_OVERCURRENT_ALM_LIM);
    CurrentCh2TripLevelSet(FAC_OS_OUTPUT_OVERCURRENT_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    /* Isolated Voltage */
    LvCurrentCh1Init(330.0, 0.025, 120.0, 3); /* Input Voltage */

    LvCurrentCh1Enable();  //LvCurrentCh1 enable
    LvCurrentCh2Disable(); //LvCurrentCh2 disable
    LvCurrentCh3Disable(); //LvCurrentCh3 disable

    /* Protection Limits */
    LvCurrentCh1AlarmLevelSet(FAC_OS_INPUT_OVERVOLTAGE_ALM_LIM);
    LvCurrentCh1TripLevelSet(FAC_OS_INPUT_OVERVOLTAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 configuration
    //Delay 2 seconds
    Pt100SetCh1Delay(2);
    Pt100SetCh2Delay(2);

    /* Pt-100 channel enable */
    Pt100Ch1Enable();
    Pt100Ch2Enable();
    Pt100Ch3Disable();
    Pt100Ch4Disable();

    /* Pt-100 Configuration Limits */
    Pt100SetCh1AlarmLevel(FAC_OS_HS_OVERTEMP_ALM_LIM);
    Pt100SetCh1TripLevel(FAC_OS_HS_OVERTEMP_ITLK_LIM);
    Pt100SetCh2AlarmLevel(FAC_OS_INDUC_OVERTEMP_ALM_LIM);
    Pt100SetCh2TripLevel(FAC_OS_INDUC_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature igbt1 configuration
    TempIgbt1Delay(3); //Inserir valor de delay

    TempIgbt1Disable(); //TempIgbt1 disable

    //Temp Igbt1 configuration limits
    TempIgbt1AlarmLevelSet(FAC_OS_IGBT1_OVERTEMP_ALM_LIM);
    TempIgbt1TripLevelSet(FAC_OS_IGBT1_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature igbt2 configuration
    TempIgbt2Delay(3); //Inserir valor de delay

    TempIgbt2Disable(); //TempIgbt2 disable

    //Temp Igbt2 configuration limits
    TempIgbt2AlarmLevelSet(FAC_OS_IGBT2_OVERTEMP_ALM_LIM);
    TempIgbt2TripLevelSet(FAC_OS_IGBT2_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature Board configuration
    BoardTempDelay(3); //Inserir valor de delay

    BoardTempEnable(); //BoardTemp enable

    //Temp board configuration limits
    BoardTempAlarmLevelSet(FAC_OS_BOARD_OVERTEMP_ALM_LIM);
    BoardTempTripLevelSet(FAC_OS_BOARD_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Humidity Board configuration
    RhDelay(3); //Inserir valor de delay

    RhEnable(); //Rh enable

    //Rh configuration limits
    RhAlarmLevelSet(FAC_OS_RH_OVERHUMIDITY_ALM_LIM);
    RhTripLevelSet(FAC_OS_RH_OVERHUMIDITY_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Driver1 error configuration
    Driver1TopErrorDisable(); //Desabilitado driver error 1 Top
    Driver1BotErrorDisable(); //Desabilitado driver error 1 Bot
    Driver1OverTempDisable(); //Desabilitado Temperatura por Hardware do modulo 1

    //Driver2 error configuration
    Driver2TopErrorDisable(); //Desabilitado driver error 2 Top
    Driver2BotErrorDisable(); //Desabilitado driver error 2 Bot
    Driver2OverTempDisable(); //Desabilitado Temperatura por Hardware do modulo 2

/////////////////////////////////////////////////////////////////////////////////////////////

    //Driver Voltage configuration
    DriverVoltageInit();

    DriverVoltageDelay(3); //Inserir valor de delay

    DriverVoltageDisable(); //DriverVoltage disable

    //Limite de alarme e interlock da tensao dos drivers
    DriverVoltageAlarmLevelSet(FAC_OS_DRIVER_OVERVOLTAGE_ALM_LIM);
    DriverVoltageTripLevelSet(FAC_OS_DRIVER_OVERVOLTAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Driver Current configuration
    DriverCurrentInit();

    DriverCurrentDelay(3); //Inserir valor de delay

    Driver1CurrentDisable(); //Driver1Current disable
    Driver2CurrentDisable(); //Driver2Current disable

    //Limite de alarme e interlock da corrente do driver 1
    Driver1CurrentAlarmLevelSet(FAC_OS_DRIVER1_OVERCURRENT_ALM_LIM);
    Driver1CurrentTripLevelSet(FAC_OS_DRIVER1_OVERCURRENT_ITLK_LIM);

    //Limite de alarme e interlock da corrente do driver 2
    Driver2CurrentAlarmLevelSet(FAC_OS_DRIVER2_OVERCURRENT_ALM_LIM);
    Driver2CurrentTripLevelSet(FAC_OS_DRIVER2_OVERCURRENT_ITLK_LIM);

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
    Gpdi5Disable();  //Gpdi5 disable
    Gpdi6Disable();  //Gpdi6 disable
    Gpdi7Disable();  //Gpdi7 disable
    Gpdi8Disable();  //Gpdi8 disable
    Gpdi9Disable();  //Gpdi9 disable
    Gpdi10Disable(); //Gpdi10 disable
    Gpdi11Disable(); //Gpdi11 disable
    Gpdi12Disable(); //Gpdi12 disable

    //Gpdo configuration
    Gpdo1Enable();   //Gpdo1 enable
    Gpdo2Enable();   //Gpdo2 enable
    Gpdo3Disable();  //Gpdo3 disable
    Gpdo4Disable();  //Gpdo4 disable

    //ReleAux and ReleExtItlk configuration
    ReleAuxEnable(); //ReleAux enable
    ReleExtItlkEnable(); //ReleExtItlk enable

/////////////////////////////////////////////////////////////////////////////////////////////

    //Init Variables
    fac_os.Iin.f                        = 0.0;
    fac_os.IinAlarmSts                  = 0;
    fac_os.IinItlkSts                   = 0;
    fac_os.Iout.f                       = 0.0;
    fac_os.IoutAlarmSts                 = 0;
    fac_os.IoutItlkSts                  = 0;
    fac_os.VdcLink.f                    = 0.0;
    fac_os.VdcLinkAlarmSts              = 0;
    fac_os.VdcLinkItlkSts               = 0;
    fac_os.TempIGBT1.f                  = 0.0;
    fac_os.TempIGBT1AlarmSts            = 0;
    fac_os.TempIGBT1ItlkSts             = 0;
    fac_os.TempIGBT1HwrItlk             = 0;
    fac_os.TempIGBT1HwrItlkSts          = 0;
    fac_os.TempIGBT2.f                  = 0.0;
    fac_os.TempIGBT2AlarmSts            = 0;
    fac_os.TempIGBT2ItlkSts             = 0;
    fac_os.TempIGBT2HwrItlk             = 0;
    fac_os.TempIGBT2HwrItlkSts          = 0;
    fac_os.DriveVoltage.f               = 0.0;
    fac_os.DriveVoltageAlarmSts         = 0;
    fac_os.DriveVoltageItlkSts          = 0;
    fac_os.Drive1Current.f              = 0.0;
    fac_os.Drive1CurrentAlarmSts        = 0;
    fac_os.Drive1CurrentItlkSts         = 0;
    fac_os.Drive2Current.f              = 0.0;
    fac_os.Drive2CurrentAlarmSts        = 0;
    fac_os.Drive2CurrentItlkSts         = 0;
    fac_os.Driver1ErrorTop              = 0;
    fac_os.Driver1ErrorTopItlkSts       = 0;
    fac_os.Driver1ErrorBot              = 0;
    fac_os.Driver1ErrorBotItlkSts       = 0;
    fac_os.Driver2ErrorTop              = 0;
    fac_os.Driver2ErrorTopItlkSts       = 0;
    fac_os.Driver2ErrorBot              = 0;
    fac_os.Driver2ErrorBotItlkSts       = 0;
    fac_os.TempL.f                      = 0.0;
    fac_os.TempLAlarmSts                = 0;
    fac_os.TempLItlkSts                 = 0;
    fac_os.TempHeatSink.f               = 0.0;
    fac_os.TempHeatSinkAlarmSts         = 0;
    fac_os.TempHeatSinkItlkSts          = 0;
    fac_os.BoardTemperature.f           = 0.0;
    fac_os.BoardTemperatureAlarmSts     = 0;
    fac_os.BoardTemperatureItlkSts      = 0;
    fac_os.RelativeHumidity.f           = 0.0;
    fac_os.RelativeHumidityAlarmSts     = 0;
    fac_os.RelativeHumidityItlkSts      = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////







