#ifndef __MESSAGESTORAGE_H
#define __MESSAGESTORAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "messagetypes.h"

// choose FS layer implementations according to definition
#ifndef	EHM_BASE_STATION
#define EHM_MONITORING_DEVICE
#include "fatfs.h"
#include "ff.h"
#else
// TODO insert BS-specific includes here
#endif

class MessageStorage  {
  public:
	static MessageStorage* getInstance()
	{
		static MessageStorage instance;
		return &instance;
	}
	
	void initialize(char * storageRoot);
	void addToStorageQueue(MessagePacket * in_msg);
	void getFromStorageQueue(MessagePacket * out_msg, bool removeFromQueue = true);
	unsigned int getStorageQueueCount();
	unsigned int readRTCStorage();
	void writeRTCStorage(unsigned int rtcValue);

private:
  // ------ start of singleton pattern specific section ------
  MessageStorage();
  MessageStorage(MessageStorage const&);         // do not implement
  void operator=(MessageStorage const&);        // do not implement
  // ------ end of singleton pattern specific section --------
  char * m_storageRoot;
  bool m_fileOpen;
  unsigned int m_queueCount;
  
  // internal filesystem access layer
  void openFile(char * fileName, bool writeAccess, bool readAccess);
  void seekToPos(unsigned int pos);
  void closeFile();
  void writeToFile(char * buffer, unsigned int count);
  void readFromFile(char * buffer, unsigned int count);
  unsigned int getDirFileCount(char *dirName);
  void mountStorage();
  void unmountStorage();
  
  // device-specific internal FS variables
#ifdef EHM_MONITORING_DEVICE
  FIL m_file;
  FRESULT m_fr;
#endif

};


#endif // __MESSAGESTORAGE_H