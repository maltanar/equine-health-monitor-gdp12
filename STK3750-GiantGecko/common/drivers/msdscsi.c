/***************************************************************************//**
 * @file  msdscsi.c
 * @brief SCSI interface for Mass Storage Devices (MSD).
 * @author Energy Micro AS
 * @version 2.0.3
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#include "em_usb.h"
#include "msdscsi.h"
#include "msdbot.h"

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

/**************************************************************************//**
 * @brief Default Command Block Wrapper (CBW) data for
 *        SCSI Read10 command.
 *****************************************************************************/
#define CBW_SCSI_READ10_INIT_DEFAULT                 \
  {                                                  \
    CBW_SIGNATURE,   /* CBW Signature, ascii USBC */ \
    0x78563412,      /* CBW Tag                   */ \
    0x00000000,      /* CBW Data Transfer Length  */ \
    { { 0, 0, 1 } }, /* CBW Flags                 */ \
    { { 0, 0 } },    /* CBW Lun                   */ \
    { { 10, 0 } },   /* CBW CBLength              */ \
    { SCSI_READ10,   /* CBW CB                    */ \
      0, 0, 0, 0,                                    \
      0, 0, 0, 0,                                    \
      0, 0, 0, 0,                                    \
      0, 0, 0 }                                      \
  }

/**************************************************************************//**
 * @brief Default Command Block Wrapper (CBW) data for
 *        SCSI Write10 command.
 *****************************************************************************/
#define CBW_SCSI_WRITE10_INIT_DEFAULT                \
  {                                                  \
    CBW_SIGNATURE,   /* CBW Signature, ascii USBC */ \
    0x78563412,      /* CBW Tag                   */ \
    0x00000000,      /* CBW Data Transfer Length  */ \
    { { 0, 0, 0 } }, /* CBW Flags                 */ \
    { { 0, 0 } },    /* CBW Lun                   */ \
    { { 10, 0 } },   /* CBW CBLength              */ \
    { SCSI_WRITE10,  /* CBW CB                    */ \
      0, 0, 0, 0,                                    \
      0, 0, 0, 0,                                    \
      0, 0, 0, 0,                                    \
      0, 0, 0 }                                      \
  }

/**************************************************************************//**
 * @brief Default Command Block Wrapper (CBW) data for
 *        SCSI Inquiry command.
 *****************************************************************************/
EFM32_ALIGN(4)
static const char cbwInquiry[ CBW_LEN ] __attribute__ ((aligned(4))) =
{
  'U',          'S',                  'B',  'C',  /* CBW Signature                     */
  0x12,                         0x34, 0x56, 0x78, /* CBW Tag                           */
  SCSI_INQUIRYDATA_LEN,                           /* CBW Data Transfer Length          */
  0x00,                         0x00, 0x00,
  BOT_DIR_IN,                   0x00,    6,       /* CBW Flags, CBW Lun, CBW CB Length */
  SCSI_INQUIRY,                 0x00, 0x00,       /* 6 byte CBW CB                     */
  0x00,         SCSI_INQUIRYDATA_LEN,
  0x00,                         0x00, 0x00, 0x00,
  0x00,                         0x00, 0x00, 0x00,
  0x00,                         0x00, 0x00
};

/**************************************************************************//**
 * @brief Default Command Block Wrapper (CBW) data for
 *        SCSI Read Capacity command.
 *****************************************************************************/
EFM32_ALIGN(4)
static const char cbwReadCap[ CBW_LEN ] __attribute__ ((aligned(4))) =
{
  'U',        'S',  'B',  'C',  /* CBW Signature                     */
  0x12,       0x34, 0x56, 0x78, /* CBW Tag                           */
  SCSI_READCAPACITYDATA_LEN,    /* CBW Data Transfer Length          */
  0x00,       0x00, 0x00,
  BOT_DIR_IN, 0x00,   10,       /* CBW Flags, CBW Lun, CBW CB Length */
  SCSI_READCAPACITY,            /* 10 byte CBW CB                    */
  0x00,       0x00, 0x00, 0x00,
  0x00,       0x00, 0x00, 0x00,
  0x00,       0x00, 0x00, 0x00,
  0x00,       0x00, 0x00
};

/**************************************************************************//**
 * @brief Default Command Block Wrapper (CBW) data for
 *        SCSI Request Sense command.
 *****************************************************************************/
EFM32_ALIGN(4)
static const char cbwRs[ CBW_LEN ] __attribute__ ((aligned(4))) =
{
  'U',                       'S',  'B',  'C',  /* CBW Signature                     */
  0x12,                      0x34, 0x56, 0x78, /* CBW Tag                           */
  SCSI_REQUESTSENSEDATA_LEN,                   /* CBW Data Transfer Length          */
  0x00,                      0x00, 0x00,
  BOT_DIR_IN,                0x00,    6,       /* CBW Flags, CBW Lun, CBW CB Length */
  SCSI_REQUESTSENSE,         0x00,             /* 6 byte CBW CB                     */
  0x00,                      0x00,
  SCSI_REQUESTSENSEDATA_LEN,
  0x00,                      0x00, 0x00, 0x00,
  0x00,                      0x00, 0x00, 0x00,
  0x00,                      0x00, 0x00
};

/**************************************************************************//**
 * @brief Default Command Block Wrapper (CBW) data for
 *        SCSI Test Unit Ready command.
 *****************************************************************************/
EFM32_ALIGN(4)
static const char cbwTur[ CBW_LEN ] __attribute__ ((aligned(4))) =
{
  'U',                 'S',  'B',  'C',  /* CBW Signature                     */
  0x12,                0x34, 0x56, 0x78, /* CBW Tag                           */
  0x00,                0x00, 0x00, 0x00, /* CBW Data Transfer Length          */
  BOT_DIR_IN,          0x00,    6,       /* CBW Flags, CBW Lun, CBW CB Length */
  SCSI_TESTUNIT_READY, 0x00,             /* 6 byte CBW CB                     */
  0x00,                0x00, 0x00, 0x00,
  0x00,                0x00, 0x00, 0x00,
  0x00,                0x00, 0x00, 0x00,
  0x00, 0x00
};


/* Media properties. */
static uint32_t lbaCount = 0;
static uint32_t lbaSize  = 0;

/** @endcond */

/***************************************************************************//**
 * @brief
 *   MSDSCSI module initialization.
 *
 * @param[in] out
 *   Pointer to an MSD bulk OUT endpoint structure.
 *
 * @param[in] in
 *   Pointer to an MSD bulk IN endpoint structure.
 *
 * @return
 *   Returns true on success, false otherwise.
 ******************************************************************************/
bool MSDSCSI_Init(USBH_Ep_TypeDef *out, USBH_Ep_TypeDef *in)
{
  /* Check if all typedef's are properly packed. */

  if ((sizeof(MSDSCSI_Read10_TypeDef) != SCSI_READ10_LEN) ||
      (sizeof(MSDSCSI_Write10_TypeDef) != SCSI_WRITE10_LEN) ||
      (sizeof(MSDSCSI_InquiryData_TypeDef) != SCSI_INQUIRYDATA_LEN) ||
      (sizeof(MSDSCSI_RequestSenseData_TypeDef) != SCSI_REQUESTSENSEDATA_LEN) ||
      (sizeof(MSDSCSI_ReadCapacityData_TypeDef) != SCSI_READCAPACITYDATA_LEN))
  {
    DEBUG_USB_API_PUTS("\nMSDSCSI_Init(), typedef size error");
    EFM_ASSERT(false);
    return false;
  }

  /* Initialize the Bulk-Only-Transport (BOT) module. */
  if (MSDBOT_Init(out, in) != MSDBOT_STATUS_OK)
    return false;

  return true;
}

/***************************************************************************//**
 * @brief
 *   Issue a SCSI Inquiry command.
 *
 * @param[out] data
 *   Inquiry response data buffer.
 *
 * @return
 *   Returns true on success, false otherwise.
 ******************************************************************************/
bool MSDSCSI_Inquiry(MSDSCSI_InquiryData_TypeDef *data)
{
  if (MSDBOT_Xfer((void*) cbwInquiry, data) == SCSI_INQUIRYDATA_LEN)
    return true;

  return false;
}

/***************************************************************************//**
 * @brief
 *   Issue a SCSI Read(10) command.
 *
 * @param[in] lba
 *   Sector address (LBA) of first sector to read.
 *
 * @param[in] sectors
 *   Number of sectors to read.
 *
 * @param[out] data
 *   Data buffer through which data is returned to caller.
 *
 * @return
 *   Returns true on success, false otherwise.
 ******************************************************************************/
bool MSDSCSI_Read10(uint32_t lba, uint16_t sectors, void *data)
{
  EFM32_ALIGN(4)
  MSDBOT_CBW_TypeDef cbw __attribute__ ((aligned(4))) = CBW_SCSI_READ10_INIT_DEFAULT;

  MSDSCSI_Read10_TypeDef *cb = (MSDSCSI_Read10_TypeDef*) &cbw.CBWCB;

  cbw.dCBWDataTransferLength = sectors * lbaSize;
  cb->Lba                    = __REV(lba);
  cb->TransferLength         = __REV16(sectors);

  if ((uint32_t) MSDBOT_Xfer(&cbw, data) == cbw.dCBWDataTransferLength)
    return true;

  return false;
}

/***************************************************************************//**
 * @brief
 *   Issue a SCSI Read Capacity command.
 *
 * @param[out] data
 *   Read Capacity response data buffer.
 *
 * @return
 *   Returns true on success, false otherwise.
 ******************************************************************************/
bool MSDSCSI_ReadCapacity(MSDSCSI_ReadCapacityData_TypeDef *data)
{
  if (MSDBOT_Xfer((void*) cbwReadCap, data) == SCSI_READCAPACITYDATA_LEN)
  {
    /* Big to Little endian conversion, keep local copy of lba count and size */
    lbaCount = __REV(data->LogicalBlockAddress);
    lbaSize  = __REV(data->LogicalBlockLength);

    data->LogicalBlockAddress = lbaCount;
    data->LogicalBlockLength  = lbaSize;

    return true;
  }

  return false;
}

/***************************************************************************//**
 * @brief
 *   Issue a SCSI Request Sense command.
 *
 * @param[out] data
 *   Request Sense response data buffer.
 *
 * @return
 *   Returns true on success, false otherwise.
 ******************************************************************************/
bool MSDSCSI_RequestSense(MSDSCSI_RequestSenseData_TypeDef *data)
{
  if (MSDBOT_Xfer((void*) cbwRs, data) == SCSI_REQUESTSENSEDATA_LEN)
    return true;

  return false;
}

/***************************************************************************//**
 * @brief
 *   Issue a SCSI Test Unit Ready command.
 *
 * @return
 *   Returns true if device ready, false if device not ready or cmd
 *   transfer error.
 ******************************************************************************/
bool MSDSCSI_TestUnitReady(void)
{
  if (MSDBOT_Xfer((void*) cbwTur, NULL) == MSDBOT_STATUS_OK)
    return true;

  return false;
}

/***************************************************************************//**
 * @brief
 *   Perform a SCSI Write(10) command.
 *
 * @param[in] lba
 *   Sector address (LBA) of first sector to write.
 *
 * @param[in] sectors
 *   Number of sectors to write.
 *
 * @param[out] data
 *   Data buffer containing data to be written.
 *
 * @return
 *   Returns true on success, false otherwise.
 ******************************************************************************/
bool MSDSCSI_Write10(uint32_t lba, uint16_t sectors, const void *data)
{
  EFM32_ALIGN(4)
  MSDBOT_CBW_TypeDef cbw __attribute__ ((aligned(4))) = CBW_SCSI_WRITE10_INIT_DEFAULT;

  MSDSCSI_Write10_TypeDef *cb = (MSDSCSI_Write10_TypeDef*) &cbw.CBWCB;

  cbw.dCBWDataTransferLength = sectors * lbaSize;
  cb->Lba                    = __REV(lba);
  cb->TransferLength         = __REV16(sectors);

  if ((uint32_t) MSDBOT_Xfer(&cbw, (void*) data) == cbw.dCBWDataTransferLength)
    return true;

  return false;
}
