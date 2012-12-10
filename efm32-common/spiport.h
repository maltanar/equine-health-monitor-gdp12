// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __SPIPort_H
#define __SPIPort_H

#include <stdint.h>
#include <stdbool.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include "usartport.h"

class SPIPort : public USARTPort {  
public:
  bool initialize();
  
  int writeChar(char c);
  short writeShort(short c);
  int readChar();
  void waitWriteComplete();
  
  // declare USARTManager as friend class, to be able to use factory pattern
  // and access private interrupt handling functions
  friend class USARTManager;
  
private: 

  // ------ start of pattern specific section --------
  SPIPort(const USARTPortConfig *cfg);
  SPIPort(SPIPort const&);                // do not implement
  void operator=(SPIPort const&);        // do not implement
  // ------ end of pattern specific section --------
  
  void handleInterrupt();
};

#endif  // SPIPort_H