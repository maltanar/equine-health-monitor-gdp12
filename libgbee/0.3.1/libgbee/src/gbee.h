/**
 * \mainpage
 *
 * This driver provides access to the XBee/XBee Pro RF modules. XBee modules
 * are connected to the host via a serial interface.
 *
 * All the serial communication is handled by the porting layer of the libgbee.
 * The porting layer adapts the platform independent part of the driver to a
 * specific architecture/operating system. Available ports can be found in the
 * libgbee/src/port directory. For more information about ports, see section
 * \ref porting.
 *
 * See \ref build_instructions for building the libgbee from source code.
 *
 * To initialize a connection with the XBee module the gbeeCreate() function
 * has to be called.
 *
 * XBee modules provide two basic modes of operation:
 *
 * <ul>
 * <li> Transparent Mode
 * <li> API Mode
 * </ul>
 *
 * By default, XBee modules operate in transparent mode, where the XBee
 * operates as a transparent serial interface replacement. In API mode the host
 * communicates with the XBee using so-called API frames. This allows the host
 * application a higher level of control over the XBee module.
 *
 * To switch from transparent mode to API mode and vice-versa, this driver
 * provides the gbeeSetMode() function. Additionally it is possible to query
 * the current mode using the gbeeGetMode() function.
 *
 * If the XBee is operated in transparent mode, it is still possible to access
 * the configuration and status registers of the XBee module using so-called AT
 * commands. To send AT commands to the XBee module this driver provides the
 * gbeeXferAtCommand() function.
 *
 * However, the preferred way is to use the XBee module in API mode. In API
 * mode all communication with the module is contained in API frames. The XBee
 * module knows the following API frame types:
 *
 * <ul>
 * <li> Modem Status (::GBeeModemStatus)
 * <li> AT Command (::GBeeAtCommand)
 * <li> AT Command - Queue Parameter Value (::GBeeAtCommandQueue)
 * <li> AT Command Response (::GBeeAtCommandResponse)
 * <li> Remote AT Command Request (::GBeeRemoteAtCommand)
 * <li> Remote Command Response (::GBeeRemoteAtCommandResponse)
 * <li> Transmit Request: 64-bit address (::GBeeTxRequest64)
 * <li> Transmit Request: 16-bit address (::GBeeTxRequest16)
 * <li> Transmit Request: 16-bit address (::GBeeTxRequest)
 * <li> Transmit Status (::GBeeTxStatus)
 * <li> Receive Packet: 64-bit address (::GBeeRxPacket64)
 * <li> Receive Packet: 16-bit address (::GBeeRxPacket16)
 * </ul>
 *
 * To send API frames to the XBee this driver provides the gbeeSend() function.
 * To receive API frames from the XBee this driver provides the gbeeReceive()
 * function.
 *
 * For convenience there is also a dedicated send function for each API frame
 * type, namely: gbeeSendAtCommand(), gbeeSendAtCommandQueue(),
 * gbeeSendRemoteAtCommand(), gbeeSendTxRequest64(), gbeeSendTxRequest16()
 * gbeeSendTxRequest().
 *
 * See section \ref utility_functions for additional utility functions provided
 * by the libgbee.
 *
 * A totally different approach to send and receive data using a XBee module is
 * provided by the XBee Tunnel Daemon. This daemon provides a virtual network
 * interface (precisely: a TUN device) on GNU/Linux platforms. So data can be
 * send to or received from the XBee using the well-known BSD socket interface.
 * For more information on the XBee Tunnel Daemon, please refer to the "XBee
 * Tunnel Daemon Reference Manual".
 *
 * \page build_instructions Build Instructions
 * \section build_instructions Build Instructions
 *
 * Before building the libgbee for a specific platform, the build system needs
 * to be configured for this platform.
 *
 * The libgbee uses the CMake build system. To configure the build system for
 * the desired target platform, create a directory where you want to build the
 * binary files, e.g.
 * \code
 * ~/xbee/build/<target OS>/<target processor>/libgbee
 * \endcode
 * Open a console window and browse to the directory just created. Type
 * \code
 * cmake-gui .
 * \endcode
 * This will start the Qt-based CMake configuration dialog. For more
 * information about CMake and the cmake-gui utility, please refer to
 * http://www.cmake.org.
 *
 * See the port-specific build instructions for building the libgbee for the
 * available target platforms:
 * <ul>
 * <li> \ref building_for_at91_sam7
 * <li> \ref building_for_linux_mipsel
 * <li> \ref building_for_linux_x86
 * <li> \ref building_for_windows_x86
 * </ul>
 *
 * \file
 * \author  d264
 * \version $Rev$
 *
 * \section DESCRIPTION
 *
 * This is the main header file for the GBee driver. It contains all the type
 * definitions for XBee API frames and the GBee low level API.
 *
 * \section LICENSE
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef __cplusplus
extern "C"{
#endif 

#ifndef GBEE_H_INCLUDED
#define GBEE_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include "gbee-port-interface.h"

/* IAR CC uses a different keyword for packed structures */
#if defined ( __ICCARM__ )
  #define PACKEDSTRUCT __packed struct
#else
  #define PACKEDSTRUCT struct __attribute__((__packed__))
#endif

#if defined(GBEE_PORT_BIG_ENDIAN) && !defined(GBEE_PORT_LITTLE_ENDIAN)
/** Convert 16-bit value into XBee byte-order (big-endian). */
#define GBEE_USHORT(x) (x)
/** Convert 32-bit value into XBee byte-order (big-endian). */
#define GBEE_ULONG(x)  (x)
#elif defined (GBEE_PORT_LITTLE_ENDIAN) && !defined(GBEE_PORT_BIG_ENDIAN)
/** Convert 16-bit value into XBee byte-order (big-endian). */
#define GBEE_USHORT(x) (\
		(((uint16_t)(x) & 0xff00) >> 8) | \
		(((uint16_t)(x) & 0x00ff) << 8))
/** Convert 32-bit value into XBee byte-order (big-endian). */
#define GBEE_ULONG(x) (\
		(((uint32_t)(x) & 0xff000000) >> 24) | \
		(((uint32_t)(x) & 0x00ff0000) >> 8)  | \
		(((uint32_t)(x) & 0x0000ff00) << 8)  | \
		(((uint32_t)(x) & 0x000000ff) << 24))
#else
#error "Must define either GBEE_PORT_LITTLE_ENDIAN or GBEE_PORT_BIG_ENDIAN"
#endif

/** Maximum length of XBee message payload. */
#define GBEE_MAX_PAYLOAD_LENGTH 100
/** Maximum length of XBee API frame. */
#define GBEE_MAX_FRAME_SIZE     (GBEE_MAX_PAYLOAD_LENGTH + sizeof(GBeeFrameData))
/** Maximum length of XBee API frame including frame header and trailer. */
#define GBEE_TOTAL_FRAME_SIZE   (sizeof(GBeeFrameHeader) + GBEE_MAX_FRAME_SIZE + sizeof(GBeeFrameTrailer))

/**
 * Enumeration of XBee modes.
 */
enum gbeeMode {
	/** XBee operates in transparent mode (i.e. serial interface replacement) */
	GBEE_MODE_TRANSPARENT = 0,
	/** XBee operates in API mode. */
	GBEE_MODE_API = 1
};

/** Type definition for ::gbeeMode. */
typedef enum gbeeMode GBeeMode;

/**
 * RF module status messages are sent from the module in response to specific
 * conditions.
 */
PACKEDSTRUCT gbeeModemStatus {
	/** API identifier: 0x8A. */
	uint8_t ident;
	/** Modem status byte. */
	uint8_t status;
};

/** Type definition for ::gbeeModemStatus. */
typedef struct gbeeModemStatus GBeeModemStatus;

/**
 * The “AT command” API type allows for module parameters to be queried or 
 * set. When using this command ID, new parameter values are applied 
 * immediately. This includes any register set with the “AT command - Queue 
 * Parameter value” (0x09) API type.
 */
PACKEDSTRUCT gbeeAtCommand {
	/** API identifier: 0x08. */
	uint8_t ident;
	/** 
	 * Identifies the UART data frame being reported. Note: If Frame ID = 0 in
	 * AT command mode, no AT command response will be given. 
	 */
	uint8_t frameId;
	/** command Name - Two ASCII characters that identify the AT command. */
	uint8_t atCommand[2];
	/** The HEX (non-ASCII) value to set the requested register to. */
	uint8_t value[GBEE_MAX_PAYLOAD_LENGTH];
} ;

/** Type definition for ::gbeeAtCommand. */
typedef struct gbeeAtCommand GBeeAtCommand;

/** Minimal length of AT command frame. */
#define GBEE_AT_COMMAND_HEADER_LENGTH (sizeof(GBeeAtCommand) - GBEE_MAX_PAYLOAD_LENGTH)

/**
 * This API type allows module parameters to be queried or set. In contrast to
 * the “AT command” API type, new parameter values are queued and not applied 
 * until either the “AT command” (0x08) API type or the AC (Apply Changes) 
 * command is issued. Register queries (reading parameter values) are returned 
 * immediately.
 */
PACKEDSTRUCT gbeeAtCommandQueue {
	/** API identifier: 0x09. */
	uint8_t ident;
	/**
	 * Identifies the UART data frame being reported. Note: If Frame ID = 0 in
	 * AT command mode, no AT command response will be given.
	 */
	uint8_t frameId;
	/** command Name - Two ASCII characters that identify the AT command. */
	uint8_t atCommand[2];
	/** The HEX (non-ASCII) value to set the requested register to. */
	uint8_t value[GBEE_MAX_PAYLOAD_LENGTH];
};

/** Type definition for ::gbeeAtCommandQueue. */
typedef struct gbeeAtCommandQueue GBeeAtCommandQueue;

/** Minimal length of AT command queue frame. */
#define GBEE_AT_COMMAND_QUEUE_HEADER_LENGTH (sizeof(GBeeAtCommandQueue) - GBEE_MAX_PAYLOAD_LENGTH)

/**
 * In response to an AT command message, the XBee will send an AT command 
 * response message. Some commands will send back multiple frames (for example,
 * the ND (Node Discover) and AS (Active Scan) commands). These commands will 
 * end by sending a frame with a status of ATCMD_OK and no data.
 */
PACKEDSTRUCT gbeeAtCommandResponse {
	/** API identifier: 0x88. */
	uint8_t ident;
	/**
	 * Identifies the UART data frame being reported. Note: If Frame ID = 0 in
	 * AT command mode, no AT command response will be given.
	 */
	uint8_t frameId;
	/** Command Name - Two ASCII characters that identify the AT command. */
	uint8_t atCommand[2];
	/** OK (0), ERROR (1), Invalid command (2), Invalid Parameter (3). */
	uint8_t status;
	/** The HEX (non -ASCII) value of the requested register. */
	uint8_t value[GBEE_MAX_PAYLOAD_LENGTH];
};

/** Type definition for ::gbeeAtCommandResponse. */
typedef struct gbeeAtCommandResponse GBeeAtCommandResponse;

/** Minimal length of AT command response frame. */
#define GBEE_AT_COMMAND_RESPONSE_HEADER_LENGTH (sizeof(GBeeAtCommandResponse) - GBEE_MAX_PAYLOAD_LENGTH)

/**
 * Allows for module parameter registers on a remote device to be queried or 
 * set.
 */
PACKEDSTRUCT gbeeRemoteAtCommand {
	/** API identifier: 0x17. */
	uint8_t ident;
	/**
	 * Identifies the UART data frame being reported. Note: If Frame ID = 0 in
	 * AT command mode, no AT command response will be given.
	 */
	uint8_t frameId;
	/**
	 * Set to match the 64-bit address of the destination, MSB first, LSB last. 
	 * Broadcast =0x000000000000FFFF. This field is ignored if the 16-bit 
	 * network address field equals anything other than 0xFFFE.
	 */
	uint32_t dstAddr64h;
	/**
	 * Set to match the 64-bit address of the destination, MSB first, LSB last. 
	 * Broadcast =0x000000000000FFFF. This field is ignored if the 16-bit 
	 * network address field equals anything other than 0xFFFE.
	 */
	uint32_t dstAddr64l;
	/**
	 * Set to match the 16-bit network address of the destination, MSB first, 
	 * LSB last. Set to 0xFFFE if 64-bit addressing is being used (MY=FFFE or 
	 * MY=FFFF).
	 */	
	uint16_t dstAddr16;
	/**
	 * 0x02 - Apply changes on remote. (If not set, AC command must be sent 
	 * before changes will take effect.) All other bits must be set to 0.
	 */	
	uint8_t cmdOpts;
	/** command Name - Two ASCII characters that identify the AT command. */
	uint8_t atCommand[2];
	/** The HEX (non -ASCII) value to set the register to. */
	uint8_t value[GBEE_MAX_PAYLOAD_LENGTH];
};

/** Type definition for ::gbeeAtCommandResponse. */
typedef struct gbeeRemoteAtCommand GBeeRemoteAtCommand;

/** Minimal length of remote AT command frame. */
#define GBEE_REMOTE_AT_COMMAND_HEADER_LENGTH (sizeof(GBeeRemoteAtCommand) - GBEE_MAX_PAYLOAD_LENGTH)

/**
 * If a module receives a remote command response RF data frame in response to
 * a Remote AT command Request, the module will send a Remote AT command 
 * response message out the UART. Some commands may send back multiple frames,
 * for example, Node Discover (ND) command.
 */
PACKEDSTRUCT gbeeRemoteAtCommandResponse {
	/** API identifier: 0x97. */
	uint8_t ident;
	/**
	 * Identifies the UART data frame being reported. Matches the Frame ID of 
	 * the Remote command Request the remote is responding to.
	 */
	uint8_t frameId;
	/**
	 * Indicates the 64-bit address of the remote module that is responding to
	 * the Remote AT command request.
	 */
	uint32_t srcAddr64h;
	/**
	 * Indicates the 64-bit address of the remote module that is responding to
	 * the Remote AT command request.
	 */
	uint32_t srcAddr64l;
	/** Set to the16- bit network address of the remote. */	
	uint16_t srcAddr16;
	/** command Name - Two ASCII characters that identify the AT command. */
	uint8_t atCommand[2];
	/** 
	 * OK (0), ERROR (1), Invalid command (2), Invalid Parameter (3),
	 * No response (4).
	 */
	uint8_t status;
	/** The HEX (non -ASCII) value read from the requested register. */
	uint8_t value[GBEE_MAX_PAYLOAD_LENGTH];
};

/** Type definition for ::gbeeAtCommandResponse. */
typedef struct gbeeRemoteAtCommandResponse GBeeRemoteAtCommandResponse;

/** Minimal length of remote AT command response frame. */
#define GBEE_REMOTE_AT_COMMAND_RESPONSE_HEADER_LENGTH (sizeof(GBeeRemoteAtCommandResponse) - GBEE_MAX_PAYLOAD_LENGTH)

/**
 * A TX Request message will cause the module to send RF data as an RF Packet.
 */
PACKEDSTRUCT gbeeTxRequest {
	/** API identifier: 0x10. */
	uint8_t ident;
	/**
	 * Identifies the UART data frame for the host to correlate with a 
	 * subsequent ACK (acknowledgement). Setting Frame ID to ‘0' will disable 
	 * response frame.
	 */
	uint8_t frameId;
	/** 
	 * Set to match the 64-bit address of the destination, MSB first, LSB 
	 * last. Broadcast =0x000000000000FFFF. This field is ignored if the 
	 * 16-bit network address field equals anything other than 0xFFFE.
	 */
	uint32_t dstAddr64h;
	/** 
	 * Set to match the 64-bit address of the destination, MSB first, LSB 
	 * last. Broadcast =0x000000000000FFFF. This field is ignored if the 
	 * 16-bit network address field equals anything other than 0xFFFE.
	 */
	uint32_t dstAddr64l;
	/**
	 * Set to the 16-bit address of the destination device, if known.
	 * Set to 0xFFFE if the address is unknown, or if sending a broadcast.
	 */
	uint16_t dstAddr16;
	/**
	 * Sets maximum number of hops a broadcast transmission can occur.
	 * If set to 0, the broadcast radius will be set to the maximum hops value.
	*/
	uint8_t bcastRadius;
	/**
	 * 0x01 = Disable ACK.
	 * 0x20 - Enable APS encryption (if EE=1)
	 * 0x04 = Send packet with Broadcast Pan ID.
	 * All other bits must be set to 0.
	 */	
	uint8_t options;
	/** Up to GBEE_MAX_PAYLOAD_LENGTH Bytes per packet. */
	uint8_t data[GBEE_MAX_PAYLOAD_LENGTH];
};

/** Type definition for ::gbeeTxRequest. */
typedef struct gbeeTxRequest GBeeTxRequest;

/** Minimal length of transmit request */
#define GBEE_TX_REQUEST_HEADER_LENGTH (sizeof(GBeeTxRequest) - GBEE_MAX_PAYLOAD_LENGTH)

/**
 * When a TX Request is completed, the module sends a TX status message. This 
 * message will indicate if the packet was transmitted successfully or if 
 * there was a failure. This is the data structure that's compatible with
 * the latest version of the protocol
 */
PACKEDSTRUCT gbeeTxStatusNew {
	/** API identifier: 0x8B. */
	uint8_t ident;
	/**
	 * Identifies UART data frame being reported. Note: If Frame ID = 0 in the 
	 * TX Request, no AT command response will be given.
	 */
	uint8_t frameId;
	/** MSB (most significant byte) first, LSB (least significant) last. */
	uint16_t srcAddr16;
	/** Number of application transmission retries that took place. */
	uint8_t retryCnt;
	/**
	 * 0 = Success, 0x01 = No ACK received, 0x02 = CCA failure, 
	 * 0x15 = Invalid destination, 0x21 = Netwock ACK failure,
	 * 0x22 = Not joined to Nw, 0x23 = Self-addressed
	 * 0x24 = Address Not Found, 0x25 = Route Not Found,
	 * 0x26 = Broadcast source failed to hear a neighbor relay the message,
	 * 0x2B = Invalid binding table index,
	 * 0x2C = Resource error lack of free buffers, timers, etc.,
	 * 0x2D = Attempted broadcast with APS transmission,
	 * 0x2E = Attempted unicast with APS transmission, but EE=0,
	 * 0x32 = Resource error lack of free buffers, timers, etc.,
	 * 0x74 = Data payload too large, 0x75 = Indirect message unrequested
	 */
	uint8_t deliveryStatus;
	/**
	 * 0x00 = No Discovery Overhead
	 * 0x01 = Address Discovery
	 * 0x02 = Route Discovery
	 * 0x03 = Address and Route
	 * 0x40 = Extended Timeout
	 */
	 uint8_t discoveryStatus;
};

/** Type definition for ::gbeeTxStatusNew. */
typedef struct gbeeTxStatusNew GBeeTxStatusNew;

/**
 * When the module receives an RF packet, it is sent out the UART using this
 * message type.
 */
PACKEDSTRUCT gbeeRxPacket {
	/** API identifier: 0x90. */
	uint8_t ident;
	/** MSB (most significant byte) first, LSB (least significant) last. */
	uint32_t srcAddr64h;
	/** MSB (most significant byte) first, LSB (least significant) last. */
	uint32_t srcAddr64l;
	/** MSB (most significant byte) first, LSB (least significant) last. */
	uint16_t srcAddr16;
	/**
	 * bit 0 [reserved]
	 * bit 1 = Package acknowledged (0x01)
	 * bit 2 = Package was broadcasted (0x02)
	 * bit 5 = Package encrypted with APS (0x20)
	 * bit 6 = Package sent from end device (0x40)
	 */
	uint8_t options;
	/** Up to GBEE_MAX_PAYLOAD_LENGTH Bytes per packet. */
	uint8_t data[GBEE_MAX_PAYLOAD_LENGTH];
};

/** Type definition for ::gbeeRxPacket. */
typedef struct gbeeRxPacket GBeeRxPacket;

/** Minimal length of receive packet with 64bit address. */
#define GBEE_RX_PACKET_HEADER_LENGTH (sizeof(GBeeRxPacket) - GBEE_MAX_PAYLOAD_LENGTH)

/**
 * Base-type for all XBee API frame data types.
 */
union gbeeFrameData {
	/** API identifier (= type of frame). */
	uint8_t ident;
	/** Modem status frame. */
	GBeeModemStatus modemStatus;
	/** AT command frame. */
	GBeeAtCommand atCommand;
	/** AT command Queue frame. */
	GBeeAtCommandQueue atCommandQueue;
	/** XBee's response to an AT command frame. */
	GBeeAtCommandResponse atCommandResponse;
	/** Remote AT command frame. */
	GBeeRemoteAtCommand remoteAtCommand;
	/** Remote GBee's AT command response frame. */
	GBeeRemoteAtCommandResponse remoteAtCommandResponse;
	/** data Tx Request frame using 64-and 16-bit addressing (ZB Protocol). */
	GBeeTxRequest txRequest;
	/** Tx Transmit Status frame (ZB Protocol)*/
	GBeeTxStatusNew txStatusNew;
	/** Rx data using 64-and 16-bit addressing (ZB Protocol) */
	GBeeRxPacket rxPacket;
};

/** Type definition for ::gbeeFrameData. */
typedef union gbeeFrameData GBeeFrameData;

/**
 * This is the header of all frames.
 */
PACKEDSTRUCT gbeeFrameHeader {
	/** Frame start delimiter, always 0x7E. */
	uint8_t startDelimiter;
	/** Length of frame data in bytes (MSB|LSB). */
	uint16_t length;
};

/** Type definition for ::gbeeFrameHeader. */
typedef struct gbeeFrameHeader GBeeFrameHeader;

/**
 * This is the trailer of all frames.
 */
PACKEDSTRUCT gbeeFrameTrailer {
	/** checksum over frame data. */
	uint8_t checksum;
};

/** Type definition for gbeeFrameTrailer. */
typedef struct gbeeFrameTrailer GBeeFrameTrailer;

/**
 * This is a complete XBee frame, including header, data, and trailer.
 */
PACKEDSTRUCT gbeeFrame {
	/** Frame header. */
	GBeeFrameHeader header;
	/** Frame data. */
	GBeeFrameData data;
	/** Frame trailer. */
	GBeeFrameTrailer trailer;
};

/** Type definition for ::gbeeFrame. */
typedef struct gbeeFrame GBeeFrame;

/** XBee modem status API identifier. */
#define GBEE_MODEM_STATUS               0x8A
/** XBee AT command API identifier. */
#define GBEE_AT_COMMAND                 0x08
/** XBee AT command queue API identifier. */
#define GBEE_AT_COMMAND_QUEUE           0x09
/** XBee AT command response API identifier. */
#define GBEE_AT_COMMAND_RESPONSE        0x88
/** XBee remote AT command API identifier. */
#define GBEE_REMOTE_AT_COMMAND          0x17
/** XBee remote AT command response API identifier. */
#define GBEE_REMOTE_AT_COMMAND_RESPONSE 0x97
/** XBee transmit request API identifier. */
#define GBEE_TX_REQUEST                 0x10
/** XBee transmission status API identifier. */
#define GBEE_TX_STATUS                  0x89
#define GBEE_TX_STATUS_NEW              0x8B
/** XBee receive packet API identifier. */
#define GBEE_RX_PACKET                  0x90

/** AT command response indicating success. */
#define GBEE_AT_COMMAND_STATUS_OK              0
/** AT command response indicating error. */
#define GBEE_AT_COMMAND_STATUS_ERROR           1
/** AT command response indicating invalid command. */
#define GBEE_AT_COMMAND_STATUS_INVALID_COMMAND 2
/** AT command response indicating invalid command parameter. */
#define GBEE_AT_COMMAND_STATUS_INVALID_PARAM   3
/** AT command response indicating no response. */
#define GBEE_AT_COMMAND_STATUS_NO_RESPONSE     4

/** Transmit request ACK disabled. */
#define GBEE_TX_DISABLE_ACK   0x01
/** Transmit request APS enabled. */
#define GBEE_TX_ENABLE_APS    0x02
/** Transmit request broadcast PAN. */
#define GBEE_TX_BROADCAST_PAN 0x04

/** Transmission status indicating success. */
#define GBEE_TX_STATUS_SUCCESS     0
/** Transmission status indicating no ACK received from remote XBee. */
#define GBEE_TX_STATUS_NO_ACK      1
/** Transmission status indicating clear channel assessment failure. */
#define GBEE_TX_STATUS_CCA_FAILURE 2
/** Transmission status indicating purge of transmit FIFO. */
#define GBEE_TX_STATUS_PURGED      3

/** Received packet with broadcast address. */
#define GBEE_RX_BROADCAST_ADDR 0x02
/** Received packet with broadcast PAN. */
#define GBEE_RX_BROADCAST_PAN  0x04
/** Received packet acknowledged. */
#define GBEE_RX_ACKNOWLEDGED 0x01
/** Received APS encrypted packet (new protocol). */
#define GBEE_RX_ENCRYPTED 0x02
/** Received packet from end device (new protocol). */
#define GBEE_RX_END_DEVICE 0x04

/**
 * This is the XBee device driver object returned by the gbeeCreate function.
 */
struct gbee {
	/** Serial device descriptor, returned by GBEE_PORT_SERIAL_OPEN. */
	int serialDevice;
	/** Scratch pad - used for preparing data transfered to/from XBee. */
	uint8_t scratch[GBEE_TOTAL_FRAME_SIZE];
	/** Last error that occurred. */
	GBeeError lastError;
};

/** Type definition for ::gbee. */
typedef struct gbee GBee;

/**
 * Creates a new XBee device. Opens the UART interface the XBee is connected
 * to by calling the GBee port.
 * 
 * \param[in] serialName is the name of the interface (e.g. "/dev/ttyS0"). See
 * the specific GBee port you are using for more information on this parameter.
 * 
 * \return A pointer to the GBee device if successful, or NULL in case of any
 * error.
 */
GBee* gbeeCreate(const char* serialName);

/**
 * Read a frame from the XBee and check its validity. This operation calls
 * the serial interface receive operation provided by the port to access the
 * XBee. This operation requires the XBee to be in API mode.
 * 
 * \param[in] self is a pointer to the XBee device to read from.
 * \param[out] frameData points to memory where to store received frame.
 * \param[out] length is the length of the received frame in bytes.
 * \param[in,out] timeout specifies a timeout in milliseconds, GBEE_NO_WAIT will
 * cause, the operation to return immediately, GBEE_INFINITE_WAIT sets an
 * infinite timeout. Upon completion of the function, timeout will be set to
 * the remaining time.
 * 
 * \retval GBEE_NO_ERROR to indicate success.
 * \retval GBEE_INHERITED_ERROR to indicate that the call failed due to an
 * error in an earlier libgbee call.
 * \retval GBEE_TIMEOUT_ERROR to indicate that the timeout expired without any
 * data was received.
 * \retval GBEE_FRAME_INTEGRITY_ERROR to indicate that an incomplete frame was
 * received from the XBee.
 * \retval GBEE_FRAME_SIZE_ERROR to indicate that the size of the frame
 * received from the XBee exceeds the maximum allowed frame size.
 * \retval GBEE_CHECKSUM_ERROR to indicate that the checksum failed for the
 * received frame.
 */
GBeeError gbeeReceive(GBee *self, GBeeFrameData *frameData, uint16_t *length, 
		uint32_t *timeout);

/**
 * Send a frame to the XBee. This operation calls the serial interface send
 * operation provided by the port to access the XBee. This operation requires
 * the XBee to be in API mode.
 * 
 * \param[in] self is a pointer to the XBee device to write to.
 * \param[in] frameData is a pointer to the frame data to write.
 * \param[in] dataLength is the size in bytes of the frame data.
 * 
 * \retval GBEE_NO_ERROR to indicate success.
 * \retval GBEE_INHERITED_ERROR to indicate that the call failed due to an
 * error in an earlier libgbee call.
 * \retval GBEE_FRAME_SIZE_ERROR to indicate that dataLength exceeds the
 * maximum allowed frame size.
 * \retval GBEE_RS232_ERROR to indicate a failure to establish serial
 * communication with the XBee.
 */
GBeeError gbeeSend(GBee *self, GBeeFrameData *frameData, uint16_t dataLength);

/**
 * Creates an AT command frame and sends it to the GBee. This operation calls
 * the serial interface send operation provided by the port to access the XBee.
 * This operation requires the XBee to be in API mode.
 * 
 * \param[in] self is a pointer to the XBee device.
 * \param[in] frameId will be used to identify command response, set to 0 to
 * suppress response.
 * \param[in] atCmd is the 2-byte AT command to send.
 * \param[in] value is the AT command value.
 * \param[in] length is the length of the AT command value.
 * 
 * \retval GBEE_NO_ERROR to indicate success.
 * \retval GBEE_INHERITED_ERROR to indicate that the call failed due to an
 * error in an earlier libgbee call.
 * \retval GBEE_FRAME_SIZE_ERROR to indicate that dataLength exceeds the
 * maximum allowed frame size.
 * \retval GBEE_RS232_ERROR to indicate a failure to establish serial
 * communication with the XBee.
 */
GBeeError gbeeSendAtCommand(GBee *self, uint8_t frameId, uint8_t *atCmd, 
		uint8_t *value, uint16_t length);

/**
 * Creates an AT command Queue frame and sends it to the GBee. This operation
 * calls the serial interface send operation provided by the port to access the
 * XBee. This operation requires the XBee to be in API mode.
 * 
 * \param[in] self is a pointer to the XBee device.
 * \param[in] frameId is the 8-bit frame identifier.
 * \param[in] atCmd is the 2-byte AT command to send.
 * \param[in] value is the AT command value.
 * \param[in] length is the length of the AT command value.
 * 
 * \retval GBEE_NO_ERROR to indicate success.
 * \retval GBEE_INHERITED_ERROR to indicate that the call failed due to an
 * error in an earlier libgbee call.
 * \retval GBEE_FRAME_SIZE_ERROR to indicate that dataLength exceeds the
 * maximum allowed frame size.
 * \retval GBEE_RS232_ERROR to indicate a failure to establish serial
 * communication with the XBee.
 */
GBeeError gbeeSendAtCommandQueue(GBee *self, uint8_t frameId, uint8_t *atCmd,
		uint8_t *value, uint16_t length);

/**
 * Creates a Remote AT command frame and sends it to the GBee. This operation
 * calls the serial interface send operation provided by the port to access the
 * XBee. This operation requires the XBee to be in API mode.
 * 
 * \param[in] self is a pointer to the XBee device to write to.
 * \param[in] frameId is the 8-bit frame identifier.
 * \param[in] dstAddr64h is the upper half of the 64 bit destination address.
 * \param[in] dstAddr64l is the lower half of the 64 bit destination address.
 * \param[in] dstAddr16 is the 16-bit destination address.
 * \param[in] atCmd is the 2-byte AT command to send.
 * \param[in] cmdOpts contains the Remote AT command flags.
 * \param[in] value is the AT command argument.
 * \param[in] length is the length of the AT command argument.
 * 
 * \retval GBEE_NO_ERROR to indicate success.
 * \retval GBEE_INHERITED_ERROR to indicate that the call failed due to an
 * error in an earlier libgbee call.
 * \retval GBEE_FRAME_SIZE_ERROR to indicate that dataLength exceeds the
 * maximum allowed frame size.
 * \retval GBEE_RS232_ERROR to indicate a failure to establish serial
 * communication with the XBee.
 */
GBeeError gbeeSendRemoteAtCommand(GBee *self, uint8_t frameId, uint32_t dstAddr64h, 
		uint32_t dstAddr64l, uint16_t dstAddr16, uint8_t *atCmd, uint8_t  cmdOpts,
		uint8_t *value, uint16_t length);

/**
 * Creates a Tx Request frame using 64-bit and optional 16-bit addressing 
 * and sends it to the GBee. This operation calls the serial interface send 
 * operation provided by the port to access the XBee. This operation 
 * requires the XBee to be in API mode.
 * 
 * \param[in] self is a pointer to the XBee device.
 * \param[in] frameId is the 8-bit frame identifier.
 * \param[in] dstAddr64h is the upper half of the 64 bit destination address.
 * \param[in] dstAddr64l is the lower half of the 64 bit destination address.
 * \param[in] dstAddr16 is the 16 bit destination address.
 * \param[in] bcastRadius is the maximum number of hops for a broadcast transmission.
 * \param[in] options contains the transmission flags.
 * \param[in] data is the data to send.
 * \param[in] length is the length of the data.
 * 
 * \retval GBEE_NO_ERROR to indicate success.
 * \retval GBEE_INHERITED_ERROR to indicate that the call failed due to an
 * error in an earlier libgbee call.
 * \retval GBEE_FRAME_SIZE_ERROR to indicate that dataLength exceeds the
 * maximum allowed frame size.
 * \retval GBEE_RS232_ERROR to indicate a failure to establish serial
 * communication with the XBee.
 */
GBeeError gbeeSendTxRequest(GBee *self, uint8_t frameId, uint32_t dstAddr64h, 
		uint32_t dstAddr64l, uint16_t dstAddr16, uint8_t bcastRadius,
		uint8_t options, uint8_t *data, uint16_t length);

/**
 * Closes the serial interface the XBee is connected to by calling the close
 * operation provided by the port.
 *
 * \param[in] self is a pointer to the XBee device to "destroy".
 */
void gbeeDestroy(GBee *self);

#endif /* GBEE_H_INCLUDED */

#ifdef __cplusplus
}
#endif
