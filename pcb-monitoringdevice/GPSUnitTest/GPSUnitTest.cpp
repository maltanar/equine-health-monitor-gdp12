// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "trace.h"
#include "gbee.h"
#include "gbee-util.h"
#include "xbee_if.h"
#include "uartmanager.h"
#include "rtc.h"
//#include "alarmmanager.h"

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
		EMU_EnterEM2(true);
		
			//if(gps_AcquireNewData)
			//{
				printf("sample and read for gps sensor \n");
				gps_Sensor->sampleSensorData();
				
				msg = (SensorMessage *) gps_Sensor->readSensorData(&size);
				  
				  // case SENSOR_GPS_INDEX:
					// // if parseOnReceive is set, no need to sample ourselves?
					gps = (GPSMessage *) msg->sensorMsgArray;
					printf("GPS: %d %d %d, %d %d %d \n", gps->latitude.degree,
						   gps->latitude.minute, gps->latitude.second, 
						   gps->longitude.degree, gps->longitude.minute, 
						   gps->longitude.second);
	
	}

}
