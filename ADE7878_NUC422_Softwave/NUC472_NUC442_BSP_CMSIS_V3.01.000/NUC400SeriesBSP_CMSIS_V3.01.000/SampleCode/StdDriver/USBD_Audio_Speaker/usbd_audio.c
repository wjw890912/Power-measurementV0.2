/******************************************************************************
 * @file     usbd_audio.c
 * @brief    NuMicro series USBD driver Sample file
 * @version  1.0.0
 * @date     23, December, 2013
 *
 * @note
 * Copyright (C) 2014 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <string.h>
#include "NUC472_442.h"
#include "usbd_audio.h"

/*--------------------------------------------------------------------------*/
static volatile uint8_t bPlayVolumeLAdjust = FALSE;
static volatile uint8_t bPlayVolumeRAdjust = FALSE;
static volatile uint8_t bIsI2CIdle = TRUE;

/*--------------------------------------------------------------------------*/
/* Global variables for Audio class */
uint32_t g_usbd_UsbAudioState = 0;

uint8_t  g_usbd_RecMute = 0x01;
int16_t  g_usbd_RecVolume = 0x1000;
int16_t  g_usbd_RecMaxVolume = 0x7FFF;
int16_t  g_usbd_RecMinVolume = 0x8000;
int16_t  g_usbd_RecResVolume = 0x400;

uint8_t  g_usbd_PlayMute = 0x01;
int16_t  g_usbd_PlayVolumeL = 0x1000;
int16_t  g_usbd_PlayVolumeR = 0x1000;
int16_t  g_usbd_PlayMaxVolume = 0x7FFF;
int16_t  g_usbd_PlayMinVolume = 0x8000;
int16_t  g_usbd_PlayResVolume = 0x400;

/*--------------------------------------------------------------------------*/
/**
 * @brief       USBD Interrupt Service Routine
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This function is the USBD ISR
 */
void USBD_IRQHandler(void)
{
    __IO uint32_t IrqStL, IrqSt;

    IrqStL = USBD->GINTSTS & USBD->GINTEN;    /* get interrupt status */

    if (!IrqStL)    return;

    /* USB interrupt */
    if (IrqStL & USBD_GINTSTS_USBIF_Msk) {
        IrqSt = USBD->BUSINTSTS & USBD->BUSINTEN;

        if (IrqSt & USBD_BUSINTSTS_SOFIF_Msk)
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_SOFIF_Msk);

        if (IrqSt & USBD_BUSINTSTS_RSTIF_Msk) {
            USBD_SwReset();
            USBD_ResetDMA();

            UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk);
            USBD_SET_ADDR(0);
            UBSD_ENABLE_BUS_INT(USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_RESUMEIEN_Msk|USBD_BUSINTEN_SUSPENDIEN_Msk);
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_RSTIF_Msk);
            USBD_CLR_CEP_INT_FLAG(0x1ffc);
        }

        if (IrqSt & USBD_BUSINTSTS_RESUMEIF_Msk) {
            UBSD_ENABLE_BUS_INT(USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_SUSPENDIEN_Msk);
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_RESUMEIF_Msk);
        }

        if (IrqSt & USBD_BUSINTSTS_SUSPENDIF_Msk) {
            UBSD_ENABLE_BUS_INT(USBD_BUSINTEN_RSTIEN_Msk | USBD_BUSINTEN_RESUMEIEN_Msk);
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_SUSPENDIF_Msk);
        }

        if (IrqSt & USBD_BUSINTSTS_HISPDIF_Msk) {
            UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk);
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_HISPDIF_Msk);
        }

        if (IrqSt & USBD_BUSINTSTS_DMADONEIF_Msk) {
            g_usbd_DmaDone = 1;
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_DMADONEIF_Msk);

            if (!(USBD->DMACTL & USBD_DMACTL_DMARD_Msk)) {
                UBSD_ENABLE_EP_INT(EPB, USBD_EPINTEN_RXPKIEN_Msk);
            }

            if (USBD->DMACTL & USBD_DMACTL_DMARD_Msk) {
                if (g_usbd_ShortPacket == 1) {
                    USBD->EPARSPCTL = USB_EP_RSPCTL_SHORTTXEN;    // packet end
                    g_usbd_ShortPacket = 0;
                }
            }
        }

        if (IrqSt & USBD_BUSINTSTS_PHYCLKVLDIF_Msk)
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_PHYCLKVLDIF_Msk);

        if (IrqSt & USBD_BUSINTSTS_VBUSDETIF_Msk) {
            if (USBD_IS_ATTACHED()) {
                /* USB Plug In */
                USBD_ENABLE_USB();
            } else {
                /* USB Un-plug */
                USBD_DISABLE_USB();
            }
            USBD_CLR_BUS_INT_FLAG(USBD_BUSINTSTS_VBUSDETIF_Msk);
        }
    }

    if (IrqStL & USBD_GINTSTS_CEPIF_Msk) {
        IrqSt = USBD->CEPINTSTS & USBD->CEPINTEN;

        if (IrqSt & USBD_CEPINTSTS_SETUPTKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_SETUPTKIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_SETUPPKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_SETUPPKIF_Msk);
            USBD_ProcessSetupPacket();
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_OUTTKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_OUTTKIF_Msk);
            UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_STSDONEIEN_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_INTKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
            if (!(IrqSt & USBD_CEPINTSTS_STSDONEIF_Msk)) {
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_TXPKIF_Msk);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_TXPKIEN_Msk);
                USBD_CtrlIn();
            } else {
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_TXPKIF_Msk);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_TXPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
            }
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_PINGIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_PINGIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_TXPKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
            USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
            if (g_usbd_CtrlInSize) {
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
            } else {
                if (g_usbd_CtrlZero == 1)
                    USBD_SET_CEP_STATE(USB_CEPCTL_ZEROLEN);
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
            }
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_TXPKIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_RXPKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_RXPKIF_Msk);
            USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
            UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_NAKIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_NAKIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_STALLIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STALLIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_ERRIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_ERRIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_STSDONEIF_Msk) {
            USBD_UpdateDeviceState();
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
            UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_BUFFULLIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_BUFFULLIF_Msk);
            return;
        }

        if (IrqSt & USBD_CEPINTSTS_BUFEMPTYIF_Msk) {
            USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_BUFEMPTYIF_Msk);
            return;
        }
    }

    /* Isochronous in */
    if (IrqStL & USBD_GINTSTS_EPAIF_Msk) {
        IrqSt = USBD->EPAINTSTS & USBD->EPAINTEN;

        EPA_Handler();
        UBSD_ENABLE_EP_INT(EPA, 0);
        USBD_CLR_EP_INT_FLAG(EPA, IrqSt);
    }
    /* Isochronous out */
    if (IrqStL & USBD_GINTSTS_EPBIF_Msk) {
        IrqSt = USBD->EPBINTSTS & USBD->EPBINTEN;

        EPB_Handler();
        USBD_CLR_EP_INT_FLAG(EPB, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPCIF_Msk) {
        IrqSt = USBD->EPCINTSTS & USBD->EPCINTEN;
        USBD_CLR_EP_INT_FLAG(EPC, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPDIF_Msk) {
        IrqSt = USBD->EPDINTSTS & USBD->EPDINTEN;
        USBD_CLR_EP_INT_FLAG(EPD, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPEIF_Msk) {
        IrqSt = USBD->EPEINTSTS & USBD->EPEINTEN;
        USBD_CLR_EP_INT_FLAG(EPE, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPFIF_Msk) {
        IrqSt = USBD->EPFINTSTS & USBD->EPFINTEN;
        USBD_CLR_EP_INT_FLAG(EPF, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPGIF_Msk) {
        IrqSt = USBD->EPGINTSTS & USBD->EPGINTEN;
        USBD_CLR_EP_INT_FLAG(EPG, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPHIF_Msk) {
        IrqSt = USBD->EPHINTSTS & USBD->EPHINTEN;
        USBD_CLR_EP_INT_FLAG(EPH, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPIIF_Msk) {
        IrqSt = USBD->EPIINTSTS & USBD->EPIINTEN;
        USBD_CLR_EP_INT_FLAG(EPI, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPJIF_Msk) {
        IrqSt = USBD->EPJINTSTS & USBD->EPJINTEN;
        USBD_CLR_EP_INT_FLAG(EPJ, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPKIF_Msk) {
        IrqSt = USBD->EPKINTSTS & USBD->EPKINTEN;
        USBD_CLR_EP_INT_FLAG(EPK, IrqSt);
    }

    if (IrqStL & USBD_GINTSTS_EPLIF_Msk) {
        IrqSt = USBD->EPLINTSTS & USBD->EPLINTEN;
        USBD_CLR_EP_INT_FLAG(EPL, IrqSt);
    }
}

/**
 * @brief       EPA Handler
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This function is used to process EPA event
 */
/* Record */
void EPA_Handler(void)
{
    /* ISO IN transfer ACK */
    if (g_usbd_UsbAudioState == UAC_START_AUDIO_RECORD) {
        UAC_DeviceEnable(UAC_MICROPHONE);
        g_usbd_UsbAudioState = UAC_PROCESSING_AUDIO_RECORD;
    } else if (g_usbd_UsbAudioState == UAC_PROCESSING_AUDIO_RECORD)
        g_usbd_UsbAudioState = UAC_BUSY_AUDIO_RECORD;

    if (g_usbd_UsbAudioState == UAC_BUSY_AUDIO_RECORD)
        UAC_SendRecData();
    else
        USBD->EPARSPCTL = USBD_EPRSPCTL_ZEROLEN_Msk;    // zero-packet
}

/**
 * @brief       EPB Handler
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This function is used to process EPB event
 */
/* Play */
void EPB_Handler(void)
{
    uint32_t volatile len, i;
    unsigned char rec[512], *ptr;

    ptr = rec;
    // get data from FIFO
    len = USBD->EPBDATCNT & 0xffff;

    for (i=0; i<(len >> 2); i++, ptr+=4)
        *(uint32_t *)(ptr) = USBD->EPBDAT;

    for (i=0; i<(len % 4); i++)
        *(uint8_t *)(ptr + i) = USBD->EPBDAT_BYTE;

    UAC_GetPlayData((int16_t *)rec, len);
}

/*--------------------------------------------------------------------------*/
/**
 * @brief       UAC Class Initial
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This function is used to configure endpoints for UAC class
 */
void UAC_Init(void)
{
    /* Configure USB controller */
    USBD->OPER = 0; /* Full Speed */
    /* Enable USB BUS, CEP and EPA , EPB global interrupt */
    UBSD_ENABLE_USB_INT(USBD_GINTEN_USBIE_Msk|USBD_GINTEN_CEPIE_Msk|USBD_GINTEN_EPAIE_Msk|USBD_GINTEN_EPBIE_Msk);
    /* Enable BUS interrupt */
    UBSD_ENABLE_BUS_INT(USBD_BUSINTEN_DMADONEIEN_Msk|USBD_BUSINTEN_RESUMEIEN_Msk|USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_VBUSDETIEN_Msk);
    /* Reset Address to 0 */
    USBD_SET_ADDR(0);

    /*****************************************************/
    /* Control endpoint */
    USBD_SetEpBufAddr(CEP, CEP_BUF_BASE, CEP_BUF_LEN);
    UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_SETUPPKIEN_Msk|USBD_CEPINTEN_STSDONEIEN_Msk);

    /*****************************************************/
    /* EPA ==> ISO IN endpoint, address 1 */
    USBD_SetEpBufAddr(EPA, EPA_BUF_BASE, EPA_BUF_LEN);
    UBSD_SET_MAX_PAYLOAD(EPA, EPA_MAX_PKT_SIZE);
    USBD_ConfigEp(EPA, ISO_IN_EP_NUM, USB_EP_CFG_TYPE_ISO, USB_EP_CFG_DIR_IN);

    /* EPB ==> ISO OUT endpoint, address 2 */
    USBD_SetEpBufAddr(EPB, EPB_BUF_BASE, EPB_BUF_LEN);
    UBSD_SET_MAX_PAYLOAD(EPB, EPB_MAX_PKT_SIZE);
    USBD_ConfigEp(EPB, ISO_OUT_EP_NUM, USB_EP_CFG_TYPE_ISO, USB_EP_CFG_DIR_OUT);
    UBSD_ENABLE_EP_INT(EPB, USBD_EPINTEN_RXPKIEN_Msk|USBD_EPINTEN_SHORTRXIEN_Msk);

}


/**
 * @brief       UAC class request
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This function is used to process UAC class requests
 */
void UAC_ClassRequest(void)
{
    if (gUsbCmd.bmRequestType & 0x80) { /* request data transfer direction */
        // Device to host
        switch (gUsbCmd.bRequest) {
        case UAC_GET_CUR: {
            switch ((gUsbCmd.wValue & 0xff00) >> 8) {
            case MUTE_CONTROL: {
                if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    USBD_PrepareCtrlIn((uint8_t *)&g_usbd_RecMute, 1);
                else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    USBD_PrepareCtrlIn((uint8_t *)&g_usbd_PlayMute, 1);

                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                break;
            }
            case VOLUME_CONTROL: {
                if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    USBD_PrepareCtrlIn((uint8_t *)&g_usbd_RecVolume, 2);
                else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff)) {
                    if((gUsbCmd.wValue & 0xff) == 1)
                        USBD_PrepareCtrlIn((uint8_t *)&g_usbd_PlayVolumeL, 2);
                    else
                        USBD_PrepareCtrlIn((uint8_t *)&g_usbd_PlayVolumeR, 2);
                }
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                break;
            }
            default: {
                /* Setup error, stall the device */
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
            }
            }
            break;
        }

        case UAC_GET_MIN: {
            switch ((gUsbCmd.wValue & 0xff00) >> 8) {
            case VOLUME_CONTROL: {
                if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    USBD_PrepareCtrlIn((uint8_t *)&g_usbd_RecMinVolume, 2);
                else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    USBD_PrepareCtrlIn((uint8_t *)&g_usbd_PlayMinVolume, 2);

                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                break;
            }
            default:
                /* STALL control pipe */
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
            }
            break;
        }

        case UAC_GET_MAX: {
            switch ((gUsbCmd.wValue & 0xff00) >> 8) {
            case VOLUME_CONTROL: {
                if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    USBD_PrepareCtrlIn((uint8_t *)&g_usbd_RecMaxVolume, 2);
                else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    USBD_PrepareCtrlIn((uint8_t *)&g_usbd_PlayMaxVolume, 2);
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                break;
            }
            default:
                /* STALL control pipe */
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
            }
            break;
        }

        case UAC_GET_RES: {
            switch ((gUsbCmd.wValue & 0xff00) >> 8) {
            case VOLUME_CONTROL: {
                if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    USBD_PrepareCtrlIn((uint8_t *)&g_usbd_RecResVolume, 2);
                else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff))
                    USBD_PrepareCtrlIn((uint8_t *)&g_usbd_PlayResVolume, 2);
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_INTKIF_Msk);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_INTKIEN_Msk);
                break;
            }
            default:
                /* STALL control pipe */
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
            }
            break;
        }

        default: {
            /* Setup error, stall the device */
            USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
        }
        }
    } else {
        // Host to device
        switch (gUsbCmd.bRequest) {
        case UAC_SET_CUR: {
            UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_OUTTKIEN_Msk | USBD_CEPINTEN_RXPKIEN_Msk);
            switch ((gUsbCmd.wValue & 0xff00) >> 8) {
            case MUTE_CONTROL:
                if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff)) {
                    USBD_CtrlOut((uint8_t *)&g_usbd_RecMute, gUsbCmd.wLength);
                } else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff)) {
                    USBD_CtrlOut((uint8_t *)&g_usbd_PlayMute, gUsbCmd.wLength);
                }

                /* Status stage */
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
                USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_STSDONEIEN_Msk);
                break;

            case VOLUME_CONTROL:
                if (REC_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff)) {
                    USBD_CtrlOut((uint8_t *)&g_usbd_RecVolume, gUsbCmd.wLength);
                } else if (PLAY_FEATURE_UNITID == ((gUsbCmd.wIndex >> 8) & 0xff)) {
                    if (((gUsbCmd.wValue >> 8) & 0xff) == 1) {
                        USBD_CtrlOut((uint8_t *)&g_usbd_PlayVolumeL, gUsbCmd.wLength);
                        if(g_usbd_PlayVolumeL & 0x8000)
                            g_usbd_PlayVolumeL = (g_usbd_PlayVolumeL & 0x7FFF) >> 8;
                        else
                            g_usbd_PlayVolumeL = (g_usbd_PlayVolumeL >> 7);
                        bPlayVolumeLAdjust = TRUE; //ATOM_I2C_WriteWAU8822(11, i16PlayVolumeL | 0x100);   /* Set left DAC volume */
                    } else {
                        USBD_CtrlOut((uint8_t *)&g_usbd_PlayVolumeR, gUsbCmd.wLength);
                        if(g_usbd_PlayVolumeR & 0x8000)
                            g_usbd_PlayVolumeR = (g_usbd_PlayVolumeR & 0x7FFF) >> 8;
                        else
                            g_usbd_PlayVolumeR = (g_usbd_PlayVolumeR >> 7);
                        bPlayVolumeRAdjust = TRUE; //ATOM_I2C_WriteWAU8822(12, i16PlayVolumeR | 0x100);   /* Set right DAC volume */
                    }
                }
                /* Status stage */
                USBD_CLR_CEP_INT_FLAG(USBD_CEPINTSTS_STSDONEIF_Msk);
                USBD_SET_CEP_STATE(USB_CEPCTL_NAKCLR);
                UBSD_ENABLE_CEP_INT(USBD_CEPINTEN_STSDONEIEN_Msk);
                break;

            default:
                /* STALL control pipe */
                USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
                break;
            }
            break;
        }

        default: {
            /* Setup error, stall the device */
            USBD_SET_CEP_STATE(USBD_CEPCTL_STALLEN_Msk);
            break;
        }
        }
    }
}

/**
 * @brief       Set Interface standard request
 *
 * @param[in]   u32AltInterface Interface
 *
 * @return      None
 *
 * @details     This function is used to set UAC Class relative setting
 */
void UAC_SetInterface(uint32_t u32AltInterface)
{
    if ((gUsbCmd.wIndex & 0xff) == 1) {
        /* Audio Iso IN interface */
        if (u32AltInterface == 1) {
            g_usbd_UsbAudioState = UAC_START_AUDIO_RECORD;
            USBD->EPARSPCTL = USBD_EPRSPCTL_ZEROLEN_Msk;
            UAC_DeviceEnable(UAC_MICROPHONE);
        } else if (u32AltInterface == 0) {
            UAC_DeviceDisable(UAC_MICROPHONE);
            USBD->EPARSPCTL = USBD_EPRSPCTL_ZEROLEN_Msk;
            g_usbd_UsbAudioState = UAC_STOP_AUDIO_RECORD;
        }
    } else if ((gUsbCmd.wIndex & 0xff) == 2) {
        /* Audio Iso OUT interface */
        if (u32AltInterface == 1)
            UAC_DeviceEnable(UAC_SPEAKER);
        else
            UAC_DeviceDisable(UAC_SPEAKER);
    }
}

/*******************************************************************/
/* For I2C transfer */
__IO uint32_t EndFlag0 = 0;
uint8_t Device_Addr0 = 0x1A;                /* WAU8822 Device ID */
uint8_t Tx_Data0[2];
uint8_t DataCnt0;

typedef enum {
    E_RS_NONE,          // no resampling
    E_RS_UP,            // up sampling
    E_RS_DOWN           // down sampling
} RESAMPLE_STATE_T;

#define BUFF_LEN    32*12
#define REC_LEN     REC_RATE / 1000

/* Recoder Buffer and its pointer */
uint16_t PcmRecBuff[BUFF_LEN] = {0};
uint32_t u32RecPos_Out = 0;
uint32_t u32RecPos_In = 0;

/* Player Buffer and its pointer */
uint32_t PcmPlayBuff[BUFF_LEN] = {0};
uint32_t u32PlayPos_Out = 0;
uint32_t u32PlayPos_In = 0;

static void Delay(uint32_t t)
{
    volatile int32_t delay;

    delay = t;

    while(delay-- >= 0);
}

void RecoveryFromArbLost(void)
{
    I2C0->CTL &= ~I2C_CTL_I2CEN_Msk;
    I2C0->CTL |= I2C_CTL_I2CEN_Msk;
}
/*---------------------------------------------------------------------------------------------------------*/
/*  Write 9-bit data to 7-bit address register of WAU8822 with I2C0                                        */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_WriteWAU8822(uint8_t u8addr, uint16_t u16data)
{
    bIsI2CIdle = FALSE;
restart:
    I2C_START(I2C0);
    I2C_WAIT_READY(I2C0);

    I2C_SET_DATA(I2C0, 0x1A<<1);
    I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    I2C_WAIT_READY(I2C0);
    if(I2C_GET_STATUS(I2C0) == 0x38) {
        RecoveryFromArbLost();
        goto restart;
    } else if(I2C_GET_STATUS(I2C0) != 0x18)
        goto stop;

    I2C_SET_DATA(I2C0, (uint8_t)((u8addr << 1) | (u16data >> 8)));
    I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    I2C_WAIT_READY(I2C0);
    if(I2C_GET_STATUS(I2C0) == 0x38) {
        RecoveryFromArbLost();
        goto restart;
    } else if(I2C_GET_STATUS(I2C0) != 0x28)
        goto stop;

    I2C_SET_DATA(I2C0, (uint8_t)(u16data & 0x00FF));
    I2C_SET_CONTROL_REG(I2C0, I2C_SI);
    I2C_WAIT_READY(I2C0);
    if(I2C_GET_STATUS(I2C0) == 0x38) {
        RecoveryFromArbLost();
        goto restart;
    } else if(I2C_GET_STATUS(I2C0) != 0x28)
        goto stop;

stop:
    I2C_STOP(I2C0);
    while(I2C0->CTL & I2C_CTL_STO_Msk);

    bIsI2CIdle = TRUE;
    EndFlag0 = 1;
}

static void ATOM_I2C_WriteWAU8822(uint8_t u8addr, uint16_t u16data)
{
    if(!bIsI2CIdle)
        while (EndFlag0 == 0);

    I2C_WriteWAU8822(u8addr, u16data);
}

void WAU8822_Setup(void)
{
    I2C_WriteWAU8822(0,  0x000);   /* Reset all registers */
    Delay(0x200);

    I2C_WriteWAU8822(1,  0x02F);
    I2C_WriteWAU8822(2,  0x1B3);   /* Enable L/R Headphone, ADC Mix/Boost, ADC */
    I2C_WriteWAU8822(3,  0x07F);   /* Enable L/R main mixer, DAC */
    I2C_WriteWAU8822(4,  0x010);   /* 16-bit word length, I2S format, Stereo */
    I2C_WriteWAU8822(5,  0x000);   /* Companding control and loop back mode (all disable) */
    I2C_WriteWAU8822(6,  0x1AD);   /* Divide by 6, 16K */
    I2C_WriteWAU8822(7,  0x006);   /* 16K for internal filter coefficients */
    I2C_WriteWAU8822(10, 0x008);   /* DAC soft mute is disabled, DAC oversampling rate is 128x */
    I2C_WriteWAU8822(14, 0x108);   /* ADC HP filter is disabled, ADC oversampling rate is 128x */
    I2C_WriteWAU8822(15, 0x1FF);   /* ADC left digital volume control */
    I2C_WriteWAU8822(16, 0x1FF);   /* ADC right digital volume control */

    I2C_WriteWAU8822(44, 0x000);   /* LLIN/RLIN is not connected to PGA */
    I2C_WriteWAU8822(47, 0x050);   /* LLIN connected, and its Gain value */
    I2C_WriteWAU8822(48, 0x050);   /* RLIN connected, and its Gain value */
    I2C_WriteWAU8822(50, 0x001);   /* Left DAC connected to LMIX */
    I2C_WriteWAU8822(51, 0x001);   /* Right DAC connected to RMIX */
}

void Tx_thresholdCallbackfn(void)
{
    uint32_t i;
    uint32_t * pBuff;

    if ((u32PlayPos_Out < u32PlayPos_In - 4) || (u32PlayPos_Out > u32PlayPos_In)) {
        pBuff = &PcmPlayBuff[u32PlayPos_Out];

        for( i = 0; i < 4; i++) {
            I2S_WRITE_TX_FIFO(I2S0, pBuff[i]);
        }

        u32PlayPos_Out += 4;

        if (u32PlayPos_Out >= BUFF_LEN) {
            u32PlayPos_Out = 0;
        }
    } else {
        for( i = 0; i < 4; i++) {
            I2S_WRITE_TX_FIFO(I2S0, 0x00);
        }
    }
}

void Rx_thresholdCallbackfn(void)
{
    int32_t  i;
    uint16_t *pBuff;

    pBuff = &PcmRecBuff[u32RecPos_In];

    for ( i = 0; i < 4; i++ ) {
        pBuff[i] = (uint16_t)(I2S_READ_RX_FIFO(I2S0) & 0xFFFF);
    }

    u32RecPos_In += 4;

    if (u32RecPos_In >= BUFF_LEN) {
        u32RecPos_In = 0;
    }
}

static uint8_t u8RecEn = 0;
static uint8_t u8PlayEn = 0;
void I2S0_IRQHandler(void)
{
    uint32_t u32I2SIntFlag;

    u32I2SIntFlag = I2S_GET_INT_FLAG(I2S0, I2S_STATUS_TXIF_Msk | I2S_STATUS_RXIF_Msk);

    if (u32I2SIntFlag & I2S_STATUS_TXIF_Msk) {
        /* Tx threshold level */
        if (I2S_GET_INT_FLAG(I2S0, I2S_STATUS_TXTHIF_Msk) & I2S_STATUS_TXTHIF_Msk) {
            if (u8PlayEn)
                Tx_thresholdCallbackfn();
        }
    } else if (u32I2SIntFlag & I2S_STATUS_RXIF_Msk) {
        /* Rx threshold level */
        if (I2S_GET_INT_FLAG(I2S0, I2S_STATUS_RXTHIF_Msk) & I2S_STATUS_RXTHIF_Msk) {
            if (u8RecEn)
                Rx_thresholdCallbackfn();
        }
    }
}



/**
  * @brief  SendRecData, prepare the record data for next ISO transfer.
  * @param  None.
  * @retval None.
  */
void UAC_SendRecData(void)
{
    uint16_t *pBuff;

    if ((u32RecPos_Out < u32RecPos_In - REC_LEN) || (u32RecPos_Out > u32RecPos_In)) {
        pBuff = &PcmRecBuff[u32RecPos_Out];

        USBD_SET_DMA_READ(ISO_IN_EP_NUM);
        UBSD_ENABLE_EP_INT(EPA, USBD_EPINTEN_TXPKIEN_Msk);
        UBSD_ENABLE_BUS_INT(USBD_BUSINTEN_DMADONEIEN_Msk|USBD_BUSINTEN_SUSPENDIEN_Msk|USBD_BUSINTEN_RSTIEN_Msk|USBD_BUSINTEN_VBUSDETIEN_Msk);
        USBD_SET_DMA_ADDR((uint32_t)pBuff);
        USBD_SET_DMA_LEN(REC_LEN * 2);
        g_usbd_ShortPacket = 1;
        g_usbd_DmaDone = 0;
        USBD_ENABLE_DMA();
        while(1) {
            if (g_usbd_DmaDone)
                break;

            if (!USBD_IS_ATTACHED())
                break;
        }
        u32RecPos_Out += REC_LEN;

        if (u32RecPos_Out >= BUFF_LEN) {
            u32RecPos_Out = 0;
        }
    } else {
        USBD->EPARSPCTL = USBD_EPRSPCTL_ZEROLEN_Msk;
    }
}


/**
  * @brief  UAC_DeviceEnable. To enable the device to play or record audio data.
  * @param  u8Object: To select the device, UAC_MICROPHONE or UAC_SPEAKER.
  * @retval None.
  */
void UAC_DeviceEnable(uint8_t u8Object)
{
    if (u8Object == UAC_MICROPHONE) {
        /* Enable record hardware */
        u8RecEn = 1;
        I2S_EnableInt(I2S0, I2S_IEN_RXTHIEN_Msk);
        I2S_ENABLE_RX(I2S0);
    } else {
        /* Enable play hardware */
        u8PlayEn = 1;
        // enable sound output
        PA9 = 0;
        I2S_EnableInt(I2S0, I2S_IEN_TXTHIEN_Msk);
        I2S_ENABLE_TX(I2S0);
    }
    NVIC_SetPriority(I2S0_IRQn, (1<<__NVIC_PRIO_BITS) - 2);
    TIMER_Start(TIMER0);
}


/**
  * @brief  UAC_DeviceDisable. To disable the device to play or record audio data.
  * @param  u8Object: To select the device, UAC_MICROPHONE or UAC_SPEAKER.
  * @retval None.
  */
void UAC_DeviceDisable(uint8_t u8Object)
{
    if (u8Object ==  UAC_MICROPHONE ) {
        /* Disable record hardware/stop record */
        u8RecEn = 0;
        I2S_DisableInt(I2S0, I2S_IEN_RXTHIEN_Msk);
        I2S_DISABLE_RX(I2S0);
    } else {
        /* Disable play hardware/stop play */
        u8PlayEn = 0;
        I2S_DisableInt(I2S0, I2S_IEN_TXTHIEN_Msk);
        I2S_DISABLE_TX(I2S0);
    }
    TIMER0->CTL |= TIMER_CTL_RSTCNT_Msk;
}


/**
  * @brief  GetPlayData, To get data from ISO OUT to play buffer.
  * @param  pi16src: The data buffer of ISO OUT.
  *         i16Samples: The sample number in data buffer.
  * @retval None.
  */
void UAC_GetPlayData(int16_t *pi16src, int16_t i16Samples)
{
    uint32_t u32len, i;
    uint32_t *pBuff;
    uint32_t *pSrc;

    u32len = PLAY_RATE/1000;
    pBuff = &PcmPlayBuff[u32PlayPos_In];
    pSrc = (uint32_t *) pi16src;

    for ( i = 0; i < u32len; i++ ) {
        pBuff[i] = pSrc[i];
    }

    u32PlayPos_In += u32len;

    if (u32PlayPos_In >= BUFF_LEN) {
        u32PlayPos_In = 0;
    }
}

void AdjustCodecPll(RESAMPLE_STATE_T r)
{
    static uint16_t tb[3][3] = {{0x00C, 0x093, 0x0E9}, // 8.192
        {0x00E, 0x1D2, 0x1E3},  // * 1.005 = 8.233
        {0x009, 0x153, 0x1EF}
    }; // * .995 = 8.151
    static RESAMPLE_STATE_T current = E_RS_NONE;
    int i, s;

    if(r == current)
        return;
    else
        current = r;
    switch(r) {
    case E_RS_UP:
        s = 1;
        break;
    case E_RS_DOWN:
        s = 2;
        break;
    case E_RS_NONE:
    default:
        s = 0;
    }

    for(i=0; i<3; i++)
        ATOM_I2C_WriteWAU8822(37+i, tb[s][i]);
}

//======================================================
void TMR0_IRQHandler(void)
{
    TIMER_ClearIntFlag(TIMER0);

    if(u8PlayEn) {
        if(u32PlayPos_In >= u32PlayPos_Out) {
            if((u32PlayPos_In-u32PlayPos_Out) > (EPA_MAX_PKT_SIZE+8)) {
                AdjustCodecPll(E_RS_UP);
            } else if((u32PlayPos_In-u32PlayPos_Out) < (EPA_MAX_PKT_SIZE-8)) {
                AdjustCodecPll(E_RS_DOWN);
            } else {
                AdjustCodecPll(E_RS_NONE);
            }
        } else {
            if((u32PlayPos_In+BUFF_LEN-u32PlayPos_Out) > (EPA_MAX_PKT_SIZE+8)) {
                AdjustCodecPll(E_RS_UP);
            } else if((u32PlayPos_In+BUFF_LEN-u32PlayPos_Out) < (EPA_MAX_PKT_SIZE-8)) {
                AdjustCodecPll(E_RS_DOWN);
            } else {
                AdjustCodecPll(E_RS_NONE);
            }
        }
    } else if(u8RecEn) {
    }
}




