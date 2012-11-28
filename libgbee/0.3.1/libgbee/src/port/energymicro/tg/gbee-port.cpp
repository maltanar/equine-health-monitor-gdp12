/**
 * \file
 * \author  d264
 * \version $Rev$
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
 *
 * \section DESCRIPTION
 *
 * This is a port of the GBee driver for embedded systems based on a SAM7
 * microcontroller. This port uses the AT91LIB for interrupt and USART
 * handling.
 *
 * For more information on the AT91LIB, please refer to http://www.at91.com.
 */

#include "gbee-port-interface.h"
#include "alarmmanager.h"
#include <stdbool.h>

/******************************************************************************/

/* callback function for the alarm handler that takes care of setting the
 * global variable "timeout_loop" that's used to control the maximal wait
 * time for receiving a Byte from the serial port */
static volatile bool usartReceiveTimout = false;
void gbeeUsartReceiveTimeout(AlarmID id)
{
	usartReceiveTimout = false;
}

/******************************************************************************/

GBeeError gbeePortUsartSendBuffer(int deviceIndex, const uint8_t *buffer,
		uint16_t length)
{
	gbeeUsartBufferPut(deviceIndex, buffer, length);
	return GBEE_NO_ERROR;
}

/******************************************************************************/

GBeeError gbeePortUsartReceiveByte(int deviceIndex, uint8_t *byte, uint32_t timeout)
{
	AlarmID alarmId;

	usartReceiveTimout = true;
	/* create an alarm that's triggered after 200ms (ALARM_TICK_MS * 1) */
	alarmId = AlarmManager::getInstance()->createAlarm(2, true, &gbeeUsartReceiveTimeout);
	/* Wait until there is a byte available, or timeout elapsed */
	do
	{
		if (gbeeUsartByteGet(deviceIndex, byte))
		{
			/* disable the alarm */
			AlarmManager::getInstance()->stopAlarm(alarmId);
			return GBEE_NO_ERROR;
		}
	}
	while (usartReceiveTimout || (timeout == GBEE_INFINITE_WAIT));
	return GBEE_TIMEOUT_ERROR;
}
