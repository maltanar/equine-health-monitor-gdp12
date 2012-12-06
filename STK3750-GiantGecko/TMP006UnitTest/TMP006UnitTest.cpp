// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "trace.h"
#include "alarmmanager.h"
#include "rtc.h"

// include files for sensors
#include "temperaturesensor.h"

TemperatureSensor * temp_Sensor;
uint16_t size;
SensorMessage *msg;
RawTemperatureMessage *tempMsg;

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
	
	AlarmManager * mgr = AlarmManager::getInstance();
	temp_Sensor = TemperatureSensor::getInstance();
	printf("TS device id %x manid %x \n", temp_Sensor->getDeviceID(), temp_Sensor->getManufacturerID());
	
	
	//temp_Sensor->setSleepState(true);
	
	mgr->pause();
	mgr->createAlarm(2, false, &wakeupHandler);
	AlarmID readID = mgr->createAlarm(2, false, &readHandler);
	mgr->setAlarmTimeout(readID, 3);
	mgr->resume();
	
	while (1)
	{
		EMU_EnterEM2(true);
		
		if(wakeup)
		{
			printf("wakeup! \n");
			wakeup = false;
			temp_Sensor->setSleepState(false);
		}
		
		if(read)
		{
			printf("read! \n");
			read = false;
			temp_Sensor->sampleSensorData();
			msg = (SensorMessage *) temp_Sensor->readSensorData(&size);

			tempMsg = (RawTemperatureMessage *) msg->sensorMsgArray;
			double a = (tempMsg->Tenv);
			double b = (tempMsg->Vobj);

			printf("TMP: %f \n", temp_Sensor->calculateTemp(a, b));
			temp_Sensor->setSleepState(true);
		}
	}

}
