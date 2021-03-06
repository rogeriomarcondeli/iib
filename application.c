
/////////////////////////////////////////////////////////////////////////////////////////////

#include "application.h"
#include "adc_internal.h"
#include "BoardTempHum.h"
#include "ntc_isolated_i2c.h"
#include "pt100.h"
#include "output.h"
#include "leds.h"
#include "can_bus.h"
#include "input.h"
#include <stdbool.h>
#include <stdint.h>
#include "fac_cmd.h"
#include "fac_is.h"
#include "fac_os.h"
#include "fap.h"

/////////////////////////////////////////////////////////////////////////////////////////////

static unsigned char PowerModuleModel = 0;
static unsigned char Interlock = 0;
static unsigned char InterlockOld = 0;
static unsigned char ItlkClrCmd = 0;
static unsigned char InitApp = 0;
static unsigned char Alarm = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

void AppConfiguration(void)
{

    // Set Power Module Model
    // This parameter guide the firmware behavior
    // Each Model has a different variable list that need to be check

    PowerModuleModel = PowerModuleModelSelect;

    switch(PowerModuleModel)
    {
    case FAP:

        init_fap();

        break;

    case FAC_OS:

        init_fac_os();

        break;

    case FAC_IS:

        init_fac_is();

        break;

    case FAC_CMD:

        init_fac_cmd();

        break;

    default:
        break;

    }
    // End of configuration
    // Turn on Led1 (board started)
    Led1TurnOff();
    Led2TurnOn();
    Led3TurnOn();
    Led4TurnOn();
    Led5TurnOn();
    Led6TurnOn();
    Led7TurnOn();
    Led8TurnOn();
    Led9TurnOn();
    Led10TurnOn();
}

/////////////////////////////////////////////////////////////////////////////////////////////

// Set Interlock clear command
void InterlockClear(void)
{
    ItlkClrCmd = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void InterlockSet(void)
{
    Interlock = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void InterlockClearCheck(void)
{
    if(ItlkClrCmd)
    {
        Interlock = 0;
        InterlockOld = 0;

        InitApp = 0;

        AdcClearAlarmTrip();
        Pt100ClearAlarmTrip();
        RhBoardTempClearAlarmTrip();
        TempIgbt1TempIgbt2ClearAlarmTrip();

        ItlkClrCmd = 0;

        switch(PowerModuleModel)
        {
        case FAP:

            clear_fap_interlocks();
            clear_fap_alarms();

            break;

        case FAC_OS:

            clear_fac_os_interlocks();
            clear_fac_os_alarms();

            break;

        case FAC_IS:

            clear_fac_is_interlocks();
            clear_fac_is_alarms();

            break;

        case FAC_CMD:

            clear_fac_cmd_interlocks();
            clear_fac_cmd_alarms();

            break;

        default:
            break;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char InterlockRead(void)
{
    return Interlock;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void AppInterlock(void)
{

    // caso haja algum Interlock, o rele auxiliar deve ser desligado e as opera��es cabiveis de Interlock devem ser executadas
    // Analisar se todos os interlocks foram apagados para poder liberar o rele auxiliar
    // caso n�o haja mais Interlock, fechar o rele auxiliar

    switch(PowerModuleModel)
    {
    case FAP:

        ReleAuxTurnOff();
        ReleExtItlkTurnOff();

        break;

    case FAC_OS:

        ReleAuxTurnOff();
        ReleExtItlkTurnOff();

        Gpdo1TurnOff();
        Gpdo2TurnOff();

        break;

    case FAC_IS:

        ReleAuxTurnOff();
        ReleExtItlkTurnOff();

        break;

    case FAC_CMD:

        ReleAuxTurnOff();
        ReleExtItlkTurnOff();

        break;

    default:
        break;

    }

}

/////////////////////////////////////////////////////////////////////////////////////////////

void AlarmSet(void)
{
    Alarm = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void AlarmClear(void)
{
    Alarm = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char AlarmRead(void)
{
    return Alarm;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void InterlockAppCheck(void)
{
    unsigned char test = 0;

    switch(PowerModuleModel)
    {
    case FAP:

        test = check_fap_interlocks();

        break;

    case FAC_OS:

        test = check_fac_os_interlocks();

        break;

    case FAC_IS:

        test = check_fac_is_interlocks();

        break;

    case FAC_CMD:

        test = check_fac_cmd_interlocks();

        break;

    default:
        break;
    }

    if(test)
    {
        InterlockSet();

        switch (PowerModuleModel)
        {
        case FAP:

            send_fap_itlk_msg();

            break;

        case FAC_OS:

            send_output_fac_os_itlk_msg();

            break;

        case FAC_IS:

            send_input_fac_is_itlk_msg();

            break;

        case FAC_CMD:

            send_fac_cmd_itlk_msg();

            break;

        default:
            break;
        }

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

void AlarmAppCheck(void)
{
    unsigned char test = 0;

    switch(PowerModuleModel)
    {
    case FAP:

        test = check_fap_alarms();

        break;

    case FAC_OS:

        test = check_fac_os_alarms();

        break;

    case FAC_IS:

        test = check_fac_is_alarms();

        break;

    case FAC_CMD:

        test = check_fac_cmd_alarms();

        break;

    default:
        break;
    }

    if(test)
    {
        AlarmSet();

        send_alarm_message(0);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

void LedIndicationStatus(void)
{
    switch(PowerModuleModel)
    {
    case FAP:

        check_fap_indication_leds();

        break;

    case FAC_OS:

        check_fac_os_indication_leds();

        break;

    case FAC_IS:

        check_fac_is_indication_leds();

        break;

    case FAC_CMD:

        check_fac_cmd_indication_leds();

        break;

    default:
        break;
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////

void Application(void)
{

    switch(PowerModuleModel)
    {
    case FAP:

        fap_application_readings();

        break;

    case FAC_OS:

        fac_os_application_readings();

        break;

    case FAC_IS:

        fac_is_application_readings();

        break;

    case FAC_CMD:

        fac_cmd_application_readings();

        break;

    default:
        break;
    }

    // Interlock Test
    if(Interlock == 1 && InterlockOld == 0)
    {
        InterlockOld = 1;
        AppInterlock();
    }

    // Actions that needs to be taken during the Application initialization
    if(InitApp == 0 && Interlock == 0)
    {
        InitApp = 1;

        switch(PowerModuleModel)
        {
        case FAP:

            ReleAuxTurnOn();
            ReleExtItlkTurnOff();

            break;

        case FAC_OS:

            ReleAuxTurnOn();
            ReleExtItlkTurnOn();

            Gpdo1TurnOn();
            Gpdo2TurnOn();

            break;

        case FAC_IS:

            ReleAuxTurnOn();
            ReleExtItlkTurnOn();

            break;

        case FAC_CMD:

            ReleAuxTurnOn();
            ReleExtItlkTurnOn();

            break;

        default:
            break;
        }
    }

    InterlockClearCheck();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_data_schedule()
{
    switch(AppType())
    {
    case FAP:

        send_fap_data();

        break;

    case FAC_OS:

        send_fac_os_data();

        break;

    case FAC_IS:

        send_fac_is_data();

        break;

    case FAC_CMD:

        send_fac_cmd_data();

        break;

    default:
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

void power_on_check()
{
    switch(AppType())
    {
    case FAP:

        fap_power_on_check();

        break;

    case FAC_OS:

        fac_os_power_on_check();

        break;

    case FAC_IS:

        fac_is_power_on_check();

        break;

    case FAC_CMD:

        fac_cmd_power_on_check();

        break;

    default:
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

unsigned char AppType(void)
{
    return PowerModuleModel;
}

/////////////////////////////////////////////////////////////////////////////////////////////





