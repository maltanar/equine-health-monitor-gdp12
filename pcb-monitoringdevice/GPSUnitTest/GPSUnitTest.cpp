// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "trace.h"
#include "port_config.h"
#include "alarmmanager.h"

// include files for sensors
#include "gpssensor.h"
#include "anthrmsensor.h"
#include "accelerationsensor.h"
#include "temperaturesensor.h"


#define	SENSOR_GPS_READ_PERIOD	4
GPSSensor * gps;
AlarmManager * alarmManager;

void allSensorsOff()
{
	// turn off GPS power
	GPSSensor::configurePower();
	GPSSensor::setPower(false, false);
	// turn off ANT power
	ANTHRMSensor::configurePower();
	ANTHRMSensor::setPower(false);
	// turn off XBee power
	XBEE_GPIO_CONFIG();
	XBEE_POWER(false);
	// set accelerometer and temp sensor to sleep mode
	TemperatureSensor::getInstance()->setSleepState(true);
	AccelerationSensor::getInstance()->setSleepState(true);
}

bool wakeup = false;
bool read = false;
	
void wakeupHandler(AlarmID id)
{
	wakeup = true;
}

void readHandler(AlarmID id)
{
	read = true;
}



/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	TRACE_SWOSetup();
	
	allSensorsOff();
	
	AlarmManager * mgr = AlarmManager::getInstance();
	
	gps  = GPSSensor::getInstance();
	gps->initialize();
	gps->setParseOnReceive(true);
	gps->setSleepState(true);
		
	mgr->pause();
	mgr->createAlarm(5, false, &wakeupHandler);
	AlarmID readID = mgr->createAlarm(5, false, &readHandler);
	mgr->setAlarmTimeout(readID, 7);
	mgr->resume();
	
	uint16_t size;
	GPSMessage * gpsMsg;
	SensorMessage * msg;
	
	while (1)
	{
		EMU_EnterEM2(true);
		
		if(wakeup)
		{
			printf("wakeup! \n");
			wakeup = false;
			gps->setSleepState(false);
		}
		
		if(read)
		{
			printf("read! \n");
			read = false;
			gps->sampleSensorData();
			gps->setSleepState(true);
			msg = (SensorMessage *) gps->readSensorData(&size);
			gpsMsg = (GPSMessage *) msg->sensorMsgArray;
			if(gpsMsg->validPosFix)
				printf("GPS: position fixed! \n");
			else
				printf("GPS: position not fixed \n");
			
			printf("GPS: %d %d %d, %d %d %d \n", gpsMsg->latitude.degree,
					 gpsMsg->latitude.minute, gpsMsg->latitude.second, 
				 	gpsMsg->longitude.degree, gpsMsg->longitude.minute, 
				 	gpsMsg->longitude.second);
			

		}
	}
}
