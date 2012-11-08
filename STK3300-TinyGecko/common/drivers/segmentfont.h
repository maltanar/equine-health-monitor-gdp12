/**************************************************************************//**
 * @file
 * @brief Segment LCD font and layout for the EFM32TG_STK3300 starter kit
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

#ifndef __SEGMENTFONT_H
#define __SEGMENTFONT_H

#include <stdint.h>

/***************************************************************************//**
 * @addtogroup Drivers
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup SegmentLcd
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************//**
 * @brief
 * Defines each text symbol's segment in terms of COM and BIT numbers,
 * in a way that we can enumerate each bit for each text segment in the
 * following bit pattern:
 * @verbatim
 *  -------0------
 *
 * |   \7  |8  /9 |
 * |5   \  |  /   |1
 *
 *  --6---  ---10--
 *
 * |    /  |  \11 |
 * |4  /13 |12 \  |2
 *
 *  -------3------
 * @endverbatim
 * E.g.: First text character bit pattern #3 (above) is
 *  Segment 1D for Display
 *  Location COM 3, BIT 0
 *****************************************************************************/
typedef struct
{
  uint8_t com[14]; /**< LCD COM line (for multiplexing) */
  uint8_t bit[14]; /**< LCD bit number */
} CHAR_TypeDef;


/**************************************************************************//**
 * @brief Defines segment COM and BIT fields numeric display
 *****************************************************************************/
typedef struct
{
  uint8_t com[7]; /**< LCD COM line (for multiplexing) */
  uint8_t bit[7]; /**< LCD bit number */
} NUMBER_TypeDef;

/**************************************************************************//**
 * @brief Defines segment COM and BIT fields for Energy Modes on display
 *****************************************************************************/
typedef struct
{
  uint8_t com[5]; /**< LCD COM line (for multiplexing) */
  uint8_t bit[5]; /**< LCD bit number */
} EM_TypeDef;

/**************************************************************************//**
 * @brief Defines segment COM and BIT fields for A-wheel (suited for Anim)
 *****************************************************************************/
typedef struct
{
  uint8_t com[8]; /**< LCD COM line (for multiplexing) */
  uint8_t bit[8]; /**< LCD bit number */
} ARING_TypeDef;

/**************************************************************************//**
 * @brief Defines segment COM and BIT fields for A-wheel (suited for Anim)
 *****************************************************************************/
typedef struct
{
  uint8_t com[4]; /**< LCD COM line (for multiplexing) */
  uint8_t bit[4]; /**< LCD bit number */
} BATTERY_TypeDef;

/**************************************************************************//**
 * @brief Defines prototype for all segments in display
 *****************************************************************************/
typedef struct
{
  CHAR_TypeDef    Text[7];      /**< Text on display */
  NUMBER_TypeDef  Number[4];    /**< Numbers on display */
  EM_TypeDef      EMode;        /**< Display energy mode */
  ARING_TypeDef   ARing;        /**< Display ring */
  BATTERY_TypeDef Battery;      /**< Display battery */
} MCU_DISPLAY;

extern const MCU_DISPLAY EFM_Display;
extern const uint16_t    EFM_Alphabet[];
extern const uint16_t    EFM_Numbers[];

#ifdef __cplusplus
}
#endif

/** @} (end group SegmentLcd) */
/** @} (end group Drivers) */

#endif
