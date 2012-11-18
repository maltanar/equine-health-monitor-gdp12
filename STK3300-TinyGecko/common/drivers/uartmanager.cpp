#include "uartmanager.h"

// TODO move definitions below to part-specific header file ------------------
void LEUART0_IRQHandler()
{
  UARTManager::getInstance()->routeInterrupt(UARTManagerPortLEUART0);
}
// TODO move definitions above to part-specific header file ------------------

UARTManager::UARTManager()
{
  m_configs = (const UARTPortConfig *) UARTManagerPortConfigs;
  for(int i = 0; i < UART_MANAGER_PORT_COUNT; i++)
    m_ports[i] = 0;
}

UARTPort * UARTManager::getPort(UARTManagerPort port)
{
  if(m_ports[port] == 0)
  {
    // port instance was not constructed, do it now
    m_ports[port] = new UARTPort(&m_configs[port]);
  }
  
  return m_ports[port];
}

void UARTManager::routeInterrupt(UARTManagerPort port)
{
  getPort(port)->handleInterrupt();
}