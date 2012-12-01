#include <stdint.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "uartport.h"
#include "em_usart.h"

#include "debug_output_control.h"


UARTPort::UARTPort(const USARTPortConfig *cfg) :
  USARTPort(cfg)
{
  
}

bool UARTPort::initialize(uint8_t *rxBuffer, uint8_t rxBufferSize, 
                          BaudRate baudRate = uartPortBaudRate9600, 
                          DataBits dataBits = uartPortDataBits8, 
                          Parity parity = uartPortParityNone, 
                          StopBits stopBits = uartPortStopBits1)
{
	// initialize the buffer variables
	m_rxReadIndex  = 0;
	m_rxWriteIndex = 0;
	m_rxCount      = 0;
	m_rxBufferSize = rxBufferSize;
	m_rxBuffer     = rxBuffer;

	// Configure GPIO pins 
	CMU_ClockEnable(cmuClock_GPIO, true);

	// To avoid false start, configure output as high 
	GPIO_PinModeSet(m_portConfig->txPort, m_portConfig->txPin, gpioModePushPull, 1);
	GPIO_PinModeSet(m_portConfig->rxPort, m_portConfig->rxPin, gpioModeInput, 0);

	// regular UART configuration
	module_debug_uart("configuring UART...");
	USART_TypeDef *usart = m_portConfig->usartBase;
	USART_InitAsync_TypeDef init   = USART_INITASYNC_DEFAULT;

	// Enable peripheral clocks 
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(m_portConfig->clockPoint, true);

	// Configure USART for basic async operation 
	init.enable = usartDisable;
	init.baudrate = baudRate;
	USART_InitAsync(usart, &init);

	// Enable pins at desired UART location
	usart->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN | 
		   (m_portConfig->routeLocation << _USART_ROUTE_LOCATION_SHIFT);

	// Clear previous RX interrupts 
	USART_IntClear(usart, USART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(m_portConfig->irqNumber);

	// Enable RX interrupts 
	USART_IntEnable(usart, USART_IF_RXDATAV);
	NVIC_EnableIRQ(m_portConfig->irqNumber);

	// Finally enable it 
	USART_Enable(usart, usartEnable);
  
	m_initialized = true;
	module_debug_uart("init OK, buffer size %d", m_rxBufferSize);

	return true;
}

void UARTPort::handleInterrupt()
{
  uint32_t uartif = USART_IntGet(m_portConfig->usartBase);
  USART_IntClear(m_portConfig->usartBase, uartif);
  
  if (uartif & USART_IF_RXDATAV)
  {
    uint8_t c = USART_Rx(m_portConfig->usartBase);
    module_debug_uart("rx interrupt! %x ", c);
    
    // execute rx hook if defined
    if(m_rxHook)
      if(m_rxHook(c))
		  return;	// exit handler if rx hook returns true, it already
					// received processed the data
	
	if(!m_rxBuffer || m_rxBufferSize == 0)
	{
		module_debug_uart("no rx buffer, dropping data");
		return;
	}

    // store data in buffer if it was not handled by rxHook
    m_rxBuffer[m_rxWriteIndex] = c;
    m_rxWriteIndex++;
    m_rxCount++;
    if (m_rxWriteIndex == m_rxBufferSize)
    {
      m_rxWriteIndex = 0;
    }
    // Check for overflow - flush buffer
    if (m_rxCount > m_rxBufferSize)
    {
      module_debug_uart("overflow, buffer flush!");
      flushRxBuffer();
    }
  }
}

int UARTPort::readChar(void)
{
  int c = -1;

  NVIC_DisableIRQ(m_portConfig->irqNumber);
  
  if (m_rxCount > 0)
  {
    c = m_rxBuffer[m_rxReadIndex];
    m_rxReadIndex++;
    if (m_rxReadIndex == m_rxBufferSize)
    {
      m_rxReadIndex = 0;
    }
    m_rxCount--;
    module_debug_uart("read %x (%c)", c, c);
  }
  NVIC_EnableIRQ(m_portConfig->irqNumber);

  return c;
}

void UARTPort::flushRxBuffer()
{
  m_rxWriteIndex = 0;
  m_rxCount      = 0;
  m_rxReadIndex  = 0;
}

int UARTPort::writeChar(char c)
{
  module_debug_uart("write %x (%c)", c, c);
  USART_Tx(m_portConfig->usartBase, c);

  return c;
}