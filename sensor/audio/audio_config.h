/*****************************************************************************
Config.h
Audio driver parameters
******************************************************************************/
#ifndef __AUDIO_CONFIG_H
#define __AUDIO_CONFIG_H

/**Audio peripheral selection*************************************************/
#define AUDIO_SPI
//#define AUDIO_I2S


/* HFXO crystal frequency */
#define HFXO_FREQUENCY  48000000

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
#define TIMER_PRESC     timerPrescale4
#define TIMER_TOP_8K    1499


/****************************************************************************
* USART *
****************************************************************************/
/* Configure USART1 channel */
#define USART_USED        USART1

/*Sampling Frequency parameters*/
#define FS_8K           8000

#ifdef AUDIO_SPI
#define SPI_PERCLK_FREQUENCY    HFXO_FREQUENCY
#define SPI_BAUDRATE            10000000
#endif

#ifdef I2S_AUDIO
#define I2S_BAUDRATE_8K  64*FS_8K
#define I2S_STARTUP_CLKS 262144  // (2^18)
#endif


#define MIC_USARTCFG	(USARTManagerPortConfigs[MIC_USART_PORT])
#define MIC_USART		(MIC_USARTCFG.usartBase)










/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/********** DEPRICATED/ UNUSED ***********************************************/

/****************************************************************************
* DMA *
****************************************************************************/
/* Address for DMA primary and alternate channel control data structure */
//#define PRI_ADDRESS     0x20001000
//#define ALT_ADDRESS     0x20001100


/* Parameters for Flash record */
#define FLASH_WR_SIZE   40
#define FLASH_WORD_SIZE FLASH_WR_SIZE/4
#define PAGE_MASK       0x0fff
#define PAGE_END        0x0ff0
#define PAGE_OFFSET     16
#define FLASHSTART      0x80000
#define RECORD_FRAMES   600
#define FRAME_MAXIMUM   26112

/* Define play list size */
//#define LIST_SIZE       2


/****************************************************************************
* PRS *
****************************************************************************/
//#define USART_PRS_CH      0
//#define USART_PRS_SEL     usartPrsTriggerCh0

//#define TIMER_PRS       PRS_CH_CTRL_SOURCESEL_TIMER0
//#define TIMER_SRC       PRS_CH_CTRL_SIGSEL_TIMER0OF

/****************************************************************************
* Giant Gecko SPEEX codec configuration parameters *
****************************************************************************/
//#ifdef __IAR_SYSTEMS_ICC__     /* IAR Compiler */
//#define inline inline
//#endif

//#ifdef  __CC_ARM               /* ARM Compiler */
//#define inline __inline
//#endif

//#define EXPORT 

//#define SPEEX_HFXO_FREQ 48000000

//#define FIXED_POINT
//#define DISABLE_WIDEBAND
//#define DISABLE_FLOAT_API
//#define DISABLE_VBR
//#define DISABLE_NOTIFICATIONS
//#define DISABLE_WARNINGS
//#define RELEASE
//#define OVERRIDE_SPEEX_PUTC
//#define OVERRIDE_SPEEX_FATAL
//#define MAX_CHARS_PER_FRAME (20/BYTES_PER_CHAR)

//#define NB_ENC_STAK     3000
//#define NB_DEC_STACK    1000

//#define SAMPLE_SIZE     160
//#define FRAME_SIZE_8K   20
//#define ENCODE_SIZE     20

//#define NARROWBAND8K    0
//#define NARROWBAND      1
//#define WIDEBAND        2
//#define ULTRAWIDEBAND   3
//#define NB8KCODEC       4

//#define SPEEX_BAND      NB8KCODEC

//extern void _speex_fatal(const char *str, const char *file, int line);
//extern void _speex_putc(int ch, void *file);

#endif