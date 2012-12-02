// ************************************************************************
// Implementations for debug functions
// No need to make changes here, modify statements in debug_output_control
// ************************************************************************

#ifdef ENABLE_DEBUG_OUTPUT_ALARM
#define module_debug_alarm(fmt, ...)   printf("ALARM: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_alarm(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_ANT
#define module_debug_ant(fmt, ...)   printf("ANT: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_ant(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_UART
#define module_debug_uart(fmt, ...)   printf("UART: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_uart(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_LEUART
#define module_debug_leuart(fmt, ...)   printf("LEUART: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_leuart(fmt, ...)   
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

#ifdef ENABLE_DEBUG_OUTPUT_GPS
#define module_debug_gps(fmt, ...)   printf("GPS: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_gps(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_ACCL
#define module_debug_accl(fmt, ...)   printf("ACCL: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_accl(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_XBEE
#define module_debug_xbee(fmt, ...)   printf("XBee: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_xbee(fmt, ...)
#endif

#ifdef ENABLE_DEBUG_OUTPUT_FATFS
#define module_debug_fatfs(fmt, ...)   printf("FATFS: "fmt"\n", ##__VA_ARGS__)
#else
#define module_debug_fatfs(fmt, ...)
#endif

