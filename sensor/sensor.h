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
// and override the (pure) virtual functions as needed

typedef unsigned short SensorPeriod;

class Sensor {
  public:
	// each sensor implementation is to provide sensor type, data size of
	// one sample and the default sampling period at construction
	Sensor(DeviceType sensorType, unsigned int sensorDataLen, 
	SensorPeriod period);

	// base sensor data access functions, no need to override these
	DeviceType getDeviceType();
	unsigned int getSensorDataLength();

	// virtual functions which can be overridden if the sensor needs
	// to modify the base functionality:
	// put the device to sleep or wake it up
	virtual char setSleepState(bool sleepState);
	// set/get sampling period
	virtual void setPeriod(SensorPeriod ms);
	virtual SensorPeriod getPeriod();
	// return the latest sampled data from the sensor, put the generated
	// data length into actualSize
	virtual const void* readSensorData(uint16_t *actualSize);

	// pure virtual functions representing common functionality, which need 
	//to be verridden for all sensors:
	// acquire a sample into interal buffers
	virtual void sampleSensorData() = 0;

  protected:
	DeviceType m_sensorType;
	unsigned int m_sensorDataLength;
	bool m_sleeping;
	SensorPeriod m_period;
	SensorMessage m_sensorMessage;
    
    
};


#endif // SENSOR_H