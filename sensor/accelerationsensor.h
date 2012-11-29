// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __ACCELERATIONSENSOR_H
#define __ACCELERATIONSENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "sensor.h"
#include "messagetypes.h"

// ADXL350 configuration definitions
#define ADXL350_DEVICE_ID		0xE5
#define ADXL350_DEVICE_ADDR		0x3A
#define ADXL350_I2C_RETRYCNT	10

// ADXL350 register address definitions
#define ADXL350_REG_DEVID		0x00
#define ADXL350_REG_PWCTL		0x2D
#define ADXL350_REG_INTEN		0x2E
#define ADXL350_REG_FIFOC		0x38
#define ADXL350_REG_DATAF		0x31
#define ADXL350_REG_BWRAT		0x2C
#define ADXL350_REG_OFSX		0x1E
#define ADXL350_REG_OFSY		0x1F
#define ADXL350_REG_OFSZ		0x20
#define ADXL350_REG_XLSB		0x32
#define ADXL350_REG_XMSB		0x33
#define ADXL350_REG_YLSB		0x34
#define ADXL350_REG_YMSB		0x35
#define ADXL350_REG_ZLSB		0x36
#define ADXL350_REG_ZMSB		0x37

#define ACCL_MAX_SAMPLES		10

// AccelerationSensor class
// Sensor implenentation for the ADXL350 sensor
class AccelerationSensor : public Sensor
{
public:
  // enumeration for ADXL350 FIFO control
  enum AccelerationSensorFIFOMode {
	fifoModeBypass = 0x00,			// do not use FIFO
	fifoModeStream = 0x40,			// discard old samples if FIFO fills up
	fifoModeStopWhenFull = 0x80,	// stop sampling when FIFO fills up
	fifoModeTrigger	= 0x90			// discard old data based on interrupt
  };
public:
  AccelerationSensor(SensorPeriod period);
  
  // virtual functions which can be overridden if the sensor supports
  // the functionality
  char setSleepState(bool sleepState);
  void setPeriod(SensorPeriod ms);
  
  // pure virtual functions representing common functionality, which need to be 
  // overridden for all sensors
  void sampleSensorData();
  const void* readSensorData(uint16_t *actualSize);
  
  // TMP0006 specific functions
  // read the device ID register and return the read value
  uint8_t getDeviceID();	// should return ADXL350_DEVICE_ID
  // change how the device FIFO operates
  bool setFIFOMode(AccelerationSensorFIFOMode mode);
  void discardOldSamples();
  
protected:
  AccelerometerMessage m_accSampleBuffer[ACCL_MAX_SAMPLES];
  
  // internal helper functions to read-write ADXL350 registers
  bool readRegister(uint8_t reg, uint8_t * val);
  bool writeRegister(uint8_t reg, uint8_t val);
};


#endif	// __ACCELERATIONSENSOR_H