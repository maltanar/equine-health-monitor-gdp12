// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "trace.h"
int main(void)
{
	TRACE_SWOSetup();
	
	
	
	while (1)
	{
		EMU_EnterEM2(true);
		
		
	}

}
