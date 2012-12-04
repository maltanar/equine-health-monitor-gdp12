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
	void addToStorageQueue(MessagePacket * in_msg, unsigned short size);
	void getFromStorageQueue(MessagePacket * out_msg);
	unsigned int getStorageQueueCount();
	
	// RTC storage functions
	unsigned int readRTCStorage();
	void writeRTCStorage(unsigned int rtcValue);
	
	// independent de-/serialization functions for MessagePacket structures. 
	static uint16_t serialize(const MessagePacket *msg, uint8_t *data);
	static void deserialize(const uint8_t *data, MessagePacket *msg);

private:
  // ------ start of singleton pattern specific section ------
  MessageStorage();
  MessageStorage(MessageStorage const&);         // do not implement
  void operator=(MessageStorage const&);        // do not implement
  // ------ end of singleton pattern specific section --------
  
  char * m_storageRoot;
  bool m_fileOpen;
  bool m_storageOK;
  
  typedef struct MessageEntry {
	  void * memPtr;	// NULL if entry has already been saved to disk
	  char * fileName; 	// NULL if entry is yet in memory
	  unsigned short size;	// size of message data, either on disk or in mem
	  MessageEntry * nextEntry;	// linked list structure, NULL if last member
  } MessageEntry;
  
  MessageEntry * m_queueHead, * m_queueTail;
  unsigned int m_queueCount;
  
  // internal queue management functions
  void enqueue(void * memPtr, char * fileName, unsigned short size);
  MessageEntry * dequeue();
  void freeMessageEntry(MessageEntry * entry);
  
  
  // internal filesystem access layer
  void openFile(const char * fileName, bool writeAccess, bool readAccess);
  void seekToPos(unsigned int pos);
  void closeFile();
  void deleteFile(const char * fileName);
  void writeToFile(char * buffer, unsigned int count);
  void readFromFile(char * buffer, unsigned int count);
  unsigned int getDirFileCount(char *dirName);
  bool mountStorage();
  void unmountStorage();
  
  // device-specific internal FS variables
#ifdef EHM_MONITORING_DEVICE
  FIL m_file;
  FRESULT m_fr;
#endif

};


#endif // __MESSAGESTORAGE_H