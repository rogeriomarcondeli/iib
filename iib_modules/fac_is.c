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
 * @file fac_is.c
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 20 de out de 2018
 *
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#include <iib_modules/fac_is.h>
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

#define FAC_IS_DCLINK_OVERVOLTAGE_ALM_LIM           560.0
#define FAC_IS_DCLINK_OVERVOLTAGE_ITLK_LIM          570.0

#define FAC_IS_INPUT_OVERCURRENT_ALM_LIM            160.0
#define FAC_IS_INPUT_OVERCURRENT_ITLK_LIM           170.0

#define FAC_IS_IGBT1_OVERTEMP_ALM_LIM               60
#define FAC_IS_IGBT1_OVERTEMP_ITLK_LIM              80

#define FAC_IS_DRIVER_OVERVOLTAGE_ALM_LIM           16.0
#define FAC_IS_DRIVER_OVERVOLTAGE_ITLK_LIM          17.0

#define FAC_IS_DRIVER1_OVERCURRENT_ALM_LIM          2.0
#define FAC_IS_DRIVER1_OVERCURRENT_ITLK_LIM         2.4

#define FAC_IS_INDUC_OVERTEMP_ALM_LIM               55.0
#define FAC_IS_INDUC_OVERTEMP_ITLK_LIM              60.0

#define FAC_IS_HS_OVERTEMP_ALM_LIM                  50.0
#define FAC_IS_HS_OVERTEMP_ITLK_LIM                 60.0

#define FAC_IS_RH_OVERHUMIDITY_ALM_LIM              50
#define FAC_IS_RH_OVERHUMIDITY_ITLK_LIM             90

#define FAC_IS_BOARD_OVERTEMP_ALM_LIM               50
#define FAC_IS_BOARD_OVERTEMP_ITLK_LIM              60

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
    } DriveVoltage;

    bool DriveVoltageAlarmSts;
    bool DriveVoltageItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } Drive1Current;

    bool Drive1CurrentAlarmSts;
    bool Drive1CurrentItlkSts;

    bool Driver1ErrorTop;
    bool Driver1ErrorTopItlkSts;

    bool Driver1ErrorBot;
    bool Driver1ErrorBotItlkSts;

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

} fac_is_t;

/////////////////////////////////////////////////////////////////////////////////////////////

fac_is_t fac_is;

static uint32_t fac_is_interlocks_indication;
static uint32_t fac_is_alarms_indication;

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

void init_fac_is()
{
    config_module();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fac_is_interlocks()
{
    fac_is.VdcLinkItlkSts            = 0;
    fac_is.IinItlkSts                = 0;
    fac_is.TempIGBT1ItlkSts          = 0;
    fac_is.TempIGBT1HwrItlkSts       = 0;
    fac_is.Driver1ErrorTopItlkSts    = 0;
    fac_is.Driver1ErrorBotItlkSts    = 0;
    fac_is.TempLItlkSts              = 0;
    fac_is.TempHeatSinkItlkSts       = 0;
    fac_is.DriveVoltageItlkSts       = 0;
    fac_is.Drive1CurrentItlkSts      = 0;
    fac_is.BoardTemperatureItlkSts   = 0;
    fac_is.RelativeHumidityItlkSts   = 0;

    itlk_id = 0;

    send_itlk_message(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fac_is_interlocks()
{
    uint8_t test = 0;

    test |= fac_is.VdcLinkItlkSts;
    test |= fac_is.IinItlkSts;
    test |= fac_is.TempIGBT1ItlkSts;
    test |= fac_is.TempIGBT1HwrItlkSts;
    test |= fac_is.Driver1ErrorTopItlkSts;
    test |= fac_is.Driver1ErrorBotItlkSts;
    test |= fac_is.TempLItlkSts;
    test |= fac_is.TempHeatSinkItlkSts;
    test |= fac_is.DriveVoltageItlkSts;
    test |= fac_is.Drive1CurrentItlkSts;
    test |= fac_is.BoardTemperatureItlkSts;
    test |= fac_is.RelativeHumidityItlkSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fac_is_alarms()
{
    fac_is.VdcLinkAlarmSts           = 0;
    fac_is.IinAlarmSts               = 0;
    fac_is.TempIGBT1AlarmSts         = 0;
    fac_is.TempLAlarmSts             = 0;
    fac_is.TempHeatSinkAlarmSts      = 0;
    fac_is.DriveVoltageAlarmSts      = 0;
    fac_is.Drive1CurrentAlarmSts     = 0;
    fac_is.BoardTemperatureAlarmSts  = 0;
    fac_is.RelativeHumidityAlarmSts  = 0;

    alarm_id = 0;

    send_alarm_message(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fac_is_alarms()
{
    uint8_t test = 0;

    test |= fac_is.VdcLinkAlarmSts;
    test |= fac_is.IinAlarmSts;
    test |= fac_is.TempIGBT1AlarmSts;
    test |= fac_is.TempLAlarmSts;
    test |= fac_is.TempHeatSinkAlarmSts;
    test |= fac_is.DriveVoltageAlarmSts;
    test |= fac_is.Drive1CurrentAlarmSts;
    test |= fac_is.BoardTemperatureAlarmSts;
    test |= fac_is.RelativeHumidityAlarmSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void check_fac_is_indication_leds()
{
    // Dc-Link Overvoltage
    if(fac_is.VdcLinkItlkSts) Led2TurnOff();
    else if(fac_is.VdcLinkAlarmSts) Led2Toggle();
    else Led2TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Input Over Current
    if(fac_is.IinItlkSts) Led3TurnOff();
    else if(fac_is.IinAlarmSts) Led3Toggle();
    else Led3TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks do Driver 1
    if(fac_is.Driver1ErrorTopItlkSts || fac_is.Driver1ErrorBotItlkSts) Led4TurnOff();
    else Led4TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Heatsink Over Temperature
    if(fac_is.TempHeatSinkItlkSts) Led5TurnOff();
    else if(fac_is.TempHeatSinkAlarmSts) Led5Toggle();
    else Led5TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Inductor Over Temperature
    if(fac_is.TempLItlkSts) Led6TurnOff();
    else if(fac_is.TempLAlarmSts) Led6Toggle();
    else Led6TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Over temperature igbt1
    if(fac_is.TempIGBT1ItlkSts || fac_is.TempIGBT1HwrItlkSts) Led7TurnOff();
    else if(fac_is.TempIGBT1AlarmSts) Led7Toggle();
    else Led7TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks do Driver
    if(fac_is.DriveVoltageItlkSts || fac_is.Drive1CurrentItlkSts) Led8TurnOff();
    else if(fac_is.DriveVoltageAlarmSts || fac_is.Drive1CurrentAlarmSts) Led8Toggle();
    else Led8TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Temperatura PCB
    if(fac_is.BoardTemperatureItlkSts) Led9TurnOff();
    else if(fac_is.BoardTemperatureAlarmSts) Led9Toggle();
    else Led9TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Umidade Relativa
    if(fac_is.RelativeHumidityItlkSts) Led10TurnOff();
    else if(fac_is.RelativeHumidityAlarmSts) Led10Toggle();
    else Led10TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fac_is_application_readings()
{
    //PT100 CH1 Dissipador
    fac_is.TempHeatSink.f = (float) Pt100ReadCh1();
    fac_is.TempHeatSinkAlarmSts = Pt100ReadCh1AlarmSts();
    if(!fac_is.TempHeatSinkItlkSts)fac_is.TempHeatSinkItlkSts = Pt100ReadCh1TripSts();

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 CH2 Indutor
    fac_is.TempL.f = (float) Pt100ReadCh2();
    fac_is.TempLAlarmSts = Pt100ReadCh2AlarmSts();
    if(!fac_is.TempLItlkSts)fac_is.TempLItlkSts = Pt100ReadCh2TripSts();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT1
    fac_is.TempIGBT1.f = (float) TempIgbt1Read();
    fac_is.TempIGBT1AlarmSts = TempIgbt1AlarmStatusRead();
    if(!fac_is.TempIGBT1ItlkSts)fac_is.TempIGBT1ItlkSts = TempIgbt1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura IGBT1 Hardware
    fac_is.TempIGBT1HwrItlk = Driver1OverTempRead();//Variavel usada para debug
    if(!fac_is.TempIGBT1HwrItlkSts)fac_is.TempIGBT1HwrItlkSts = Driver1OverTempRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura PCB IIB
    fac_is.BoardTemperature.f = (float) BoardTempRead();
    fac_is.BoardTemperatureAlarmSts = BoardTempAlarmStatusRead();
    if(!fac_is.BoardTemperatureItlkSts)fac_is.BoardTemperatureItlkSts = BoardTempTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Umidade Relativa
    fac_is.RelativeHumidity.f = (float) RhRead();
    fac_is.RelativeHumidityAlarmSts = RhAlarmStatusRead();
    if(!fac_is.RelativeHumidityItlkSts)fac_is.RelativeHumidityItlkSts = RhTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //DriverVotage
    fac_is.DriveVoltage.f = DriverVoltageRead();
    fac_is.DriveVoltageAlarmSts = DriverVoltageAlarmStatusRead();
    if(!fac_is.DriveVoltageItlkSts)fac_is.DriveVoltageItlkSts = DriverVolatgeTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Drive1Current
    fac_is.Drive1Current.f = Driver1CurrentRead();
    fac_is.Drive1CurrentAlarmSts = Driver1CurrentAlarmStatusRead();
    if(!fac_is.Drive1CurrentItlkSts)fac_is.Drive1CurrentItlkSts = Driver1CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    fac_is.VdcLink.f = LvCurrentCh1Read();
    fac_is.VdcLinkAlarmSts = CurrentCh1AlarmStatusRead();
    if(!fac_is.VdcLinkItlkSts)fac_is.VdcLinkItlkSts = CurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    fac_is.Iin.f = CurrentCh1Read();
    fac_is.IinAlarmSts = CurrentCh1AlarmStatusRead();
    if(!fac_is.IinItlkSts)fac_is.IinItlkSts = CurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 1 Top
    fac_is.Driver1ErrorTop = Driver1TopErrorRead();//Variavel usada para debug
    if(!fac_is.Driver1ErrorTopItlkSts)fac_is.Driver1ErrorTopItlkSts = Driver1TopErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Erro do Driver 1 Bot
    fac_is.Driver1ErrorBot = Driver1BotErrorRead();//Variavel usada para debug
    if(!fac_is.Driver1ErrorBotItlkSts)fac_is.Driver1ErrorBotItlkSts = Driver1BotErrorRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Se nao houver sinal na entrada digital dos 3 sinais, defina a acao como Interlock.
    if(fac_is.Driver1ErrorTopItlkSts || fac_is.Driver1ErrorBotItlkSts || fac_is.TempIGBT1HwrItlkSts) InterlockSet();

/////////////////////////////////////////////////////////////////////////////////////////////

    map_vars();
    get_itlks_id();
    get_alarms_id();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fac_is_power_on_check()
{
    Led1TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void map_vars()
{
    fac_is_interlocks_indication = itlk_id;
    fac_is_alarms_indication = alarm_id;

    g_controller_iib.iib_itlk[0].u32        = fac_is_interlocks_indication;
    g_controller_iib.iib_itlk[1].u32        = ResetInterlocksRegister;

    g_controller_iib.iib_alarm[0].u32       = fac_is_alarms_indication;
    g_controller_iib.iib_alarm[1].u32       = ResetAlarmsRegister;

    g_controller_iib.iib_signals[0].f       = fac_is.VdcLink.f;
    g_controller_iib.iib_signals[1].f       = fac_is.Iin.f;
    g_controller_iib.iib_signals[2].f       = fac_is.TempIGBT1.f;
    g_controller_iib.iib_signals[3].f       = fac_is.DriveVoltage.f;
    g_controller_iib.iib_signals[4].f       = fac_is.Drive1Current.f;
    g_controller_iib.iib_signals[5].f       = fac_is.TempL.f;
    g_controller_iib.iib_signals[6].f       = fac_is.TempHeatSink.f;
    g_controller_iib.iib_signals[7].f       = fac_is.BoardTemperature.f;
    g_controller_iib.iib_signals[8].f       = fac_is.RelativeHumidity.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_fac_is_data()
{
    static uint8_t i = 0;

    send_data_message(i);

    i++;

    if (i > 8) i = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void get_itlks_id()
{
    if (fac_is.VdcLinkItlkSts)              itlk_id |= FAC_IS_DCLINK_OVERVOLTAGE_ITLK;
    if (fac_is.IinItlkSts)                  itlk_id |= FAC_IS_INPUT_OVERCURRENT_ITLK;
    if (fac_is.TempIGBT1ItlkSts)            itlk_id |= FAC_IS_IGBT1_OVERTEMP_ITLK;
    if (fac_is.TempIGBT1HwrItlkSts)         itlk_id |= FAC_IS_IGBT1_HWR_OVERTEMP_ITLK;
    if (fac_is.DriveVoltageItlkSts)         itlk_id |= FAC_IS_DRIVER_OVERVOLTAGE_ITLK;
    if (fac_is.Drive1CurrentItlkSts)        itlk_id |= FAC_IS_DRIVER1_OVERCURRENT_ITLK;
    if (fac_is.Driver1ErrorTopItlkSts)      itlk_id |= FAC_IS_DRIVER1_ERROR_TOP_ITLK;
    if (fac_is.Driver1ErrorBotItlkSts)      itlk_id |= FAC_IS_DRIVER1_ERROR_BOT_ITLK;
    if (fac_is.TempLItlkSts)                itlk_id |= FAC_IS_INDUC_OVERTEMP_ITLK;
    if (fac_is.TempHeatSinkItlkSts)         itlk_id |= FAC_IS_HS_OVERTEMP_ITLK;
    if (fac_is.BoardTemperatureItlkSts)     itlk_id |= FAC_IS_BOARD_IIB_OVERTEMP_ITLK;
    if (fac_is.RelativeHumidityItlkSts)     itlk_id |= FAC_IS_BOARD_IIB_OVERHUMIDITY_ITLK;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_input_fac_is_itlk_msg()
{
    send_itlk_message(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void get_alarms_id()
{
    if (fac_is.VdcLinkAlarmSts)             alarm_id |= FAC_IS_DCLINK_OVERVOLTAGE_ALM;
    if (fac_is.IinAlarmSts)                 alarm_id |= FAC_IS_INPUT_OVERCURRENT_ALM;
    if (fac_is.TempIGBT1AlarmSts)           alarm_id |= FAC_IS_IGBT1_OVERTEMP_ALM;
    if (fac_is.DriveVoltageAlarmSts)        alarm_id |= FAC_IS_DRIVER_OVERVOLTAGE_ALM;
    if (fac_is.Drive1CurrentAlarmSts)       alarm_id |= FAC_IS_DRIVER1_OVERCURRENT_ALM;
    if (fac_is.TempLAlarmSts)               alarm_id |= FAC_IS_INDUC_OVERTEMP_ALM;
    if (fac_is.TempHeatSinkAlarmSts)        alarm_id |= FAC_IS_HS_OVERTEMP_ALM;
    if (fac_is.BoardTemperatureAlarmSts)    alarm_id |= FAC_IS_BOARD_IIB_OVERTEMP_ALM;
    if (fac_is.RelativeHumidityAlarmSts)    alarm_id |= FAC_IS_BOARD_IIB_OVERHUMIDITY_ALM;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void config_module()
{
    /* Set current range */
    CurrentCh1Init(300.0, 0.150, 50.0, 10); /* Input current */

    CurrentCh1Enable();  //CurrentCh1 enable
    CurrentCh2Disable(); //CurrentCh2 disable
    CurrentCh3Disable(); //CurrentCh3 disable
    CurrentCh4Disable(); //CurrentCh4 disable

    /* Protection Limits */
    CurrentCh1AlarmLevelSet(FAC_IS_INPUT_OVERCURRENT_ALM_LIM);
    CurrentCh1TripLevelSet(FAC_IS_INPUT_OVERCURRENT_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    /* Isolated Voltage */
    LvCurrentCh1Init(555.0, 0.025, 120.0, 10); /* Input Voltage */

    LvCurrentCh1Enable();  //LvCurrentCh1 enable
    LvCurrentCh2Disable(); //LvCurrentCh2 disable
    LvCurrentCh3Disable(); //LvCurrentCh3 disable

    /* Protection Limits */
    LvCurrentCh1AlarmLevelSet(FAC_IS_DCLINK_OVERVOLTAGE_ALM_LIM);
    LvCurrentCh1TripLevelSet(FAC_IS_DCLINK_OVERVOLTAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 configuration
    //Delay 2 seconds
    Pt100SetCh1Delay(4);
    Pt100SetCh2Delay(4);

    /* Pt-100 channel enable */
    Pt100Ch1Enable();
    Pt100Ch2Enable();
    Pt100Ch3Disable();
    Pt100Ch4Disable();

    /* Pt-100 Configuration Limits */
    Pt100SetCh1AlarmLevel(FAC_IS_HS_OVERTEMP_ALM_LIM);
    Pt100SetCh1TripLevel(FAC_IS_HS_OVERTEMP_ITLK_LIM);
    Pt100SetCh2AlarmLevel(FAC_IS_INDUC_OVERTEMP_ALM_LIM);
    Pt100SetCh2TripLevel(FAC_IS_INDUC_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature igbt1 configuration
    TempIgbt1Delay(3); //Inserir valor de delay

    TempIgbt1Disable(); //TempIgbt1 disable
    TempIgbt2Disable(); //TempIgbt1 disable

    //Temp Igbt1 configuration limits
    TempIgbt1AlarmLevelSet(FAC_IS_IGBT1_OVERTEMP_ALM_LIM);
    TempIgbt1TripLevelSet(FAC_IS_IGBT1_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature Board configuration
    BoardTempDelay(3); //Inserir valor de delay

    BoardTempEnable(); //BoardTemp enable

    //Temp board configuration limits
    BoardTempAlarmLevelSet(FAC_IS_BOARD_OVERTEMP_ALM_LIM);
    BoardTempTripLevelSet(FAC_IS_BOARD_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Humidity Board configuration
    RhDelay(3); //Inserir valor de delay

    RhEnable(); //Rh enable

    //Rh configuration limits
    RhAlarmLevelSet(FAC_IS_RH_OVERHUMIDITY_ALM_LIM);
    RhTripLevelSet(FAC_IS_RH_OVERHUMIDITY_ITLK_LIM);

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
    DriverVoltageAlarmLevelSet(FAC_IS_DRIVER_OVERVOLTAGE_ALM_LIM);
    DriverVoltageTripLevelSet(FAC_IS_DRIVER_OVERVOLTAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Driver Current configuration
    DriverCurrentInit();

    DriverCurrentDelay(3); //Inserir valor de delay

    Driver1CurrentDisable(); //Driver1Current disable
    Driver2CurrentDisable(); //Driver2Current disable

    //Limite de alarme e interlock da corrente do driver 1
    Driver1CurrentAlarmLevelSet(FAC_IS_DRIVER1_OVERCURRENT_ALM_LIM);
    Driver1CurrentTripLevelSet(FAC_IS_DRIVER1_OVERCURRENT_ITLK_LIM);

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
    Gpdo1Disable();  //Gpdo1 disable
    Gpdo2Disable();  //Gpdo2 disable
    Gpdo3Disable();  //Gpdo3 disable
    Gpdo4Disable();  //Gpdo4 disable

    //ReleAux and ReleExtItlk configuration
    ReleAuxEnable(); //ReleAux enable
    ReleExtItlkEnable(); //ReleExtItlk enable

/////////////////////////////////////////////////////////////////////////////////////////////

    // Init Variables
    fac_is.Iin.f                      = 0.0;
    fac_is.IinAlarmSts                = 0;
    fac_is.IinItlkSts                 = 0;
    fac_is.VdcLink.f                  = 0.0;
    fac_is.VdcLinkAlarmSts            = 0;
    fac_is.VdcLinkItlkSts             = 0;
    fac_is.TempIGBT1.f                = 0.0;
    fac_is.TempIGBT1AlarmSts          = 0;
    fac_is.TempIGBT1ItlkSts           = 0;
    fac_is.TempIGBT1HwrItlk           = 0;
    fac_is.TempIGBT1HwrItlkSts        = 0;
    fac_is.DriveVoltage.f             = 0.0;
    fac_is.DriveVoltageAlarmSts       = 0;
    fac_is.DriveVoltageItlkSts        = 0;
    fac_is.Drive1Current.f            = 0.0;
    fac_is.Drive1CurrentAlarmSts      = 0;
    fac_is.Drive1CurrentItlkSts       = 0;
    fac_is.Driver1ErrorTop            = 0;
    fac_is.Driver1ErrorTopItlkSts     = 0;
    fac_is.Driver1ErrorBot            = 0;
    fac_is.Driver1ErrorBotItlkSts     = 0;
    fac_is.TempL.f                    = 0.0;
    fac_is.TempLAlarmSts              = 0;
    fac_is.TempLItlkSts               = 0;
    fac_is.TempHeatSink.f             = 0.0;
    fac_is.TempHeatSinkAlarmSts       = 0;
    fac_is.TempHeatSinkItlkSts        = 0;
    fac_is.BoardTemperature.f         = 0.0;
    fac_is.BoardTemperatureAlarmSts   = 0;
    fac_is.BoardTemperatureItlkSts    = 0;
    fac_is.RelativeHumidity.f         = 0.0;
    fac_is.RelativeHumidityAlarmSts   = 0;
    fac_is.RelativeHumidityItlkSts    = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////





