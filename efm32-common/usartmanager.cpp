#include <stdio.h>
#include "usartmanager.h"

#include "leuartport.h"
#include "uartport.h"
#include "spiport.h"
#include "i2sport.h"

#ifdef ROUTE_ISR_LEUART0
void LEUART0_IRQHandler()
{
  USARTManager::getInstance()->routeInterrupt(USARTManagerPortLEUART0);
}
#endif

#ifdef ROUTE_ISR_LEUART1
void LEUART1_IRQHandler()
{
  USARTManager::getInstance()->routeInterrupt(USARTManagerPortLEUART1);
}
#endif

#ifdef ROUTE_ISR_USART0_RX
void USART0_RX_IRQHandler()
{
  USARTManager::getInstance()->routeInterrupt(USARTManagerPortUSART0);
}
#endif

#ifdef ROUTE_ISR_USART1_RX
void USART1_RX_IRQHandler()
{
  USARTManager::getInstance()->routeInterrupt(USARTManagerPortUSART1);
}
#endif

#ifdef ROUTE_ISR_USART2_RX
void USART2_RX_IRQHandler()
{
  USARTManager::getInstance()->routeInterrupt(USARTManagerPortUSART2);
}
#endif

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
		else
                {
                        if (m_configs[port].i2s == true)
                            m_ports[port] = new I2SPort(&m_configs[port]);
                        else
                            m_ports[port] = new SPIPort(&m_configs[port]);
                }
	}
  }
  
  return m_ports[port];
}

void USARTManager::routeInterrupt(USARTManagerPort port)
{
  getPort(port)->handleInterrupt();
}