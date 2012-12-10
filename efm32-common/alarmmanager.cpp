#include <stddef.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_rtc.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_rtc.h"
#include "alarmmanager.h"
#include "debug_output_control.h"

#define RTC_FREQ    32768               // RTC clock at 32.768 kHz
	 
void RTC_IRQHandler(void)
{
	// retrieve the RTC interrupt flags
	uint32_t rtcIF = RTC_IntGet();
	
	// clear the interrupt sources
	RTC_IntClear(rtcIF);
	
	if(rtcIF & RTC_IF_COMP0)
		AlarmManager::getInstance()->tick();
	
	if(rtcIF & RTC_IF_COMP1) 
	{
		// Enable interrupt on COMP0
  		RTC_IntDisable(RTC_IF_COMP1);
		AlarmManager::getInstance()->m_delayWait = false;
	}
}

void AlarmManager::rtcSetup(void)
{
  RTC_Init_TypeDef init;

  // Ensure LE modules are accessible
  CMU_ClockEnable(cmuClock_CORELE, true);

  // Enable clock to RTC module
  CMU_ClockEnable(cmuClock_RTC, true);

  init.enable   = false;
  init.debugRun = false;
  init.comp0Top = true; /* Count to cc0 before wrapping */
  RTC_Init(&init);

  // Disable interrupt generation from RTC
  RTC_IntDisable(_RTC_IF_MASK);
  
  // Enable interrupts
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);

  // Clear interrupt source
  RTC_IntClear(RTC_IF_COMP0);

  // Calculate trigger value in ticks based on 32768Hz clock
  RTC_CompareSet(0, (RTC_FREQ * m_tickMs) / 1000);

  // Enable RTC
  RTC_Enable(true);

  // Enable interrupt on COMP0
  RTC_IntEnable(RTC_IF_COMP0);
  RTC_IntDisable(RTC_IF_COMP1);
}

void AlarmManager::lowPowerDelay(uint16_t ms, DelaySleepMode mode)
{
	if(m_delayWait)
	{
		module_debug_alarm("LP delay already active!");
		return;
	}
	
	if(ms >= m_tickMs)
	{
		module_debug_alarm("low power delay cannot be smaller than tick period!");
		return;
	}
	
	// do comparisons / calculations in terms of RTC ticks to use less
	// operations
	uint32_t current = RTC_CounterGet();
	uint32_t delayBase = (RTC_FREQ * ms) / 1000;
	uint32_t period = (RTC_FREQ * m_tickMs) / 1000;
	
	RTC_IntClear(RTC_IF_COMP1);
	
	// use RTC COMP1 to set up interrupt at end of delay
	if(current + delayBase <= period)
		RTC_CompareSet(1, current + delayBase);
	else
		RTC_CompareSet(1, delayBase - (period - current));
	
	// Enable interrupt on COMP1
  	RTC_IntEnable(RTC_IF_COMP1);
	
	// set delay wait flag to true, will be set to false inside interrupt
	m_delayWait = true;
		
	while(m_delayWait)
		switch(mode)
		{
		case sleepModeEM0:
			// EM0 sleep is no sleep
			break;
		case sleepModeEM1:
			// enter EM1
			EMU_EnterEM1();
			break;
		case sleepModeEM2:
			// enter EM2, restore clocks on wakeup
			EMU_EnterEM2(true);
			break;
		}
}

AlarmManager::AlarmManager()
{
	m_unixTime = 0;
	m_tickMs = ALARM_TICK_DEFAULT_MS;
	m_delayWait = false;
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

	// setup the RTC to generate alarm ticks
	rtcSetup();
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
	module_debug_alarm("could not create alarm!");
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
	return (RTC_CounterGet() * 1000) / RTC_FREQ;
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
