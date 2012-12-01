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
 * Simple queue implementation for receiving data from the XBee.
 */

#include "gbee-usart.h"
#include <string.h>
#include <stdio.h>
#include "usartmanager.h"

UARTPort * port = NULL;
static uint8_t msgbuf[80];

/******************************************************************************/

int gbeeUsartEnable(const char *deviceName)
{
  port = (UARTPort *) USARTManager::getInstance()->getPort(XBEE_USART_PORT);
  port->initialize(msgbuf, 80, UARTPort::uartPortBaudRate9600,
                   UARTPort::uartPortDataBits8, UARTPort::uartPortParityNone,
                   UARTPort::uartPortStopBits1);

	return 0;
}

/******************************************************************************/

void gbeeUsartDisable(int deviceIndex)
{
  // TODO we are ignoring deviceIndex for now
}

/******************************************************************************/

void gbeeUsartBufferPut(int deviceIndex, const uint8_t *buffer, uint16_t length)
{
  int i = 0;
  for(i = 0; i < length; i++)
    port->writeChar(buffer[i]);
}

/******************************************************************************/

bool gbeeUsartByteGet(int deviceIndex, uint8_t *byte)
{
  // TODO ignoring deviceIndex
  int ch = port->readChar();
  if(ch == -1)
    return false;
  else {
    *byte = (uint8_t) ch;
    return true;
  }
}

/******************************************************************************/

