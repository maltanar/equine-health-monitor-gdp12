// University of Southampton, 2012
// EMECS Group Design Project

#include <math.h>
#include <stdio.h>
#include "em_i2c.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "temperaturesensor.h"
#include "debug_output_control.h"

// TMP006 Internal Pointer Register Addresses
#define TMP006_P_VOBJ       0x00        // TMP006 object voltage register pointer 
#define TMP006_P_TABT       0x01        // TMP006 ambient temperature register pointer 
#define TMP006_P_WRITE_REG  0x02        // TMP006 configuration register pointer 
#define TMP006_P_MAN_ID     0xFE        // TMP006 manufacturer ID register pointer 
#define TMP006_P_DEVICE_ID  0xFF        // TMP006 device ID register pointer 


// TMP006 Configuration Register Bits
#define TMP006_RST          0x8000
#define TMP006_POWER_DOWN   0x0000
#define TMP006_POWER_UP     0x7000
#define TMP006_CR_4         0x0000
#define TMP006_CR_2         0x0200
#define TMP006_CR_1         0x0400
#define TMP006_CR_0_5       0x0600
#define TMP006_CR_0_25      0x0800
#define TMP006_EN           0x0100
#define TMP006_DRDY         0x0080
/*! @} */
  
// I2C device address for temperature sensor 
// this is set by the value of the ADR* pins 
// which are always 0 for the breakout board 
#define TMP006_I2C_ADDR     0x80


// TODO move the code below to I2C module ------------------------------
I2C_TransferReturn_TypeDef I2C_Status; 
void I2C0_IRQHandler(void)
{
  /* Just run the I2C_Transfer function that checks interrupts flags and returns */
  /* the appropriate status */
  I2C_Status = I2C_Transfer(I2C0);
}
// TODO move the code above to I2C module ------------------------------

TemperatureSensor::TemperatureSensor(SensorPeriod period) 
 : Sensor(sensorTypeTemperature, 8, period)
{
  m_temp = 0;
  m_rate = 0;
  
  // initialize I2C driver
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

  // Reset TMP006
  writeRegister(TMP006_P_WRITE_REG, TMP006_RST);
  // power-up
  writeRegister(TMP006_P_WRITE_REG, TMP006_POWER_UP);
  
  // do a sanity check on the sensor using the manufacturer and device ID
  uint16_t mid = getManufacturerID(), did = getDeviceID();
  if(mid != 0x5449 || did != 0x0067)
    module_debug_temp("Unknown manufacturer/device id: %x / %x", mid, did);
  else
    module_debug_temp("TMP006 manufacturer/device id OK: %x / %x", mid, did);
  
  // set conversion rate
  setPeriod(period);
  
}

void TemperatureSensor::setPeriod(SensorPeriod ms)
{
  Sensor::setPeriod(ms);
  
  // also need to set sensor config flags to set conversion rate
  // determine which of the fixed conversion rate settings to use
  // we can use ones smaller than requested to ensure samples are ready by then
  if(ms < 500)
     m_rate = TMP006_CR_4;
  else if(ms >= 500 && ms < 1000)
     m_rate = TMP006_CR_2;
  else if(ms >= 1000 && ms < 2000)
     m_rate = TMP006_CR_1;
  else if(ms >= 2000 && ms < 4000)
     m_rate = TMP006_CR_0_5;
  else if(ms >= 4000)
     m_rate = TMP006_CR_0_25;
  else
    module_debug_temp("Unknown period: %x \n", ms);
  
  module_debug_temp("period: %d rate bits %x", ms, m_rate);

  // read the configuration register, filter out current conversion rate bits
  int16_t settings = readRegister(TMP006_P_WRITE_REG) & ~(0x0E00);
  // insert the new conversion rate setting bits
  settings |= m_rate;
  // write the configuration register
  writeRegister(TMP006_P_WRITE_REG, settings);
}

char TemperatureSensor::readSensor(void *dataBuffer)
{
  // TODO get data from the temperature sensor
  return 0;
}

// Manufacturer ID should return 0x5449
uint16_t TemperatureSensor::getManufacturerID()
{
  return readRegister(TMP006_P_MAN_ID);
}

// Device ID should return 0x0067
uint16_t TemperatureSensor::getDeviceID(void)
{
  return readRegister(TMP006_P_DEVICE_ID);
}

int16_t TemperatureSensor::readRegister(unsigned char reg)
{
  // TODO move I2C specific code into own module?
  I2C_TransferSeq_TypeDef seq;
  uint8_t data[2], addr_data[1];
  
  module_debug_temp("read %x", reg);

  seq.addr = TMP006_I2C_ADDR;
  seq.flags = I2C_FLAG_WRITE_READ;
  /* Select register to be read */
  addr_data[0] = (uint8_t) reg;
  seq.buf[0].data = addr_data;
  seq.buf[0].len = 1;
  /* Select location/length to place register */  
  seq.buf[1].data = data;
  seq.buf[1].len = 2;

  /* Do a polled transfer */
  I2C_Status = I2C_TransferInit(I2C0, &seq);
  while (I2C_Status == i2cTransferInProgress)
  {
    /* Enter EM1 while waiting for I2C interrupt */
    // TODO energy saving!
    EMU_EnterEM1();
    /* Could do a timeout function here? */
  }
  
  if (I2C_Status != i2cTransferDone)
  {
    module_debug_temp("error reading, status %x", I2C_Status);
    return -1;
  }

  return (((uint16_t)(data[0])) << 8) | data[1];
}

void TemperatureSensor::writeRegister(unsigned char reg, unsigned int val)
{
  I2C_TransferSeq_TypeDef seq;
  uint8_t data[3];
  
  module_debug_temp("write to reg %x val %x \n", reg, val);

  seq.addr = TMP006_I2C_ADDR;
  seq.flags = I2C_FLAG_WRITE;
  /* Select register to be written */
  data[0] = ((uint8_t)reg);
  seq.buf[0].data = data;
  data[1] = (uint8_t)(val >> 8);
  data[2] = (uint8_t)val;
  seq.buf[0].len = 3;

  /* Do a polled transfer */
  I2C_Status = I2C_TransferInit(I2C0, &seq);
  while (I2C_Status == i2cTransferInProgress)
  {
    /* Enter EM1 while waiting for I2C interrupt */
    // TODO energy saving!
    EMU_EnterEM1();
    /* Could do a timeout function here. */
  }
  
  if (I2C_Status != i2cTransferDone)
  {
    module_debug_temp("error writing, status %x", I2C_Status);
  }
}

double TemperatureSensor::calculateTemp(double * tDie, double * vObj)
{
  // Calculate Tobj, based on data/formula from TI
  double S0 = 0.00000000000006;       /* Default S0 cal value,  6 * pow(10, -14) */
  double a1 = 0.00175; /* 1.75 * pow(10, -3) */
  double a2 = -0.00001678; /* -1.678 * pow(10, -5) */
  double b0 = -0.0000294; /* -2.94 * pow(10, -5) */
  double b1 = -0.00000057;  /* -5.7*pow(10, -7); */
  double b2 = 0.00000000463 /* 4.63*pow(10, -9) */;
  double c2 = 13.4;
  double Tref = 298.15;
  double S = S0*(1+a1*(*tDie - Tref)+a2*pow((*tDie - Tref),2));
  double Vos = b0 + b1*(*tDie - Tref) + b2*pow((*tDie - Tref),2);
  double fObj = (*vObj - Vos) + c2*pow((*vObj - Vos),2);
  double Tobj = pow(pow(*tDie,4) + (fObj/S), (double).25) - 273.15;
  
  module_debug_temp("Vobj: %f", *vObj);
  module_debug_temp("Tdie: %f", *tDie);
  module_debug_temp("Temp: %f", Tobj);
  
  return Tobj;
}

bool TemperatureSensor::isMeasurementReady()
{
  // Read DRDY status from Config register
  if (TMP006_DRDY & readRegister(TMP006_P_WRITE_REG))
    return true;
  else
    return false;
}

double TemperatureSensor::getTemperatureReading()
{
    double vObjcorr = 0, tDieKelvin = 0;
    int vObj = 0, tDie = 0;
    
    // return a special invalid value if the sensor is not yet ready
    if(!isMeasurementReady())
      return INVALID_TEMPERATURE;

    // Read the object voltage. Assuming that the data is ready.
    vObj = readRegister(TMP006_P_VOBJ);
    // Read the ambient temperature
    tDie = readRegister(TMP006_P_TABT);

    // Convert latest tDie measurement to Kelvin
    tDieKelvin = (((double)(tDie >> 2)) * .03125) + 273.15;
    vObjcorr = ((double)(vObj)) * .00000015625;

    // call helper function to make the final Tobj calculation
    m_temp = calculateTemp(&tDieKelvin, &vObjcorr);
    return m_temp;
}

char TemperatureSensor::setSleepState(bool sleepState)
{
  // read the configuration register
  unsigned int settings = readRegister(TMP006_P_WRITE_REG);

  // adjust the power down bit depending on the sleepState param
  if(sleepState)
    settings &= ~(TMP006_POWER_UP);
  else
    settings |= TMP006_POWER_UP;

  // write the configuration register
  writeRegister(TMP006_P_WRITE_REG, settings);
  
  return Sensor::setSleepState(sleepState);
}