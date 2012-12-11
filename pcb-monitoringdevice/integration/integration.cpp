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
#include "anthrmsensor.h"

#include "audio.h"

// include files for sensors
#include "gpssensor.h"
#include "accelerationsensor.h"
#include "temperaturesensor.h"

#include <math.h>

extern "C"
{
extern void __iar_dlmalloc_stats();
}

// Section: Local definitions and macros -------------------------------------

#define SENSOR_COUNT				4
#define DATA_SEND_PERIOD			10
#define MAX_DEBUG_MSG				255

// End Section: Local definitions and macros ---------------------------------


// Section: Local Type Definitions --------------------------------------------

// used to control the tasks executed in the main loop
// the alarm handler for each sensor is responsible for setting the flags
// and the main loop will clear the flags when requests are fulfilled
typedef struct {
	Sensor * sensor;
	bool requestSleep;
	bool requestWakeup;
	uint8_t requestSamples;
} SensorControlBlock;

typedef struct {
	uint8_t period;
	uint8_t readOffset;
	uint8_t samples;
	bool enabled;
	bool requestChange;
} SensorParameters;

// End Section: Local Type Definitions ----------------------------------------

// Section: Local Helper Function Declarations --------------------------------

void initializeMCU();						// initialize clocks, debug
void initializeDevicePowerPins();			// config GPIOs used for pwr control
void saveAudioSample(uint16_t audioLenS);	// save audio sample of given length
void md_printf(int len);					// print over ZigBee / DebugMsg
void recoverRTC();							// read RTC from SD card
void saveRTC();								// save RTC to SD card
bool configureZigBee();
void configureDataCollection();
void sendOrSaveData();
void receiveData();
void handleConfigSensorMsg(ConfigSensor *msg);
// Section: Local Helper Function Declarations --------------------------------


// Section: Local Variables ---------------------------------------------------
bool dataSaveFlag;
bool zigbeeOK;
char mdMessageBuffer[255];
AlarmManager * alarmManager;
MessageStorage *msgStore;
XBee *xbee;
Audio * mic;

SensorControlBlock sensorControl[SENSOR_COUNT];
SensorParameters sensorParam[SENSOR_COUNT] =
{
	// GPS configuration
	{
		.period = 10,
		.readOffset = 5,
		.samples = 1,
		.enabled = true,
		.requestChange = false
	},
	// Accelerometer configuration
	{
		.period = 5,
		.readOffset = 1,
		.samples = 10,
		.enabled = true,
		.requestChange = false
	},
	// Temperature sensor configuration
	{
		.period = 5,
		.readOffset = 1,
		.samples = 1,
		.enabled = true,
		.requestChange = false
	},
	// Heart Rate Monitor configuration
	{
		.period = 5,
		.readOffset = 2,
		.samples = 1,
		.enabled = true,
		.requestChange = false
	}
};
	
// End Section: Local Variables -----------------------------------------------


// Section: Callback function declarations  -----------------------------------

// executed on data save alarm - simply set save flag to true
void dataSaveHandler(AlarmID id);
void tempSensorHandler(AlarmID id);
void acclSensorHandler(AlarmID id);
void gpsSensorHandler(AlarmID id);
void antSensorHandler(AlarmID id);
// End Section: Callback functions --------------------------------------------


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	// set up debug output and clocks
	initializeMCU();
	
	// store the message storage instance
	msgStore = MessageStorage::getInstance();
	msgStore->initialize("/", true);	// TODO FINAL remove the true
	
	// store the alarm manager instance
	alarmManager = AlarmManager::getInstance();
	
	// recover the RTC from storage, if possible
	recoverRTC();
	
	// initialize the device power pins
	initializeDevicePowerPins();
	
	// pause the alarm manager as we create the alarms in the next section
	// this way we can start them all simultaneously
	alarmManager->pause();
	
	// create the sensor objects and alarms, call handlers with NULL
	tempSensorHandler(NULL);
	acclSensorHandler(NULL);
	gpsSensorHandler(NULL);
	antSensorHandler(NULL);
	
	printf("Configuring data collection...\n");
	configureDataCollection();
	printf("Configuring ZigBee...\n");
	configureZigBee();
	
	printf("Starting periodic sample and send... \n");
	
	// start counting!
	alarmManager->resume();
	
	
	uint16_t size;
	SensorMessage *msg;
	MessagePacket pkt;
	
	pkt.mainType = msgSensorData;
	

	// md_printf(sprintf(mdMessageBuffer, "Hi! This is the monitoring device speaking"));
	
	while (1)
	{
		EMU_EnterEM2(true);
		
		// uncomment to view heap status
		//__iar_dlmalloc_stats();
		
		if(dataSaveFlag)
		{
			sendOrSaveData();
		}
		
		
		for(int i = 0; i < SENSOR_COUNT; i++)
		{
			Sensor * currentSensor = sensorControl[i].sensor;
			
			if(sensorControl[i].requestWakeup)
			{
				printf("wakeup for sensor %d time %u \n", i, alarmManager->getUnixTime());
				currentSensor->setSleepState(false);
				// TODO set allowable sleep state to that which is required
				// by the sensor in order not to miss its data
				sensorControl[i].requestWakeup = false;
			}
			
			// if samples were requested for this sensor...
			if(sensorControl[i].requestSamples)
			{
				pkt.relTimestampS = alarmManager->getUnixTime();
				printf("sample and read for sensor %d time %u \n", i, 
					   pkt.relTimestampS);
				
				// acquire the desired number of samples
				while(sensorControl[i].requestSamples)
				{
					sensorControl[i].requestSamples--;
					currentSensor->sampleSensorData();
				}
				
				// read out the samples
				msg = (SensorMessage *) currentSensor->readSensorData(&size);
				pkt.payload = (uint8_t *) msg;
				msg->endTimestampS = alarmManager->getUnixTime();
				
				// add to storage queue
				msgStore->addToStorageQueue(&pkt, size + sizeof(MessagePacket));
			}
			
			//send device back to sleep if requested
			if(sensorControl[i].requestSleep)
			{
				sensorControl[i].requestSleep = false;
				currentSensor->setSleepState(true);
			}
		}
	}

}


// Section: Local helper function implementations ----------------------------

void initializeMCU()
{
	// handle chip errata
	CHIP_Init();
	
	// enable SWO output for printf over SWO
	TRACE_SWOSetup();
	// enable trace during deep sleep as well
	// consumes additional power but worth it for debugging
	EMU->CTRL |= EMU_CTRL_EMVREG_FULL;
	
	// configure the system clocks
	SystemHFXOClockSet(48000000); 
	
	// enable the external crystal oscillators
	CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	
	// select the external crystal oscillators and deselect old ones
	CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
	CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
}


void saveAudioSample(uint16_t audioLenS)
{
	printf("start acquiring audio sample, duration %d seconds \n", audioLenS);
	const uint16_t bufferSize = 1000;
	mic = new Audio(Fs_8khz, bufferSize);
	mic->init();

	msgStore->startAudioSample();	
	mic->startRecording(audioLenS);
	
	int buffer_count =0;

	audioStatus_typedef micStatus;
	micStatus = mic->getStatus();

	uint16_t *new_buffer;

	while (micStatus == recording) 
	{
		new_buffer = (uint16_t *)mic->getBuffer();

		if (new_buffer != NULL)
		{
			buffer_count++;
			
			msgStore->flushAudioSample((char *) new_buffer, bufferSize * sizeof(uint16_t));
			
		}
		EMU_EnterEM1();
		micStatus = mic->getStatus();
	}
	
	new_buffer = (uint16_t *)mic->getBuffer();
	
	if(new_buffer)
	{
		printf("got last buffer! \n");
		msgStore->flushAudioSample((char *) new_buffer, bufferSize * sizeof(uint16_t));
	}
	else
		printf("no last buffer! \n");
	
	msgStore->endAudioSample();
	
	printf("end acquiring audio sample \n");
}

// send zero-terminated buffer in mdMessageBuffer as a DebugMessage over ZigBee
void md_printf(int len)
{	
	if(!alarmManager || !msgStore)
		return;
	
	if(len >= 255)
	{
		printf("debug msg too big! \n");
		return;
	}
	
	MessagePacket msg_pkt;
	msg_pkt.mainType = msgDebug;
  	msg_pkt.relTimestampS = alarmManager->getUnixTime();
	DebugMessage dbg_msg;
	dbg_msg.timestampS = alarmManager->getUnixTime();
  	dbg_msg.debugData = (uint8_t *) mdMessageBuffer;
	msg_pkt.payload = (uint8_t *) &dbg_msg;
	msgStore->addToStorageQueue(&msg_pkt, len + sizeof(DebugMessage) 
								+ sizeof(MessagePacket));
}

// read stored RTC value from SD card
void recoverRTC()
{
	alarmManager->setUnixTime(msgStore->readRTCStorage());
}

// store RTC value in SD card
void saveRTC()
{
	msgStore->writeRTCStorage(alarmManager->getUnixTime());
}

void initializeDevicePowerPins()
{
	// configure the GPIO pins we use to control the power to the
	// ANT, GPS and the XBee module
	XBEE_GPIO_CONFIG();
	GPSSensor::configurePower();
	ANTHRMSensor::configurePower();
}


void configureDataCollection()
{
	dataSaveFlag = false;
	alarmManager->createAlarm(DATA_SEND_PERIOD, false, &dataSaveHandler);
}

bool configureZigBee()
{
	zigbeeOK = false;
	
	// Xbee hard reset - turn power off for 150 ms
	XBEE_POWER(false);
	alarmManager->lowPowerDelay(150);
	XBEE_POWER(true);
	
	// set configuration options for XBee device
    uint8_t pan_id[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xBC, 0xCD};
    
	XBee_Config config("", "denver", false, pan_id, 0, B9600, 1);

    // initialize XBee device
    xbee = new XBee(config);

    uint8_t error_code = xbee->xbee_init();
	zigbeeOK = (error_code == GBEE_NO_ERROR);
	
    if (error_code != GBEE_NO_ERROR) {
         printf("Error: unable to configure device, code: %02x\n", error_code);
    } else
		printf("ZigBee OK!");
	
	return zigbeeOK;
}

void sendOrSaveData()
{
	printf("***** send or save data *****\n");
	uint16_t size;
	dataSaveFlag = false;
	saveRTC();
	
	if(!zigbeeOK)
	{
		msgStore->flushAllToDisk();
	}
	char * buf = NULL;
	
	printf("Xbee status: %d \n", xbee->xbee_status());
	if(xbee->xbee_status() == 0x00)
	{
		while(buf = msgStore->getFromStorageQueueRaw(&size))
		{
			printf("sending msg, size %d \n", size);
			uint8_t ret = xbee->xbee_send_data("coordinator", 
											   (const uint8_t *) buf, size);
			printf("return value %d \n", ret);
			free(buf);
		}
		
		// TODO uncomment if receiption causes errors
		receiveData();
	}
	else
		msgStore->flushAllToDisk();
	
	printf("***** send or save data done *****\n");
}

void receiveData()
{
	uint16_t size = 0;
	uint8_t * buf = NULL;
		
	// see if the base station sent us anything
	printf("Receiving XBee message.. \n");
	XBee_Message *rcv = xbee->xbee_receive_message();
	if(rcv)
	{
		size = 0;
		buf = rcv->get_payload(&size);
		if(size > 0)
		{
			printf("got message! \n");
			MessagePacket * pkt = (MessagePacket *) malloc(size + 8);
			MessageStorage::deserialize(buf, pkt);
			if(pkt->mainType == msgSensorConfig)
			{
				ConfigMessage * cfg = (ConfigMessage *) pkt->payload;
				ConfigSensor * cfgArray = (ConfigSensor *) cfg->configMsgArray;
				
				// traverse the received config messages and handle each
				// with helper function
				for(int i = 0; i < cfg->arrayLength; i++)
					handleConfigSensorMsg(&cfgArray[i]);
			}
			else
				printf("What to do with type %d? \n", pkt->mainType);
				
			// free the deserialization buffer
			free(pkt);
		}
		// free the XBee_Message
		delete rcv;
	}
	else
		printf("no message! \n");
}

void handleConfigSensorMsg(ConfigSensor *msg)
{
	printf("sensor config: %d %d %d \n", msg->sensorType, 
		   msg->enableSensor, msg->samplePeriodMs);
	
	if(msg->sensorType > SENSOR_COUNT)
	{
		printf("error, cfg for sensor %d undefined! \n", msg->sensorType);
		return;
	}
	
	// change sensorParam for the corresponding sensor
	sensorParam[msg->sensorType].enabled = msg->enableSensor;
	sensorParam[msg->sensorType].period = msg->samplePeriodMs;	// TODO one of these should be s
	sensorParam[msg->sensorType].readOffset = msg->sampleIntervalMs;	// TODO one of these should be s
	sensorParam[msg->sensorType].requestChange = true;
}

// End Section: Local helper function implementations -------------------------

// Section: Alarm callback handlers / task control ----------------------------
// The function in this section will be called from inside ISR and should be
// as short as possible.

// data save callback handler - just set flag to true
void dataSaveHandler(AlarmID id)
{
	dataSaveFlag = true;
}

// temperature sensor callback handler
void tempSensorHandler(AlarmID id)
{
	const uint8_t i = typeRawTemperature;
	SensorParameters * params = &sensorParam[i];
	SensorControlBlock * control = &sensorControl[i];	// for convenience
	static TemperatureSensor * tmp = NULL;
	static AlarmID wakeupAlarmID = ALARM_INVALID_ID;
	static AlarmID readAlarmID = ALARM_INVALID_ID;
	
	// detect first run - create and configure sensor
	if(tmp == NULL)
	{
		printf("Configuring TMP006...\n");
		// get the sensor instance
		tmp = TemperatureSensor::getInstance();
		// put device to sleep
		tmp->setSleepState(true);
		// create the wakeup alarm
		wakeupAlarmID = alarmManager->createAlarm(params->period,
												  false, &tempSensorHandler);
		// create the read alarm
		readAlarmID = alarmManager->createAlarm(params->period,
												  false, &tempSensorHandler);
		
		// offset the read alarm - it needs to be triggered after the wakeup
		alarmManager->setAlarmTimeout(readAlarmID, params->period + 
									  params->readOffset);
		
		// initialize the control block structure
		control->requestWakeup = true;
		control->requestSleep = false;
		control->requestSamples = 0;
		control->sensor = tmp;
	} 
	
	if(params->requestChange)
	{
		// want to change sensor parameters
		params->requestChange = false;
		printf("changing sensor parameters... \n");
		// TODO implement the actual changes
	}
	else
	{
		// sensor has already been created - take action depending on
		// alarm identifier
		if(id == wakeupAlarmID)
		{
			control->requestWakeup = true;
		}
		else if(id == readAlarmID)
		{
			control->requestSamples = params->samples;
			control->requestSleep = true;
		}
	}
}

// accelerometer callback handler
void acclSensorHandler(AlarmID id)
{
	const uint8_t i = typeAccelerometer;
	SensorParameters * params = &sensorParam[i];
	SensorControlBlock * control = &sensorControl[i];	// for convenience
	static AlarmID wakeupAlarmID = ALARM_INVALID_ID;
	static AlarmID readAlarmID = ALARM_INVALID_ID;
	
	static AccelerationSensor * acc = NULL;
	
	// detect first run - create and configure sensor
	if(acc == NULL)
	{
		printf("Configuring ADXL350...\n");
		// get the sensor instance
		acc = AccelerationSensor::getInstance();
		// put device to sleep
		acc->setSleepState(true);
		// create the wakeup alarm
		wakeupAlarmID = alarmManager->createAlarm(params->period,
												  false, &acclSensorHandler);
		// create the read alarm
		readAlarmID = alarmManager->createAlarm(params->period,
												  false, &acclSensorHandler);
		
		// offset the read alarm - it needs to be triggered after the wakeup
		alarmManager->setAlarmTimeout(readAlarmID, params->period + 
									  params->readOffset);
		
		// initialize the control block structure
		control->requestWakeup = true;
		control->requestSleep = false;
		control->requestSamples = 0;
		control->sensor = acc;
	} 
	else
	{
		// sensor has already been created - take action depending on
		// alarm identifier
		if(id == wakeupAlarmID)
		{
			control->requestWakeup = true;
		}
		else if(id == readAlarmID)
		{
			control->requestSamples = params->samples;
			control->requestSleep = true;
		}
	}
}


// GPS callback handler
void gpsSensorHandler(AlarmID id)
{
	const uint8_t i = typeGPS;
	SensorParameters * params = &sensorParam[i];
	SensorControlBlock * control = &sensorControl[i];	// for convenience
	static AlarmID wakeupAlarmID = ALARM_INVALID_ID;
	static AlarmID readAlarmID = ALARM_INVALID_ID;
	
	static GPSSensor * gps = NULL;
	
	// detect first run - create and configure sensor
	if(gps == NULL)
	{
		printf("Configuring GPS...\n");
		// get the sensor instance
		gps = GPSSensor::getInstance();
		gps->initialize();
		gps->setParseOnReceive(true);
		// put device to sleep
		gps->setSleepState(true);
		
		// create the wakeup alarm
		wakeupAlarmID = alarmManager->createAlarm(params->period,
												  false, &gpsSensorHandler);
		// create the read alarm
		readAlarmID = alarmManager->createAlarm(params->period,
												  false, &gpsSensorHandler);
		
		// offset the read alarm - it needs to be triggered after the wakeup
		alarmManager->setAlarmTimeout(readAlarmID, params->period + 
									  params->readOffset);
		
		// initialize the control block structure
		control->requestWakeup = true;
		control->requestSleep = false;
		control->requestSamples = 0;
		control->sensor = gps;
	} 
	else
	{
		// sensor has already been created - take action depending on
		// alarm identifier
		if(id == wakeupAlarmID)
		{
			control->requestWakeup = true;
		}
		else if(id == readAlarmID)
		{
			control->requestSamples = params->samples;
			control->requestSleep = true;
		}
	}
}

// ANT callback handler
void antSensorHandler(AlarmID id)
{
	const uint8_t i = typeHeartRate;
	SensorParameters * params = &sensorParam[i];
	SensorControlBlock * control = &sensorControl[i];	// for convenience
	static AlarmID wakeupAlarmID = ALARM_INVALID_ID;
	static AlarmID readAlarmID = ALARM_INVALID_ID;
	
	static ANTHRMSensor * ant = NULL;
	
	// detect first run - create and configure sensor
	if(ant == NULL)
	{
		printf("Configuring ANT...\n");
		// get the sensor instance
		ant = ANTHRMSensor::getInstance();
		ant->initializeNetwork();
		// put device to sleep
		ant->setSleepState(true);
		
		// create the wakeup alarm
		wakeupAlarmID = alarmManager->createAlarm(params->period,
												  false, &antSensorHandler);
		// create the read alarm
		readAlarmID = alarmManager->createAlarm(params->period,
												  false, &antSensorHandler);
		
		// offset the read alarm - it needs to be triggered after the wakeup
		alarmManager->setAlarmTimeout(readAlarmID, params->period + 
									  params->readOffset);
		
		// initialize the control block structure
		control->requestWakeup = true;
		control->requestSleep = false;
		control->requestSamples = 0;
		control->sensor = ant;
	} 
	else
	{
		// sensor has already been created - take action depending on
		// alarm identifier
		if(id == wakeupAlarmID)
		{
			control->requestWakeup = true;
		}
		else if(id == readAlarmID)
		{
			control->requestSamples = params->samples;
			control->requestSleep = true;
		}
	}
}


// End Section: Alarm callback handlers / task control ------------------------