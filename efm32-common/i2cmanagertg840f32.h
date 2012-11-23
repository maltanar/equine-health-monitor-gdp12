// University of Southampton, 2012
// EMECS Group Design Project

#ifndef I2CMANAGERTG840F32_H
#define I2CMANAGERTG840F32_H

const I2CPortConfig I2CPortConf = 
{
  // port configuration for I2C0, pos1
  .i2cTypeDef = I2C0;
  .i2cLocation = 1;
  .irqNumber = I2C0_IRQn;
  .sclPort = gpioPortD;
  .sclPin = 7;
  .sdaPort = gpioPortD;
  .sdaPin = 6;
};

#else
#error "i2cmanager.h: PART NUMBER undefined"
#endif

#endif // I2CMANAGERTG840F32_H