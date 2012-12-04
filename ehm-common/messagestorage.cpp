#include "messagestorage.h"
#include <string.h>

#ifdef EHM_MONITORING_DEVICE
#include "debug_output_control.h"
#else
#define module_debug_strg(fmt, ...)
#endif

MessageStorage::MessageStorage()
{
	m_storageRoot = NULL;
	m_fileOpen = false;
	m_queueCount = 0;
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
	mountStorage();
	
	FATFS_speedTest(256);
	FATFS_speedTest(512);
	FATFS_speedTest(1024);
	
	// count list of existing files in storage root
	m_queueCount = getDirFileCount("");
}

void MessageStorage::addToStorageQueue(MessagePacket * in_msg)
{
}

void MessageStorage::getFromStorageQueue(MessagePacket * out_msg, bool removeFromQueue)
{
}

unsigned int MessageStorage::getStorageQueueCount()
{
	return m_queueCount;
}
  
unsigned int MessageStorage::readRTCStorage()
{
	unsigned int rtcVal = 0;
	openFile("rtc", false, true);
	readFromFile((char *) &rtcVal, sizeof(unsigned int));
	closeFile();
	
	return rtcVal;
}

void MessageStorage::writeRTCStorage(unsigned int rtcValue)
{
	openFile("rtc", true, false);
	seekToPos(0);
	writeToFile((char *) &rtcValue, sizeof(unsigned int));
	closeFile();
}

	
// internal filesystem access layer functions --------------------------------
#ifdef EHM_MONITORING_DEVICE
void MessageStorage::openFile(char * fileName, bool writeAccess, bool readAccess)
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

unsigned int MessageStorage::getDirFileCount(char *path)
{
	unsigned int count = 0;
	FILINFO     fno;
	DIR         dir;
	int         i;
	char        *fn;

	m_fr = f_opendir(&dir, path);
	
	if (m_fr == FR_OK)
	{
		i = strlen(path);
		while(1)
		{
			m_fr = f_readdir(&dir, &fno);
			if (m_fr != FR_OK || fno.fname[0] == 0)
			{
				module_debug_strg("f_readdir failure %d", m_fr);
				break;
			}
		  
		  if (fno.fname[0] == '.') continue;	// skip hidden files
		  
		  fn = fno.fname;

		  if (!(fno.fattrib & AM_DIR)) // ignore directories
		  {
			if(strlen(path))
				module_debug_strg("%s/%s", path, fn);
			else
				module_debug_strg("%s", fn);
			count++;
		  }
		}
	}
	else
	{
		module_debug_strg("f_opendir failure %d", m_fr);
	}
	
	return count;
}

void MessageStorage::mountStorage()
{
	// only needed for the DVK
	// Enable SPI access to MicroSD card
#ifdef __DVK_H
	DVK_peripheralAccess(DVK_MICROSD, true);
#endif
	FATFS_initializeFilesystem();
}

void MessageStorage::unmountStorage()
{
	FATFS_deinitializeFilesystem();
}
#endif
// end of internal filesystem access layer functions ------------------------