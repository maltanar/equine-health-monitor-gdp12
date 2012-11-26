// University of Southampton, 2012
// EMECS Group Design Project

// Port configurations for the Giant Gecko used in the GDP PCB
// (part number GG332F1024)
// automatically included by port_config.h

#ifndef __PORTS_GG332F1024_H
#define __PORTS_GG332F1024_H

#include "uartport.h"
#include "i2cbus.h"

// -------------------------------------------------------------------------
// Section: Misc GPIO port config ------------------------------------------
// -------------------------------------------------------------------------
// TODO add #define statements here for each GPIO we use in the PCB
// by looking at the schematic
// example :
// PE8 controls the transistor that turns on/off the GPS Vcc
#define	GPIO_GPS_VCC_PORT		gpioPortE
#define	GPIO_GPS_VCC_PIN		8
#define GPIO_GPS_VCC			GPIO_GPS_VCC_PORT, GPIO_GPS_VCC_PIN
// PA5 controls the transistor that turns on/off the GPS Vbat
#define	GPIO_GPS_VBAT_PORT		gpioPortA
#define	GPIO_GPS_VBAT_PIN		5
#define GPIO_GPS_VBAT			GPIO_GPS_VBAT_PORT, GPIO_GPS_VBAT_PIN
// TODO Merve will continue with the other ports and pins
// -------------------------------------------------------------------------
// End Section: Misc GPIO port config --------------------------------------
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
// Section: I2C port config ------------------------------------------------
// -------------------------------------------------------------------------
const I2CPortConfig I2CPortConf = 
{
  // port configuration for I2C0, pos 6
  .i2cTypeDef = I2C0,
  .i2cLocation = 6,
  .irqNumber = I2C0_IRQn,
  .sclPort = gpioPortE,
  .sclPin = 13,
  .sdaPort = gpioPortE,
  .sdaPin = 12
 };
// -------------------------------------------------------------------------
// End Section: I2C port config --------------------------------------------
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// Section: USART and LEUART port config -----------------------------------
// -------------------------------------------------------------------------

#define UART_MANAGER_PORT_COUNT         5

const UARTPortConfig UARTManagerPortConfigs[UART_MANAGER_PORT_COUNT] =
{
  // port configuration for LEUART0, location 2
  {
    .clockPoint = cmuClock_LEUART0,
    .irqNumber = LEUART0_IRQn,
    .usartBase = (USART_TypeDef *) LEUART0,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = LEUART_ROUTE_LOCATION_LOC2,
    .txPort = gpioPortE,
    .txPin = 14,
    .rxPort = gpioPortE,
    .rxPin = 15,
    .lowEnergy = true
  },
  // port configuration for LEUART1, location 0
  {
    .clockPoint = cmuClock_LEUART1,
    .irqNumber = LEUART1_IRQn,
    .usartBase = (USART_TypeDef *) LEUART1,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = LEUART_ROUTE_LOCATION_LOC0,
    .txPort = gpioPortC,
    .txPin = 6,
    .rxPort = gpioPortC,
    .rxPin = 7,
    .lowEnergy = true
  },
  // port configuration for USART0, location 2
  {
    .clockPoint = cmuClock_USART0 ,
    .irqNumber = USART0_RX_IRQn,
    .usartBase = (USART_TypeDef *) USART0,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = USART_ROUTE_LOCATION_LOC2,
    .txPort = gpioPortC,
    .txPin = 11,
    .rxPort = gpioPortC,
    .rxPin = 10,
	//.async = true,
    .lowEnergy = false
  },
  // port configuration for USART1, location 1
  {
    .clockPoint = cmuClock_USART1 ,
    .irqNumber = USART1_RX_IRQn,
    .usartBase = (USART_TypeDef *) USART1,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = USART_ROUTE_LOCATION_LOC1,
    .txPort = gpioPortD,
    .txPin = 0,
    .rxPort = gpioPortD,
    .rxPin = 1,
	//.async = false,
    .lowEnergy = false
  },
  // port configuration for USART2, location 0
  {
    .clockPoint = cmuClock_USART2,
    .irqNumber = USART2_RX_IRQn,
    .usartBase = (USART_TypeDef *) USART2,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = USART_ROUTE_LOCATION_LOC0,
    .txPort = gpioPortC,
    .txPin = 11,
    .rxPort = gpioPortC,
    .rxPin = 12,
	//.async = true,
    .lowEnergy = false
  }
};

typedef enum {
  UARTManagerPortLEUART0	= 0,
  UARTManagerPortLEUART1	= 1,
  UARTManagerPortUSART0		= 2,
  UARTManagerPortUSART1		= 3,
  UARTManagerPortUSART2		= 4,
} UARTManagerPort;

// -------------------------------------------------------------------------
// End Section: USART and LEUART port config -------------------------------
// -------------------------------------------------------------------------

#endif	// __PORTS_GG332F1024_H