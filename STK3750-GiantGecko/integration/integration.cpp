/**************************************************************************//**
 * @file
 * @brief Simple LED Blink Demo for EFM32GG_DK3750
 * @author Energy Micro AS
 * @version 2.0.3
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "dvk.h"
#include "trace.h"
#include "gbee.h"
#include "gbee-util.h"
#include "xbee_if.h"
#include "uartmanager.h"
#include "gpssensor.h"
#include "rtc.h"
#include "accelerationsensor.h"
#include "temperaturesensor.h"

XBee_Message* get_message(XBee* interface, const string &dest, uint16_t size) {
	const XBee_Address *addr = interface->xbee_get_address(dest);
	uint8_t *payload = new uint8_t[size];

	if (!addr) {
		printf("Error getting address for node %s \n", dest.c_str());
		return NULL;
	}
	for (int i = 0; i < size; i++) {
		payload[i] = (uint8_t)i % 255;
	}
	XBee_Message* test_msg = new XBee_Message(*addr, payload, size);
	delete[] payload;

	return test_msg;
}

void sfHook(uint8_t *)
{
	GPSSensor::getInstance()->sampleSensorData();
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	/* Configure board. Select either EBI or SPI mode. */
	DVK_init(DVK_Init_SPI);
	TRACE_SWOSetup();

	/* If first word of user data page is non-zero, enable eA Profiler trace */
	// TODO disabled as it may be taking over some of the PD pins we want
	// needed for eA profiler?
	// TRACE_ProfilerSetup();

	// set configuration options for XBee device
	uint8_t pan_id[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xBC, 0xCD};
	XBee_Config config("", "denver", false, pan_id, 1000, B9600, 1);

	GPIO_PinModeSet(gpioPortD, 11, gpioModePushPull, 1);

	printf("rest starting \n");

	// hardware reset the ZigBee module
    // set !RST to low for 10 ms
	GPIO_PinOutClear(gpioPortD, 11);
   	RTC_Delay(10);
   	GPIO_PinOutSet(gpioPortD, 11);

	// wait for device to settle down after reset
    RTC_Delay(1000);

	printf("rest finished \n");


	// initialize XBee device
	XBee interface(config);
	uint8_t error_code = interface.xbee_init();
	if (error_code != GBEE_NO_ERROR) {
		printf("Error: unable to configure device, code: %02x\n", error_code);
		return 0;
	}
	while(interface.xbee_status() != 0);

	XBee_At_Command m("OI");
	interface.xbee_send_at_command(m);
	printf("%d \n", m.length);
	printf("%x \n", *(uint16_t *)m.data);

	UARTManager::getInstance()->getPort(UARTManagerPortLEUART0)->setSignalFrameHook(&sfHook);
	GPSSensor::getInstance();
	/*AccelerationSensor *a = new AccelerationSensor(100);
	TemperatureSensor *t = new TemperatureSensor(1000);

	printf("accelerometer devid: %x \n", a->getDeviceID());
	printf("tempsens ids %x %x \n", t->getManufacturerID(),
		   t->getDeviceID());*/



	while (1)
	{
		XBee_Message *x = get_message(&interface, "coordinator", 10);
		interface.xbee_send_data(*x);
		delete x;
		//EMU_EnterEM2(true);
		//a->sampleSensorData();
		//t->sampleSensorData();

	}

}
