// University of Southampton, 2012
// EMECS Group Design Project

#include <string.h>
#include <stdlib.h>
#include "em_emu.h"
#include "uartmanager.h"
#include "gpssensor.h"
#include "debug_output_control.h"

GPSSensor::GPSSensor() :
  Sensor(sensorTypeGPS, 8, 5000)
{
  // initialize UART
  m_port= UARTManager::getInstance()->getPort(UARTManagerPortLEUART0);
  
  m_port->initialize(m_msgBuffer, GPS_MSGBUFFER_SIZE, 
                     UARTPort::uartPortBaudRate9600, 
                     UARTPort::uartPortDataBits8, UARTPort::uartPortParityNone, 
                     UARTPort::uartPortStopBits1);
  
  // TODO perform sanity check?
  // configure DMA -  TODO maybe make this optional?
  m_port->setupDMA(GPS_DMA_CHANNEL, '\n');
  
  
  // send NMEA message to restrict rate and wanted message types, as we
  // are not interested in satellites in view etc. notifications
  //setPeriod(5000);
  configureWantedNMEASentences();
  
  module_debug_gps("initialized with period %d", 5000);
}

char GPSSensor::setSleepState(bool sleepState)
{
  // TODO implement GPS sleep function
  return 1;
}

void GPSSensor::setPeriod(SensorPeriod ms)
{
  // call parent implementation to take care of the trivial data copying
  Sensor::setPeriod(ms);
  
  // send NMEA message to device to set fix rate
  setFixRate(ms);
}
  
void GPSSensor::sampleSensorData()
{
  // GPS sends data to us at a fixed rate, so there is no real way of "sampling"
  // instead, parse the stored string data
  processNMEAMessage(m_msgBuffer);
}

const void* GPSSensor::readSensorData(uint16_t *actualSize)
{
  // TODO implement
  return 0;
}

void GPSSensor::queryFirmwareVersion()
{ 
  // send firmware version query
  sendNMEAString("PMTK605");
  
  // we expect something like this in return:
  // PMTK705,M-core_2.02,8305,Fastrax UP500,
}

// query the GPS fix rate
void GPSSensor::queryFixRate()
{
  // send fix rate query
  sendNMEAString("PMTK400");
  
  // we expect something like this in return:
  // PMTK500,1000,0,0,0,0
}

// sets the GPS fix rate in miliseconds
void GPSSensor::setFixRate(int fixRateMs)
{
  char msg[30];
  
  // check if fix rate is within allowed range - must be greater than 200ms
  if(fixRateMs < 200)
  {
    module_debug_gps("mix rate %d is too small!", fixRateMs);
    return;
  }
  
  // create and send the relevant NMEA command
  sprintf(msg, "PMTK300,%d,0,0,0,0", fixRateMs);
  sendNMEAString(msg);
}

// set the config flags on the device so that we don't get NMEA sentences
// which we have no use for
void GPSSensor::configureWantedNMEASentences()
{
  /*
  List of NMEA sentences supported by our GPS:
    0 NMEA_SEN_GLL, // GPGLL interval - Geographic Position - Latitude longitude
    1 NMEA_SEN_RMC, // GPRMC interval - Recomended Minimum Specific GNSS Sentence
    2 NMEA_SEN_VTG, // GPVTG interval - Course Over Ground and Ground Speed
    3 NMEA_SEN_GGA, // GPGGA interval - GPS Fix Data
    4 NMEA_SEN_GSA, // GPGSA interval - GNSS DOPS and Active Satellites
    5 NMEA_SEN_GSV, // GPGSV interval - GNSS Satellites in View
    6 NMEA_SEN_GRS, // GPGRS interval - GNSS Range Residuals
    7 NMEA_SEN_GST, // GPGST interval - GNSS Pseudorange Erros Statistics
    13 NMEA_SEN_MALM, // PMTKALM interval - GPS almanac information
    14 NMEA_SEN_MEPH, // PMTKEPH interval - GPS ephmeris information
    15 NMEA_SEN_MDGP, // PMTKDGP interval - GPS differential correction information
    16 NMEA_SEN_MDBG, // PMTKDBG interval – MTK debug information
    17 NMEA_SEN_ZDA, // GPZDA interval – Time & Date
    18 NMEA_SEN_MCHN, // PMTKCHN interval – GPS channel status
  */
  // Here we disable all but the GPRMC sentences
  // TODO make this function more generic to be able to configure other options
  sendNMEAString("PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");
}

// hot restart: restart the GPS with all available data in the NV store
void GPSSensor::hotRestart()
{
  sendNMEAString("PMTK101");
}


// process a buffer as an NMEA message
void GPSSensor::processNMEAMessage(uint8_t * buffer)
{
  // TODO implement message parsing
  // TODO maybe implement checksum control
  // TODO handle corrupt or incorrect messages, don't loop forever
  int i = 0, c = 0;
  uint8_t fieldPos[20];
  
  // find the start sequence
  while(buffer[i] != '$')
    i++;
  fieldPos[c++] = i + 1;
  
  while(buffer[i] != '*')
  {
    // convert commas to zero delimiter & add field
    if(buffer[i] == ',')
    {
      buffer[i] = 0;
      fieldPos[c++] = i + 1;
    }
    i++;
  }
  
  buffer[i] = 0;
  
  module_debug_gps("num of fields %d", c);
  for(i = 0; i < c; i++)
    module_debug_gps("field %d is %s", i, &buffer[fieldPos[i]]);
  
  
  module_debug_gps("%s", buffer);
}

void GPSSensor::sendNMEAString(char *data)
{
  char checksum = 0, checksumString[2];
  
  // send the NMEA preamble, a dollar sign
  m_port->writeChar('$');
  
  // send the data string while calculating the checksum
  int i = 0;
  while(data[i] != 0)
  {
    checksum = checksum ^ data[i];
    m_port->writeChar(data[i]);
    i++;
  }
  
  sprintf(checksumString, "%02x", checksum);
  
  // send the end-of-data sign, an asterisk
  m_port->writeChar('*');
  // send the calculated checksum for the data
  m_port->writeChar(checksumString[0]);
  m_port->writeChar(checksumString[1]);
  // send CRLF to mark end of message
  m_port->writeChar('\r');
  m_port->writeChar('\n');
  
  module_debug_gps("sendNMEAcommand $%s*%c%c\r\n",data,checksumString[0],
                   checksumString[1]);
}