#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"{
#endif

#ifndef ALARM_H
#define ALARM_H

#define ALARM_TICK_MS           100

// type definition for functions that will handle the alarm timeout
typedef void(*AlarmEventHandler)(int id);

// interface functions for the alarm module
void Alarm_InitializeSystem();
int Alarm_Create(int timeout_count, bool oneshot, AlarmEventHandler handler);
void Alarm_SetTimeout(int id, int timeout_count);

// internal helpers/functions for the alarm module
void _Alarm_CheckStartStopNeeded();
void _Alarm_RTCCallback();

#endif  // ALARM_H

#ifdef __cplusplus
}
#endif