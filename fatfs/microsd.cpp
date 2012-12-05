/**************************************************************************//**
 * @file
 * @brief Micro SD card driver for the EFM32GG_DK3750 development kit.
 *        This file provides the interface layer between the DVK and the
 *        fat filesystem provided.
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
#include "diskio.h"
#include "microsd.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

//#define MICROSD_USARTCFG	(USARTManagerPortConfigs[SD_USART_PORT])
//#define MICROSD_USART		(MICROSD_USARTCFG.usartBase)
	
//#define CS_LOW()   GPIO->P[4].DOUTCLR = 0x10;  /**< Chip Selection in SPI mode. Enable when card is selected. */
//#define CS_HIGH()  GPIO->P[4].DOUTSET = 0x10;  /**< Chip Selection in SPI mode. Enable when card is deselected. */

#define CS_LOW()   GPIO_PinOutClear(MICROSD_USARTCFG.csPort, MICROSD_USARTCFG.csPin)
#define CS_HIGH()  GPIO_PinOutSet(MICROSD_USARTCFG.csPort, MICROSD_USARTCFG.csPin)

extern DSTATUS Stat;    /**< Disk status */


/* Local prototypes. */
static BYTE wait_ready(void);

/**************************************************************************//**
 * @brief
 *	Initialize SPI setting.
 *  The FatFS system controls the SPI CS directly through the macros
 *  CS_HIGH() and CS_LOW().
 *****************************************************************************/
void MICROSD_init(void)
{
  USART_TypeDef *spi;

  /* Enabling clock to USART 0 */
  CMU_ClockEnable(MICROSD_USARTCFG.clockPoint, true);
  CMU_ClockEnable(cmuClock_GPIO, true);

  spi = MICROSD_USART;

  /* Setting baudrate */
  FCLK_SLOW()

  /* Configure SPI */
  /* Using synchronous (SPI) mode*/
  spi->CTRL = USART_CTRL_SYNC | USART_CTRL_MSBF;
  /* Clearing old transfers/receptions, and disabling interrupts */
  spi->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
  spi->IEN = 0;
  /* Enabling pins and setting location, SPI CS not enable */
  spi->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN | 
	  			(MICROSD_USARTCFG.routeLocation << _USART_ROUTE_LOCATION_SHIFT);
  
  /* Enabling TX and RX */
  spi->CMD = USART_CMD_TXEN | USART_CMD_RXEN;

  /* Set to master and without controlling the CS line */
  spi->CMD = USART_CMD_MASTEREN;

  /* Clear previous interrupts */
  spi->IFC = _USART_IFC_MASK;

  /* IO configuration */
  GPIO_PinModeSet(MICROSD_USARTCFG.txPort, MICROSD_USARTCFG.txPin, gpioModePushPull, 0);  /* MOSI */
  GPIO_PinModeSet(MICROSD_USARTCFG.rxPort, MICROSD_USARTCFG.rxPin, gpioModeInputPull, 1); /* MISO */
  GPIO_PinModeSet(MICROSD_USARTCFG.csPort, MICROSD_USARTCFG.csPin, gpioModePushPull, 0);  /* CS */
  GPIO_PinModeSet(MICROSD_USARTCFG.sclkPort, MICROSD_USARTCFG.sclkPin, gpioModePushPull, 0);  /* Clock */
}

/**************************************************************************//**
 * @brief
 *	Deinitialize SPI setting.
 *  Releases any resources alliocated in the initialization.
 *****************************************************************************/
void MICROSD_deinit(void)
{
  MICROSD_USART->CTRL = 0;

  /* IO configuration */
  GPIO_PinModeSet(MICROSD_USARTCFG.txPort, MICROSD_USARTCFG.txPin, gpioModeDisabled, 0);  /* MOSI */
  GPIO_PinModeSet(MICROSD_USARTCFG.rxPort, MICROSD_USARTCFG.rxPin, gpioModeDisabled, 0); /* MISO */
  GPIO_PinModeSet(MICROSD_USARTCFG.csPort, MICROSD_USARTCFG.csPin, gpioModeDisabled, 0);  /* CS */
  GPIO_PinModeSet(MICROSD_USARTCFG.sclkPort, MICROSD_USARTCFG.sclkPin, gpioModeDisabled, 0);  /* Clock */
}

/**************************************************************************//**
 * @brief
 *     Do one SPI transfer on a SD-card
 *
 * @param data
 *     Byte to transmit
 *
 * @return
 *     Byte received
 *****************************************************************************/
uint8_t xfer_spi(uint8_t data)
{
  USART_TypeDef *spi = MICROSD_USART;

  spi->TXDATA = data;
  while (!(spi->STATUS & USART_STATUS_TXC))
  {
  }
  return (uint8_t)(spi->RXDATA);
}

/**************************************************************************//**
 * @brief Wait for card ready
 * @return 0xff: card ready, other value: card not ready
 *****************************************************************************/
static BYTE wait_ready(void)
{
  BYTE res;
  /* Wait for ready in timeout of 500ms */
  setDiskAlarm(1);
  do
    res = xfer_spi(0xff);
  while ((res != 0xFF) && !diskAlarmTimeout);
  stopDiskAlarm();
  return res;
}

/**************************************************************************//**
 * @brief Deselect the card and release SPI bus
 *****************************************************************************/
void deselect(void)
{
  CS_HIGH();
  xfer_spi(0xff);
}

/**************************************************************************//**
 * @brief Select the card and wait ready
 * @return 1:Successful, 0:Timeout
 *****************************************************************************/
int select(void)
{
  CS_LOW();
  if (wait_ready() != 0xFF)
  {
    deselect();
    return 0;
  }
  return 1;
}

/**************************************************************************//**
 * @brief Turn on SD card power
 *        DVK doesn't support socket power control, only enable the SPI clock
 *****************************************************************************/
void power_on(void)
{
  /* Enable SPI */
  CMU_ClockEnable(MICROSD_USARTCFG.clockPoint, true);
}

/**************************************************************************//**
 * @brief Turn off SD card power
 *        DVK doesn't support socket power control, only disable the SPI clock
 *****************************************************************************/
void power_off(void)
{
  /* Wait for card ready */
  select();
  deselect();
  /* Disable USART */
  CMU_ClockEnable(MICROSD_USARTCFG.clockPoint, false);
  /* Set STA_NOINIT */
  Stat |= STA_NOINIT;
}

/**************************************************************************//**
 * @brief Receive a data packet from MMC
 * @param[out] buff
 *        Data buffer to store received data
 * @param btr
 *        Byte count (must be multiple of 4)
 * @return
 *        1:OK, 0:Failed
 *****************************************************************************/
int rcvr_datablock(BYTE *buff, UINT btr)
{
  BYTE     token;
  uint16_t *buff_16 = (uint16_t *) buff;
  uint32_t framectrl;
  uint32_t ctrl;

  /* Wait for data packet in timeout of 100ms */
  setDiskAlarm(1);
  do
    token = xfer_spi(0xff);
  while ((token == 0xFF) && (!diskAlarmTimeout));
  stopDiskAlarm();

  if (token != 0xFE)
  {
    /* Invalid data token */
    return 0;
  }

  USART_TypeDef *spi = MICROSD_USART;
  /* Clear send and receive buffer */
  spi->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
  /* Store old configuration. */
  framectrl = spi->FRAME;
  ctrl      = spi->CTRL;
  /* Set up frames to 16 bit on each frame. This will increase the
   * data rate and make the maximum use of the buffers available. */
  spi->FRAME = (spi->FRAME & (~_USART_FRAME_DATABITS_MASK))
               | USART_FRAME_DATABITS_SIXTEEN;
  spi->CTRL = (spi->CTRL & (~_USART_CTRL_BYTESWAP_MASK))
              | USART_CTRL_BYTESWAP;
  while (!(spi->STATUS & USART_STATUS_TXBL)) ;
  /* Pipelining - The USART has two buffers of 16 bit in both
   * directions. Make sure that at least one is in the pipe at all
   * times to maximize throughput. */
  spi->TXDOUBLE = 0xffff;
  /* Receive the data block into buffer */
  do
  {
    spi->TXDOUBLE = 0xffff;
    while (!(spi->STATUS & USART_STATUS_RXFULL)) ;
    *buff_16++ = (uint16_t) MICROSD_USART->RXDOUBLE;
  } while (btr -= 2);

  /* Next two bytes is the CRC which we discard. */
  while (!(spi->STATUS & USART_STATUS_RXFULL)) ;
  spi->CMD = USART_CMD_CLEARRX;

  /* Restore 8-bit operation */
  spi->FRAME = framectrl;
  spi->CTRL  = ctrl;

  return 1;     /* Return with success */
}

/**************************************************************************//**
 * @brief Send a data packet to MMC
 * @param[in] buff 512 bytes data block to be transmitted
 * @param token Data token
 * @return 1:OK, 0:Failed
 *****************************************************************************/
#if _READONLY == 0
int xmit_datablock(const BYTE *buff, BYTE token)
{
  BYTE resp;
  UINT bc = 512;

  if (wait_ready() != 0xFF)
    return 0;

  xfer_spi(token);          /* Xmit a token */
  if (token != 0xFD)
  {                         /* Not StopTran token */
    do
    {
      /* Xmit the 512 byte data block to the MMC */
      xfer_spi(*buff++);
      xfer_spi(*buff++);
    } while (bc -= 2);

    xfer_spi(0xFF);             /* CRC (Dummy) */
    xfer_spi(0xFF);
    resp = xfer_spi(0xff);      /* Receive a data response */
    if ((resp & 0x1F) != 0x05)  /* If not accepted, return with error */
      return 0;
  }
  return 1;
}
#endif  /* _READONLY */

/**************************************************************************//**
 * @brief
 *	Send a command packet to MMC
 *
 * @param[in] cmd
 *	Command byte
 *
 * @param[in] arg
 *	Argument
 *
 * @return
 *	Response value
 *****************************************************************************/
BYTE send_cmd(BYTE cmd, DWORD arg)
{
  BYTE         n, res;
  volatile int timeOut;

  if (cmd & 0x80)
  { /* ACMD<n> is the command sequense of CMD55-CMD<n> */
    cmd &= 0x7F;
    res  = send_cmd(CMD55, 0);
    if (res > 1) return res;
  }

  /* Select the card and wait for ready */
  deselect();
  if (!select())
    return 0xFF;

  /* Send command packet */
  xfer_spi(0x40 | cmd);           /* Start + Command index */
  xfer_spi((BYTE)(arg >> 24));    /* Argument[31..24] */
  xfer_spi((BYTE)(arg >> 16));    /* Argument[23..16] */
  xfer_spi((BYTE)(arg >> 8));     /* Argument[15..8] */
  xfer_spi((BYTE) arg);           /* Argument[7..0] */
  n = 0x01;                       /* Dummy CRC + Stop */
  if (cmd == CMD0)
    n = 0x95;                     /* Valid CRC for CMD0(0) */
  if (cmd == CMD8)
    n = 0x87;                     /* Valid CRC for CMD8(0x1AA) */
  xfer_spi(n);

  /* Receive command response */
  if (cmd == CMD12)
    xfer_spi(0xff);               /* Skip a stuff byte when stop reading */
  timeOut = 10;                   /* Wait for a valid response in timeout of 10 attempts */
  do
    res = xfer_spi(0xff);
  while ((res & 0x80) && --timeOut);

  return res;             /* Return with the response value */
}
