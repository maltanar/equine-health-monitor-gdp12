#include "alarmmanager.h"
#include "rtc.h"
#include "debug_output_control.h"

// ISR for RTC callback event - provides the alarm tick
void AlarmManager_RTCCallback()
{
	AlarmManager::getInstance()->tick();
}

AlarmManager::AlarmManager()
{
	m_unixTime = 0;
	m_tickMs = ALARM_TICK_DEFAULT_MS;
	m_activeAlarmCount = 0;
	m_isPaused = true;	// AlarmManager is paused on creation

	// initialize the RTC and the structures
	for(int i = 0; i < MAX_ALARMS; i++)
	{
		m_alarms[i].active = false;
		m_alarms[i].counter = 0;
		m_alarms[i].period = 0;
		m_alarms[i].handler = NULL;
	}

	// trigger the RTC
	RTC_Trigger(m_tickMs, &AlarmManager_RTCCallback);
	module_debug_alarm("initialized");
}

// tick handler - decrement alarm countdowns and execute the handlers for the
// triggered ones
void AlarmManager::tick()
{	
	// update the Unix time field
	m_unixTime += m_tickMs / 1000;
	module_debug_alarm("unix time: %d", m_unixTime);
	
	if(!m_isPaused)	// pausing prevents alarm counters from counting down
	{
		for(int i = 0; i < MAX_ALARMS; i++)
		{
			if(m_alarms[i].active)
			{
				m_alarms[i].counter--;

				if(m_alarms[i].counter == 0)
				{
					module_debug_alarm("%d triggered!", i);

					// call handler for triggered alarm
					(m_alarms[i].handler)(i);

					if(m_alarms[i].period == 0)
					{
						// deactivate alarm if one-shot mode was selected
						m_alarms[i].active = false;
						m_alarms[i].handler = NULL;
						m_activeAlarmCount--;
					} else {
						// re-set timeout to period, period will be 0 for one-shots
						m_alarms[i].counter = m_alarms[i].period;
					}
				}
			}
		}
	}
	
	// re-trigger the RTC
	RTC_Trigger(m_tickMs, &AlarmManager_RTCCallback);
}


AlarmID AlarmManager::createAlarm(uint8_t timeoutCount, bool oneShot, 
								  AlarmEventHandler handler)
{
	// first, check if we have room for more alarms
	if(m_activeAlarmCount >= MAX_ALARMS)
		return ALARM_INVALID_ID;
  
	// check the validity of given creation parameters
	if(timeoutCount <= 0 || handler == 0)
		return ALARM_INVALID_ID;
  
	// find an empty slot in the alarms array
	for(int i = 0; i < MAX_ALARMS; i++)
	{
		if(m_alarms[i].active == false)
		{
		  // we can use this free slot for the alarm
		  m_alarms[i].counter = timeoutCount; // TODO compensate for current period pos!
		  m_alarms[i].handler = handler;
		  m_alarms[i].period = (oneShot ? 0 : timeoutCount);
		  m_alarms[i].active = true;
		  m_activeAlarmCount++;
		  
		  module_debug_alarm("Created with id %d and timeout %d, period %d", i, 
							 timeoutCount, m_alarms[i].period);
		  
		  // return the slot number as alarm ID
		  return i;
		}
	}
  
  // if we fall through the loop that means no alarm slots are available
  // return a specific value to indicate failure
  return ALARM_INVALID_ID;
}

void AlarmManager::stopAlarm(AlarmID alarmID)
{
	// check to see if this is a valid alarm
	if(alarmID >= MAX_ALARMS || m_alarms[alarmID].active == false)
		return;
	
	m_alarms[alarmID].active = false;
	m_alarms[alarmID].handler = NULL;
	m_activeAlarmCount--;
	
}

void AlarmManager::setAlarmTimeout(AlarmID alarmID, uint8_t timeoutCount)
{
	// check to see if this is a valid alarm
	if(alarmID >= MAX_ALARMS || m_alarms[alarmID].active == false)
		return;
	
	// set the alarm counter to given number
	if(timeoutCount != 0)
		m_alarms[alarmID].counter = timeoutCount;
	else
		stopAlarm(alarmID);
}

void AlarmManager::setUnixTime(uint32_t secondsSinceEpoch)
{
	module_debug_alarm("settings unix time to %d", secondsSinceEpoch);
	m_unixTime = secondsSinceEpoch;
}

uint32_t AlarmManager::getUnixTime()
{
	return m_unixTime;
}

uint16_t AlarmManager::getMsCounter()
{
	return RTC_CounterGetMs();
}

void AlarmManager::pause()
{
	m_isPaused = true;
	module_debug_alarm("paused");
}

void AlarmManager::resume()
{
	m_isPaused = false;	
	module_debug_alarm("resumed");
}

uint64_t AlarmManager::getMsTimestamp()
{
	return m_tickMs * m_unixTime + getMsCounter();
}
