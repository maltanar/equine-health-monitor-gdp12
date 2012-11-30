/**************************************************************************//**
 * @file
 * @brief FAT example using FatFS for access to the MicroSD card on the DVK.
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
 *
 ***** MODIFIED FOR EFM32GG STK, SPI MODE, USART2 LOCATION #0
 ***** UNIVERSITY OF SOUTHAMPTON 2012-2013
 ***** EMECS GDP
 *
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "efm32.h"
#include "em_cmu.h"
#include "ff.h"
#include "microsd.h"
#include "diskio.h"
#include "em_chip.h"

/* Ram buffers
 * BUFFERSIZE should be between 512 and 1024, depending on available ram on efm32
 */

#define BUFFERSIZE      1024
//#define BUFFERSIZE      512

/* Filename to open/write/read from SD-card */

#define TEST_FILENAME    "mnoprs.txt"

FIL fsrc;				/* File objects */
FATFS Fatfs;				/* File system specific */
FRESULT res;				/* FatFs function common result code */
UINT br, bw;				/* File read/write count */
DSTATUS resCard;			/* SDcard status */
int8_t ramBufferWrite[BUFFERSIZE];	/* Temporary buffer for write file */
int8_t ramBufferRead[BUFFERSIZE];	/* Temporary buffer for read file */
int8_t StringBuffer[] = "GDP25 - Equine Health Monitor";

/* counts 1ms timeTicks */
volatile uint32_t msTicks;
/* 1000Hz decrement timer, used for timeouts in FAT driver */
extern UINT       Timer1;

/***************************************************************************//**
 * @brief
 *   This function is required by the FAT file system in order to provide
 *   timestamps for created files. Since we do not have a reliable clock we
 *   hardcode a value here.
 *
 *   Refer to drivers/fatfs/doc/en/fattime.html for the format of this DWORD.
 * @return
 *    A DWORD containing the current time and date as a packed datastructure.
 ******************************************************************************/
DWORD get_fattime(void)
{
  return (28 << 25) | (2 << 21) | (1 << 16);
}


/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter.
 *****************************************************************************/
void SysTick_Handler(void)
{
  /* Increment counter necessary in Delay()*/
  msTicks++;
  /* Timer1 is used for timeout purposes in various FatFS operatios */
  if (Timer1 > 0)
    Timer1--;
}


/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}


//void setupSWO(void)
//{
//  uint32_t *dwt_ctrl = (uint32_t *) 0xE0001000;
//  uint32_t *tpiu_prescaler = (uint32_t *) 0xE0040010;
//  uint32_t *tpiu_protocol = (uint32_t *) 0xE00400F0;
//
//  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
//  /* Enable Serial wire output pin */
//  GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;
//#if defined(_EFM32_GIANT_FAMILY)
//  /* Set location 0 */
//  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;
//
//  /* Enable output on pin - GPIO Port F, Pin 2 */
//  GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
//  GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;
//#else
//  /* Set location 1 */
//  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC1;
//  /* Enable output on pin */
//  GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
//  GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
//#endif
//  /* Enable debug clock AUXHFRCO */
//  CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;
//
//  while(!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));
//
//  /* Enable trace in core debug */
//  CoreDebug->DHCSR |= 1;
//  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
//
//  /* Enable PC and IRQ sampling output */
//  *dwt_ctrl = 0x400113FF;
//  /* Set TPIU prescaler to 16. */
//  *tpiu_prescaler = 0xf;
//  /* Set protocol to NRZ */
//  *tpiu_protocol = 2;
//  /* Unlock ITM and output data */
//  ITM->LAR = 0xC5ACCE55;
//  ITM->TCR = 0x10009;
//}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  CHIP_Init();
  int16_t i;
  int16_t filecounter;

  /* Use 32MHZ HFXO as core clock frequency */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

  //setupSWO();

 /* Setup SysTick Timer for 10 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 100))
  {
    while (1) ;
  }

  /*Step1*/
  /*Initialization file buffer write */
  filecounter = sizeof(StringBuffer);

  for(i = 0; i < filecounter ; i++)
  {
     ramBufferWrite[i] = StringBuffer[i];
  }
  
  /*Step2*/
  /*Detect micro-SD*/
  while(1)
  {
    MICROSD_init();
    resCard = disk_initialize(0);       /*Check micro-SD card status */
    
    switch(resCard)
    {
    case STA_NOINIT:                    /* Drive not initialized */
      break;
    case STA_NODISK:                    /* No medium in the drive */
      break;
    case STA_PROTECT:                   /* Write protected */
      break;
    default:
      break;
    }
   
    if (!resCard) break;                /* Drive initialized. */
 
    Delay(1);    
  }

  /*Step3*/
  /* Initialize filesystem */
  if (f_mount(0, &Fatfs) != FR_OK)
  {
    /* Error.No micro-SD with FAT32 is present */
    while(1);
  }
   
  /*Step4*/
  /* Open  the file for write */
   res = f_open(&fsrc, TEST_FILENAME,  FA_WRITE); 
   if (res != FR_OK)
   {
     /*  If file does not exist create it*/ 
     res = f_open(&fsrc, TEST_FILENAME, FA_CREATE_ALWAYS | FA_WRITE ); 
      if (res != FR_OK) 
     {
      /* Error. Cannot create the file */
      while(1);
    }
   }
   
  /*Step5*/
  /*Set the file write pointer to first location */ 
  res = f_lseek(&fsrc, 0);
   if (res != FR_OK) 
  {
    /* Error. Cannot set the file write pointer */
    while(1);
  }
  
/*Step6*/
  /*Write a buffer to file*/
   res = f_write(&fsrc, ramBufferWrite, filecounter, &bw);
   if ((res != FR_OK) || (filecounter != bw)) 
  {
    /* Error. Cannot write the file */
    while(1);
  }
   
  /*Step7*/
  /* Close the file */
  f_close(&fsrc);
  if (res != FR_OK) 
  {
    /* Error. Cannot close the file */
    while(1);
  }
  
  /*Step8*/
  /* Open the file for read */
  res = f_open(&fsrc, TEST_FILENAME,  FA_READ); 
   if (res != FR_OK) 
  {
    /* Error. Cannot create the file */
    while(1);
  }

   /*Step9*/
   /*Set the file read pointer to first location */ 
   res = f_lseek(&fsrc, 0);
   if (res != FR_OK) 
  {
    /* Error. Cannot set the file pointer */
    while(1);
  }
  
  /*Step10*/
  /* Read some data from file */
  res = f_read(&fsrc, ramBufferRead, filecounter, &br);
   if ((res != FR_OK) || (filecounter != br)) 
  {
    /* Error. Cannot read the file */
    while(1);
  }
	
  /*Step11*/
  /* Close the file */
  f_close(&fsrc);
  if (res != FR_OK) 
  {
    /* Error. Cannot close the file */
    while(1);
  }
  
  /*Step12*/
  /*Compare ramBufferWrite and ramBufferRead */
  for(i = 0; i < filecounter ; i++)
  {
    if ((ramBufferWrite[i]) != (ramBufferRead[i]))
    {
      /* Error compare buffers*/
      while(1);
    }
  }

/*Set here a breakpoint*/
  /*If the breakpoint is trap here then write and read functions were passed */
  while (1)
  {
  }
}
