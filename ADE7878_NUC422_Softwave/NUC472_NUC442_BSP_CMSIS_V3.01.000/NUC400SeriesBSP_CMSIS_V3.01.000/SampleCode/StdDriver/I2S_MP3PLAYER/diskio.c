/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various existing      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include "NUC472_442.h"
#include "diskio.h"     /* FatFs lower layer API */

/* Supported Interface */
//#define SUPPORT_USBH
#define SUPPORT_SDIO

/* Definitions of physical drive number for each media */

#ifndef SUPPORT_USBH
#define DRV_SD      0
#define DRV_FMC     1
#else
#define DRV_USBH    0
#define DRV_SD      1
#define DRV_FMC     2
#endif

#ifdef SUPPORT_SDIO
extern DISK_DATA_T SD_DiskInfo0;
extern int SD_Open_(uint32_t cardSel);
#endif
/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv               /* Physical drive number (0..) */
)
{
    switch (pdrv) {

    case DRV_SD :
        SD_Open_(SD_PORT0 | CardDetect_From_GPIO);
        return RES_OK;
    }

    return RES_PARERR;
}
/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv       /* Physical drive number (0..) */
)
{
    switch (pdrv) {
    case DRV_SD :
        return RES_OK;
    }

    return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,      /* Physical drive number (0..) */
    BYTE *buff,     /* Data buffer to store read data */
    DWORD sector,   /* Sector address (LBA) */
    BYTE count      /* Number of sectors to read (1..128) */
)
{
    uint32_t status;

    switch (pdrv) {

    case DRV_SD :
        status = SD_Read(SD_PORT0, (uint8_t*)buff, sector, count);
        if (status != Successful)
            return RES_ERROR;

        return RES_OK;
    }

    return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
    BYTE pdrv,          /* Physical drive number (0..) */
    const BYTE *buff,   /* Data to be written */
    DWORD sector,       /* Sector address (LBA) */
    BYTE count          /* Number of sectors to write (1..128) */
)
{
    uint32_t status;

    switch (pdrv) {

    case DRV_SD :
        status = SD_Write(SD_PORT0, (uint8_t*)buff, sector, count);
        if (status != Successful)
            return RES_ERROR;

        return RES_OK;
    }
    return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
    BYTE pdrv,      /* Physical drive number (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    switch (pdrv) {

    case DRV_SD :
        switch(cmd) {
        case CTRL_SYNC:
            break;

        case GET_SECTOR_COUNT:
            *(DWORD*)buff = SD_DiskInfo0.totalSectorN;
            break;

        case GET_SECTOR_SIZE:
            *(WORD*)buff = SD_DiskInfo0.sectorSize;
            break;

        default:
            return RES_PARERR;
        }
        return RES_OK;
    }

    return RES_PARERR;
}
#endif
