// ************************************************************************
// Implementations for debug functions
// No need to make changes here, modify statements in debug_output_control
// ************************************************************************

extern char mdMessageBuffer[255];

void md_printf(int len);

#ifdef ENABLE_DEBUG_OUTPUT_ALARM
	#define module_debug_alarm(fmt, ...)   printf("ALRM: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_ALARM)
	#define module_debug_alarm(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "ALRM: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_alarm(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_ANT
	#define module_debug_ant(fmt, ...)   printf("ANT: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_ANT)
	#define module_debug_ant(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "ANT: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_ant(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_UART
	#define module_debug_uart(fmt, ...)   printf("UART: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_UART)
	#define module_debug_uart(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "UART: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_uart(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_LEUART
	#define module_debug_leuart(fmt, ...)   printf("LEUART: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_LEUART)
	#define module_debug_leuart(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "LEUART: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_leuart(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_TEMP
	#define module_debug_temp(fmt, ...)   printf("TEMP: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_TEMP)
	#define module_debug_temp(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "TEMP: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_temp(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_I2C
	#define module_debug_i2c(fmt, ...)   printf("I2C: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_I2C)
	#define module_debug_i2c(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "I2C: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_i2c(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_GPS
	#define module_debug_gps(fmt, ...)   printf("GPS: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_GPS)
	#define module_debug_gps(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "GPS: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_gps(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_ACCL
	#define module_debug_accl(fmt, ...)   printf("ACCL: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_ACCL)
	#define module_debug_accl(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "ACCL: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_accl(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_XBEE
	#define module_debug_xbee(fmt, ...)   printf("XBEE: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_XBEE)
	#define module_debug_xbee(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "XBEE: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_xbee(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_FATFS
	#define module_debug_fatfs(fmt, ...)   printf("FATFS: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_FATFS)
	#define module_debug_fatfs(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "FATFS: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_fatfs(fmt, ...)   
#endif


#ifdef ENABLE_DEBUG_OUTPUT_STRG
	#define module_debug_strg(fmt, ...)   printf("STRG: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_STRG)
	#define module_debug_strg(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "STRG: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_strg(fmt, ...)   
#endif


#ifdef ENABLE_DEBUG_OUTPUT_SPI
	#define module_debug_spi(fmt, ...)   printf("SPI: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_STRG)
	#define module_debug_spi(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "SPI: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_spi(fmt, ...)   
#endif

#ifdef ENABLE_DEBUG_OUTPUT_AUDIO
	#define module_debug_audio(fmt, ...)   printf("AUD: "fmt"\n", ##__VA_ARGS__)
#elif defined(ZB_ENABLE_DEBUG_OUTPUT_AUDIO)
	#define module_debug_audio(fmt, ...)   md_printf(sprintf(mdMessageBuffer, "AUD: "fmt"\n", ##__VA_ARGS__))
#else
	#define module_debug_audio(fmt, ...)   
#endif