#include <stdint.h>
#include <stdbool.h>
#include "efm32.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "rtc.h"
#include "alarm.h"
#include "debug_output_control.h"

// TODO function semantics not really clear after adding non-oneshot timers,
// adjust function names and improve documentation!

// TODO disable TIMER0 interrupt while changing the shared variables!
// TODO add debug statements for the code using module_debug_alarm()
// TODO add function descriptions

// TODO set RTC trigger period to minimum of all current active alarms

#define MAX_ALARMS      5

/**************************************************************************
 Local variables for the module
***************************************************************************/

// keep an integer for each alarm, representing how many period expirations
// there are remaining, or -1 if the alarm is inactive
int16_t alarmTimeouts[MAX_ALARMS], alarmPeriods[MAX_ALARMS];
// handler function for each alarm
AlarmEventHandler alarmHandlers[MAX_ALARMS];
// number of active alarms
unsigned char alarmCount;
unsigned char alarmTimerActive;

/**************************************************************************
 Internal/helper functions for the module
***************************************************************************/

void _Alarm_CheckStartStopNeeded()
{
  // make sure we start the alarm timer if this is the first active alarm
  if(alarmTimerActive == 0 && alarmCount != 0)
  {
    module_debug_alarm("Starting alarm timer");
    
    RTC_Trigger(500, &_Alarm_RTCCallback);
    
    alarmTimerActive = 1;
    
    return; 
  } else if(alarmTimerActive == 1 && alarmCount != 0) {
    // just re-trigger the RTC
    RTC_Trigger(2000, &_Alarm_RTCCallback);
  } else if(alarmTimerActive == 1 && alarmCount == 0)
  {
    module_debug_alarm("Stopping alarm timer");
    // RTC is only activated by re-triggering so nothing to do here?
    alarmTimerActive = 0;
    
    return;
  }
}

void _Alarm_RTCCallback(void)
{ 
 
  // decrement alarm timeouts, signal those who have expired
  for(int i = 0; i < MAX_ALARMS; i++)
  {
    if(alarmTimeouts[i] > 0)
      alarmTimeouts[i]--;
    
    if(alarmTimeouts[i] == 0)
    {
      module_debug_alarm("%d triggered!", i);
      
      // trigger alarm
      (alarmHandlers[i])(i);
      
      // re-set timeout to period, period will be -1 for one-shots
      alarmTimeouts[i] = alarmPeriods[i];
      
      // deactivate alarm if one-shot mode was selected
      if(alarmPeriods[i] == -1)
      {
        alarmHandlers[i] = 0;
        alarmCount--;
      } 
      
    }
  }
  
  // check timer status as there may be no active alarms left
  _Alarm_CheckStartStopNeeded();
}

/**************************************************************************
 External interface functions for the module
***************************************************************************/

void Alarm_InitializeSystem()
{
  // initialize the data structures and variables we use for alarms
  for(int i=0; i < MAX_ALARMS; i++)
  {
    alarmTimeouts[i] = alarmPeriods[i] -1;
    alarmHandlers[i] = 0;
  }
  alarmCount = 0;
  alarmTimerActive = 0;
  
  module_debug_alarm("Initialized");
}

int Alarm_Create(int timeout_count, bool oneshot, AlarmEventHandler handler)
{
  // first, check if we have room for more alarms
  if(alarmCount >= MAX_ALARMS)
    return -1;
  
  // check the validity of given creation parameters
  if(timeout_count <= 0 || handler == 0)
    return -1;
  
  // find an empty slot in the alarms array
  for(int i = 0; i < MAX_ALARMS; i++)
  {
    if(alarmTimeouts[i] == -1)
    {
      // we can use this slot for the alarm
      alarmTimeouts[i] = timeout_count; // TODO compensate for current period pos!
      alarmHandlers[i] = handler;
      alarmPeriods[i] = (oneshot ? -1 : timeout_count);
      alarmCount++;
      
      module_debug_alarm("Created with id %d and timeout %d, period %d", i, timeout_count, alarmPeriods[i]);
      
      // may be necessary to start the timer here
      _Alarm_CheckStartStopNeeded();
      
      // return the slot number as alarm ID
      return i;
    }
  }
  
  // if we fall through the loop that means no alarm slots are available
  // return -1 to indicate failure
  return -1;
}

/**************************************************************************//**
 * @brief Alarm_SetTimeout
 * Set timeout for given alarm or disable it
 * id - alarm identifier returned from Alarm_Create
 * timeout_count - how many periods the alarm will expire after, 0 to disable
 *****************************************************************************/
void Alarm_SetTimeout(int id, int timeout_count)
{
  // check to see if this is a valid alarm
  if(id < 0 || id >= MAX_ALARMS)
    return;
  
  if(alarmTimeouts[id] == -1 || timeout_count < 0)
    return;
  
  // set the given alarm count to given number
  if(timeout_count != 0)
    alarmTimeouts[id] = timeout_count;
  else 
  {
    // timeout count of 0 stops the alarm without triggering it
    alarmTimeouts[id] = -1;
    alarmPeriods[id] = -1;
    alarmHandlers[id] = 0;
    alarmCount--;
  }
  
  _Alarm_CheckStartStopNeeded();
}