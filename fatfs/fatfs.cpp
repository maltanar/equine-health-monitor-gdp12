#include "fatfs.h"
#include "microsd.h"
#include "diskio.h"
#include "alarmmanager.h"

#include "debug_output_control.h"

FATFS Fatfs;
bool m_fsAvailable = false;              // have we successfully mounted the FS?

/***************************************************************************//**
 * @brief
 *   This function is required by the FAT file system in order to provide
 *   timestamps for created files. Since we do not have a reliable clock we
 *   hardcode a value here.
 *
 *   Refer to drivers/fatfs/doc/en/fattime.html for the format of this DWORD.
 * @return
 *    A DWORD containing the current time and date as a packed datastructure.
 ******************************************************************************/
DWORD get_fattime(void)
{
	// TODO connect this to the AlarmManager's unix time
  return (28 << 25) | (2 << 21) | (1 << 16);
}

bool FATFS_isFilesystemAvailable()
{
	return m_fsAvailable;
}

uint64_t FATFS_speedTest(uint32_t kilobytesToWrite, bool keepTestFile)
{
	const char *filename = "stst.raw";
	uint32_t data = 0xDEADBEEF;
	UINT bw;
	uint32_t bwTotal = 0;
	FIL logfile;
	FRESULT fr;
	
	// remove possible existing test file
	fr = f_unlink(filename);
	
	// open the file in write + create always mode
	fr = f_open(&logfile, filename, FA_WRITE | FA_CREATE_ALWAYS);
	if(fr != FR_OK) {
		module_debug_fatfs("f_open failed! %x", fr);
		return 0;
	}
	// get start timestamp
	uint64_t start = AlarmManager::getInstance()->getMsTimestamp();
	// write the number of desired kilobytes
	for(int i = 0; i < kilobytesToWrite; i++)
		for(int j = 0; j < 256; j++)
		{
			bw = 0;
			fr = f_write(&logfile, &data, 4, &bw);
			if(fr != FR_OK || bw != 4)
			{
				module_debug_fatfs("f_write failed! %x %d", fr, bw);
				goto done;
			}
			bwTotal += bw;
		}
  done:
	// get end timestamp
	uint64_t end = AlarmManager::getInstance()->getMsTimestamp();
	fr = f_sync(&logfile);
	fr = f_close(&logfile);
	if(!keepTestFile)
		fr = f_unlink(filename);
	// subtract start from end to get elapsed time
	end -= start;
	
	module_debug_fatfs("took %lld ms", end);
	module_debug_fatfs("bytes written %d ", bwTotal);
	module_debug_fatfs("bw: %f b/s", (float)bwTotal / ((float)(end)/1000));
	
	return end;
}

// test whether the filesystem works by writing some data to a file and reading
// it back
bool FATFS_testFilesystem()
{
	FRESULT res;
	FIL fsrc;
	UINT bytesProcessed = 0;
	char * testFileName = "mnoprs.txt";
	char * testFileData = "12345678901234567890";
	char readBuffer[21];
	
	// open the file for writing, with create flags
	res = f_open(&fsrc, testFileName,  FA_WRITE | FA_CREATE_ALWAYS); 
	if (res != FR_OK)
	{
		module_debug_fatfs("f_open failed for write step, %x", res);
		return false;
	}

	// set the file pointer to first location
	res = f_lseek(&fsrc, 0);
	if (res != FR_OK) 
	{
		module_debug_fatfs("f_lseek failed for write step, %x", res);
		f_close(&fsrc);
		return false;
	}

	// write a buffer to file
	res = f_write(&fsrc, testFileData, 21, &bytesProcessed);
	if ((res != FR_OK) || (21 != bytesProcessed)) 
	{
		module_debug_fatfs("f_write failed, %x, written %d of 21", res,
						   bytesProcessed);
		f_close(&fsrc);
		return false;
	}

	// close the file
	res = f_close(&fsrc);
	if (res != FR_OK) 
	{
		module_debug_fatfs("f_close failed for write step, %x", res);
		return false;
	}

	// reopen the file for reading
	res = f_open(&fsrc, testFileName,  FA_READ); 
	if (res != FR_OK)
	{
		module_debug_fatfs("f_open failed for read step, %x", res);
		return false;
	}

	// set the file pointer to first location
	res = f_lseek(&fsrc, 0);
	if (res != FR_OK) 
	{
		module_debug_fatfs("f_lseek failed for read step, %x", res);
		f_close(&fsrc);
		return false;
	}

	// read buffer from file
	res = f_read(&fsrc, readBuffer, 21, &bytesProcessed);
	if ((res != FR_OK) || (21 != bytesProcessed)) 
	{
		module_debug_fatfs("f_read failed, %x, read %d of 21", res,
						   bytesProcessed);
		f_close(&fsrc);
		return false;
	}

	// close the file
	res = f_close(&fsrc);
	if (res != FR_OK) 
	{
		module_debug_fatfs("f_close failed for read step, %x", res);
		return false;
	}
	
	// remove the file
	res = f_unlink(testFileName);

	// compare the read and written buffers
	for(int i = 0; i < 21 ; i++)
	{
		if ((testFileData[i]) != (readBuffer[i]))
		{
			module_debug_fatfs("r and w buffers differ!");
			return false;
		}
	}
	
	// all OK!
	module_debug_fatfs("testFilesystem OK!");
	return true;
}

void FATFS_initializeFilesystem()
{
	DSTATUS resCard;			/* SDcard status */
	FRESULT	fr;					// return value from f_* calls
	
	m_fsAvailable = false;
	
	MICROSD_init();
	
	resCard = disk_initialize(0);       /*Check micro-SD card status */

	switch(resCard)
	{
	case STA_NOINIT:                    /* Drive not initialized */
		module_debug_fatfs("initialize returned STA_NOINIT!");
		break;
	case STA_NODISK:                    /* No medium in the drive */
		module_debug_fatfs("initialize returned STA_NODISK!");
		break;
	case STA_PROTECT:                   /* Write protected */
		module_debug_fatfs("initialize returned STA_PROTECT!");
		break;
	case 0:
		module_debug_fatfs("initialize OK!");
		break;
	default:
		module_debug_fatfs("initialized return unkown value: %x", resCard);
	}

	if(resCard != 0)
		return;

        
    // mount the filesystem if successful
	fr = f_mount(0, &Fatfs);
	if (fr != FR_OK)
	{
		module_debug_fatfs("f_mount not successful, %x", fr);
		return;
	}
	
	// otherwise, we successfully mounted the filesystem
	// run a simple test before marking it as available
	m_fsAvailable = FATFS_testFilesystem();
}

void FATFS_deinitializeFilesystem()
{
	FRESULT fr = f_mount(0, NULL);
	if (fr != FR_OK)
	{
		module_debug_fatfs("f_mount unmount not successful, %x", fr);
		return;
	}
	
	MICROSD_deinit();
}