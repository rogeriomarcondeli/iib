
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __BOARDTEMPHUM_H__
#define __BOARDTEMPHUM_H__

/////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    unsigned char Enable;
    unsigned char Value;
    unsigned char AlarmLimit;
    unsigned char TripLimit;
    unsigned char Alarm;
    unsigned char Trip;
    unsigned int  Alarm_Delay_ms; // milisecond
    unsigned int  Alarm_DelayCount;
    unsigned int  Itlk_Delay_ms; // milisecond
    unsigned int  Itlk_DelayCount;
}rh_tempboard_t;

/////////////////////////////////////////////////////////////////////////////////////////////

extern rh_tempboard_t TemperatureBoard;
extern rh_tempboard_t RelativeHumidity;

/////////////////////////////////////////////////////////////////////////////////////////////
void RhBoardTempSenseInit(void);
unsigned char RhRead(void);
unsigned char BoardTempRead(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void BoardTemperatureStartConversion(void);
extern void BoardTemperatureRead(void);
extern void RelativeHumidityStartConversion(void);
extern void RelativeHumidityRead(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void BoardTempAlarmLevelSet(unsigned char nValue);
extern void BoardTempTripLevelSet(unsigned char nValue);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void BoardTempEnable(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void BoardTempDisable(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void BoardTempDelay(unsigned int Delay_Set);

/////////////////////////////////////////////////////////////////////////////////////////////

extern unsigned char BoardTempAlarmStatusRead(void);
extern unsigned char BoardTempTripStatusRead(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void RhAlarmLevelSet(unsigned char nValue);
extern void RhTripLevelSet(unsigned char nValue);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void RhEnable(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void RhDisable(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void RhDelay(unsigned int Delay_Set);

/////////////////////////////////////////////////////////////////////////////////////////////

extern unsigned char RhAlarmStatusRead(void);
extern unsigned char RhTripStatusRead(void);

/////////////////////////////////////////////////////////////////////////////////////////////

extern void RhBoardTempClearAlarmTrip(void);

/////////////////////////////////////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////////////////////////////////////


