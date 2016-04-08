#include "stdint.h"

// server.c
void server_init(void);
void ConnectToServer(void);

// cfgtool.c
void cfgtool_init(void);
void DefaultSettings(void);
void LoadSysSettings(void);
void SaveSysSettings(void);
void RcuToolSend(uint8_t *pBuf, uint16_t sLen);

// serial.c
void serial_init(void);

// app.c
void app_init(void);
