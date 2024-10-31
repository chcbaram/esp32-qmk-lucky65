#include "ap.h"
#include "qmk/qmk.h"


static void cliThread(void *args);




void apInit(void)
{
  cliOpen(HW_UART_CH_CLI, 115200);

  if (xTaskCreate(cliThread, "cliThread", _HW_DEF_RTOS_THREAD_MEM_CLI, NULL, _HW_DEF_RTOS_THREAD_PRI_CLI, NULL) != pdPASS)
  {
    logPrintf("[NG] cliThread()\n");   
  }  
  logBoot(false);

  qmkInit();
}


void apMain(void)
{
  uint32_t pre_time;
  bool is_led_on = true;


  ledOn(_DEF_LED1);

  pre_time = millis();
  while(1)
  {
    if (is_led_on && millis()-pre_time >= 500)
    {
      is_led_on = false;
      ledOff(_DEF_LED1);
    }
    delay(100);
  }
}

void cliThread(void *args)
{
  while(1)
  {
    cliMain();
    delay(2);
  }
}

