/**************************************************************************//**
 * @file
 * @brief Segment LCD Font And Layout for the EFM32TG_STK3300 starter kit
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
#include "segmentfont.h"

/**************************************************************************//**
 * @brief Working instance of LCD display
 *****************************************************************************/
const MCU_DISPLAY EFM_Display = {
  .Text        = {
    { /* 1 */
      .com[0] = 1, .com[1] = 1, .com[2] = 5, .com[3] = 7,
      .bit[0] = 1, .bit[1] = 2, .bit[2] = 2, .bit[3] = 2,

      .com[4] = 7, .com[5] = 3, .com[6] = 4, .com[7] = 2,
      .bit[4] = 1, .bit[5] = 1, .bit[6] = 1, .bit[7] = 1,

      .com[8] = 3, .com[9] = 2, .com[10] = 4, .com[11] = 6,
      .bit[8] = 2, .bit[9] = 2, .bit[10] = 2, .bit[11] = 2,

      .com[12] = 5, .com[13] = 6,
      .bit[12] = 1, .bit[13] = 1
    },
    { /* 2 */
      .com[0] = 1, .com[1] = 1, .com[2] = 5, .com[3] = 7,
      .bit[0] = 3, .bit[1] = 4, .bit[2] = 4, .bit[3] = 4,

      .com[4] = 7, .com[5] = 3, .com[6] = 4, .com[7] = 2,
      .bit[4] = 3, .bit[5] = 3, .bit[6] = 3, .bit[7] = 3,

      .com[8] = 3, .com[9] = 2, .com[10] = 4, .com[11] = 6,
      .bit[8] = 4, .bit[9] = 4, .bit[10] = 4, .bit[11] = 4,

      .com[12] = 5, .com[13] = 6,
      .bit[12] = 3, .bit[13] = 3
    },
    { /* 3 */
      .com[0] = 1, .com[1] = 1, .com[2] = 5, .com[3] = 7,
      .bit[0] = 5, .bit[1] = 6, .bit[2] = 6, .bit[3] = 6,

      .com[4] = 7, .com[5] = 3, .com[6] = 4, .com[7] = 2,
      .bit[4] = 5, .bit[5] = 5, .bit[6] = 5, .bit[7] = 5,

      .com[8] = 3, .com[9] = 2, .com[10] = 4, .com[11] = 6,
      .bit[8] = 6, .bit[9] = 6, .bit[10] = 6, .bit[11] = 6,

      .com[12] = 5, .com[13] = 6,
      .bit[12] = 5, .bit[13] = 5
    },
    { /* 4 */
      .com[0] = 1, .com[1] = 1, .com[2] = 5, .com[3] = 7,
      .bit[0] = 7, .bit[1] = 8, .bit[2] = 8, .bit[3] = 8,

      .com[4] = 7, .com[5] = 3, .com[6] = 4, .com[7] = 2,
      .bit[4] = 7, .bit[5] = 7, .bit[6] = 7, .bit[7] = 7,

      .com[8] = 3, .com[9] = 2, .com[10] = 4, .com[11] = 6,
      .bit[8] = 8, .bit[9] = 8, .bit[10] = 8, .bit[11] = 8,

      .com[12] = 5, .com[13] = 6,
      .bit[12] = 7, .bit[13] = 7
    },
    { /* 5 */
      .com[0] = 0, .com[1] = 1, .com[2] = 5, .com[3] = 7,
      .bit[0] = 9, .bit[1] = 10, .bit[2] = 10, .bit[3] = 10,

      .com[4] = 6, .com[5] = 2, .com[6] = 3, .com[7] = 1,
      .bit[4] = 9, .bit[5] = 9, .bit[6] = 9, .bit[7] = 9,

      .com[8] = 3, .com[9] = 2, .com[10] = 4, .com[11] = 6,
      .bit[8] = 10, .bit[9] = 10, .bit[10] = 10, .bit[11] = 10,

      .com[12] = 4, .com[13] = 5,
      .bit[12] = 9, .bit[13] = 9
    },
    { /* 6 */
      .com[0] = 0, .com[1] = 1, .com[2] = 5, .com[3] = 7,
      .bit[0] = 11, .bit[1] = 12, .bit[2] = 12, .bit[3] = 12,

      .com[4] = 6, .com[5] = 2, .com[6] = 3, .com[7] = 1,
      .bit[4] = 11, .bit[5] = 11, .bit[6] = 11, .bit[7] = 11,

      .com[8] = 3, .com[9] = 2, .com[10] = 4, .com[11] = 6,
      .bit[8] = 12, .bit[9] = 12, .bit[10] = 12, .bit[11] = 12,

      .com[12] = 4, .com[13] = 5,
      .bit[12] = 11, .bit[13] = 11
    },
    { /* 7 */
      .com[0] = 1, .com[1] = 1, .com[2] = 5, .com[3] = 7,
      .bit[0] = 13, .bit[1] = 14, .bit[2] = 14, .bit[3] = 14,

      .com[4] = 7, .com[5] = 3, .com[6] = 4, .com[7] = 2,
      .bit[4] = 13, .bit[5] = 13, .bit[6] = 13, .bit[7] = 13,

      .com[8] = 3, .com[9] = 2, .com[10] = 4, .com[11] = 6,
      .bit[8] = 14, .bit[9] = 14, .bit[10] = 14, .bit[11] = 14,

      .com[12] = 5, .com[13] = 6,
      .bit[12] = 13, .bit[13] = 13
    },
  },
  .Number      = {
    {
      .com[0] = 7, .com[1] = 5, .com[2] = 2, .com[3] = 1,
      .bit[0] = 15, .bit[1] = 15, .bit[2] = 15, .bit[3] = 15,

      .com[4] = 3, .com[5] = 6, .com[6] = 4,
      .bit[4] = 15, .bit[5] = 15, .bit[6] = 15,
    },
    {
      .com[0] = 7, .com[1] = 5, .com[2] = 2, .com[3] = 1,
      .bit[0] = 16, .bit[1] = 16, .bit[2] = 16, .bit[3] = 16,

      .com[4] = 3, .com[5] = 6, .com[6] = 4,
      .bit[4] = 16, .bit[5] = 16, .bit[6] = 16,
    },
    {
      .com[0] = 7, .com[1] = 5, .com[2] = 2, .com[3] = 1,
      .bit[0] = 17, .bit[1] = 17, .bit[2] = 17, .bit[3] = 17,

      .com[4] = 3, .com[5] = 6, .com[6] = 4,
      .bit[4] = 17, .bit[5] = 17, .bit[6] = 17,
    },
    {
      .com[0] = 7, .com[1] = 5, .com[2] = 2, .com[3] = 1,
      .bit[0] = 18, .bit[1] = 18, .bit[2] = 18, .bit[3] = 18,

      .com[4] = 3, .com[5] = 6, .com[6] = 4,
      .bit[4] = 18, .bit[5] = 18, .bit[6] = 18,
    },
  },
  .EMode       = {
    .com[0] = 0, .bit[0] = 19,
    .com[1] = 1, .bit[1] = 19,
    .com[2] = 7, .bit[2] = 19,
    .com[3] = 2, .bit[3] = 19,
    .com[4] = 6, .bit[4] = 19,
  },
  .ARing       = {
    .com[0] = 0, .bit[0] = 7,
    .com[1] = 0, .bit[1] = 6,
    .com[2] = 0, .bit[2] = 5,
    .com[3] = 0, .bit[3] = 4,

    .com[4] = 0, .bit[4] = 3,
    .com[5] = 0, .bit[5] = 2,
    .com[6] = 0, .bit[6] = 1,
    .com[7] = 0, .bit[7] = 0,
  },
  .Battery     = {
    .com[0] = 0, .bit[0] = 13,
    .com[1] = 0, .bit[1] = 17,
    .com[2] = 0, .bit[2] = 16,
    .com[3] = 0, .bit[3] = 18,
  }
};

/**************************************************************************//**
 * @brief
 * Defines higlighted segments for the alphabet, starting from "blank" (SPACE)
 * Uses bit pattern as defined for text segments above.
 * E.g. a capital O, would have bits 0 1 2 3 4 5 => 0x003f defined
 *****************************************************************************/
const uint16_t EFM_Alphabet[] = {
  0x0000, /* space */
  0x1100, /* ! */
  0x0280, /* " */
  0x0000, /* # */
  0x0000, /* $ */
  0x0602, /* % */
  0x0000, /* & */
  0x0020, /* ' */
  0x0039, /* ( */
  0x000f, /* ) */
  0x0000, /* * */
  0x1540, /* + */
  0x2000, /* , */
  0x0440, /* - */
  0x1000, /* . */
  0x2200, /* / */

  0x003f, /* 0 */
  0x0006, /* 1 */
  0x045b, /* 2 */
  0x044f, /* 3 */
  0x0466, /* 4 */
  0x046d, /* 5 */
  0x047d, /* 6 */
  0x0007, /* 7 */
  0x047f, /* 8 */
  0x046f, /* 9 */

  0x0000, /* : */
  0x0000, /* ; */
  0x0a00, /* < */
  0x0000, /* = */
  0x2080, /* > */
  0x0000, /* ? */
  0xffff, /* @ */

  0x0477, /* A */
  0x0a79, /* B */
  0x0039, /* C */
  0x20b0, /* D */
  0x0079, /* E */
  0x0071, /* F */
  0x047d, /* G */
  0x0476, /* H */
  0x0006, /* I */
  0x000e, /* J */
  0x0a70, /* K */
  0x0038, /* L */
  0x02b6, /* M */
  0x08b6, /* N */
  0x003f, /* O */
  0x0473, /* P */
  0x083f, /* Q */
  0x0c73, /* R */
  0x046d, /* S */
  0x1101, /* T */
  0x003e, /* U */
  0x2230, /* V */
  0x2836, /* W */
  0x2a80, /* X */
  0x046e, /* Y */
  0x2209, /* Z */

  0x0039, /* [ */
  0x0880, /* backslash */
  0x000f, /* ] */
  0x0001, /* ^ */
  0x0008, /* _ */
  0x0100, /* ` */

  0x1058, /* a */
  0x047c, /* b */
  0x0058, /* c */
  0x045e, /* d */
  0x2058, /* e */
  0x0471, /* f */
  0x0c0c, /* g */
  0x0474, /* h */
  0x0004, /* i */
  0x000e, /* j */
  0x0c70, /* k */
  0x0038, /* l */
  0x1454, /* m */
  0x0454, /* n */
  0x045c, /* o */
  0x0473, /* p */
  0x0467, /* q */
  0x0450, /* r */
  0x0c08, /* s */
  0x0078, /* t */
  0x001c, /* u */
  0x2010, /* v */
  0x2814, /* w */
  0x2a80, /* x */
  0x080c, /* y */
  0x2048, /* z */

  0x0000,
};

/**************************************************************************//**
 * @brief
 * Defines higlighted segments for the numeric display
 *****************************************************************************/
const uint16_t EFM_Numbers[] =
{
  0x003f, /* 0 */
  0x0006, /* 1 */
  0x005b, /* 2 */
  0x004f, /* 3 */
  0x0066, /* 4 */
  0x006d, /* 5 */
  0x007d, /* 6 */
  0x0007, /* 7 */
  0x007f, /* 8 */
  0x006f, /* 9 */
  0x0077, /* A */
  0x007c, /* b */
  0x0027, /* C */
  0x005e, /* d */
  0x0079, /* E */
  0x0071, /* F */
  0x0040, /* - */
};
