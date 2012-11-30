/**************************************************************************//**
 * @file
 * @brief USART prototypes and definitions
 * @author Energy Micro AS
 * @version 1.10
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#ifndef __USART_H
#define __USART_H

#include <stdbool.h>
#include "efm32.h"

#define NO_RX                    0
#define NO_TX                    NO_RX

#define USART0_NUM               0
#define USART1_NUM               1

#define GPIO_POS0                0
#define GPIO_POS1                1
#define GPIO_POS2                2


void USART1_getBuffer(uint16_t*, int);



#endif
