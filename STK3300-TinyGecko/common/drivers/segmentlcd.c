/**************************************************************************//**
 * @file
 * @brief EFM32TG_STK3300 Segment LCD Display driver
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_lcd.h"
#include "segmentlcd.h"
#include "segmentfont.h"

/**************************************************************************//**
 * @brief Write number on numeric part on Segment LCD display
 * @param value Numeric value to put on display, in range -999 to +9999
 *****************************************************************************/
void SegmentLCD_Number(int value)
{
  int      num, i, com, bit, digit, div, neg;
  uint16_t bitpattern;

  /* Parameter consistancy check */
  if (value >= 9999)
  {
    value = 9999;
  }
  if (value <= -1000)
  {
    value = -999;
  }
  if (value < 0)
  {
    value = abs(value);
    neg   = 1;
  }
  else
  {
    neg = 0;
  }

  /* If an update is in progress we must block, or there might be tearing */
  LCD_SyncBusyDelay(0xFFFFFFFF);

  /* Freeze updates to avoid partial refresh of display */
  LCD_FreezeEnable(true);

  /* Turn off all number LCD segments */
  SegmentLCD_NumberOff();

  /* Extract useful digits */
  div = 1;
  for (digit = 0; digit < 4; digit++)
  {
    num = (value / div) % 10;
    if ((neg == 1) && (digit == 3)) num = 16;
    /* Get number layout of display */
    bitpattern = EFM_Numbers[num];
    for (i = 0; i < 7; i++)
    {
      bit = EFM_Display.Number[digit].bit[i];
      com = EFM_Display.Number[digit].com[i];
      if (bitpattern & (1 << i))
      {
        LCD_SegmentSet(com, bit, true);
      }
    }
    div = div * 10;
  }
  /* Sync LCD registers to LE domain */
  LCD_FreezeEnable(false);
}

/**************************************************************************//**
 * @brief Write number on numeric part on Segment LCD display
 * @param value Numeric value to put on display, in range 0x0000 to 0xFFFF
 *****************************************************************************/
void SegmentLCD_UnsignedHex(uint16_t value)
{
  int      num, i, com, bit, digit;
  uint16_t bitpattern;

  /* If an update is in progress we must block, or there might be tearing */
  LCD_SyncBusyDelay(0xFFFFFFFF);

  /* Freeze updates to avoid partial refresh of display */
  LCD_FreezeEnable(true);

  /* Turn off all number LCD segments */
  SegmentLCD_NumberOff();

  for (digit = 0; digit < 4; digit++)
  {
    num        = (value >> (4 * digit)) & 0x0f;
    bitpattern = EFM_Numbers[num];
    for (i = 0; i < 7; i++)
    {
      bit = EFM_Display.Number[digit].bit[i];
      com = EFM_Display.Number[digit].com[i];
      if (bitpattern & (1 << i))
      {
        LCD_SegmentSet(com, bit, true);
      }
    }
  }

  /* Sync LCD registers to LE domain */
  LCD_FreezeEnable(false);
}


/**************************************************************************//**
 * @brief Turn all segments on numeric digits in display off
 *****************************************************************************/
void SegmentLCD_NumberOff(void)
{
  /* Turn off all number segments */
  LCD_SegmentSetLow(0, 0x00078000, 0x00000000);
  LCD_SegmentSetLow(1, 0x00078000, 0x00000000);
  LCD_SegmentSetLow(2, 0x00078000, 0x00000000);
  LCD_SegmentSetLow(3, 0x00078000, 0x00000000);
  LCD_SegmentSetLow(4, 0x00078000, 0x00000000);
  LCD_SegmentSetLow(5, 0x00078000, 0x00000000);
  LCD_SegmentSetLow(6, 0x00078000, 0x00000000);
  LCD_SegmentSetLow(7, 0x00078000, 0x00000000);

  return;
}


/**************************************************************************//**
 * @brief Turn all segments on alpha characters in display off
 *****************************************************************************/
void SegmentLCD_AlphaNumberOff(void)
{
  /* Turn off all number segments */
  LCD_SegmentSetLow(7, 0x000075FE, 0x00000000);
  LCD_SegmentSetLow(6, 0x00007FFE, 0x00000000);
  LCD_SegmentSetLow(5, 0x00007FFE, 0x00000000);
  LCD_SegmentSetLow(4, 0x00007FFE, 0x00000000);
  LCD_SegmentSetLow(3, 0x00007FFE, 0x00000000);
  LCD_SegmentSetLow(2, 0x00007FFE, 0x00000000);
  LCD_SegmentSetLow(1, 0x00007FFE, 0x00000000);
  LCD_SegmentSetLow(0, 0x00000A00, 0x00000000);

  return;
}

/**************************************************************************//**
 * @brief Write text on LCD display
 * @param string Text string to show on display
 *****************************************************************************/
void SegmentLCD_Write(char *string)
{
  int      data, length, index;
  uint16_t bitfield;
  uint32_t com, bit;
  int      i;

  length = strlen(string);
  index  = 0;

  /* If an update is in progress we must block, or there might be tearing */
  LCD_SyncBusyDelay(0xFFFFFFFF);

  /* Freeze LCD to avoid partial updates */
  LCD_FreezeEnable(true);

  /* Turn all segments off */
  SegmentLCD_AlphaNumberOff();

  /* Fill out all characters on display */
  for (index = 0; index < 7; index++)
  {
    if (index < length)
    {
      data = (int) *string;
    }
    else           /* Padding with space */
    {
      data = 0x20; /* SPACE */
    }
    /* Defined letters currently starts at "SPACE" - ASCII 0x20; */
    data = data - 0x20;
    /* Get font for this letter */
    bitfield = EFM_Alphabet[data];

    for (i = 0; i < 14; i++)
    {
      bit = EFM_Display.Text[index].bit[i];
      com = EFM_Display.Text[index].com[i];

      if (bitfield & (1 << i))
      {
        /* Turn on segment */
        LCD_SegmentSet(com, bit, true);
      }
    }
    string++;
  }
  /* Enable update */
  LCD_FreezeEnable(false);
}


/**************************************************************************//**
 * @brief Disable all segments
 *****************************************************************************/
void SegmentLCD_AllOff(void)
{
  LCD_SegmentSetLow(0, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(1, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(2, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(3, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(4, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(5, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(6, 0xFFFFFFFF, 0x00000000);
  LCD_SegmentSetLow(7, 0xFFFFFFFF, 0x00000000);
}


/**************************************************************************//**
 * @brief Enable all segments
 *****************************************************************************/
void SegmentLCD_AllOn(void)
{
  LCD_SegmentSetLow(0, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(1, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(2, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(3, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(4, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(5, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(6, 0xFFFFFFFF, 0xFFFFFFFF);
  LCD_SegmentSetLow(7, 0xFFFFFFFF, 0xFFFFFFFF);
}


/**************************************************************************//**
 * @brief Light up or shut off Energy Mode indicator
 * @param em Energy Mode numer 0 to 4
 * @param on Zero is off, non-zero is on
 *****************************************************************************/
void SegmentLCD_EnergyMode(int em, int on)
{
  uint32_t com, bit;

  com = EFM_Display.EMode.com[em];
  bit = EFM_Display.EMode.bit[em];

  if (on)
  {
    LCD_SegmentSet(com, bit, true);
  }
  else
  {
    LCD_SegmentSet(com, bit, false);
  }
}


/**************************************************************************//**
 * @brief Light up or shut off Ring of Indicators
 * @param anum "Segment number" on "Ring", range 0 - 7
 * @param on Zero is off, non-zero is on
 *****************************************************************************/
void SegmentLCD_ARing(int anum, int on)
{
  uint32_t com, bit;

  com = EFM_Display.ARing.com[anum];
  bit = EFM_Display.ARing.bit[anum];

  if (on)
  {
    LCD_SegmentSet(com, bit, true);
  }
  else
  {
    LCD_SegmentSet(com, bit, false);
  }
}


/**************************************************************************//**
 * @brief Light up or shut off various symbols on Segment LCD
 * @param s Which symbol to turn on or off
 * @param on Zero is off, non-zero is on
 *****************************************************************************/
void SegmentLCD_Symbol(lcdSymbol s, int on)
{
  int com = 0;
  int bit = 0;

  switch (s)
  {
  case LCD_SYMBOL_GECKO:
    com = 1; bit = 0;
    break;
  case LCD_SYMBOL_ANT:
    com = 0; bit = 12;
    break;
  case LCD_SYMBOL_PAD0:
    com = 3; bit = 19;
    break;
  case LCD_SYMBOL_PAD1:
    com = 2; bit = 0;
    break;
  case LCD_SYMBOL_EFM32:
    com = 0; bit = 8;
    break;
  case LCD_SYMBOL_MINUS:
    com = 3; bit = 0;
    break;
  case LCD_SYMBOL_COL3:
    com = 4; bit = 0;
    break;
  case LCD_SYMBOL_COL5:
    com = 0; bit = 10;
    break;
  case LCD_SYMBOL_COL10:
    com = 5; bit = 19;
    break;
  case LCD_SYMBOL_DEGC:
    com = 0; bit = 14;
    break;
  case LCD_SYMBOL_DEGF:
    com = 0; bit = 15;
    break;
  case LCD_SYMBOL_DP2:
    com = 7; bit = 0;
    break;
  case LCD_SYMBOL_DP3:
    com = 5; bit = 0;
    break;
  case LCD_SYMBOL_DP4:
    com = 6; bit = 0;
    break;
  case LCD_SYMBOL_DP5:
    com = 7; bit = 9;
    break;
  case LCD_SYMBOL_DP6:
    com = 7; bit = 11;
    break;
  case LCD_SYMBOL_DP10:
    com = 4; bit = 19;
    break;
  }
  if (on)
  {
    LCD_SegmentSet(com, bit, true);
  }
  else
  {
    LCD_SegmentSet(com, bit, false);
  }
}


/**************************************************************************//**
 * @brief Light up or shut off Battery Indicator
 * @param batteryLevel Battery Level, 0 to 4 (0 turns all off)
 *****************************************************************************/
void SegmentLCD_Battery(int batteryLevel)
{
  uint32_t com, bit;
  int      i, on;

  for (i = 0; i < 4; i++)
  {
    if (i < batteryLevel)
    {
      on = 1;
    }
    else
    {
      on = 0;
    }
    com = EFM_Display.Battery.com[i];
    bit = EFM_Display.Battery.bit[i];

    if (on)
    {
      LCD_SegmentSet(com, bit, true);
    }
    else
    {
      LCD_SegmentSet(com, bit, false);
    }
  }
}


/**************************************************************************//**
 * @brief Segment LCD Initialization routine for EFM32 STK display
 * @param useBoost Set to use voltage boost
 *****************************************************************************/
void SegmentLCD_Init(bool useBoost)
{
  const LCD_Init_TypeDef lcdInit = { true,
                                     lcdMuxOctaplex,
                                     lcdBiasOneFourth,
                                     lcdWaveLowPower,
                                     lcdVLCDSelVDD,
                                     lcdConConfVLCD };

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Enable LFRCO as LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);

  /* LCD Controller Prescaler (divide LFACLK / 64) */
  CMU_ClockDivSet(cmuClock_LCDpre, cmuClkDiv_64);
  /* LFACLK_LCDpre = 512 Hz */
  CMU_LCDClkFDIVSet(cmuClkDiv_1);
  /* Set FDIV=0, means 512/1 = 512 Hz */
  /* With octaplex mode, 512/16 => 32 Hz Frame Rate */

  /* Enable clock to LCD module */
  CMU_ClockEnable(cmuClock_LCD, true);

  /* Disable interrupts */
  LCD_IntDisable(0xFFFFFFFF);

  /* Initialize and enable LCD controller */
  LCD_Init(&lcdInit);

  /* Enable all display segments */
  LCD_SegmentRangeEnable(lcdSegmentAll, true);
  /* Enable boost if necessary */
  if (useBoost)
  {
    LCD_VBoostSet(lcdVBoostLevel3);
    LCD_VLCDSelect(lcdVLCDSelVExtBoost);
    CMU->LCDCTRL |= CMU_LCDCTRL_VBOOSTEN;
  }

  /* Turn all segments off */
  SegmentLCD_AllOff();

  LCD_SyncBusyDelay(0xFFFFFFFF);
}


/**************************************************************************//**
 * @brief Disables LCD controller
 *****************************************************************************/
void SegmentLCD_Disable(void)
{
  /* Disable LCD */
  LCD_Enable(false);

  /* Make sure CTRL register has been updated */
  LCD_SyncBusyDelay(LCD_SYNCBUSY_CTRL);

  /* Turn off LCD clock */
  CMU_ClockEnable(cmuClock_LCD, false);

  /* Turn off voltage boost if enabled */
  CMU->LCDCTRL = 0;
}
