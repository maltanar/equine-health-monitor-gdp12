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
#define GPS_USART_PORT		USARTManagerPortLEUART0
#define XBEE_USART_PORT		USARTManagerPortUSART1
#define ANT_USART_PORT		USARTManagerPortLEUART1
#define MIC_USART_PORT		USARTManagerPortUSART?	// TODO
#define SD_USART_PORT		USARTManagerPortUSART0
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
// Section: UART and LEUART port config -----------------------------------
// -------------------------------------------------------------------------

#define USART_MANAGER_PORT_COUNT         4

static const USARTPortConfig USARTManagerPortConfigs[USART_MANAGER_PORT_COUNT] =
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
	.async = true,
    .lowEnergy = true,
	.rxDMASource = DMAREQ_LEUART0_RXDATAV
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
	.async = true,
    .lowEnergy = true,
	.rxDMASource = DMAREQ_LEUART1_RXDATAV
  },
  // port configuration for USART0, location 1
  {
    .clockPoint = cmuClock_USART0 ,
    .irqNumber = USART0_RX_IRQn,
    .usartBase = (USART_TypeDef *) USART0,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = 1,
    .txPort = gpioPortE,
    .txPin = 7,
    .rxPort = gpioPortE,
    .rxPin = 6,
	.csPort = gpioPortE,
	.csPin = 4,
	.sclkPort = gpioPortE,
	.sclkPin = 5,
	.async = false,
    .lowEnergy = false,
	.rxDMASource = DMAREQ_USART0_RXDATAV
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
	.async = true,
    .lowEnergy = false,
	.rxDMASource = DMAREQ_USART1_RXDATAV
  }
};

typedef enum {
  USARTManagerPortLEUART0 = 0,
  USARTManagerPortLEUART1 = 1,
  USARTManagerPortUSART0 = 2,
  USARTManagerPortUSART1 = 3
} USARTManagerPort;

// -------------------------------------------------------------------------
// End Section: UART and LEUART port config -------------------------------
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
// Section: USART interrupt routing control --------------------------------
// -------------------------------------------------------------------------

#define	ROUTE_ISR_LEUART0
#define	ROUTE_ISR_LEUART1
#define NO_ROUTE_ISR_USART0_RX
#define ROUTE_ISR_USART1_RX
#define NO_ROUTE_ISR_USART2_RX

// -------------------------------------------------------------------------
// End Section: USART interrupt routing control ----------------------------
// -------------------------------------------------------------------------


#endif	// __PORTS_GG990F1024_H