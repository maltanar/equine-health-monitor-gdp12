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

typedef enum 
{
	sleepModeEM0,
	sleepModeEM1,
	sleepModeEM2
} DelaySleepMode;


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

	// pause mode for AlarmManager: does not stop the tick counter, merely
	// prevents alarms from counting down and triggering, Unix timekeeping still
	// enabled
	bool isPaused();
	void pause();
	void resume();
	
	void lowPowerDelay(uint16_t ms, DelaySleepMode mode = sleepModeEM0);

	void setUnixTime(uint32_t secondsSinceEpoch);
	uint32_t getUnixTime();
	uint16_t getMsCounter();
	uint64_t getMsTimestamp();

	// RTC callback will need access to tick(), so declare as friend
	friend void RTC_IRQHandler(void);

private:
	// ------ start of singleton pattern specific section ------
	AlarmManager();  
	AlarmManager(AlarmManager const&);               // do not implement
	void operator=(AlarmManager const&);        // do not implement
	// ------ end of singleton pattern specific section --------  

	uint16_t m_tickMs;				// number of miliseconds for an alarm tick
	uint8_t m_activeAlarmCount;		// number of active alarms
	bool m_isPaused;				// prevent alarms from counting down
	AlarmSlot m_alarms[MAX_ALARMS];
	bool m_delayWait;				// poll flag for the low-power delay
	uint32_t m_unixTime;				// Unix time, number of seconds since
									// the Unix epoch

	void tick();						// handle alarm tick event
	void rtcSetup();			// setup RTC	

};

#endif  // __ALARM_H