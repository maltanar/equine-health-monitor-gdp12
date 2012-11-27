#include "alarmmanager.h"
#include "debug_output_control.h"

AlarmManager::AlarmManager()
{
  m_tickMs = ALARM_TICK_DEFAULT_MS;
}
