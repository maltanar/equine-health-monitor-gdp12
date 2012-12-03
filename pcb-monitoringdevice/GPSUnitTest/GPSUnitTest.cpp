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
#include "gpssensor.h"

#define	SENSOR_GPS_READ_PERIOD	4

Sensor * gps_Sensor;

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	TRACE_SWOSetup();
	
	gps_Sensor  = GPSSensor::getInstance();
	
	uint16_t size;
	SensorMessage *msg;
	GPSMessage *gps;
	
	while (1)
	{
          // uncomment the line below if the device does not wake from EM2
          // RTC_Trigger(1000, NULL);
          EMU_EnterEM2(true);

          printf("sample and read for gps sensor \n");
          gps_Sensor->sampleSensorData();
          
          msg = (SensorMessage *) gps_Sensor->readSensorData(&size);
            

                  gps = (GPSMessage *) msg->sensorMsgArray;
                  printf("GPS: %d %d %d, %d %d %d \n", gps->latitude.degree,
                             gps->latitude.minute, gps->latitude.second, 
                             gps->longitude.degree, gps->longitude.minute, 
                             gps->longitude.second);
	
	}

}
