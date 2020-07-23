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
 * @file fac_is.h
 * @brief Brief description of module
 * 
 * Detailed description
 *
 * @author allef.silva
 * @date 20 de out de 2018
 *
 */

/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef FAC_IS_H_
#define FAC_IS_H_

/////////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

/////////////////////////////////////////////////////////////////////////////////////////////

#define FAC_IS_DCLINK_OVERVOLTAGE_ITLK         0x00000001
#define FAC_IS_INPUT_OVERCURRENT_ITLK          0x00000002
#define FAC_IS_IGBT1_OVERTEMP_ITLK             0x00000004
#define FAC_IS_IGBT1_HWR_OVERTEMP_ITLK         0x00000008
#define FAC_IS_DRIVER_OVERVOLTAGE_ITLK         0x00000010
#define FAC_IS_DRIVER1_OVERCURRENT_ITLK        0x00000020
#define FAC_IS_DRIVER1_ERROR_TOP_ITLK          0x00000040
#define FAC_IS_DRIVER1_ERROR_BOT_ITLK          0x00000080
#define FAC_IS_INDUC_OVERTEMP_ITLK             0x00000100
#define FAC_IS_HS_OVERTEMP_ITLK                0x00000200
#define FAC_IS_BOARD_IIB_OVERTEMP_ITLK         0x00000400
#define FAC_IS_BOARD_IIB_OVERHUMIDITY_ITLK     0x00000800

/////////////////////////////////////////////////////////////////////////////////////////////

#define FAC_IS_DCLINK_OVERVOLTAGE_ALM          0x00000001
#define FAC_IS_INPUT_OVERCURRENT_ALM           0x00000002
#define FAC_IS_IGBT1_OVERTEMP_ALM              0x00000004
#define FAC_IS_DRIVER_OVERVOLTAGE_ALM          0x00000008
#define FAC_IS_DRIVER1_OVERCURRENT_ALM         0x00000010
#define FAC_IS_INDUC_OVERTEMP_ALM              0x00000020
#define FAC_IS_HS_OVERTEMP_ALM                 0x00000040
#define FAC_IS_BOARD_IIB_OVERTEMP_ALM          0x00000080
#define FAC_IS_BOARD_IIB_OVERHUMIDITY_ALM      0x00000100

/////////////////////////////////////////////////////////////////////////////////////////////

extern void init_fac_is(void);
extern void clear_fac_is_interlocks(void);
extern uint8_t check_fac_is_interlocks(void);
extern void clear_fac_is_alarms(void);
extern uint8_t check_fac_is_alarms(void);
extern void check_fac_is_indication_leds(void);
extern void fac_is_application_readings(void);
extern void send_input_fac_is_itlk_msg(void);
extern void fac_is_power_on_check(void);
extern void send_fac_is_data(void);

/////////////////////////////////////////////////////////////////////////////////////////////

#endif /* FAC_IS_H_ */

/////////////////////////////////////////////////////////////////////////////////////////////






