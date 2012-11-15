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

void rxHook(int c)
{
  /*if(c == '$')
  {
    UARTPort::getInstance()->flushRxBuffer();
    gps->sampleSensorData();
  }*/
  printf("%c",c);
}

int main()
{
  CHIP_Init();
  TRACE_SWOSetup();
  
  char typ[10], msg[80];
  uint8_t len;
  
  UARTPort::getInstance()->setRxHook(&rxHook);
  gps = new GPSSensor(2000);

  while(1)
  {
    //gps->receiveNMEAString(typ, msg, 80, &len);
    //printf("%s \n", msg);
    //gps->queryFirmwareVersion();
    //printf("Fixrate: %d \n", gps->getFixRate());
    EMU_EnterEM2(true);
  }
}
