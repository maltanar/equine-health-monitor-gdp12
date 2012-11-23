/**************************************************************************//**
 * @file
 * @brief Simple LED Blink Demo for EFM32TG_STK3300
 * @author Energy Micro AS
 * @version 3.0.2
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
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"
#include "rtc.h"
#include "trace.h"
#include "gbee.h"
#include "gbee-util.h"
#include "xbee_if.h"

#define GBEE_UTIL_DEFAULT_FRAME_ID 0xCF

volatile uint32_t msTicks; /* counts 1ms timeTicks */

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}

/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}

void setupSWO(void)
{
  uint32_t *dwt_ctrl = (uint32_t *) 0xE0001000;
  uint32_t *tpiu_prescaler = (uint32_t *) 0xE0040010;
  uint32_t *tpiu_protocol = (uint32_t *) 0xE00400F0;

  CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;
  /* Enable Serial wire output pin */
  GPIO->ROUTE |= GPIO_ROUTE_SWOPEN;
#if defined(_EFM32_GIANT_FAMILY)
  /* Set location 0 */
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC0;

  /* Enable output on pin - GPIO Port F, Pin 2 */
  GPIO->P[5].MODEL &= ~(_GPIO_P_MODEL_MODE2_MASK);
  GPIO->P[5].MODEL |= GPIO_P_MODEL_MODE2_PUSHPULL;
#else
  /* Set location 1 */
  GPIO->ROUTE = (GPIO->ROUTE & ~(_GPIO_ROUTE_SWLOCATION_MASK)) | GPIO_ROUTE_SWLOCATION_LOC1;
  /* Enable output on pin */
  GPIO->P[2].MODEH &= ~(_GPIO_P_MODEH_MODE15_MASK);
  GPIO->P[2].MODEH |= GPIO_P_MODEH_MODE15_PUSHPULL;
#endif
  /* Enable debug clock AUXHFRCO */
  CMU->OSCENCMD = CMU_OSCENCMD_AUXHFRCOEN;

  while(!(CMU->STATUS & CMU_STATUS_AUXHFRCORDY));

  /* Enable trace in core debug */
  CoreDebug->DHCSR |= 1;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* Enable PC and IRQ sampling output */
  *dwt_ctrl = 0x400113FF;
  /* Set TPIU prescaler to 16. */
  *tpiu_prescaler = 0xf;
  /* Set protocol to NRZ */
  *tpiu_protocol = 2;
  /* Unlock ITM and output data */
  ITM->LAR = 0xC5ACCE55;
  ITM->TCR = 0x10009;
}

GBeeError gbeeUtilReadRegister(GBee *gbee, const char *regName, uint8_t *value,
                uint16_t *length, uint16_t maxLength)
{
        /* Error code returned by the GBee driver. */
        GBeeError error;
        /* GBee response to AT commands. */
        GBeeAtCommandResponse atCommandResponse;
        /* Length of AT command response. */
        uint16_t responseLength;
        /* Timeout in milliseconds. */
        uint32_t timeout;

        /* Query the GBee for the given register. */
        error = gbeeSendAtCommand(gbee, GBEE_UTIL_DEFAULT_FRAME_ID, (uint8_t *)regName,
                        NULL, 0);
        GBEE_THROW(error);

        /* Receive the response. */
        timeout = 1000;
        do
        {
                error = gbeeReceive(gbee, (GBeeFrameData *)&atCommandResponse, &responseLength,
                                &timeout);
        }
        while ((error == GBEE_NO_ERROR)
                        && (atCommandResponse.ident != GBEE_AT_COMMAND_RESPONSE));
        GBEE_THROW(error);

        responseLength -= GBEE_AT_COMMAND_RESPONSE_HEADER_LENGTH;

        /* Check the response. */
        if ((atCommandResponse.atCommand[0] != regName[0])
                        || (atCommandResponse.atCommand[1] != regName[1])
                        || (atCommandResponse.status != GBEE_AT_COMMAND_STATUS_OK)
                        || (maxLength < responseLength))
        {
                GBEE_THROW(GBEE_RESPONSE_ERROR);
        }

        GBEE_PORT_MEMORY_COPY(value, atCommandResponse.value, responseLength);
        return GBEE_NO_ERROR;
}

XBee_Message* get_message(uint16_t size) {
	uint8_t *payload = new uint8_t[size];

	for (int i = 0; i < size; i++) {
		payload[i] = (uint8_t)i % 255;
	}
	XBee_Message* test_msg = new XBee_Message(messageTypeRawTemperature, payload, size);
	delete[] payload;

	return test_msg;
}

void speed_measurement(XBee* interface, uint16_t size, uint16_t iterations) {
	uint32_t start_time, end_time, duration_ms;
	uint8_t error_code;
	XBee_Message* test_msg = get_message(size);

	if (!test_msg) {
		printf("Memory allocation problem\n");
		return;
	}

	start_time = msTicks;
	for (uint16_t i = 0; i < iterations; i++) {
		test_msg = get_message(size);
		if ((error_code = interface->xbee_send_to_coordinator(*test_msg))!= 0x00) {
			printf("Error transmitting: %u\n", error_code);
			break;
		}
		printf("Successfully transmitted msg %u with type %02x\n", i+1, (uint8_t)test_msg->get_type());
	}

	end_time = msTicks;
	duration_ms = end_time - start_time;

	printf("Elapsed time: %ld milliseconds\n", duration_ms);
	if (error_code == 0x00)
		printf("Data throughput: %.1f kB/s\n", (iterations * size) / (float) duration_ms);

	delete test_msg;
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
    /* Chip errata */
    CHIP_Init();

    setupSWO();

	/* Setup SysTick Timer for 1 msec interrupts  */
  //	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

    // XBee assumed to be connected on USART1 with PD0 and PD1
    // use PD2 for reset
    GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 1);

    // hardware reset the ZigBee module
    // set !RST to low for 10 ms

	GPIO_PinOutClear(gpioPortD, 2);
    RTC_Trigger(10, NULL);
    EMU_EnterEM2(true);
   	GPIO_PinOutSet(gpioPortD, 2);

    // wait for device to settle down after reset
    RTC_Trigger(1000, NULL);
    EMU_EnterEM2(true);
    // TODO maybe wait for HW reset ACK from device instead of fixed time wait

    // set configuration options for XBee device
    uint8_t pan_id[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xBC, 0xCD};
    XBee_Config config("", "denver", false, pan_id, 1000, B9600, 1);

	// initialize XBee device
    XBee interface(config);
    uint8_t error_code = interface.xbee_init();
    if (error_code != GBEE_NO_ERROR) {
		printf("Error: unable to configure device, code: %02x\n", error_code);
		return 0;
    }
    interface.xbee_status();

	// test the speed of the connection
	speed_measurement(&interface, 10, 20);
 }


