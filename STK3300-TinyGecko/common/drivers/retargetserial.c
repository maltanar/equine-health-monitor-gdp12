/***************************************************************************//**
 * @file
 * @brief Provide stdio retargeting to USART/UART or LEUART.
 * @author Energy Micro AS
 * @version 3.0.2
 *******************************************************************************
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
#include <stdio.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_int.h"
#include "em_gpio.h"
#include "retargetserial.h"

/***************************************************************************//**
 * @addtogroup RetargetIo
 * @{
 ******************************************************************************/

/** Use USART1 by default. */
#define RETARGET_USART1
/* Override if needed with commandline parameter -DRETARGET_xxx */
#if !defined(RETARGET_USART1) && !defined(RETARGET_LEUART0)
#define RETARGET_USART1    /**< Use USART1 by default. */
#endif

#if defined(RETARGET_USART1)
  #define RETARGET_IRQ_NAME    USART1_RX_IRQHandler         /**< USART IRQ Handler */
  #define RETARGET_CLK         cmuClock_USART1              /**< HFPER Clock */
  #define RETARGET_IRQn        USART1_RX_IRQn               /**< IRQ# 11 */
  #define RETARGET_UART        USART1                       /**< Adress range: 0x4000C400 – 0x4000C7FF */
  #define RETARGET_TX          USART_Tx                     /**< Set TX to USART_Tx */
  #define RETARGET_RX          USART_Rx                     /**< Set RX to USART_Rx */
  #define RETARGET_LOCATION    USART_ROUTE_LOCATION_LOC1    /**< Location of of the USART I/O pins */
  #define RETARGET_TXPORT      gpioPortD                    /**< USART transmission port */
  #define RETARGET_TXPIN       0                            /**< USART transmission pin */
  #define RETARGET_RXPORT      gpioPortD                    /**< USART reception port */
  #define RETARGET_RXPIN       1                            /**< USART reception pin */
  #define RETARGET_USART       1                            /**< Includes em_usart.h */

#elif defined(RETARGET_LEUART0)
  #define RETARGET_IRQ_NAME    LEUART0_IRQHandler           /**< LEUART IRQ Handler */
  #define RETARGET_CLK         cmuClock_LEUART0             /**< LFB Clock */
  #define RETARGET_IRQn        LEUART0_IRQn                 /**< IRQ# 14 */
  #define RETARGET_UART        LEUART0                      /**< Adress range: 0x40084000 – 0x400843FF */
  #define RETARGET_TX          LEUART_Tx                    /**< Set TX to LEUART_Tx */
  #define RETARGET_RX          LEUART_Rx                    /**< Set RX to LEUART_Rx */
  #define RETARGET_TXPORT      gpioPortD                    /**< LEUART transmission port */
  #define RETARGET_TXPIN       4                            /**< LEUART transmission pin */
  #define RETARGET_RXPORT      gpioPortD                    /**< LEUART reception port */
  #define RETARGET_RXPIN       5                            /**< LEUART reception pin */
  #define RETARGET_LOCATION    LEUART_ROUTE_LOCATION_LOC0   /**< Location of of the LEUART I/O pins */
  #define RETARGET_LEUART      1                            /**< Includes em_leuart.h */

#else
#error "Illegal USART/LEUART selection."
#endif

#if defined(RETARGET_USART)
#include "em_usart.h"
#endif

#if defined(RETARGET_LEUART)
#include "em_leuart.h"
#endif

/* Receive buffer */
#define RXBUFSIZE    8                          /**< Buffer size for RX */
static volatile int     rxReadIndex  = 0;       /**< Index in buffer to be read */
static volatile int     rxWriteIndex = 0;       /**< Index in buffer to be written to */
static volatile int     rxCount      = 0;       /**< Keeps track of how much data which are stored in the buffer */
static volatile uint8_t rxBuffer[RXBUFSIZE];    /**< Buffer to store data */
static uint8_t          LFtoCRLF    = 0;        /**< LF to CRLF conversion disabled */
static bool             initialized = false;    /**< Initialize UART/LEUART */

/**************************************************************************//**
 * @brief UART/LEUART IRQ Handler
 *****************************************************************************/
void RETARGET_IRQ_NAME(void)
{
#if defined(RETARGET_USART)
  if (RETARGET_UART->STATUS & USART_STATUS_RXDATAV)
  {
#else
  if (RETARGET_UART->IF & LEUART_IF_RXDATAV)
  {
#endif

    /* Store Data */
    rxBuffer[rxWriteIndex] = RETARGET_RX(RETARGET_UART);
    rxWriteIndex++;
    rxCount++;
    if (rxWriteIndex == RXBUFSIZE)
    {
      rxWriteIndex = 0;
    }
    /* Check for overflow - flush buffer */
    if (rxCount > RXBUFSIZE)
    {
      rxWriteIndex = 0;
      rxCount      = 0;
      rxReadIndex  = 0;
    }
  }
}

/** @} (end group RetargetIo) */

/**************************************************************************//**
 * @brief UART/LEUART toggle LF to CRLF conversion
 * @param on If non-zero, automatic LF to CRLF conversion will be enabled
 *****************************************************************************/
void RETARGET_SerialCrLf(int on)
{
  if (on)
    LFtoCRLF = 1;
  else
    LFtoCRLF = 0;
}


/**************************************************************************//**
 * @brief Intializes UART/LEUART
 *****************************************************************************/
void RETARGET_SerialInit(void)
{
  /* Configure GPIO pins */
  CMU_ClockEnable(cmuClock_GPIO, true);
  /* To avoid false start, configure output as high */
  GPIO_PinModeSet(RETARGET_TXPORT, RETARGET_TXPIN, gpioModePushPull, 1);
  GPIO_PinModeSet(RETARGET_RXPORT, RETARGET_RXPIN, gpioModeInput, 0);

#if defined(RETARGET_USART)
  USART_TypeDef           *usart = RETARGET_UART;
  USART_InitAsync_TypeDef init   = USART_INITASYNC_DEFAULT;

  /* Enable peripheral clocks */
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(RETARGET_CLK, true);

  /* Configure USART for basic async operation */
  init.enable = usartDisable;
  USART_InitAsync(usart, &init);

  /* Enable pins at UART1 location #2 */
  usart->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN | RETARGET_LOCATION;

  /* Clear previous RX interrupts */
  USART_IntClear(RETARGET_UART, USART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(RETARGET_IRQn);

  /* Enable RX interrupts */
  USART_IntEnable(RETARGET_UART, USART_IF_RXDATAV);
  NVIC_EnableIRQ(RETARGET_IRQn);

  /* Finally enable it */
  USART_Enable(usart, usartEnable);

#else
  LEUART_TypeDef      *leuart = RETARGET_UART;
  LEUART_Init_TypeDef init    = LEUART_INIT_DEFAULT;

  /* Enable CORE LE clock in order to access LE modules */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Select LFXO for LEUARTs (and wait for it to stabilize) */
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

  CMU_ClockEnable(RETARGET_CLK, true);

  /* Do not prescale clock */
  CMU_ClockDivSet(RETARGET_CLK, cmuClkDiv_1);

  /* Configure LEUART */
  init.enable = leuartDisable;
  LEUART_Init(leuart, &init);
  /* Enable pins at default location */
  leuart->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN | RETARGET_LOCATION;

  /* Clear previous RX interrupts */
  LEUART_IntClear(RETARGET_UART, LEUART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(RETARGET_IRQn);

  /* Enable RX interrupts */
  LEUART_IntEnable(RETARGET_UART, LEUART_IF_RXDATAV);
  NVIC_EnableIRQ(RETARGET_IRQn);

  /* Finally enable it */
  LEUART_Enable(leuart, leuartEnable);
#endif

#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
  setvbuf(stdout, NULL, _IONBF, 0);   /*Set unbuffered mode for stdout (newlib)*/
#endif

  initialized = true;
}


/**************************************************************************//**
 * @brief Receive a byte from USART/LEUART and put into global buffer
 * @return -1 on failure, or positive character integer on sucesss
 *****************************************************************************/
int RETARGET_ReadChar(void)
{
  int c = -1;

  INT_Disable();
  if (rxCount > 0)
  {
    c = rxBuffer[rxReadIndex];
    rxReadIndex++;
    if (rxReadIndex == RXBUFSIZE)
    {
      rxReadIndex = 0;
    }
    rxCount--;
  }
  INT_Enable();

  return c;
}

/**************************************************************************//**
 * @brief Transmit single byte to USART/LEUART
 * @param c Character to transmit
 *****************************************************************************/
int RETARGET_WriteChar(char c)
{
  if (initialized == false)
  {
    RETARGET_SerialInit();
  }

  /* Add CR or LF to CRLF if enabled */
  if (LFtoCRLF && (c == '\n'))
  {
    RETARGET_TX(RETARGET_UART, '\r');
  }
  RETARGET_TX(RETARGET_UART, c);

  if (LFtoCRLF && (c == '\r'))
  {
    RETARGET_TX(RETARGET_UART, '\n');
  }

  return c;
}
