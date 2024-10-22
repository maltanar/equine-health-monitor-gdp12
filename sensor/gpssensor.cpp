// University of Southampton, 2012
// EMECS Group Design Project

#include <string.h>
#include <stdlib.h>
#include "em_emu.h"
#include "em_gpio.h"
#include "usartmanager.h"
#include "leuartport.h"
#include "gpssensor.h"
#include "alarmmanager.h"
#include "debug_output_control.h"

#define ASCIITONUM(x)	(x - 0x30)


// if parse-on-receive is enabled while operating in DMA mode, every GPRMC
// message received will be automatically parsed into internal structures
void gpsSignalFrameHandler(uint8_t *buf)
{
	module_debug_gps("signal frame!");
	GPSSensor::getInstance()->processNMEAMessage(buf);
}

// PE8 controls the transistor that turns on/off the GPS Vcc
#define	GPIO_GPS_VCC_PORT		gpioPortE
#define	GPIO_GPS_VCC_PIN		8
#define GPIO_GPS_VCC			GPIO_GPS_VCC_PORT, GPIO_GPS_VCC_PIN
// PA5 controls the transistor that turns on/off the GPS Vbat
#define	GPIO_GPS_VBAT_PORT		gpioPortA
#define	GPIO_GPS_VBAT_PIN		5
#define GPIO_GPS_VBAT			GPIO_GPS_VBAT_PORT, GPIO_GPS_VBAT_PIN

void GPSSensor::configurePower()
{
#ifdef GPIO_GPS_VCC
#ifdef GPIO_GPS_VBAT
	// configure both power control GPIOs as push-pull outputs
	GPIO_PinModeSet(GPIO_GPS_VCC, gpioModePushPull, 0);
	GPIO_PinModeSet(GPIO_GPS_VBAT, gpioModePushPull, 0);
#endif
#endif
}

void GPSSensor::setPower(bool vccOn, bool vbatOn)
{
	module_debug_gps("vcc %d vbat %d", vccOn, vbatOn);
#ifdef GPIO_GPS_VCC
#ifdef GPIO_GPS_VBAT
	if(!vccOn)
		GPIO_PinOutSet(GPIO_GPS_VCC);
	else
		GPIO_PinOutClear(GPIO_GPS_VCC);
	
	if(!vbatOn)
		GPIO_PinOutSet(GPIO_GPS_VBAT);
	else
		GPIO_PinOutClear(GPIO_GPS_VBAT);
#endif
#endif
}

GPSSensor::GPSSensor() :
  Sensor(typeGPS, sizeof(GPSMessage), 1000)
{
	m_sensorMessage.sensorMsgArray = (uint8_t *) &m_gpsMessage;
	m_parseOnReceive = false;
	m_port = NULL;
	m_initialized = false;
	m_numSatellitesInView = 0;
	
	// initialize the GPS message to all 0xFF's
	m_gpsMessage.latitude.degree = 0xFF;
	m_gpsMessage.latitude.minute = 0xFF;
	m_gpsMessage.latitude.second = 0xFF;
	m_gpsMessage.latitudeNorth = false;
	m_gpsMessage.longitude.degree = 0xFF;
	m_gpsMessage.longitude.minute = 0xFF;
	m_gpsMessage.longitude.second = 0xFF;
	m_gpsMessage.longitudeWest = false;
	m_gpsMessage.validPosFix = false;
	
	// configure GPS power pins
	configurePower();
}

void GPSSensor::hardReset(uint16_t powerLowMs, uint16_t settleMs)
{
	// turn power off to both Vcc and Vbat
	setPower(false, false);
	// wait desired amount for reset to occur
	AlarmManager::getInstance()->lowPowerDelay(powerLowMs, sleepModeEM2);
	// turn power back on
	setPower(true, true);
	// wait desired amount for the device to settle down
	AlarmManager::getInstance()->lowPowerDelay(settleMs, sleepModeEM2);
}

bool GPSSensor::initialize()
{
	
	m_initialized = false;
	// initialize UART
	m_port = (LEUARTPort *) USARTManager::getInstance()->getPort(GPS_USART_PORT);

	if(!m_port)
	{
		  module_debug_gps("could not attach to LEUART! %d", GPS_USART_PORT);
		  return false;
	}

	m_port->initialize(m_msgBuffer, GPS_MSGBUFFER_SIZE, 
					 LEUARTPort::leuartPortBaudRate9600);

	// TODO should we perform a sanity check on the GPS module here?

#ifdef USE_GPS_DMA
	// generate interrupt when newline is received at the very end of the frame
	m_port->setupSignalFrame('\n');
	// configure DMA 
	m_port->setupDMA(m_dmaBuffer, GPS_MSGBUFFER_SIZE, DMA_CHANNEL_GPS);
#endif

	// power cycle the GPS
	hardReset(10, 150);
	
	m_initialized = true;

	// TODO GPS is responding weirdly to these control msgs, but why?
	// the setPeriod is causing a restart and not keeping the new period..
	// unless sent in a loop
	// send NMEA message to restrict rate and wanted message types, as we
	// are not interested in satellites in view etc. notifications
	//setPeriod(500);
	configureWantedNMEASentences();

	module_debug_gps("initialized");

	return m_initialized;
}

// if enabled, data received via DMA is parsed as soon as one complete
// message is received (inside the signal frame ISR)
void GPSSensor::setParseOnReceive(bool enable)
{
	if(!m_initialized)
	{
		module_debug_gps("not initialized!");
		return;
	}
	
	m_parseOnReceive = enable;
	
	if(enable)
		m_port->setSignalFrameHook(&gpsSignalFrameHandler);
	else
		m_port->setSignalFrameHook(NULL);
}

char GPSSensor::setSleepState(bool sleepState)
{
	// note: GPS sleep function only possible when power pins are controllable
	setPower(!sleepState, true);
	
	/*if(!sleepState)
	{
		// send configure message upon wakeup
		configureWantedNMEASentences();
	}*/
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
	if(!m_initialized)
	{
		module_debug_gps("sampleSensorData: not initialized!");
		return;
	}
	// GPS sends data to us at a fixed rate, so there is no real way of "sampling"
	// instead, parse the stored string data brought by DMA
	// no point in parsing again if parse on receive was set
	if(!m_parseOnReceive)
		processNMEAMessage(m_msgBuffer);
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
    16 NMEA_SEN_MDBG, // PMTKDBG interval � MTK debug information
    17 NMEA_SEN_ZDA, // GPZDA interval � Time & Date
    18 NMEA_SEN_MCHN, // PMTKCHN interval � GPS channel status
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
  // TODO implement detailed message parsing?
  // TODO maybe implement checksum control
  // TODO handle corrupt or incorrect messages, don't loop forever
  int i = 0, c = 0;
  uint8_t fieldPos[20];
  
	// find the start sequence
	while(buffer[i] != '$')
	{
		i++;
		if(i == GPS_MSGBUFFER_SIZE)
		{
		  // reached end of buffer without finding start char
		  module_debug_gps("corrupt NMEA message - no start sequence");
		  return;
		}
	}
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
		
		// reached end of buffer without finding terminator
		if(i == GPS_MSGBUFFER_SIZE)
		{
			module_debug_gps("corrupt NMEA message - no terminator");
			return;
		}
	}
  
  buffer[i] = 0;
  
  module_debug_gps("NMEA msg %s \n", &buffer[fieldPos[0]]);
  module_debug_gps("num of fields %d", c);
  for(i = 0; i < c; i++)
    module_debug_gps("field %d is %s", i, &buffer[fieldPos[i]]);
  
  // check if this was a GPRMC message
  if(strcmp((char const *) &buffer[fieldPos[0]], "GPRMC") == 0)
  {
  	
	// copy information from the parsed fields into GPSMessage structure
	// position fix is invalid if V
	m_gpsMessage.validPosFix = (buffer[fieldPos[2]] == 'V' ? false : true);
	// latitude format is DDMM.SSxx
	m_gpsMessage.latitude.degree = ASCIITONUM(buffer[fieldPos[3] + 0]) * 10 + ASCIITONUM(buffer[fieldPos[3] + 1]);
	m_gpsMessage.latitude.minute = ASCIITONUM(buffer[fieldPos[3] + 2]) * 10 + ASCIITONUM(buffer[fieldPos[3] + 3]);
	m_gpsMessage.latitude.second = ASCIITONUM(buffer[fieldPos[3] + 5]) * 10 + ASCIITONUM(buffer[fieldPos[3] + 6]);
	// latitude hemisphere is N for north and S for south
	m_gpsMessage.latitudeNorth = (buffer[fieldPos[4]] == 'N' ? true : false);
	// longitude format is DDDMM.SSxx
	m_gpsMessage.longitude.degree = ASCIITONUM(buffer[fieldPos[5] + 0]) * 100 
	  								+ ASCIITONUM(buffer[fieldPos[5] + 1]) * 10
									+ ASCIITONUM(buffer[fieldPos[5] + 2]) * 1;
	m_gpsMessage.longitude.minute = ASCIITONUM(buffer[fieldPos[5] + 3]) * 10 + ASCIITONUM(buffer[fieldPos[5] + 4]);
	m_gpsMessage.longitude.second = ASCIITONUM(buffer[fieldPos[5] + 6]) * 10 + ASCIITONUM(buffer[fieldPos[5] + 7]);
	// longitude hemisphere is E for east and W for west
	m_gpsMessage.longitudeWest = (buffer[fieldPos[6]] == 'W' ? true : false);
	
	module_debug_gps("validPosFix: %d", m_gpsMessage.validPosFix);
	module_debug_gps("lat %d %d %d %d", m_gpsMessage.latitude.degree,
					m_gpsMessage.latitude.minute,
					m_gpsMessage.latitude.second,
					m_gpsMessage.latitudeNorth);
	module_debug_gps("long %d %d %d %d", m_gpsMessage.longitude.degree,
					m_gpsMessage.longitude.minute,
					m_gpsMessage.longitude.second,
					m_gpsMessage.longitudeWest);
  } 
	else if(strcmp((char const *) &buffer[fieldPos[0]], "GPGGA") == 0)
  	{
		// copy information from the parsed fields into GPSMessage structure
		// position fix is invalid if V
		m_gpsMessage.validPosFix = (buffer[fieldPos[6]] == '0' ? false : true);
		// latitude format is DDMM.SSxx
		m_gpsMessage.latitude.degree = ASCIITONUM(buffer[fieldPos[2] + 0]) * 10 + ASCIITONUM(buffer[fieldPos[2] + 1]);
		m_gpsMessage.latitude.minute = ASCIITONUM(buffer[fieldPos[2] + 2]) * 10 + ASCIITONUM(buffer[fieldPos[2] + 3]);
		m_gpsMessage.latitude.second = ASCIITONUM(buffer[fieldPos[2] + 5]) * 10 + ASCIITONUM(buffer[fieldPos[2] + 6]);
		// latitude hemisphere is N for north and S for south
		m_gpsMessage.latitudeNorth = (buffer[fieldPos[3]] == 'N' ? true : false);
		// longitude format is DDDMM.SSxx
		m_gpsMessage.longitude.degree = ASCIITONUM(buffer[fieldPos[4] + 0]) * 100 
								+ ASCIITONUM(buffer[fieldPos[4] + 1]) * 10
								+ ASCIITONUM(buffer[fieldPos[4] + 2]) * 1;
		m_gpsMessage.longitude.minute = ASCIITONUM(buffer[fieldPos[4] + 3]) * 10 + ASCIITONUM(buffer[fieldPos[5] + 4]);
		m_gpsMessage.longitude.second = ASCIITONUM(buffer[fieldPos[4] + 6]) * 10 + ASCIITONUM(buffer[fieldPos[5] + 7]);
		// longitude hemisphere is E for east and W for west
		m_gpsMessage.longitudeWest = (buffer[fieldPos[5]] == 'W' ? true : false);
		// number of satellites in view:
		m_numSatellitesInView = ASCIITONUM(buffer[fieldPos[7]]);
		
		module_debug_gps("satellites in view: %d", m_numSatellitesInView);
		module_debug_gps("validPosFix: %d", m_gpsMessage.validPosFix);
		module_debug_gps("lat %d %d %d %d", m_gpsMessage.latitude.degree,
				m_gpsMessage.latitude.minute,
				m_gpsMessage.latitude.second,
				m_gpsMessage.latitudeNorth);
		module_debug_gps("long %d %d %d %d", m_gpsMessage.longitude.degree,
				m_gpsMessage.longitude.minute,
				m_gpsMessage.longitude.second,
				m_gpsMessage.longitudeWest);
	}
  // TODO decide on action when we get a non-GPRMC message
  // "keep old position data" assumed for now
  // ideally we should have a separate message handler that does this
  
}

void GPSSensor::sendNMEAString(char *data)
{
	if(!m_initialized)
	{
		module_debug_gps("sendNMEAString: not initialized!");
		return;
	}
	
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