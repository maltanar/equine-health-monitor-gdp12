#include "em_emu.h"
#include "anthrmsensor.h"
#include "debug_output_control.h"


ANTHRMSensor::ANTHRMSensor() :
  Sensor(typeHeartRate, 8, ANTHRM_DEFAULT_RATE)
{
    // initialize UART
    // TODO UARTManagerPortLEUART0 should be defined in part-specific config!
    m_port = UARTManager::getInstance()->getPort(UARTManagerPortLEUART0);

    m_port->initialize(m_msgBuffer, ANTHRM_MSGBUFFER_SIZE, 
                     UARTPort::uartPortBaudRate4800, 
                     UARTPort::uartPortDataBits8, UARTPort::uartPortParityNone, 
                     UARTPort::uartPortStopBits1);
    
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


void ANTHRMSensor::processANTMessage(uint8_t * buffer)
{

}

void ANTHRMSensor::sendANTMessage(char *data)
{

}
