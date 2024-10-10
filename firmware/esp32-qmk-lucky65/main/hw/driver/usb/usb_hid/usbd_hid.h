#ifndef __USB_HID_H
#define __USB_HID_H

#ifdef __cplusplus
extern "C" {
#endif


#include "hw_def.h"


#define HID_EP_SIZE                                     64U

#define HID_EPIN_ADDR                                   0x81U
#define HID_EPIN_SIZE                                   64U

#define HID_VIA_EP_IN                                   0x84U
#define HID_VIA_EP_OUT                                  0x04U
#define HID_VIA_EP_SIZE                                 32U


#define HID_KEYBOARD_VIA_REPORT_DESC_SIZE               34U
#define HID_KEYBOARD_REPORT_DESC_SIZE                   64U


enum 
{
  USB_HID_LED_NUM_LOCK    = (1 << 0),
  USB_HID_LED_CAPS_LOCK   = (1 << 1),
  USB_HID_LED_SCROLL_LOCK = (1 << 2),
  USB_HID_LED_COMPOSE     = (1 << 3),
  USB_HID_LED_KANA        = (1 << 4)
};

bool usbHidInit(void);
bool usbHidSetViaReceiveFunc(void (*func)(uint8_t *, uint8_t));
bool usbHidSendReport(uint8_t *p_data, uint16_t length);
bool usbHidSendReportEXK(uint8_t *p_data, uint16_t length);
void usbHidSetStatusLed(uint8_t led_bits);


#ifdef __cplusplus
}
#endif

#endif