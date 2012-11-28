// University of Southampton, 2012
// EMECS Group Design Project

#include "sensor.h"


Sensor::Sensor(DeviceType sensorType, unsigned int sensorDataLen, 
               SensorPeriod period)
{
  m_sensorType = sensorType;
  m_sensorDataLength = sensorDataLen;
  m_period = period;
}

DeviceType Sensor::getDeviceType()
{
  return m_sensorType;
}

char Sensor::setSleepState(bool sleepState)
{
  m_sleeping = sleepState;
  
  return 1;
}

unsigned int Sensor::getSensorDataLength()
{
  return m_sensorDataLength;
}

void Sensor::setPeriod(SensorPeriod ms)
{
  m_period = ms;
}

SensorPeriod Sensor::getPeriod()
{
  return m_period;
}