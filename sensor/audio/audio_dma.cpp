/*****************************************************************************
AUDIO_DMA_PINGPONG
 *****************************************************************************/
#define UINT32_MAX	0xFFFFFFFF
#include <stdbool.h>
#include <stdio.h>
#include "em_chip.h"
#include "em_dma.h"
#include "em_emu.h"
#include "em_int.h"
#include "em_msc.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_cmu.h"

#include "usartmanager.h"
#include "dmamanager.h"
#include "audio/audio_dma.h"
#include "audio/audio_config.h"

#include "debug_output_control.h"

/* DMA Callback structure */
DMA_CB_TypeDef cb_tx;
DMA_CB_TypeDef cb_rx;

/* Dummy word for usart TX */
const uint16_t zeroes = 0x0000;

/* Transfer Flags */
volatile PingPongStatus_TypeDef PingPongStatus;

/* Ping-pong buffer full flags */
volatile bool spiTransferActive;

volatile int bufA_count;
volatile int bufB_count;
volatile int tx_count;
volatile int rx_count;

volatile uint16_t spiTotalTransfers;
volatile uint16_t spiBufferSize;

DMAManager * dma_mgr; 

void spiBlockCompleted(unsigned int channel, bool primary, void *user)
{
  (void) user;
  
  if (channel == DMA_CHANNEL_TX)
  {
    if (tx_count < (spiTotalTransfers - 2))
    {
      DMA_RefreshPingPong(channel,
                        primary,
                        false,
                        NULL,
                        NULL,
                        spiBufferSize -1,
                        false);
    }

    tx_count++;
//    PingPongStatus = dma_hold;
  }
  else if (channel == DMA_CHANNEL_RX)
  {
	  module_debug_audio("!rx! primary %d \n", primary);
    if (rx_count < (spiTotalTransfers - 2))
    {
      DMA_RefreshPingPong(channel,
                          primary,
                          false,
                          NULL,
                          NULL,
                          spiBufferSize -1,
                          false);
    }
    // Keeping track of the number of transfers 
    rx_count++;

    if (primary)
    {
      bufA_count++;
      PingPongStatus = bufferA_full;
    }
    else
    {
      bufB_count++;
      PingPongStatus = bufferB_full;
    }
  }
  else
  {
    while(1); // ERROR, No action defined for this channel 
  }
 
}


/**************************************************************************//**
 * @brief Configure DMA in basic mode for both TX and RX to/from USART
 *****************************************************************************/
void setupDmaSpi(void)
{
  /* Initialization structs */
  DMA_CfgChannel_TypeDef  rxChnlCfg;
  DMA_CfgDescr_TypeDef    rxDescrCfg;
  DMA_CfgChannel_TypeDef  txChnlCfg;
  DMA_CfgDescr_TypeDef    txDescrCfg; 
  // getting an instance of DMAManager is enough to initalize DMA for our case
  dma_mgr = DMAManager::getInstance();
  
  /* Setup call-back function */  
  cb_rx.cbFunc  = spiBlockCompleted;
  cb_rx.userPtr = NULL;
  cb_tx.cbFunc  = spiBlockCompleted;
  cb_tx.userPtr = NULL;
  
  
  // "TX" DMA - push dummy words into SPI to generate clock signals for the peripheral

  /* Setting up channel */
  txChnlCfg.highPri   = true;
////changed, disable interrupts from TX
  txChnlCfg.enableInt = true;
  txChnlCfg.select    = DMAREQ_TIMER0_UFOF;	// TODO move to defines
  txChnlCfg.cb        = &cb_tx;
  DMA_CfgChannel(DMA_CHANNEL_TX, &txChnlCfg);

  /* Setting up channel descriptor */
  // do not increment neither src nor dest, keep copying same dummy byte
  txDescrCfg.dstInc  = dmaDataIncNone;
  txDescrCfg.srcInc  = dmaDataIncNone;
  txDescrCfg.size    = dmaDataSize2;   // 16-bit words
  txDescrCfg.arbRate = dmaArbitrate1;
  txDescrCfg.hprot   = 0;
  DMA_CfgDescr(DMA_CHANNEL_TX, true, &txDescrCfg);
  DMA_CfgDescr(DMA_CHANNEL_TX, false, &txDescrCfg);
  
  /*** Setting up RX DMA ***/

  /* Setting up channel */
  rxChnlCfg.highPri   = true;
  rxChnlCfg.enableInt = true;
  rxChnlCfg.select    = DMAREQ_USART1_RXDATAV;
  rxChnlCfg.cb        = &cb_rx;
  DMA_CfgChannel(DMA_CHANNEL_RX, &rxChnlCfg);

  /* Setting up channel descriptor */
  rxDescrCfg.dstInc  = dmaDataInc2;	// 2 bytes
  rxDescrCfg.srcInc  = dmaDataIncNone;	// read src does not change
  rxDescrCfg.size    = dmaDataSize2;	// 16 bits
  rxDescrCfg.arbRate = dmaArbitrate1;
  rxDescrCfg.hprot   = 0;
  /* Ping-pong mode. primary and alternate channel descriptors have the same configuration */
  DMA_CfgDescr(DMA_CHANNEL_RX, true, &rxDescrCfg);
  DMA_CfgDescr(DMA_CHANNEL_RX, false, &rxDescrCfg);

  PingPongStatus = transfer_done;


}

/**************************************************************************//**
 * @brief  SPI DMA Transfer
 * NULL can be input as txBuffer if tx data to transmit dummy data
 * If only sending data, set rxBuffer as NULL to skip DMA activation on RX
 *****************************************************************************/
//void spiDmaTransfer_pp(void *rxBufferP,  int bytes)
void spiDmaTransfer_pp(void *rxBufferPri,  void *rxBufferAlt, int bufferSize, int cycles)
{ 
	spiTotalTransfers = cycles;
	spiBufferSize = bufferSize;
  

  /* Setting flag to indicate that TX is in progress
   * will be cleared by call-back function */
  spiTransferActive = true;
  
  /* Clear TX regsiters */
  MIC_USART->CMD = USART_CMD_CLEARTX;
  
  /* Activate TX channel */
  DMA_ActivatePingPong(DMA_CHANNEL_TX,
                       false,
                       (void *)&(MIC_USART->TXDOUBLE),
                       (void *)&zeroes,
                       spiBufferSize - 1,
                       (void *)&(MIC_USART->TXDOUBLE),
                       (void *)&zeroes,
                       spiBufferSize - 1);
  
  /* Clear RX regsiters */
  MIC_USART->CMD = USART_CMD_CLEARRX;
  
  /* Activate RX channel */
  DMA_ActivatePingPong(DMA_CHANNEL_RX,
                       false,
                       rxBufferPri,
                       (void *)&(MIC_USART->RXDOUBLE),
                       spiBufferSize - 1,
                       rxBufferAlt,
                       (void *)&(MIC_USART->RXDOUBLE),
                       spiBufferSize - 1);

  PingPongStatus = dma_ready;

}



/**************************************************************************//**
 * @brief  Returns if an SPI transfer is active
 *****************************************************************************/
bool spiDmaIsActive(void)
{
  bool temp;
  INT_Disable();
  temp = spiTransferActive;
  INT_Enable();
  return temp;
  //return spiTransferActive;
}


// TODO IMPORTANT do not disable all interrupts! determine the exact functionality
// of this and port accordingly

// return DMA status
PingPongStatus_TypeDef getDmaStatus(void)
{
  PingPongStatus_TypeDef temp;
  
  /*INT_Disable();
  temp = PingPongStatus;

  if(rx_count == spiTotalTransfers)
    temp = transfer_done;
  INT_Enable();
  return temp;*/
  return PingPongStatus;

}

uint16_t getDmaRxCount(void) {
  return rx_count;
}


/**************************************************************************//**
 * @brief  Sleep in EM1 until DMA transfer is done
 *****************************************************************************/
void sleepUntilDmaDone(void)
{
  /* Wait in EM1 in until DMA is finished and callback is called */
  /* Disable interrupts until flag is checked in case DMA finishes after flag 
  * check but before sleep command. Device will still wake up on any set IRQ 
  * and any pending interrupts will be handled after interrupts are enabled 
  * again. The INT_Disable and INT_Enable functions include a lock level counter
  * so even though INT_Enable is run inside of spiDmaisActive(), interrupts will
  * not be enabled until it is run as many times as INT_Disable has been run. */
  INT_Disable();
  while(spiDmaIsActive())
  {
   EMU_EnterEM1(); 
   INT_Enable();
   INT_Disable();
  }
  INT_Enable();
}

