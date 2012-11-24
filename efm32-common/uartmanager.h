// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __UARTMANAGER_H
#define __UARTMANAGER_H

#include "em_usart.h"
#include "em_leuart.h"
#include "port_config.h"

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

#endif // __UARTMANAGER_H