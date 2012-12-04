#ifndef __FATFS_H
#define __FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ff.h"

DWORD get_fattime(void);
uint64_t FATFS_speedTest(uint32_t kilobytesToWrite, bool keepTestFile = false);
bool FATFS_isFilesystemAvailable();
bool FATFS_testFilesystem();
bool FATFS_initializeFilesystem();
void FATFS_deinitializeFilesystem();

#ifdef __cplusplus
}
#endif

#endif // __FATFS_H