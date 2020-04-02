


#ifndef __PT100_H__
#define __PT100_H__

void Pt100Init(void);

void Pt100Ch1Sample(void);
void Pt100Ch2Sample(void);
void Pt100Ch3Sample(void);
void Pt100Ch4Sample(void);

unsigned char Pt100ReadCh1(void);
unsigned char Pt100ReadCh2(void);
unsigned char Pt100ReadCh3(void);
unsigned char Pt100ReadCh4(void);

void Pt100Ch1Enable(void);
void Pt100Ch2Enable(void);
void Pt100Ch3Enable(void);
void Pt100Ch4Enable(void);

void Pt100Ch1Disable(void);
void Pt100Ch2Disable(void);
void Pt100Ch3Disable(void);
void Pt100Ch4Disable(void);

unsigned char Pt100ReadCh1Error(void);
unsigned char Pt100ReadCh2Error(void);
unsigned char Pt100ReadCh3Error(void);
unsigned char Pt100ReadCh4Error(void);


void Pt100SetCh1AlarmLevel(unsigned char alarm);
unsigned char Pt100ReadCh1AlarmLevel(void);
void Pt100SetCh1TripLevel(unsigned char trip);
unsigned char Pt100ReadCh1TripLevel(void);

void Pt100SetCh2AlarmLevel(unsigned char alarm);
unsigned char Pt100ReadCh2AlarmLevel(void);
void Pt100SetCh2TripLevel(unsigned char trip);
unsigned char Pt100ReadCh2TripLevel(void);

void Pt100SetCh3AlarmLevel(unsigned char alarm);
unsigned char Pt100ReadCh3AlarmLevel(void);

void Pt100SetCh3TripLevel(unsigned char trip);
unsigned char Pt100ReadCh3TripLevel(void);

void Pt100SetCh4AlarmLevel(unsigned char alarm);
unsigned char Pt100ReadCh4AlarmLevel(void);

void Pt100SetCh4TripLevel(unsigned char trip);
unsigned char Pt100ReadCh4TripLevel(void);

unsigned char Pt100ReadCh1AlarmSts(void);
unsigned char Pt100ReadCh1TripSts(void);

unsigned char Pt100ReadCh2AlarmSts(void);
unsigned char Pt100ReadCh2TripSts(void);

unsigned char Pt100ReadCh3AlarmSts(void);
unsigned char Pt100ReadCh3TripSts(void);

unsigned char Pt100ReadCh4AlarmSts(void);
unsigned char Pt100ReadCh4TripSts(void);

void Pt100SetCh1Delay(unsigned char Delay_Set);
void Pt100SetCh2Delay(unsigned char Delay_Set);
void Pt100SetCh3Delay(unsigned char Delay_Set);
void Pt100SetCh4Delay(unsigned char Delay_Set);

unsigned char Pt100ReadCh1CNC(void);
unsigned char Pt100ReadCh2CNC(void);
unsigned char Pt100ReadCh3CNC(void);
unsigned char Pt100ReadCh4CNC(void);

unsigned char Pt100ReadCh1RtdSts(void);
unsigned char Pt100ReadCh2RtdSts(void);
unsigned char Pt100ReadCh3RtdSts(void);
unsigned char Pt100ReadCh4RtdSts(void);

void Pt100Ch1Clear(void);
void Pt100Ch2Clear(void);
void Pt100Ch3Clear(void);
void Pt100Ch4Clear(void);

void Pt100Ch1Reset(void);
void Pt100Ch2Reset(void);
void Pt100Ch3Reset(void);
void Pt100Ch4Reset(void);

void Pt100ClearAlarmTrip(void);

#endif
