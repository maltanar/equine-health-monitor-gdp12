/**************************************************************************//**
 * @file
 * @brief USART example
 * @author Energy Micro AS
 * @version 1.10
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
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
#include "usart.h"
#include "em_gpio.h"

/*
void USART1_wait_write_complete()
{
  while (!(USART1->STATUS & USART_STATUS_TXC)) ;
}
*/
/*
char USART1_sendChar(char c)
{
  USART_TypeDef *uart = USART1;

  while (!(uart->STATUS & USART_STATUS_TXBL)) ;
  
  uart->TXDATA = c;
  
  while(!(uart->STATUS & USART_STATUS_RXDATAV)) ;
  
  char r = uart->RXDATA;
  //printf("SPI sent %x got %x \n", c, r);
  
  // Reading out data
  return r;
}
*/


void USART1_sendBuffer(uint16_t* txBuffer, int bufferSize)
{
  USART_TypeDef *uart = USART1;
  int           ii;

  /* Sending the data */
  for (ii = 0; ii < bufferSize;  ii++)
  {

    /* Waiting for the usart to be ready */
    while (!(uart->STATUS & USART_STATUS_TXBL)) ;
//jacc!! The cpu gets stuck here!
   
    if (txBuffer != 0)
    {
      /* Writing next byte to USART */
      uart->TXDOUBLE = *txBuffer;
      txBuffer++;
    }
    else
    {
      uart->TXDOUBLE = 0;
    }

      /*Waiting for transmission of last byte */
      while (!(uart->STATUS & USART_STATUS_TXC)) ;
  }

}


/********************************************************




















/********************************************************
//jc*/

void USART1_getBuffer(uint16_t* RxBuffer, int bufferSize) {
  
  USART_TypeDef *uart = USART1;
  int           ii;
  
  /* Sending the data */
  for (ii = 0; ii < bufferSize;  ii++)
  {

    /* Waiting for the usart to be ready */
    while (!(uart->STATUS & USART_STATUS_TXBL));
   
    GPIO_PinOutClear(gpioPortD, 3);
    
    //Send dummy data
    uart->TXDOUBLE = 0;
    
    //wait for received data to be available
    while(!(uart->STATUS & USART_STATUS_RXDATAV)) ;
 
    //Store received date in the buffer
    RxBuffer[ii] = uart->RXDOUBLE;
    //printf("SPI sent %x got %x \n", c, r);

   /*Waiting for transmission of last byte */
    while (!(uart->STATUS & USART_STATUS_TXC)) ;

    GPIO_PinOutSet(gpioPortD, 3);
  }
}


