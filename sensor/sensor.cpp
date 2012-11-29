// University of Southampton, 2012
// EMECS Group Design Project

#include "sensor.h"


Sensor::Sensor(DeviceType sensorType, unsigned int sensorDataLen, 
               SensorPeriod period)
{
  m_sensorType = sensorType;
  m_sensorDataLength = sensorDataLen;
  m_period = period;
  m_sensorMessage.sensorType = m_sensorType;
  m_sensorMessage.arrayLength = 0;
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
  m_sensorMessage.sampleIntervalMs = ms;
}

SensorPeriod Sensor::getPeriod()
{
  return m_period;
}