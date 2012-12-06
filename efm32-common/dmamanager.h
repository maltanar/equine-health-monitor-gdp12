// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __DMAMANAGER_H
#define __DMAMANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "em_dma.h"

class DMAManager {
public:
  static DMAManager* getInstance()
  {
    static DMAManager instance;
    return &instance;
  }
  
  void configureChannel(uint8_t ch, bool highPrio, uint32_t src);
  DMA_DESCRIPTOR_TypeDef * configureDescriptor(uint8_t ch, DMA_DataInc_TypeDef dstIncMode,
                           DMA_DataInc_TypeDef srcIncMode, 
                           DMA_DataSize_TypeDef unitSize,
                           DMA_ArbiterConfig_TypeDef rate);
  
  void activateBasic(uint8_t ch, void * sourceBuf, void * destBuf, uint32_t size);
  
protected: 
  // ------ start of singleton pattern specific section ------
  DMAManager();  
  DMAManager(DMAManager const&);             // do not implement
  void operator=(DMAManager const&);        // do not implement
  // ------ end of singleton pattern specific section --------
};

#endif  // __DMAMANAGER_H