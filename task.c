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
 * @file task.c
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author joao.rosa
 * @date 6 de nov de 2017
 *
 */


/////////////////////////////////////////////////////////////////////////////////////////////

/**
 * TODO: Put here your includes
 */

#include <stdbool.h>
#include <stdint.h>
#include <adc_internal.h>
#include "leds.h"
#include "output.h"
#include "input.h"
#include "BoardTempHum.h"
#include "pt100.h"
#include "ntc_isolated_i2c.h"
#include "application.h"

/**
 * TODO: Put here your defines. Just what is local. If you don't
 * need to access it from other module, consider use a constant (const)
 */

/////////////////////////////////////////////////////////////////////////////////////////////

/**
 * TODO: Put here your constants and variables. Always use static for 
 * private members.
 */

unsigned int Second = 0;
unsigned char uSecond = 0;
unsigned int _8Hz = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

bool TempCh1Read             = 0;
bool TempCh2Read             = 0;
bool TempCh3Read             = 0;
bool TempCh4Read             = 0;
bool RhSample                = 0;
bool RhSampleRead            = 0;
bool BoardTempSample         = 0;
bool BoardTempSampleRead     = 0;
bool ErrorCheck              = 0;
bool LedUpdate               = 0;
bool InterlockAlarmCheckTask = 0;
bool DriverVoltReadTask      = 0;
bool Driver1CurrtReadTask    = 0;
bool Driver2CurrtReadTask    = 0;
bool SendCanData             = 0;
bool NtcSample               = 0;
bool NtcSampleRead           = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

void ErrorCheckHandle(void)
{
    if(Pt100ReadCh1Error()) Pt100Ch1Clear();
    if(Pt100ReadCh2Error()) Pt100Ch2Clear();
    if(Pt100ReadCh3Error()) Pt100Ch3Clear();
    if(Pt100ReadCh4Error()) Pt100Ch4Clear();

    //Check CanNotCommunicate
    //Indicate channel problem, need a PT100 initialization or a chip verification
    // Reset() -> Reset implemented by software


    //Check RTD Out Of Range
    //send a command to indication layer to signalize the user
    if(Pt100ReadCh1RtdSts()) Pt100Ch1Reset();
    if(Pt100ReadCh2RtdSts()) Pt100Ch2Reset();
    if(Pt100ReadCh3RtdSts()) Pt100Ch3Reset();
    if(Pt100ReadCh4RtdSts()) Pt100Ch4Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void task_100_us(void)
{

    if (uSecond >= 10 )
    {
        uSecond = 0;
    }
    else uSecond++;

    if(uSecond == 0)
    {
        if(VoltageCh1.Enable) VoltageCh1Sample();

        if(VoltageCh2.Enable) VoltageCh2Sample();
    }
    else if(uSecond == 1)
    {
        if(VoltageCh3.Enable) VoltageCh3Sample();
    }
    else if(uSecond == 2)
    {
        if(VoltageCh4.Enable) VoltageCh4Sample();
    }
    else if(uSecond == 3)
    {
        if(CurrentCh1.Enable) CurrentCh1Sample();
    }
    else if(uSecond == 4)
    {
        if(CurrentCh2.Enable) CurrentCh2Sample();
    }
    else if(uSecond == 5)
    {
        if(CurrentCh3.Enable) CurrentCh3Sample();
    }
    else if(uSecond == 6)
    {
        if(CurrentCh4.Enable) CurrentCh4Sample();
    }
    else if(uSecond == 7)
    {
        if(LvCurrentCh1.Enable) LvCurrentCh1Sample();
    }

    else if(uSecond == 8)
    {
        if(LvCurrentCh2.Enable) LvCurrentCh2Sample();
    }
    else if(uSecond == 9)
    {
        if(LvCurrentCh3.Enable) LvCurrentCh3Sample();
    }

}

/////////////////////////////////////////////////////////////////////////////////////////////

void task_1_ms(void)
{

    // timestamp for 8Hz tasks (no critical tasks)
    if(_8Hz >= 125)
    {
        _8Hz = 0;
        LedUpdate = 1;
    }
    else _8Hz++;

    // timestamp for 1 second tasks
    if(Second >= 1000)
    {
        Second = 0;
    }
    else Second++;

    // trigger for 1s period tasks (no critical tasks)
    switch(Second)
    {
    case 20:
       SendCanData = 1;
       break;
    case 100:
       TempCh1Read = 1;
       break;
    case 200:
       RhSample = 1;
       break;
    case 250:
       RhSampleRead = 1;
       break;
    case 350:
       TempCh2Read = 1;
       break;
    case 450:
       BoardTempSample = 1;
       break;
    case 500:
       BoardTempSampleRead = 1;
       break;
    case 600:
       TempCh3Read = 1;
       break;
    case 700:
       TempCh4Read = 1;
       break;
    case 740:
       DriverVoltReadTask = 1;
       break;
    case 780:
       Driver1CurrtReadTask = 1;
       break;
    case 820:
       Driver2CurrtReadTask = 1;
       break;
    case 850:
       NtcSample = 1;
       break;
    case 900:
       NtcSampleRead = 1;
       break;
    case 950:
       ErrorCheck = 1;
       break;
    case 990:
       InterlockAlarmCheckTask = 1;
       break;
    case 1000:

       break;
    default:

       break;

    }

}

/////////////////////////////////////////////////////////////////////////////////////////////

void BoardTask(void)
{

  RunToggle();

  if(TempCh1Read)
  {
      if(Pt100Ch1.Enable) Pt100Ch1Sample();

      TempCh1Read = 0;
  }
  else if(TempCh2Read)
  {
      if(Pt100Ch2.Enable) Pt100Ch2Sample();

      TempCh2Read = 0;
  }
  else if(TempCh3Read)
  {
      if(Pt100Ch3.Enable) Pt100Ch3Sample();

      TempCh3Read = 0;
  }
  else if(TempCh4Read)
  {
      if(Pt100Ch4.Enable) Pt100Ch4Sample();

      TempCh4Read = 0;
  }
  else if(RhSample)
  {
      if(RelativeHumidity.Enable) RelativeHumidityStartConversion();

      RhSample = 0;
  }
  else if(RhSampleRead)
  {
      if(RelativeHumidity.Enable) RelativeHumidityRead();

      RhSampleRead = 0;
  }
  else if(BoardTempSample)
  {
      if(TemperatureBoard.Enable) BoardTemperatureStartConversion();

      BoardTempSample = 0;
  }
  else if(BoardTempSampleRead)
  {
      if(TemperatureBoard.Enable) BoardTemperatureRead();

      BoardTempSampleRead = 0;
  }
  else if(NtcSample)
  {
      if(TempNtcIgbt1.Enable || TempNtcIgbt2.Enable) NtcStartConversion();

      NtcSample = 0;
  }
  else if(NtcSampleRead)
  {
      if(TempNtcIgbt1.Enable || TempNtcIgbt2.Enable) NtcRead();

      NtcSampleRead = 0;
  }
  else if(DriverVoltReadTask)
  {
      if(DriverVolt.Enable) DriverVoltageSample();

      DriverVoltReadTask = 0;
  }

  else if(Driver1CurrtReadTask)
  {
      if(Driver1Curr.Enable) Driver1CurrentSample();

      Driver1CurrtReadTask = 0;
  }

  else if(Driver2CurrtReadTask)
  {
      if(Driver2Curr.Enable) Driver2CurrentSample();

      Driver2CurrtReadTask = 0;
  }

  else if(ErrorCheck)
  {
      ErrorCheckHandle();

      ErrorCheck = 0;
  }

  else if(InterlockAlarmCheckTask)
  {
      InterlockAppCheck();

      AlarmAppCheck();

      InterlockAlarmCheckTask = 0;
  }

  else if(LedUpdate)
  {
      LedIndicationStatus();

      LedUpdate = 0;
  }

  else if (SendCanData)
  {
      send_data_schedule();

      SendCanData = 0;
  }

  power_on_check();

}

/////////////////////////////////////////////////////////////////////////////////////////////


