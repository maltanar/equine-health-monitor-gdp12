/*****************************************************************************
DMA driver for audio acquisition

** CH0
used to generate SPI transaction (SCK + CS)
source: fixed dummy memory variable
Detination: fixed, TXDOUBLE

** CH1
To fetch audio samples and store them in a buffer in RAM
source: fixed, RXDOUBLE
destination: BUffer, increasing.
 *****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif   
   
#ifndef __AUDIO_DMA_H
#define __AUDIO_DMA_H

#include <stdbool.h>
#include <stdio.h>

// TODO move DMA channel allocs to a central header file
// TODO rename defines to something more audio specific
#define DMA_CHANNEL_TX       3
#define DMA_CHANNEL_RX       4
//#define DMA_CHANNEL_FLASH    2

#define DMA_CHANNELS     2
//#define DMA_CHANNELS     3
   

/**************************************************************************//**
 * SpiDma Call-back function
 *****************************************************************************/
void spiBlockCompleted(unsigned int channel, bool primary, void *user);


/**************************************************************************//**
 * Configure DMA in Ping-pong mode for both TX and RX to/from USART
 *****************************************************************************/
void setupDmaSpi(void);


/**************************************************************************//**
 * SPI DMA Transfer
 * Enable pingpong transfers.
 *****************************************************************************/
void spiDmaTransfer(void *rxBuffer,  int bytes);


/**************************************************************************//**
 * Returns if an SPI transfer is active
 *****************************************************************************/
bool spiDmaIsActive(void);


/**************************************************************************//**
 * Sleep in EM1 until DMA transfer is done
 *****************************************************************************/
void sleepUntilDmaDone(void);


/****************************************************************************
* DMA ping pong mode 
 *****************************************************************************/

void spiDmaTransfer_pp(void *rxBufferPri,  void *rxBufferAlt, int bufferSize, int cycles);


/// ping pong transfer status state_machine
typedef enum
{
  dma_ready,
  bufferA_full, 
  bufferB_full,
  transfer_done,
  dma_hold
} PingPongStatus_TypeDef;

PingPongStatus_TypeDef getDmaStatus(void);

uint16_t getDmaRxCount(void);



#endif


#ifdef __cplusplus
 }
#endif   