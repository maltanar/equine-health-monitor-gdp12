#ifndef __ALARM_H
#define __ALARM_H

#include <stdint.h>
#include <stdbool.h>

// Alarm class
// create and manage periodic and singleshot alarms, designed to work down to
// EM2 for low power sleep while still maintaining timekeeping

#define ALARM_TICK_DEFAULT_MS           1000

// type definition for functions that will handle the alarm timeout
typedef void(*AlarmEventHandler)(int id);

class AlarmManager {
public:
  // singleton instance accessor
  static AlarmManager* getInstance()
  {
    static AlarmManager instance;
    return &instance;
  }

private:
  // ------ start of singleton pattern specific section ------
  AlarmManager();  
  AlarmManager(AlarmManager const&);               // do not implement
  void operator=(AlarmManager const&);        // do not implement
  // ------ end of singleton pattern specific section --------  
  
  uint16_t m_tickMs;
    
};

/*

// TODO add class members and functions corresponding to old C-style funcs:


void Alarm_InitializeSystem();
int Alarm_Create(int timeout_count, bool oneshot, AlarmEventHandler handler);
void Alarm_SetTimeout(int id, int timeout_count);

void _Alarm_CheckStartStopNeeded();
void _Alarm_RTCCallback();
*/

#endif  // __ALARM_H