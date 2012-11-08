/**************************************************************************//**
 * @file
 * @brief EFM32TG_STK3000 Starter Kit, Board Support Header File
 * @author Energy Micro AS
 * @version 3.0.2
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
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
#ifndef __STK_H
#define __STK_H
#include <stdint.h>
#include <stdbool.h>
#include "stk_packet.h"

/***************************************************************************//**
 * @addtogroup BSP
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void STK_SendPacket(STK_Packet *pkt);
bool STK_ReceivePacket(STK_Packet *pkt);
void STK_Init(void);

void STK_StdOut(char *buffer, int length);
float STK_Current(void);
float STK_Voltage(void);
bool STK_Ready(void);
bool STK_EnergyMode(uint8_t em);

#ifdef __cplusplus
}
#endif

/** @} (end group BSP) */

#endif
