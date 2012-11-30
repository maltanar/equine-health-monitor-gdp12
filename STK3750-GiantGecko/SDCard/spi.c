/**************************************************************************//**
 * @file
 * @brief SPI
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
#include "spi.h"
#include "em_gpio.h"
#include "spi_project.h"


/**************************************************************************//**
 * @brief Setup a USART as SPI
 * @param spiNumber is the number of the USART to use (e.g. 1 USART1)
 * @param location is the GPIO location to use for the device
 * @param master set if the SPI is to be master
 *****************************************************************************/
void SPI_setup(uint8_t spiNumber, uint8_t location, bool master)
{
  USART_TypeDef *spi;

  /* Determining USART */
  switch (spiNumber)
  {
  case 0:
    spi = USART0;
    break;
  case 1:
    spi = USART1;
    break;
  default:
    return;
  }

  /* Setting baudrate */
  spi->CLKDIV = 128 * (SPI_PERCLK_FREQUENCY / SPI_BAUDRATE - 2);

  /* Configure SPI */
  /* Using synchronous (SPI) mode*/
  spi->CTRL = USART_CTRL_SYNC;
  /* Clearing old transfers/receptions, and disabling interrupts */
  spi->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
  spi->IEN = 0;
  
  /* Enabling pins and setting location */
//jc
  //  spi->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN | USART_ROUTE_CSPEN | (location << 8);
  spi->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN | (location << 8);

//jc
  //Setting frame size to 16 bits
  spi->FRAME &= ~USART_FRAME_DATABITS_DEFAULT;
  spi->FRAME |= USART_FRAME_DATABITS_SIXTEEN;
  //SET clock polarity and phase
  spi->CTRL |= USART_CTRL_CLKPOL_IDLEHIGH | USART_CTRL_CLKPHA_SAMPLETRAILING; 
  //set Transmitter as Tri-stated
  spi->CMD = USART_CMD_TXTRIEN;
  
  
  /* Set GPIO config to slave */
  GPIO_Mode_TypeDef gpioModeMosi = gpioModeInput;
  GPIO_Mode_TypeDef gpioModeMiso = gpioModePushPull;
  GPIO_Mode_TypeDef gpioModeCs   = gpioModeInput;
  GPIO_Mode_TypeDef gpioModeClk  = gpioModeInput;
  
  /* Set to master and to control the CS line */
  if (master)
  {
    /* Enabling Master, TX and RX */
    spi->CMD   = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
//jc NO AUTOCS
//    spi->CTRL |= USART_CTRL_AUTOCS;
    spi->CTRL |= USART_CTRL_MSBF;

    
    /* Set GPIO config to master */
    gpioModeMosi = gpioModePushPull;
    gpioModeMiso = gpioModeInput;
    gpioModeCs   = gpioModePushPull;
    gpioModeClk  = gpioModePushPull;
  }
  else
  {
    /* Enabling TX and RX */
    spi->CMD = USART_CMD_TXEN | USART_CMD_RXEN;
  }

  /* Clear previous interrupts */
  spi->IFC = _USART_IFC_MASK;

  /* IO configuration */
  switch(spiNumber)
  {
    case 0: switch(location)
            {
              case 0: /* IO configuration (USART 0, Location #0) */
                      GPIO_PinModeSet(gpioPortE, 10, gpioModeMosi, 0); /* MOSI */
                      GPIO_PinModeSet(gpioPortE, 11, gpioModeMiso, 0); /* MISO */
                      GPIO_PinModeSet(gpioPortE, 13, gpioModeCs,   0); /* CS */
                      GPIO_PinModeSet(gpioPortE, 12, gpioModeClk,  0); /* Clock */
                      break;
              case 1: /* IO configuration (USART 0, Location #1) */
                      GPIO_PinModeSet(gpioPortE, 7, gpioModeMosi, 0);  /* MOSI */ 
                      GPIO_PinModeSet(gpioPortE, 6, gpioModeMiso, 0);  /* MISO */
                      GPIO_PinModeSet(gpioPortE, 4, gpioModeCs,   0);  /* CS */
                      GPIO_PinModeSet(gpioPortE, 5, gpioModeClk,  0);  /* Clock */
                      break;
              case 2: /* IO configuration (USART 0, Location #2) */
                      GPIO_PinModeSet(gpioPortC, 11, gpioModeMosi, 0); /* MOSI */
                      GPIO_PinModeSet(gpioPortC, 10, gpioModeMiso, 0); /* MISO */
                      GPIO_PinModeSet(gpioPortC,  8, gpioModeCs,   0); /* CS */
                      GPIO_PinModeSet(gpioPortC,  9, gpioModeClk,  0); /* Clock */
                      break;
              default: break;
            }
            break;
    case 1: switch(location)
            {
              case 0: /* IO configuration (USART 1, Location #0) */
                      GPIO_PinModeSet(gpioPortC, 0, gpioModeMosi, 0);  /* MOSI */
                      GPIO_PinModeSet(gpioPortC, 1, gpioModeMiso, 0);  /* MISO */
                      GPIO_PinModeSet(gpioPortB, 8, gpioModeCs,   0);  /* CS */
                      GPIO_PinModeSet(gpioPortB, 7, gpioModeClk,  0);  /* Clock */
                      break;
              case 1: /* IO configuration (USART 1, Location #1) */
                      GPIO_PinModeSet(gpioPortD, 0, gpioModeMosi, 0);  /* MOSI */
                      GPIO_PinModeSet(gpioPortD, 1, gpioModeMiso, 0);  /* MISO */
                      GPIO_PinModeSet(gpioPortD, 3, gpioModeCs,   0);  /* CS */
                      GPIO_PinModeSet(gpioPortD, 2, gpioModeClk,  0);  /* Clock */
                      break;              
              default: break;
            }
    default: break;  
  }
}

/**
 * Internal spi handling, works on both avr tiny, with USI,
 * and also regular hardware SPI.
 *
 * For regular hardware spi, requires the spi hardware to already be setup!
 * (TODO, you can handle that yourself, or even, have a compile time flag that
 * determines whether to use internal, or provided spi_tx/rx routines)
 */
//uint8_t spi_tx8bit(uint8_t cData) 
//{
//  return USART1_sendChar(cData);
//}
/*
uint16_t spi1_tx16(uint16_t cData)
{
  return USART1_send_word16(cData);
}

uint16_t spi1_get_word16(uint16_t sData) {

    GPIO_PinOutClear(gpioPortD, 3);
    uint16_t res = spi1_tx16(0);
    USART1_wait_write_complete();
    GPIO_PinOutSet(gpioPortD, 3);
    return res;
}

*/






//** IRQ handlers
/* Buffer pointers and indexes */
//uint16_t* masterRxBuffer;
//int masterRxBufferSize;
//volatile int masterRxBufferIndex;

/**************************************************************************//**
 * @brief USART1 RX IRQ Handler Setup
 * @param receiveBuffer points to where to place recieved data
 * @param receiveBufferSize indicates the number of bytes to receive
 *****************************************************************************/
//void SPI1_setupRXInt(uint16_t* receiveBuffer, int receiveBufferSize)
//{
//  USART_TypeDef *spi = USART1;

//  /* Setting up pointer and indexes */
//  masterRxBuffer      = receiveBuffer;
//  masterRxBufferSize  = receiveBufferSize;
//  masterRxBufferIndex = 0;

//  /* Clear RX */
//  spi->CMD = USART_CMD_CLEARRX;

//  /* Enable interrupts */
//  NVIC_ClearPendingIRQ(USART1_RX_IRQn);
//  NVIC_EnableIRQ(USART1_RX_IRQn);
//  spi->IEN |= USART_IEN_RXDATAV;
//}


/**************************************************************************//**
 * @brief USART1 RX IRQ Handler
 *****************************************************************************/
//void USART1_RX_IRQHandler(void)
//{
//  USART_TypeDef *spi = USART1;
//  uint16_t       rxdata;
//
//  if (spi->STATUS & USART_STATUS_RXDATAV)
//  {
//    /* Reading out data */
//    rxdata = spi->RXDOUBLE;
//
//    if (masterRxBufferIndex < masterRxBufferSize)
//    {
//      /* Store Data */
//      masterRxBuffer[masterRxBufferIndex] = rxdata;
//      masterRxBufferIndex++;
//    }
//  }
//}
