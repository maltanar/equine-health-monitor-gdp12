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
///
#include "alarmmanager.h"
///
// include files for sensors
#include "accelerationsensor.h"

Sensor * accl_Sensor;
///
bool wakeup = false;
bool read = false;
	
void wakeupHandler(AlarmID id)
{
	wakeup = true;
}

void readHandler(AlarmID id)
{
	read = true;
}
///
/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
    TRACE_SWOSetup();
    
    /// 
    AlarmManager * mgr = AlarmManager::getInstance();
    ///
    
    accl_Sensor = AccelerationSensor::getInstance();
    
    ///
    	mgr->pause();
	mgr->createAlarm(2, false, &wakeupHandler);
	AlarmID readID = mgr->createAlarm(2, false, &readHandler);
	mgr->setAlarmTimeout(readID, 3);
	mgr->resume();
    ///
    
    uint16_t size;
    SensorMessage *msg;
    AccelerometerMessage *acclMsg;
    int acclSampleCount = 0;
	
    
    while (1)
    {
      //RTC_Trigger(100, NULL);
      EMU_EnterEM2(true);
      
      printf("sample and read for Accelerometer sensor \n");
	 // accl_Sensor->setSleepState(false);
	  //RTC_Trigger(10, NULL);
      ///
      if(wakeup)
		{
			printf("wakeup! \n");
			wakeup = false;
			accl_Sensor->setSleepState(false);
		}
      ///
      
      //EMU_EnterEM2(true);
      
      ///
      if(read)
		{
			printf("read! \n");
			read = false;
                        for (int i=0; i < 5; i++)
                        {
                          accl_Sensor->sampleSensorData();
                          acclSampleCount++;
                        
                          mgr ->lowPowerDelay(100, sleepModeEM2);
                       
                        }
			                        
                          accl_Sensor->readSensorData(&size);
                          msg = (SensorMessage *) accl_Sensor->readSensorData(&size);
                          printf("ACCL: %d samples \n", msg->arrayLength);
                          for(int i=0; i < 5; i++)
                          {
                            acclMsg = &(((AccelerometerMessage *) msg->sensorMsgArray)[i]);
                            printf("sample %d : %d %d %d \n", i, acclMsg->x,
                            acclMsg->y, acclMsg->z);
                          }
                          acclSampleCount = 0;
                        			
			accl_Sensor->setSleepState(true);
		}

    }

}
