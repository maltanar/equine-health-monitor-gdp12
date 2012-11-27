#include <stdio.h>

// University of Southampton, 2012
// EMECS Group Design Project

// This file enables or disables the module debug outputs via printf
// from different modules

#ifndef DEBUGOUTPUTCONTROL_H
#define DEBUGOUTPUTCONTROL_H

// ************************************************************************
// Use the statement below to disable ALL debug outputs, takes priority
// over sections below
// ************************************************************************

#define nDISABLE_ALL_DEBUG_OUTPUTS

// ************************************************************************
// Use the section below to enable / disable debug outputs for each module
// Example:
// #define ENABLE_DEBUG_OUTPUT_ALARM - to enable debug output from Alarm module
// #define nENABLE_DEBUG_OUTPUT_ALARM - to disable debug output from Alarm md.
// ************************************************************************

#ifndef DISABLE_ALL_DEBUG_OUTPUTS

#define nENABLE_DEBUG_OUTPUT_ALARM
#define nENABLE_DEBUG_OUTPUT_ANTIF
#define nENABLE_DEBUG_OUTPUT_UART
#define nENABLE_DEBUG_OUTPUT_HRMRX
#define nENABLE_DEBUG_OUTPUT_TEMP
#define nENABLE_DEBUG_OUTPUT_I2C
#define nENABLE_DEBUG_OUTPUT_XBEE

#endif

// ************************************************************************
// Implementations for debug functions
// No need to make changes here, modify statements above instead
// ************************************************************************

#ifdef ENABLE_DEBUG_OUTPUT_ALARM
#define module_debug_alarm(fmt, ...)   printf("ALARM: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_alarm(fmt, ...)
#endif

#ifdef ENABLE_DEBUG_OUTPUT_ANTIF
#define module_debug_antif(fmt, ...)   printf("ANTIF: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_antif(fmt, ...)
#endif

#ifdef ENABLE_DEBUG_OUTPUT_UART
#define module_debug_uart(fmt, ...)   printf("UART: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_uart(fmt, ...)
#endif

#ifdef ENABLE_DEBUG_OUTPUT_HRMRX
#define module_debug_hrmrx(fmt, ...)   printf("HRMRX: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_hrmrx(fmt, ...)
#endif

#ifdef ENABLE_DEBUG_OUTPUT_TEMP
#define module_debug_temp(fmt, ...)   printf("TEMP: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_temp(fmt, ...)
#endif

#ifdef ENABLE_DEBUG_OUTPUT_I2C
#define module_debug_i2c(fmt, ...)   printf("I2C: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_i2c(fmt, ...)
#endif

#ifdef ENABLE_DEBUG_OUTPUT_XBEE
#define module_debug_xbee(fmt, ...)   printf("XBee: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_xbee(fmt, ...)
#endif

#endif // DEBUGOUTPUTCONTROL_H