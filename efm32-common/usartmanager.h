// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __USARTMANAGER_H
#define __USARTMANAGER_H

#include "em_usart.h"
#include "em_leuart.h"
#include "port_config.h"

class USARTManager {
public:
  // singleton instance accessor
  static USARTManager* getInstance()
  {
    static USARTManager instance;
    return &instance;
  }

  USARTPort * getPort(USARTManagerPort port);
  void routeInterrupt(USARTManagerPort port);

private:
  // ------ start of singleton pattern specific section ------
  USARTManager();
  USARTManager(USARTManager const&);                // do not implement
  void operator=(USARTManager const&);        // do not implement
  // ------ end of singleton pattern specific section --------

  const USARTPortConfig * m_configs;
  USARTPort * m_ports[USART_MANAGER_PORT_COUNT];
};

#endif // __USARTMANAGER_H