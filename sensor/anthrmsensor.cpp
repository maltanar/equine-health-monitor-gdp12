#include "em_emu.h"
#include "anthrmsensor.h"
#include "debug_output_control.h"

ANTHRMSensor::ANTHRMSensor() :
  Sensor(typeHeartRate, 8, 4000)
{

}

char ANTHRMSensor::setSleepState(bool sleepState)
{
    return 0;
}

void ANTHRMSensor::setPeriod(SensorPeriod ms)
{

}

void ANTHRMSensor::sampleSensorData()
{

}

const void* ANTHRMSensor::readSensorData(uint16_t *actualSize)
{
    return NULL;
}


void ANTHRMSensor::processANTMessage(uint8_t * buffer)
{

}

void ANTHRMSensor::sendANTMessage(char *data)
{

}
