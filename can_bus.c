
/////////////////////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_can.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/can.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "can_bus.h"
#include "iib_data.h"
#include "BoardTempHum.h"
#include "input.h"
#include "application.h"
#include "adc_internal.h"
#include "leds.h"
#include "board_drivers/hardware_def.h"
#include "peripheral_drivers/gpio/gpio_driver.h"

/////////////////////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
// A flag to indicate that some transmission error occurred.
//
//*****************************************************************************
volatile bool g_bErrFlag = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

/******************************************************************************
 *                          Object Messages
 *****************************************************************************/

tCANMsgObject tx_message_data_iib;

tCANMsgObject tx_message_itlk_iib;

tCANMsgObject tx_message_alarm_iib;

tCANMsgObject tx_message_param_iib;

/////////////////////////////////////////////////////////////////////////////////////////////

tCANMsgObject rx_message_reset_udc;

tCANMsgObject rx_message_param_udc;

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t message_data_iib[MESSAGE_DATA_IIB_LEN];

uint8_t message_itlk_iib[MESSAGE_ITLK_IIB_LEN];

uint8_t message_alarm_iib[MESSAGE_ALARM_IIB_LEN];

uint8_t message_param_iib[MESSAGE_PARAM_IIB_LEN];

/////////////////////////////////////////////////////////////////////////////////////////////

uint8_t message_reset_udc[MESSAGE_RESET_UDC_LEN];

uint8_t message_param_udc[MESSAGE_PARAM_UDC_LEN];

/////////////////////////////////////////////////////////////////////////////////////////////

volatile uint8_t can_address    = 0;

/////////////////////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
//
// This function is the interrupt handler for the CAN peripheral.  It checks
// for the cause of the interrupt, and maintains a count of all messages that
// have been transmitted.
//
//*****************************************************************************
void can_isr(void)
{
    uint32_t ui32Status;

    // Read the CAN interrupt status to find the cause of the interrupt
    ui32Status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

    // If the cause is a controller status interrupt, then get the status
    if(ui32Status == CAN_INT_INTID_STATUS)
    {
        // Read the controller status.  This will return a field of status
        // error bits that can indicate various errors.  Error processing
        // is not done in this example for simplicity.  Refer to the
        // API documentation for details about the error status bits.
        // The act of reading this status will clear the interrupt.  If the
        // CAN peripheral is not connected to a CAN bus with other CAN devices
        // present, then errors will occur and will be indicated in the
        // controller status.
        ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);

        // Set a flag to indicate some errors may have occurred.
        g_bErrFlag = 1;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Check if the cause is message object 1, which what we are using for
    // sending messages.
    else if(ui32Status == MESSAGE_DATA_IIB_OBJ_ID)
    {
        // Getting to this point means that the TX interrupt occurred on
        // message object 1, and the message TX is complete.
        // Clear the message object interrupt.

        CANIntClear(CAN0_BASE, MESSAGE_DATA_IIB_OBJ_ID);

        /* Tx object 1. Nothing to do for now. */

        // Since the message was sent, clear any error flags.
        g_bErrFlag = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Check if the cause is message object 2, which what we are using for
    // sending messages.
    else if(ui32Status == MESSAGE_ITLK_IIB_OBJ_ID)
    {
        // Getting to this point means that the TX interrupt occurred on
        // message object 2, and the message TX is complete.
        // Clear the message object interrupt.

        CANIntClear(CAN0_BASE, MESSAGE_ITLK_IIB_OBJ_ID);

        /* Tx object 2. Nothing to do for now. */

        // Since the message was sent, clear any error flags.
        g_bErrFlag = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Check if the cause is message object 3, which what we are using for
    // sending messages.
    else if(ui32Status == MESSAGE_ALARM_IIB_OBJ_ID)
    {
        // Getting to this point means that the TX interrupt occurred on
        // message object 3, and the message TX is complete.
        // Clear the message object interrupt.

        CANIntClear(CAN0_BASE, MESSAGE_ALARM_IIB_OBJ_ID);

        /* Tx object 3. Nothing to do for now. */

        // Since the message was sent, clear any error flags.
        g_bErrFlag = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Check if the cause is message object 4, which what we are using for
    // sending messages.
    else if(ui32Status == MESSAGE_PARAM_IIB_OBJ_ID)
    {
        // Getting to this point means that the TX interrupt occurred on
        // message object 4, and the message TX is complete.
        // Clear the message object interrupt.

        CANIntClear(CAN0_BASE, MESSAGE_PARAM_IIB_OBJ_ID);

        /* Tx object 4. Nothing to do for now. */

        // Since the message was sent, clear any error flags.
        g_bErrFlag = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Check if the cause is message object 5, which what we are using for
    // receiving messages.
    else if(ui32Status == MESSAGE_RESET_UDC_OBJ_ID)
    {
        // Getting to this point means that the RX interrupt occurred on
        // message object 5, and the message RX is complete.
        // Clear the message object interrupt.

        CANIntClear(CAN0_BASE, MESSAGE_RESET_UDC_OBJ_ID);

        handle_reset_message();

        // Since the message was sent, clear any error flags.
        g_bErrFlag = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Check if the cause is message object 6, which what we are using for
    // receiving messages.
    else if(ui32Status == MESSAGE_PARAM_UDC_OBJ_ID)
    {
        // Getting to this point means that the RX interrupt occurred on
        // message object 6, and the message RX is complete.
        // Clear the message object interrupt.

        CANIntClear(CAN0_BASE, MESSAGE_PARAM_UDC_OBJ_ID);

        /* Rx object 6. Nothing to do for now. */

        // Since the message was sent, clear any error flags.
        g_bErrFlag = 0;
    }

/////////////////////////////////////////////////////////////////////////////////////////////

    // Otherwise, something unexpected caused the interrupt.
    // This should never happen.
    else
    {

        // Spurious interrupt handling can go here.

    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

void InitCan(uint32_t ui32SysClock)
{
    // Configure the GPIO pin muxing to select CAN0 functions for these pins.
    GPIOPinConfigure(GPIO_PA0_CAN0RX);
    GPIOPinConfigure(GPIO_PA1_CAN0TX);

    // Enable the alternate function on the GPIO pins.  The above step selects
    GPIOPinTypeCAN(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // The GPIO port and pins have been set up for CAN.  The CAN peripheral
    // must be enabled.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    // Initialize the CAN controller
    CANInit(CAN0_BASE);

    // Set up the bit rate for the CAN bus 1Mbps
    CANBitRateSet(CAN0_BASE, ui32SysClock, 1000000);

    // Enable interrupts on the CAN peripheral.
    CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);

    CANIntRegister(CAN0_BASE, &can_isr);

    IntPrioritySet(INT_CAN0, 1);

    // Disable auto-retry if no ACK-bit is received by the CAN controller.
    CANRetrySet(CAN0_BASE, 0);

    // Enable the CAN for operation.
    CANEnable(CAN0_BASE);

/////////////////////////////////////////////////////////////////////////////////////////////
    /*configuration sending messages*/
/////////////////////////////////////////////////////////////////////////////////////////////

    //message object 1
    tx_message_data_iib.ui32MsgID           = MESSAGE_DATA_IIB_ID;
    tx_message_data_iib.ui32MsgIDMask       = 0;
    tx_message_data_iib.ui32Flags           = (MSG_OBJ_TX_INT_ENABLE | MSG_OBJ_FIFO);
    tx_message_data_iib.ui32MsgLen          = MESSAGE_DATA_IIB_LEN;

/////////////////////////////////////////////////////////////////////////////////////////////

    //message object 2
    tx_message_itlk_iib.ui32MsgID           = MESSAGE_ITLK_IIB_ID;
    tx_message_itlk_iib.ui32MsgIDMask       = 0;
    tx_message_itlk_iib.ui32Flags           = (MSG_OBJ_TX_INT_ENABLE | MSG_OBJ_FIFO);
    tx_message_itlk_iib.ui32MsgLen          = MESSAGE_ITLK_IIB_LEN;

/////////////////////////////////////////////////////////////////////////////////////////////

    //message object 3
    tx_message_alarm_iib.ui32MsgID          = MESSAGE_ALARM_IIB_ID;
    tx_message_alarm_iib.ui32MsgIDMask      = 0;
    tx_message_alarm_iib.ui32Flags          = (MSG_OBJ_TX_INT_ENABLE | MSG_OBJ_FIFO);
    tx_message_alarm_iib.ui32MsgLen         = MESSAGE_ALARM_IIB_LEN;

/////////////////////////////////////////////////////////////////////////////////////////////

    //message object 4
    tx_message_param_iib.ui32MsgID          = MESSAGE_PARAM_IIB_ID;
    tx_message_param_iib.ui32MsgIDMask      = 0;
    tx_message_param_iib.ui32Flags          = (MSG_OBJ_TX_INT_ENABLE | MSG_OBJ_FIFO);
    tx_message_param_iib.ui32MsgLen         = MESSAGE_PARAM_IIB_LEN;

/////////////////////////////////////////////////////////////////////////////////////////////
    /*configuration receiving messages*/
/////////////////////////////////////////////////////////////////////////////////////////////

    //message object 5
    rx_message_reset_udc.ui32MsgID          = MESSAGE_RESET_UDC_ID;
    rx_message_reset_udc.ui32MsgIDMask      = 0xfffff;
    rx_message_reset_udc.ui32Flags          = (MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER | MSG_OBJ_FIFO);
    rx_message_reset_udc.ui32MsgLen         = MESSAGE_RESET_UDC_LEN;

    CANMessageSet(CAN0_BASE, MESSAGE_RESET_UDC_OBJ_ID, &rx_message_reset_udc, MSG_OBJ_TYPE_RX);

/////////////////////////////////////////////////////////////////////////////////////////////

    //message object 6
    rx_message_param_udc.ui32MsgID         = MESSAGE_PARAM_UDC_ID;
    rx_message_param_udc.ui32MsgIDMask     = 0xfffff;
    rx_message_param_udc.ui32Flags         = (MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER | MSG_OBJ_FIFO);
    rx_message_param_udc.ui32MsgLen        = MESSAGE_PARAM_UDC_LEN;

    CANMessageSet(CAN0_BASE, MESSAGE_PARAM_UDC_OBJ_ID, &rx_message_param_udc, MSG_OBJ_TYPE_RX);

/////////////////////////////////////////////////////////////////////////////////////////////

    // Module ID
    can_address = BoardAddressRead();

    if (can_address == 0) can_address = 1;

}

/////////////////////////////////////////////////////////////////////////////////////////////

void handle_reset_message(void)
{
    uint8_t id;

    rx_message_reset_udc.pui8MsgData = message_reset_udc;

    CANMessageGet(CAN0_BASE, MESSAGE_RESET_UDC_OBJ_ID, &rx_message_reset_udc, 0);

    id = message_reset_udc[0];

    if (id == can_address)
    {
        InterlockClear();

        send_itlk_message(1);

        AlarmClear();

        send_alarm_message(1);

        message_reset_udc[0] = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_data_message(uint8_t var)
{
    message_data_iib[0] = can_address;
    message_data_iib[1] = var;
    message_data_iib[2] = 0;
    message_data_iib[3] = 0;
    message_data_iib[4] = g_controller_iib.iib_signals[var].u8[0];
    message_data_iib[5] = g_controller_iib.iib_signals[var].u8[1];
    message_data_iib[6] = g_controller_iib.iib_signals[var].u8[2];
    message_data_iib[7] = g_controller_iib.iib_signals[var].u8[3];

    tx_message_data_iib.pui8MsgData = message_data_iib;

    CANMessageSet(CAN0_BASE, MESSAGE_DATA_IIB_OBJ_ID, &tx_message_data_iib, MSG_OBJ_TYPE_TX);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_itlk_message(uint8_t var)
{
    message_itlk_iib[0] = can_address;
    message_itlk_iib[1] = var;
    message_itlk_iib[2] = 0;
    message_itlk_iib[3] = 0;
    message_itlk_iib[4] = g_controller_iib.iib_itlk[var].u8[0];
    message_itlk_iib[5] = g_controller_iib.iib_itlk[var].u8[1];
    message_itlk_iib[6] = g_controller_iib.iib_itlk[var].u8[2];
    message_itlk_iib[7] = g_controller_iib.iib_itlk[var].u8[3];

    tx_message_itlk_iib.pui8MsgData = message_itlk_iib;

    CANMessageSet(CAN0_BASE, MESSAGE_ITLK_IIB_OBJ_ID, &tx_message_itlk_iib, MSG_OBJ_TYPE_TX);

    message_itlk_iib[0] = 0;
    message_itlk_iib[1] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void send_alarm_message(uint8_t var)
{
    message_alarm_iib[0] = can_address;
    message_alarm_iib[1] = var;
    message_alarm_iib[2] = 0;
    message_alarm_iib[3] = 0;
    message_alarm_iib[4] = g_controller_iib.iib_alarm[var].u8[0];
    message_alarm_iib[5] = g_controller_iib.iib_alarm[var].u8[1];
    message_alarm_iib[6] = g_controller_iib.iib_alarm[var].u8[2];
    message_alarm_iib[7] = g_controller_iib.iib_alarm[var].u8[3];

    tx_message_alarm_iib.pui8MsgData = message_alarm_iib;

    CANMessageSet(CAN0_BASE, MESSAGE_ALARM_IIB_OBJ_ID, &tx_message_alarm_iib, MSG_OBJ_TYPE_TX);

    message_alarm_iib[0] = 0;
    message_alarm_iib[1] = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

uint16_t get_can_address(void)
{
    return can_address;
}

/////////////////////////////////////////////////////////////////////////////////////////////





