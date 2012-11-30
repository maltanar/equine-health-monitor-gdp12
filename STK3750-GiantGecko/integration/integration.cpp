// University of Southampton, 2012
// EMECS Group Design Project

#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "dvk.h"
#include "trace.h"
#include "gbee.h"
#include "gbee-util.h"
#include "xbee_if.h"
#include "uartmanager.h"
#include "rtc.h"
#include "alarmmanager.h"

// include files for sensors
#include "gpssensor.h"
#include "accelerationsensor.h"
#include "temperaturesensor.h"

#define SENSOR_COUNT		3

#define	SENSOR_TEMP_INDEX	0
#define	SENSOR_ACCL_INDEX	1
#define	SENSOR_GPS_INDEX	2

#define	SENSOR_TEMP_READ_PERIOD	2
#define	SENSOR_ACCL_READ_PERIOD	1
#define	SENSOR_GPS_READ_PERIOD	4

// local variables for the module
Sensor * sensors[SENSOR_COUNT];
AlarmManager * alarmManager;
bool acquireNewData[SENSOR_COUNT];
AlarmID sensorAlarmId[SENSOR_COUNT];

// Alarm handler function
void dataReadHandler(AlarmID id)
{
	// set read flag for triggered sensor
	for(int i = 0; i < SENSOR_COUNT; i++)
		if(id == sensorAlarmId[i])
		{
			acquireNewData[i] = true;
			printf("trigger read for sensor %d \n", i);
			break;
		}
}

// ----------------------------------------------------------------------------
// SD card testing section
// ----------------------------------------------------------------------------

#include "ff.h"
#include "microsd.h"
#include "diskio.h"

#define BUFFERSIZE      1024
//#define BUFFERSIZE      512

/* Filename to open/write/read from SD-card */

#define TEST_FILENAME    "mnoprs.txt"

/***************************************************************************//**
 * @brief
 *   This function is required by the FAT file system in order to provide
 *   timestamps for created files. Since we do not have a reliable clock we
 *   hardcode a value here.
 *
 *   Refer to drivers/fatfs/doc/en/fattime.html for the format of this DWORD.
 * @return
 *    A DWORD containing the current time and date as a packed datastructure.
 ******************************************************************************/
DWORD get_fattime(void)
{
	// TODO connect this to the AlarmManager's unix time
  return (28 << 25) | (2 << 21) | (1 << 16);
}

void testSDCard()
{
	
	FIL fsrc;				/* File objects */
	FATFS Fatfs;				/* File system specific */
	FRESULT res;				/* FatFs function common result code */
	UINT br, bw;				/* File read/write count */
	DSTATUS resCard;			/* SDcard status */
	int8_t ramBufferWrite[BUFFERSIZE];	/* Temporary buffer for write file */
	int8_t ramBufferRead[BUFFERSIZE];	/* Temporary buffer for read file */
	int8_t StringBuffer[] = "Hi Konke, can you read this? Say w0000t if you can!";
	int16_t i;
  int16_t filecounter;

	/*Step1*/
	/*Initialization file buffer write */
	filecounter = sizeof(StringBuffer);

	for(i = 0; i < filecounter ; i++)
	{
	 ramBufferWrite[i] = StringBuffer[i];
	}

	/*Step2*/
	/*Detect micro-SD*/
	while(1)
	{
	MICROSD_init();
	resCard = disk_initialize(0);       /*Check micro-SD card status */

	switch(resCard)
	{
	case STA_NOINIT:                    /* Drive not initialized */
	  break;
	case STA_NODISK:                    /* No medium in the drive */
	  break;
	case STA_PROTECT:                   /* Write protected */
	  break;
	default:
	  break;
	}

	if (!resCard) break;                /* Drive initialized. */

	//Delay(1);    
	}

	/*Step3*/
	/* Initialize filesystem */
	if (f_mount(0, &Fatfs) != FR_OK)
	{
	/* Error.No micro-SD with FAT32 is present */
	while(1);
	}

	/*Step4*/
	/* Open  the file for write */
	res = f_open(&fsrc, TEST_FILENAME,  FA_WRITE); 
	if (res != FR_OK)
	{
	 /*  If file does not exist create it*/ 
	 res = f_open(&fsrc, TEST_FILENAME, FA_CREATE_ALWAYS | FA_WRITE ); 
	  if (res != FR_OK) 
	 {
	  /* Error. Cannot create the file */
	  while(1);
	}
	}

	/*Step5*/
	/*Set the file write pointer to first location */ 
	res = f_lseek(&fsrc, 0);
	if (res != FR_OK) 
	{
	/* Error. Cannot set the file write pointer */
	while(1);
	}

	/*Step6*/
	/*Write a buffer to file*/
	res = f_write(&fsrc, ramBufferWrite, filecounter, &bw);
	if ((res != FR_OK) || (filecounter != bw)) 
	{
	/* Error. Cannot write the file */
	while(1);
	}

	/*Step7*/
	/* Close the file */
	f_close(&fsrc);
	if (res != FR_OK) 
	{
	/* Error. Cannot close the file */
	while(1);
	}

	/*Step8*/
	/* Open the file for read */
	res = f_open(&fsrc, TEST_FILENAME,  FA_READ); 
	if (res != FR_OK) 
	{
	/* Error. Cannot create the file */
	while(1);
	}

	/*Step9*/
	/*Set the file read pointer to first location */ 
	res = f_lseek(&fsrc, 0);
	if (res != FR_OK) 
	{
	/* Error. Cannot set the file pointer */
	while(1);
	}

	/*Step10*/
	/* Read some data from file */
	res = f_read(&fsrc, ramBufferRead, filecounter, &br);
	if ((res != FR_OK) || (filecounter != br)) 
	{
	/* Error. Cannot read the file */
	while(1);
	}

	/*Step11*/
	/* Close the file */
	f_close(&fsrc);
	if (res != FR_OK) 
	{
	/* Error. Cannot close the file */
	while(1);
	}

	/*Step12*/
	/*Compare ramBufferWrite and ramBufferRead */
	for(i = 0; i < filecounter ; i++)
	{
	if ((ramBufferWrite[i]) != (ramBufferRead[i]))
	{
	  /* Error compare buffers*/
	  while(1);
	}
	}
}
// ----------------------------------------------------------------------------
// end of SD card testing section
// ----------------------------------------------------------------------------

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	/* Configure board. Select either EBI or SPI mode. */
	DVK_init(DVK_Init_SPI);
	TRACE_SWOSetup();
	
	// TODO clock setting - move to own function
	/* Use 32MHZ HFXO as core clock frequency */
  	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	
	// TODO only needed for the DVK
	/* Enable SPI access to MicroSD card */
  	DVK_peripheralAccess(DVK_MICROSD, true);
	
	printf("starting SD card test... \n");
	testSDCard();
	printf("SD card test OK!... \n");
	
	// store the alarm manager instance, paused on creation
	alarmManager = AlarmManager::getInstance();
	
	// create the sensor objects and alarms
	TemperatureSensor * tmp = new TemperatureSensor(1000);
	printf("TS device id %x manid %x \n", tmp->getDeviceID(), tmp->getManufacturerID());
	sensors[SENSOR_TEMP_INDEX] = tmp;
	acquireNewData[SENSOR_TEMP_INDEX] = false;
	sensorAlarmId[SENSOR_TEMP_INDEX] = alarmManager->createAlarm(SENSOR_TEMP_READ_PERIOD, false, &dataReadHandler);
	
	/*sensors[SENSOR_ACCL_INDEX] = new AccelerationSensor(100);
	acquireNewData[SENSOR_ACCL_INDEX] = false;
	sensorAlarmId[SENSOR_ACCL_INDEX] = alarmManager->createAlarm(SENSOR_ACCL_READ_PERIOD, false, &dataReadHandler);
	
	sensors[SENSOR_GPS_INDEX]  = GPSSensor::getInstance();
	//((GPSSensor*)sensors[SENSOR_GPS_INDEX])->setParseOnReceive(true);
	acquireNewData[SENSOR_GPS_INDEX] = false;
	sensorAlarmId[SENSOR_GPS_INDEX] = alarmManager->createAlarm(SENSOR_GPS_READ_PERIOD, false, &dataReadHandler);*/
	
	// start counting!
	alarmManager->resume();
	
	uint16_t size;
	SensorMessage *msg;
	GPSMessage *gps;
	TemperatureMessage *tempMsg;
	AccelerometerMessage *acclMsg;
	int acclSampleCount = 0;
	
	while (1)
	{
		EMU_EnterEM2(true);
		
		for(int i = 0; i < SENSOR_COUNT; i++)
			if(acquireNewData[i])
			{
				printf("sample and read for sensor %d \n", i);
				sensors[i]->sampleSensorData();
				if(i != SENSOR_ACCL_INDEX)
					msg = (SensorMessage *) sensors[i]->readSensorData(&size);
				switch(i)
				{
				  case SENSOR_GPS_INDEX:
					// if parseOnReceive is set, no need to sample ourselves?
					gps = (GPSMessage *) msg->sensorMsgArray;
					printf("GPS: %d %d %d, %d %d %d \n", gps->latitude.degree,
						   gps->latitude.minute, gps->latitude.second, 
						   gps->longitude.degree, gps->longitude.minute, 
						   gps->longitude.second);
					break;
				  case SENSOR_TEMP_INDEX:
					tempMsg = (TemperatureMessage *) msg->sensorMsgArray;
					printf("TMP: %f \n", tempMsg->Tobj);
					break;
				  case SENSOR_ACCL_INDEX:
					acclSampleCount++;
					if(acclSampleCount == ACCL_MAX_SAMPLES)
					{
						msg = (SensorMessage *) sensors[i]->readSensorData(&size);
						printf("ACCL: %d samples \n", msg->arrayLength);
						for(int i=0; i < ACCL_MAX_SAMPLES; i++)
						{
							acclMsg = &(((AccelerometerMessage *) msg->sensorMsgArray)[i]);
							printf("sample %d : %d %d %d \n", i, acclMsg->x,
								   acclMsg->y, acclMsg->z);
						}
						acclSampleCount = 0;
					}
					break;
				  default:
					;
				}
				msg->endTimestampS = alarmManager->getUnixTime();
				acquireNewData[i] = false;
				break;
			}
	}

}
