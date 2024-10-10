#include "usbd_hid.h"



#define USB_HID_LOG                 0


#if USB_HID_LOG == 1
#define logDebug(...)                              \
  {                                                \
    logPrintf(__VA_ARGS__);                        \
  }
#else
#define logDebug(...) 
#endif

#include "cli.h"
#include "log.h"
#include "keys.h"
#include "qbuffer.h"



#include "tinyusb.h"
#include "class/hid/hid_device.h"


enum
{
  ITF_ID_KEYBOARD = 0,
  ITF_ID_VIA,
  ITF_ID_COUNT
};

enum
{
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_MOUSE,
  REPORT_ID_COUNT
};


#define TUSB_DESC_TOTAL_LEN      (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
#define HID_KEYBOARD_REPORT_SIZE (HW_KEYS_PRESS_MAX + 2U)

typedef struct
{
  uint8_t  buf[HID_KEYBOARD_REPORT_SIZE];
} report_info_t;


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = 0x0200,

  .bDeviceClass       = 0x00,
  .bDeviceSubClass    = 0x00,
  .bDeviceProtocol    = 0x00,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

  .idVendor           = USB_VID,
  .idProduct          = USB_PID,
  .bcdDevice          = 0x0100,

  .iManufacturer      = 0x01,
  .iProduct           = 0x02,
  .iSerialNumber      = 0x03,

  .bNumConfigurations = 0x01
};

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+
static const uint8_t hid_keyboard_descriptor[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
  TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),
};

static const uint8_t hid_via_descriptor[HID_KEYBOARD_VIA_REPORT_DESC_SIZE] = 
{
  //
  0x06, 0x60, 0xFF, // Usage Page (Vendor Defined)
  0x09, 0x61,       // Usage (Vendor Defined)
  0xA1, 0x01,       // Collection (Application)
  // Data to host
  0x09, 0x62,       //   Usage (Vendor Defined)
  0x15, 0x00,       //   Logical Minimum (0)
  0x26, 0xFF, 0x00, //   Logical Maximum (255)
  0x95, 32,         //   Report Count
  0x75, 0x08,       //   Report Size (8)
  0x81, 0x02,       //   Input (Data, Variable, Absolute)
  // Data from host
  0x09, 0x63,       //   Usage (Vendor Defined)
  0x15, 0x00,       //   Logical Minimum (0)
  0x26, 0xFF, 0x00, //   Logical Maximum (255)
  0x95, 32,         //   Report Count
  0x75, 0x08,       //   Report Size (8)
  0x91, 0x02,       //   Output (Data, Variable, Absolute)
  0xC0              // End Collection
};


const char *hid_string_descriptor[5] =
{
  (char[]){0x09, 0x04},     // 0: is supported language is English (0x0409)
  "BARAM",                  // 1: Manufacturer
  KBD_NAME,                 // 2: Product
  "123456",                 // 3: Serials, should use chip ID
  "HID Keyboard",           // 4: HID
};

static const uint8_t hid_configuration_descriptor[] = {

  TUD_CONFIG_DESCRIPTOR(1,  // Configuration number,
                        2,  // interface count
                        0,  // string index
                        TUSB_DESC_TOTAL_LEN,                // total length
                        TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, // attribute
                        500                                 // power in mA
                        ),

  TUD_HID_DESCRIPTOR(0,                                     // Interface number
                     0,                                     // string index
                     HID_ITF_PROTOCOL_KEYBOARD,             // boot protocol
                     sizeof(hid_keyboard_descriptor),       // report descriptor len
                     HID_EPIN_ADDR,                         // EP In address
                     HID_EPIN_SIZE,                         // size
                     1                                      // polling inerval
                     ),

  TUD_HID_INOUT_DESCRIPTOR(1,                               // Interface number
                           0,                               // string index
                           HID_ITF_PROTOCOL_NONE,           // protocol
                           sizeof(hid_via_descriptor),      // report descriptor len
                           HID_VIA_EP_OUT,                  // EP Out Address
                           HID_VIA_EP_IN,                   // EP In Address
                           64,                              // size
                           10                               // polling interval
                           ),
};


static void (*via_hid_receive_func)(uint8_t *data, uint8_t length) = NULL;
static uint8_t via_hid_usb_report[32];

static qbuffer_t     report_q;
static report_info_t report_buf[128];


#ifdef _USE_HW_CLI
static void cliCmd(cli_args_t *args);
#endif
static void hidThread(void *args);





bool usbHidInit(void)
{
  const tinyusb_config_t tusb_cfg = {
    .device_descriptor        = &desc_device,
    .string_descriptor        = hid_string_descriptor,
    .string_descriptor_count  = sizeof(hid_string_descriptor) / sizeof(hid_string_descriptor[0]),
    .external_phy             = false,
    .configuration_descriptor = hid_configuration_descriptor,
    .self_powered             = false,
    .vbus_monitor_io          = -1,
  };
  bool ret = true;

  qbufferCreateBySize(&report_q, (uint8_t *)report_buf, sizeof(report_info_t), 128); 

  tinyusb_driver_install(&tusb_cfg);


  if (xTaskCreate(hidThread, "hidThread", _HW_DEF_RTOS_THREAD_MEM_HID, NULL, _HW_DEF_RTOS_THREAD_PRI_HID, NULL) != pdPASS)
  {
    logPrintf("[NG] hidThread()\n");   
    ret = false;
  }    

#ifdef _USE_HW_CLI
  cliAdd("usbhid", cliCmd);
#endif  
  return ret;
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf)
{
  uint8_t const *p_ret;

  logPrintf("tud_hid_descriptor_report_cb(%d)\n", itf);

  switch(itf)
  {
    case ITF_ID_VIA:
      p_ret = hid_via_descriptor;
      break;

    default:
      p_ret = hid_keyboard_descriptor;
      break;
  }
  
  return p_ret;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  (void)itf;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  logDebug("tud_hid_get_report_cb()\n");
  logDebug("  itf         : %d\n", itf);
  logDebug("  report_id   : %d\n", report_id);
  logDebug("  report_type : %d\n", (int)report_type);
  logDebug("  reqlen      : %d\n", (int)reqlen);

  return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void) itf;
  (void) report_id;
  (void) report_type;

  logDebug("tud_hid_set_report_cb()\n");
  logDebug("  itf         : %d\n", itf);
  logDebug("  report_id   : %d\n", report_id);
  logDebug("  report_type : %d\n", (int)report_type);
  logDebug("  reqlen      : %d\n", (int)bufsize);

  switch(itf)
  {
    case ITF_ID_KEYBOARD:
      break;

    case ITF_ID_VIA:
      memcpy(via_hid_usb_report, buffer, HID_VIA_EP_SIZE);
      if (via_hid_receive_func != NULL)
      {
        via_hid_receive_func(via_hid_usb_report, HID_VIA_EP_SIZE);
      }
      tud_hid_n_report(itf, report_id, via_hid_usb_report, HID_VIA_EP_SIZE);
      break;
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t itf, uint8_t const* report, uint16_t len)
{
  (void) itf;
  (void) report;
  (void) len;

  // logPrintf("tud_hid_report_complete_cb()\n");
  // logPrintf("  itf         : %d\n", itf);
  // logPrintf("  len         : %d\n", (int)len);                              
}                              

bool usbHidSetViaReceiveFunc(void (*func)(uint8_t *, uint8_t))
{
  via_hid_receive_func = func;
  return true;
}

bool usbHidSendReport(uint8_t *p_data, uint16_t length)
{
  bool ret = true;

  if (!tud_suspended())
  {
    report_info_t report_info;

    memcpy(report_info.buf, p_data, HID_KEYBOARD_REPORT_SIZE);
    qbufferWrite(&report_q, (uint8_t *)&report_info, 1);   
  }
  else
  {
    tud_remote_wakeup();
    ret = false;
  }

  return ret;
}

bool usbHidSendReportEXK(uint8_t *p_data, uint16_t length)
{
  // exk_report_info_t report_info;

  // if (length > HID_EXK_EP_SIZE)
  //   return false;

  // if (!USBD_is_suspended())
  // {
  //   memcpy(hid_buf_exk, p_data, length);
  //   if (!USBD_HID_SendReportEXK((uint8_t *)hid_buf_exk, length))
  //   {
  //     report_info.len = length;
  //     memcpy(report_info.buf, p_data, length);
  //     qbufferWrite(&report_exk_q, (uint8_t *)&report_info, 1);        
  //   }    
  // }
  // else
  // {
  //   usbHidUpdateWakeUp(&USBD_Device);
  // }
  
  return true;
}

void hidThread(void *args)
{
  bool ret;
  report_info_t report_info;


  while(1)
  {
    if (tud_hid_n_ready(ITF_ID_KEYBOARD))
    {
      if (qbufferAvailable(&report_q) > 0)
      {
        qbufferRead(&report_q, (uint8_t *)&report_info, 1);
        // ret = tud_hid_n_report(ITF_ID_KEYBOARD, REPORT_ID_KEYBOARD, report_info.buf, sizeof(report_info.buf));
        // if (!ret)
        // {
        //   logPrintf("usbHidSendReport() Fail\n");
        // }
      }
    }
    delay(1);
  }
}

#ifdef _USE_HW_CLI
void cliCmd(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("usbhid info\n");
  }
}
#endif
