#ifndef BLE_H_
#define BLE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_BLE


#include "ble_hid.h"



bool bleInit(void);


#endif


#ifdef __cplusplus
}
#endif

#endif 
