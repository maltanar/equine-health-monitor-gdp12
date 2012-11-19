#include <stdint.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "uartport.h"
#include "dmamanager.h"
#include "em_usart.h"
#include "em_leuart.h"

#include "debug_output_control.h"

#define UARTPORT_TX(x,y)        (m_portConfig->lowEnergy ? LEUART_Tx((LEUART_TypeDef *) x, y) : USART_Tx(x, y))
#define UARTPORT_RX(x)          (m_portConfig->lowEnergy ? LEUART_Rx((LEUART_TypeDef *) x) : USART_Rx(x))
#define UARTPORT_INTGET(x)      (m_portConfig->lowEnergy ? LEUART_IntGet((LEUART_TypeDef *) x ) : USART_IntGet(x))
#define UARTPORT_INTCLR(x,y)    (m_portConfig->lowEnergy ? LEUART_IntClear((LEUART_TypeDef *) x,y) : USART_IntClear(x,y))  
#define UARTPORT_RXDATAV        (m_portConfig->lowEnergy ? LEUART_IF_RXDATAV : USART_IF_RXDATAV)

UARTPort::UARTPort(const UARTPortConfig *cfg)
{
  m_portConfig = cfg;
  m_initialized = false;
  m_sfHook = 0;
  m_rxHook = 0;
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
  GPIO_PinModeSet(m_portConfig->txPort, m_portConfig->txPin, gpioModePushPull, 1);
  GPIO_PinModeSet(m_portConfig->rxPort, m_portConfig->rxPin, gpioModeInput, 0);
  
  if(m_portConfig->lowEnergy)
  {
    // LEUART configuration
    module_debug_uart("configuring LEUART...");
    LEUART_TypeDef      *leuart = (LEUART_TypeDef *) m_portConfig->usartBase;
    LEUART_Init_TypeDef init    = LEUART_INIT_DEFAULT;

    // Enable CORE LE clock in order to access LE modules 
    CMU_ClockEnable(cmuClock_CORELE, true);

    // Start LFXO, and use LFXO for low-energy modules
    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

    CMU_ClockEnable(m_portConfig->clockPoint, true);

    // Do not prescale clock 
    CMU_ClockDivSet(m_portConfig->clockPoint, cmuClkDiv_1);

    // Configure LEUART 
    init.enable = leuartDisable;
    init.baudrate = baudRate;
    LEUART_Init(leuart, &init);
    // Enable pins at default location 
    leuart->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN 
                    | m_portConfig->routeLocation;

    // Clear previous RX interrupts
    LEUART_IntClear(leuart, LEUART_IF_RXDATAV);
    NVIC_ClearPendingIRQ(m_portConfig->irqNumber);

    // Enable RX interrupts
    LEUART_IntEnable(leuart, LEUART_IF_RXDATAV);
    NVIC_EnableIRQ(m_portConfig->irqNumber);

    // Finally enable it
    LEUART_Enable(leuart, leuartEnable);  
  }
  else
  {
    // regular UART configuration
    module_debug_uart("configuring USART...");
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
                   m_portConfig->routeLocation;

    // Clear previous RX interrupts 
    USART_IntClear(usart, USART_IF_RXDATAV);
    NVIC_ClearPendingIRQ(m_portConfig->irqNumber);

    // Enable RX interrupts 
    USART_IntEnable(usart, USART_IF_RXDATAV);
    NVIC_EnableIRQ(m_portConfig->irqNumber);

    // Finally enable it 
    USART_Enable(usart, usartEnable);
  }
  
  m_initialized = true;
  module_debug_uart("init OK, buffer size %d", m_rxBufferSize);
  
  return true;
}

void UARTPort::setRxHook(RxHook h)
{
  m_rxHook = h;
}

void UARTPort::setSignalFrameHook(SigFrameHook h)
{
  m_sfHook = h;
}

void UARTPort::handleInterrupt()
{
  uint32_t uartif = UARTPORT_INTGET(m_portConfig->usartBase);
  UARTPORT_INTCLR(m_portConfig->usartBase, uartif);
  
  if (uartif & UARTPORT_RXDATAV)
  {
    int c = UARTPORT_RX(m_portConfig->usartBase);
    module_debug_uart("rx interrupt!");
    
    // execute rx hook if defined
    if(m_rxHook)
      m_rxHook(c);

    // store data 
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
  
  if (uartif & LEUART_IF_SIGF)
  {
    module_debug_uart("signal frame!");
  
    // call the signal frame hook if set
    if(m_sfHook)
      m_sfHook(m_rxBuffer);
    
    // reactivate DMA
    DMAManager::getInstance()->activateBasic(m_dmaChannel, NULL, NULL, 
                                             m_rxBufferSize - 1);
  }
}

void UARTPort::setupDMA(uint8_t dmaChannel, uint8_t signalFrameChar)
{
  // TODO add support for non-LE UART
  if(!m_portConfig->lowEnergy)
  {
    module_debug_uart("DMA for non-LE UART not yet supported!");
    return;
  }
  
  // require normal initialization first
  if(!m_initialized)
    return;
  
  // TODO this is hardcoded for LEUART0, modify this to support others
  
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
  LEUART_IntDisable((LEUART_TypeDef *) m_portConfig->usartBase, LEUART_IF_RXDATAV);
  
  // Make sure the LEUART wakes up the DMA on RX data
  LEUART0->CTRL = LEUART_CTRL_RXDMAWU;
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
  UARTPORT_TX(m_portConfig->usartBase, c);

  return c;
}