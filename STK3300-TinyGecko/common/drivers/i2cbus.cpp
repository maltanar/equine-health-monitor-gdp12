#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "i2cbus.h"
#include "debug_output_control.h"

void I2C0_IRQHandler(void)
{
  I2CBus::getInstance()->handleI2CInterrupt();
}

I2CBus::I2CBus()
{
  module_debug_i2c("instance constructed!");
  
  // initialize I2C driver
  // TODO make I2C location and index configurable?
  // TODO GIANTGECKO this bit will have to change to use correct pin/location
  I2C_Init_TypeDef i2cInit = I2C_INIT_DEFAULT;

  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_I2C0, true);

  /* Use location 3: SDA - Pin D6, SCL - Pin D7 */
  /* Output value must be set to 1 to not drive lines low... We set */
  /* SCL first, to ensure it is high before changing SDA. */
  GPIO_PinModeSet(gpioPortD, 7, gpioModeWiredAnd, 1);
  GPIO_PinModeSet(gpioPortD, 6, gpioModeWiredAnd, 1);

  /* In some situations (after a reset during an I2C transfer), the slave */
  /* device may be left in an unknown state. Send 9 clock pulses just in case. */
  for (int i = 0; i < 9; i++)
  {
    /*
     * TBD: Seems to be clocking at appr 80kHz-120kHz depending on compiler
     * optimization when running at 14MHz. A bit high for standard mode devices,
     * but DVK only has fast mode devices. Need however to add some time
     * measurement in order to not be dependable on frequency and code executed.
     */
    GPIO_PinModeSet(gpioPortD, 7, gpioModeWiredAnd, 0);
    GPIO_PinModeSet(gpioPortD, 7, gpioModeWiredAnd, 1);
  }

  /* Enable pins at location 1 */
  I2C0->ROUTE = I2C_ROUTE_SDAPEN |
                I2C_ROUTE_SCLPEN |
                (1 << _I2C_ROUTE_LOCATION_SHIFT);

  I2C_Init(I2C0, & i2cInit);
  
  NVIC_ClearPendingIRQ(I2C0_IRQn);
  NVIC_EnableIRQ(I2C0_IRQn);
}

void I2CBus::handleI2CInterrupt()
{
  // Just run the I2C_Transfer function that checks interrupts flags and returns
  // the appropriate status
  m_status = I2C_Transfer(I2C0);
  module_debug_i2c("interrupt! status=%d", m_status);
}

bool I2CBus::readRegister16Bit(uint16_t addr, uint8_t reg, uint16_t *val)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t data[2], addr_data[1];
  
  // check for invalid read buffer pointer
  if(val == NULL)
    return false;
  
  module_debug_i2c("read from addr %x reg %x", addr, reg);

  seq.addr = addr;
  seq.flags = I2C_FLAG_WRITE_READ;
  // Select register to be read
  addr_data[0] = (uint8_t) reg;
  seq.buf[0].data = addr_data;
  seq.buf[0].len = 1;
  // Select location/length to place register
  seq.buf[1].data = data;
  seq.buf[1].len = 2;

  // Do a polled transfer 
  m_status = I2C_TransferInit(I2C0, &seq);
  while (m_status == i2cTransferInProgress)
  {
    // Enter EM1 while waiting for I2C interrupt
    // TODO is this a good idea to do inside a generic driver module?
    // TODO add timeout function here?
    EMU_EnterEM1();
  }
  
  if (m_status != i2cTransferDone)
  {
    module_debug_i2c("error reading, status %d", m_status);
    return false;
  } else {
  	*val = (((uint16_t)(data[0])) << 8) | data[1];
	module_debug_i2c("read value %x from register %x", *val, reg);
  }
  
  return true;
}

bool I2CBus::writeRegister16Bit(uint16_t addr, uint8_t reg, uint16_t val)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t data[3];
  
  module_debug_i2c("write to addr %x reg %x val %x", addr, reg, val);

  seq.addr = addr;
  seq.flags = I2C_FLAG_WRITE;
  // Select register to be written
  data[0] = ((uint8_t)reg);
  seq.buf[0].data = data;
  data[1] = (uint8_t)(val >> 8);
  data[2] = (uint8_t)val;
  seq.buf[0].len = 3;

  // Do a polled transfer
  m_status = I2C_TransferInit(I2C0, &seq);
  while (m_status == i2cTransferInProgress)
  {
    // Enter EM1 while waiting for I2C interrupt
    // TODO is this a good idea to do inside a generic driver module?
    // TODO add timeout function here?
    EMU_EnterEM1();
  }
  
  if (m_status != i2cTransferDone)
  {
    module_debug_i2c("error writing, status %d", m_status);
    return false;
  } 
  else
    return true;
    
}

bool I2CBus::readRegister8Bit(uint16_t addr, uint8_t reg, uint8_t *val)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t data[1], addr_data[1];
  
  // check for invalid read buffer pointer
  if(val == NULL)
    return false;
  
  module_debug_i2c("8 bit read from addr %x reg %x", addr, reg);

  seq.addr = addr;
  seq.flags = I2C_FLAG_WRITE_READ;
  // Select register to be read
  addr_data[0] = (uint8_t) reg;
  seq.buf[0].data = addr_data;
  seq.buf[0].len = 1;
  // Select location/length to place register
  seq.buf[1].data = data;
//  seq.buf[1].len = 2;
  seq.buf[1].len = 1;

  // Do a polled transfer 
  m_status = I2C_TransferInit(I2C0, &seq);
  while (m_status == i2cTransferInProgress)
  {
    // Enter EM1 while waiting for I2C interrupt
    // TODO is this a good idea to do inside a generic driver module?
    // TODO add timeout function here?
    EMU_EnterEM1();
  }
  
  if (m_status != i2cTransferDone)
  {
    module_debug_i2c("error reading 8 bit register %x, status %d", reg, m_status);
    return false;
  }

  *val = data[0];
  

  return true;
}

bool I2CBus::writeRegister8Bit(uint16_t addr, uint8_t reg, uint8_t val)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t data[2];
  
  module_debug_i2c("8 bit write to addr %x reg %x val %x", addr, reg, val);

  seq.addr = addr;
  seq.flags = I2C_FLAG_WRITE;
  // Select register to be written
  data[0] = ((uint8_t)reg);
  data[1] = val ;
  seq.buf[0].data = data;
  seq.buf[0].len = 2;

  // Do a polled transfer
  m_status = I2C_TransferInit(I2C0, &seq);
  while (m_status == i2cTransferInProgress)
  {
    // Enter EM1 while waiting for I2C interrupt
    // TODO is this a good idea to do inside a generic driver module?
    // TODO add timeout function here?
    EMU_EnterEM1();
  }
  
  if (m_status != i2cTransferDone)
  {
    module_debug_i2c("error writing 8 bits to %x, status %d", reg, m_status);
    return false;
  } 
  else
    return true;
    
}