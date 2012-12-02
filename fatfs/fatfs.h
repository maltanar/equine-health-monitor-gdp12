#ifndef __FATFS_H
#define __FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ff.h"

DWORD get_fattime(void);
bool isFilesystemAvailable();
bool testFilesystem();
void initializeFilesystem();

#ifdef __cplusplus
}
#endif

#endif // __FATFS_H