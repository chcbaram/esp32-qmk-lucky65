#include "cdc.h"

#include "qbuffer.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"



static void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event);


static bool      is_init = false;
static qbuffer_t rx_q;
static uint8_t   rx_q_buf[1024];
static uint8_t   rx_usb_buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];


static tinyusb_config_cdcacm_t acm_cfg = {
  .usb_dev                      = TINYUSB_USBDEV_0,
  .cdc_port                     = TINYUSB_CDC_ACM_0,
  .rx_unread_buf_sz             = 64,
  .callback_rx                  = &tinyusb_cdc_rx_callback, // the first way to register a callback
  .callback_rx_wanted_char      = NULL,
  .callback_line_state_changed  = NULL,
  .callback_line_coding_changed = NULL};


bool cdcInit(void)
{
  bool ret;
  esp_err_t esp_ret = ESP_OK;

  qbufferCreate(&rx_q, rx_q_buf, 1024);

  // esp_ret = tusb_cdc_acm_init(&acm_cfg);
  is_init = esp_ret == ESP_OK;

  logPrintf("[%s] cdcInit()\n", is_init ? "OK":"E_");   

  ret = is_init;

  return ret;
}

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
  size_t rx_size = 0;

  esp_err_t ret = tinyusb_cdcacm_read(itf, rx_usb_buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
  if (ret == ESP_OK)
  {
    qbufferWrite(&rx_q, rx_usb_buf, rx_size);
  }
}

bool cdcIsInit(void)
{
  return is_init;
}

uint32_t cdcAvailable(void)
{
  uint32_t ret;
  
  ret = qbufferAvailable(&rx_q);
  return ret;
}

uint8_t cdcRead(void)
{
  uint8_t ret;

  qbufferRead(&rx_q, &ret, 1);

  return ret;
}

uint32_t cdcWrite(uint8_t *p_data, uint32_t length)
{
  int ret;
  
  ret = tinyusb_cdcacm_write_queue(0, p_data, length);
  tinyusb_cdcacm_write_flush(0, 100);
  
  return (uint32_t)ret;
}

uint32_t cdcGetBaud(void)
{
  return 115200;
}