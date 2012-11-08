// University of Southampton, 2012
// EMECS Group Design Project

#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include "sensor.h"

// define an "invalid temperature" value of sub-zero kelvins
// this is the value provided when the sensor is not yet ready
#define INVALID_TEMPERATURE     -274

// TemperatureSensor class
// Sensor implementation for the TMP006 sensor
class TemperatureSensor : public Sensor {
public:
  TemperatureSensor(SensorPeriod period);

  // virtual functions which can be overridden if the sensor supports
  // the functionality
  char setSleepState(bool sleepState);
  
  // pure virtual functions representing common functionality, which need to be 
  // overridden for all sensors
  char readSensor(void *dataBuffer);
  void setPeriod(SensorPeriod ms);
  
  // TMP0006 specific functions
  // read sensor data, make calculations and return Tobj
  double getTemperatureReading();
  // read the manufacturer / device ID registers and return the read value
  uint16_t getManufacturerID(); // should return 0x5449
  uint16_t getDeviceID();       // should return 0x0067
  
protected:
  double m_temp;           // Calculated temperature reading 
  int16_t m_rate;             // conversion rate flags for internal config
  
  // internal helper functions to read-write TMP006 registers
  int16_t readRegister(unsigned char reg);
  void writeRegister(unsigned char reg, unsigned int val);
  // calculate temperature given the current die temp and Vobj measurements
  double calculateTemp(double * tDie, double * vObj);
  // return the measurement ready bit from the configuration register
  bool isMeasurementReady();

};


#endif // TEMPERATURESENSOR_H