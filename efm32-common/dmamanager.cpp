#include "dmamanager.h"
#include "efm32.h"
#include "em_chip.h"

// DMA control block, must be aligned to 256
// definition block taken from Energy Micro's LEUART application note
#if defined (__ICCARM__)
#pragma data_alignment=256
DMA_DESCRIPTOR_TypeDef dmaControlBlock[DMA_CHAN_COUNT * 2];
#elif defined (__CC_ARM)
DMA_DESCRIPTOR_TypeDef dmaControlBlock[DMA_CHAN_COUNT * 2] __attribute__ ((aligned(256)));
#elif defined (__GNUC__)
DMA_DESCRIPTOR_TypeDef dmaControlBlock[DMA_CHAN_COUNT * 2] __attribute__ ((aligned(256)));
#else
#error Undefined toolkit, need to define alignment for DMA control block
#endif

DMAManager::DMAManager()
{
  // setup the data structure we use to initialize DMA
  DMA_Init_TypeDef dmaInit =
  {
    .hprot = 0,
    .controlBlock = dmaControlBlock
  };
  DMA_Init(&dmaInit);
  
}

// TODO keep track of used DMA channels instead of asking the user, make
//      management easier

void DMAManager::configureChannel(uint8_t ch, bool highPrio, uint32_t source)
{
  // setup the data structure we use to initialize the DMA channel
  DMA_CfgChannel_TypeDef channelConfig =
  {
    .highPri   = highPrio,
    .enableInt = false,
    .select    = source,
    .cb        = NULL
  };
  DMA_CfgChannel(ch, &channelConfig);
}

DMA_DESCRIPTOR_TypeDef * DMAManager::configureDescriptor(uint8_t ch, DMA_DataInc_TypeDef dstIncMode,
                           DMA_DataInc_TypeDef srcIncMode, 
                           DMA_DataSize_TypeDef unitSize,
                           DMA_ArbiterConfig_TypeDef rate)
{
  // setup the data structure we use to initialize the DMA descriptor
  DMA_CfgDescr_TypeDef descriptorConfig =
  {
    .dstInc  = dstIncMode,
    .srcInc  = srcIncMode,
    .size    = unitSize,
    .arbRate = rate,
    .hprot   = 0
  };
  DMA_CfgDescr(ch, true, &descriptorConfig);
  
  return ((DMA_DESCRIPTOR_TypeDef *)(DMA->CTRLBASE)) + ch;
}

void DMAManager::activateBasic(uint8_t ch, void * sourceBuf, void * destBuf, 
                               uint32_t size)
{
  DMA_ActivateBasic(ch, true, false, destBuf, sourceBuf,size);
}