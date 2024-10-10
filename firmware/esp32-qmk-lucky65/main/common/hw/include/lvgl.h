#ifndef LVGL_H_
#define LVGL_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "hw_def.h"

#ifdef _USE_HW_LVGL
#include "lvgl/lvgl.h"


typedef enum 
{
  LVGL_FONT_16,
  LVGL_FONT_20,
  LVGL_FONT_24,
  LVGL_FONT_28,
  LVGL_FONT_32,
} LvglFontType_t;



bool lvglInit(void);
bool lvglUpdate(void);
bool lvglSuspend(void);
bool lvglResume(void);

void *lvglMalloc(size_t size);
void *lvglRealloc(void * p, size_t new_size);
void lvglFree(void * p);

lv_font_t *lvglGetFont(LvglFontType_t font_type);

#endif

#ifdef __cplusplus
}
#endif

#endif