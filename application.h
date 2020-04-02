#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#define  FAP                        0
#define  FAC_OS                     1
#define  RECTIFIER_MODULE           2
#define  FAC_IS                     3
#define  FAC_CMD_MODULE             4
#define  FAP_300A                   5

void LedIndicationStatus(void);
void AppConfiguration(void);

void InterlockClear(void);
void InterlockSet(void);
void InterlockClearCheck(void);
unsigned char InterlockRead(void);
void AppInterlock(void);

void AlarmSet(void);
void AlarmClear(void);
unsigned char AlarmRead(void);
void AppAlarm(void);

void InterlockAppCheck(void);
void AlarmAppCheck(void);

void Application(void);
unsigned char AppType(void);

extern void power_on_check();
extern void send_data_schedule();


#endif
