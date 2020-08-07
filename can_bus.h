
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CAN_BUS_H__
#define CAN_BUS_H__

/////////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_can.h"
#include "driverlib/can.h"

#define MESSAGE_DATA_IIB_LEN          8
#define MESSAGE_DATA_IIB_OBJ_ID       1

#define MESSAGE_ITLK_IIB_LEN          8
#define MESSAGE_ITLK_IIB_OBJ_ID       2

#define MESSAGE_ALARM_IIB_LEN         8
#define MESSAGE_ALARM_IIB_OBJ_ID      3

#define MESSAGE_PARAM_IIB_LEN         8
#define MESSAGE_PARAM_IIB_OBJ_ID      4

/////////////////////////////////////////////////////////////////////////////////////////////

#define MESSAGE_RESET_UDC_LEN         1
#define MESSAGE_RESET_UDC_OBJ_ID      5

#define MESSAGE_PARAM_UDC_LEN         8
#define MESSAGE_PARAM_UDC_OBJ_ID      6

/////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    MESSAGE_DATA_IIB_ID = 1,
    MESSAGE_ITLK_IIB_ID,
    MESSAGE_ALARM_IIB_ID,
    MESSAGE_PARAM_IIB_ID,
    MESSAGE_RESET_UDC_ID,
    MESSAGE_PARAM_UDC_ID
}can_message_id_t;

/////////////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    ID_MODULE_1 = 0x00b,
    ID_MODULE_2 = 0x01b,
    ID_MODULE_3 = 0x02b,
    ID_MODULE_4 = 0x03b,
}id_module_t;

/////////////////////////////////////////////////////////////////////////////////////////////

extern void can_isr(void);
extern void InitCan(uint32_t ui32SysClock);
extern void handle_reset_message(void);
extern void send_data_message(uint8_t var);
extern uint16_t get_can_address(void);
extern void send_itlk_message(uint8_t var);
extern void send_alarm_message(uint8_t var);
extern void InitConsole(void);
extern void PrintCANMessageInfo(tCANMsgObject *psCANMsg, uint32_t ui32MsgObj);

/////////////////////////////////////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////////////////////////////////////





