/***************************************************************************//**
 * @file
 * @brief Real Time Counter, implements simple trigger functionality
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

#include <stddef.h>
#include "efm32.h"
#include "em_rtc.h"
#include "em_cmu.h"
#include "rtc.h"

static void          (*rtcCb)(void);        /**< RTC callback */
static uint8_t       rtcInitialized = 0;    /**< 1 if rtc is initialized */
static volatile bool rtcDelayComplete;      /**< True while CB delay and false while polled delay  */
static uint32_t      rtcFreq;               /**< RTC Frequence. 32.768 kHz */

/***************************************************************************//**
 * @brief Delay RTC callback
 ******************************************************************************/
static void RTC_DelayCB(void)
{
  rtcDelayComplete = true;
}

/***************************************************************************//**
 * @brief
 *	Enables LFACLK and selects LFXO as clock source for RTC
 *
 * @param osc
 *	Oscillator
 ******************************************************************************/
void RTC_Setup(CMU_Select_TypeDef osc)
{
  RTC_Init_TypeDef init;

  rtcInitialized = 1;

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Enable osc as LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, osc);

  /* Use a 32 division prescaler to reduce power consumption. */
  CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_32);

  rtcFreq = CMU_ClockFreqGet(cmuClock_RTC);

  /* Enable clock to RTC module */
  CMU_ClockEnable(cmuClock_RTC, true);

  init.enable   = false;
  init.debugRun = false;
  init.comp0Top = false; /* Count to max before wrapping */
  RTC_Init(&init);

  /* Disable interrupt generation from RTC0 */
  RTC_IntDisable(_RTC_IF_MASK);

  /* Enable interrupts */
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);
}


/***************************************************************************//**
 * @brief RTC polled delay
 * @param msec Number of msec to delay (poll based)
 ******************************************************************************/
void RTC_Delay(uint32_t msec)
{
  rtcDelayComplete = false;
  RTC_Trigger(msec, RTC_DelayCB);
  while (!rtcDelayComplete) ;
}


/***************************************************************************//**
 * @brief RTC Interrupt Handler, invoke callback if defined.
 *        The interrupt table is in assembly startup file startup_efm32.s
 ******************************************************************************/
void RTC_IRQHandler(void)
{
  /* Disable RTC */
  RTC_Enable(false);

  /* Clear interrupt source */
  RTC_IntClear(RTC_IF_COMP0);

  /* Disable interrupt */
  RTC_IntDisable(RTC_IF_COMP0);

  /* Trigger callback if defined */
  if (rtcCb)
  {
    rtcCb();
  }
}


/***************************************************************************//**
 * @brief RTC trigger enable
 * @param msec Enable trigger in msec
 * @param cb Callback invoked when @p msec elapsed
 ******************************************************************************/
void RTC_Trigger(uint32_t msec, void (*cb)(void))
{
  /* Auto init if not configured already */
  if (!rtcInitialized)
  {
    /* Default to LFRCO as clock source */
    RTC_Setup(cmuSelect_LFRCO);
  }

  /* Register callback */
  rtcCb = cb;

  /* Clear interrupt source */
  RTC_IntClear(RTC_IF_COMP0);

  /* Calculate trigger value in ticks based on 32768Hz clock */
  RTC_CompareSet(0, (rtcFreq * msec) / 1000);

  /* Enable RTC */
  RTC_Enable(true);

  /* Enable interrupt on COMP0 */
  RTC_IntEnable(RTC_IF_COMP0);
}
