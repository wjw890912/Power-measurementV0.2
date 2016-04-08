#ifndef __INCLUDED_HID_H__
#define __INCLUDED_HID_H__

#include <usbh_core.h>


/* 
 * Constants
 */
#define PATH_SIZE               10 			/*!< maximum depth for Path */
#define USAGE_TAB_SIZE          50 			/*!< Size of usage stack */

/*! Including FEATURE, INPUT and OUTPUT */
#define MAX_REPORT             	300

/*! Size max of Report Descriptor */
#define REPORT_DSC_SIZE       	6144

typedef enum hid_return_t {
	HID_RET_SUCCESS = 0,
	HID_RET_INVALID_PARAMETER,
	HID_RET_NOT_INITIALISED,
	HID_RET_ALREADY_INITIALISED,
	HID_RET_FAIL_FIND_BUSSES,
	HID_RET_FAIL_FIND_DEVICES,
	HID_RET_FAIL_OPEN_DEVICE,
	HID_RET_DEVICE_NOT_FOUND,
	HID_RET_DEVICE_NOT_OPENED,
	HID_RET_DEVICE_ALREADY_OPENED,
	HID_RET_FAIL_CLOSE_DEVICE,
	HID_RET_FAIL_CLAIM_IFACE,
	HID_RET_FAIL_DETACH_DRIVER,
	HID_RET_NOT_HID_DEVICE,
	HID_RET_HID_DESC_SHORT,
	HID_RET_REPORT_DESC_SHORT,
	HID_RET_REPORT_DESC_LONG,
	HID_RET_FAIL_ALLOC,
	HID_RET_OUT_OF_SPACE,
	HID_RET_FAIL_SET_REPORT,
	HID_RET_FAIL_GET_REPORT,
	HID_RET_FAIL_INT_READ,
	HID_RET_NOT_FOUND,
	HID_RET_TIMEOUT,
	HID_RET_IO_ERR
} hid_return;


typedef void (HID_INT_READ_FUNC)(uint8_t *rdata, int data_len);
typedef void (HID_INT_WRITE_FUNC)(uint8_t **wbuff, int *buff_size);

/* 
 * Items
 * -------------------------------------------------------------------------- */
#define SIZE_0                0x00
#define SIZE_1                0x01
#define SIZE_2                0x02
#define SIZE_4                0x03
#define SIZE_MASK             0x03
															
#define TYPE_MAIN             0x00
#define TYPE_GLOBAL           0x04
#define TYPE_LOCAL            0x08
#define TYPE_MASK             0x0C

/* Main items */
#define ITEM_COLLECTION       0xA0
#define ITEM_END_COLLECTION   0xC0
#define ITEM_FEATURE          0xB0
#define ITEM_INPUT            0x80
#define ITEM_OUTPUT           0x90

/* Global items */
#define ITEM_UPAGE            0x04
#define ITEM_LOG_MIN          0x14
#define ITEM_LOG_MAX          0x24
#define ITEM_PHY_MIN          0x34
#define ITEM_PHY_MAX          0x44
#define ITEM_UNIT_EXP         0x54
#define ITEM_UNIT             0x64
#define ITEM_REP_SIZE         0x74
#define ITEM_REP_ID           0x84
#define ITEM_REP_COUNT        0x94

/* Local items */
#define ITEM_USAGE            0x08
#define ITEM_STRING           0x78

/* Long item */
#define ITEM_LONG	      	  0xFC

#define ITEM_MASK             0xFC

/* Attribute Flags */
#define ATTR_DATA_CST         0x01
#define ATTR_NVOL_VOL         0x80


struct usb_dev_handle;

/// @cond HIDDEN_SYMBOLS

/*!
 * Describe a HID Path point 
 */
typedef struct
{
	uint16_t UPage;
	uint16_t Usage;
} HIDNode;

/*!
 * Describe a HID Path
 */
typedef struct
{
	uint8_t  	Size;             /*!< HID Path size */
	HIDNode		Node[PATH_SIZE];  /*!< HID Path */
} HIDPath;

/// @endcond HIDDEN_SYMBOLS

/*!
 * Describe a HID Data with its location in report 
 */
typedef struct
{
	long    	Value;            /*!< HID Object Value                             */
	HIDPath 	Path;             /*!< HID Path                                     */

	uint8_t 	ReportID;         /*!< Report ID, (from incoming report) ???        */
	uint8_t 	Offset;           /*!< Offset of data in report                     */
	uint8_t 	Size;             /*!< Size of data in bit                          */
														
	uint8_t 	Type;             /*!< Type : FEATURE / INPUT / OUTPUT              */
	uint8_t 	Attribute;        /*!< Report field attribute                       */
														
	uint32_t   	Unit;             /*!< HID Unit                                     */
	char    	UnitExp;          /*!< Unit exponent                                */

	long    	LogMin;           /*!< Logical Min                                  */
	long    	LogMax;           /*!< Logical Max                                  */
	long    	PhyMin;           /*!< Physical Min                                 */
	long    	PhyMax;           /*!< Physical Max                                 */
} HIDData;                        /*!< HID data                                     */


/*!
 *  HID parser
 */
typedef struct                                
{
	uint8_t   	ReportDesc[REPORT_DSC_SIZE];  /*!< Store Report Descriptor          */
	uint16_t  	ReportDescSize;               /*!< Size of Report Descriptor        */
	uint16_t  	Pos;                          /*!< Store current pos in descriptor  */
	uint8_t   	Item;                         /*!< Store current Item               */
	long    	Value;                        /*!< Store current Value              */

	HIDData 	Data;                         /*!< Store current environment        */

	uint8_t   	OffsetTab[MAX_REPORT][3];     /*!< Store ID, type & offset of report*/
	uint8_t   	ReportCount;                  /*!< Store Report Count               */
	uint8_t   	Count;                        /*!< Store local report count         */

	uint16_t  	UPage;                        /*!< Global UPage                     */
	HIDNode 	UsageTab[USAGE_TAB_SIZE];     /*!< Usage stack                      */
	uint8_t   	UsageSize;                    /*!< Design number of usage used      */

	uint8_t   	nObject;                      /*!< Count objects in Report Descriptor */
	uint8_t   	nReport;                      /*!< Count reports in Report Descriptor */
} HIDParser;                                  /*!< HID parser                       */


/// @cond HIDDEN_SYMBOLS

/*!@brief Interface description
 *
 * This structure contains information associated with a given USB device
 * interface. The identification information allows multiple HID-class
 * interfaces to be accessed on a single device.
 *
 * Also available are raw and parsed descriptor information.
 */
typedef struct HIDInterface_t {
	int				dev_handle;
	int 			interface;
	char 			id[32];
	HIDData* 		hid_data;
	HIDParser* 		hid_parser;
} HIDInterface;


typedef struct HIDInterfaceMatcher_t {
	uint16_t 	vendor_id;
	uint16_t 	product_id;
	void		*uid;
} HIDInterfaceMatcher;


#define HID_ID_MATCH_ANY 0x0000

/*!@brief Bitmask for selection of debugging messages
 *
 * The values of this enumeration can be combined with the bitwise-OR operator
 * to select which debug messages should be printed. The selection can be set
 * with hid_set_debug(). You can set a file descriptor for error messages with
 * hid_set_debug_stream().
 */
typedef enum HIDDebugLevel_t {
	HID_DEBUG_NONE = 0x0,		//!< Default
	HID_DEBUG_ERRORS = 0x1,	//!< Serious conditions
	HID_DEBUG_WARNINGS = 0x2,	//!< Less serious conditions
	HID_DEBUG_NOTICES = 0x4,	//!< Informational messages
	HID_DEBUG_TRACES = 0x8,	//!< Verbose tracing of functions
	HID_DEBUG_ASSERTS = 0x10,	//!< Assertions for sanity checking
	HID_DEBUG_NOTRACES = HID_DEBUG_ERRORS | HID_DEBUG_WARNINGS | HID_DEBUG_NOTICES | HID_DEBUG_ASSERTS,
				//!< This is what you probably want to start with while developing with libhid
	HID_DEBUG_ALL = HID_DEBUG_ERRORS | HID_DEBUG_WARNINGS | HID_DEBUG_NOTICES | HID_DEBUG_TRACES | HID_DEBUG_ASSERTS
} HIDDebugLevel;

/// @endcond HIDDEN_SYMBOLS

	
#ifdef __cplusplus
extern "C" {
#endif

void     USBH_HidInit(void);
int32_t  USBH_HidOpen(int interface, HIDInterfaceMatcher *matcher, int32_t * i32Hanlde);
int32_t  USBH_HidClose(int32_t i32Hanlde);
int32_t  USBH_HidDumpTree(int32_t i32Hanlde);
int32_t  USBH_HidGetInputReport(int32_t i32Hanlde, int path[], uint32_t depth, char* buffer, uint32_t size);
int32_t  USBH_HidSetOutputReport(int32_t i32Hanlde, int path[], uint32_t depth, char *buffer, uint32_t size);
int32_t  USBH_HidGetFeatureReport(int32_t i32Hanlde, int path[], uint32_t depth, char *buffer, uint32_t size);
int32_t  USBH_HidSetFeatureReport(int32_t i32Hanlde, int path[], uint32_t depth, char * buffer, uint32_t size);
int32_t  USBH_HidGetItemValue(int32_t i32Hanlde, int path[], uint32_t depth, double *value);
int32_t  USBH_HidGetReportSize(int32_t i32Hanlde, uint32_t reportID, uint32_t reportType, uint32_t *size);
int32_t  USBH_HidSetIdle(int32_t i32Hanlde, uint32_t duration, uint32_t report_id);
int32_t  USBH_HidStartIntReadPipe(int32_t i32Hanlde, HID_INT_READ_FUNC *func);
int32_t  USBH_HidStartIntWritePipe(int32_t i32Hanlde, HID_INT_WRITE_FUNC *func);


#ifdef __cplusplus
}
#endif

#endif /* __INCLUDED_HID_H__ */

