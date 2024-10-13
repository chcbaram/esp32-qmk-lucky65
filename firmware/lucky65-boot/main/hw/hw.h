#ifndef MAIN_HW_HW_H_
#define MAIN_HW_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#include "uart.h"
#include "cli.h"
#include "log.h"
#include "led.h"
#include "nvs.h"
#include "i2c.h"
#include "eeprom.h"
#include "reset.h"
#include "qbuffer.h"
#include "keys.h"
#include "adc.h"
#include "battery.h"
#include "usb.h"

bool hwInit(void);


#ifdef __cplusplus
}
#endif

#endif
