// University of Southampton, 2012
// EMECS Group Design Project

// Port configurations for the Tiny Gecko used in the TG STK
// (part number TG840F32)
// automatically included by port_config.h

#ifndef __PORTS_TG840F32_H
#define __PORTS_TG840F32_H

#include "uartport.h"
#include "i2cbus.h"

// -------------------------------------------------------------------------
// Section: I2C port config ------------------------------------------------
// -------------------------------------------------------------------------
const I2CPortConfig I2CPortConf =
{
  // port configuration for I2C0, pos3
  .i2cTypeDef = I2C0,
  .i2cLocation = 1,
  .irqNumber = I2C0_IRQn,
  .sclPort = gpioPortD,
  .sclPin = 7,
  .sdaPort = gpioPortD,
  .sdaPin = 6
 };
// -------------------------------------------------------------------------
// End Section: I2C port config --------------------------------------------
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// Section: USART and LEUART port config -----------------------------------
// -------------------------------------------------------------------------

#define UART_MANAGER_PORT_COUNT         2

const UARTPortConfig UARTManagerPortConfigs[UART_MANAGER_PORT_COUNT] =
{
  // port configuration for LEUART0, location 0
  {
    .clockPoint = cmuClock_LEUART0,
    .irqNumber = LEUART0_IRQn,
    .usartBase = (USART_TypeDef *) LEUART0,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = 0,
    .txPort = gpioPortD,
    .txPin = 4,
    .rxPort = gpioPortD,
    .rxPin = 5,
    .lowEnergy = true
  },
  // port configuration for USART1, location 1
  {
    .clockPoint = cmuClock_USART1 ,
    .irqNumber = USART1_RX_IRQn,
    .usartBase = (USART_TypeDef *) USART1,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = 1,
    .txPort = gpioPortD,
    .txPin = 0,
    .rxPort = gpioPortD,
    .rxPin = 1,
    .lowEnergy = false
  }
};

typedef enum {
  UARTManagerPortLEUART0 = 0,
  UARTManagerPortUSART1 = 1
} UARTManagerPort;

// -------------------------------------------------------------------------
// End Section: USART and LEUART port config -------------------------------
// -------------------------------------------------------------------------

#endif	// __PORTS_TG840F32_H