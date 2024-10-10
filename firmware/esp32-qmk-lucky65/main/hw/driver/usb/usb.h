#ifndef USB_H_
#define USB_H_




#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_USB


#include "usb_hid/usbd_hid.h"



bool usbInit(void);
bool usbIsOpen(void);
bool usbIsConnect(void);
bool usbIsSuspended(void);

#endif


#ifdef __cplusplus
}
#endif




#endif /* SRC_HW_USB_CDC_USB_H_ */
