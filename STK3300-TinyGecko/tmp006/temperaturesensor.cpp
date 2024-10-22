// University of Southampton, 2012
// EMECS Group Design Project

#include <math.h>
#include <stdio.h>
#include "em_cmu.h"
#include "em_emu.h"
#include "temperaturesensor.h"
#include "i2cbus.h"
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

TemperatureSensor::TemperatureSensor(SensorPeriod period)
 : Sensor(sensorTypeTemperature, 8, period)
{
  m_temp = 0;
  m_rate = 0;

  // TODO initialize I2C driver here

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
    module_debug_temp("Unknown period: %x", ms);

  module_debug_temp("period: %d rate bits %x", ms, m_rate);

  // read the configuration register, filter out current conversion rate bits
  int16_t settings = readRegister(TMP006_P_WRITE_REG) & ~(0x0E00);
  // insert the new conversion rate setting bits
  settings |= m_rate;
  // write the configuration register
  writeRegister(TMP006_P_WRITE_REG, settings);
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
  uint16_t val;

  module_debug_temp("read %x", reg);

  if(I2CBus::getInstance()->readRegister16Bit(TMP006_I2C_ADDR, reg, &val))
    return val;
  else {
    module_debug_temp("failed to get register %x", reg);
    return -1;
  }
}

void TemperatureSensor::writeRegister(unsigned char reg, unsigned int val)
{
  module_debug_temp("write %x to %x", val, reg);

  if(!I2CBus::getInstance()->writeRegister16Bit(TMP006_I2C_ADDR, reg, val))
    module_debug_temp("failed to write register %x", reg);
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

void TemperatureSensor::sampleSensorData()
{
  double vObjcorr = 0, tDieKelvin = 0;
  int vObj = 0, tDie = 0;

  // do not modify previous sample is new data is not ready
  if(!isMeasurementReady())
  {
    return;
  }

  // Read the object voltage. Assuming that the data is ready.
  vObj = readRegister(TMP006_P_VOBJ);
  // Read the ambient temperature
  tDie = readRegister(TMP006_P_TABT);

  // Convert latest tDie measurement to Kelvin
  tDieKelvin = (((double)(tDie >> 2)) * .03125) + 273.15;
  vObjcorr = ((double)(vObj)) * .00000015625;

  // call helper function to make the final Tobj calculation
  m_temp = calculateTemp(&tDieKelvin, &vObjcorr);
}

const void* TemperatureSensor::readSensorData(uint16_t *actualSize)
{
  *actualSize = sizeof(m_temp);
  return (const void*) &m_temp;
}

double TemperatureSensor::getTemperatureReading()
{
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