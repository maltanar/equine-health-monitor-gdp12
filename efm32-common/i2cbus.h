// University of Southampton, 2012
// EMECS Group Design Project

// The I2C module provides a C++ singleton interface to the I2C bus on the
// device

#ifndef I2CBUS_H
#define I2CBUS_H

#include "em_i2c.h"

typedef struct {
  I2C_TypeDef i2cTypeDef;
  uint8_t i2cLocation;
  IRQn irqNumber;
  GPIO_Port_TypeDef sclPort;
  uint8_t sclPin;
  GPIO_Port_TypeDef sdaPort;
  uint8_t sdaPin;
} I2CPortConfig;

class I2CBus {
public:
  static I2CBus* getInstance()
  {
    static I2CBus instance;
    return &instance;
  }
  
  bool readRegister16Bit(uint16_t addr, uint8_t reg, uint16_t *val);
  bool writeRegister16Bit(uint16_t addr, uint8_t reg, uint16_t val);
  
  bool readRegister8Bit(uint16_t addr, uint8_t reg, uint8_t *val);
  bool writeRegister8Bit(uint16_t addr, uint8_t reg, uint8_t val);
  
  // declare the IRQ handler as a friend function, as it needs to access
  // the protected I2C interrupt handler function from this class
  friend void I2C0_IRQHandler(void);
  
private:
  // ------ start of singleton pattern specific section ------
  I2CBus();  
  I2CBus(I2CBus const&);                // do not implement
  void operator=(I2CBus const&);        // do not implement
  // ------ end of singleton pattern specific section --------

protected:
  I2C_TransferReturn_TypeDef m_status; 
  
  void handleI2CInterrupt();
};

#endif // I2CBUS_H