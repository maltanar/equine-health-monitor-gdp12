/////////////////////////////////////////////////////////////////////////////////////////
// THE FOLLOWING EXAMPLE CODE IS INTENDED FOR LIMITED CIRCULATION ONLY.
// 
// Please forward all questions regarding this code to ANT Technical Support.
// 
// Dynastream Innovations Inc.
// 228 River Avenue
// Cochrane, Alberta, Canada
// T4C 2C1
// 
// (P) (403) 932-9292
// (F) (403) 932-6521
// (TF) 1-866-932-9292
// (E) support@thisisant.com
// 
// www.thisisant.com
//
// Reference Design Disclaimer
//
// The references designs and codes provided may be used with ANT devices only and remain the copyrighted property of 
// Dynastream Innovations Inc. The reference designs and codes are being provided on an "as-is" basis and as an accommodation, 
// and therefore all warranties, representations, or guarantees of any kind (whether express, implied or statutory) including, 
// without limitation, warranties of merchantability, non-infringement,
// or fitness for a particular purpose, are specifically disclaimed.
//
// ©2008 Dynastream Innovations Inc. All Rights Reserved
// This software may not be reproduced by
// any means without express written approval of Dynastream
// Innovations Inc.
//
/////////////////////////////////////////////////////////////////////////////////////////
#ifndef __HRM_RX__
#define __HRM_RX__

#include "antplus.h"                


// Page structs
typedef struct
{
   uint16_t usBeatTime;                                // All pages
   uint8_t ucBeatCount;                                // All pages
   uint8_t ucComputedHeartRate;                        // All pages
} HRMPage0_Data;

typedef struct
{
   uint32_t ulOperatingTime;                            // Page 1
} HRMPage1_Data;

typedef struct
{
   uint8_t ucManId;                                    // Page 2
   uint32_t ulSerialNumber;                             // Page 2
} HRMPage2_Data;

typedef struct
{
   uint8_t ucHwVersion;                                // Page 3
   uint8_t ucSwVersion;                                // Page 3
   uint8_t ucModelNumber;                              // Page 3
} HRMPage3_Data;

typedef struct
{
   uint8_t ucManufSpecific;                            // Page 4
   uint16_t usPreviousBeat;                            // Page 4
} HRMPage4_Data;

#endif
