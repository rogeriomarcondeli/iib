


#ifndef __BOARDTEMPHUM_H__
#define __BOARDTEMPHUM_H__

void RhTempSenseInit(void);
unsigned char RhRead(void);
unsigned char BoardTempRead(void);

extern void BoardTemperatureStartConversion(void);
extern void BoardTemperatureRead(void);
extern void RelativeHumidityStartConversion(void);
extern void RelativeHumidityRead(void);


extern void TempBoardAlarmLimitSet(unsigned char TempLimit);
extern void TempBoardTripLimitSet(unsigned char TempLimit);

extern unsigned char TempAlarmStatusRead(void);
extern unsigned char TempTripStatusRead(void);

extern void RhAlarmLimitSet(unsigned char RhLimit);
extern void RhTripLimitSet(unsigned char RhLimit);

extern unsigned char RhAlarmStatusRead(void);
extern unsigned char RhTripStatusRead(void);

extern void RhTempClearAlarmTrip(void);

#endif
