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

void alarmHandler(AlarmID id)
{
	printf("Handled alarm %d \n", id);
	
	if(id == 2)
		AlarmManager::getInstance()->stopAlarm(0);
}

int main()
{
  CHIP_Init();
  TRACE_SWOSetup();
  
  AlarmManager * am = AlarmManager::getInstance();
  
  am->createAlarm(1, false, &alarmHandler);
  am->createAlarm(2, false, &alarmHandler);
  am->createAlarm(4, false, &alarmHandler);
  
  
  while(1)
  {
    EMU_EnterEM2(true);
  }
}
