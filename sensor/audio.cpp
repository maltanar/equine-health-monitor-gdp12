// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include <stdlib.h>
#include "em_timer.h"
#include "em_dma.h"

#include "audio/audio_timer.h"
#include "audio/audio_dma.h"

#include "usartmanager.h"

#include "audio/audio_config.h"
#include "audio.h"

#include "debug_output_control.h"


Audio::Audio(samplingFreq_typedef samplingFreq, unsigned int bufferSize)
{

  m_samplingFreq = samplingFreq;
  m_bufferSize = bufferSize;

  m_audioStatus = deinit;
  
  m_BufferA = m_BufferB = NULL;
}


samplingFreq_typedef Audio::getsamplingFreq(){
    return m_samplingFreq;
}

unsigned int Audio::getbufferSize(){
    return m_bufferSize;
}

audioStatus_typedef Audio::getStatus(void){
  
    updateBuffStatus();
    
    if(m_audioStatus == recording)
    {
//      if (m_dmaStatus == transfer_done)
//        m_audioStatus = transferdone;
      
      /*if(m_userFetchCount == m_totalDmaCycles)
        m_audioStatus = transferdone;*/
		
		if(m_dmaRxCount == m_totalDmaCycles)
        m_audioStatus = transferdone;
    }
  
    return m_audioStatus;
}


void Audio::setSamplingFreq(samplingFreq_typedef samplingFreq) {
    m_samplingFreq = samplingFreq;
}

void Audio::setBufferSize(unsigned int BufferSize){
  m_bufferSize = BufferSize;
}
  
void Audio::init(void)
{
	m_BufferA = (uint16_t *) malloc(sizeof(uint16_t) * m_bufferSize);
	m_BufferB = (uint16_t *) malloc(sizeof(uint16_t) * m_bufferSize);
  
  if(!m_BufferA || !m_BufferB)
  {
	  module_debug_error("could not allocate audio DMA buffers!");
	  __iar_dlmalloc_stats();
	  return;
  }
  
//  m_port = (SPIPort *) USARTManager::getInstance()->getPort(MIC_USART_PORT);
    m_port = (I2SPort *) USARTManager::getInstance()->getPort(MIC_USART_PORT);

  m_port->initialize();

  //setup timer
//  initTimer();
//timer ot needed for I2S

  //setup dma
//  setupDmaSpi();
  setupDmaI2s();

  m_audioStatus = ready;
}

void Audio::startRecording(unsigned short secs){

  if (m_samplingFreq == Fs_8khz) {
    m_totalDmaCycles = 2*(secs*8000)/(m_bufferSize);
  }
  else
  {
		module_debug_audio("only 8 kHz sampling freq supported!");
		return;
  }
  
  // trigger spi DMA transfer
//  spiDmaTransfer_pp((void*) m_BufferA, (void*) m_BufferB, m_bufferSize, m_totalDmaCycles);
  i2sDmaTransfer((void*) m_BufferA, (void*) m_BufferB, m_bufferSize, m_totalDmaCycles);

  //start TIMER0!
//  startTimer();
//timer ot needed for I2S

  // enable AUTO_TX instead
  m_port->enable(true);
  
  user_read = false;
  m_audioStatus = recording;
  m_bufferStatus = wait_BuffA;
  m_dmaStatus = getDmaStatus();

  m_dmaRxCount = 0;
  m_userFetchCount = 0;

}
  
  
  
void Audio::updateBuffStatus(void){

  m_dmaStatus = getDmaStatus();
  m_dmaRxCount = getDmaRxCount();
  
  //module_debug_audio("updateBuffStatus before: dmaStatus %d rxc %d bufferStatus %d", m_dmaStatus, m_dmaRxCount, m_bufferStatus);
  
  if((m_dmaRxCount - m_userFetchCount) > 1)
	  module_debug_audio("overflow!");
  else if (m_userFetchCount > m_dmaRxCount)
	  module_debug_audio("underflow");
	  
    
  switch(m_bufferStatus) {
    
    case wait_BuffA:
      if (m_dmaStatus == bufferA_full || m_dmaStatus == transfer_done)
        m_bufferStatus = BufferA_new;
	  else if(m_dmaStatus == dma_ready)
		  module_debug_audio("waiting for data...");
      else
		  module_debug_audio("handle me, cwba nbaf");
      break;

    case BufferA_new:
      module_debug_audio("do I need handling? ban");
      break;
	  
    case wait_BuffB:
      if (m_dmaStatus == bufferB_full || m_dmaStatus == transfer_done)
        m_bufferStatus = BufferB_new;
      else
		  module_debug_audio("handle me, cwbb nbbf");
      break;

    case BufferB_new:
      module_debug_audio("do I need handling? bbn");
      break;
    
  }
  
  //module_debug_audio("updateBuffStatus after: dmaStatus %d rxc %d bufferStatus %d", m_dmaStatus, m_dmaRxCount, m_bufferStatus);
      
}



void Audio::gotoSleep(void){

//  stopTimer();

  m_port->enable(false);

  return;
}

void* Audio::getBuffer(void) {

	void * ret = NULL;
	
	//module_debug_audio("getBuff before:  bufferStatus %d userread %d fetchcount %d", m_bufferStatus, user_read, m_userFetchCount);
  if(m_audioStatus == recording || m_audioStatus == transferdone)
  {

//    updateBuffStatus();
	
	// once the last buffer has been read, go back to ready state
	if(m_audioStatus == transferdone)
		m_audioStatus = ready;

    if (m_bufferStatus == BufferA_new)
    {
      m_bufferStatus = wait_BuffB;
      m_userFetchCount++;
      ret = (void *)m_BufferA;
    } 
    else if (m_bufferStatus == BufferB_new)
    {
      m_bufferStatus = wait_BuffA;
      m_userFetchCount++;
      ret = (void *)m_BufferB;
    } 
  }
  else
  {
    printf("ERROR: recording not activated!");
  }
  
  //module_debug_audio("getBuff before:  bufferStatus %d userread %d fetchcount %d", m_bufferStatus, user_read, m_userFetchCount);
  
  return ret;
}

