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

	while(1)
	{
		if(OK)
			hrm->transaction();
		else {
		  OK = hrm->initializeNetwork();			
		}
		
		alm->lowPowerDelay(500, sleepModeEM2);
			
	}
}
