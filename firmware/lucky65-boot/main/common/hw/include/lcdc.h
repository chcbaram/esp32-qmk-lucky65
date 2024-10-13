#ifndef LCDC_H_
#define LCDC_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "hw_def.h"

#ifdef _USE_HW_LCDC



bool lcdcInit(void);
bool lcdcBegin(uint16_t width, uint16_t height, uint8_t bus_width, uint32_t freq_mhz);
bool lcdcSetCallBack(bool (*p_func)(void));
bool lcdcIsBusy(void);
void *lcdcGetFrameBuffer(uint8_t index);
bool lcdcRefreshFrameBuffer(void *buf);


#endif

#ifdef __cplusplus
}
#endif

#endif