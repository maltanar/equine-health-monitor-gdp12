// University of Southampton, 2012
// EMECS Group Design Project

#ifndef __ANTHRMSENSOR_H
#define __ANTHRMSENSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "leuartport.h"
#include "ANT/antplus.h"
#include "ANT/hrm_rx.h"
#include "sensor.h"

#define ANTHRM_DEFAULT_RATE     4000

static const uint8_t aucNetworkKey[] = ANTPLUS_NETWORK_KEY;

// stuff we borrow from the ANT HRM RX example -----------------------------
#define ANT_SERIAL_QUEUE_BUFFER_LENGTH     ((uint8_t) 15)                                  // 15 assumes that an exteneded message is the longest that will be recieved.                                               
#define ANT_SERIAL_QUEUE_RX_SIZE           ((uint8_t) 6)                                   // Same for TX and RX buffers
#define ANT_TXBUFSIZE						15

typedef struct
{
   uint8_t aucBuffer[ANT_SERIAL_QUEUE_BUFFER_LENGTH]; 
}  ANTSerialBuffer;

typedef struct
{
   ANTSerialBuffer* astBuffer;
   uint8_t ucHead;
   uint8_t ucTail;
   uint8_t ucCurrentByte;
   uint8_t ucCheckSum;
}  ANTSerialQueue;

typedef enum
{
   UART_STATE_WFSYNC,         // Wait for sync
   UART_STATE_WFSIZE,         // Wait for size
   UART_STATE_BODY,           // Reading in the body of the message
   UART_STATE_WFCHKSUM        // Wait for checksum
} ANTUartRxState;

typedef enum
{
   STATE_INIT_PAGE = 0,                                  // Initializing state
   STATE_STD_PAGE = 1,                                   // No extended messages to process
   STATE_EXT_PAGE = 2                                    // Extended messages to process
} StatePage;

// ANT Channel
#define HRMRX_CHANNEL_TYPE            ((uint8_t) 0x00)     // Slave

// Channel ID
#define HRMRX_DEVICE_TYPE             ((uint8_t) 0x78)	

// Message Periods
//#define HRMRX_MSG_PERIOD              ((uint16_t) 32280)	// hex 0x1F86: decimal 8070 (4.06Hz) or 32280 for approx 1 Hz
#define HRMRX_MSG_PERIOD              ((uint16_t) 8070)	// hex 0x1F86: decimal 8070 (4.06Hz) or 32280 for approx 1 Hz

// HRM Page Toggle Bit Mask
#define TOGGLE_MASK                   ((uint8_t) 0x80)

// Default ANT Channel
#define ANT_CHANNEL_HRMRX                          ((uint8_t) 0)        

#define HRM_PRECISION                              ((uint32_t) 1000)


// end of stuff we borrow from the ANT HRM RX example ----------------------

class ANTHRMSensor : public Sensor {
public:
	static void setPower(bool vccOn);
	static void configurePower();
	
	// singleton instance accessor
	static ANTHRMSensor* getInstance()
	{
		static ANTHRMSensor instance;
		return &instance;
	}

	// ANTRxHook needs access to private functions
	friend bool ANTRxHook(uint8_t c);

	// virtual functions which can be overridden if the sensor supports
	// the functionality
	char setSleepState(bool sleepState);
	void setPeriod(SensorPeriod ms);

	// pure virtual functions representing common functionality, which need to be 
	// overridden for all sensors
	void sampleSensorData();

	// ANT HRM module control commands
	// TODO device ID
	// TODO connection status
	// TODO clear device pairing / trigger search
	bool initializeNetwork(bool hardReset = false);
	void hardReset();
	bool isConnected();
	uint16_t getPairedDeviceID();
	void setPairedDeviceID(uint16_t id);
	void closeChannel();
	void openChannel();
  
private:
	// ------ start of singleton pattern specific section ------
	ANTHRMSensor();  
	ANTHRMSensor(ANTHRMSensor const&);              // do not implement
	void operator=(ANTHRMSensor const&);            // do not implement
	// ------ end of singleton pattern specific section --------

	ANTUartRxState m_rxState;						// stage of ANT message reception
	uint8_t m_ucCheckSum;
	uint8_t m_ucTheLength;
	uint8_t m_ucIndex;
	uint8_t* m_pucTheBuffer;
	LEUARTPort * m_port;                            // the UART IF for the ANT HRM
	HeartRateMessage m_hrmMessage;
	bool m_isConnected;
	
	// rx-tx queues and buffers
	ANTSerialBuffer m_stRxBuffer[ANT_SERIAL_QUEUE_RX_SIZE];
	ANTSerialQueue m_stTheRxQueue;
	
	// device channel, transmission type and number
	uint16_t m_usDeviceNumber;
	uint8_t m_ucTransType;
	uint8_t m_ucAntChannel;
	
	// page data
	StatePage m_eThePageState;
	HRMPage0_Data m_page0Data;

	// process one char received from UART
	void processUARTRxChar(uint8_t c);
	// send an ANT message
	void sendANTMessage(uint8_t * data, uint8_t length);
	// parse a correctly formatted ANT message and get some fields
	bool parseANTMessage(uint8_t * in_data, uint8_t *out_channel, uint8_t *out_msgcode);
	
	// internal event management
	bool channelEvent(uint8_t * pucEventBuffer_, ANTPLUS_EVENT_RETURN* pstEventStruct_);
	bool handleResponseEvents(uint8_t * pucBuffer_);
	void handleDataMessages(uint8_t* pucBuffer_, ANTPLUS_EVENT_RETURN* pstEventStruct_);
	void decodeDefault(uint8_t* pucPayload_);
	void handleSearchTimeout();
	//void processANTHRMRXEvents(ANTPLUS_EVENT_RETURN* pstEvent_);
	
	
	// ANT base interface level messages
	bool ANT_Reset();
	bool ANT_UnAssignChannel(uint8_t ucChannelNumber);
	bool ANT_AssignChannel(uint8_t ucChannelNumber_, uint8_t ucChannelType_, uint8_t ucNetworkNumber_);
	bool ANT_SearchTimeout(uint8_t ucChannelNumber_, uint8_t ucTimeout_);
	bool ANT_NetworkKey(uint8_t ucNetworkNumber_, const uint8_t* pucKey_);
	bool ANT_RequestMessage(uint8_t ucANTChannel_, uint8_t ucRequestedMessage_, bool blocking = false);
	bool ANT_ChannelId(uint8_t ucANTChannel_, uint16_t usDeviceNumber_, uint8_t ucDeviceType_, uint8_t ucTransmitType_);
	bool ANT_ChannelPower(uint8_t ucANTChannel_,  uint8_t ucPower_);  
	bool ANT_ChannelRFFreq(uint8_t ucANTChannel_, uint8_t ucFreq_);
	bool ANT_ChannelPeriod(uint8_t ucANTChannel_, uint16_t usPeriod_);
	bool ANT_Broadcast(uint8_t ucANTChannel_, uint8_t* pucBuffer_);
	bool ANT_AcknowledgedTimeout(uint8_t ucChannel_, uint8_t* pucData_, uint16_t usTimeout_);
	bool ANT_Acknowledged(uint8_t ucANTChannel_,  uint8_t* pucBuffer_);
	bool ANT_BurstPacket(uint8_t ucControl_, uint8_t* pucBuffer_);
	bool ANT_OpenChannel(uint8_t ucANTChannel_);
	bool ANT_CloseChannel(uint8_t ucANTChannel_);
	
	// wait for given message
	bool waitForResponse(uint8_t channel, uint8_t responseID, uint8_t messageID);
	
	// RX buffer queue management
	uint8_t * getRxBuffer();
	void putRxBuffer();
	uint8_t * readRxBuffer();
	void releaseRxBuffer();
	uint8_t * readRxTop();
	void releaseRxTop();
	void flushRx();
	
	// functions to access UART layer
	void writeByte(uint8_t data);
	void writeBuffer(uint8_t * buffer, uint8_t length, uint8_t *checksum);
	
};

#endif  // __ANTHRMSENSOR_H