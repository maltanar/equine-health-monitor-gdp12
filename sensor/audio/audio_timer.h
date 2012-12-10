/**************************************************************************//**
 *****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif  

#ifndef __AUDIO_TIMER_H
#define __AUDIO_TIMER_H

/****************************************************************************
* TIMER *

AUDIO_SPI: Timer0 for sampling trigger. (PRS)
Fsampling = HFPERCLK_FREQUENCY/ ( 2^(PRESC + 1) x (TOP+ 1) ) 

parameters for 8KHz sampling
HFPERCLK_FREQUENCY = 48000000
PRESC = 1 : Div by4
TOP = 1499
*******************************************************************************/
/* Configure Timer */
#define TIMER_USED      TIMER0
#define TIMER_CLK       cmuClock_TIMER0
//#define TIMER_CLK       CMU_HFPERCLKEN0_TIMER0
#define TIMER_PRESC timerPrescale4
#define TIMER_TOP_8K 1499



//TIMER0 config
void initTimer (void);
void startTimer(void);
void stopTimer(void);

//TIMER0 IRQ
void TIMER0_IRQHandler(void);
void timer0_IRQ_setup (void);

#endif

#ifdef __cplusplus
 }
#endif  
