// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __ANTHRMSENSOR_H
#define __ANTHRMSENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "uartport.h"
#include "sensor.h"

#define ANTHRM_MSGBUFFER_SIZE   40
#define ANTHRM_DEFAULT_RATE     4000

class ANTHRMSensor : public Sensor {
public:
  // singleton instance accessor
  static ANTHRMSensor* getInstance()
  {
    static ANTHRMSensor instance;
    return &instance;
  }
  
  // virtual functions which can be overridden if the sensor supports
  // the functionality
  char setSleepState(bool sleepState);
  void setPeriod(SensorPeriod ms);
  
  // pure virtual functions representing common functionality, which need to be 
  // overridden for all sensors
  void sampleSensorData();
  const void* readSensorData(uint16_t *actualSize);
  
  // ANT HRM module control commands
  // TODO reset
  // TODO device ID
  // TODO connection status
  
  
private:
  // ------ start of singleton pattern specific section ------
  ANTHRMSensor();  
  ANTHRMSensor(ANTHRMSensor const&);              // do not implement
  void operator=(ANTHRMSensor const&);            // do not implement
  // ------ end of singleton pattern specific section --------
  
  uint8_t m_msgBuffer[ANTHRM_MSGBUFFER_SIZE];   // message buffer into which we receive ANT msgs
  UARTPort * m_port;                            // the UART IF for the ANT HRM
  HeartRateMessage m_hrmMessage;
  
  // process one char received from UART
  void processUARTRxChar(uint8_t c);
  // process a buffer
  void processANTMessage(uint8_t * buffer);
  // send an ANT message. adds preamble, checksum, etc.
  void sendANTMessage(char *data);
};

#endif  // __ANTHRMSENSOR_H