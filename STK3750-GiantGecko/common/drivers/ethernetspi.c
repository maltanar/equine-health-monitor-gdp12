/**************************************************************************//**
 * @file
 * @brief SPI Interface for Ethernet controller; Micrel KSZ8851SNL
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
#include "efm32.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "ethernetspi.h"

/** SPI initialization structure */
static const USART_InitSync_TypeDef initeth =
{ usartEnable,    /* Enable RX/TX when init completed. */
  48000000,       /* Use 48MHz reference clock */
  7000000,        /* 7 Mbits/s. */
  usartDatabits8, /* 8 databits. */
  true,           /* Master mode. */
  true,           /* Send most significant bit first. */
  usartClockMode0,
  false,
  usartPrsRxCh0,
  false,          };

/** Bytes to SPI packet byte select bits conversion table */
static const uint8_t bitSelect[] = { 0x00, 0x1, 0x3, 0x7, 0xf };

/**************************************************************************//**
 * @brief SPI_ETH_Init
 *    Initialize SPI interface to Ethernet controller.
 * @note To enable access, be sure to call the functions
 *            DVK_peripheralAccess(DVK_ETH, enable);
 *    before using this interface.
 *****************************************************************************/
void SPI_ETH_Init(void)
{
  /* Enabling clock to USART */
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_USART1, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* IO configuration (USART 1, Location #1) */
  GPIO_PinModeSet(gpioPortD, 0, gpioModePushPull, 0); /* TX - MOSI */
  GPIO_PinModeSet(gpioPortD, 1, gpioModeInput, 0);    /* RX - MISO */
  GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 0); /* CLK */
  GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1); /* CS  */

  USART_Reset(USART1);

  /* Initialize USART1, in SPI master mode. */
  USART_InitSync(USART1, &initeth);

  USART1->ROUTE =
    USART_ROUTE_TXPEN |
    USART_ROUTE_RXPEN |
    USART_ROUTE_CLKPEN |
    USART_ROUTE_LOCATION_LOC1;
}


/**************************************************************************//**
 * @brief Read ethernet controller register
 * @param[in] reg Register to read
 * @param[in] numBytes Number of bytes to read, should be 1-4
 * @param[out] data Pointer to element where data should be put
 *****************************************************************************/
void SPI_ETH_ReadRegister(uint8_t reg, int numBytes, void *data)
{
  uint8_t   first, second;
  const int shift     = 2;
  uint8_t   *rxBuffer = (uint8_t *) data;

  EFM_ASSERT(numBytes > 0 && numBytes < 5);

  /* First Opcode:00, ByteEnable and MSB of register address */
  first  = bitSelect[numBytes] << (shift + (reg & 3));
  first |= ((reg & 0xc0) >> 6);
  /* Second LSB of register address, last 4 LSB is ignored */
  second = (reg << 2);

  /* Enable chip select */
  GPIO_PinOutClear(gpioPortD, 3);

  USART_Tx(USART1, first);
  /* Ignore first packet */
  USART_Rx(USART1);
  USART_Tx(USART1, second);
  /* Ignore second packet */
  USART_Rx(USART1);
  /* Read data back */
  while (numBytes--)
  {
    USART_Tx(USART1, 0x00);
    *rxBuffer++ = USART_Rx(USART1);
  }
  /* Disable chip select */
  GPIO_PinOutSet(gpioPortD, 3);
}


/**************************************************************************//**
 * @brief Write ethernet controller register
 * @param[in] reg Register to read
 * @param[in] numBytes Number of bytes to read, should be 1-4
 * @param[out] data Pointer to element where data should be put
 *****************************************************************************/
void SPI_ETH_WriteRegister(uint8_t reg, int numBytes, void *data)
{
  uint8_t   first, second;
  const int shift     = 2;
  uint8_t   *txBuffer = (uint8_t *) data;

  /* First Opcode:01, ByteEnable and MSB of register address */
  first  = 0x40 | (bitSelect[numBytes] << (shift + (reg & 3)));
  first |= ((reg & 0xc0) >> 6);
  /* Second LSB of register address, last 4 LSB is ignored */
  second = (reg << 2);

  /* Enable chip select */
  GPIO_PinOutClear(gpioPortD, 3);

  USART_Tx(USART1, first);
  USART_Rx(USART1);
  USART_Tx(USART1, second);
  USART_Rx(USART1);
  /* Write Packet data */
  while (numBytes--)
  {
    USART_Tx(USART1, *txBuffer++);
    USART_Rx(USART1);
  }
  /* Disable chip select */
  GPIO_PinOutSet(gpioPortD, 3);
}


/**************************************************************************//**
 * @brief Read ethernet controller FIFO
 * @param[in] numBytes Number of bytes to read, 1-12K
 * @param[out] data Pointer to element where data should be read from
 *****************************************************************************/
void SPI_ETH_ReadFIFO(int numBytes, void *data)
{
  uint8_t *rxBuffer = (uint8_t *) data;

  /* Enable chip select */
  GPIO_PinOutClear(gpioPortD, 3);

  USART_Tx(USART1, 0x80);
  USART_Rx(USART1);
  while (numBytes--)
  {
    USART_Tx(USART1, 0x00);
    *rxBuffer++ = USART_Rx(USART1);
  }
  /* Disable chip select */
  GPIO_PinOutSet(gpioPortD, 3);
}


/**************************************************************************//**
 * @brief Write ethernet controller FIFO
 * @param[in] numBytes Number of bytes to write, 1-12K
 * @param[out] data Pointer to element where data should be put
 *****************************************************************************/
void SPI_ETH_WriteFIFO(int numBytes, uint8_t *data)
{
  uint8_t *txBuffer = (uint8_t *) data;

  /* Enable chip select */
  GPIO_PinOutClear(gpioPortD, 3);

  USART_Tx(USART1, 0xc0);
  USART_Rx(USART1);
  while (numBytes--)
  {
    USART_Tx(USART1, *txBuffer++);
    USART_Rx(USART1);
  }
  /* Disable chip select */
  GPIO_PinOutSet(gpioPortD, 3);
}
