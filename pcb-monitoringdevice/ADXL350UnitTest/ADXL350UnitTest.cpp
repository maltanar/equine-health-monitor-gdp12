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
#include "uartmanager.h"
#include "rtc.h"
//#include "alarmmanager.h"

// include files for sensors
#include "accelerationsensor.h"

#define	SENSOR_ACCL_READ_PERIOD	1

Sensor * accl_Sensor;
//AlarmManager * alarmManager;
//bool accl_AcquireNewData;
//AlarmID accl_SensorAlarmId;

// void dataReadHandler(AlarmID id)
// {
		// if(id == accl_SensorAlarmId)
		// {
			// accl_AcquireNewData = true;
			// printf("trigger read for sensor accelerometer \n");
			// break;
		// }
// }

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	TRACE_SWOSetup();
	
	// store the alarm manager instance, paused on creation
	//alarmManager = AlarmManager::getInstance();
		
	accl_Sensor = new AccelerationSensor(100);
	//accl_AcquireNewData = false;
	//accl_SensorAlarmId = alarmManager->createAlarm(SENSOR_ACCL_READ_PERIOD, false, &dataReadHandler);
		
	// start itcounting!
	//alarmManager->resume();
	
	uint16_t size;
	SensorMessage *msg;
	AccelerometerMessage *acclMsg;
	int acclSampleCount = 0;
	
	while (1)
	{
		RTC_Trigger(100, NULL);
		EMU_EnterEM2(true);
		
			//if(accl_AcquireNewData)
			//{
				printf("sample and read for Accelerometer sensor \n");
				accl_Sensor->sampleSensorData();
				
					acclSampleCount++;
					if(acclSampleCount == ACCL_MAX_SAMPLES)
					{
						msg = (SensorMessage *) accl_Sensor->readSensorData(&size);
						printf("ACCL: %d samples \n", msg->arrayLength);
						for(int i=0; i < ACCL_MAX_SAMPLES; i++)
						{
							acclMsg = &(((AccelerometerMessage *) msg->sensorMsgArray)[i]);
							printf("sample %d : %d %d %d \n", i, acclMsg->x,
								   acclMsg->y, acclMsg->z);
						}
						acclSampleCount = 0;
					}
				//msg->endTimestampS = alarmManager->getUnixTime();
				//accl_AcquireNewData = false;
				//break;
			//}
	}

}
