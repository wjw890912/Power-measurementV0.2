/**************************************************************************//**
 * @file     hid_helper.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 14/05/29 4:23p $ 
 * @brief    NUC472/NUC442 MCU USB Host HID library helper
 *
 * @note
 * Copyright (C) 2013 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/   
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>

#include "NUC472_442.h"
#include "usbh_core.h"

#include "usbh_hid.h"
#include "hid.h"




/// @cond HIDDEN_SYMBOLS

const char ItemSize[4]={0,1,2,4};


/*!
 * Reset HIDParser structure for new parsing.
 * Keep Report descriptor data
 */
void ResetParser(HIDParser* pParser)
{
    pParser->Pos=0;
    pParser->Count=0;
    pParser->nObject=0;
    pParser->nReport=0;

    pParser->UsageSize=0;
    memset(pParser->UsageTab,0,sizeof(pParser->UsageTab));

    memset(pParser->OffsetTab,0,sizeof(pParser->OffsetTab));
    memset(&pParser->Data,0,sizeof(pParser->Data));
}


/*!
   Note: The USB HID specification states that Local items do not
   carry over to the next Main item (version 1.11, section
   6.2.2.8). Therefore the local state must be reset after each main
   item. In particular, any unused usages on the Usage tabs must be
   discarded and must not carry over to the next Main item. Some APC
   equipment actually sends multiple redundant "usage" commands for a
   single control, so resetting the local state is important.

   Also note: UsageTab[0] is used as the usage of the next control,
   even if UsageSize=0. Therefore, this must be initialized */
static void ResetLocalState(HIDParser* pParser)
{
    pParser->UsageSize = 0;
    memset(pParser->UsageTab, 0, sizeof(pParser->UsageTab));
}


/*!
 * @return pointer to current offset value for the Report designated by 
 * @a ReportID and @a ReportType, or @a NULL if not found.
 */
uint8_t* GetReportOffset(HIDParser* pParser, 
                       const uint8_t ReportID, 
                       const uint8_t ReportType)
{
    uint16_t Pos=0;
    while((Pos < MAX_REPORT) && (pParser->OffsetTab[Pos][0] != 0))
    {
        if ((pParser->OffsetTab[Pos][0] == ReportID) && 
            (pParser->OffsetTab[Pos][1] == ReportType))
            return &pParser->OffsetTab[Pos][2];
        Pos++;
    }
    if (Pos < MAX_REPORT)
    {
        /* Increment Report count */
        pParser->nReport++;
        pParser->OffsetTab[Pos][0]=ReportID;
        pParser->OffsetTab[Pos][1]=ReportType;
        pParser->OffsetTab[Pos][2]=0;
        return &pParser->OffsetTab[Pos][2];
    }
    return NULL;
}


/*!
 * Format Value to fit with long format with respect to negative values
 */
long FormatValue(long Value, uint8_t Size)
{
    if (Size == 1) 
        Value = (long)(char)Value;
    else if (Size == 2) 
        Value = (long)(short)Value;
    return Value;
}


/*!
 * Analyze report descriptor stored in HIDParser struct and store
 * local and global context.  Return in pData the last object found.
 *
 * @pre Initialize HIDParser structure with ResetParser() before beginning
 *
 * @return TRUE when there is another item to parse.
 */
int HIDParse(HIDParser* pParser, HIDData* pData)
{
    int Found=0;

    while (!Found && pParser->Pos<pParser->ReportDescSize)
    {
        /* Get new pParser->Item if current pParser->Count is empty */
        if (pParser->Count==0)
        {
            pParser->Item=pParser->ReportDesc[pParser->Pos++];
            pParser->Value=0;
#ifdef WORDS_BIGENDIAN
    {
        int i;
        unsigned long valTmp=0;
        for (i=0;i<ItemSize[pParser->Item & SIZE_MASK];i++)
            {
        memcpy(&valTmp, &pParser->ReportDesc[(pParser->Pos)+i], 1);
        pParser->Value+=valTmp>>((3-i)*8);
        valTmp=0;
            }
    }
#else
            memcpy(&pParser->Value, &pParser->ReportDesc[pParser->Pos], ItemSize[pParser->Item & SIZE_MASK]);
#endif
            /* Pos on next item */
            pParser->Pos+=ItemSize[pParser->Item & SIZE_MASK];
        }

        switch(pParser->Item & ITEM_MASK)
        {
            case ITEM_UPAGE :
            {
                /* Copy UPage in Usage stack */
                pParser->UPage=(uint16_t)pParser->Value;
                break;
            }
            case ITEM_USAGE :
            {
                /* Copy global or local UPage if any, in Usage stack */
                 if((pParser->Item & SIZE_MASK)>2)
                    pParser->UsageTab[pParser->UsageSize].UPage=(uint16_t)(pParser->Value>>16);
                else
                    pParser->UsageTab[pParser->UsageSize].UPage=pParser->UPage;

                /* Copy Usage in Usage stack */
                pParser->UsageTab[pParser->UsageSize].Usage=(uint16_t)(pParser->Value & 0xFFFF);

                /* Increment Usage stack size */
                pParser->UsageSize++;

                break;
            }
            case ITEM_COLLECTION :
            {
                /* Get UPage/Usage from UsageTab and store them in pParser->Data.Path */
                pParser->Data.Path.Node[pParser->Data.Path.Size].UPage=pParser->UsageTab[0].UPage;
                pParser->Data.Path.Node[pParser->Data.Path.Size].Usage=pParser->UsageTab[0].Usage;
                pParser->Data.Path.Size++;
            
                /* Unstack UPage/Usage from UsageTab (never remove the last) */
                if(pParser->UsageSize>0)
                {
                    uint8_t ii=0;
                    while(ii<pParser->UsageSize)
                    {
                        pParser->UsageTab[ii].Usage=pParser->UsageTab[ii+1].Usage;
                        pParser->UsageTab[ii].UPage=pParser->UsageTab[ii+1].UPage;
                        ii++;
                    }
                    /* Remove Usage */
                    pParser->UsageSize--;
                }

                /* Get Index if any */
                if(pParser->Value>=0x80)
                {
                    pParser->Data.Path.Node[pParser->Data.Path.Size].UPage=0xFF;
                    pParser->Data.Path.Node[pParser->Data.Path.Size].Usage=pParser->Value & 0x7F;
                    pParser->Data.Path.Size++;
                }
    ResetLocalState(pParser);
                break;
            }
            case ITEM_END_COLLECTION :
            {
                pParser->Data.Path.Size--;
                /* Remove Index if any */
                if(pParser->Data.Path.Node[pParser->Data.Path.Size].UPage==0xFF)
                    pParser->Data.Path.Size--;
    ResetLocalState(pParser);
                break;
            }
            case ITEM_FEATURE :
            case ITEM_INPUT :
            case ITEM_OUTPUT :
            {
                /* An object was found */
                Found=1;

                /* Increment object count */
                pParser->nObject++;

                /* Get new pParser->Count from global value */
                if(pParser->Count==0)
                {
                    pParser->Count=pParser->ReportCount;
                }

                /* Get UPage/Usage from UsageTab and store them in pParser->Data.Path */
                pParser->Data.Path.Node[pParser->Data.Path.Size].UPage=pParser->UsageTab[0].UPage;
                pParser->Data.Path.Node[pParser->Data.Path.Size].Usage=pParser->UsageTab[0].Usage;
                pParser->Data.Path.Size++;
    
                /* Unstack UPage/Usage from UsageTab (never remove the last) */
                if(pParser->UsageSize>0)
                {
                    uint8_t ii=0;
                    while(ii<pParser->UsageSize)
                    {
                        pParser->UsageTab[ii].UPage=pParser->UsageTab[ii+1].UPage;
                        pParser->UsageTab[ii].Usage=pParser->UsageTab[ii+1].Usage;
                        ii++;
                    }
                    /* Remove Usage */
                    pParser->UsageSize--;
                }

                /* Copy data type */
                pParser->Data.Type=(uint8_t)(pParser->Item & ITEM_MASK);

                /* Copy data attribute */
                pParser->Data.Attribute=(uint8_t)pParser->Value;

                /* Store offset */
                pParser->Data.Offset=*GetReportOffset(pParser, pParser->Data.ReportID, (uint8_t)(pParser->Item & ITEM_MASK));
        
                /* Get Object in pData */
                /* -------------------------------------------------------------------------- */
                memcpy(pData, &pParser->Data, sizeof(HIDData));
                /* -------------------------------------------------------------------------- */

                /* Increment Report Offset */
                *GetReportOffset(pParser, pParser->Data.ReportID, (uint8_t)(pParser->Item & ITEM_MASK)) += pParser->Data.Size;

                /* Remove path last node */
                pParser->Data.Path.Size--;

                /* Decrement count */
                pParser->Count--;
    if (pParser->Count == 0) {
        ResetLocalState(pParser);
    }
                break;
            }
            case ITEM_REP_ID :
            {
                pParser->Data.ReportID=(uint8_t)pParser->Value;
                break;
            }
            case ITEM_REP_SIZE :
            {
                pParser->Data.Size=(uint8_t)pParser->Value;
                break;
            }
            case ITEM_REP_COUNT :
            {
                pParser->ReportCount=(uint8_t)pParser->Value;
                break;
            }
            case ITEM_UNIT_EXP :
            {
                pParser->Data.UnitExp=(char)pParser->Value;
    // Convert 4 bits signed value to 8 bits signed value
    if (pParser->Data.UnitExp > 7)
        pParser->Data.UnitExp|=0xF0;
                break;
            }
            case ITEM_UNIT :
            {
                pParser->Data.Unit=pParser->Value;
                break;
            }
            case ITEM_LOG_MIN :
            {
                pParser->Data.LogMin=FormatValue(pParser->Value, ItemSize[pParser->Item & SIZE_MASK]);
                break;
            }
            case ITEM_LOG_MAX :
            {
                pParser->Data.LogMax=FormatValue(pParser->Value, ItemSize[pParser->Item & SIZE_MASK]);
                break;
            }
            case ITEM_PHY_MIN :
            {
                pParser->Data.PhyMin=FormatValue(pParser->Value, ItemSize[pParser->Item & SIZE_MASK]);
                break;
            }
            case ITEM_PHY_MAX :
            {
                pParser->Data.PhyMax=FormatValue(pParser->Value, ItemSize[pParser->Item & SIZE_MASK]);
                break;
            }
            case ITEM_LONG :
            {
                /* can't handle long items, but should at least skip them */
                pParser->Pos+=(uint8_t)(pParser->Value & 0xff);
            }
        }
    } /* while(!Found && pParser->Pos<pParser->ReportDescSize) */

    if (pParser->Data.Path.Size>=PATH_SIZE)
    {
        HID_DBGMSG("pParser->Data.Path.Size>=PATH_SIZE!\n");
        return HID_RET_OUT_OF_SPACE;
    }
    if (pParser->ReportDescSize>=REPORT_DSC_SIZE)
    {
        HID_DBGMSG("pParser->ReportDescSize>=REPORT_DSC_SIZE!\n");
        return HID_RET_OUT_OF_SPACE;
    }
    if (pParser->UsageSize>=USAGE_TAB_SIZE)
    {
        HID_DBGMSG("pParser->UsageSize>=USAGE_TAB_SIZE!\n");
        return HID_RET_OUT_OF_SPACE;
    }
    //if (pParser->Data.ReportID >= MAX_REPORT)
    //{
    //  HID_DBGMSG("pParser->Data.ReportID>=MAX_REPORT!\n");
    //  return HID_RET_OUT_OF_SPACE;
    //}
    return Found;
}


hid_return hid_prepare_parser(HIDInterface* const hidif)
{
    int     i;

    ResetParser(hidif->hid_parser);

    HID_DBGMSG("dumping the raw report descriptor\n");

    HID_DBGMSG("0x%03x: ", 0);
    for (i = 0; i < hidif->hid_parser->ReportDescSize; i++) 
    {
        if (!(i % 8)) 
        {
            if (i != 0)
            {
                HID_DBGMSG("0x%03x: ", i);
            }

            HID_DBGMSG("0x%02x ", (int)(hidif->hid_parser->ReportDesc[i]));
        }
    }
    HID_DBGMSG("\n\n");
    
    /* TODO: the return value here should be used, no? */
    HIDParse(hidif->hid_parser, hidif->hid_data);
    return HID_RET_SUCCESS;
}


static void hid_prepare_parse_path(HIDInterface* hidif, int path[], uint32_t depth)
{
    unsigned int i = 0;

    //ASSERT(hid_is_opened(hidif));
    //ASSERT(hidif->hid_data);

    //HID_DBGMSG("preparing search path of depth %d for parse tree...", depth);
    for (; i < depth; ++i) {
        hidif->hid_data->Path.Node[i].UPage = path[i] >> 16;
        hidif->hid_data->Path.Node[i].Usage = path[i] & 0x0000ffff;
    }

    hidif->hid_data->Path.Size = depth;

    //HID_DBGMSG("search path prepared for parse tree.\n");
}


/*!
 * Get pData characteristics from pData->Path or from pData->ReportID/Offset.
 * @return TRUE if object was found
 */
int FindObject(HIDParser* pParser, HIDData* pData)
{
    HIDData FoundData;

    ResetParser(pParser);

    while (HIDParse(pParser, &FoundData))
    {
        if ((pData->Path.Size > 0) && 
            (FoundData.Type == pData->Type) &&
            (memcmp(FoundData.Path.Node, pData->Path.Node, (pData->Path.Size)*sizeof(HIDNode))==0))
        {
            memcpy(pData, &FoundData, sizeof(HIDData));
            return 1;
        }
        /* Found by ReportID/Offset */
        else if ((FoundData.ReportID==pData->ReportID) && 
                 (FoundData.Type==pData->Type) &&
                 (FoundData.Offset==pData->Offset))
        {
            memcpy(pData, &FoundData, sizeof(HIDData));
            return 1;
        }
    }
    return 0;
}


hid_return hid_format_path(char* buffer, uint32_t length, int path[], uint32_t depth)
{
    uint8_t    ITEMSIZE = sizeof("0xdeadbeef");
    uint32_t   i = 0;

    if (!buffer) 
    {
        HID_DBGMSG("cannot format path into NULL buffer.\n");
        return HID_RET_INVALID_PARAMETER;
    }

    HID_DBGMSG("formatting device path...\n");
    for (; i < depth; ++i) 
    {
        if (length < ITEMSIZE) 
        {
            HID_DBGMSG("not enough space in buffer to finish formatting of path.\n");
            return HID_RET_OUT_OF_SPACE;
        }
        snprintf(buffer + i * ITEMSIZE, ITEMSIZE + 1, "0x%08x.", path[i]);
        length -= ITEMSIZE;
    }
    buffer[i * ITEMSIZE - 1] = '\0';

    return HID_RET_SUCCESS;
}

extern int hid_is_opened(HIDInterface * hidif);

hid_return hid_find_object(HIDInterface* hidif, int path[], uint32_t depth)
{
    uint8_t   ITEMSIZE = sizeof("0xdeadbeef");
    char     * buffer;

    if (!hid_is_opened(hidif)) {
        HID_DBGMSG("cannot prepare parser of unopened HIDinterface.");
        return HID_RET_DEVICE_NOT_OPENED;
    }
    //ASSERT(hidif->hid_parser);
    //ASSERT(hidif->hid_data);
    
    hid_prepare_parse_path(hidif, path, depth);

    if (FindObject(hidif->hid_parser, hidif->hid_data) == 1) {
        HID_DBGMSG("found requested item.\n");
        return HID_RET_SUCCESS;
    }

    buffer = (char*)malloc(depth * ITEMSIZE);
    if (buffer == NULL)
    {
        HID_DBGMSG("Failed to allocate memory buffer size %d!\n", depth * ITEMSIZE);
        return HID_RET_FAIL_ALLOC;
    }
    hid_format_path(buffer, depth * ITEMSIZE, path, depth); 
    HID_DBGMSG("can't find requested item %s of USB device %s.\n", buffer, hidif->id);
    free(buffer);
    
    return HID_RET_NOT_FOUND;
}

/// @endcond HIDDEN_SYMBOLS


/** @addtogroup NUC472_442_Device_Driver NUC472/NUC442 Device Driver
  @{
*/

/** @addtogroup NUC472_442_USBH_HID_Driver USB Host HID Driver
  @{
*/

/** @addtogroup NUC472_442_USBH_HID_EXPORTED_FUNCTIONS USB Host HID Driver Exported Functions
  @{
*/

/**
  * @brief  Fully dump the HID reports to debug port output. 
 *  @param[in] i32Hanlde   HID interface handle.
 *  @retval   0   Success
 *  @retval   Otherwise   Failed
 */
int32_t  USBH_HidDumpTree(int32_t i32Hanlde)
{
    HIDInterface * hidif = (HIDInterface *)i32Hanlde;
    unsigned int i = 0;

    ResetParser(hidif->hid_parser);
    
    while (HIDParse(hidif->hid_parser, hidif->hid_data)) 
    {
        HID_DBGMSG("  path: ");
        for (i = 0; i < hidif->hid_data->Path.Size; ++i) 
        {
            HID_DBGMSG("0x%08x", (hidif->hid_data->Path.Node[i].UPage << 16)
                    | hidif->hid_data->Path.Node[i].Usage);
            if ((signed)i < (hidif->hid_data->Path.Size - 1)) 
            {
                HID_DBGMSG(".");
            }
        }
        HID_DBGMSG("; type: 0x%02x\n", hidif->hid_data->Type);
    }

    ResetParser(hidif->hid_parser);

    return HID_RET_SUCCESS;
}


/*@}*/ /* end of group NUC472_442_USBH_HID_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NUC472_442_USBH_HID_Driver */

/*@}*/ /* end of group NUC472_442_Device_Driver */

/*** (C) COPYRIGHT 2013 Nuvoton Technology Corp. ***/

