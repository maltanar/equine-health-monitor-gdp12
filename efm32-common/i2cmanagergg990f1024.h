// University of Southampton, 2012
// EMECS Group Design Project

#ifndef I2CMANAGERGG990F1024_H
#define I2CMANAGERGG990F1024_H

#include "em_i2c.h"

const I2CPortConfig I2CPortConf = 
{
  // port configuration for I2C0, pos3
  .i2cTypeDef = I2C0,
  .i2cLocation = 1,
  .irqNumber = I2C0_IRQn,
  .sclPort = gpioPortD,
  .sclPin = 7,
  .sdaPort = gpioPortD,
  .sdaPin = 6
 };

#endif // I2CMANAGERGG990F1024_H