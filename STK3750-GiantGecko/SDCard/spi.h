/**************************************************************************//**
 * @file
 * @brief SPI prototypes and definitions
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
#ifndef __SPI_H
#define __SPI_H

#include <stdbool.h>
#include "efm32.h"


void SPI_setup(uint8_t spiNumber, uint8_t location, bool master);

//void SPI1_setupRXInt(uint16_t* receiveBuffer, int receiveBufferSize);

//jacc
uint16_t spi1_tx16(uint16_t cData);
uint16_t spi1_get_word16(uint16_t sData);

void SPI1_getBuffer(uint16_t* RxBuffer, int bufferSize);

#endif
