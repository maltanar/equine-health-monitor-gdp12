// University of Southampton, 2012
// EMECS Group Design Project

#include <stdio.h>
#include <stdint.h>
#include "em_usart.h"
#include "em_chip.h"
#include "em_leuart.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "trace.h"
#include "usartmanager.h"
#include "port_config.h"
#include "alarmmanager.h"

// include files for sensors
#include "gpssensor.h"
#include "anthrmsensor.h"
#include "accelerationsensor.h"
#include "temperaturesensor.h"

#define	READ_PERIOD	5
#define READ_OFFSET	2

ANTHRMSensor * hrm;
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
int main()
{
	CHIP_Init();
	TRACE_SWOSetup();
	
	allSensorsOff();
	
	alarmManager = AlarmManager::getInstance();
	hrm = ANTHRMSensor::getInstance();
	hrm->initializeNetwork(true);
	hrm->setSleepState(true);
	
	alarmManager->pause();
	alarmManager->createAlarm(READ_PERIOD, false, &wakeupHandler);
	AlarmID readID = alarmManager->createAlarm(READ_PERIOD, false, &readHandler);
	alarmManager->setAlarmTimeout(readID, READ_PERIOD+READ_OFFSET);
	alarmManager->resume();

	bool OK = false; 
	
	SensorMessage * msg;
	HeartRateMessage * hrm_msg;
	uint16_t size;
		

	while(1)
	{
		EMU_EnterEM2(true);
		
		if(wakeup)
		{
			printf("wakeup! \n");
			wakeup = false;
			hrm->setSleepState(false);
			
			if(!hrm->isInitialized())
				hrm->initializeNetwork();
		}
		
		if(read)
		{
			printf("read! \n");
			read = false;
			hrm->sampleSensorData();
			hrm->setSleepState(true);
			msg = (SensorMessage *) hrm->readSensorData(&size);
			hrm_msg = (HeartRateMessage *) msg->sensorMsgArray;
			printf("measured heart rate: %d bpm \n", hrm_msg->bpm);
		}
			
	}
	
}
