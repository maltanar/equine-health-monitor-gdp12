#include <stdint.h>
#include <string.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "leuartport.h"
#include "dmamanager.h"
#include "em_usart.h"
#include "em_leuart.h"

#include "debug_output_control.h"


LEUARTPort::LEUARTPort(const USARTPortConfig *cfg) :
	USARTPort(cfg)
{
    // no additional initialization necessary for LEUART
	m_dmaBuffer = NULL;
	m_dmaBufferSize = 0;
	m_sfHook = 0;
}

bool LEUARTPort::initialize(uint8_t *rxBuffer, uint8_t rxBufferSize, 
                            LEBaudRate baudRate = leuartPortBaudRate9600)
{
	// initialize the buffer variables
	m_rxReadIndex = 0;
	m_rxWriteIndex = 0;
	m_rxCount = 0;
	m_rxBufferSize = rxBufferSize;
	m_rxBuffer = rxBuffer;
	

	// Configure GPIO pins 
	CMU_ClockEnable(cmuClock_GPIO, true);

	// To avoid false start, configure output as high 
	GPIO_PinModeSet(m_portConfig->txPort, m_portConfig->txPin, gpioModePushPull, 1);
	GPIO_PinModeSet(m_portConfig->rxPort, m_portConfig->rxPin, gpioModeInput, 0);

	// LEUART configuration
	module_debug_leuart("configuring LEUART...");
	LEUART_TypeDef *leuart = (LEUART_TypeDef *) m_portConfig->usartBase;
	LEUART_Init_TypeDef init = LEUART_INIT_DEFAULT;

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
				   | (m_portConfig->routeLocation << _LEUART_ROUTE_LOCATION_SHIFT);

	// Clear previous RX interrupts
	LEUART_IntClear(leuart, LEUART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(m_portConfig->irqNumber);

	// Enable RX interrupts
	LEUART_IntEnable(leuart, LEUART_IF_RXDATAV);
	NVIC_EnableIRQ(m_portConfig->irqNumber);

	// Finally enable it
	LEUART_Enable(leuart, leuartEnable);    


	m_initialized = true;
	module_debug_leuart("init OK, buffer size %d", m_rxBufferSize);
    
    return true;
}

void LEUARTPort::setSignalFrameHook(SigFrameHook h)
{
    m_sfHook = h;
}

void LEUARTPort::handleInterrupt()
{
	uint32_t uartif = LEUART_IntGet((LEUART_TypeDef *) m_portConfig->usartBase);
	LEUART_IntClear((LEUART_TypeDef *) m_portConfig->usartBase, uartif);

	if (uartif & LEUART_IF_RXDATAV)
	{
		uint8_t c = LEUART_Rx((LEUART_TypeDef *) m_portConfig->usartBase);
		module_debug_leuart("rx interrupt! %x ", c);

		// execute rx hook if defined
		if(m_rxHook)
			if(m_rxHook(c))
				return;	// exit handler if rx hook returns true, it already
				// received processed the data

		if(!m_rxBuffer || m_rxBufferSize == 0)
		{
			module_debug_leuart("no rx buffer, dropping data");
			return;
		}

		// store data in buffer if it was not handled by rxHook
		m_rxBuffer[m_rxWriteIndex] = c;
		m_rxWriteIndex++;
		m_rxCount++;
		if (m_rxWriteIndex == m_rxBufferSize)
		{
	
		}
		// Check for overflow - flush buffer
		if (m_rxCount > m_rxBufferSize)
		{
			module_debug_leuart("overflow, buffer flush!");
			flushRxBuffer();
		}
	}

	if (uartif & LEUART_IF_SIGF)
	{
		module_debug_leuart("signal frame!");
		// nothing to do without DMA buffer
		if(!m_dmaBuffer)
			return;
		
		// copy data to rx buffer from DMA buffer
		// TODO maybe used rxWriteIndex and do wraparound
		uint32_t len = m_dmaBufferSize - 1 - ((m_dmaDescriptor->CTRL >> 4) & 0x3FF);
		module_debug_leuart("len = %d %x %x", len, m_rxBuffer, m_dmaBuffer);
		
		DMAManager::getInstance()->activateBasic(m_dmaChannel, NULL, NULL, 
											 m_dmaBufferSize - 1);
		
		memcpy(m_rxBuffer, m_dmaBuffer, len);
		
		// call the signal frame hook if set
		if(m_sfHook)
			m_sfHook(m_rxBuffer);
	}
}

void LEUARTPort::setupSignalFrame(uint8_t signalFrameChar)
{
	// TODO is there anything we cannot accept as the signal frame char?
    m_signalFrameChar = signalFrameChar;
	
	// configure signal frame - interrupt to be generated upon encountering
    // this character
    ((LEUART_TypeDef *) m_portConfig->usartBase)->SIGFRAME = m_signalFrameChar;
    
    // enable LEUART signal frame interrupt
    LEUART_IntEnable(LEUART0, LEUART_IEN_SIGF);
}

void LEUARTPort::setupDMA(uint8_t *dmaBuffer, uint8_t dmaBufferSize, uint8_t dmaChannel)
{
    // require normal initialization first
    if(!m_initialized)
        return;
	
	if(!dmaBuffer || dmaBufferSize == 0)
    {
		module_debug_leuart("cannot enable DMA without buffer");
		return;
    }
    
    module_debug_leuart("Configuring DMA on channel %d ", dmaChannel);
    
    // TODO this is hardcoded for LEUART0, modify this to support others
    
	m_dmaBufferSize = dmaBufferSize; 
	m_dmaBuffer = dmaBuffer;
    m_dmaChannel = dmaChannel;
    
    
    // config LEUART DMA, interrupts and DMAManager
    DMAManager * dmaMgr = DMAManager::getInstance();
    
    // configure the DMA channel and descriptor for LEUART0
    // TODO make this customizable - not only for LEUART0!! need dma no in config
    dmaMgr->configureChannel(m_dmaChannel, false, DMAREQ_LEUART0_RXDATAV);
    m_dmaDescriptor = dmaMgr->configureDescriptor(m_dmaChannel, dmaDataInc1, dmaDataIncNone, 
                                dmaDataSize1, dmaArbitrate1);
    // activate DMA transfer
    dmaMgr->activateBasic(m_dmaChannel, (void *) &((LEUART_TypeDef *) m_portConfig->usartBase)->RXDATA,
                                                (void *) m_dmaBuffer, m_dmaBufferSize - 1);
    
    // disable LEUART receive interrupt
    LEUART_IntDisable((LEUART_TypeDef *) m_portConfig->usartBase, LEUART_IF_RXDATAV);
	
	// TODO enable DMA interrupt and reroute to here!
	// this should be done similar to what USARTManager does for the ports
	// for LEUART it is fine to not have ,t as we can live with signal frame 
	// interrupts instead
    
    // Make sure the LEUART wakes up the DMA on RX data
    ((LEUART_TypeDef *) m_portConfig->usartBase)->CTRL = LEUART_CTRL_RXDMAWU;
}

int LEUARTPort::readChar(void)
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
        module_debug_leuart("read %x (%c)", c, c);
    }
    NVIC_EnableIRQ(m_portConfig->irqNumber);

    return c;
}

void LEUARTPort::flushRxBuffer()
{
    m_rxWriteIndex = 0;
    m_rxCount            = 0;
    m_rxReadIndex    = 0;
}

int LEUARTPort::writeChar(char c)
{
    module_debug_leuart("write %x (%c)", c, c);
    LEUART_Tx((LEUART_TypeDef *) m_portConfig->usartBase, c);

    return c;
}