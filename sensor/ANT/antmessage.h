/////////////////////////////////////////////////////////////////////////////////////////
// THE FOLLOWING EXAMPLE CODE IS INTENDED FOR LIMITED CIRCULATION ONLY.
// 
// Please forward all questions regarding this code to ANT Technical Support.
// 
// Dynastream Innovations Inc.
// 228 River Avenue
// Cochrane, Alberta, Canada
// T4C 2C1
// 
// (P) (403) 932-9292
// (F) (403) 932-6521
// (TF) 1-866-932-9292
// (E) support@thisisant.com
// 
// www.thisisant.com
//
// Reference Design Disclaimer
//
// The references designs and codes provided may be used with ANT devices only and remain the copyrighted property of 
// Dynastream Innovations Inc. The reference designs and codes are being provided on an "as-is" basis and as an accommodation, 
// and therefore all warranties, representations, or guarantees of any kind (whether express, implied or statutory) including, 
// without limitation, warranties of merchantability, non-infringement,
// or fitness for a particular purpose, are specifically disclaimed.
//
// ©2008 Dynastream Innovations Inc. All Rights Reserved
// This software may not be reproduced by
// any means without express written approval of Dynastream
// Innovations Inc.
//
/////////////////////////////////////////////////////////////////////////////////////////
// Modified by Yaman Umuroglu: add ANT_ in front of defines in file

#ifndef ANTMESSAGE_H
#define ANTMESSAGE_H

#include "types.h"

/////////////////////////////////////////////////////////////////////////////
// Message Format
// Messages are in the format:
//
// AX XX YY -------- CK
//
// where: AX    is the 1 byte sync byte either transmit or recieve
//        XX    is the 1 byte size of the message (0-249) NOTE: THIS WILL BE LIMITED BY THE EMBEDDED RECEIVE BUFFER SIZE
//        YY    is the 1 byte ID of the message (1-255, 0 is invalid)
//        ----- is the data of the message (0-249 bytes of data)
//        CK    is the 1 byte Checksum of the message
/////////////////////////////////////////////////////////////////////////////
#define ANT_MESG_TX_SYNC                      ((UCHAR)0xA4)
#define ANT_MESG_RX_SYNC                      ((UCHAR)0xA5)
#define ANT_MESG_SIZE_OFFSET                  ((UCHAR)0)    
#define ANT_MESG_ID_OFFSET                    ((UCHAR)1)     
#define ANT_MESG_SYNC_SIZE                    ((UCHAR)1)
#define ANT_MESG_SIZE_SIZE                    ((UCHAR)1)
#define ANT_MESG_ID_SIZE                      ((UCHAR)1)
#define ANT_MESG_CHECKSUM_SIZE                ((UCHAR)1)
#define ANT_MESG_MAX_DATA_SIZE                ((UCHAR)9)     
#define ANT_MESG_MAX_DATA_BYTES               ((UCHAR)8)
#define ANT_MESG_HEADER_SIZE                  (ANT_MESG_SYNC_SIZE + ANT_MESG_SIZE_SIZE + ANT_MESG_ID_SIZE)
#define ANT_MESG_DATA_OFFSET                  ANT_MESG_HEADER_SIZE-1  //2
#define ANT_MESG_FRAME_SIZE                   (ANT_MESG_HEADER_SIZE + ANT_MESG_CHECKSUM_SIZE)
#define ANT_MESG_SAVED_FRAME_SIZE             (ANT_MESG_SIZE_SIZE + ANT_MESG_ID_SIZE)
#define ANT_MESG_MAX_SIZE                     (ANT_MESG_MAX_DATA_SIZE + ANT_MESG_FRAME_SIZE)
#define ANT_MESG_BUFFER_SIZE                  (ANT_MESG_MAX_DATA_SIZE + ANT_MESG_SAVED_FRAME_SIZE)

#define ANT_DATA_SIZE                     8                    // ANT message payload size.
#define ANT_TX_SYNC                      ((UCHAR) 0xA4)
#define ANT_RX_SYNC                      ((UCHAR) 0xA5)
#define ANT_SIZE_OFFSET                  ((UCHAR) 0x00)
#define ANT_ID_OFFSET                    ((UCHAR) 0x01)
#define ANT_SYNC_SIZE                    ((UCHAR) 0x01)
#define ANT_SIZE_SIZE                    ((UCHAR) 0x01)
#define ANT_ID_SIZE                      ((UCHAR) 0x01)
#define ANT_RESPONSE_ERROR_SIZE          ((UCHAR) 0x01)
#define ANT_CHECKSUM_SIZE                ((UCHAR) 0x01)
#define ANT_MAX_DATA_SIZE                ((UCHAR) 0x17)
#define ANT_HEADER_SIZE                  (ANT_SYNC_SIZE + ANT_SIZE_SIZE + ANT_ID_SIZE)
#define ANT_REPLY_HEADER_SIZE            ANT_HEADER_SIZE + ANT_RESPONSE_ERROR_SIZE
#define ANT_DATA_OFFSET                  ANT_HEADER_SIZE-1     
#define ANT_FRAME_SIZE                   (ANT_HEADER_SIZE + ANT_CHECKSUM_SIZE)
#define ANT_MAX_SIZE                     (ANT_MAX_DATA_SIZE + ANT_FRAME_SIZE)


//////////////////////////////////////////////
// Buffer Indices
//////////////////////////////////////////////
#define BUFFER_INDEX_MESG_SIZE            ((UCHAR)0x00)
#define BUFFER_INDEX_MESG_ID              ((UCHAR)0x01)
#define BUFFER_INDEX_CHANNEL_NUM          ((UCHAR)0x02)
#define BUFFER_INDEX_MESG_DATA            ((UCHAR)0x03)
#define BUFFER_INDEX_RESPONSE_MESG_ID     ((UCHAR)0x03)
#define BUFFER_INDEX_RESPONSE_CODE        ((UCHAR)0x04)
#define BUFFER_INDEX_DATA0                ((UCHAR)0x03)
#define BUFFER_INDEX_SHARED_ADDRESS_LSB   ((UCHAR)0x03)
#define BUFFER_INDEX_SHARED_ADDRESS_MSB   ((UCHAR)0x04)
#define BUFFER_INDEX_SHARED_DATA_TYPE     ((UCHAR)0x05)

//////////////////////////////////////////////
// Message ID's
//////////////////////////////////////////////
#define ANT_MESG_INVALID_ID                   ((UCHAR)0x00)
#define ANT_MESG_EVENT_ID                     ((UCHAR)0x01)

#define ANT_MESG_APPVERSION_ID                ((UCHAR)0x3D)  ///< application interface version
#define ANT_MESG_VERSION_ID                   ((UCHAR)0x3E)  ///< protocol library version
#define ANT_MESG_RESPONSE_EVENT_ID            ((UCHAR)0x40)

#define ANT_MESG_UNASSIGN_CHANNEL_ID          ((UCHAR)0x41)
#define ANT_MESG_ASSIGN_CHANNEL_ID            ((UCHAR)0x42)
#define ANT_MESG_CHANNEL_MESG_PERIOD_ID       ((UCHAR)0x43)
#define ANT_MESG_CHANNEL_SEARCH_TIMEOUT_ID    ((UCHAR)0x44)
#define ANT_MESG_CHANNEL_RADIO_FREQ_ID        ((UCHAR)0x45)
#define ANT_MESG_NETWORK_KEY_ID               ((UCHAR)0x46)
#define ANT_MESG_RADIO_TX_POWER_ID            ((UCHAR)0x47)
#define ANT_MESG_RADIO_CW_MODE_ID             ((UCHAR)0x48)
#define ANT_MESG_SEARCH_WAVEFORM_ID           ((UCHAR)0x49)

#define ANT_MESG_SYSTEM_RESET_ID              ((UCHAR)0x4A)
#define ANT_MESG_OPEN_CHANNEL_ID              ((UCHAR)0x4B)
#define ANT_MESG_CLOSE_CHANNEL_ID             ((UCHAR)0x4C)
#define ANT_MESG_REQUEST_ID                   ((UCHAR)0x4D)

#define ANT_MESG_BROADCAST_DATA_ID            ((UCHAR)0x4E)
#define ANT_MESG_ACKNOWLEDGED_DATA_ID         ((UCHAR)0x4F)
#define ANT_MESG_BURST_DATA_ID                ((UCHAR)0x50)

#define ANT_MESG_CHANNEL_ID_ID                ((UCHAR)0x51)
#define ANT_MESG_CHANNEL_STATUS_ID            ((UCHAR)0x52)
#define ANT_MESG_RADIO_CW_INIT_ID             ((UCHAR)0x53)
#define ANT_MESG_CAPABILITIES_ID              ((UCHAR)0x54)
#define ANT_MESG_SENSRCORE_DIGITAL_DATA       ((UCHAR)0x40)

#define ANT_MESG_PIN_DIODE_CONTROL_ID		   ((UCHAR)0x90)
#define ANT_MESG_RUN_SCRIPT_ID                ((UCHAR)0x91)
#define ANT_MESG_STARTUP_ID                   ((UCHAR)0x6F)

//////////////////////////////////////////////
// Message Sizes
//////////////////////////////////////////////
#define ANT_MESG_INVALID_SIZE                 ((UCHAR)0)

#define ANT_MESG_RESPONSE_EVENT_SIZE          ((UCHAR)3)
#define ANT_MESG_CHANNEL_STATUS_SIZE          ((UCHAR)2)
#define ANT_MESG_VERSION_SIZE                 ((UCHAR)9)

#define ANT_MESG_UNASSIGN_CHANNEL_SIZE        ((UCHAR)1)
#define ANT_MESG_ASSIGN_CHANNEL_SIZE          ((UCHAR)3)
#define ANT_MESG_CHANNEL_ID_SIZE              ((UCHAR)5)
#define ANT_MESG_CHANNEL_MESG_PERIOD_SIZE     ((UCHAR)3)
#define ANT_MESG_CHANNEL_SEARCH_TIMEOUT_SIZE  ((UCHAR)2)
#define ANT_MESG_CHANNEL_RADIO_FREQ_SIZE      ((UCHAR)2)
#define ANT_MESG_NETWORK_KEY_SIZE             ((UCHAR)9)
#define ANT_MESG_RADIO_TX_POWER_SIZE          ((UCHAR)2)
#define ANT_MESG_RADIO_CW_MODE_SIZE           ((UCHAR)3)
#define ANT_MESG_RADIO_CW_INIT_SIZE           ((UCHAR)1)
#define ANT_MESG_SEARCH_WAVEFORM_SIZE         ((UCHAR)3)

#define ANT_MESG_SYSTEM_RESET_SIZE            ((UCHAR)1)
#define ANT_MESG_OPEN_CHANNEL_SIZE            ((UCHAR)1)
#define ANT_MESG_CLOSE_CHANNEL_SIZE           ((UCHAR)1)
#define ANT_MESG_REQUEST_SIZE                 ((UCHAR)2)
#define ANT_MESG_CAPABILITIES_SIZE            ((UCHAR)4)

#define ANT_MESG_DATA_SIZE                    ((UCHAR)9)

#define ANT_MESG_PIN_DIODE_CONTROL_ID_SIZE    ((UCHAR)2)
#define ANT_MESG_RUN_SCRIPT_SIZE              ((UCHAR)2)


#endif // !ANTMESSAGE_H
