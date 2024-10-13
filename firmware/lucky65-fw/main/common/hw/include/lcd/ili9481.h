#ifndef ILI9481_H_
#define ILI9481_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"


#ifdef _USE_HW_ILI9481
#include "lcd.h"
#include "ili9481_regs.h"



#define ILI9481_WIDTH     HW_ILI9481_WIDTH
#define ILI9481_HEIGHT    HW_ILI9481_HEIGHT



bool ili9481Init(void);
bool ili9481SetRotate(bool enable);

lcd_driver_t *ili9481GetDriver(void);


#endif

#ifdef __cplusplus
}
#endif

#endif