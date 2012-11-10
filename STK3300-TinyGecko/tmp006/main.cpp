// University of Southampton, 2012
// EMECS Group Design Project
// Example application setting up and polling the TMP006 over I2C periodically

#include <stdlib.h>
#include <stdio.h>
#include "efm32.h"
#include "em_chip.h"
#include "em_emu.h"
#include "temperaturesensor.h"
#include "alarm.h"
#include "i2cbus.h"

#define TEMP_SENSOR_PERIOD_MS   1000

TemperatureSensor * ts;

void setupSWO(void);

void TempSensorTimeout(int id)
{
  printf("Temperature reading: %f \n", ts->getTemperatureReading());
}

void main(void)
{
  CHIP_Init();
  setupSWO();

  ts = new TemperatureSensor(TEMP_SENSOR_PERIOD_MS);
  Alarm_InitializeSystem();
  
  Alarm_Create(ts->getPeriod()/ALARM_TICK_MS, false, &TempSensorTimeout);
  
  while (1)
  {
    // important: the sampleSensorData call has to be here and not in the
    // TempSensorTimeout call, since the latter executes inside the ISR
    // for RTC and lots of peripherals are disabled during that time
    // general principle: don't sample pins/do fancy stuff in ISRs except
    // moving around memory
    // one idea is to create a "deferred execution" / "pending task queue"
    // model and add a call request for the function there
    // and when we get back to this main loop, we execute it in here instead
    ts->sampleSensorData();
    
    EMU_EnterEM2(true);
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