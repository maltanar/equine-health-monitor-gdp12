// University of Southampton, 2012
// EMECS Group Design Project

#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "messagetypes.h"

// Sensor class
// common functionality for all sensors we use within the GDP
// subclasses need to call constructor with appropriate type information
// and override the (pure) virtual functions

typedef unsigned short SensorPeriod;

class Sensor {
public:
  Sensor(SensorType sensorType, unsigned int sensorDataLen, 
               SensorPeriod period);
  
  // base sensor data access functions, no need to override these
  SensorType getSensorType();
  unsigned int getSensorDataLength();
  
  // virtual functions which can be overridden if the sensor supports
  // the functionality
  virtual char setSleepState(bool sleepState);
  virtual void setPeriod(SensorPeriod ms);
  virtual SensorPeriod getPeriod();
  
  // pure virtual functions representing common functionality, which need to be 
  // overridden for all sensors
  virtual void sampleSensorData() = 0;
  virtual const void* readSensorData(uint16_t *actualSize) = 0;
  
protected:
  SensorType m_sensorType;
  unsigned int m_sensorDataLength;
  bool m_sleeping;
  SensorPeriod m_period;
    
    
};


#endif // SENSOR_H