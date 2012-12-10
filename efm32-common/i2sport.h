// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __I2SPort_H
#define __I2SPort_H

#include <stdint.h>
#include <stdbool.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include "usartport.h"

class I2SPort : public USARTPort {  
public:
  bool initialize();

//	turn on/off AUTO_TX mode
  void enable(bool active);
  
  int writeChar(char c);
  short writeShort(short c);
  int readChar();
  void waitWriteComplete();
  
  // declare USARTManager as friend class, to be able to use factory pattern
  // and access private interrupt handling functions
  friend class USARTManager;
  
private: 

  // ------ start of pattern specific section --------
  I2SPort(const USARTPortConfig *cfg);
  I2SPort(I2SPort const&);                // do not implement
  void operator=(I2SPort const&);        // do not implement
  // ------ end of pattern specific section --------
  
  void handleInterrupt();
};

#endif  // I2SPort_H