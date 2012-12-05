// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "trace.h"
#include "gbee.h"
#include "gbee-util.h"
#include "xbee_if.h"
#include "usartmanager.h"
#include "alarmmanager.h"
#include "messagestorage.h"

// include files for sensors
#include "gpssensor.h"
#include "accelerationsensor.h"
#include "temperaturesensor.h"

#include <math.h>

#define SENSOR_COUNT		3

#define	SENSOR_TEMP_INDEX	0
#define	SENSOR_ACCL_INDEX	1
#define	SENSOR_GPS_INDEX	2

#define	SENSOR_TEMP_READ_PERIOD	5
#define	SENSOR_ACCL_READ_PERIOD	1
#define	SENSOR_GPS_READ_PERIOD	10


// local variables for the module
Sensor * sensors[SENSOR_COUNT];
AlarmManager * alarmManager;
MessageStorage *msgStore;
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
			//printf("trigger read for sensor %d \n", i);
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
			//printf("trigger wakeup for sensor %d \n", i);
			break;
		}
}

void dataSave(AlarmID id)
{
	dataSaveFlag = true;
}

void recoverRTC()
{
	alarmManager->setUnixTime(msgStore->readRTCStorage());
}

void saveRTC()
{
	msgStore->writeRTCStorage(alarmManager->getUnixTime());
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	CHIP_Init();
	TRACE_SWOSetup();
	
	// TODO clock setting - move to own function
	/* Use 32MHZ HFXO as core clock frequency */
  	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	
	// configure GPS power pins and enable both Vcc and Vbat
	GPSSensor::configurePower();
	GPSSensor::setPower(true, true);
	
	dataSaveFlag = false;
	
	// store the alarm manager instance
	alarmManager = AlarmManager::getInstance();
	
	
	msgStore = MessageStorage::getInstance();
	msgStore->initialize("");
	
	// recover the RTC from storage, if possible
	recoverRTC();
	
	for(int i = 0; i < SENSOR_COUNT; i++)
	{
		acquireNewData[i] = false;
		wakeup[i] = false;
		wakeupAlarmId[i] = ALARM_INVALID_ID;
		sensorAlarmId[i] = ALARM_INVALID_ID;
	}
	
	alarmManager->pause();
	// create the sensor objects and alarms
	TemperatureSensor * tmp = TemperatureSensor::getInstance();
	printf("TS device id %x manid %x \n", tmp->getDeviceID(), tmp->getManufacturerID());
	tmp->setSleepState(true);
	sensors[SENSOR_TEMP_INDEX] = tmp;
	wakeupAlarmId[SENSOR_TEMP_INDEX] = alarmManager->createAlarm(SENSOR_TEMP_READ_PERIOD, false, &deviceWakeupHandler);
	sensorAlarmId[SENSOR_TEMP_INDEX] = alarmManager->createAlarm(SENSOR_TEMP_READ_PERIOD, false, &dataReadHandler);
	// offset the data acquire alarm by 1
	// TODO add support for fixed offsets in alarm creation
	alarmManager->setAlarmTimeout(sensorAlarmId[SENSOR_TEMP_INDEX], SENSOR_TEMP_READ_PERIOD + 1);
	
	
	AccelerationSensor * acc = AccelerationSensor::getInstance();
	acc->setSleepState(true);
	sensors[SENSOR_ACCL_INDEX] = acc;
	wakeupAlarmId[SENSOR_ACCL_INDEX] = alarmManager->createAlarm(SENSOR_ACCL_READ_PERIOD, false, &deviceWakeupHandler);
	sensorAlarmId[SENSOR_ACCL_INDEX] = alarmManager->createAlarm(SENSOR_ACCL_READ_PERIOD, false, &dataReadHandler);
	// offset the data acquire alarm by 1
	// TODO add support for fixed offsets in alarm creation
	alarmManager->setAlarmTimeout(sensorAlarmId[SENSOR_ACCL_INDEX], SENSOR_ACCL_READ_PERIOD + 3);
	
	GPSSensor * gps = GPSSensor::getInstance();
	sensors[SENSOR_GPS_INDEX]  = gps;
	gps->setSleepState(true);
	wakeupAlarmId[SENSOR_GPS_INDEX] = alarmManager->createAlarm(SENSOR_GPS_READ_PERIOD, false, &deviceWakeupHandler);
	sensorAlarmId[SENSOR_GPS_INDEX] = alarmManager->createAlarm(SENSOR_GPS_READ_PERIOD, false, &dataReadHandler);
	// offset the data acquire alarm by 2
	// TODO add support for fixed offsets in alarm creation
	alarmManager->setAlarmTimeout(sensorAlarmId[SENSOR_GPS_INDEX], SENSOR_GPS_READ_PERIOD + 2);
	
	// start counting!
	alarmManager->resume();
	
	uint16_t size;
	SensorMessage *msg;
	GPSMessage *gpsMsg;
	RawTemperatureMessage *tempMsg;
	AccelerometerMessage *acclMsg;
	int acclSampleCount = 0;

	
	while (1)
	{
		EMU_EnterEM2(true);
		
		if(dataSaveFlag)
		{
			dataSaveFlag = false;
			saveRTC();
			//FATFS_speedTest(1);
		}
		
		
		for(int i = 0; i < SENSOR_COUNT; i++)
		{
			if(wakeup[i])
			{
				printf("wakeup for sensor %d time %lld \n", i, alarmManager->getMsTimestamp());
				sensors[i]->setSleepState(false);
				// TODO set allowable sleep state to that which is required
				// by the sensor in order not to miss its data
				wakeup[i] = false;
			}
			
			if(acquireNewData[i])
			{
				printf("sample and read for sensor %d time %lld \n", i, alarmManager->getMsTimestamp());
				
				
				if(i != SENSOR_ACCL_INDEX)
				{
					sensors[i]->sampleSensorData();
					msg = (SensorMessage *) sensors[i]->readSensorData(&size);
				} else {
					for(int c = 0; c < ACCL_MAX_SAMPLES; c++)
					{
						sensors[i]->sampleSensorData();
						//alarmManager->lowPowerDelay(350, sleepModeEM2);
					}
				}
				//send device back to sleep
				sensors[i]->setSleepState(true);
				switch(i)
				{
				  case SENSOR_GPS_INDEX:
					// if parseOnReceive is set, no need to sample ourselves?
					gpsMsg = (GPSMessage *) msg->sensorMsgArray;
					printf("GPS: %d %d %d, %d %d %d \n", gpsMsg->latitude.degree,
						   gpsMsg->latitude.minute, gpsMsg->latitude.second, 
						   gpsMsg->longitude.degree, gpsMsg->longitude.minute, 
						   gpsMsg->longitude.second);
					break;
				  case SENSOR_TEMP_INDEX:
					tempMsg = (RawTemperatureMessage *) msg->sensorMsgArray;
					printf("TMP: %f \n", TemperatureSensor::calculateTemp(tempMsg->Tenv, tempMsg->Vobj));
					break;
				  case SENSOR_ACCL_INDEX:
					msg = (SensorMessage *) sensors[i]->readSensorData(&size);
					printf("ACCL: %d samples \n", msg->arrayLength);
					for(int j=0; j < msg->arrayLength; j++)
					{
						acclMsg = &(((AccelerometerMessage *) msg->sensorMsgArray)[i]);
						printf("sample %d : %d %d %d \n", i, acclMsg->x,
							   acclMsg->y, acclMsg->z);
					}
						acclSampleCount = 0;
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
