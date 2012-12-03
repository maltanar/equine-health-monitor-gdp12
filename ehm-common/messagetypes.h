// University of Southampton, 2012
// EMECS Group Design Project

// Structs definitions for messages that will be exchanged between the
// monitoring device and the base station are stored here
// This is to ensure message content is consistent across both devices
// while serializing and deserializing raw data

#ifndef __MESSAGETYPES_H
#define __MESSAGETYPES_H

#include <stdint.h>
#include <stdbool.h>

// IAR CC uses a different keyword for packed structures
#if defined ( __ICCARM__ )
  #define PACKEDSTRUCT __packed struct
#else
  #define PACKEDSTRUCT struct __attribute__((__packed__))
#endif

typedef enum {
	msgSensorData,
	msgSensorConfig,
	msgDebug
} MessageType;

typedef enum  {
	typeGPS,
	typeAccelerometer,
	typeRawTemperature,
	typeHeartRate,
	typeZigBee,
	typeMonitoringDevice
} DeviceType;

// definition of the data structure at the highest abstraction level.
// This structure is used to transmit data between monitoring
// devices and the base station
typedef PACKEDSTRUCT {
	MessageType mainType;
	uint32_t relTimestampS;
	uint8_t *payload;
} MessagePacket;

// definition of the three main message groups: Sensor, Config and Debug
// They contain a subtype field for specialization
typedef PACKEDSTRUCT {
	DeviceType sensorType;
	uint32_t endTimestampS;		// Unix-style timestamp for the latest sample in this message
	uint16_t sampleIntervalMs;
	uint8_t arrayLength;	// could also be calculated during de-serialization
	uint8_t *sensorMsgArray;
} SensorMessage;

typedef PACKEDSTRUCT {
	DeviceType deviceType;
	bool isReadRequest;
	uint8_t arrayLength;
	uint8_t *configMsgArray;
} ConfigMessage;

typedef PACKEDSTRUCT {
	uint32_t timestampS;
	uint8_t *debugData;
} DebugMessage;

// definition of subtypes for ConfigMessages
typedef PACKEDSTRUCT {
	DeviceType sensorType;
	uint8_t enableSensor;
	uint16_t sampleIntervalMs;
	uint16_t samplePeriodMs;
} ConfigSensor;

// definition of subtypes for SensorMessage
typedef PACKEDSTRUCT {
	uint8_t degree;
	uint8_t minute;
	uint8_t second;
} Coordinate;

typedef PACKEDSTRUCT {
	Coordinate latitude;
	bool latitudeNorth;
	Coordinate longitude;
	bool longitudeWest;
	bool validPosFix;
} GPSMessage;

typedef PACKEDSTRUCT {
	int16_t x;
	int16_t y;
	int16_t z;
} AccelerometerMessage;

typedef PACKEDSTRUCT {
	double Vobj;
	double Tenv;
} RawTemperatureMessage;

typedef PACKEDSTRUCT {
	uint8_t bpm;
} HeartRateMessage;

#endif	// __MESSAGETYPES_H
