// University of Southampton, 2012
// EMECS Group Design Project

#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdbool.h>
#include "audio/audio_dma.h"
//#include "spiport.h"
#include "i2sport.h"

// Audio class
typedef enum {
    Fs_8khz  = 8000,
    Fs_16khz = 16000,
} samplingFreq_typedef;


typedef enum {
    deinit,
    ready,
    recording,
    transferdone,
    sleeping
} audioStatus_typedef;


typedef enum {
    wait_BuffA,
    BufferA_new,
    wait_BuffB,
    BufferB_new
} bufferStatus_typedef;


class Audio {
public:
  Audio(samplingFreq_typedef samplingFreq, unsigned int bufferSize);
  
  samplingFreq_typedef getsamplingFreq();
  unsigned int getbufferSize();
  audioStatus_typedef getStatus(void);
  
  void setSamplingFreq(samplingFreq_typedef samplingFreq);
  void setBufferSize(unsigned int BufferSize);
  
  void init(void);
  void startRecording(unsigned short secs);
  void gotoSleep(void);

  void* getBuffer(void);
  
protected:
  samplingFreq_typedef m_samplingFreq;
  unsigned int m_bufferSize;
  I2SPort * m_port;
  
  audioStatus_typedef    m_audioStatus;
  PingPongStatus_TypeDef m_dmaStatus;  //TODO: is it necessary a separate status var?
  bufferStatus_typedef   m_bufferStatus;
  bool user_read;
  
  uint16_t *m_BufferA;
  uint16_t *m_BufferB;
  
  uint16_t m_dmaRxCount;
  uint16_t m_userFetchCount;
  uint16_t m_totalDmaCycles;
  
  void updateBuffStatus(void);

};

#endif // AUDIO_H