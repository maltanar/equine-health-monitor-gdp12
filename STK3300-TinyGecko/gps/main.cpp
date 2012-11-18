#include <stdio.h>
#include <stdint.h>
#include "em_usart.h"
#include "em_chip.h"
#include "em_leuart.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "uartport.h"
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
  
  UARTPort::getInstance()->setSignalFrameHook(&frameHandler);
  gps = GPSSensor::getInstance();
  
  while(1)
  {
    /*gps->receiveNMEAString(typ, msg, 80, &len);
    printf("%s \n", msg);*/
    //gps->queryFirmwareVersion();
    //printf("Fixrate: %d \n", gps->getFixRate());
    EMU_EnterEM2(true);
  }
}
