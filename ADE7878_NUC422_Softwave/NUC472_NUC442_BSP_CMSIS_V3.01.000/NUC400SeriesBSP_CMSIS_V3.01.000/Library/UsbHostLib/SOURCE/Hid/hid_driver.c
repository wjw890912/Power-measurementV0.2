/**************************************************************************//**
 * @file     hid_driver.c
 * @version  V1.00
 * $Revision: 6 $
 * $Date: 14/05/29 4:23p $ 
 * @brief    NUC472/NUC442 MCU USB Host HID driver
 *
 * @note
 * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/   

#include <stdio.h>
#include <string.h>

#include "NUC472_442.h"
#include "usbh_core.h"

#include "hid.h"


/** @addtogroup NUC472_442_Device_Driver NUC472/NUC442 Device Driver
  @{
*/

/** @addtogroup NUC472_442_USBH_HID_Driver USB Host HID Driver
  @{
*/

/** @addtogroup NUC472_442_USBH_HID_EXPORTED_FUNCTIONS USB Host HID Driver Exported Functions
  @{
*/

/// @cond HIDDEN_SYMBOLS

static HID_DEV_T    g_hid_dev[CONFIG_HID_MAX_DEV];
static int          g_hid_handle = 0x101;



static HID_DEV_T *alloc_hid_device(void)
{
    int     i;
    
    for (i = 0; i < CONFIG_HID_MAX_DEV; i++)
    {
        if (g_hid_dev[i].udev == NULL)
        {
            memset((char *)&g_hid_dev[i], 0, sizeof(HID_DEV_T));
            g_hid_dev[i].handle = g_hid_handle++;
            return &g_hid_dev[i];
        }
    }
    return NULL;
}

void  free_hid_device(HID_DEV_T *hid_dev)
{
    hid_dev->udev = NULL;
    memset((char *)hid_dev, 0, sizeof(HID_DEV_T));
}


static int  usbhid_probe(USB_DEV_T *dev, USB_IF_DESC_T *ifd, const USB_DEV_ID_T *id)
{
    EP_INFO_T   *ep_info;
    int         ifnum;
    HID_DEV_T   *hid;
    int         i;

    HID_DBGMSG("usbhid_probe - dev=0x%x\n", (int)dev);
    
    ifnum = ifd->bInterfaceNumber;

    HID_DBGMSG("HID probe called for ifnum %d\n", ifnum); 

    ep_info = NULL;
    for (i = 0; i < dev->ep_list_cnt; i++)
    {
        if ((dev->ep_list[i].ifnum == ifnum) &&
            ((dev->ep_list[i].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT))
        {
            ep_info = &dev->ep_list[i];
            HID_DBGMSG("Interrupt Endpoint 0x%x found.\n", ep_info->bEndpointAddress);
        }
    }
    if (ep_info == NULL)
    {
        HID_DBGMSG("couldn't find an input interrupt endpoint\n");
        return USB_ERR_NODEV;
    }

    hid = alloc_hid_device();
    if (hid == NULL)
        return USB_ERR_NODEV;
        
    hid->ifnum = ifnum;
    hid->udev = dev;
    
    return 0;
}


static void  hid_disconnect(USB_DEV_T *dev)
{
    HID_DEV_T   *hid_dev;
    
    HID_DBGMSG("HID device disconnected!\n");
    
    hid_dev = find_hid_deivce_by_udev(dev);
    if (hid_dev == NULL)
        return;
    
    if (hid_dev->urbin)
    {
        USBH_UnlinkUrb(hid_dev->urbin);
        USBH_FreeUrb(hid_dev->urbin);
    }   
    if (hid_dev->urbout)
    {
        USBH_UnlinkUrb(hid_dev->urbout);
        USBH_FreeUrb(hid_dev->urbout);
    }   
    free_hid_device(hid_dev);
}


static USB_DEV_ID_T  hid_id_table[] = 
{
    USB_DEVICE_ID_MATCH_INT_CLASS,     /* match_flags */
    0, 0, 0, 0, 0, 0, 0,
    USB_INTERFACE_CLASS_HID,           /* bInterfaceClass */
    0, 0, 0 
};


static USB_DRIVER_T  hid_driver = 
{
    "hid driver",
    usbhid_probe,
    hid_disconnect,
    hid_id_table,
    NULL,                       /* suspend */
    NULL,                       /* resume */
    {NULL,NULL}                 /* driver_list */ 
};


EP_INFO_T *hid_get_ep_info(USB_DEV_T *dev, int ifnum, uint16_t dir)
{
    int   i;
    
    for (i = 0; i < dev->ep_list_cnt; i++)
    {
        if ((dev->ep_list[i].ifnum == ifnum) &&
            ((dev->ep_list[i].bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT))
        {
            if ((dev->ep_list[i].bEndpointAddress & USB_ENDPOINT_DIR_MASK) == dir)
            {
            	//printf("Endpoint found: 0x%x\n", dev->ep_list[i].bEndpointAddress);
                return &dev->ep_list[i];
            }
        }
    }
    return NULL;
}


HID_DEV_T *find_hid_deivce_by_udev(USB_DEV_T *udev)
{
    int     i;
    
    if (udev == NULL)
        return NULL;
    
    for (i = 0; i < CONFIG_HID_MAX_DEV; i++)
    {
        if (g_hid_dev[i].udev == udev)
        {
            return &g_hid_dev[i];
        }
    }
    return NULL;
}


HID_DEV_T *find_hid_deivce_by_handle(int handle)
{
    int     i;
    
    for (i = 0; i < CONFIG_HID_MAX_DEV; i++)
    {
        if ((g_hid_dev[i].udev != NULL) && (g_hid_dev[i].handle == handle))
        {
            return &g_hid_dev[i];
        }
    }
    return NULL;
}


hid_return  usbh_hid_find_device(HIDInterface* hidif, HIDInterfaceMatcher *matcher)
{
    int     i;
    
    for (i = 0; i < CONFIG_HID_MAX_DEV; i++)
    {
        if (g_hid_dev[i].udev != NULL)
        {
            if ((g_hid_dev[i].udev->descriptor.idVendor == matcher->vendor_id) &&
                (g_hid_dev[i].udev->descriptor.idProduct == matcher->product_id))
            {
                hidif->dev_handle = g_hid_dev[i].handle;
                return HID_RET_SUCCESS;
            }
        }
    }
    
    for (i = 0; i < CONFIG_HID_MAX_DEV; i++)
    {
        if (g_hid_dev[i].udev != NULL)
        {
            if ((g_hid_dev[i].udev->descriptor.idVendor != matcher->vendor_id) &&
                (matcher->vendor_id != 0))
                continue;
                
            if ((g_hid_dev[i].udev->descriptor.idProduct != matcher->product_id) &&
                (matcher->product_id != 0))
                continue;

            hidif->dev_handle = g_hid_dev[i].handle;
            return HID_RET_SUCCESS;
        }
    }
    return HID_RET_DEVICE_NOT_FOUND;
}

/// @endcond /* HIDDEN_SYMBOLS */

/**
  * @brief    Init USB Host HID driver.
  */
void USBH_HidInit(void)
{
    memset((char *)&g_hid_dev[0], 0, sizeof(g_hid_dev));    
    USBH_RegisterDriver(&hid_driver);
}


/*@}*/ /* end of group NUC472_442_USBH_HID_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC472_442_USBH_HID_Driver */

/*@}*/ /* end of group NUC472_442_Device_Driver */

/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/

