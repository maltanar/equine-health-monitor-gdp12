#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_chip.h"
#include "trace.h"

#include "mcusetup.h"

void initializeMCU(bool setupSWODebug, bool SWOWhileSleep)
{
	// handle chip errata
	CHIP_Init();
	
	// set the frequencies for the oscillators
	SystemHFXOClockSet(48000000); 	// 48 MHz
	SystemLFXOClockSet(32768);		// 32 KHz
	
	// enable the external crystal oscillators
	CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	
	// select the external crystal oscillators and deselect old ones
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	//CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

	if(setupSWODebug)
		// enable SWO output for printf over SWO
		TRACE_SWOSetup();
	
	if(SWOWhileSleep)
		// enable trace during deep sleep as well
		// consumes additional power but worth it for debugging
		EMU->CTRL |= EMU_CTRL_EMVREG_FULL;	
}