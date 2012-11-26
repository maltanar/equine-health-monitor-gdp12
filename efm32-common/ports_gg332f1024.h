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
// PA1 is connected to INT1 pin of ADXL
#define GPIO_ADXL_INT1_PORT 	gpioPortA 
#define GPIO_ADXL_INT1_PIN 		1
#define GPIO_ADXL_INT1			GPIO_ADXL_INT1_PORT, GPIO_ADXL_INT1_PIN
// PA0 is connected to INT2 pin of ADXL
#define GPIO_ADXL_INT2_PORT 	gpioPortA 
#define GPIO_ADXL_INT2_PIN 		0
#define GPIO_ADXL_INT2			GPIO_ADXL_INT2_PORT, GPIO_ADXL_INT2_PIN
// PE9 is connected to DRDY' pin of TMP006
#define GPIO_TMP006_DRDY_PORT 	gpioPortE 
#define GPIO_TMP006_DRDY_PIN 	9
#define GPIO_TMP006_DRDY		GPIO_TMP006_DRDY_PORT, GPIO_TMP006_DRDY_PIN
// PA10 is connected to the transistor that turns on/off the ANT Vcc
#define GPIO_ANT_VCC_PORT 	gpioPortA 
#define GPIO_ANT_VCC_PIN 	10
#define GPIO_ANT_VCC		GPIO_ANT_VCC_PORT, GPIO_ANT_VCC_PIN
// PA4 is connected to the sleep pin of the ANT
#define GPIO_ANT_SLEEP_PORT 	gpioPortA 
#define GPIO_ANT_SLEEP_PIN 		4
#define GPIO_ANT_SLEEP			GPIO_ANT_SLEEP_PORT, GPIO_ANT_SLEEP_PIN
// PA2 is connected to the RTS/SEN pin of the ANT
#define GPIO_ANT_RTS_PORT 	gpioPortA 
#define GPIO_ANT_RTS_PIN 	2
#define GPIO_ANT_RTS		GPIO_ANT_RTS_PORT, GPIO_ANT_RTS_PIN
// What about ANT GLBL_RST?
// Nothing with microSD
// PA8 is connected to the DTR'/SLEEP_RQ pin of the XBEE
#define GPIO_XBEE_DTR_PORT 	gpioPortA 
#define GPIO_XBEE_DTR_PIN 	8
#define GPIO_XBEE_DTR		GPIO_XBEE_DTR_PORT, GPIO_XBEE_DTR_PIN
// PA9 is connected to the transistor that turns on/off the XBEE Vcc
#define GPIO_XBEE_VCC_PORT 	gpioPortA 
#define GPIO_XBEE_VCC_PIN 	9
#define GPIO_XBEE_VCC		GPIO_XBEE_VCC_PORT, GPIO_XBEE_VCC_PIN
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