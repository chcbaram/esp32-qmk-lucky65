#include "ble_hid.h"


#ifdef _USE_HW_BLE_HID

#define BLE_HID_LOG                 1
#if BLE_HID_LOG == 1
#define logDebug(...)                              \
  {                                                \
    logPrintf(__VA_ARGS__);                        \
  }
#else
#define logDebug(...) 
#endif

#define HID_KEYBOARD_REPORT_SIZE (HW_KEYS_PRESS_MAX + 2U)


#include "cli.h"
#include "tinyusb.h"
#include "battery.h"
#include "keys.h"

#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_sleep.h"
#include "esp_hidd.h"
#include "driver/esp_hid_gap.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"


typedef struct
{
  TaskHandle_t    task_hdl;
  esp_hidd_dev_t *hid_dev;
  uint8_t         protocol_mode;
  uint8_t        *buffer;
} ble_hid_param_t;


#ifdef _USE_HW_CLI
static void cliCmd(cli_args_t *args);
#endif
static void ble_hidd_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data);
static void bleHidThread(void *args);
static bool bleHidSetup(void);

const unsigned char hid_keyboard_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (Num Lock)
    0x29, 0x05,        //   Usage Maximum (Kana)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x03,        //   Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x29, 0xFF,        //   Usage Maximum (0x65)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection

    // 65 bytes
};

static esp_hid_raw_report_map_t ble_report_maps[] = {
  {.data = hid_keyboard_descriptor,
   .len  = sizeof(hid_keyboard_descriptor)},
};

static esp_hid_device_config_t ble_hid_config = {
    .vendor_id          = USB_VID,
    .product_id         = USB_PID+1,
    .version            = 0x0100,
    .device_name        = KBD_NAME "-BLE",
    .manufacturer_name  = "BARAM",
    .serial_number      = "1234567890",
    .report_maps        = ble_report_maps,
    .report_maps_len    = 1
};

static ble_hid_param_t ble_hid_param = {0};
static bool is_connected = false;
static bool is_sleep = false;




bool bleHidInit(void)
{
  bool ret = true;
 

  if (xTaskCreate(bleHidThread, "bleHidThread", _HW_DEF_RTOS_THREAD_MEM_HID, NULL, _HW_DEF_RTOS_THREAD_PRI_HID, NULL) != pdPASS)
  {
    logPrintf("[NG] bleHidThread()\n");   
    ret = false;
  }   

#ifdef _USE_HW_CLI
  cliAdd("blehid", cliCmd);
#endif
  return ret;
}

bool bleHidSetup(void)
{
  bool ret = true;
  esp_err_t esp_ret;


  logPrintf("[  ] esp_hid_gap_init()\n");
  esp_ret = esp_hid_gap_init(HID_DEV_MODE);
  if (esp_ret != ESP_OK)
  {
    logPrintf("[E_] esp_hid_gap_init()\n");
    return false;
  }

  logPrintf("[  ] esp_hid_ble_gap_adv_init()\n");
  esp_ret = esp_hid_ble_gap_adv_init(ESP_HID_APPEARANCE_KEYBOARD, ble_hid_config.device_name);
  if (esp_ret != ESP_OK)
  {
    logPrintf("[E_] esp_hid_ble_gap_adv_init()\n");
    return false;
  }

  logPrintf("[  ] esp_ble_gatts_register_callback()\n");
  esp_ret = esp_ble_gatts_register_callback(esp_hidd_gatts_event_handler);
  if (esp_ret != ESP_OK)
  {
    logPrintf("[E_] esp_ble_gatts_register_callback()\n");
    return false;
  }

  logPrintf("[  ] esp_hidd_dev_init()\n");
  esp_ret = esp_hidd_dev_init(&ble_hid_config, ESP_HID_TRANSPORT_BLE, ble_hidd_event_callback, &ble_hid_param.hid_dev);
  if (esp_ret != ESP_OK)
  {
    logPrintf("[E_] esp_hidd_dev_init()\n");
    return false;
  }

  return ret;
}

bool bleHidIsConnected(void)
{
  return is_connected;
}

bool bleHidSendReport(uint8_t *p_data, uint16_t length)
{
  bool ret = true;

  if (is_connected)
  {
    esp_err_t esp_ret;
    static uint8_t buffer[8] = {0};

    memcpy(buffer, p_data, HID_KEYBOARD_REPORT_SIZE); 
    esp_ret = esp_hidd_dev_input_set(ble_hid_param.hid_dev, 0, 1, buffer, 8);
    if (esp_ret != ESP_OK)
    {
      logPrintf("esp_hidd_dev_input_set() Fail\n");
    }
  }
  else
  {
    // logPrintf("key in disconnected\n");
  }
  return ret;
}

void ble_hidd_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data)
{
  esp_hidd_event_t       event = (esp_hidd_event_t)id;
  esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;


  switch (event)
  {
    case ESP_HIDD_START_EVENT:
      {
        logDebug("START\n");
        esp_hid_ble_gap_adv_start();
        break;
      }
    case ESP_HIDD_CONNECT_EVENT:
      {
        logDebug("CONNECT\n");
        is_connected = true;
        break;
      }
    case ESP_HIDD_PROTOCOL_MODE_EVENT:
      {
        logDebug("PROTOCOL MODE[%u]: %s\n", param->protocol_mode.map_index, param->protocol_mode.protocol_mode ? "REPORT" : "BOOT");
        break;
      }
    case ESP_HIDD_CONTROL_EVENT:
      {
        logDebug("CONTROL[%u]: %sSUSPEND\n", param->control.map_index, param->control.control ? "EXIT_" : "");
        if (param->control.control)
        {
          // exit suspend
          // ble_hid_task_start_up();
          logPrintf("ble_hid_task_start_up()\n");
          is_connected = true;
        }
        else
        {
          // suspend
          // ble_hid_task_shut_down();
          logPrintf("ble_hid_task_shut_down()\n");
          is_connected = false;
        }
        break;
      }
    case ESP_HIDD_OUTPUT_EVENT:
      {
        // logDebug("OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:\n", param->output.map_index, esp_hid_usage_str(param->output.usage), param->output.report_id, param->output.length);
        // ESP_LOG_BUFFER_HEX(TAG, param->output.data, param->output.length);
        break;
      }
    case ESP_HIDD_FEATURE_EVENT:
      {
        logDebug("FEATURE[%u]: %8s ID: %2u, Len: %d, Data:\n", param->feature.map_index, esp_hid_usage_str(param->feature.usage), param->feature.report_id, param->feature.length);
        // ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
        break;
      }
    case ESP_HIDD_DISCONNECT_EVENT:
      {
        logDebug("DISCONNECT: %s\n", esp_hid_disconnect_reason_str(esp_hidd_dev_transport_get(param->disconnect.dev), param->disconnect.reason));
        // ble_hid_task_shut_down();
        is_connected = false;
        logPrintf("ble_hid_task_shut_down()\n");

        if (!is_sleep)
        {
          esp_hid_ble_gap_adv_start();
        }
        break;
      }
    case ESP_HIDD_STOP_EVENT:
      {
        logDebug("STOP\n");
        break;
      }
    default:
      logDebug("EVENT %d\n", event);
      break;
  }
  return;
}

void bleHidThread(void *args)
{
  uint32_t pre_time;  
  uint8_t bat_level = 0;


  bleHidSetup();

  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();
      if (is_connected)
      {
        bat_level = batteryGetPercent();
        esp_hidd_dev_battery_set(ble_hid_param.hid_dev, bat_level);
      }
    }
    delay(1);
  }
}
extern esp_bd_addr_t remote_device;
extern esp_err_t esp_hid_ble_gap_adv_start2(void);

#ifdef _USE_HW_CLI
void cliCmd(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "send") == true)
  {
    static uint8_t buffer[8] = {0};

    buffer[2] = HID_KEY_A;
    esp_hidd_dev_input_set(ble_hid_param.hid_dev, 0, 1, buffer, 8);
    delay(100);
    memset(buffer, 0, sizeof(uint8_t) * 8);
    esp_hidd_dev_input_set(ble_hid_param.hid_dev, 0, 1, buffer, 8);    
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "sleep") == true)
  {
    keysEnterSleep();

    is_sleep = true;
    esp_ble_gap_disconnect(remote_device);
    delay(100);
    esp_light_sleep_start();
    cliPrintf("Wake Up\n");

    is_sleep = false;
    keysExitSleep();
    esp_hid_ble_gap_adv_start();

    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("blehid info\n");
    cliPrintf("blehid send\n");
    cliPrintf("blehid sleep\n");    
  }
}
#endif
#endif