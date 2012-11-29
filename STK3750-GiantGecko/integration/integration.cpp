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
	
	// store the alarm manager instance, paused on creation
	alarmManager = AlarmManager::getInstance();
	
	// create the sensor objects and alarms
	TemperatureSensor * tmp = new TemperatureSensor(1000);
	printf("TS device id %x manid %x \n", tmp->getDeviceID(), tmp->getManufacturerID());
	sensors[SENSOR_TEMP_INDEX] = tmp;
	acquireNewData[SENSOR_TEMP_INDEX] = false;
	sensorAlarmId[SENSOR_TEMP_INDEX] = alarmManager->createAlarm(SENSOR_TEMP_READ_PERIOD, false, &dataReadHandler);
	
	sensors[SENSOR_ACCL_INDEX] = new AccelerationSensor(100);
	acquireNewData[SENSOR_ACCL_INDEX] = false;
	sensorAlarmId[SENSOR_ACCL_INDEX] = alarmManager->createAlarm(SENSOR_ACCL_READ_PERIOD, false, &dataReadHandler);
	
	sensors[SENSOR_GPS_INDEX]  = GPSSensor::getInstance();
	//((GPSSensor*)sensors[SENSOR_GPS_INDEX])->setParseOnReceive(true);
	acquireNewData[SENSOR_GPS_INDEX] = false;
	sensorAlarmId[SENSOR_GPS_INDEX] = alarmManager->createAlarm(SENSOR_GPS_READ_PERIOD, false, &dataReadHandler);
	
	// start counting!
	alarmManager->resume();
	
	uint16_t size;
	SensorMessage *msg;
	GPSMessage *gps;
	TemperatureMessage *tempMsg;
	AccelerometerMessage *acclMsg;
	int acclSampleCount = 0;
	
	while (1)
	{
		EMU_EnterEM2(true);
		
		for(int i = 0; i < SENSOR_COUNT; i++)
			if(acquireNewData[i])
			{
				printf("sample and read for sensor %d \n", i);
				sensors[i]->sampleSensorData();
				if(i != SENSOR_ACCL_INDEX)
					msg = (SensorMessage *) sensors[i]->readSensorData(&size);
				switch(i)
				{
				  case SENSOR_GPS_INDEX:
					// if parseOnReceive is set, no need to sample ourselves?
					gps = (GPSMessage *) msg->sensorMsgArray;
					printf("GPS: %d %d %d, %d %d %d \n", gps->latitude.degree,
						   gps->latitude.minute, gps->latitude.second, 
						   gps->longitude.degree, gps->longitude.minute, 
						   gps->longitude.second);
					break;
				  case SENSOR_TEMP_INDEX:
					tempMsg = (TemperatureMessage *) msg->sensorMsgArray;
					printf("TMP: %f \n", tempMsg->Tobj);
					break;
				  case SENSOR_ACCL_INDEX:
					acclSampleCount++;
					if(acclSampleCount == ACCL_MAX_SAMPLES)
					{
						msg = (SensorMessage *) sensors[i]->readSensorData(&size);
						printf("ACCL: %d samples \n", msg->arrayLength);
						for(int i=0; i < ACCL_MAX_SAMPLES; i++)
						{
							acclMsg = &(((AccelerometerMessage *) msg->sensorMsgArray)[i]);
							printf("sample %d : %d %d %d \n", i, acclMsg->x,
								   acclMsg->y, acclMsg->z);
						}
						acclSampleCount = 0;
					}
					break;
				  default:
				}
				msg->endTimestampS = alarmManager->getUnixTime();
				acquireNewData[i] = false;
				break;
			}
	}

}
