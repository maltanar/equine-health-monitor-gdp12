#include <stdint.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

#include "spiport.h"
#include "debug_output_control.h"

#include "audio/audio_config.h"

SPIPort::SPIPort(const USARTPortConfig *cfg) :
  USARTPort(cfg)
{
  
}

bool SPIPort::initialize()
{
	USART_TypeDef *spi;
	
	spi = m_portConfig->usartBase;
	CMU_ClockEnable(m_portConfig->clockPoint, true);
	
	spi->CLKDIV = 128 * (SPI_PERCLK_FREQUENCY / SPI_BAUDRATE - 2);
	
	
	
	/* Configure SPI */
	/* Using synchronous (SPI) mode*/
	spi->CTRL = USART_CTRL_SYNC;
	/* Clearing old transfers/receptions, and disabling interrupts */
	spi->CMD = USART_CMD_CLEARRX | USART_CMD_CLEARTX;
	spi->IEN = 0;
	
	spi->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN 
				| USART_ROUTE_CSPEN | 
					(m_portConfig->routeLocation << _USART_ROUTE_LOCATION_SHIFT);
	
	//Setting frame size to 16 bits
	spi->FRAME &= ~USART_FRAME_DATABITS_DEFAULT;
	spi->FRAME |= USART_FRAME_DATABITS_SIXTEEN;
	//SET clock polarity and phase
	spi->CTRL |= USART_CTRL_CLKPOL_IDLEHIGH | USART_CTRL_CLKPHA_SAMPLETRAILING; 
	//set Transmitter as Tri-stated
	spi->CMD = USART_CMD_TXTRIEN;
	
	/* Set GPIO config to slave */
	GPIO_Mode_TypeDef gpioModeMosi = gpioModePushPull;
	GPIO_Mode_TypeDef gpioModeMiso = gpioModeInput;
	GPIO_Mode_TypeDef gpioModeCs   = gpioModePushPull;
	GPIO_Mode_TypeDef gpioModeClk  = gpioModePushPull;
	
	/* Enabling Master, TX and RX */
    spi->CMD   = USART_CMD_MASTEREN | USART_CMD_TXEN | USART_CMD_RXEN;
    spi->CTRL |= USART_CTRL_AUTOCS;
    spi->CTRL |= USART_CTRL_MSBF;
	
	/* Clear previous interrupts */
	spi->IFC = _USART_IFC_MASK;
	
	GPIO_PinModeSet(m_portConfig->txPort, m_portConfig->txPin, gpioModeMosi, 0);  /* MOSI */
	GPIO_PinModeSet(m_portConfig->rxPort, m_portConfig->rxPin, gpioModeMiso, 0);  /* MISO */
	GPIO_PinModeSet(m_portConfig->csPort, m_portConfig->csPin, gpioModeCs,   1);  /* CS */
	GPIO_PinModeSet(m_portConfig->sclkPort, m_portConfig->sclkPin, gpioModeClk, 0);  /* Clock */
	
  
	m_initialized = true;
	module_debug_spi("init OK");

	return true;
}

void SPIPort::handleInterrupt()
{
	module_debug_spi("handleInterrupt not implemented!");
}

int SPIPort::readChar(void)
{
	module_debug_spi("readChar not implemented!");
	// TODO just transmit dummy data
	return -1;
}


int SPIPort::writeChar(char c)
{
  module_debug_spi("write %x (%c)", c, c);
  
  
  USART_TypeDef *uart = m_portConfig->usartBase;

  while (!(uart->STATUS & USART_STATUS_TXBL)) ;
  
  uart->TXDATA = c;
  
  while(!(uart->STATUS & USART_STATUS_RXDATAV)) ;
  
  char r = uart->RXDATA;
  
  // Reading out data
  return r;
}

short SPIPort::writeShort(short c)
{
  module_debug_spi("write16 %x (%c)", c, c);
  
  USART_TypeDef *uart = m_portConfig->usartBase;

  while (!(uart->STATUS & USART_STATUS_TXBL)) ;
  
  uart->TXDOUBLE = c;
  
  while(!(uart->STATUS & USART_STATUS_RXFULL)) ;
  
  uint16_t r = uart->RXDOUBLE;
  
  // Reading out data
  return r;
}

void SPIPort::waitWriteComplete()
{
	while (!(m_portConfig->usartBase->STATUS & USART_STATUS_TXC)) ;
}