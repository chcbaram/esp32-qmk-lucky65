#ifndef JPEGD_H_
#define JPEGD_H_


#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"


#ifdef _USE_HW_JPEGD




bool jpegdInit(void);
bool jpegdDrawFile(int16_t x, int16_t y, int16_t w, int16_t h, char *file_name);

#endif


#ifdef __cplusplus
 }
#endif


#endif 
