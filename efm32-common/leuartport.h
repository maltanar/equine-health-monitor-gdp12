// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __LEUARTPORT_H
#define __LEUARTPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include "usartport.h"

typedef void(*SigFrameHook)(uint8_t *buf);


class LEUARTPort : public USARTPort {
public:
  typedef enum {
    leuartPortBaudRate4800 = 4800,
    leuartPortBaudRate9600 = 9600
  } LEBaudRate;
  
public:
  bool initialize(uint8_t *rxBuffer, uint8_t rxBufferSize, LEBaudRate baudRate);
  
  void setupSignalFrame(uint8_t signalFrameChar);
  void setupDMA(uint8_t *dmaBuffer, uint8_t dmaBufferSize, uint8_t dmaChannel);
  
  int writeChar(char c);
  int readChar();
  void flushRxBuffer();
  
  void setSignalFrameHook(SigFrameHook h);
  void reactivateDMA(void);
  
  // declare USARTManager as friend class, to be able to use factory pattern
  // and access private interrupt handling functions
  friend class USARTManager;
  
protected: 
  volatile int m_rxReadIndex;
  volatile int m_rxWriteIndex;
  volatile int m_rxCount;
  uint8_t *m_rxBuffer;
  uint8_t m_rxBufferSize;
  uint8_t *m_dmaBuffer;
  uint8_t m_dmaBufferSize;
  uint8_t m_dmaChannel;
  uint8_t m_signalFrameChar;
  SigFrameHook m_sfHook;
  DMA_DESCRIPTOR_TypeDef * m_dmaDescriptor;

  // ------ start of pattern specific section --------
  LEUARTPort(const USARTPortConfig *cfg);
  LEUARTPort(USARTPort const&);                // do not implement
  void operator=(LEUARTPort const&);        // do not implement
  // ------ end of pattern specific section --------
  
  void handleInterrupt();
};

#endif  // UART_H