#include "em_emu.h"
#include "em_gpio.h"
#include "uartmanager.h"
#include "anthrmsensor.h"
#include "ANT/antmessage.h"
#include "debug_output_control.h"

bool ANTRxHook(uint8_t c)
{
	ANTHRMSensor::getInstance()->processUARTRxChar(c);
	return true; // tell the UART driver we already processed the data
}

ANTHRMSensor::ANTHRMSensor() :
  Sensor(typeHeartRate, 8, ANTHRM_DEFAULT_RATE)
{
	// initialize rx-tx queues
	m_stTheTxQueue.ucHead = 0;                     // Reset TX queue
	m_stTheTxQueue.ucTail = 0;
	m_stTheTxQueue.ucCurrentByte = 0;
	m_stTheTxQueue.ucCheckSum = 0;
	m_stTheTxQueue.astBuffer = m_stTxBuffer;

	m_stTheRxQueue.ucHead = 0;                     //  Reset RX queue
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
	
	// initialize page data
	m_page0Data.usBeatTime = 0;                                  
	m_page0Data.ucBeatCount = 0;                                 
	m_page0Data.ucComputedHeartRate = 0; 
	m_page1Data.ulOperatingTime = 0;
	m_page2Data.ucManId = 0;
	m_page2Data.ulSerialNumber = 0;
	m_page3Data.ucHwVersion = 0;
	m_page3Data.ucSwVersion = 0;
	m_page3Data.ucModelNumber = 0;
	m_page4Data.usPreviousBeat = 0;

	m_eThePageState = STATE_INIT_PAGE; 
	
	
    // initialize UART
    // TODO UARTManagerPortLEUART0 should be defined in part-specific config!
    m_port = UARTManager::getInstance()->getPort(UARTManagerPortLEUART0);

    m_port->initialize((uint8_t *) NULL, 0, 
                     UARTPort::uartPortBaudRate4800, 
                     UARTPort::uartPortDataBits8, UARTPort::uartPortParityNone, 
                     UARTPort::uartPortStopBits1);
	
	// set the rx hook we use to trap incoming characters
	m_port->setRxHook(&ANTRxHook);
	
	// initialize other GPIO pins used to control the ANT
	GPIO_PinModeSet(GPIO_ANT_RTS, gpioModeInput, 0);
	// RST won't exist for final project but just for prototyping stage
	// so only configure it if it exists
#ifdef GPIO_ANT_RST
	GPIO_PinModeSet(GPIO_ANT_RST, gpioModePushPull, 1);
#endif
	// TODO configure sleep pin here
    
    setPeriod(ANTHRM_DEFAULT_RATE);
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

}

const void* ANTHRMSensor::readSensorData(uint16_t *actualSize)
{
    *actualSize = sizeof(HeartRateMessage);
    return (const void *) &m_hrmMessage;
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

uint8_t * ANTHRMSensor::getTxBuffer()
{
	if(((m_stTheTxQueue.ucHead + 1) & (ANT_SERIAL_QUEUE_TX_SIZE - 1)) != m_stTheTxQueue.ucTail)
		return(m_stTheTxQueue.astBuffer[m_stTheTxQueue.ucHead].aucBuffer);
	
	return (uint8_t *) NULL;
}

void ANTHRMSensor::putTxBuffer()
{
	m_stTheTxQueue.ucHead = ((m_stTheTxQueue.ucHead + 1) & (ANT_SERIAL_QUEUE_TX_SIZE - 1));
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

void ANTHRMSensor::flushTx()
{
   
   m_stTheTxQueue.ucHead = 0;
   m_stTheTxQueue.ucTail = 0;
   m_stTheTxQueue.ucCurrentByte = 0;
   m_stTheTxQueue.ucCheckSum = 0;
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

// TODO this is a "cleverly hidden" send function - should be rewritten
// with name and possibly DMA semantics if we are planning on using them
// it just sends one pending message from the tx queue
// pretends to do RTS checking, but does it only once?
void ANTHRMSensor::sendPendingTx()
{
	if(m_stTheTxQueue.ucHead != m_stTheTxQueue.ucTail)  // if we have a message to send
   {
      //ASYNC_SLEEP_DEASSERT();       // keep ANT from sleeping TODO SLEEP
      // Do HW flow control at the message level
      if (!GPIO_PinInGet(GPIO_ANT_RTS)) // check RTS
      {  

         uint8_t* pucTxBuffer;        
         pucTxBuffer = m_stTheTxQueue.astBuffer[m_stTheTxQueue.ucTail].aucBuffer;
		 sendANTMessage(pucTxBuffer, pucTxBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
         m_stTheTxQueue.ucTail = (m_stTheTxQueue.ucTail + 1) & (ANT_SERIAL_QUEUE_TX_SIZE - 1);   // Update queue
      }
      //ASYNC_SLEEP_ASSERT();      // let ANT sleep TODO sleep
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

bool ANTHRMSensor::initializeNetwork()
{
	// reset the module with software command
	if(!ANT_Reset()) {
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
	if(!ANT_ChannelId(ANT_CHANNEL_HRMRX, 0, HRMRX_DEVICE_TYPE, 0)) {
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
	uint8_t * pucBuffer = getTxBuffer();            // Get space from the queue

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
	uint8_t * pucBuffer = getTxBuffer();            // Get space from the queue

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
	uint8_t * pucBuffer = getTxBuffer();            // Get space from the queue

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
	uint8_t * pucBuffer = getTxBuffer();            // Get space from the queue

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
	uint8_t * pucBuffer = getTxBuffer();            // Get space from the queue

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
	uint8_t * pucBuffer = getTxBuffer();            // Get space from the queue

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
	uint8_t * pucBuffer = getTxBuffer();            // Get space from the queue

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

bool ANTHRMSensor::ANT_RequestMessage(uint8_t ucANTChannel_, uint8_t ucRequestedMessage_)
{
	module_debug_ant("ANT_OpenChannel");
	uint8_t * pucBuffer = getTxBuffer();            // Get space from the queue

	if(pucBuffer)                                         // If there is space in the queue
	{
         pucBuffer[0] = ANT_MESG_REQUEST_SIZE;
         pucBuffer[1] = ANT_MESG_REQUEST_ID;
         pucBuffer[2] = ucANTChannel_;
         pucBuffer[3] = ucRequestedMessage_;

		sendANTMessage(pucBuffer, pucBuffer[0] + ANT_MESG_SAVED_FRAME_SIZE);
	} else
		return false;
	
	return waitForResponse(0, ucRequestedMessage_, ANT_MESG_REQUEST_ID);
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
	module_debug_ant("not implemented");
	return false;
}

bool ANTHRMSensor::channelEvent(uint8_t* pucEventBuffer_, ANTPLUS_EVENT_RETURN* pstEventStruct_)
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
               bTransmit = handleDataMessages(pucEventBuffer_, pstEventStruct_);
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
               break;     
            }
         }
      } 
   }
   return (bTransmit);
}

bool ANTHRMSensor::handleResponseEvents(uint8_t * pucBuffer_)
{
	module_debug_ant("handleResponseEvents not implemented");
	return false;
}

bool ANTHRMSensor::handleDataMessages(uint8_t* pucBuffer_, 
									  ANTPLUS_EVENT_RETURN* pstEventStruct_)
{
   bool bTransmit = FALSE;
   static uint8_t ucOldPage;
   
   uint8_t ucPage = pucBuffer_[BUFFER_INDEX_MESG_DATA];
   pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
   pstEventStruct_->usParam1 = HRM_PAGE_0;

   switch(m_eThePageState)
   {
      case STATE_INIT_PAGE:
      {
         m_eThePageState = STATE_STD_PAGE;               
         break;                  
      }
      case STATE_STD_PAGE:
      {

         // Check if the page if changing, if yes
         // then move to the next state, otherwise
         // only interpret page 0
         if(ucOldPage == ucPage)
            break;
         else
            m_eThePageState = STATE_EXT_PAGE;
   
         // INTENTIONAL FALLTHROUGH !!!
      }
      case STATE_EXT_PAGE:
      {
         switch(ucPage & ~TOGGLE_MASK)
         {
            case HRM_PAGE_1:
            {
               HRMPage1_Data* pstPage1Data = &m_page1Data;

               pstPage1Data->ulOperatingTime  = (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+1]; 
               pstPage1Data->ulOperatingTime |= (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+2] << 8; 
               pstPage1Data->ulOperatingTime |= (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+3] << 16; 
               pstPage1Data->ulOperatingTime *= 2;
      
               pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
               pstEventStruct_->usParam1 = HRM_PAGE_1;
               break;
            }
            case HRM_PAGE_2:
            {
               HRMPage2_Data* pstPage2Data = &m_page2Data;

               pstPage2Data->ucManId = pucBuffer_[BUFFER_INDEX_MESG_DATA + 1];
               pstPage2Data->ulSerialNumber  = (uint32_t) m_usDeviceNumber;
               pstPage2Data->ulSerialNumber |= (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+2] << 16;
               pstPage2Data->ulSerialNumber |= (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+3] << 24;

               pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
               pstEventStruct_->usParam1 = HRM_PAGE_2;
               break;                        
            }
            case HRM_PAGE_3:
            {
               HRMPage3_Data* pstPage3Data = &m_page3Data;

               pstPage3Data->ucHwVersion   = (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+1];
               pstPage3Data->ucSwVersion   = (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+2];
               pstPage3Data->ucModelNumber = (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+3];

               pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
               pstEventStruct_->usParam1 = HRM_PAGE_3;
               break;
            }
            case HRM_PAGE_4:
            {
               HRMPage4_Data* pstPage4Data = &m_page4Data;

               pstPage4Data->usPreviousBeat  = (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+2];
               pstPage4Data->usPreviousBeat |= (uint32_t)pucBuffer_[BUFFER_INDEX_MESG_DATA+3] << 8;

               pstEventStruct_->eEvent = ANTPLUS_EVENT_PAGE;
               pstEventStruct_->usParam1 = HRM_PAGE_4;
               break;
            }
            case HRM_PAGE_0:
            {
               // Handled above and below, so don't fall-thru to default case
               break;
            }
            default:
            {
               pstEventStruct_->eEvent = ANTPLUS_EVENT_UNKNOWN_PAGE;
               break;
            }
         }
         break;
      }
   }
   ucOldPage = ucPage;
   decodeDefault( &pucBuffer_[BUFFER_INDEX_MESG_DATA+4] );  

   if(m_usDeviceNumber == 0)
   {
      if(ANT_RequestMessage(m_ucAntChannel, ANT_MESG_CHANNEL_ID_ID))
         bTransmit = TRUE;
   }
   
   return(bTransmit);
}

void ANTHRMSensor::decodeDefault(uint8_t* pucPayload_)
{
   HRMPage0_Data* pstPage0Data = &m_page0Data;

   pstPage0Data->usBeatTime = (uint16_t)pucPayload_[0];                  // Measurement time
   pstPage0Data->usBeatTime |= (uint16_t)pucPayload_[1] << 8;
   pstPage0Data->ucBeatCount = (uint8_t)pucPayload_[2];                  // Measurement count
   pstPage0Data->ucComputedHeartRate = (uint16_t)pucPayload_[3];         // Computed heart rate
}

void ANTHRMSensor::processANTHRMRXEvents(ANTPLUS_EVENT_RETURN* pstEvent_)
{
   static UCHAR ucPreviousBeatCount = 0;
   
   switch (pstEvent_->eEvent)
   {

      case ANTPLUS_EVENT_CHANNEL_ID:
      {
         // Can store this device number for future pairings so that 
         // wild carding is not necessary.
         printf("Device Number is %d\n", pstEvent_->usParam1);
         printf("Transmission type is %d\n\n", pstEvent_->usParam2);
         break;
      }
      case ANTPLUS_EVENT_PAGE:
      {
         HRMPage0_Data* pstPage0Data = &m_page0Data; //common data
         BOOL bCommonPage = FALSE;

         //IOBOARD_LED3_OUT &= ~IOBOARD_LED3_BIT;    // TURN ON LED 3
         printf("LED3 on\n");

         //print formulated page identifier
         if (pstEvent_->usParam1 <= HRM_PAGE_4)
            printf("Heart Rate Monitor Page %d\n", pstEvent_->usParam1);

         // Get data correspinding to the page. Only get the data you 
         // care about.
         switch(pstEvent_->usParam1)
         {
            case HRM_PAGE_0:
            {
               bCommonPage = TRUE;
               break;
            }
            case HRM_PAGE_1:
            {
               HRMPage1_Data* pstPage1Data = &m_page1Data;
               ULONG ulMinutes, ulHours, ulDays, ulSeconds;

               ulDays = (ULONG)((pstPage1Data->ulOperatingTime) / 86400);  //1 day == 86400s
               ulHours = (ULONG)((pstPage1Data->ulOperatingTime) % 86400); // half the calculation so far
               ulMinutes = ulHours % (ULONG)3600;
               ulSeconds = ulMinutes % (ULONG)60;
               ulHours /= (ULONG)3600; //finish the calculations: hours = 1hr == 3600s
               ulMinutes /= (ULONG)60; //finish the calculations: minutes = 1min == 60s

               printf("Cumulative operating time: %dd ", ulDays);
               printf("%dh ", ulHours);
               printf("%dm ", ulMinutes);
               printf("%ds\n\n", ulSeconds);
               bCommonPage = TRUE;
               break;
            }
            case HRM_PAGE_2:
            {
               HRMPage2_Data* pstPage2Data = &m_page2Data;

               printf("Manufacturer ID: %u\n", pstPage2Data->ucManId);
               printf("Serial No (upper 16-bits): 0x%X\n", pstPage2Data->ulSerialNumber);               
               bCommonPage = TRUE;
               break;
            }
            case HRM_PAGE_3:
            {
               HRMPage3_Data* pstPage3Data = &m_page3Data;

               printf("Hardware Rev ID %u ", pstPage3Data->ucHwVersion);
               printf("Model %u\n", pstPage3Data->ucModelNumber);
               printf("Software Ver ID %u\n", pstPage3Data->ucSwVersion);
               bCommonPage = TRUE;
               break;
            }
            case HRM_PAGE_4:
            {
               HRMPage4_Data* pstPage4Data = &m_page4Data;
               
               printf("Previous heart beat event: %u.", (ULONG)(pstPage4Data->usPreviousBeat/1024));
               printf("%03u s\n", (ULONG)((((pstPage4Data->usPreviousBeat % 1024) * HRM_PRECISION) + 512) / 1024));
               
               if((pstPage0Data->ucBeatCount - ucPreviousBeatCount) == 1)	// ensure that there is only one beat between time intervals
               {
                  USHORT usR_RInterval = pstPage0Data->usBeatTime - pstPage4Data->usPreviousBeat;	// subtracting the event time gives the R-R interval
                  printf("R-R Interval: %u.", (ULONG)(usR_RInterval/1024));
                  printf("%03u s\n", (ULONG)((((usR_RInterval % 1024) * HRM_PRECISION) + 512) / 1024));
               }
               ucPreviousBeatCount = pstPage0Data->ucBeatCount;
                              
               bCommonPage = TRUE;
               break;
            }
           default:
            {
               // ASSUME PAGE 0
               printf("Unknown format\n\n");
               break; 
            }
         }
         if(bCommonPage)
         {
            printf("Time of last heart beat event: %u.", (ULONG)(pstPage0Data->usBeatTime/1024));
            printf("%03u s\n", (ULONG)((((pstPage0Data->usBeatTime % 1024) * HRM_PRECISION) + 512) / 1024));
            printf("Heart beat count: %u\n", pstPage0Data->ucBeatCount);
            printf("Instantaneous heart rate: %u bpm\n\n", pstPage0Data->ucComputedHeartRate);
         }
         //IOBOARD_LED3_OUT |= IOBOARD_LED3_BIT;    // TURN OFF LED 3
         printf("LED3 off\n");
         break;
      }

      case ANTPLUS_EVENT_UNKNOWN_PAGE:  // Decode unknown page manually
      case ANTPLUS_EVENT_NONE:
      default:
      {
     	 break;
      }  
   }
}

void ANTHRMSensor::transaction()
{
	uint8_t *pucRxBuffer = readRxBuffer();        // Check if any data has been recieved from serial
    ANTPLUS_EVENT_RETURN stEventStruct;
    
    if(pucRxBuffer)
    {
       /*if(HRMRX_ChannelEvent(pucRxBuffer, &stEventStruct))
          usLowPowerMode = 0;*/
       channelEvent(pucRxBuffer, &stEventStruct);
       processANTHRMRXEvents(&stEventStruct);

       releaseRxBuffer();
    }  
}