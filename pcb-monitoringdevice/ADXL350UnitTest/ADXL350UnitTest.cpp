// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "trace.h"
#include "rtc.h"

// include files for sensors
#include "accelerationsensor.h"

Sensor * accl_Sensor;

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
    TRACE_SWOSetup();
            
    accl_Sensor = AccelerationSensor::getInstance();
    
    uint16_t size;
    SensorMessage *msg;
    AccelerometerMessage *acclMsg;
    int acclSampleCount = 0;
    
    while (1)
    {
      RTC_Trigger(100, NULL);
      EMU_EnterEM2(true);
      
      printf("sample and read for Accelerometer sensor \n");
      accl_Sensor->sampleSensorData();
      
      acclSampleCount++;
      if(acclSampleCount == ACCL_MAX_SAMPLES)
      {
              msg = (SensorMessage *) accl_Sensor->readSensorData(&size);
              printf("ACCL: %d samples \n", msg->arrayLength);
              for(int i=0; i < ACCL_MAX_SAMPLES; i++)
              {
                      acclMsg = &(((AccelerometerMessage *) msg->sensorMsgArray)[i]);
                      printf("sample %d : %d %d %d \n", i, acclMsg->x,
                                 acclMsg->y, acclMsg->z);
              }
              acclSampleCount = 0;
      }

    }

}
