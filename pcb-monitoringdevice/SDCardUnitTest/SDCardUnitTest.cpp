// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "trace.h"
#include "fatfs.h"
#include "alarmmanager.h"
#include "mcusetup.h"

AlarmManager * alm;

int main(void)
{
	initializeMCU(true, false);
	
	alm = AlarmManager::getInstance();
	
	printf("initializing filesystem... \n");
	FATFS_initializeFilesystem();
	
	uint8_t testCount = 10;
		
	while(testCount--)
	{
		printf("Speed test - 8 KB: \n");
		FATFS_speedTest(8);
		alm->lowPowerDelay(900, sleepModeEM2);
	}
	
	testCount = 10;
	
	while(testCount--)
	{
		printf("Speed test - 256 KB: \n");
		FATFS_speedTest(256);
		alm->lowPowerDelay(900, sleepModeEM2);
	}
	
	testCount = 2;
	
	while(testCount--)
	{
		printf("Speed test - 512 KB: \n");
		FATFS_speedTest(512);
		alm->lowPowerDelay(900, sleepModeEM2);
	}
	
	
	printf("deinitializing filesystem... \n");
	FATFS_deinitializeFilesystem();
	printf("All speed tests finished! \n");
	
	while (1)
	{
		EMU_EnterEM2(true);	
	}

}
