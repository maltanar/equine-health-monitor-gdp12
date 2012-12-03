// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __USARTPORT_H
#define __USARTPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <em_cmu.h>
#include <em_gpio.h>


typedef struct {
  CMU_Clock_TypeDef clockPoint;
  IRQn irqNumber;
  USART_TypeDef * usartBase;
  uint32_t routeLocation;
  GPIO_Port_TypeDef txPort;
  uint8_t txPin;
  GPIO_Port_TypeDef rxPort;
  uint8_t rxPin;
  GPIO_Port_TypeDef sclkPort;
  uint8_t sclkPin;
  GPIO_Port_TypeDef csPort;
  uint8_t csPin;
  bool async;
  bool lowEnergy;
} USARTPortConfig;

typedef bool(*RxHook)(uint8_t c);

class USARTPort {
public:
  void setRxHook(RxHook h);
  
  virtual int writeChar(char c) = 0;
  virtual int readChar() = 0;
  
  // declare USARTManager as friend class, to be able to use factory pattern
  // and access private interrupt handling functions
  friend class USARTManager;
  
protected: 
  bool m_initialized;
  RxHook m_rxHook;
  const USARTPortConfig * m_portConfig;

  // ------ start of pattern specific section --------
  USARTPort(const USARTPortConfig *cfg);
  USARTPort(USARTPort const&);                // do not implement
  void operator=(USARTPort const&);        // do not implement
  // ------ end of pattern specific section --------
  
  virtual void handleInterrupt() = 0;
};

#endif  // UART_H