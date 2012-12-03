#include <stdio.h>
#include <stdint.h>
#include "em_usart.h"
#include "em_chip.h"
#include "em_leuart.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "usartmanager.h"
#include "anthrmsensor.h"
#include "alarmmanager.h"

extern "C" {
void TRACE_SWOSetup();
}

void frameHandler(uint8_t *buf)
{
  // GPSSensor::getInstance()->sampleSensorData();
}

bool rxHook(uint8_t c)
{
  /*if(c == '\n')
    GPSSensor::getInstance()->sampleSensorData();*/
  return true;
    
}

int main()
{
	CHIP_Init();
	TRACE_SWOSetup();
	ANTHRMSensor * hrm = ANTHRMSensor::getInstance();
	AlarmManager * alm = AlarmManager::getInstance();

	//USARTManager::getInstance()->getPort(USARTManagerPortLEUART0)->setSignalFrameHook(&frameHandler);
	//USARTManager::getInstance()->getPort(USARTManagerPortLEUART0)->setRxHook(&rxHook);

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
			
	}
}
