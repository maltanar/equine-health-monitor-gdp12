// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "trace.h"
#include "gbee.h"
#include "gbee-util.h"
#include "xbee_if.h"
#include "usartmanager.h"
#include "alarmmanager.h"
#include "fatfs.h"

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

// local variables for the module
Sensor * sensors[SENSOR_COUNT];
AlarmManager * alarmManager;
bool acquireNewData[SENSOR_COUNT], wakeup[SENSOR_COUNT];
AlarmID sensorAlarmId[SENSOR_COUNT], wakeupAlarmId[SENSOR_COUNT];
bool dataSaveFlag;

// Alarm handler function
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

void deviceWakeupHandler(AlarmID id)
{
	// set wakeup flag for triggered sensor
	for(int i = 0; i < SENSOR_COUNT; i++)
		if(id == wakeupAlarmId[i])
		{
			wakeup[i] = true;
			printf("trigger wakeup for sensor %d \n", i);
			break;
		}
}

void dataSave(AlarmID id)
{
	dataSaveFlag = true;
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	TRACE_SWOSetup();
	dataSaveFlag = false;
	
	// TODO clock setting - move to own function?
	// Use 48MHZ HFXO as core clock frequency
  	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	
	// store the alarm manager instance, paused on creation
	alarmManager = AlarmManager::getInstance();
	
	FATFS_initializeFilesystem();
	FATFS_speedTest(256);
	
	for(int i = 0; i < SENSOR_COUNT; i++)
	{
		acquireNewData[i] = false;
		wakeup[i] = false;
	}
	
	
	// create the sensor objects and alarms
	TemperatureSensor * tmp = TemperatureSensor::getInstance();
	printf("TS device id %x manid %x \n", tmp->getDeviceID(), tmp->getManufacturerID());
	sensors[SENSOR_TEMP_INDEX] = tmp;
	wakeupAlarmId[SENSOR_TEMP_INDEX] = alarmManager->createAlarm(SENSOR_TEMP_READ_PERIOD, false, &deviceWakeupHandler);
	sensorAlarmId[SENSOR_TEMP_INDEX] = alarmManager->createAlarm(SENSOR_TEMP_READ_PERIOD, false, &dataReadHandler);
	// offset the data acquire alarm by 1
	// TODO add support for fixed offsets in alarm creation
	alarmManager->setAlarmTimeout(sensorAlarmId[SENSOR_TEMP_INDEX], SENSOR_TEMP_READ_PERIOD + 1);
	
	/*sensors[SENSOR_ACCL_INDEX] = AccelerationSensor::getInstance();
	acquireNewData[SENSOR_ACCL_INDEX] = false;
	sensorAlarmId[SENSOR_ACCL_INDEX] = alarmManager->createAlarm(SENSOR_ACCL_READ_PERIOD, false, &dataReadHandler);*/
	
	/*sensors[SENSOR_GPS_INDEX]  = GPSSensor::getInstance();
	acquireNewData[SENSOR_GPS_INDEX] = false;
	sensorAlarmId[SENSOR_GPS_INDEX] = alarmManager->createAlarm(SENSOR_GPS_READ_PERIOD, false, &dataReadHandler);*/
	
	alarmManager->createAlarm(10, false, &dataSave);
	
	// start counting!
	alarmManager->resume();
	
	uint16_t size;
	SensorMessage *msg;
	GPSMessage *gps;
	RawTemperatureMessage *tempMsg;
	AccelerometerMessage *acclMsg;
	MessagePacket pkt;
	pkt.mainType = msgSensorData;
	
	int acclSampleCount = 0;
	uint8_t serializeBuffer[100];
	
	while (1)
	{
		EMU_EnterEM2(true);
		
		if(dataSaveFlag)
		{
			dataSaveFlag = false;
			FATFS_speedTest(1);
		}
		
		
		for(int i = 0; i < SENSOR_COUNT; i++)
		{
			if(wakeup[i])
			{
				printf("wakeup for sensor %d time %d \n", i, alarmManager->getMsTimestamp());
				sensors[i]->setSleepState(false);
				// TODO set allowable sleep state to that which is required
				// by the sensor in order not to miss its data
				wakeup[i] = false;
			}
			
			if(acquireNewData[i])
			{
				printf("sample and read for sensor %d time %d \n", i, alarmManager->getMsTimestamp());
				sensors[i]->sampleSensorData();
				//send device back to sleep
				sensors[i]->setSleepState(true);
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
					tempMsg = (RawTemperatureMessage *) msg->sensorMsgArray;
					printf("TMP: %f \n", tempMsg->Vobj);
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
					;
				}
				msg->endTimestampS = alarmManager->getUnixTime();
				acquireNewData[i] = false;
			}
		}
	}

}
