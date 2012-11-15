// University of Southampton, 2012
// EMECS Group Design Project

#include <string.h>
#include <stdlib.h>
#include "em_emu.h"
#include "gpssensor.h"
#include "debug_output_control.h"

GPSSensor::GPSSensor(SensorPeriod period) :
  Sensor(sensorTypeGPS, 8, period)
{
  // initialize UART
  m_port = UARTPort::getInstance();
  m_port->initialize(m_msgBuffer, GPS_MSGBUFFER_SIZE, 
                     UARTPort::uartPortBaudRate9600, 
                     UARTPort::uartPortDataBits8, UARTPort::uartPortParityNone, 
                     UARTPort::uartPortStopBits1);
  
  // TODO perform sanity check?
  
  // send NMEA message to restrict rate and wanted message types, as we
  // are not interested in satellites in view etc. notifications
  setPeriod(period);
  configureWantedNMEASentences();
  
  module_debug_gps("initialized with period %d", period);
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
  //module_debug_gps("%s", m_msgBuffer);
}

const void* GPSSensor::readSensorData(uint16_t *actualSize)
{
  // TODO implement
  return 0;
}

void GPSSensor::queryFirmwareVersion()
{
  //  TODO this function assumes we will receive the correct reply message and
  //       it will loop forever if it doesn't - fix this
  
  char type[10], msg[40];
  uint8_t len;
  bool replyReceived = false;
  
  // do a partial initialization of the buffers
  type[0] = 0;
  msg[0] = 0;
  
  // send firmware version query
  sendNMEAString("PMTK605");
  
  // we expect something like this in return:
  // PMTK705,M-core_2.02,8305,Fastrax UP500,
  
  while(!replyReceived)
  {
    receiveNMEAString(type, msg, 40, &len);
    if(strcmp(type, "PMTK705") == 0)
        replyReceived = true;
  }
  
  module_debug_gps("firmware string %s", msg);
}

// gets the GPS fix rate in miliseconds
int GPSSensor::getFixRate()
{
  char type[10], msg[20];
  uint8_t len;
  bool replyReceived = false;
  
  // do a partial initialization of the buffers
  type[0] = 0;
  msg[0] = 0;
  
  // send fix rate query
  sendNMEAString("PMTK400");
  
  // we expect something like this in return:
  // PMTK500,1000,0,0,0,0
  
  while(!replyReceived)
  {
    receiveNMEAString(type, msg, 20, &len);
    if(strcmp(type, "PMTK500") == 0)
        replyReceived = true;
  }
  
  module_debug_gps("received fixrate string %s", msg);
  
  // atoi will parse up to the first non-number character, which is the rate
 
  return atoi(msg);
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
  
  if(waitForReply("PMTK001", "300,3"))
    module_debug_gps("setFixRate OK!");
  else
    module_debug_gps("setFixRate failed!");
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
  
  if(waitForReply("PMTK001", "314,3"))
    module_debug_gps("configureWantedNMEASentences OK!");
  else
    module_debug_gps("configureWantedNMEASentences failed!");
}

// hot restart: restart the GPS with all available data in the NV store
void GPSSensor::hotRestart()
{
  sendNMEAString("PMTK101");
  
  if(waitForReply("PMTK010", "001"))
    module_debug_gps("hotRestart OK!");
  else
    module_debug_gps("hotRestart failed!");
}

// TODO this function assumes we *will* receive the correct reply message and
//       it will loop forever if it doesn't - fix this!
// wait until desired reply message is received
bool GPSSensor::waitForReply(char *compareCmd, char *compareMsg)
{
  char type[10], msg[80];
  uint8_t len;
  
  module_debug_gps("waiting for reply: %s with data %s", compareCmd, compareMsg);
  
  // wait for expected message type
  while(1)
  {
    receiveNMEAString(type, msg, 80, &len);
    
    // TODO we can still receive position fix msgs while waiting for the reply,
    // implement this so we don't lose any info.
    if(len > 0)
      if(strcmp(compareCmd, type) == 0)
        break;
  }
  
  module_debug_gps("got reply: %s with data %s", type, msg);
  
  // compare returned message, if desired
  if(compareMsg != NULL)
    if(strcmp(compareMsg, msg) != 0)
      return false;
  
  return true;
}

void GPSSensor::receiveNMEAString(char *cmdType, char *cmdData, 
                                  uint8_t maxDataLength, 
                                  uint8_t *actualDataLength)
{
  // TODO this function assumes properly formatted NMEA replies will be received
  // in case there is some problem with the UART and we don't get what we
  // expect, this code will hang and loop forever -  fix this
  int currentChar = -1;
  char checksum = 0;
  char * currentCmdCharPtr = cmdType;
  
  if(!cmdType || !cmdData || !actualDataLength)
    return;
  
  *actualDataLength = 0;
  
  // wait for preamble
  while(currentChar != '$')
  {
    currentChar = m_port->readChar();
    EMU_EnterEM2(true);
  }
  
  // wait for command type
  while(currentChar != ',')
  {
    currentChar = m_port->readChar();
    EMU_EnterEM2(true);
    
    if(currentChar != -1 && currentChar != ',')
    {
      // write into cmdType
      *(currentCmdCharPtr++) = (char) currentChar;
    }
  }
  
  // zero-terminate command string
  *(currentCmdCharPtr++) = 0;
  
  // wait for terminating character
  while(currentChar != '*')
  {
    currentChar = m_port->readChar();
    EMU_EnterEM2(true);
    
    if(currentChar != -1 && currentChar != '*')
    {
      // update checksum and write into cmdData
      checksum ^= currentChar;
      if(*actualDataLength < maxDataLength)
        cmdData[(*actualDataLength)++] = (char) currentChar;
    }
  }
  
  // zero terminate received string
  if(*actualDataLength < maxDataLength)
    cmdData[(*actualDataLength)++] = 0;
  
  // receive and validate checksum
  while(currentChar != '\n')
  {
    currentChar = m_port->readChar();
    EMU_EnterEM2(true);
    
    // TODO store received checksum and compare
  }
  
  module_debug_gps("got NMEA %s len %d", cmdType, *actualDataLength);
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
  
  /*module_debug_gps("sendNMEAcommand $%s*%c%c\r\n",data,checksumString[0],
                   checksumString[1]);*/
}