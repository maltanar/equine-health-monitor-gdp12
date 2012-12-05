#include "em_emu.h"
#include "em_gpio.h"
#include "usartmanager.h"
#include "alarmmanager.h"
#include "anthrmsensor.h"
#include "ANT/antmessage.h"
#include "ANT/antdefines.h"
#include "debug_output_control.h"

// either GPIO_ANT_RST or GPIO_ANT_VCC must be defined to reset the ANT
#ifdef GPIO_ANT_RST
#define ANT_RESET	GPIO_ANT_RST
#else
#define ANT_RESET	GPIO_ANT_VCC
#endif

bool ANTRxHook(uint8_t c)
{
	ANTHRMSensor::getInstance()->processUARTRxChar(c);
	return true; // tell the UART driver we already processed the data
}

ANTHRMSensor::ANTHRMSensor() :
  Sensor(typeHeartRate, sizeof(HeartRateMessage), ANTHRM_DEFAULT_RATE)
{
	// initialize message structures
	m_sensorMessage.arrayLength = 1;
	m_sensorMessage.sampleIntervalMs = ANTHRM_DEFAULT_RATE;
	m_sensorMessage.sensorMsgArray = (uint8_t *) &m_hrmMessage;
	m_hrmMessage.bpm = 0;

	//  Reset RX queue
	m_stTheRxQueue.ucHead = 0;                     
	m_stTheRxQueue.ucTail = 0;
	m_stTheRxQueue.ucCurrentByte = 0;
	m_stTheRxQueue.ucCheckSum = 0;
	m_stTheRxQueue.astBuffer = m_stRxBuffer;
	
	// initialize internal state variables
	m_rxState = UART_STATE_WFSYNC;
	m_ucCheckSum = 0;
	m_ucTheLength = 0;
	m_ucIndex = 0;
	m_pucTheBuffer = (uint8_t *) NULL;
	
	m_ucAntChannel = ANT_CHANNEL_HRMRX;
	m_usDeviceNumber = 0;
	m_ucTransType = 0;
	m_isConnected = false;
	
	// initialize page data
	m_page0Data.usBeatTime = 0;                                  
	m_page0Data.ucBeatCount = 0;                                 
	m_page0Data.ucComputedHeartRate = 0; 

	m_eThePageState = STATE_INIT_PAGE; 
	
	
    // initialize UART
    // TODO UARTManagerPortLEUART0 should be defined in part-specific config!
    m_port = (LEUARTPort *) USARTManager::getInstance()->getPort(ANT_USART_PORT);

    m_port->initialize((uint8_t *) NULL, 0, 
                     LEUARTPort::leuartPortBaudRate4800);
	
	// set the rx hook we use to trap incoming characters
	m_port->setRxHook(&ANTRxHook);
	
	// initialize other GPIO pins used to control the ANT
	GPIO_PinModeSet(GPIO_ANT_RTS, gpioModeInput, 0);
	GPIO_PinModeSet(ANT_RESET, gpioModePushPull, 1);
	GPIO_PinModeSet(GPIO_ANT_SLEEP, gpioModePushPull, 0);

	
	
	// TODO configure sleep pin here
    
    setPeriod(ANTHRM_DEFAULT_RATE);
}

void ANTHRMSensor::hardReset()
{
	module_debug_ant("hard reset!");
	// we implement hard reset for the ANT module in two ways:
	// - if the GPIO RST pin is defined, pull that low for some time
	// - if not (f.ex for the PCB all reset lines are connected together)
	//   use the power pin instead
#ifndef GPIO_ANT_VCC
	GPIO_PinOutClear(ANT_RESET);
	AlarmManager::getInstance()->lowPowerDelay(10, sleepModeEM1);
	GPIO_PinOutSet(ANT_RESET);
#else
	GPIO_PinOutSet(ANT_RESET);
	AlarmManager::getInstance()->lowPowerDelay(999, sleepModeEM1);
	AlarmManager::getInstance()->lowPowerDelay(999, sleepModeEM1);
	AlarmManager::getInstance()->lowPowerDelay(999, sleepModeEM1);
	AlarmManager::getInstance()->lowPowerDelay(999, sleepModeEM1);
	AlarmManager::getInstance()->lowPowerDelay(999, sleepModeEM1);
	GPIO_PinOutClear(ANT_RESET);
#endif
}

char ANTHRMSensor::setSleepState(bool sleepState)
{
    // TODO implement
    return 0;
}

void ANTHRMSensor::setPeriod(SensorPeriod ms)
{
    // TODO implement
    
    // also call parent implementation
    Sensor::setPeriod(ms);
}

void ANTHRMSensor::sampleSensorData()
{
	uint8_t *pucRxBuffer = readRxBuffer();        // Check if any data has been recieved from serial
    ANTPLUS_EVENT_RETURN stEventStruct;
    
    if(pucRxBuffer)
    {
       /*if(HRMRX_ChannelEvent(pucRxBuffer, &stEventStruct))
          usLowPowerMode = 0;*/
       channelEvent(pucRxBuffer, &stEventStruct);
       //processANTHRMRXEvents(&stEventStruct);

       releaseRxBuffer();
    } 
}

uint8_t * ANTHRMSensor::getRxBuffer()
{
   if(((m_stTheRxQueue.ucHead + 1) & (ANT_SERIAL_QUEUE_RX_SIZE-1)) != m_stTheRxQueue.ucTail)
      return(m_stTheRxQueue.astBuffer[m_stTheRxQueue.ucHead].aucBuffer);
   
   return (uint8_t *) NULL;
}

void ANTHRMSensor::putRxBuffer()
{
	m_stTheRxQueue.ucHead = ((m_stTheRxQueue.ucHead + 1) & (ANT_SERIAL_QUEUE_RX_SIZE - 1));
}

uint8_t * ANTHRMSensor::readRxBuffer()
{
   if(m_stTheRxQueue.ucHead != m_stTheRxQueue.ucTail)
      return(m_stTheRxQueue.astBuffer[m_stTheRxQueue.ucTail].aucBuffer);
   
	return (uint8_t *) NULL;  
}

void ANTHRMSensor::releaseRxBuffer()
{
   m_stTheRxQueue.ucTail = ((m_stTheRxQueue.ucTail + 1) & (ANT_SERIAL_QUEUE_RX_SIZE - 1));
} 

uint8_t * ANTHRMSensor::readRxTop()
{
   if(m_stTheRxQueue.ucHead != m_stTheRxQueue.ucTail)
      return(m_stTheRxQueue.astBuffer[(m_stTheRxQueue.ucHead - 1) & (ANT_SERIAL_QUEUE_RX_SIZE - 1)].aucBuffer);
   
	return (uint8_t *) NULL ;
}

void ANTHRMSensor::releaseRxTop()
{
   if(m_stTheRxQueue.ucHead != m_stTheRxQueue.ucTail)
      m_stTheRxQueue.ucHead = (m_stTheRxQueue.ucHead- 1) & (ANT_SERIAL_QUEUE_RX_SIZE-1);
} 

void ANTHRMSensor::flushRx()
{
   m_stTheRxQueue.ucHead = 0;
   m_stTheRxQueue.ucTail = 0;
   m_stTheRxQueue.ucCurrentByte = 0;
   m_stTheRxQueue.ucCheckSum = 0;
}

void ANTHRMSensor::processUARTRxChar(uint8_t c)
{
	switch(m_rxState)
	{
	case UART_STATE_WFSYNC:                         // waiting for SYNC
		if(c == ANT_MESG_TX_SYNC)                  // Got SYNC
		{
			m_rxState = UART_STATE_WFSIZE;            // Move to waiting for size
			m_ucCheckSum = ANT_MESG_TX_SYNC; 
			m_ucIndex = 0;         
		}
		break;
	
	case UART_STATE_WFSIZE:                         // waiting for size
		if(c < ANT_SERIAL_QUEUE_BUFFER_LENGTH)     // Make sure size makes sense
		{
			m_pucTheBuffer = getRxBuffer();    // Try to get a buffer from the RX queue
			if(m_pucTheBuffer)
			{
				m_pucTheBuffer[m_ucIndex++] = c;
				m_ucCheckSum ^= c; 
				m_ucTheLength = c;                 // Move to waiting for body
				m_rxState = UART_STATE_BODY;
			}
			else
			{
				m_rxState = UART_STATE_WFSYNC;         // If no buffer in queue, drop this message and reset
			}
		}
		else
		{
			m_rxState = UART_STATE_WFSYNC;            // If the size is weird drop message and reset
		}
		break;
     
	case UART_STATE_BODY:                           // Waiting for body bytes
		if(m_pucTheBuffer)
		{
			m_pucTheBuffer[m_ucIndex++] = c;
			m_ucCheckSum ^= c; 

			if(m_ucTheLength == 0)                      // Body will be collected up to checksum
			{
				m_rxState = UART_STATE_WFCHKSUM;       // Move to waiting for checksum
			}
			else
			{         
				m_ucTheLength--;
			}      
		}

		break;
	
	case UART_STATE_WFCHKSUM:                       // Waiting for checksum byte
		if( c == m_ucCheckSum)                   // If the checksum is good
		{
			putRxBuffer();                   // put buffer back in queue
			module_debug_ant("checksum %x OK!", c);
		} 
		else
			module_debug_ant("checksum %x not OK! expected %x", c, m_ucCheckSum);

		// Reset the state machine
		m_ucCheckSum = 0;
		m_rxState = UART_STATE_WFSYNC;
		m_ucTheLength = 0;
		m_ucIndex = 0;
		m_pucTheBuffer = (uint8_t *) NULL;

		break;
		
	default:                                        // Shouldn't happen
		m_rxState = UART_STATE_WFSYNC;
		break;
	}
}

void ANTHRMSensor::sendANTMessage(uint8_t *data, uint8_t length)
{
	uint8_t checksum = ANT_MESG_TX_SYNC;
	
	// send the sync byte / preamble
	writeByte(ANT_MESG_TX_SYNC); 
	// send the message body
	writeBuffer(data, length, &checksum);
    // send the calculated checksum
	writeByte(checksum);
}

void ANTHRMSensor::writeByte(uint8_t data)
{
	m_port->writeChar(data);
}

void ANTHRMSensor::writeBuffer(uint8_t * buffer, uint8_t length, uint8_t *checksum)
{
	for(int i = 0; i < length; i++)
	{
		writeByte(buffer[i]);
		*checksum ^= buffer[i];
	}
}

bool ANTHRMSensor::parseANTMessage(uint8_t *in_data, uint8_t *out_channel, 
									 uint8_t *out_msgcode)
{
	if(!in_data || !out_channel || !out_msgcode)
		return false;
	
	*out_channel = in_data[2];
	*out_msgcode = in_data[BUFFER_INDEX_MESG_ID];
	return true;
}

bool ANTHRMSensor::waitForResponse(uint8_t channel, uint8_t responseID, 
									   uint8_t messageID)
{
	uint8_t rcv_channel, rcv_msgcode;
	uint8_t * rcv_msg;
	
	// TODO "loop of death" until expected msg arives is a bad idea - should have
	// an explicitly settable timeout mode
	// TODO channel information is ignored, this code will not work for
	// multiple channel connections
	while(1)
	{
		rcv_msg = readRxBuffer();
		
		if(parseANTMessage(rcv_msg, &rcv_channel, &rcv_msgcode))
		{
			module_debug_ant("waitForResponse %d %d / got %d %d ", responseID, messageID, rcv_msgcode, rcv_msg[3]);
			 if((rcv_msgcode == ANT_MESG_RESPONSE_EVENT_ID && rcv_msg[3] == messageID) 
				|| rcv_msgcode == responseID)
				 break;
			 // TODO this approach also cannot deal with messages arriving out of order
			 releaseRxBuffer();
		}
	}
	
	return true;
}

uint16_t ANTHRMSensor::getPairedDeviceID()
{
	// return paired device it - will be 0 if not paired
	return m_usDeviceNumber;
}

bool ANTHRMSensor::isConnected()
{
	return m_isConnected;
}

void ANTHRMSensor::decodeDefault(uint8_t* pucPayload_)
{
   HRMPage0_Data* pstPage0Data = &m_page0Data;

   pstPage0Data->usBeatTime = (uint16_t)pucPayload_[0];                  // Measurement time
   pstPage0Data->usBeatTime |= (uint16_t)pucPayload_[1] << 8;
   pstPage0Data->ucBeatCount = (uint8_t)pucPayload_[2];                  // Measurement count
   pstPage0Data->ucComputedHeartRate = (uint16_t)pucPayload_[3];         // Computed heart rate
   
   // TODO maybe include other message fields to send to base station
   m_hrmMessage.bpm = pstPage0Data->ucComputedHeartRate;
}

// change the paired device ID, set to 0 for wilcard match (any device)
// need to call initializeNetwork afterwards for changes to take effect 
void ANTHRMSensor::setPairedDeviceID(uint16_t id)
{
	m_usDeviceNumber = id;
}

void ANTHRMSensor::closeChannel()
{
	// open channel
	if(!ANT_CloseChannel(ANT_CHANNEL_HRMRX)) {
		module_debug_ant("closeChannel failed");
	}
	else
		module_debug_ant("closeChannel OK!");
}

void ANTHRMSensor::openChannel()
{
	// open channel
	if(!ANT_OpenChannel(ANT_CHANNEL_HRMRX)) {
		module_debug_ant("OpenChannel failed");
	}
	else
		module_debug_ant("OpenChannel OK!");
}


bool ANTHRMSensor::initializeNetwork(bool doHardReset)
{
	bool resetOK;
	
	if(!doHardReset)
		resetOK = ANT_Reset();
	else 
	{
		hardReset();
		resetOK = waitForResponse(0, ANT_MESG_STARTUP_ID, ANT_MESG_SYSTEM_RESET_ID);
	}
  
	// reset the module with software command
	if(!resetOK) {
		module_debug_ant("initializeNetwork: reset failed");
		return false;
	}
	else
		module_debug_ant("initializeNetwork: reset OK!");

	// set the network key
	if(!ANT_NetworkKey(ANTPLUS_NETWORK_NUMBER, aucNetworkKey)) {
		module_debug_ant("initializeNetwork: network key failed");
		return false;
	}
	else
		module_debug_ant("initializeNetwork: network key OK!");
	
	// start opening the HRM channel
	// assign the hrm-rx channel
	if(!ANT_AssignChannel(ANT_CHANNEL_HRMRX, HRMRX_CHANNEL_TYPE, 
						  ANTPLUS_NETWORK_NUMBER)) {
		module_debug_ant("initializeNetwork: AssignChannel failed");
		return false;
	}
	else
		module_debug_ant("initializeNetwork: AssignChannel OK!");
	
	// assign the channel identifier
	// device number 0 to set to wildcard - any device
	//ANT_ChannelId(ucAntChannel, usDeviceNumber, HRMRX_DEVICE_TYPE, ucTransType );
	if(!ANT_ChannelId(ANT_CHANNEL_HRMRX, m_usDeviceNumber, HRMRX_DEVICE_TYPE, 0)) {
		module_debug_ant("initializeNetwork: ChannelId failed");
		return false;
	}
	else
		module_debug_ant("initializeNetwork: ChannelId OK!");
	
	// assign the radio frequency
	if(!ANT_ChannelRFFreq(ANT_CHANNEL_HRMRX, ANTPLUS_RF_FREQ)) {
		module_debug_ant("initializeNetwork: ChannelRFFreq failed");
		return false;
	}
	else
		module_debug_ant("initializeNetwork: ChannelRFFreq OK!");
	
	// assign the channel period
	if(!ANT_ChannelPeriod(ANT_CHANNEL_HRMRX, HRMRX_MSG_PERIOD)) {
		module_debug_ant("initializeNetwork: ChannelPeriod failed");
		return false;
	}
	else
		module_debug_ant("initializeNetwork: ChannelPeriod OK!");
	
	// open channel
	if(!ANT_OpenChannel(ANT_CHANNEL_HRMRX)) {
		module_debug_ant("initializeNetwork: OpenChannel failed");
		return false;
	}
	else
		module_debug_ant("initializeNetwork: OpenChannel OK!");
	
	module_debug_ant("initializeNetwork successful!");
	return true;
}

bool ANTHRMSensor::ANT_Reset()
{
	module_debug_ant("ANT_Reset");
	uint8_t pucBuffer[ANT_TXBUFSIZE];

	if(pucBuffer)                                         // If there is space in the queue
	{
		pucBuffer[0] = ANT_MESG_SYSTEM_RESET_SIZE;
		pucBuffer[1] = ANT_MESG_SYSTEM_RESET_ID;
		pucBuffer[2] = 0;

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	
	return waitForResponse(0, ANT_MESG_STARTUP_ID, ANT_MESG_SYSTEM_RESET_ID);
}

bool ANTHRMSensor::ANT_NetworkKey(uint8_t ucNetworkNumber_, 
								  const uint8_t* pucKey_)
{
	module_debug_ant("ANT_NetworkKey");
	uint8_t pucBuffer[ANT_TXBUFSIZE];

	if(pucBuffer)                                         // If there is space in the queue
	{
		pucBuffer[0] = ANT_MESG_NETWORK_KEY_SIZE;
		pucBuffer[1] = ANT_MESG_NETWORK_KEY_ID;
		pucBuffer[2] = ucNetworkNumber_;
		pucBuffer[3] = pucKey_[0];
		pucBuffer[4] = pucKey_[1];
		pucBuffer[5] = pucKey_[2];
		pucBuffer[6] = pucKey_[3];
		pucBuffer[7] = pucKey_[4];
		pucBuffer[8] = pucKey_[5];
		pucBuffer[9] = pucKey_[6];
		pucBuffer[10] = pucKey_[7];

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	
	return waitForResponse(0, ANT_MESG_RESPONSE_EVENT_ID, 
						   ANT_MESG_NETWORK_KEY_ID);
}

bool ANTHRMSensor::ANT_AssignChannel(uint8_t ucChannelNumber_, 
									 uint8_t ucChannelType_, 
									 uint8_t ucNetworkNumber_)
{
	module_debug_ant("ANT_AssignChannel");
	uint8_t pucBuffer[ANT_TXBUFSIZE];

	if(pucBuffer)                                         // If there is space in the queue
	{
		pucBuffer[0] = ANT_MESG_ASSIGN_CHANNEL_SIZE;
		pucBuffer[1] = ANT_MESG_ASSIGN_CHANNEL_ID;
		pucBuffer[2] = ucChannelNumber_;
		pucBuffer[3] = ucChannelType_;
		pucBuffer[4] = ucNetworkNumber_;

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	
	return waitForResponse(0, ANT_MESG_RESPONSE_EVENT_ID, 
						   ANT_MESG_ASSIGN_CHANNEL_ID);
}

bool ANTHRMSensor::ANT_ChannelId(uint8_t ucANTChannel_, uint16_t usDeviceNumber_, 
								 uint8_t ucDeviceType_, uint8_t ucTransmitType_)
{
	module_debug_ant("ANT_ChannelId");
	uint8_t pucBuffer[ANT_TXBUFSIZE];

	if(pucBuffer)                                         // If there is space in the queue
	{
         pucBuffer[0] = ANT_MESG_CHANNEL_ID_SIZE;
         pucBuffer[1] = ANT_MESG_CHANNEL_ID_ID;
         pucBuffer[2] = ucANTChannel_;
         pucBuffer[3] = (usDeviceNumber_ & 0xFF);
         pucBuffer[4] = (usDeviceNumber_ >> 8) & 0xFF;
         pucBuffer[5] = ucDeviceType_;
         pucBuffer[6] = ucTransmitType_;

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	return waitForResponse(0, ANT_MESG_RESPONSE_EVENT_ID, ANT_MESG_CHANNEL_ID_ID);
}

bool ANTHRMSensor::ANT_ChannelRFFreq(uint8_t ucANTChannel_, uint8_t ucFreq_)
{
	module_debug_ant("ANT_ChannelRFFreq");
	uint8_t pucBuffer[ANT_TXBUFSIZE];

	if(pucBuffer)                                         // If there is space in the queue
	{
         pucBuffer[0] = ANT_MESG_CHANNEL_RADIO_FREQ_SIZE;
         pucBuffer[1] = ANT_MESG_CHANNEL_RADIO_FREQ_ID;
         pucBuffer[2] = ucANTChannel_;
         pucBuffer[3] = ucFreq_;

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	return waitForResponse(0, ANT_MESG_RESPONSE_EVENT_ID, ANT_MESG_CHANNEL_RADIO_FREQ_ID);
}

bool ANTHRMSensor::ANT_ChannelPeriod(uint8_t ucANTChannel_, uint16_t usPeriod_)
{
	module_debug_ant("ANT_ChannelPeriod");
	uint8_t pucBuffer[ANT_TXBUFSIZE];

	if(pucBuffer)                                         // If there is space in the queue
	{
		pucBuffer[0] = ANT_MESG_CHANNEL_MESG_PERIOD_SIZE;
		pucBuffer[1] = ANT_MESG_CHANNEL_MESG_PERIOD_ID;
		pucBuffer[2] = ucANTChannel_;
		pucBuffer[3] = (usPeriod_ & 0xFF);
		pucBuffer[4] = (usPeriod_ >> 8) & 0xFF;

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	return waitForResponse(0, ANT_MESG_RESPONSE_EVENT_ID, 
						   ANT_MESG_CHANNEL_MESG_PERIOD_ID);
}

bool ANTHRMSensor::ANT_OpenChannel(uint8_t ucANTChannel_)
{
	module_debug_ant("ANT_OpenChannel");
	uint8_t pucBuffer[ANT_TXBUFSIZE];

	if(pucBuffer)                                         // If there is space in the queue
	{
		pucBuffer[0] = ANT_MESG_OPEN_CHANNEL_SIZE;
		pucBuffer[1] = ANT_MESG_OPEN_CHANNEL_ID;
		pucBuffer[2] = ucANTChannel_;

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	return waitForResponse(0, ANT_MESG_RESPONSE_EVENT_ID, 
						   ANT_MESG_OPEN_CHANNEL_ID);
}

bool ANTHRMSensor::ANT_RequestMessage(uint8_t ucANTChannel_, uint8_t ucRequestedMessage_, bool blocking)
{
	module_debug_ant("ANT_RequestMessage");
	uint8_t pucBuffer[ANT_TXBUFSIZE];

	if(pucBuffer)                                         // If there is space in the queue
	{
         pucBuffer[0] = ANT_MESG_REQUEST_SIZE;
         pucBuffer[1] = ANT_MESG_REQUEST_ID;
         pucBuffer[2] = ucANTChannel_;
         pucBuffer[3] = ucRequestedMessage_;

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	
	if(blocking)
		return waitForResponse(0, ucRequestedMessage_, ANT_MESG_REQUEST_ID);
	else
		return true;
}

bool ANTHRMSensor::ANT_UnAssignChannel(uint8_t ucChannelNumber)
{
	module_debug_ant("not implemented");
	return false;
}

bool ANTHRMSensor::ANT_SearchTimeout(uint8_t ucChannelNumber_, uint8_t ucTimeout_)
{
	module_debug_ant("not implemented");
	return false;
}

bool ANTHRMSensor::ANT_ChannelPower(uint8_t ucANTChannel_,  uint8_t ucPower_)
{
	module_debug_ant("not implemented");
	return false;
}

bool ANTHRMSensor::ANT_Broadcast(uint8_t ucANTChannel_, uint8_t* pucBuffer_)
{
	module_debug_ant("not implemented");
	return false;
}


bool ANTHRMSensor::ANT_AcknowledgedTimeout(uint8_t ucChannel_, uint8_t* pucData_, uint16_t usTimeout_)
{
	module_debug_ant("not implemented");
	return false;
}


bool ANTHRMSensor::ANT_Acknowledged(uint8_t ucANTChannel_,  uint8_t* pucBuffer_)
{
	module_debug_ant("not implemented");
	return false;
}


bool ANTHRMSensor::ANT_BurstPacket(uint8_t ucControl_, uint8_t* pucBuffer_)
{
	module_debug_ant("not implemented");
	return false;
}

bool ANTHRMSensor::ANT_CloseChannel(uint8_t ucANTChannel_)
{
	module_debug_ant("ANT_CloseChannel");
	uint8_t pucBuffer[ANT_TXBUFSIZE];

	if(pucBuffer)                                         // If there is space in the queue
	{
		pucBuffer[0] = ANT_MESG_CLOSE_CHANNEL_SIZE;
		pucBuffer[1] = ANT_MESG_CLOSE_CHANNEL_ID;
		pucBuffer[2] = ucANTChannel_;

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	return waitForResponse(0, ANT_MESG_RESPONSE_EVENT_ID, 
						   ANT_MESG_CLOSE_CHANNEL_ID);
}

bool ANTHRMSensor::channelEvent(uint8_t* pucEventBuffer_, 
								ANTPLUS_EVENT_RETURN* pstEventStruct_)
{
   bool bTransmit = true;
   
   uint8_t ucChannel = pucEventBuffer_[BUFFER_INDEX_CHANNEL_NUM] & 0x1F;
   pstEventStruct_->eEvent = ANTPLUS_EVENT_NONE;
   
   if(ucChannel == ANT_CHANNEL_HRMRX)
   {
      if(pucEventBuffer_)
      {
         uint8_t ucANTEvent = pucEventBuffer_[BUFFER_INDEX_MESG_ID];   
         switch( ucANTEvent )
         {
            case ANT_MESG_RESPONSE_EVENT_ID:
            {
               bTransmit = handleResponseEvents( pucEventBuffer_ );
               break;
            }
			// Handle both BROADCAST, ACKNOWLEDGED and BURST data the same
            case ANT_MESG_BROADCAST_DATA_ID:                    
            case ANT_MESG_ACKNOWLEDGED_DATA_ID:
            case ANT_MESG_BURST_DATA_ID:    
            {
               handleDataMessages(pucEventBuffer_, pstEventStruct_);
               break;
               
            } 
            case ANT_MESG_CHANNEL_ID_ID:
            {
               m_usDeviceNumber = (uint16_t) pucEventBuffer_[BUFFER_INDEX_MESG_DATA]; 
               m_usDeviceNumber |= (uint16_t)pucEventBuffer_[BUFFER_INDEX_MESG_DATA+1] << 8;
               m_ucTransType    = (uint8_t) pucEventBuffer_[BUFFER_INDEX_MESG_DATA+3];

               pstEventStruct_->eEvent = ANTPLUS_EVENT_CHANNEL_ID;
               pstEventStruct_->usParam1 = m_usDeviceNumber;
               pstEventStruct_->usParam2 = m_ucTransType;
			   
			   module_debug_ant("got channel id: %x", m_usDeviceNumber);
			   // we are connected to the HRM strap if deviceNumber is nonzero
			   if(m_usDeviceNumber != 0)
				   m_isConnected = true;
			   
               break;     
            }
         }
      } 
   }
   return (bTransmit);
}

void ANTHRMSensor::handleSearchTimeout()
{
	// current our strategy for handling search timeout is opening the channel
	// again, basically providing infinite timeout
	m_isConnected = false;
	if(!ANT_OpenChannel(ANT_CHANNEL_HRMRX)) {
		module_debug_ant("initializeNetwork: OpenChannel failed");
	}
}

bool ANTHRMSensor::handleResponseEvents(uint8_t * pucBuffer_)
{
	
	if(pucBuffer_)
   {
	   //module_debug_ant("resp code %x", pucBuffer_[BUFFER_INDEX_RESPONSE_CODE]);
	   switch(pucBuffer_[BUFFER_INDEX_RESPONSE_CODE] )
	   {
	   case RESPONSE_NO_ERROR:
		   // good, good...
		   break;
	   case EVENT_RX_SEARCH_TIMEOUT:
		   module_debug_ant("search timeout!");
		   handleSearchTimeout();
		   break;
	   case EVENT_CHANNEL_CLOSED:
		   module_debug_ant("channel closed!");
		   // TODO how should we handle channel closure?
		   break;
	   default:
		   break;
	   }
   }
   
	return false;
}

void ANTHRMSensor::handleDataMessages(uint8_t* pucBuffer_, 
									  ANTPLUS_EVENT_RETURN* pstEventStruct_)
{
   pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
   pstEventStruct_->usParam1 = HRM_PAGE_0;

   decodeDefault( &pucBuffer_[BUFFER_INDEX_MESG_DATA+4] );
   
   if(m_usDeviceNumber == 0)
   {
		// non-blocking ANT_RequestMessage call, we'll handle the result
		// in the event handlers
		module_debug_ant("requesting channel ID...");
		ANT_RequestMessage(m_ucAntChannel, ANT_MESG_CHANNEL_ID_ID, false);
   }
}
