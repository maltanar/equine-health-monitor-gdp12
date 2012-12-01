#include <stdint.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "uartport.h"
#include "dmamanager.h"
#include "em_usart.h"
#include "em_leuart.h"

#include "debug_output_control.h"

USARTPort::USARTPort(const USARTPortConfig *cfg)
{
  m_portConfig = cfg;
  m_initialized = false;
  m_rxHook = 0;
}

void USARTPort::setRxHook(RxHook h)
{
  m_rxHook = h;
}
