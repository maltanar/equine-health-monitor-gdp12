// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __UARTPORT_H
#define __UARTPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include "usartport.h"

class UARTPort : public USARTPort {
public:
  enum BaudRate {
    uartPortBaudRate4800 = 4800,
    uartPortBaudRate9600 = 9600,
    uartPortBaudRate115200 = 115200,
  };
  
  enum Parity {
    uartPortParityNone,
    uartPortParityOdd,
    uartPortParityEven
  };
  
  enum DataBits {
    uartPortDataBits8,
    uartPortDataBits9,
  };
  
  enum StopBits {
    uartPortStopBits1,
    uartPortStopBits2
  };
  
public:
  bool initialize(uint8_t *rxBuffer, uint8_t rxBufferSize, BaudRate baudRate, 
                  DataBits dataBits, Parity parity, StopBits stopBits);
  
  int writeChar(char c);
  int readChar();
  void flushRxBuffer();
  
  // declare USARTManager as friend class, to be able to use factory pattern
  // and access private interrupt handling functions
  friend class USARTManager;
  
private: 
  volatile int m_rxReadIndex;
  volatile int m_rxWriteIndex;
  volatile int m_rxCount;
  uint8_t *m_rxBuffer;
  uint8_t m_rxBufferSize;
  uint8_t m_dmaChannel;
  uint8_t m_signalFrameChar;

  // ------ start of pattern specific section --------
  UARTPort(const USARTPortConfig *cfg);
  UARTPort(UARTPort const&);                // do not implement
  void operator=(UARTPort const&);        // do not implement
  // ------ end of pattern specific section --------
  
  void handleInterrupt();
};

#endif  // UARTPORT_H