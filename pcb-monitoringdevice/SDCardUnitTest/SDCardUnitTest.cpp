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

int main(void)
{
	CHIP_Init();
	TRACE_SWOSetup();
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	printf("initializing filesystem... \n");
	FATFS_initializeFilesystem();
	
	uint8_t testCount = 10;
		
	while(testCount--)
	{
		printf("Speed test - 256 KB: \n");
		FATFS_speedTest(256);
		AlarmManager::getInstance()->lowPowerDelay(900, sleepModeEM2);
		/*printf("Speed test - 512 KB: \n");
		FATFS_speedTest(512);
		printf("Speed test - 1 MB: \n");
		FATFS_speedTest(1024, true);*/
	}
	printf("deinitializing filesystem... \n");
	FATFS_deinitializeFilesystem();
	printf("All speed tests finished! \n");
	
	while (1)
	{
		EMU_EnterEM2(true);	
	}

}
