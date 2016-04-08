#define SEQUENCE_ERASE			0x3
#define SCENE_ADD				0x4
#define SEQUENCE_ADD			0x5
#define DEVICE_SET				0x9
#define SEQUENCE_MD5			0x83
#define SCENE_ACK				0x84
#define SEQUENCE_ACK			0x85
#define DEVICE_STATUS			0x88

#define APP_REGISTER			0x21
#define APP_KEEPALIVE			0x22
#define APP_SETDEV				0x23
#define APP_REG_ACK				0xa1
#define APP_KEEPALIVE_ACK		0xa2
#define APP_SETDEV_ACK			0xa3
#define APP_DEVICE_STATUS		0xa4
#define APP_REG_FAIL			0xa5

#define TOOL_RCU_FIND			0x41
#define TOOL_CONNECT_ACK		0x42
#define TOOL_SET_CONFIG			0x43
#define TOOL_DEVICE_FIND		0x44
#define TOOL_DEVICE_SET			0x45
#define TOOL_INFRED_LEARN		0x46
#define TOOL_INFRED_SEQ_SET		0x47
#define TOOL_INFRED_SAVE_START	0x48
#define TOOL_INFRED_SAVE_REPORT	0x49
#define TOOL_INFRED_LOAD		0x4a
#define TOOL_CONNECT			0xc1
#define TOOL_DEVICE_EXIST		0xc2
#define TOOL_DEVICE_STATUS		0xc3
#define TOOL_INFRED_FINISH		0xc4
#define TOOL_INFRED_SAVE		0xc5
#define TOOL_INFRED_SAVE_FINISH	0xc6
#define TOOL_INFRED_LOAD_REPORT	0xc7
#define TOOL_INPUT_CFG_ACK		0xc8

#define OTH_WL_DEVICE_STATUS	0x60
#define OTH_WL_EVENT_SET		0x61
#define OTH_W_DEVICE_SET		0xe0
#define OTH_W_EVENT_SET			0xe1

// System Settings in Data Flash
#define OPT_BASE			0
#define OPT_IP_ADDRESS		0
#define OPT_SUBNET_MASK		4
#define OPT_ROUTER_IP		8
#define OPT_SERVER_IP		12
#define OPT_ROOM_NUMBER		16
#define OPT_IP_TYPE			32
#define OPT_MAC_ADDRESS		33
#define OPT_RESERVED		39
#define OPT_MAX_LEN			40

#define DFLASH_BASE			0x7F800

// Port defination
#define CFGTOOL_LOCAL_PORT		21224
#define APP_LOCAL_PORT			21222
#define SERVER_LISTEN_PORT 		21221
#define RCU_LOCAL_PORT			21221

#define RXBUFSIZE 	512
#define TXBUFSIZE	512

#define EVM_BOARD

// Firmware Version info
#define SOFT_VER_HIGH		0
#define SOFT_VER_LOW		0
#define SOFT_VER_BUILD		1
#define ENG_BUILD_HIGH		0
#define ENG_BUILD_MID		0
#define ENG_BUILD_LOW		1
