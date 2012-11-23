// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __UARTMANAGERGG990F1024_H
#define __UARTMANAGERGG990F1024_H

#include <stdint.h>
#include "em_cmu.h"
#include "em_gpio.h"
#include "uartport.h"

// TODO move definitions below to part-specific header file ------------------

// TODO add other interfaces for the Tiny Gecko?

#define UART_MANAGER_PORT_COUNT         2

const UARTPortConfig UARTManagerPortConfigs[UART_MANAGER_PORT_COUNT] = 
{
  // port configuration for LEUART0
  {
    .clockPoint = cmuClock_LEUART0,
    .irqNumber = LEUART0_IRQn,
    .usartBase = (USART_TypeDef *) LEUART0,
    // TODO route location & pins should be runtime configurable?
    .routeLocation = LEUART_ROUTE_LOCATION_LOC0, 
    .txPort = gpioPortD,
    .txPin = 4,
    .rxPort = gpioPortD,
    .rxPin = 5,
    .lowEnergy = true
  },
  // port configuration for USART1
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
    .lowEnergy = false
  }
};

typedef enum {
  UARTManagerPortLEUART0 = 0,
  UARTManagerPortUSART1 = 1
} UARTManagerPort;

// TODO move definitions above to part-specific header file ------------------

class UARTManager {
public:
  // singleton instance accessor
  static UARTManager* getInstance()
  {
    static UARTManager instance;
    return &instance;
  }
  
  UARTPort * getPort(UARTManagerPort port);
  void routeInterrupt(UARTManagerPort port);
  
private:
  // ------ start of singleton pattern specific section ------
  UARTManager();  
  UARTManager(UARTManager const&);                // do not implement
  void operator=(UARTManager const&);        // do not implement
  // ------ end of singleton pattern specific section --------
  
  const UARTPortConfig * m_configs;
  UARTPort * m_ports[UART_MANAGER_PORT_COUNT];
};

#endif // __UARTMANAGERGG990F1024_H