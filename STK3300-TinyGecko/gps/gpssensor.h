// University of Southampton, 2012
// EMECS Group Design Project

#ifndef GPSSENSOR_H
#define GPSSENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "uartport.h"
#include "sensor.h"

// GPS Sensor class
// implement UART-based interface for Fastrax GPS sensor

// sets the size of the message buffer into which we receive raw NMEA strings
#define GPS_MSGBUFFER_SIZE      80

class GPSSensor : public Sensor {
public:
  GPSSensor(SensorPeriod period);
  
  // virtual functions which can be overridden if the sensor supports
  // the functionality
  char setSleepState(bool sleepState);
  void setPeriod(SensorPeriod ms);
  
  // pure virtual functions representing common functionality, which need to be 
  // overridden for all sensors
  void sampleSensorData();
  const void* readSensorData(uint16_t *actualSize);
  
  // GPS module control commands
  void queryFirmwareVersion();
  int getFixRate();
  void setFixRate(int fixRateMs);
  
  // send an NMEA command string. adds preamble, checksum, terminator and crlf 
  void sendNMEAString(char *data);
  // receive an NMEA string
  void receiveNMEAString(char *cmdType, char *cmdData, 
                                  uint8_t maxDataLength, 
                                  uint8_t *actualDataLength);
  // set the config flags on the device so that we don't get NMEA sentences
  // which we have no use for
  void configureWantedNMEASentences();
  // send the NMEA hot start command
  void hotRestart();
  
  
protected:
  uint8_t m_msgBuffer[GPS_MSGBUFFER_SIZE];      // message buffer into which we receive raw NMEA strings
  UARTPort * m_port;                            // the UART IF for the GPS
  
  bool waitForReply(char *compareCmd, char *compareMsg);
};


#endif // GPSSENSOR_H