#ifndef BT_HIDH_H_
#define BT_HIDH_H_


#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"


#ifdef _USE_HW_BT_HIDH



typedef struct
{
  uint8_t btn;
  int16_t x;
  int16_t y;
} bt_hidh_mouse_info_t;


bool btHidhInit(void);
bool btHidhBegin(void);
bool btHidhIsBegin(void);
bool btHidhConnect(void);
bool btHidhIsConnect(void);
bool btHidhDisconnect(void);
bool btHidhReconnect(bool enable);
void btHidhStopCmd(uint32_t timeout_ms);

uint32_t btHidhMouseAvailable(void);
bool btHidhMouseFlush(void);
bool btHidhMouseRead(bt_hidh_mouse_info_t *p_info);


#endif


#ifdef __cplusplus
 }
#endif


#endif 
