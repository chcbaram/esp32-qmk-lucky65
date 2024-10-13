#ifndef BLE_HID_H_
#define BLE_HID_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_BLE_HID


bool bleHidInit(void);
bool bleHidIsConnected(void);
bool bleHidSendReport(uint8_t *p_data, uint16_t length);

#endif


#ifdef __cplusplus
}
#endif

#endif
