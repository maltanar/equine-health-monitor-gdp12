// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "trace.h"
#include "gbee.h"
#include "gbee-util.h"
#include "xbee_if.h"
#include "usartmanager.h"
#include "alarmmanager.h"
#include "messagestorage.h"

// include files for sensors
#include "gpssensor.h"
#include "accelerationsensor.h"
#include "temperaturesensor.h"

#include <math.h>

#define SENSOR_COUNT		3

#define	SENSOR_TEMP_INDEX	0
#define	SENSOR_ACCL_INDEX	1
#define	SENSOR_GPS_INDEX	2

#define	SENSOR_TEMP_READ_PERIOD	2
#define	SENSOR_ACCL_READ_PERIOD	1
#define	SENSOR_GPS_READ_PERIOD	5

extern "C"
{
extern void __iar_dlmalloc_stats();
}


// local variables for the module
Sensor * sensors[SENSOR_COUNT];
AlarmManager * alarmManager;
MessageStorage *msgStore;
bool acquireNewData[SENSOR_COUNT], wakeup[SENSOR_COUNT];
AlarmID sensorAlarmId[SENSOR_COUNT], wakeupAlarmId[SENSOR_COUNT];
bool dataSaveFlag;

// Alarm handler function
void dataReadHandler(AlarmID id)
{
	// set read flag for triggered sensor
	for(int i = 0; i < SENSOR_COUNT; i++)
		if(id == sensorAlarmId[i])
		{
			acquireNewData[i] = true;
			//printf("trigger read for sensor %d \n", i);
			break;
		}
}

void deviceWakeupHandler(AlarmID id)
{
	// set wakeup flag for triggered sensor
	for(int i = 0; i < SENSOR_COUNT; i++)
		if(id == wakeupAlarmId[i])
		{
			wakeup[i] = true;
			//printf("trigger wakeup for sensor %d \n", i);
			break;
		}
}

void dataSave(AlarmID id)
{
	dataSaveFlag = true;
}

void recoverRTC()
{
	alarmManager->setUnixTime(msgStore->readRTCStorage());
}

void saveRTC()
{
	msgStore->writeRTCStorage(alarmManager->getUnixTime());
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	CHIP_Init();
	TRACE_SWOSetup();
	
	// TODO clock setting - move to own function
	/* Use 32MHZ HFXO as core clock frequency */
  	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	
	// configure GPS power pins and enable both Vcc and Vbat
	GPSSensor::configurePower();
	GPSSensor::setPower(true, true);
	
	dataSaveFlag = false;
	
	// store the alarm manager instance
	alarmManager = AlarmManager::getInstance();
	
	
	msgStore = MessageStorage::getInstance();
	msgStore->initialize("");
	
	// recover the RTC from storage, if possible
	recoverRTC();
	
	for(int i = 0; i < SENSOR_COUNT; i++)
	{
		acquireNewData[i] = false;
		wakeup[i] = false;
		wakeupAlarmId[i] = ALARM_INVALID_ID;
		sensorAlarmId[i] = ALARM_INVALID_ID;
	}
	
	alarmManager->pause();
	// create the sensor objects and alarms
	TemperatureSensor * tmp = TemperatureSensor::getInstance();
	printf("TS device id %x manid %x \n", tmp->getDeviceID(), tmp->getManufacturerID());
	tmp->setSleepState(true);
	sensors[SENSOR_TEMP_INDEX] = tmp;
	/*wakeupAlarmId[SENSOR_TEMP_INDEX] = alarmManager->createAlarm(SENSOR_TEMP_READ_PERIOD, false, &deviceWakeupHandler);
	sensorAlarmId[SENSOR_TEMP_INDEX] = alarmManager->createAlarm(SENSOR_TEMP_READ_PERIOD, false, &dataReadHandler);
	// offset the data acquire alarm by 1
	// TODO add support for fixed offsets in alarm creation
	alarmManager->setAlarmTimeout(sensorAlarmId[SENSOR_TEMP_INDEX], SENSOR_TEMP_READ_PERIOD + 1);
	*/
	
	AccelerationSensor * acc = AccelerationSensor::getInstance();
	acc->setSleepState(true);
	sensors[SENSOR_ACCL_INDEX] = acc;
	wakeupAlarmId[SENSOR_ACCL_INDEX] = alarmManager->createAlarm(SENSOR_ACCL_READ_PERIOD, false, &deviceWakeupHandler);
	sensorAlarmId[SENSOR_ACCL_INDEX] = alarmManager->createAlarm(SENSOR_ACCL_READ_PERIOD, false, &dataReadHandler);
	// offset the data acquire alarm by 1
	// TODO add support for fixed offsets in alarm creation
	alarmManager->setAlarmTimeout(sensorAlarmId[SENSOR_ACCL_INDEX], SENSOR_ACCL_READ_PERIOD + 3);
	
	GPSSensor * gps = GPSSensor::getInstance();
	sensors[SENSOR_GPS_INDEX]  = gps;
	gps->setSleepState(true);
	wakeupAlarmId[SENSOR_GPS_INDEX] = alarmManager->createAlarm(SENSOR_GPS_READ_PERIOD, false, &deviceWakeupHandler);
	sensorAlarmId[SENSOR_GPS_INDEX] = alarmManager->createAlarm(SENSOR_GPS_READ_PERIOD, false, &dataReadHandler);
	// offset the data acquire alarm by 2
	// TODO add support for fixed offsets in alarm creation
	alarmManager->setAlarmTimeout(sensorAlarmId[SENSOR_GPS_INDEX], SENSOR_GPS_READ_PERIOD + 2);
	
	// save or send data every 10 seconds
	alarmManager->createAlarm(10, false, &dataSave);
	
	GPIO_PinModeSet(GPIO_XBEE_VCC, gpioModePushPull, 0);	// XBee power
	GPIO_PinModeSet(GPIO_XBEE_DTR, gpioModePushPull, 0);	// XBee sleep
	
	GPIO_PinOutSet(GPIO_XBEE_VCC);
	alarmManager->lowPowerDelay(150);
	GPIO_PinOutClear(GPIO_XBEE_VCC);
	
	// set configuration options for XBee device
    uint8_t pan_id[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xBC, 0xCD};
    
	XBee_Config config("", "denver", false, pan_id, 0, B9600, 1);

    // initialize XBee device
    XBee interface(config);

    uint8_t error_code = interface.xbee_init();
	
    if (error_code != GBEE_NO_ERROR) {
         printf("Error: unable to configure device, code: %02x\n", error_code);
		 return 0;
    }
	
	printf("Starting period sample and send... \n");
	
	//GPIO_PinOutSet(GPIO_XBEE_DTR);
	
	
	// start counting!
	alarmManager->resume();
	
	uint16_t size;
	SensorMessage *msg;
	GPSMessage *gpsMsg;
	MessagePacket pkt;
	
	pkt.mainType = msgSensorData;
	
	RawTemperatureMessage *tempMsg;
	AccelerometerMessage *acclMsg;

	
	while (1)
	{
		EMU_EnterEM2(true);
		
		//__iar_dlmalloc_stats();
		
		if(dataSaveFlag)
		{
			dataSaveFlag = false;
			saveRTC();
			
			//GPIO_PinOutClear(GPIO_XBEE_DTR);
			printf("Xbee status: %d \n", interface.xbee_status());
			if(interface.xbee_status() == 0x00)
			{
				char * buf = NULL;
				while(buf = msgStore->getFromStorageQueueRaw(&size))
				{
					printf("sending msg, size %d \n", size);
					uint8_t ret = interface.xbee_send_data("coordinator", (const uint8_t *) buf, size);
					printf("return value %d \n", ret);
					free(buf);
				}
			}
			else
				msgStore->flushAllToDisk();
			
			//GPIO_PinOutSet(GPIO_XBEE_DTR);
			
			//FATFS_speedTest(1);
		}
		
		
		for(int i = 0; i < SENSOR_COUNT; i++)
		{
			if(wakeup[i])
			{
				printf("wakeup for sensor %d time %u \n", i, alarmManager->getUnixTime());
				sensors[i]->setSleepState(false);
				// TODO set allowable sleep state to that which is required
				// by the sensor in order not to miss its data
				wakeup[i] = false;
			}
			
			if(acquireNewData[i])
			{
				pkt.relTimestampS = alarmManager->getUnixTime();
				printf("sample and read for sensor %d time %u \n", i, pkt.relTimestampS);
				
				
				if(i != SENSOR_ACCL_INDEX)
				{
					sensors[i]->sampleSensorData();
					msg = (SensorMessage *) sensors[i]->readSensorData(&size);
					pkt.payload = (uint8_t *) msg;
					msgStore->addToStorageQueue(&pkt, size + sizeof(MessagePacket));
				} else {
					for(int c = 0; c < ACCL_MAX_SAMPLES; c++)
					{
						sensors[i]->sampleSensorData();
						//alarmManager->lowPowerDelay(350, sleepModeEM2);
					}
				}
				//send device back to sleep
				sensors[i]->setSleepState(true);
				switch(i)
				{
				  case SENSOR_GPS_INDEX:
					// if parseOnReceive is set, no need to sample ourselves?
					gpsMsg = (GPSMessage *) msg->sensorMsgArray;
					printf("GPS: %d %d %d, %d %d %d \n", gpsMsg->latitude.degree,
						   gpsMsg->latitude.minute, gpsMsg->latitude.second, 
						   gpsMsg->longitude.degree, gpsMsg->longitude.minute, 
						   gpsMsg->longitude.second);
					break;
				  case SENSOR_TEMP_INDEX:
					tempMsg = (RawTemperatureMessage *) msg->sensorMsgArray;
					printf("TMP: %f \n", TemperatureSensor::calculateTemp(tempMsg->Tenv, tempMsg->Vobj));
					break;
				  case SENSOR_ACCL_INDEX:
					msg = (SensorMessage *) sensors[i]->readSensorData(&size);
					printf("ACCL: %d samples \n", msg->arrayLength);
					for(int j=0; j < msg->arrayLength; j++)
					{
						acclMsg = &(((AccelerometerMessage *) msg->sensorMsgArray)[i]);
						printf("sample %d : %d %d %d \n", i, acclMsg->x,
							   acclMsg->y, acclMsg->z);
					}
					pkt.payload = (uint8_t *) msg;
					msgStore->addToStorageQueue(&pkt, size + sizeof(MessagePacket));
					break;
				  default:
					;
				}
				msg->endTimestampS = alarmManager->getUnixTime();
				acquireNewData[i] = false;
			}
		}
	}

}
