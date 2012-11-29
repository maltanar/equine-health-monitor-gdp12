// University of Southampton, 2012
// EMECS Group Design Project

// Port configurations for the Giant Gecko used in the GG STK and DK
// (part number GG990F1024)
// automatically included by port_config.h

#ifndef __PORTS_GG990F1024_H
#define __PORTS_GG990F1024_H

#include "uartport.h"
#include "i2cbus.h"

// -------------------------------------------------------------------------
// Section: Peripheral port mappings ---------------------------------------
// -------------------------------------------------------------------------
#define GPS_USART_PORT		UARTManagerPortLEUART0
#define XBEE_USART_PORT		UARTManagerPortUSART1
#define ANT_USART_PORT		UARTManagerPortLEUART1
#define MIC_USART_PORT		UARTManagerPortUSART?	// TODO
#define SD_USART_PORT		UARTManagerPortUSART?	// TODO
// -------------------------------------------------------------------------
// End Section: Peripheral port mappings -----------------------------------
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// Section: Misc GPIO port config ------------------------------------------
// -------------------------------------------------------------------------
// PE8 controls the transistor that turns on/off the GPS Vcc
// TODO correct these when/if connected
/*#define	GPIO_GPS_VCC_PORT		gpioPortE
#define	GPIO_GPS_VCC_PIN		8
#define GPIO_GPS_VCC			GPIO_GPS_VCC_PORT, GPIO_GPS_VCC_PIN
// PA5 controls the transistor that turns on/off the GPS Vbat
#define	GPIO_GPS_VBAT_PORT		gpioPortA
#define	GPIO_GPS_VBAT_PIN		5
#define GPIO_GPS_VBAT			GPIO_GPS_VBAT_PORT, GPIO_GPS_VBAT_PIN
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
#define GPIO_ANT_VCC_PORT 		gpioPortA 
#define GPIO_ANT_VCC_PIN 		10
#define GPIO_ANT_VCC			GPIO_ANT_VCC_PORT, GPIO_ANT_VCC_PIN
// PA4 is connected to the sleep pin of the ANT
#define GPIO_ANT_SLEEP_PORT 	gpioPortA 
#define GPIO_ANT_SLEEP_PIN 		4
#define GPIO_ANT_SLEEP			GPIO_ANT_SLEEP_PORT, GPIO_ANT_SLEEP_PIN
// PA2 is connected to the RTS/SEN pin of the ANT
#define GPIO_ANT_RTS_PORT 		gpioPortA 
#define GPIO_ANT_RTS_PIN 		2
#define GPIO_ANT_RTS			GPIO_ANT_RTS_PORT, GPIO_ANT_RTS_PIN
// PA8 is connected to the DTR'/SLEEP_RQ pin of the XBEE
#define GPIO_XBEE_DTR_PORT 		gpioPortA 
#define GPIO_XBEE_DTR_PIN 		8
#define GPIO_XBEE_DTR			GPIO_XBEE_DTR_PORT, GPIO_XBEE_DTR_PIN
// PA9 is connected to the transistor that turns on/off the XBEE Vcc
#define GPIO_XBEE_VCC_PORT 		gpioPortA 
#define GPIO_XBEE_VCC_PIN 		9
#define GPIO_XBEE_VCC			GPIO_XBEE_VCC_PORT, GPIO_XBEE_VCC_PIN*/
// -------------------------------------------------------------------------
// End Section: Misc GPIO port config --------------------------------------
// -------------------------------------------------------------------------


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

#define UART_MANAGER_PORT_COUNT         3

static const UARTPortConfig UARTManagerPortConfigs[UART_MANAGER_PORT_COUNT] =
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
  // port configuration for LEUART1, location 1
  {
    .clockPoint = cmuClock_LEUART1,
    .irqNumber = LEUART1_IRQn,
    .usartBase = (USART_TypeDef *) LEUART1,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = 1,
    .txPort = gpioPortA,
    .txPin = 5,
    .rxPort = gpioPortA,
    .rxPin = 6,
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
  UARTManagerPortLEUART1 = 1,
  UARTManagerPortUSART1 = 2
} UARTManagerPort;

// -------------------------------------------------------------------------
// End Section: USART and LEUART port config -------------------------------
// -------------------------------------------------------------------------

#endif	// __PORTS_GG990F1024_H