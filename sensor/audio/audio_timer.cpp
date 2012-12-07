/**************************************************************************//**
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_timer.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "audio/audio_timer.h"
#include "audio/audio_config.h"

void initTimer (void) 
{
	// TODO move clockpoint to define or config
	CMU_ClockEnable(cmuClock_TIMER0, true);
  /* Select TIMER0 parameters */  
  TIMER_Init_TypeDef timerInit =
  {
    .enable     = false, 
    .debugRun   = true, 
    .prescale   = TIMER_PRESC, 
    .clkSel     = timerClkSelHFPerClk, 
    .fallAction = timerInputActionNone, 
    .riseAction = timerInputActionNone, 
    .mode       = timerModeUp, 
    .dmaClrAct  = true,
    .quadModeX4 = false, 
    .oneShot    = false, 
    .sync       = false, 
  };

  /* Set TIMER Top value */
  TIMER_TopSet(TIMER_USED, TIMER_TOP_8K);

  /* Configure TIMER */
  TIMER_Init(TIMER_USED, &timerInit);

}

/************************************/

void startTimer (void) {
  TIMER_Enable(TIMER_USED, true);
}

void stopTimer (void) {
  TIMER_Enable(TIMER_USED, false);
}



void timer0_IRQ_setup (void) {
//   Enable overflow interrupt 
  TIMER_IntEnable(TIMER0, TIMER_IF_OF);
  
//   Enable TIMER0 interrupt vector in NVIC 
  NVIC_EnableIRQ(TIMER0_IRQn);
}

//TIMER0 IRQ
void TIMER0_IRQHandler(void)
{ 
//   Clear flag for TIMER0 overflow interrupt 
  TIMER_IntClear(TIMER0, TIMER_IF_OF);

}