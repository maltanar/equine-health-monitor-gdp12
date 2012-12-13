#include <stdbool.h>

#ifndef __MCUSETUP_H
#define	__MCUSETUP_H

#ifdef __cplusplus
extern "C" {
#endif

void initializeMCU(bool setupDebug, bool debugWhileSleep);
	
#ifdef __cplusplus
}
#endif

#endif	// __MCUSETUP_H