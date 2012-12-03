// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "trace.h"
#include "rtc.h"

// include files for sensors
#include "temperaturesensor.h"


Sensor * temp_Sensor;

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	TRACE_SWOSetup();
	
	TemperatureSensor * tmp = TemperatureSensor::getInstance();
	printf("TS device id %x manid %x \n", tmp->getDeviceID(), tmp->getManufacturerID());
	temp_Sensor = tmp;
	
	uint16_t size;
	SensorMessage *msg;
	RawTemperatureMessage *tempMsg;
	
	while (1)
	{
		RTC_Trigger(1000, NULL);
		EMU_EnterEM2(true);

              printf("sample and read for Temperature sensor \n");
              temp_Sensor->sampleSensorData();
              
                      msg = (SensorMessage *) temp_Sensor->readSensorData(&size);

                      tempMsg = (RawTemperatureMessage *) msg->sensorMsgArray;
					  double a = (tempMsg->Tenv);
					  double b = (tempMsg->Vobj);
					  
                       printf("TMP: %f \n", tmp->calculateTemp(&a, &b));
					
	}

}
