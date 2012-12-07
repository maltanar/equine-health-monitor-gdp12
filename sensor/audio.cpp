// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
//#include "em_gpio.h"
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

  m_BufferA = new uint16_t[m_bufferSize];
  m_BufferB = new uint16_t[m_bufferSize];
  
//  init();  //TODO, remove hardcoding!
  
}


samplingFreq_typedef Audio::getsamplingFreq(){
    return m_samplingFreq;
}

unsigned int Audio::getbufferSize(){
    return m_bufferSize;
}

audioStatus_typedef Audio::getStatus(void){
//    updateStatus();
  
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
  
void Audio::init(void){

//check clock frequency?

  //setup spi, USART1 as Master
  //SPI_setup(USART1_NUM, GPIO_POS1, true);
  m_port = (SPIPort *) USARTManager::getInstance()->getPort(MIC_USART_PORT);
  m_port->initialize();

  //setup timer
  initTimer();

  //setup dma
  setupDmaSpi();

  m_audioStatus = ready;
}

void Audio::startRecording(unsigned short secs){

  if (m_samplingFreq == Fs_8khz) {
    m_totalDmaCycles = (secs*8000)/m_bufferSize;
  }
  else
  {
		module_debug_audio("only 8 kHz sampling freq supported!");
		return;
  }
  
  // trigger spi DMA transfer
  spiDmaTransfer_pp((void*) m_BufferA, (void*) m_BufferB, m_bufferSize, m_totalDmaCycles);

  //start TIMER0!
  startTimer();

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
    
  switch(m_bufferStatus) {
    
    case wait_BuffA:
      if (m_dmaRxCount == m_userFetchCount)
        m_bufferStatus = wait_BuffA;
      else if (m_dmaRxCount == m_userFetchCount+1) {
        m_bufferStatus = BufferA_new;
        user_read = false;
      }
      else {
        printf("Error: buffer overflow/underflow");
      }
      break;

    case BufferA_new:
      if (m_dmaRxCount == m_userFetchCount+1)
        m_bufferStatus = BufferA_new;
      else if (m_dmaRxCount == m_userFetchCount) {
        m_bufferStatus = wait_BuffB;
        user_read = false;
      }
      else {
        printf("Error: buffer overflow/underflow");
      }
      break;

    case wait_BuffB:
      if (m_dmaRxCount == m_userFetchCount)
        m_bufferStatus = wait_BuffB;
      else if (m_dmaRxCount == m_userFetchCount+1) {
        m_bufferStatus = BufferB_new;
        user_read = false;
      }
      else {
        printf("Error: buffer overflow/underflow");
      }
      break;

    case BufferB_new:
      if (m_dmaRxCount == m_userFetchCount+1)
        m_bufferStatus = BufferB_new;
      else if (m_dmaRxCount == m_userFetchCount) {
        m_bufferStatus = wait_BuffA;
        user_read = false;
      }
      else {
        printf("Error: buffer overflow/underflow");
      }
      break;
    
  }
      
}



void Audio::gotoSleep(void){

  stopTimer();


  return;
}

void* Audio::getBuffer(void) {

  
  if(m_audioStatus == recording || m_audioStatus == transferdone)
  {

//    updateBuffStatus();
	
	// once the last buffer has been read, go back to ready state
	if(m_audioStatus == transferdone)
		m_audioStatus = ready;

    if (m_bufferStatus == BufferA_new)
    {
      user_read = true;
      m_userFetchCount++;
      return (void *)m_BufferA;
    } 
    else if (m_bufferStatus == BufferB_new)
    {
      user_read = true;
      m_userFetchCount++;
      return (void *)m_BufferB;
    } 
    else 
    {
      return NULL;
    }
	
	
  }
  else
  {
    printf("ERROR: recording not activated!");
    return NULL;
  }
}

