// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "trace.h"
#include "fatfs.h"

int main(void)
{
	CHIP_Init();
	TRACE_SWOSetup();
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	FATFS_initializeFilesystem();
	
	//FATFS_speedTest(256);
	//FATFS_speedTest(512);
	FATFS_speedTest(1024);
	
	while (1)
	{
		EMU_EnterEM2(true);	
	}

}
