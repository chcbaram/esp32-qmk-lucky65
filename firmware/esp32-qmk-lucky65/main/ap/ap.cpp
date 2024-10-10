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
  uint8_t index = 0;

  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 2000)
    {
      pre_time = millis();

      index++;
    }
    delay(1000);
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

