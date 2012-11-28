#include "accelerationsensor.h"
#include "i2cbus.h"
#include "debug_output_control.h"

AccelerationSensor::AccelerationSensor(SensorPeriod period)	:
  Sensor(typeAccelerometer, 6, period)
{
	m_ofsX = m_ofsY = m_ofsZ = 0;
	
  	// sanity check by device ID control
	uint8_t deviceID = getDeviceID();

	if(deviceID != ADXL350_DEVICE_ID)
	{
		module_debug_accl("unknown device id %x", deviceID);
		return;
	}
	else
		module_debug_accl("device id OK: %x", deviceID);

	// default configuration for the accelerometer:
	// disable interrupts
	writeRegister( ADXL350_REG_INTEN, 0x00); 
	// bypass FIFO
	setFIFOMode(fifoModeBypass);
	// data format register, self test bit is reset
	// range bits are set to 11, justify bit 0
	// TODO decide how much range vs precision we need
	writeRegister( ADXL350_REG_DATAF, 0x03); 
	setPeriod(period);
	// wake up device and enable measurements
	setSleepState(false);
	
	// read the offset values for each axis
	readRegister(ADXL350_REG_OFSX, (uint8_t *) &m_ofsX);
	readRegister(ADXL350_REG_OFSY, (uint8_t *) &m_ofsY);
	readRegister(ADXL350_REG_OFSZ, (uint8_t *) &m_ofsZ);
	
	module_debug_accl("offsets %d %d %d", m_ofsX, m_ofsY, m_ofsZ);
}
 
char AccelerationSensor::setSleepState(bool sleepState)
{
	module_debug_accl("sleep state=%d", sleepState);
	
	if(sleepState)
	{
		// disable measurements and put device to sleep
		// wakeup bits set to 11 for lowest rate possible
		writeRegister( ADXL350_REG_PWCTL, 0x07);
	} else
	{
		// recommended wakeup procedure from the datasheet:
		// first bring device out of sleep by clearing sleep bit
		writeRegister( ADXL350_REG_PWCTL, 0x00); 
		// now re-enable measurements
		writeRegister( ADXL350_REG_PWCTL, 0x08); 
	}
	
	return 1;
}

void AccelerationSensor::setPeriod(SensorPeriod ms)
{
	Sensor::setPeriod(ms);
	// low power mode is set, data output rate 3.125 Hz
	// TODO determine and write correct rate value!
	writeRegister( ADXL350_REG_BWRAT, 0x16); 
}
  
// read the acceleration data from registers & store in memory
void AccelerationSensor::sampleSensorData()
{
  	uint8_t aH, aL;
	
	// x-axis
	if(readRegister(ADXL350_REG_XMSB, &aH) && readRegister(ADXL350_REG_XLSB, &aL))
	  m_accMsg.x = (aH << 8) | aL;
	else
	  module_debug_accl("failed to read x accel");
	
	// y-axis
	if(readRegister(ADXL350_REG_YMSB, &aH) && readRegister(ADXL350_REG_YLSB, &aL))
	  m_accMsg.y = (aH << 8) | aL;
	else
	  module_debug_accl("failed to read y accel");
	
	// z-axis
	if(readRegister(ADXL350_REG_ZMSB, &aH) && readRegister(ADXL350_REG_ZLSB, &aL))
	  m_accMsg.z = (aH << 8) | aL;
	else
	  module_debug_accl("failed to read z accel");
	
	module_debug_accl("sampled x %d y %d z %d", m_accMsg.x, m_accMsg.y, m_accMsg.z);
}

const void* AccelerationSensor::readSensorData(uint16_t *actualSize)
{
  // TODO implement
  return NULL;
}
 
uint8_t AccelerationSensor::getDeviceID()
{
  // read the value of the ADXL350 device ID register (address 0x00)
  uint8_t ret = 0;
  
  if(!readRegister(ADXL350_REG_DEVID, &ret))
	module_debug_accl("failed to read device id!");
  
  return ret;
}


// change how the device FIFO operates
bool AccelerationSensor::setFIFOMode(AccelerationSensorFIFOMode mode)
{
	// TODO at the moment we are paying no attention to the samples bits,
	// as we are not using interrupts / FIFO
	return writeRegister(ADXL350_REG_FIFOC, (uint8_t) mode);
}

// internal helper - read given register into given buffer 
// in case of bus failure, retry up to ADXL350_I2C_RETRYCNT times
// return true if succeeds, false if read operation fails
bool AccelerationSensor::readRegister(uint8_t reg, uint8_t * val)
{
  uint8_t retryCount = ADXL350_I2C_RETRYCNT;
  
  while(retryCount-- > 0)
  {
	if(I2CBus::getInstance()->readRegister8Bit(ADXL350_DEVICE_ADDR, reg, val))
	  return true;
	
	// TODO add a small sleep delay between retries?
  }
  
  return false;
}

// internal helper - write given value into given register
// in case of bus failure, retry up to ADXL350_I2C_RETRYCNT times
// return true if succeeds, false if write operation fails
bool AccelerationSensor::writeRegister(uint8_t reg, uint8_t val)
{
  uint8_t retryCount = ADXL350_I2C_RETRYCNT;
  
  while(retryCount-- > 0)
  {
	if(I2CBus::getInstance()->writeRegister8Bit(ADXL350_DEVICE_ADDR, reg, val))
	  return true;
	
	// TODO add a small sleep delay between retries?
  }
  
  return false;
}

