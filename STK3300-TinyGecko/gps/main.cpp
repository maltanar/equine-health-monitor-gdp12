#include <stdio.h>
#include <stdint.h>
#include "em_usart.h"
#include "em_chip.h"
#include "em_leuart.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "uartmanager.h"

#include "gpssensor.h"

extern "C" {
void TRACE_SWOSetup();
}

GPSSensor * gps;

void frameHandler(uint8_t *buf)
{
  GPSSensor::getInstance()->sampleSensorData();
}

int main()
{
  CHIP_Init();
  TRACE_SWOSetup();  
  UARTManager::getInstance()->getPort(UARTManagerPortLEUART0)->setSignalFrameHook(&frameHandler);
  gps = GPSSensor::getInstance();
  
  while(1)
  {
    EMU_EnterEM2(true);
  }
}
