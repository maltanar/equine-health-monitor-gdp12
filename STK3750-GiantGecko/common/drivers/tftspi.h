/**************************************************************************//**
 * @file
 * @brief EFM32GG_DK3750, SPI controller API for SSD2119 display interface
 *        when using Generic/Direct Drive mode
 * @author Energy Micro AS
 * @version 2.0.3
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
#ifndef __SPI_TFT_H
#define __SPI_TFT_H

#include <stdint.h>

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Tft
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void SPI_TFT_Init(void);
void SPI_TFT_WriteRegister(uint8_t reg, uint16_t data);

#ifdef __cplusplus
}
#endif

/** @} (end group Tft) */
/** @} (end group Drivers) */

#endif
