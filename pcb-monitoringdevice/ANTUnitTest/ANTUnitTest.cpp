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
#include "anthrmsensor.h"
#include "alarmmanager.h"


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main()
{
	CHIP_Init();
	TRACE_SWOSetup();
	
	GPIO_PinModeSet(GPIO_ANT_VCC, gpioModePushPull, 1);
	GPIO_PinModeSet(GPIO_XBEE_VCC, gpioModePushPull, 1);
	
	while(1)
	{
		GPIO_PinOutSet(GPIO_ANT_VCC);
		GPIO_PinOutSet(GPIO_XBEE_VCC);
		AlarmManager::getInstance()->lowPowerDelay(999, sleepModeEM1);
		AlarmManager::getInstance()->lowPowerDelay(999, sleepModeEM1);
		GPIO_PinOutClear(GPIO_ANT_VCC);
		GPIO_PinOutClear(GPIO_XBEE_VCC);
		AlarmManager::getInstance()->lowPowerDelay(999, sleepModeEM1);
		AlarmManager::getInstance()->lowPowerDelay(999, sleepModeEM1);
	
	}
		
		
			
	
	/*
	ANTHRMSensor * hrm = ANTHRMSensor::getInstance();
	AlarmManager * alm = AlarmManager::getInstance();

	bool OK = false; 
	
	SensorMessage * msg;
	HeartRateMessage * hrm_msg;
	uint16_t size;
		

	while(1)
	{
		alm->lowPowerDelay(900, sleepModeEM2);
		
		if(OK) {
			hrm->sampleSensorData();
			msg = (SensorMessage *) hrm->readSensorData(&size);
			hrm_msg = (HeartRateMessage *) msg->sensorMsgArray;
			printf("measured heart rate: %d bpm \n", hrm_msg->bpm);
		}
		else {
		  OK = hrm->initializeNetwork(true);			
		}
			
	}*/
}
