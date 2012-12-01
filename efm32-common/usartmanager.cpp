#include <stdio.h>
#include "usartmanager.h"

#include "leuartport.h"
#include "uartport.h"
// TODO remove comment below after completing SPIPort framework
//#include "spiport.h"

// TODO move definitions below to part-specific header file ------------------
void LEUART0_IRQHandler()
{
  USARTManager::getInstance()->routeInterrupt(USARTManagerPortLEUART0);
}

void USART1_RX_IRQHandler()
{
  USARTManager::getInstance()->routeInterrupt(USARTManagerPortUSART1);
}

// TODO move definitions above to part-specific header file ------------------

USARTManager::USARTManager()
{
  m_configs = (const USARTPortConfig *) USARTManagerPortConfigs;
  for(int i = 0; i < USART_MANAGER_PORT_COUNT; i++)
    m_ports[i] = 0;
}

USARTPort * USARTManager::getPort(USARTManagerPort port)
{
  if(m_ports[port] == 0)
  {
    // port instance was not constructed, do it now
	// this will depend on lowenergy/sync/async types
	if(m_configs[port].lowEnergy)
		m_ports[port] = new LEUARTPort(&m_configs[port]);
	else 
	{
		if(m_configs[port].async)
			m_ports[port] = new UARTPort(&m_configs[port]);
			// TODO remove comment below after completing SPIPort framework
		/*else
			m_ports[port] = new SPIPort(&m_configs[port]);*/
	}
  }
  
  return m_ports[port];
}

void USARTManager::routeInterrupt(USARTManagerPort port)
{
  getPort(port)->handleInterrupt();
}