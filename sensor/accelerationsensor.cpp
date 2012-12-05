#include "accelerationsensor.h"
#include "i2cbus.h"
#include "debug_output_control.h"

#define ACCL_DEFAULT_PERIOD	100

AccelerationSensor::AccelerationSensor()	:
  Sensor(typeAccelerometer, sizeof(AccelerometerMessage), ACCL_DEFAULT_PERIOD)
{
	m_sensorMessage.arrayLength = 0;
	m_sensorMessage.sensorMsgArray = (uint8_t *) m_accSampleBuffer;
	
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
	setPeriod(ACCL_DEFAULT_PERIOD);
	// wake up device and enable measurements
	setSleepState(false);
	
	// read the offset values for each axis
	/*readRegister(ADXL350_REG_OFSX, (uint8_t *) &m_ofsX);
	readRegister(ADXL350_REG_OFSY, (uint8_t *) &m_ofsY);
	readRegister(ADXL350_REG_OFSZ, (uint8_t *) &m_ofsZ);
	// TODO these offset values need to be calibrated - useless to read them
	// like this
	*/
}
 
char AccelerationSensor::setSleepState(bool sleepState)
{
	module_debug_accl("sleep state=%d", sleepState);
	
	if(sleepState)
	{
		// disable measurements and put device to sleep
		// wakeup bits set to 11 for lowest rate possible
		writeRegister( ADXL350_REG_PWCTL, 0x07); // 0b0000 0111
	} else
	{
		// recommended wakeup procedure from the datasheet:
		// first bring device out of sleep by clearing sleep bit
		writeRegister( ADXL350_REG_PWCTL, 0x00); // 0b0000 0000
		// now re-enable measurements
		writeRegister( ADXL350_REG_PWCTL, 0x08); // 0b0000 1000
	}
	
	return 1;
}

void AccelerationSensor::setPeriod(SensorPeriod ms)
{
	Sensor::setPeriod(ms);
	// TODO determine and write correct rate value!
	writeRegister( ADXL350_REG_BWRAT, 0x1A); // 0b0001 1010
}
  
// read the acceleration data from registers & store in memory
void AccelerationSensor::sampleSensorData()
{
	// discard old samples if buffer was full
	if(m_sensorMessage.arrayLength == ACCL_MAX_SAMPLES)
		discardOldSamples();
	
  	uint8_t aH, aL;
	AccelerometerMessage accMsg;
	
	// x-axis
	if(readRegister(ADXL350_REG_XMSB, &aH) && readRegister(ADXL350_REG_XLSB, &aL))
	  accMsg.x = (aH << 8) | aL;
	else
	  module_debug_accl("failed to read x accel");
	
	// y-axis
	if(readRegister(ADXL350_REG_YMSB, &aH) && readRegister(ADXL350_REG_YLSB, &aL))
	  accMsg.y = (aH << 8) | aL;
	else
	  module_debug_accl("failed to read y accel");
	
	// z-axis
	if(readRegister(ADXL350_REG_ZMSB, &aH) && readRegister(ADXL350_REG_ZLSB, &aL))
	  accMsg.z = (aH << 8) | aL;
	else
	  module_debug_accl("failed to read z accel");
	
	// write into buffer and increment the sample count
	m_accSampleBuffer[m_sensorMessage.arrayLength++] = accMsg;
	
	module_debug_accl("sampled x %d y %d z %d", accMsg.x, accMsg.y, accMsg.z);
}

void AccelerationSensor::discardOldSamples()
{
	module_debug_accl("discarding old samples");
	m_sensorMessage.arrayLength = 0;
}

const void* AccelerationSensor::readSensorData(uint16_t *actualSize)
{
	// call parent implementation
	const void* ret = Sensor::readSensorData(actualSize);
	  	
	// if we had not reached the full sample cycle, discard the old ones
	// this is because we cannot afford a cyclic buffer and memory reordering
	if(m_sensorMessage.arrayLength != ACCL_MAX_SAMPLES)
		discardOldSamples();
	
	return ret;
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

