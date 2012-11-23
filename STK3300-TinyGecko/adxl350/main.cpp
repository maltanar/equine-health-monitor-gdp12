// University of Southampton, 2012
// EMECS Group Design Project
// Example application setting up and polling the TMP006 over I2C periodically

#include <stdlib.h>
#include <stdio.h>
#include "efm32.h"
#include "em_chip.h"
#include "em_emu.h"
#include "i2cbus.h"

#include "rtc.h"

void setupSWO(void);

void main(void)
{
  CHIP_Init();
  setupSWO();
  
  I2CBus *i2c = I2CBus::getInstance();
  uint8_t id, datarate;
  uint8_t x0, x1, y0, y1, z0, z1;
  uint8_t ofx, ofy, ofz;
  
  while (1)
  {
    // important: the sampleSensorData call has to be here and not in the
    // TempSensorTimeout call, since the latter executes inside the ISR
    // for RTC and lots of peripherals are disabled during that time
    // general principle: don't sample pins/do fancy stuff in ISRs except
    // moving around memory
    // one idea is to create a "deferred execution" / "pending task queue"
    // model and add a sensor sampling call request from the ISR
    // and when we get back to this main loop, we execute it in here instead
   
    
    i2c -> readRegister8Bit(0x3A, 0x00, &id);
    i2c -> writeRegister8Bit(0x3A, 0x2D, 0x08); // measure bit of power_ctl register is set
    i2c -> writeRegister8Bit(0x3A, 0x2E, 0x00); // interrupt disable
    i2c -> writeRegister8Bit(0x3A, 0x38, 0x40); // FIFO in FIFO mode
    i2c -> writeRegister8Bit(0x3A, 0x31, 0x03); // data format register, self test bit is reset, range bits are set to 11, justify bit 0
    i2c -> writeRegister8Bit(0x3A, 0x2C, 0x16); // low power mode is set, data output rate 3.125 Hz 
    i2c->readRegister8Bit(0x3A, 0x2C, &datarate);
    
    printf("id %x \n", id);
    
    printf("datarate %x \n", datarate);  
   
    i2c -> readRegister8Bit(0x3A, 0x1E, &ofx);
    i2c -> readRegister8Bit(0x3A, 0x1F, &ofy);    
    i2c -> readRegister8Bit(0x3A, 0x20, &ofz);
     
    printf("ofx: %x, ofy: %x, ofz: %x \n", ofx, ofy, ofz);  
   
   while(1)
   {
    
    //i2c -> writeRegister8Bit(0x3A, 0x31, 0x80); // data format register, self test bit is set
    
    i2c -> readRegister8Bit(0x3A, 0x32, &x0);
    i2c -> readRegister8Bit(0x3A, 0x33, &x1);
    
    i2c -> readRegister8Bit(0x3A, 0x34, &y0);
    i2c -> readRegister8Bit(0x3A, 0x35, &y1);
    
    i2c -> readRegister8Bit(0x3A, 0x36, &z0);
    i2c -> readRegister8Bit(0x3A, 0x37, &z1);
    
    //i2c -> readRegister8Bit(0x3A, 0x2B, &act);
    //printf("act %x \n", act);  
//    
//    i2c -> readRegister8Bit(0x3A, 0x31, &act);
//    printf("act %x \n", act);  
    
    //printf("x: %x %x, y: %x %x, z: %x %x \n", x1, x0, y1, y0, z1, z0);
    int16_t x = (x1 << 8) | x0;
    int16_t y = (y1 << 8) | y0;
    int16_t z = (y1 << 8) | z0;
    printf("x %d, y %d, z %d \n", x, y, z); 
    
    //i2c -> writeRegister8Bit(0x3A, 0x2D, 0x00);
     uint8_t fifostatus;
    i2c->readRegister8Bit(0x3A, 0x39, &fifostatus);
    printf("\n fifostatus: %x \n", fifostatus);
    
    RTC_Trigger(320, NULL);
    EMU_EnterEM2(true);
   }
    
  }
  
}

// setupSWO function for enabling printf over the J-Link connection, plus
// to be able to use the energyAware Profiler
void setupSWO(void)
{
  uint32_t *dwt_ctrl = (uint32_t *) 0xE0001000;
  uint32_t *tpiu_prescaler = (uint32_t *) 0xE0040010;
  uint32_t *tpiu_protocol = (uint32_t *) 0xE00400F0;

  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
  /* Enable Serial wire output pin */
  GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;
#if defined(_EFM32_GIANT_FAMILY)
  /* Set location 0 */
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;

  /* Enable output on pin - GPIO Port F, Pin 2 */
  GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
  GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;
#else
  /* Set location 1 */
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC1;
  /* Enable output on pin */
  GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
  GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
#endif
  /* Enable debug clock AUXHFRCO */
  CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;

  while(!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));

  /* Enable trace in core debug */
  CoreDebug->DHCSR |= 1;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* Enable PC and IRQ sampling output */
  *dwt_ctrl = 0x400113FF;
  /* Set TPIU prescaler to 16. */
  *tpiu_prescaler = 0xf;
  /* Set protocol to NRZ */
  *tpiu_protocol = 2;
  /* Unlock ITM and output data */
  ITM->LAR = 0xC5ACCE55;
  ITM->TCR = 0x10009;
}