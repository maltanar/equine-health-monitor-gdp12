#include <stdint.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

#include "i2sport.h"
#include "debug_output_control.h"

#include "audio/audio_config.h"

I2SPort::I2SPort(const USARTPortConfig *cfg) :
  USARTPort(cfg)
{
  
}

bool I2SPort::initialize()
{
	USART_TypeDef *i2s;
	
	i2s = m_portConfig->usartBase;
	CMU_ClockEnable(m_portConfig->clockPoint, true);

    USART_InitI2s_TypeDef init = {
	  {
	    .enable      = usartDisable,    /* Initially disabled! */                 \
        .refFreq     = 0,               /* current configured reference clock */  \
        .baudrate    = 64*8000,         /* Baudrate:64*8000 */                    \
        .databits    = usartDatabits16, /* 16 databits. */                        \
        .master      = true,            /* Operate as I2S master. */              \
        .msbf        = true,            /* Most significant bit first. */         \
        .clockMode   = usartClockMode0, /* Clk idle high sample rising edge*/     \
        .prsRxEnable = false,           /* Don't enable USARTRx via PRS. */       \
        .prsRxCh     = usartPrsRxCh0,   /* PRS channel selection (dummy). */      \
        .autoTx      = true             /* enable AUTOTX mode. */                 \
  	  },                                                                          \
	  .format   = usartI2sFormatW32D16, /* 32-bit word, 24-bit data */            \
	  .delay    = true,                 /* Delay on I2S data. */                  \
	  .dmaSplit = true,                 /* DMA split. */                          \
	  .justify  = usartI2sJustifyLeft,  /* data left-justified within the frame */\
	  .mono     = false                 /* Stereo mode needed, only left channel used though! . */\
	};
  
    USART_InitI2s(i2s, &init);	
	
  	i2s->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN 
				| USART_ROUTE_CSPEN | 
				(m_portConfig->routeLocation << _USART_ROUTE_LOCATION_SHIFT);

    
    GPIO_PinModeSet(m_portConfig->txPort, m_portConfig->txPin, gpioModePushPull, 0);  /* MOSI */
	GPIO_PinModeSet(m_portConfig->rxPort, m_portConfig->rxPin, gpioModeInputPull, 0);  /* MISO */
	GPIO_PinModeSet(m_portConfig->csPort, m_portConfig->csPin, gpioModePushPull,   1);  /* CS (WS)*/
	GPIO_PinModeSet(m_portConfig->sclkPort, m_portConfig->sclkPin, gpioModePushPull, 0);  /* Clock */

	m_initialized = true;
	module_debug_spi("init OK");

	return true;
}

void I2SPort::handleInterrupt()
{
	module_debug_spi("handleInterrupt not implemented!");
}

int I2SPort::readChar(void)
{
	module_debug_spi("readChar not implemented!");
	// TODO just transmit dummy data
	return -1;
}


int I2SPort::writeChar(char c)
{
	module_debug_spi("writeChar not implemented!");
	// TODO just transmit dummy data
	return -1;
}

void I2SPort::enable(bool active) {

  if (active)
    USART_Enable(m_portConfig->usartBase, usartEnable);
  else 
    USART_Enable(m_portConfig->usartBase, usartDisable);

}

