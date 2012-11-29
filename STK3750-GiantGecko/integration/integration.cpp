// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "dvk.h"
#include "trace.h"
#include "gbee.h"
#include "gbee-util.h"
#include "xbee_if.h"
#include "uartmanager.h"
#include "rtc.h"
#include "alarmmanager.h"

// include files for sensors
#include "gpssensor.h"
#include "accelerationsensor.h"
#include "temperaturesensor.h"

#define SENSOR_COUNT		3

#define	SENSOR_TEMP_INDEX	0
#define	SENSOR_ACCL_INDEX	1
#define	SENSOR_GPS_INDEX	2

#define	SENSOR_TEMP_READ_PERIOD	2
#define	SENSOR_ACCL_READ_PERIOD	1
#define	SENSOR_GPS_READ_PERIOD	4

Sensor * sensors[SENSOR_COUNT];
AlarmManager * alarmManager;
bool acquireNewData[SENSOR_COUNT];
AlarmID sensorAlarmId[SENSOR_COUNT];

void dataReadHandler(AlarmID id)
{
	// set read flag for triggered sensor
	for(int i = 0; i < SENSOR_COUNT; i++)
		if(id == sensorAlarmId[i])
		{
			acquireNewData[i] = true;
			printf("trigger read for sensor %d \n", i);
			break;
		}
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	/* Configure board. Select either EBI or SPI mode. */
	DVK_init(DVK_Init_SPI);
	TRACE_SWOSetup();
	
	// store the alarm manager instance
	alarmManager = AlarmManager::getInstance();
	
	// create the sensor objects and alarms
	// TODO add inactive alarm creation mode to start alarms simultaneously?
	sensors[SENSOR_TEMP_INDEX] = new TemperatureSensor(1000);
	acquireNewData[SENSOR_TEMP_INDEX] = false;
	sensorAlarmId[SENSOR_TEMP_INDEX] = alarmManager->createAlarm(SENSOR_TEMP_READ_PERIOD, false, &dataReadHandler);
	
	sensors[SENSOR_ACCL_INDEX] = new AccelerationSensor(100);
	acquireNewData[SENSOR_ACCL_INDEX] = false;
	sensorAlarmId[SENSOR_ACCL_INDEX] = alarmManager->createAlarm(SENSOR_ACCL_READ_PERIOD, false, &dataReadHandler);
	
	sensors[SENSOR_GPS_INDEX]  = GPSSensor::getInstance();
	((GPSSensor*)sensors[SENSOR_GPS_INDEX])->setParseOnReceive(true);
	acquireNewData[SENSOR_GPS_INDEX] = false;
	sensorAlarmId[SENSOR_GPS_INDEX] = alarmManager->createAlarm(SENSOR_GPS_READ_PERIOD, false, &dataReadHandler);
	uint16_t size;
	
	void * data;
	SensorMessage *msg;
	GPSMessage *gps;
	while (1)
	{
		EMU_EnterEM2(true);
		
		for(int i = 0; i < SENSOR_COUNT; i++)
			if(acquireNewData[i])
			{
				printf("do read for sensor %d \n", i);
				if(i == SENSOR_GPS_INDEX)
				{
					//sensors[i]->sampleSensorData();
					msg = (SensorMessage *) sensors[i]->readSensorData(&size);
					gps = (GPSMessage *) msg->sensorMsgArray;
					printf("GPS: %d %d %d, %d %d %d \n", gps->latitude.degree,
						   gps->latitude.minute, gps->latitude.second, 
						   gps->longitude.degree, gps->longitude.minute, 
						   gps->longitude.second);
				}
				acquireNewData[i] = false;
				break;
			}
	}

}
