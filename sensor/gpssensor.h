// University of Southampton, 2012
// EMECS Group Design Project

#ifndef GPSSENSOR_H
#define GPSSENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "leuartport.h"
#include "sensor.h"

// GPS Sensor class
// implement UART-based interface for Fastrax GPS sensor

// sets the size of the message buffer into which we receive raw NMEA strings
#define GPS_MSGBUFFER_SIZE      80

// TODO maybe this should be defined in a more global header file?
// the DMA channel used to receive data from the UART for the GPS
#define USE_GPS_DMA
#define DMA_CHANNEL_GPS         0

class GPSSensor : public Sensor {
public:
  // singleton instance accessor
  static GPSSensor* getInstance()
  {
    static GPSSensor instance;
    return &instance;
  }
  
  static void configurePower();
  static void setPower(bool vccOn, bool vbatOn);
	  
  
  // virtual functions which can be overridden if the sensor supports
  // the functionality
  char setSleepState(bool sleepState);
  void setPeriod(SensorPeriod ms);
  
  // pure virtual functions representing common functionality, which need to be 
  // overridden for all sensors
  void sampleSensorData();
  
  // GPS driver control
  void setParseOnReceive(bool enable);
  
  // GPS module control commands
  void hotRestart();
  void queryFirmwareVersion();
  void queryFixRate();
  void setFixRate(int fixRateMs);
  void configureWantedNMEASentences();  // set the config flags on the device 
                                        // so that we don't get NMEA sentences
                                        // which we have no use for
  
  friend void gpsSignalFrameHandler(uint8_t *buf);
  
private:
  // ------ start of singleton pattern specific section ------
  GPSSensor();  
  GPSSensor(GPSSensor const&);                // do not implement
  void operator=(GPSSensor const&);        // do not implement
  // ------ end of singleton pattern specific section --------
  
  uint8_t m_msgBuffer[GPS_MSGBUFFER_SIZE];      // message buffer into which we receive raw NMEA strings
  LEUARTPort * m_port;                            // the UART IF for the GPS
  GPSMessage m_gpsMessage;
  
  // process a buffer as an NMEA message
  void processNMEAMessage(uint8_t * buffer);
  // send an NMEA command string. adds preamble, checksum, terminator and crlf 
  void sendNMEAString(char *data);
};


#endif // GPSSENSOR_H