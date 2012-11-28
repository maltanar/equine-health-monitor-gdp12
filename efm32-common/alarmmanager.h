#ifndef __ALARM_H
#define __ALARM_H

#include <stdint.h>
#include <stdbool.h>

// Alarm class
// create and manage periodic and singleshot alarms, designed to work down to
// EM2 for low power sleep while still maintaining timekeeping

#define ALARM_TICK_DEFAULT_MS           1000
#define	MAX_ALARMS						8
#define ALARM_INVALID_ID				MAX_ALARMS


typedef uint8_t AlarmID;

// type definition for functions that will handle the alarm timeout
typedef void(*AlarmEventHandler)(AlarmID id);

typedef struct {
	bool active;				// is the alarm in this slot active?
	uint8_t counter;			// the countdown field for the alarm
	uint8_t period;				// 0 if one shot, counter refill value otherwise
	AlarmEventHandler handler;	// the event handler that will be executed
								// upon alarm timeout
} AlarmSlot;

class AlarmManager {
public:
  // singleton instance accessor
  static AlarmManager* getInstance()
  {
    static AlarmManager instance;
    return &instance;
  }
  
  AlarmID createAlarm(uint8_t timeoutCount, bool oneShot, AlarmEventHandler handler);
  void stopAlarm(AlarmID alarmID);
  void setAlarmTimeout(AlarmID alarmID, uint8_t timeoutCount);
  
  // RTC callback will need access to tick(), so declare as friend
  friend void AlarmManager_RTCCallback();

private:
  // ------ start of singleton pattern specific section ------
  AlarmManager();  
  AlarmManager(AlarmManager const&);               // do not implement
  void operator=(AlarmManager const&);        // do not implement
  // ------ end of singleton pattern specific section --------  
  
  uint16_t m_tickMs;				// number of miliseconds for an alarm tick
  uint8_t m_activeAlarmCount;		// number of active alarms
  bool m_alarmTickActive;			// whether the alarm module tick is currently active
  									// (requires at least one active alarm)
  AlarmSlot m_alarms[MAX_ALARMS];
  
  void checkStartStopNeeded();		// do we need to stop or start the RTC?
  void tick();						// handle alarm tick event
    
};

#endif  // __ALARM_H