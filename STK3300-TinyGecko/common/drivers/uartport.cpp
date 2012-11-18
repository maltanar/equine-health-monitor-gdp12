#include <stdint.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "uartport.h"
#include "dmamanager.h"

#include "debug_output_control.h"

#if defined(RETARGET_USART)
#include "em_usart.h"
#endif

#if defined(RETARGET_LEUART)
#include "em_leuart.h"
#endif

void RETARGET_IRQ_NAME(void)
{
  uint32_t leuartif = LEUART_IntGet(LEUART0);
  LEUART_IntClear(LEUART0, leuartif);
  
  if (leuartif & LEUART_IF_RXDATAV)
    UARTPort::getInstance()->handleRxInterrupt();
  else if (leuartif & LEUART_IF_SIGF)
    UARTPort::getInstance()->handleSigFrameInterrupt();
}

UARTPort::UARTPort()
{
  m_initialized = false;
  m_sfHook = 0;
  m_rxHook = 0;
}

void UARTPort::handleSigFrameInterrupt()
{
  module_debug_uart("signal frame!");
  
  // call the signal frame hook if set
  if(m_sfHook)
    m_sfHook(m_rxBuffer);
  
  // reactivate DMA
  DMAManager::getInstance()->activateBasic(m_dmaChannel, NULL, NULL, 
                                           m_rxBufferSize - 1);
}

void UARTPort::handleRxInterrupt()
{
  int c = RETARGET_RX(RETARGET_UART);
  module_debug_uart("rx interrupt!");
  
  // execute rx hook if defined
  if(m_rxHook)
    m_rxHook(c);

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

  // Start LFXO, and use LFXO for low-energy modules
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
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


void UARTPort::setupDMA(uint8_t dmaChannel, uint8_t signalFrameChar)
{
  // require normal initialization first
  if(!m_initialized)
    return;
  
  m_dmaChannel = dmaChannel;
  // TODO is there anything we cannot accept as the signal frame char?
  m_signalFrameChar = signalFrameChar;
  
  // config LEUART DMA, interrupts and DMAManager
  DMAManager * dmaMgr = DMAManager::getInstance();
  
  // configure the DMA channel and descriptor for LEUART0
  // TODO make this customizable
  dmaMgr->configureChannel(m_dmaChannel, false, DMAREQ_LEUART0_RXDATAV);
  dmaMgr->configureDescriptor(m_dmaChannel, dmaDataInc1, dmaDataIncNone, 
                              dmaDataSize1, dmaArbitrate1);
  // activate DMA transfer
  dmaMgr->activateBasic(m_dmaChannel, (void *) &LEUART0->RXDATA,
                        (void *) m_rxBuffer, m_rxBufferSize - 1);
  
  // configure signal frame - interrupt to be generated upon encountering
  // this character
  LEUART0->SIGFRAME = m_signalFrameChar;
  
  // enable LEUART signal frame interrupt
  LEUART_IntEnable(LEUART0, LEUART_IEN_SIGF);
  // disable LEUART receive interrupt
  LEUART_IntDisable(RETARGET_UART, LEUART_IF_RXDATAV);
  
  // Make sure the LEUART wakes up the DMA on RX data
  LEUART0->CTRL = LEUART_CTRL_RXDMAWU;
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
  m_rxHook = h;
}

void UARTPort::setSignalFrameHook(SigFrameHook h)
{
  m_sfHook = h;
}