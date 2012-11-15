#include <stdint.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "uartport.h"

#include "debug_output_control.h"

#if defined(RETARGET_USART)
#include "em_usart.h"
#endif

#if defined(RETARGET_LEUART)
#include "em_leuart.h"
#endif

void RETARGET_IRQ_NAME(void)
{
  UARTPort::getInstance()->handleUARTInterrupt();
}

UARTPort::UARTPort()
{
  m_initialized = false;
  m_hook = 0;
}

void UARTPort::handleUARTInterrupt()
{
#if defined(RETARGET_USART)
  if (RETARGET_UART->STATUS & USART_STATUS_RXDATAV)
  {
#else
  if (RETARGET_UART->IF & LEUART_IF_RXDATAV)
  {
#endif
    int c = RETARGET_RX(RETARGET_UART);
    
    // execute rx hook if defined
    if(m_hook)
      m_hook(c);

    // Store Data 
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
      flushRxBuffer();
    }
  }
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
  
  if(!rxBuffer || rxBufferSize == 0) {
    module_debug_uart("cannot work without rx buffer!");
    return false;
  }
  
  // Configure GPIO pins 
  CMU_ClockEnable(cmuClock_GPIO, true);
  
  // To avoid false start, configure output as high 
  GPIO_PinModeSet(RETARGET_TXPORT, RETARGET_TXPIN, gpioModePushPull, 1);
  GPIO_PinModeSet(RETARGET_RXPORT, RETARGET_RXPIN, gpioModeInput, 0);

#if defined(RETARGET_USART)
  USART_TypeDef           *usart = RETARGET_UART;
  USART_InitAsync_TypeDef init   = USART_INITASYNC_DEFAULT;

  // Enable peripheral clocks 
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(RETARGET_CLK, true);

  // Configure USART for basic async operation 
  init.enable = usartDisable;
  init.baudrate = baudRate;
  USART_InitAsync(usart, &init);

  // Enable pins at UART1 location #2 
  usart->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN | RETARGET_LOCATION;

  // Clear previous RX interrupts 
  USART_IntClear(RETARGET_UART, USART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(RETARGET_IRQn);

  // Enable RX interrupts 
  USART_IntEnable(RETARGET_UART, USART_IF_RXDATAV);
  NVIC_EnableIRQ(RETARGET_IRQn);

  // Finally enable it 
  USART_Enable(usart, usartEnable);

#else
  LEUART_TypeDef      *leuart = RETARGET_UART;
  LEUART_Init_TypeDef init    = LEUART_INIT_DEFAULT;

  // Enable CORE LE clock in order to access LE modules 
  CMU_ClockEnable(cmuClock_CORELE, true);

  // Select LFXO for LEUARTs (and wait for it to stabilize) 
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

  CMU_ClockEnable(RETARGET_CLK, true);

  // Do not prescale clock 
  CMU_ClockDivSet(RETARGET_CLK, cmuClkDiv_1);

  // Configure LEUART 
  init.enable = leuartDisable;
  init.baudrate = baudRate;
  LEUART_Init(leuart, &init);
  // Enable pins at default location 
  leuart->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN | RETARGET_LOCATION;

  // Clear previous RX interrupts
  LEUART_IntClear(RETARGET_UART, LEUART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(RETARGET_IRQn);

  // Enable RX interrupts
  LEUART_IntEnable(RETARGET_UART, LEUART_IF_RXDATAV);
  NVIC_EnableIRQ(RETARGET_IRQn);

  // Finally enable it
  LEUART_Enable(leuart, leuartEnable);
#endif

#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
  setvbuf(stdout, NULL, _IONBF, 0);   //Set unbuffered mode for stdout (newlib)
#endif
  
  m_initialized = true;
  return true;
}

int UARTPort::readChar(void)
{
  int c = -1;

  NVIC_DisableIRQ(RETARGET_IRQn);
  if (m_rxCount > 0)
  {
    c = m_rxBuffer[m_rxReadIndex];
    m_rxReadIndex++;
    if (m_rxReadIndex == m_rxBufferSize)
    {
      m_rxReadIndex = 0;
    }
    m_rxCount--;
  }
  NVIC_EnableIRQ(RETARGET_IRQn);

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
  RETARGET_TX(RETARGET_UART, c);

  return c;
}

void UARTPort::setRxHook(RxHook h)
{
  m_hook = h;
}