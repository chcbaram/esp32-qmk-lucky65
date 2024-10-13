#include "hw.h"
#include "esp_pm.h"


static void bootMsg(void);




bool hwInit(void)
{
  bspInit();


  cliInit();
  logInit();
  ledInit();
  uartInit();
  uartOpen(_DEF_UART1, 115200);

  logOpen(HW_LOG_CH, 115200);

  bootMsg();
  logPrintf("\r\n[ Bootloader Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", _DEF_BOARD_NAME);
  logPrintf("Booting..Ver  \t\t: %s\r\n", _DEF_FIRMWATRE_VERSION);  
  logPrintf("\n");

  nvsInit();
  resetInit();
  adcInit();
  batteryInit();
  i2cInit();
  eepromInit();
  keysInit();


#if CONFIG_PM_ENABLE
  // Configure dynamic frequency scaling:
  // maximum and minimum frequencies are set in sdkconfig,
  // automatic light sleep is enabled if tickless idle support is enabled.
  esp_pm_config_t pm_config = {
    .max_freq_mhz = 80,
    .min_freq_mhz = 80,
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
    .light_sleep_enable = true
#endif
  };
  ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif // CONFIG_PM_ENABLE

  delay(100);
  usbInit();

  return true;
}

void bootMsg(void)
{
}
