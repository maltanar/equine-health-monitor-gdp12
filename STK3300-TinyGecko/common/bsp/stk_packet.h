/**************************************************************************//**
 * @file
 * @brief STK Starter Kit - UART Communication Packet definition
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
#ifndef __STK_PACKET_H
#define __STK_PACKET_H
#include <stdint.h>

/***************************************************************************//**
 * @addtogroup BSP
 * @{
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* STK Packet Types */
#define STK_PACKETTYPE_INVALID          0   /**< Invalid packet received */

#define STK_PACKETTYPE_FIRST            1   /**< Smallest numerical value of  message type */

#define STK_PACKETTYPE_ACK              5   /**< Generic ACK for one way packages */
#define STK_PACKETTYPE_ECHO_REQ         10  /**< EFM32 BC alive request */
#define STK_PACKETTYPE_ECHO_REPLY       11  /**< BC alive response */
#define STK_PACKETTYPE_CURRENT_REQ      14  /**< EFM32 Request AEM current */
#define STK_PACKETTYPE_CURRENT_REPLY    16  /**< BC Response AEM current */
#define STK_PACKETTYPE_VOLTAGE_REQ      18  /**< EFM32 Request AEM voltage */
#define STK_PACKETTYPE_VOLTAGE_REPLY    20  /**< BC Response AEM voltage */
#define STK_PACKETTYPE_ENERGYMODE       22  /**< EFM32 Report Energy Mode (for AEM) */
#define STK_PACKETTYPE_STDOUT           24  /**< Debug packet (not used)  */
#define STK_PACKETTYPE_STDERR           26  /**< Debug packet (not used)  */
#define STK_PACKETTYPE_TEST             32  /**< Reserved type for test */
#define STK_PACKETTYPE_TEST_REPLY       33  /**< Reserved type for test (reply) */

#define STK_PACKETTYPE_LAST             100 /**< Last defined message type */

/** Speed of Board Controller UART, EFM32 must use this speed */
#define STK_BAUDRATE                    115200
/** Magic byte to indicate start of packet */
#define STK_MAGIC                       0xf1
/** Maximum payload for packets */
#ifndef STK_PACKET_SIZE
#define STK_PACKET_SIZE                 30
#endif

#if STK_PACKET_SIZE >= 255
#error "STK Board Controller communications packets must be less than 255 bytes in size!"
#endif

/** @brief STK Packet Structure - Simple Comm. Protocol */
typedef struct
{
  uint8_t magic;                  /**< Magic - start of packet - must be STK_MAGIC */
  uint8_t type;                   /**< Type of packet */
  uint8_t payloadLength;          /**< Length of data segment >=0 and <=STK_PACKET_SIZE */
  uint8_t data[STK_PACKET_SIZE];  /**< STK Packet Data payload */
} STK_Packet;

#ifdef __cplusplus
}
#endif

/** @} (end group BSP) */

#endif
