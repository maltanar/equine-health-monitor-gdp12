// University of Southampton, 2012
// EMECS Group Design Project

#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include "sensor.h"


// define an "invalid temperature" value of sub-zero kelvins
// this is the value provided when the sensor is not yet ready
#define INVALID_TEMPERATURE     -274


// this driver has two modes: raw reading and regular mode
// - raw reading mode outputs the measured Vobj and Tdie values
// - regular mode calculates Tobj and outputs this
// since Tobj calculation involves a lot of double precision floating
// point calculations, it can consume a lot of cycles to calculate
// and may be better done on the base station
#define TMP006_RAW_READING

#ifndef TMP006_RAW_READING
#define TMP006MSG	TemperatureMessage
#define TMP006TYP	typeTemperature
#else
#define TMP006MSG	RawTemperatureMessage
#define TMP006TYP	typeRawTemperature
#endif

// TemperatureSensor class
// Sensor implementation for the TMP006 sensor
class TemperatureSensor : public Sensor {
public:
  // singleton instance accessor
  static TemperatureSensor* getInstance()
  {
    static TemperatureSensor instance;
    return &instance;
  }

  // virtual functions which can be overridden if the sensor supports
  // the functionality
  char setSleepState(bool sleepState);
  void setPeriod(SensorPeriod ms);
  
  // pure virtual functions representing common functionality, which need to be 
  // overridden for all sensors
  void sampleSensorData();
  
  // TMP0006 specific functions
  // read sensor data, make calculations and return Tobj
  double getTemperatureReading();
  // read the manufacturer / device ID registers and return the read value
  uint16_t getManufacturerID(); // should return 0x5449
  uint16_t getDeviceID();       // should return 0x0067
  
protected:
  // ------ start of singleton pattern specific section ------
  TemperatureSensor();  
  TemperatureSensor(TemperatureSensor const&);                // do not implement
  void operator=(TemperatureSensor const&);        // do not implement
  // ------ end of singleton pattern specific section --------
  
  TMP006MSG m_temperatureMessage;	// raw or calculated temperature reading 
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