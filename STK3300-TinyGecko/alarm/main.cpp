#include <stdio.h>
#include <stdint.h>
#include "em_usart.h"
#include "em_chip.h"
#include "em_leuart.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "alarmmanager.h"


extern "C" {
void TRACE_SWOSetup();
}

int main()
{
  CHIP_Init();
  TRACE_SWOSetup();
  
  AlarmManager::getInstance();
  
  
  while(1)
  {
    EMU_EnterEM2(true);
  }
}
