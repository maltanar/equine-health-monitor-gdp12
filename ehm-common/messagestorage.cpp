#include "messagestorage.h"
#include <string.h>
#include <stdlib.h>

#ifdef EHM_MONITORING_DEVICE
#include "alarmmanager.h"
#include "debug_output_control.h"
#elif EHM_BASE_STATION
#include "debug_output_control.h"
#else
#define module_debug_strg(fmt, ...)
#endif

MessageStorage::MessageStorage()
{
	m_storageRoot = NULL;
	m_storageOK = m_fileOpen = false;
	m_nextMessageSeqNumber = 1;
	m_queueCount = 0;
	m_queueCountMem = 0;
}

void MessageStorage::initialize(char * storageRoot)
{
	if(storageRoot == NULL)
	{
		module_debug_strg("cannot work without storage root!");
		return;
	}
	
	strcpy(storageRoot, m_storageRoot);
	
	// mount the actual storage device we use for storing data
	module_debug_strg("mounting storage...");
	if(mountStorage())
		module_debug_strg("storage now available!");
	else
		module_debug_strg("storage not available!");
	
	// count list of existing files in storage root,
	module_debug_strg("counting files...");
	
	// TODO reenable file counting
	/*m_queueCount = */getDirFileCount("");
}


/* Function will de-serialize data into a MessagePacket structure.
 * data[in]: const Pointer to memory to de-serialize
 * msg[out]: Pointer to preallocated MessagePacket struct of sufficient size 
 * 
 * Hint: use size returned by XBee_Message->get_payload(&size) and add 
 * sizeof(MessagePacket) + sizeof(void*) to get the exact required size*/
void MessageStorage::deserialize(const uint8_t *data, MessagePacket *msg) {
	/* !sizeof(MessagePacket) returns 12, but we do not serialize the pointer
	 * member and can pack the mainType into the first byte of the data,
	 * hence the serialized size of this struct is 5 Byte instead of sizeof(MessagePacket) */
	#define  MESSAGE_PACKET_SIZE 5
	
	/* MessageType is always the first byte of the serialized data */
	msg->mainType = (MessageType)data[0];
	msg->relTimestampS = *(uint32_t *)&data[1];
	msg->payload = (uint8_t *)&msg->payload + sizeof(msg->payload);
	
	/* Copy the message group specific fields into the MessagePacket */
	SensorMessage *sensor_msg = NULL; 
	ConfigMessage *config_msg = NULL;
	DebugMessage *debug_msg = NULL;

	switch (msg->mainType) {
	case msgSensorData:
		sensor_msg = (SensorMessage *)msg->payload;
		memcpy(msg->payload, &data[MESSAGE_PACKET_SIZE], sizeof(SensorMessage));
		sensor_msg->sensorMsgArray = (uint8_t *)&sensor_msg->sensorMsgArray + sizeof(void *);
		break;
	case msgSensorConfig:
		config_msg = (ConfigMessage *)msg->payload;
		memcpy(msg->payload, &data[MESSAGE_PACKET_SIZE], sizeof(ConfigMessage));
		config_msg->configMsgArray = (uint8_t *)&config_msg->configMsgArray + sizeof(void *);
		break;
	case msgDebug:
		debug_msg = (DebugMessage *)msg->payload;
		memcpy(msg->payload, &data[MESSAGE_PACKET_SIZE], sizeof(DebugMessage));
		debug_msg->debugData = (uint8_t *)&debug_msg->debugData + sizeof(void *);
		break;
	}

	/* copy the message type specific payload */
	if (msg->mainType == msgSensorData) {
		/* calculate the address offset for the sensorMsgArray in the
		 * serialized data. The offset can be found by adding the size of
		 * the structs that come before the sensorMsgArray and deducting
		 * the size of the pointer members of the struct, because they 
		 * are not serialized. In this case there is one pointer member that
		 * needs to be deducted (*sensorMsgArray in SensorMessage) */
		int sensorMsgArrayOffset = MESSAGE_PACKET_SIZE + sizeof(SensorMessage) - sizeof(void *);
		
		/* copy the sensorMsgArray into the serialized data structure */
		switch (sensor_msg->sensorType) {
		case typeHeartRate:
			memcpy(sensor_msg->sensorMsgArray, &data[sensorMsgArrayOffset], 
				sensor_msg->arrayLength*sizeof(HeartRateMessage));
			break;
		case typeRawTemperature:
			memcpy(sensor_msg->sensorMsgArray, &data[sensorMsgArrayOffset], 
				sensor_msg->arrayLength*sizeof(RawTemperatureMessage));
			break;
		case typeAccelerometer:
			memcpy(sensor_msg->sensorMsgArray, &data[sensorMsgArrayOffset], 
				sensor_msg->arrayLength*sizeof(AccelerometerMessage));
			break;
		case typeGPS:
			memcpy(sensor_msg->sensorMsgArray, &data[sensorMsgArrayOffset], 
				sensor_msg->arrayLength*sizeof(GPSMessage));
		break;
		default:
			printf("Cannot de-serialize messages with sensorType %u\n", sensor_msg->sensorType);
		}
	} else if (msg->mainType == msgSensorConfig) {
		/* calculate the address offset for the configMsgArray in the
		 * serialized data. See above for explanation */
		int configMsgArrayOffset = MESSAGE_PACKET_SIZE + sizeof(ConfigMessage) - sizeof(void *);
		
		/* copy the configMsgArray into the serialized data structure */
		memcpy(config_msg->configMsgArray, &data[configMsgArrayOffset], 
				config_msg->arrayLength*sizeof(ConfigSensor));
	} else if (msg->mainType == msgDebug) {
		/* calculate the address offset for the debugData in the
		 * serialized data. See above for explanation */
		int debugDataOffset = MESSAGE_PACKET_SIZE + sizeof(DebugMessage) - sizeof(void *);
		/* copy the debug string into the de-serialized data structure,
		 * for now the length of the copy operation is based the first
		 * occurrence of a terminating null byte */
		strcpy((char *)debug_msg->debugData, (const char *)&data[debugDataOffset]);
		}
}

/* Function will serialize data in the MessagePacket structure into continuous
 * memory area, that has to be preallocated and passed to the function 
 * msg[in]: Pointer to MessagePacket structure 
 * data[out]: Pointer to preallocated memory
 * returns: length of *data */ 
uint16_t MessageStorage::serialize(const MessagePacket *msg, uint8_t *data) {
	uint16_t size = 0;
	
	/* copy the header information of the MessagePacket structure */
	data[size++] = (uint8_t)msg->mainType;
	*(uint32_t *)&data[size] = msg->relTimestampS;
	size += sizeof(msg->relTimestampS);
 
	/* copy the header information of the main message groups
	 * and set the sensorMsgArray pointer to the next element in the mem-space */
	switch (msg->mainType) {
	case msgSensorData:
		memcpy(&data[size], msg->payload, sizeof(SensorMessage) - sizeof(void *)); 
		size += sizeof(SensorMessage) - sizeof(void *);
		break;
	case msgSensorConfig:
		memcpy(&data[size], msg->payload, sizeof(ConfigMessage) - sizeof(void *)); 
		size += sizeof(ConfigMessage) - sizeof(void *);
		break;
	case msgDebug:
		memcpy(&data[size], msg->payload, sizeof(DebugMessage) - sizeof(void *));
		size += sizeof(DebugMessage) - sizeof(void *);
		break;
	}
	/* copy the message type specific payload */
	if (msg->mainType == msgSensorData) {
		const SensorMessage *sensor_msg = (const SensorMessage *)msg->payload;
		
		printf("SensorMsg: arrayLength %u\n", sensor_msg->arrayLength);
		/* copy the sensorMsgArray into the serialized data structure */
		switch (sensor_msg->sensorType) {
		case typeHeartRate:
			memcpy(&data[size], sensor_msg->sensorMsgArray, 
				sensor_msg->arrayLength*sizeof(HeartRateMessage));
			size += sensor_msg->arrayLength*sizeof(HeartRateMessage);
			break;
		case typeRawTemperature:
			memcpy(&data[size], sensor_msg->sensorMsgArray, 
				sensor_msg->arrayLength*sizeof(RawTemperatureMessage));
			size += sensor_msg->arrayLength*sizeof(RawTemperatureMessage);
			break;
		case typeAccelerometer:
			memcpy(&data[size], sensor_msg->sensorMsgArray, 
				sensor_msg->arrayLength*sizeof(AccelerometerMessage));
			size += sensor_msg->arrayLength*sizeof(AccelerometerMessage);
			break;
		case typeGPS:
			memcpy(&data[size], sensor_msg->sensorMsgArray, 
				sensor_msg->arrayLength*sizeof(GPSMessage));
			size += sensor_msg->arrayLength*sizeof(GPSMessage);
		break;
		default:
			printf("Cannot serialize messages with sensorType %u\n", sensor_msg->sensorType);
		}
	} else if (msg->mainType == msgSensorConfig) {
		const ConfigMessage *config_msg = (const ConfigMessage *)msg->payload;
		
		/* copy the configMsgArray into the serialized data structure */
		memcpy(&data[size], config_msg->configMsgArray, 
				config_msg->arrayLength*sizeof(ConfigSensor));
		size += config_msg->arrayLength*sizeof(ConfigSensor);
	} else if (msg->mainType == msgDebug) {
		DebugMessage *debug_msg = (DebugMessage *)msg->payload;
		
		/* copy the debug string into the de-serialized data structure,
		 * for now the length of the copy operation is based the first
		 * occurrence of a terminating null byte */
		strcpy((char *)&data[size], (const char *)debug_msg->debugData);
		size += strlen((const char *)debug_msg->debugData);
	}
	return size;
}


void MessageStorage::addToStorageQueue(MessagePacket * in_msg, unsigned short size)
{
	// serialize and enqueue message
	char * serializedMessage = (char *) malloc(size);

	if(!serializedMessage)
	{
		module_debug_strg("failed to allocate serialization space!");
		return;
	}
	
	uint16_t serializedSize = serialize(in_msg, (uint8_t *) serializedMessage);
	enqueue(serializedMessage, NULL, size);
	m_queueCountMem++;
}

char * MessageStorage::getFromStorageQueueRaw(unsigned short * size)
{
	module_debug_strg("getFromStorageQueueRaw");
	// dequeue and deserialize message
	MessageEntry * headEntry = dequeue();
	
	if(!headEntry)
	{
		module_debug_strg("nothing to get from queue!");
		return NULL;
	}
	
	char * entryContent = (char *) headEntry->memPtr;	// NULL if file (handled below)
	
	// process serialized data depending on storage mode
	if(headEntry->fileName)
	{
		module_debug_strg("reading from file: %d", headEntry->fileName);
		// open file and read contained data
		char fnBuffer[13];
		entryContent = (char *) malloc(headEntry->size);
		sprintf(fnBuffer, "%d", headEntry->fileName);
		openFile(fnBuffer, false, true);
		readFromFile(entryContent, headEntry->size);
		closeFile();
	}
	
	// make room for the deserialized data
	char * out_msg = (char*) malloc(headEntry->size);
	
	*size = headEntry->size;
	
	memcpy(out_msg, entryContent, headEntry->size);
	
	// free the resources used by the entry
	freeMessageEntry(headEntry);
	
	return out_msg;	
}


// TODO derive this from func above!
MessagePacket *  MessageStorage::getFromStorageQueue()
{
	// dequeue and deserialize message
	MessageEntry * headEntry = dequeue();
	
	if(!headEntry)
	{
		module_debug_strg("nothing to get from queue!");
		return NULL;
	}
	
	char * entryContent = (char *) headEntry->memPtr;	// NULL if file (handled below)
	
	// process serialized data depending on storage mode
	if(headEntry->fileName)
	{
		// open file and read contained data
		char fnBuffer[13];
		entryContent = (char *) malloc(headEntry->size);
		sprintf(fnBuffer, "%d", headEntry->fileName);
		openFile(fnBuffer, false, true);
		readFromFile(entryContent, headEntry->size);
		closeFile();
	}
	
	// make room for the deserialized data
	MessagePacket * out_msg = (MessagePacket *) malloc(headEntry->size);
	
	deserialize((uint8_t const *)entryContent, out_msg);
	
	// free the resources used by the entry
	freeMessageEntry(headEntry);
	
	return out_msg;
}

// TODO also serialize message queue structure for long time ZigBee-less
// operation!

void MessageStorage::enqueue(void * memPtr, unsigned int fileName, unsigned short size)
{
	// either fileName or memPtr is allowed - an entry cannot have both
	if((memPtr != NULL && fileName != 0) || size == 0)
	{
		module_debug_strg("invalid enqueue!");
		return;
	}
	
	module_debug_strg("enqueue");
	// allocate space from heap for the new entry
	MessageEntry * newEntry = (MessageEntry *) malloc(sizeof(MessageEntry));

	
	if(!newEntry)
	{
		module_debug_strg("failed to allocate new entry!");
		return;
	}
	
	newEntry->size = size;
	newEntry->fileName = fileName;
	newEntry->memPtr = memPtr;
	newEntry->nextEntry = NULL;	// always at the end of the queue
	
	if(m_queueHead == NULL || m_queueTail == NULL)
	{
		// empty queue, this will be the first member
		m_queueHead = m_queueTail = newEntry;
	} else {
		// enqueue at the tail of the queue
		m_queueTail->nextEntry = newEntry;
		m_queueTail = newEntry;
	}
	
	// now we have one more entry in the queue
	m_queueCount++;
}

void MessageStorage::flushEntryToDisk(MessageEntry * entry)
{
	// check if this is a valid mem entry
	if(!entry)
		return;
	
	if(entry->memPtr == NULL || entry->fileName != NULL)
	{
		module_debug_strg("cannot flush non-mem entry!");
		return;
	}
	
	module_debug_strg("flushing...");
	
	// move mem entry to disk and free the occupied mem
	// file name will be the current Unix time
	entry->fileName = m_nextMessageSeqNumber++;
	
	char fnBuffer[13];
	sprintf(fnBuffer, "%d", entry->fileName);
	
	// TODO handle msg seq nr overflow
	
	module_debug_strg("flush to disk: %s", fnBuffer);
	
	// create new file and write data
	openFile(fnBuffer, true, false);
	writeToFile((char*) entry->memPtr, entry->size);
	closeFile();
	
	// free occupied entry memory and set memPtr to NULL
	free(entry->memPtr);
	
		
	entry->memPtr = NULL;
	
	m_queueCountMem--;
	
}

void MessageStorage::flushAllToDisk()
{
	module_debug_strg("start flushAllToDisk! qc = %d qcm %d", m_queueCount,
					  m_queueCountMem);
	// TODO traversing through all entries is nonoptimal, keep separate track?
	MessageEntry * entry = m_queueHead;
	while(m_queueCountMem && (entry != NULL))
	{
		if(entry->memPtr)
			flushEntryToDisk(entry);
		
		entry = entry->nextEntry;
	}
	module_debug_strg("end flushAllToDisk! qc = %d qcm %d", m_queueCount,
					  m_queueCountMem);
}

MessageStorage::MessageEntry * MessageStorage::dequeue()
{
	if(m_queueHead == NULL || m_queueTail == NULL)
	{
		module_debug_strg("nothing to dequeue!");
		return NULL;
	}
	
	// dequeue head of the queue
	MessageEntry * ret = m_queueHead;
	
	// advance head of queue to next element
	m_queueHead = ret->nextEntry;
	
	m_queueCount--;
	
	// detect empty queue
	if(m_queueHead == NULL)
		m_queueTail = NULL;
	
	return ret;
}

void MessageStorage::freeMessageEntry(MessageEntry * entry)
{
	// check entry validity
	if((entry->memPtr == NULL && entry->fileName == NULL) ||
	   (entry->memPtr != NULL && entry->fileName != NULL))
	{
		module_debug_strg("freeMessageEntry: invalid entry!");
		return;
	}
	
	// first free the filename / memory buffers for the entry
	if(entry->memPtr != NULL)
	{
		free(entry->memPtr);
		entry->memPtr = NULL;
	}
	
	// finally, deallocate the MessageEntry space itself
	free(entry);
}

unsigned int MessageStorage::getStorageQueueCount()
{
	return m_queueCount;
}
  
unsigned int MessageStorage::readRTCStorage()
{
	unsigned int rtcVal = 0;
	
	if(!m_storageOK)
		return 0;
	
	openFile("rtc", false, true);
	readFromFile((char *) &rtcVal, sizeof(unsigned int));
	closeFile();
	
	return rtcVal;
}

void MessageStorage::writeRTCStorage(unsigned int rtcValue)
{
	if(!m_storageOK)
		return;
	
	openFile("rtc", true, false);
	seekToPos(0);
	writeToFile((char *) &rtcValue, sizeof(unsigned int));
	closeFile();
}

void MessageStorage::startAudioSample()
{
	deleteFile("audio");
	openFile("audio", true, false);
}

void MessageStorage::flushAudioSample(char * buf, uint16_t size)
{
	writeToFile(buf, size);
}

void MessageStorage::endAudioSample()
{
	closeFile();
}				   

	
// internal filesystem access layer functions --------------------------------
#ifdef EHM_MONITORING_DEVICE
void MessageStorage::openFile(const char * fileName, bool writeAccess, bool readAccess)
{
	BYTE access = 0;
	
	if(m_fileOpen)
	{
		module_debug_strg("file already open, close it first!");
		return;
	}
	
	if(writeAccess)
		access |= FA_WRITE | FA_CREATE_ALWAYS;
	
	if(readAccess)
		access |= FA_READ;
	
	m_fr = f_open(&m_file, fileName,  access); 
	
	if(m_fr == FR_OK)
	{
		module_debug_strg("file opened!");
		m_fileOpen = true;
	}
	else 
	{
		module_debug_strg("could not open file! %x", m_fr);
		m_fileOpen = false;
	}
		
}

void MessageStorage::seekToPos(unsigned int pos)
{
	if(!m_fileOpen)
	{
		module_debug_strg("file not open!");
		return;
	}
	
	f_lseek(&m_file, pos);
}

void MessageStorage::closeFile()
{
	if(!m_fileOpen)
	{
		module_debug_strg("file not open!");
		return;
	}
	
	m_fr = f_close(&m_file);
	m_fileOpen = false;
}

void MessageStorage::writeToFile(char * buffer, unsigned int count)
{
	if(!m_fileOpen)
	{
		module_debug_strg("file not open!");
		return;
	}
	
	UINT bw;
	m_fr = f_write(&m_file, (void *) buffer, count, &bw);
	if(m_fr != FR_OK || bw != count)
		module_debug_strg("error while writing: %x wrote %d of %d", m_fr, bw, count);
}

void MessageStorage::readFromFile(char * buffer,  unsigned int count)
{
	if(!m_fileOpen)
	{
		module_debug_strg("file not open!");
		return;
	}
	
	UINT br;
	m_fr = f_read(&m_file, (void *) buffer, count, &br);
	if(m_fr != FR_OK || br != count)
		module_debug_strg("error while reading: %x read %d of %d", m_fr, br, count);	
}

void MessageStorage::deleteFile(const char * fileName)
{
	m_fr = f_unlink(fileName);
}

unsigned int MessageStorage::getDirFileCount(char *path)
{
	unsigned int count = 0;
	FILINFO     fno;
	DIR         dir;
	int         i;
	char        *fn;
	
	// TODO copy files into queue as we iterate
	// TODO update latest sequence number

	m_fr = f_opendir(&dir, path);
	
	if (m_fr == FR_OK)
	{
		i = strlen(path);
		while(1)
		{
			m_fr = f_readdir(&dir, &fno);
			if (m_fr != FR_OK)
			{
				module_debug_strg("f_readdir failure %d", m_fr);
				break;
			}
			if(fno.fname[0] == 0) break;
		  
		  if (fno.fname[0] == '.') continue;	// skip hidden files
		  
		  fn = fno.fname;

		  if (!(fno.fattrib & AM_DIR)) // ignore directories
		  {
			if(strlen(path))
				module_debug_strg("%s/%s", path, fn);
			else
				module_debug_strg("%s", fn);
			f_unlink(fn);	// TODO remove this after we stabilize!
			count++;
		  }
		}
	}
	else
	{
		module_debug_strg("f_opendir failure %d", m_fr);
	}
	
	module_debug_strg("found %d files", count);
	
	return count;
}

bool MessageStorage::mountStorage()
{
	// only needed for the DVK
	// Enable SPI access to MicroSD card
#ifdef __DVK_H
	DVK_peripheralAccess(DVK_MICROSD, true);
#endif
	m_storageOK = FATFS_initializeFilesystem();
	return m_storageOK;
}

void MessageStorage::unmountStorage()
{
	FATFS_deinitializeFilesystem();
	m_storageOK = false;
}

unsigned int MessageStorage::getTimestamp()
{
	return AlarmManager::getInstance()->getUnixTime();
}
#endif
// end of internal filesystem access layer functions ------------------------

// start of empty implementations for the internal filesystem functions, to be able
// to use the static functins of the class in other projects
#ifdef EHM_BASE_STATION
void MessageStorage::openFile(const char * fileName, bool writeAccess, bool readAccess){}
void MessageStorage::seekToPos(unsigned int pos){}
void MessageStorage::closeFile(){}
void MessageStorage::deleteFile(const char * fileName){}
void MessageStorage::writeToFile(char * buffer, unsigned int count){}
void MessageStorage::readFromFile(char * buffer, unsigned int count){}
unsigned int MessageStorage::getDirFileCount(char *dirName){ return -1; }
bool MessageStorage::mountStorage(){ return false; }
void MessageStorage::unmountStorage(){}
unsigned int MessageStorage::getTimestamp(){ return -1; }
#endif
// end of internal filesystem access layer functions ------------------------
