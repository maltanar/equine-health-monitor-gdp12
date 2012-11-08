/**************************************************************************//**
 * @file
 * @brief EFM32TG_STK3000, Board Support Package support functions
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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "efm32.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_usart.h"
#include "stk.h"

/* Configure USART1 */
static USART_TypeDef *usart = USART1;       /**< Configure UART0 */
#define CMU_USART_CLOCK    cmuClock_USART1  /**< Configure CMU Clock for UART0 */
/** Configure USART in UART mode at 115200 8-N-1 */
static USART_InitAsync_TypeDef usartInit = USART_INITASYNC_DEFAULT;

/**************************************************************************//**
 * @brief Transmit single byte with given USART
 * @param data Character to transmit
 * @param usart USART configurations
 *****************************************************************************/
__STATIC_INLINE void txByte(USART_TypeDef *usart, uint8_t data)
{
  /* Check that transmit buffer is empty */
  while (!(usart->STATUS & USART_STATUS_TXBL)) ;
  usart->TXDATA = (uint32_t) data;
}


/**************************************************************************//**
 * @brief Receive a packet over the UART interface
 * @param pkt Packet to receive.
 * @return false on failure, or true on success
 *****************************************************************************/
bool STK_ReceivePacket(STK_Packet *pkt)
{
  uint8_t *rxPtr = (uint8_t *) pkt;
  int     length;

  /* Receive packet magic */
  while (!(usart->STATUS & USART_STATUS_RXDATAV)) ;
  *rxPtr++ = usart->RXDATA;
  if (pkt->magic != STK_MAGIC)
  {
    /* Invalid packet */
    memset(pkt, 0x00, sizeof(STK_Packet));
    return false;
  }

  /* Receive packet type */
  while (!(usart->STATUS & USART_STATUS_RXDATAV)) ;
  *rxPtr++ = usart->RXDATA;
  if ((pkt->type < STK_PACKETTYPE_FIRST) || (pkt->type > STK_PACKETTYPE_LAST))
  {
    /* Invalid packet */
    memset(pkt, 0x00, sizeof(STK_Packet));
    return false;
  }

  /* Receive packet length */
  while (!(usart->STATUS & USART_STATUS_RXDATAV)) ;
  *rxPtr++ = usart->RXDATA;
  if (pkt->payloadLength > STK_PACKET_SIZE)
  {
    /* Invalid packet */
    memset(pkt, 0x00, sizeof(STK_Packet));
    return false;
  }
  /* Receive packet data */
  length = pkt->payloadLength;
  while (length)
  {
    while (!(usart->STATUS & USART_STATUS_RXDATAV)) ;
    *rxPtr++ = usart->RXDATA;
    length--;
  }

  return true;
}


/**************************************************************************//**
 * @brief Transmit packet to board controller
 * @param pkt Packet to transmit
 *****************************************************************************/
void STK_SendPacket(STK_Packet *pkt)
{
  int i;

  /* Apply magic */
  pkt->magic = 0xf1;

  /* Transmit packet magic */
  txByte(usart, pkt->magic);
  /* Transmit packet type */
  txByte(usart, pkt->type);
  /* Transmit packet length */
  txByte(usart, pkt->payloadLength);
  /* Transmit packet payload */
  for (i = 0; i < pkt->payloadLength; i++)
  {
    txByte(usart, pkt->data[i]);
  }
}


/**************************************************************************//**
 * @brief Transmit packets to transport output to Board Controller
 * @param buffer Buffer containg packets.
 * @param length Size of packets (bytes).
 *****************************************************************************/
void STK_StdOut(char *buffer, int length)
{
  STK_Packet pkt;
  int        pktLength;

  /* Split buffer into multiple packets if necessary */
  while (length)
  {
    pkt.type = STK_PACKETTYPE_STDOUT;
    if (length > STK_PACKET_SIZE)
    {
      pktLength = STK_PACKET_SIZE;
    }
    else
    {
      pktLength = length;
    }
    pkt.payloadLength = pktLength;
    memcpy(pkt.data, buffer, pktLength);

    /* Transfer packet */
    STK_SendPacket(&pkt);
    if (!STK_ReceivePacket(&pkt)) return;

    /* Update pointers */
    length -= pktLength;
    buffer += pktLength;
  }
}


/**************************************************************************//**
 * @brief STK Check if Board Controller is alive
 *****************************************************************************/
bool STK_Ready(void)
{
  STK_Packet pkt;

  /* Transfer Echo packet */
  pkt.type          = STK_PACKETTYPE_ECHO_REQ;
  pkt.payloadLength = 0;

  /* Send Request/Get reply */
  STK_SendPacket(&pkt);
  STK_ReceivePacket(&pkt);

  if (pkt.type == STK_PACKETTYPE_ECHO_REPLY)
  {
    return true;
  }
  return false;
}


/**************************************************************************//**
 * @brief Inform board controller about current energy mode for AEM display
 * @param em Current Energymode
 *****************************************************************************/
bool STK_EnergyMode(uint8_t em)
{
  STK_Packet pkt;

  /* Transfer Echo packet */
  pkt.type          = STK_PACKETTYPE_ENERGYMODE;
  pkt.payloadLength = 1;
  pkt.data[0]       = em;

  /* Send Request/Get reply */
  STK_SendPacket(&pkt);
  STK_ReceivePacket(&pkt);

  if (pkt.type == STK_PACKETTYPE_ACK)
  {
    return true;
  }
  return false;
}


/**************************************************************************//**
 * @brief Request AEM current from Board Controller
 *****************************************************************************/
float STK_Current(void)
{
  STK_Packet pkt;
  float      current;

  pkt.type          = STK_PACKETTYPE_CURRENT_REQ;
  pkt.payloadLength = 0;

  /* Send Request/Get reply */
  STK_SendPacket(&pkt);
  STK_ReceivePacket(&pkt);

  /* Process reply */
  if (pkt.type == STK_PACKETTYPE_CURRENT_REPLY)
  {
    memcpy(&current, pkt.data, sizeof(float));
    return current;
  }
  else
  {
    return (float) 0.0;
  }
}


/**************************************************************************//**
 * @brief Request AEM current from Board Controller
 *****************************************************************************/
float STK_Voltage(void)
{
  STK_Packet pkt;
  float      voltage;

  pkt.type          = STK_PACKETTYPE_VOLTAGE_REQ;
  pkt.payloadLength = 0;

  /* Send Request/Get reply */
  STK_SendPacket(&pkt);
  STK_ReceivePacket(&pkt);

  /* Process reply */
  if (pkt.type == STK_PACKETTYPE_VOLTAGE_REPLY)
  {
    memcpy(&voltage, pkt.data, sizeof(float));
    return voltage;
  }
  else
  {
    return (float) 0.0;
  }
}


/**************************************************************************//**
 * @brief Intializes STK Clock Management Unit
 *****************************************************************************/
static void STK_CMUInit(void)
{
  /* Enable High Frequency Peripherals */
  CMU_ClockEnable(cmuClock_HFPER, true);

  /* Enable clocks to GPIO */
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(CMU_USART_CLOCK, true);
}


/**************************************************************************//**
 * @brief Intializes STK GPIO Pin Usage
 *****************************************************************************/
static void STK_GPIOInit(void)
{
  /* Configure GPIO pins USART1 LOCATION 2 */
  /* TX - Pin D7; RX - Pin D6 */
  GPIO_PinModeSet(gpioPortD, 6, gpioModeInput, 0);
  /* To avoid false start, configure output as high   */
  GPIO_PinModeSet(gpioPortD, 7, gpioModePushPull, 1);

  /* Enable switch U602A "VMCU switch" - to enable USART communication */
  /* See board schematics for details */
  GPIO_PinModeSet(gpioPortC, 14, gpioModePushPull, 1);
}


/**************************************************************************//**
 * @brief Intializes STK USART1 for UART mode towards Board Controller
 *****************************************************************************/
static void STK_USARTInit(void)
{
  /* Initialize USART */
  USART_InitAsync(usart, &usartInit);

  /* Enable location 2 - PD6 + PD7 */
  usart->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN |
                 USART_ROUTE_LOCATION_LOC2;
}


/**************************************************************************//**
 * @brief Intializes STK
 * @note If HFPERCLK is changed, this routine needs to be called again
 *****************************************************************************/
void STK_Init(void)
{
  /* Enable CMU clocks */
  STK_CMUInit();
  /* Enable GPIO pins, for USART - E0/E1, and analog switch D13 */
  STK_GPIOInit();
  /* Enable USART */
  STK_USARTInit();
}
