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
 * @file fac_cmd.c
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 20 de out de 2018
 *
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#include <iib_modules/fac_cmd.h>
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

#define FAC_CMD_CAPBANK_OVERVOLTAGE_ALM_LIM                 270.0
#define FAC_CMD_CAPBANK_OVERVOLTAGE_ITLK_LIM                290.0

#define FAC_CMD_OUTPUT_OVERVOLTAGE_ALM_LIM                  220.0
#define FAC_CMD_OUTPUT_OVERVOLTAGE_ITLK_LIM                 240.0

#define FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ALM_LIM      16.0
#define FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ITLK_LIM     17.0

#define FAC_CMD_AUX_SUPPLY_OVERCURRENT_ALM_LIM              2.0
#define FAC_CMD_AUX_SUPPLY_OVERCURRENT_ITLK_LIM             2.4

#define FAC_CMD_IDB_SUPPLY_OVERCURRENT_ALM_LIM              2.0
#define FAC_CMD_IDB_SUPPLY_OVERCURRENT_ITLK_LIM             2.4

#define FAC_CMD_GROUND_LEAKAGE_ALM_LIM                      45.0
#define FAC_CMD_GROUND_LEAKAGE_ITLK_LIM                     50.0

#define FAC_CMD_INDUC_OVERTEMP_ALM_LIM                      55
#define FAC_CMD_INDUC_OVERTEMP_ITLK_LIM                     60

#define FAC_CMD_HS_OVERTEMP_ALM_LIM                         55
#define FAC_CMD_HS_OVERTEMP_ITLK_LIM                        60

#define FAC_CMD_RH_OVERHUMIDITY_ALM_LIM                     50
#define FAC_CMD_RH_OVERHUMIDITY_ITLK_LIM                    90

#define FAC_CMD_BOARD_OVERTEMP_ALM_LIM                      50
#define FAC_CMD_BOARD_OVERTEMP_ITLK_LIM                     60

/////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    union {
        float       f;
        uint8_t     u8[4];
    } VcapBank;

    bool VcapBankAlarmSts;
    bool VcapBankItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } Vout;

    bool VoutAlarmSts;
    bool VoutItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } AuxIdbVoltage;

    bool AuxIdbVoltageAlarmSts;
    bool AuxIdbVoltageItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } AuxCurrent;

    bool AuxCurrentAlarmSts;
    bool AuxCurrentItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } IdbCurrent;

    bool IdbCurrentAlarmSts;
    bool IdbCurrentItlkSts;

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

    bool MainOverCurrentItlk;
    bool MainOverCurrentItlkSts;

    bool EmergencyButtonItlk;
    bool EmergencyButtonItlkSts;

    bool MainUnderVoltageItlk;
    bool MainUnderVoltageItlkSts;

    bool MainOverVoltageItlk;
    bool MainOverVoltageItlkSts;

    union {
        float       f;
        uint8_t     u8[4];
    } GroundLeakage;

    bool GroundLeakageAlarmSts;
    bool GroundLeakageItlkSts;

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

} fac_cmd_t;

/////////////////////////////////////////////////////////////////////////////////////////////

fac_cmd_t fac_cmd;

static uint32_t fac_cmd_interlocks_indication;
static uint32_t fac_cmd_alarms_indication;

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

void init_fac_cmd()
{
    config_module();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fac_cmd_interlocks()
{
    fac_cmd.VcapBankItlkSts         = 0;
    fac_cmd.VoutItlkSts             = 0;
    fac_cmd.AuxIdbVoltageItlkSts    = 0;
    fac_cmd.AuxCurrentItlkSts       = 0;
    fac_cmd.IdbCurrentItlkSts       = 0;
    fac_cmd.MainOverCurrentItlkSts  = 0;
    fac_cmd.EmergencyButtonItlkSts  = 0;
    fac_cmd.MainUnderVoltageItlkSts = 0;
    fac_cmd.MainOverVoltageItlkSts  = 0;
    fac_cmd.GroundLeakageItlkSts    = 0;
    fac_cmd.TempLItlkSts            = 0;
    fac_cmd.TempHeatSinkItlkSts     = 0;
    fac_cmd.BoardTemperatureItlkSts = 0;
    fac_cmd.RelativeHumidityItlkSts = 0;

    itlk_id = 0;

    send_itlk_message(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fac_cmd_interlocks()
{
    uint8_t test = 0;

    test |= fac_cmd.VcapBankItlkSts;
    test |= fac_cmd.VoutItlkSts;
    test |= fac_cmd.AuxIdbVoltageItlkSts;
    test |= fac_cmd.AuxCurrentItlkSts;
    test |= fac_cmd.IdbCurrentItlkSts;
    test |= fac_cmd.MainOverCurrentItlkSts;
    test |= fac_cmd.EmergencyButtonItlkSts;
    test |= fac_cmd.MainUnderVoltageItlkSts;
    test |= fac_cmd.MainOverVoltageItlkSts;
    test |= fac_cmd.GroundLeakageItlkSts;
    test |= fac_cmd.TempLItlkSts;
    test |= fac_cmd.TempHeatSinkItlkSts;
    test |= fac_cmd.BoardTemperatureItlkSts;
    test |= fac_cmd.RelativeHumidityItlkSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void clear_fac_cmd_alarms()
{
    fac_cmd.VcapBankAlarmSts            = 0;
    fac_cmd.VoutAlarmSts                = 0;
    fac_cmd.AuxIdbVoltageAlarmSts       = 0;
    fac_cmd.AuxCurrentAlarmSts          = 0;
    fac_cmd.IdbCurrentAlarmSts          = 0;
    fac_cmd.GroundLeakageAlarmSts       = 0;
    fac_cmd.TempLAlarmSts               = 0;
    fac_cmd.TempHeatSinkAlarmSts        = 0;
    fac_cmd.BoardTemperatureAlarmSts    = 0;
    fac_cmd.RelativeHumidityAlarmSts    = 0;

    alarm_id = 0;

    send_alarm_message(1);
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t check_fac_cmd_alarms()
{
    uint8_t test = 0;

    test |= fac_cmd.VcapBankAlarmSts;
    test |= fac_cmd.VoutAlarmSts;
    test |= fac_cmd.AuxIdbVoltageAlarmSts;
    test |= fac_cmd.AuxCurrentAlarmSts;
    test |= fac_cmd.IdbCurrentAlarmSts;
    test |= fac_cmd.GroundLeakageAlarmSts;
    test |= fac_cmd.TempLAlarmSts;
    test |= fac_cmd.TempHeatSinkAlarmSts;
    test |= fac_cmd.BoardTemperatureAlarmSts;
    test |= fac_cmd.RelativeHumidityAlarmSts;

    return test;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void check_fac_cmd_indication_leds()
{
    //Input over voltage
    if(fac_cmd.VcapBankItlkSts) Led2TurnOff();
    else if(fac_cmd.VcapBankAlarmSts) Led2Toggle();
    else Led2TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Output over voltage
    if(fac_cmd.VoutItlkSts) Led3TurnOff();
    else if(fac_cmd.VoutAlarmSts) Led3Toggle();
    else Led3TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks Aux and Idb voltage and current
    if(fac_cmd.AuxIdbVoltageItlkSts || fac_cmd.AuxCurrentItlkSts || fac_cmd.IdbCurrentItlkSts) Led4TurnOff();
    else if(fac_cmd.AuxIdbVoltageAlarmSts || fac_cmd.AuxCurrentAlarmSts || fac_cmd.IdbCurrentAlarmSts) Led4Toggle();
    else Led4TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks main over current and emergency button
    if(fac_cmd.MainOverCurrentItlkSts || fac_cmd.EmergencyButtonItlkSts) Led5TurnOff();
    else Led5TurnOn();

////////////////////////////////////////////////////////////////////////////////////////////

    //Interlocks main under voltage and main over voltage
    if(fac_cmd.MainUnderVoltageItlkSts || fac_cmd.MainOverVoltageItlkSts) Led6TurnOff();
    else Led6TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Fuga para o Terra
    if(fac_cmd.GroundLeakageItlkSts) Led7TurnOff();
    else if(fac_cmd.GroundLeakageAlarmSts) Led7Toggle();
    else Led7TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Heatsink Over temperature
    if(fac_cmd.TempHeatSinkItlkSts) Led8TurnOff();
    else if(fac_cmd.TempHeatSinkAlarmSts) Led8Toggle();
    else Led8TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    // Inductor Over temperature
    if(fac_cmd.TempLItlkSts) Led9TurnOff();
    else if(fac_cmd.TempLAlarmSts) Led9Toggle();
    else Led9TurnOn();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Temperatura PCB e Umidade Relativa
    if(fac_cmd.BoardTemperatureItlkSts || fac_cmd.RelativeHumidityItlkSts) Led10TurnOff();
    else if(fac_cmd.BoardTemperatureAlarmSts || fac_cmd.RelativeHumidityAlarmSts) Led10Toggle();
    else Led10TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fac_cmd_application_readings()
{
    //PT100 CH1 Indutor
    fac_cmd.TempL.f = (float) Pt100ReadCh1();
    fac_cmd.TempLAlarmSts = Pt100ReadCh1AlarmSts();
    if(!fac_cmd.TempLItlkSts)fac_cmd.TempLItlkSts = Pt100ReadCh1TripSts();

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 CH2 Dissipador
    fac_cmd.TempHeatSink.f = (float) Pt100ReadCh2();
    fac_cmd.TempHeatSinkAlarmSts = Pt100ReadCh2AlarmSts();
    if(!fac_cmd.TempHeatSinkItlkSts)fac_cmd.TempHeatSinkItlkSts = Pt100ReadCh2TripSts();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperatura PCB IIB
    fac_cmd.BoardTemperature.f = (float) BoardTempRead();
    fac_cmd.BoardTemperatureAlarmSts = BoardTempAlarmStatusRead();
    if(!fac_cmd.BoardTemperatureItlkSts)fac_cmd.BoardTemperatureItlkSts = BoardTempTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Umidade Relativa
    fac_cmd.RelativeHumidity.f = (float) RhRead();
    fac_cmd.RelativeHumidityAlarmSts = RhAlarmStatusRead();
    if(!fac_cmd.RelativeHumidityItlkSts)fac_cmd.RelativeHumidityItlkSts = RhTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Tensao de Saida
    fac_cmd.Vout.f = LvCurrentCh1Read();
    fac_cmd.VoutAlarmSts = LvCurrentCh1AlarmStatusRead();
    if(!fac_cmd.VoutItlkSts)fac_cmd.VoutItlkSts = LvCurrentCh1TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Tensao no Banco de Capacitores
    fac_cmd.VcapBank.f = LvCurrentCh2Read();
    fac_cmd.VcapBankAlarmSts = LvCurrentCh2AlarmStatusRead();
    if(!fac_cmd.VcapBankItlkSts)fac_cmd.VcapBankItlkSts = LvCurrentCh2TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Medida de Fuga para o Terra
    fac_cmd.GroundLeakage.f = LvCurrentCh3Read();
    fac_cmd.GroundLeakageAlarmSts = LvCurrentCh3AlarmStatusRead();
    if(!fac_cmd.GroundLeakageItlkSts)fac_cmd.GroundLeakageItlkSts = LvCurrentCh3TripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Aux and Idb Voltage
    fac_cmd.AuxIdbVoltage.f = DriverVoltageRead();
    fac_cmd.AuxIdbVoltageAlarmSts = DriverVoltageAlarmStatusRead();
    if(!fac_cmd.AuxIdbVoltageItlkSts)fac_cmd.AuxIdbVoltageItlkSts = DriverVolatgeTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Aux Current
    fac_cmd.AuxCurrent.f = Driver1CurrentRead();
    fac_cmd.AuxCurrentAlarmSts = Driver1CurrentAlarmStatusRead();
    if(!fac_cmd.AuxCurrentItlkSts)fac_cmd.AuxCurrentItlkSts = Driver1CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Idb Current
    fac_cmd.IdbCurrent.f = Driver2CurrentRead();
    fac_cmd.IdbCurrentAlarmSts = Driver2CurrentAlarmStatusRead();
    if(!fac_cmd.IdbCurrentItlkSts)fac_cmd.IdbCurrentItlkSts = Driver2CurrentTripStatusRead();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Main Over Current
    fac_cmd.MainOverCurrentItlk = Gpdi5Read();//Variavel usada para debug
    if(!fac_cmd.MainOverCurrentItlkSts)fac_cmd.MainOverCurrentItlkSts = Gpdi5Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Emergency Button
    fac_cmd.EmergencyButtonItlk = Gpdi6Read();//Variavel usada para debug
    if(!fac_cmd.EmergencyButtonItlkSts)fac_cmd.EmergencyButtonItlkSts = Gpdi6Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Main Under Voltage
    fac_cmd.MainUnderVoltageItlk = Gpdi7Read();//Variavel usada para debug
    if(!fac_cmd.MainUnderVoltageItlkSts)fac_cmd.MainUnderVoltageItlkSts = Gpdi7Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    //Interlock Main Over Voltage
    fac_cmd.MainOverVoltageItlk = Gpdi8Read();//Variavel usada para debug
    if(!fac_cmd.MainOverVoltageItlkSts)fac_cmd.MainOverVoltageItlkSts = Gpdi8Read();

/////////////////////////////////////////////////////////////////////////////////////////////

    map_vars();
    get_itlks_id();
    get_alarms_id();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void fac_cmd_power_on_check()
{
    Led1TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void map_vars()
{
    fac_cmd_interlocks_indication = itlk_id;
    fac_cmd_alarms_indication = alarm_id;

    g_controller_iib.iib_itlk[0].u32        = fac_cmd_interlocks_indication;
    g_controller_iib.iib_itlk[1].u32        = ResetInterlocksRegister;

    g_controller_iib.iib_alarm[0].u32       = fac_cmd_alarms_indication;
    g_controller_iib.iib_alarm[1].u32       = ResetAlarmsRegister;

    g_controller_iib.iib_signals[0].f       = fac_cmd.VcapBank.f;
    g_controller_iib.iib_signals[1].f       = fac_cmd.Vout.f;
    g_controller_iib.iib_signals[2].f       = fac_cmd.AuxIdbVoltage.f;
    g_controller_iib.iib_signals[3].f       = fac_cmd.AuxCurrent.f;
    g_controller_iib.iib_signals[4].f       = fac_cmd.IdbCurrent.f;
    g_controller_iib.iib_signals[5].f       = fac_cmd.GroundLeakage.f;
    g_controller_iib.iib_signals[6].f       = fac_cmd.TempL.f;
    g_controller_iib.iib_signals[7].f       = fac_cmd.TempHeatSink.f;
    g_controller_iib.iib_signals[8].f       = fac_cmd.BoardTemperature.f;
    g_controller_iib.iib_signals[9].f       = fac_cmd.RelativeHumidity.f;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_fac_cmd_data()
{
    static uint8_t i = 0;

    send_data_message(i);

    i++;

    if (i > 9) i = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void get_itlks_id()
{
    if (fac_cmd.VcapBankItlkSts)             itlk_id |= FAC_CMD_CAPBANK_OVERVOLTAGE_ITLK;
    if (fac_cmd.VoutItlkSts)                 itlk_id |= FAC_CMD_OUTPUT_OVERVOLTAGE_ITLK;
    if (fac_cmd.AuxIdbVoltageItlkSts)        itlk_id |= FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ITLK;
    if (fac_cmd.AuxCurrentItlkSts)           itlk_id |= FAC_CMD_AUX_SUPPLY_OVERCURRENT_ITLK;
    if (fac_cmd.IdbCurrentItlkSts)           itlk_id |= FAC_CMD_IDB_SUPPLY_OVERCURRENT_ITLK;
    if (fac_cmd.TempLItlkSts)                itlk_id |= FAC_CMD_INDUC_OVERTEMP_ITLK;
    if (fac_cmd.TempHeatSinkItlkSts)         itlk_id |= FAC_CMD_HS_OVERTEMP_ITLK;
    if (fac_cmd.MainOverCurrentItlkSts)      itlk_id |= FAC_CMD_MAIN_OVER_CURRENT_ITLK;
    if (fac_cmd.EmergencyButtonItlkSts)      itlk_id |= FAC_CMD_EMERGENCY_BUTTON_ITLK;
    if (fac_cmd.MainUnderVoltageItlkSts)     itlk_id |= FAC_CMD_MAIN_UNDER_VOLTAGE_ITLK;
    if (fac_cmd.MainOverVoltageItlkSts)      itlk_id |= FAC_CMD_MAIN_OVER_VOLTAGE_ITLK;
    if (fac_cmd.GroundLeakageItlkSts)        itlk_id |= FAC_CMD_GROUND_LKG_ITLK;
    if (fac_cmd.BoardTemperatureItlkSts)     itlk_id |= FAC_CMD_BOARD_IIB_OVERTEMP_ITLK;
    if (fac_cmd.RelativeHumidityItlkSts)     itlk_id |= FAC_CMD_BOARD_IIB_OVERHUMIDITY_ITLK;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_fac_cmd_itlk_msg()
{
    send_itlk_message(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void get_alarms_id()
{
    if (fac_cmd.VcapBankAlarmSts)            alarm_id |= FAC_CMD_CAPBANK_OVERVOLTAGE_ALM;
    if (fac_cmd.VoutItlkSts)                 alarm_id |= FAC_CMD_OUTPUT_OVERVOLTAGE_ALM;
    if (fac_cmd.AuxIdbVoltageAlarmSts)       alarm_id |= FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ALM;
    if (fac_cmd.AuxCurrentAlarmSts)          alarm_id |= FAC_CMD_AUX_SUPPLY_OVERCURRENT_ALM;
    if (fac_cmd.IdbCurrentAlarmSts)          alarm_id |= FAC_CMD_IDB_SUPPLY_OVERCURRENT_ALM;
    if (fac_cmd.TempLAlarmSts)               alarm_id |= FAC_CMD_INDUC_OVERTEMP_ALM;
    if (fac_cmd.TempHeatSinkAlarmSts)        alarm_id |= FAC_CMD_HS_OVERTEMP_ALM;
    if (fac_cmd.GroundLeakageAlarmSts)       alarm_id |= FAC_CMD_GROUND_LKG_ALM;
    if (fac_cmd.BoardTemperatureAlarmSts)    alarm_id |= FAC_CMD_BOARD_IIB_OVERTEMP_ALM;
    if (fac_cmd.RelativeHumidityAlarmSts)    alarm_id |= FAC_CMD_BOARD_IIB_OVERHUMIDITY_ALM;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static void config_module()
{
    /* Isolated Voltage */
    LvCurrentCh1Init(300.0, 0.025, 120.0, 3); /* Output Voltage */
    LvCurrentCh2Init(600.0, 0.025, 120.0, 3); /* Voltage Capacitor Bank */
    LvCurrentCh3Init(50.0, 0.025, 120.0, 3);  /* GND Leakage */

    LvCurrentCh1Enable();  //LvCurrentCh1 enable
    LvCurrentCh2Enable();  //LvCurrentCh2 enable
    LvCurrentCh3Enable();  //LvCurrentCh3 enable

    /* Protection Limits */
    LvCurrentCh1AlarmLevelSet(FAC_CMD_OUTPUT_OVERVOLTAGE_ALM_LIM);
    LvCurrentCh1TripLevelSet(FAC_CMD_OUTPUT_OVERVOLTAGE_ITLK_LIM);
    LvCurrentCh2AlarmLevelSet(FAC_CMD_CAPBANK_OVERVOLTAGE_ALM_LIM);
    LvCurrentCh2TripLevelSet(FAC_CMD_CAPBANK_OVERVOLTAGE_ITLK_LIM);
    LvCurrentCh3AlarmLevelSet(FAC_CMD_GROUND_LEAKAGE_ALM_LIM);
    LvCurrentCh3TripLevelSet(FAC_CMD_GROUND_LEAKAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //PT100 configuration
    //Delay 4 seconds
    Pt100SetCh1Delay(4);
    Pt100SetCh2Delay(4);

    /* Pt-100 channel enable */
    Pt100Ch1Enable();
    Pt100Ch2Enable();
    Pt100Ch3Disable();
    Pt100Ch4Disable();

    /* Pt-100 Configuration Limits */
    Pt100SetCh1AlarmLevel(FAC_CMD_INDUC_OVERTEMP_ALM_LIM);
    Pt100SetCh1TripLevel(FAC_CMD_INDUC_OVERTEMP_ITLK_LIM);
    Pt100SetCh2AlarmLevel(FAC_CMD_HS_OVERTEMP_ALM_LIM);
    Pt100SetCh2TripLevel(FAC_CMD_HS_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature igbt1 and igbt2 configuration
    TempIgbt1Disable(); //TempIgbt1 disable
    TempIgbt2Disable(); //TempIgbt2 disable

/////////////////////////////////////////////////////////////////////////////////////////////

    //Temperature Board configuration
    BoardTempDelay(3); //Inserir valor de delay

    BoardTempEnable(); //BoardTemp enable

    //Temp board configuration limits
    BoardTempAlarmLevelSet(FAC_CMD_BOARD_OVERTEMP_ALM_LIM);
    BoardTempTripLevelSet(FAC_CMD_BOARD_OVERTEMP_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Humidity Board configuration
    RhDelay(3); //Inserir valor de delay

    RhEnable(); //Rh enable

    //Rh configuration limits
    RhAlarmLevelSet(FAC_CMD_RH_OVERHUMIDITY_ALM_LIM);
    RhTripLevelSet(FAC_CMD_RH_OVERHUMIDITY_ITLK_LIM);

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

    //Configuration Aux and Idb voltage
    DriverVoltageInit();

    DriverVoltageDelay(3); //Inserir valor de delay

    DriverVoltageEnable(); //Voltage Aux and Idb enable

    //Limite de alarme e interlock da tensao
    DriverVoltageAlarmLevelSet(FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ALM_LIM);
    DriverVoltageTripLevelSet(FAC_CMD_AUX_AND_IDB_SUPPLY_OVERVOLTAGE_ITLK_LIM);

/////////////////////////////////////////////////////////////////////////////////////////////

    //Configuration Aux and Idb current
    DriverCurrentInit();

    DriverCurrentDelay(3); //Inserir valor de delay

    Driver1CurrentEnable(); //Current Aux enable
    Driver2CurrentEnable(); //Current Idb enable

    //Limite de alarme e interlock Aux
    Driver1CurrentAlarmLevelSet(FAC_CMD_AUX_SUPPLY_OVERCURRENT_ALM_LIM);
    Driver1CurrentTripLevelSet(FAC_CMD_AUX_SUPPLY_OVERCURRENT_ITLK_LIM);

    //Limite de alarme e interlock Idb
    Driver2CurrentAlarmLevelSet(FAC_CMD_IDB_SUPPLY_OVERCURRENT_ALM_LIM);
    Driver2CurrentTripLevelSet(FAC_CMD_IDB_SUPPLY_OVERCURRENT_ITLK_LIM);

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
    Gpdi5Enable();   //Gpdi5 enable Main Over Current
    Gpdi6Enable();   //Gpdi6 enable Emergency Button
    Gpdi7Enable();   //Gpdi7 enable Main Under Voltage
    Gpdi8Enable();   //Gpdi8 enable Main Over Voltage
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
    fac_cmd.VcapBank.f               = 0.0;
    fac_cmd.VcapBankAlarmSts         = 0;
    fac_cmd.VcapBankItlkSts          = 0;
    fac_cmd.Vout.f                   = 0.0;
    fac_cmd.VoutAlarmSts             = 0;
    fac_cmd.VoutItlkSts              = 0;
    fac_cmd.AuxIdbVoltage.f          = 0.0;
    fac_cmd.AuxIdbVoltageAlarmSts    = 0;
    fac_cmd.AuxIdbVoltageItlkSts     = 0;
    fac_cmd.AuxCurrent.f             = 0.0;
    fac_cmd.AuxCurrentAlarmSts       = 0;
    fac_cmd.AuxCurrentItlkSts        = 0;
    fac_cmd.IdbCurrent.f             = 0.0;
    fac_cmd.IdbCurrentAlarmSts       = 0;
    fac_cmd.IdbCurrentItlkSts        = 0;
    fac_cmd.MainOverCurrentItlk      = 0;
    fac_cmd.MainOverCurrentItlkSts   = 0;
    fac_cmd.EmergencyButtonItlk      = 0;
    fac_cmd.EmergencyButtonItlkSts   = 0;
    fac_cmd.MainUnderVoltageItlk     = 0;
    fac_cmd.MainUnderVoltageItlkSts  = 0;
    fac_cmd.MainOverVoltageItlk      = 0;
    fac_cmd.MainOverVoltageItlkSts   = 0;
    fac_cmd.GroundLeakage.f          = 0.0;
    fac_cmd.GroundLeakageAlarmSts    = 0;
    fac_cmd.GroundLeakageItlkSts     = 0;
    fac_cmd.TempL.f                  = 0.0;
    fac_cmd.TempLAlarmSts            = 0;
    fac_cmd.TempLItlkSts             = 0;
    fac_cmd.TempHeatSink.f           = 0.0;
    fac_cmd.TempHeatSinkAlarmSts     = 0;
    fac_cmd.TempHeatSinkItlkSts      = 0;
    fac_cmd.BoardTemperature.f       = 0.0;
    fac_cmd.BoardTemperatureAlarmSts = 0;
    fac_cmd.BoardTemperatureItlkSts  = 0;
    fac_cmd.RelativeHumidity.f       = 0.0;
    fac_cmd.RelativeHumidityAlarmSts = 0;
    fac_cmd.RelativeHumidityItlkSts  = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////






