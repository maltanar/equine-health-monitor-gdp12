#include <stdio.h>
#include <stdint.h>
#include "em_usart.h"
#include "em_chip.h"
#include "em_leuart.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "uartmanager.h"
#include "anthrmsensor.h"

extern "C" {
void TRACE_SWOSetup();
}

void frameHandler(uint8_t *buf)
{
  // GPSSensor::getInstance()->sampleSensorData();
}

void rxHook(int c)
{
  /*if(c == '\n')
    GPSSensor::getInstance()->sampleSensorData();*/
}

int main()
{
  CHIP_Init();
  TRACE_SWOSetup();
  ANTHRMSensor * hrm = ANTHRMSensor::getInstance();
  
  //UARTManager::getInstance()->getPort(UARTManagerPortLEUART0)->setSignalFrameHook(&frameHandler);
  //UARTManager::getInstance()->getPort(UARTManagerPortLEUART0)->setRxHook(&rxHook);
  
  bool OK = hrm->initializeNetwork();
  
	while(1)
	{
		if(OK)
			hrm->transaction();
		
		EMU_EnterEM2(true);
	}
}
