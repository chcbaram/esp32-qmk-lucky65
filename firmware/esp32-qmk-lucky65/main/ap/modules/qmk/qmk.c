#include "qmk.h"
#include "qmk/port/port.h"
#include "tinyusb.h"


static void cliQmk(cli_args_t *args);
static void idle_task(void);
static void qmkThread(void *args);
static void usbThread(void *args);

static bool is_suspended = false;




bool qmkInit(void)
{
  eeprom_init();
  via_hid_init();

  keyboard_setup();
  keyboard_init();

  
  is_suspended = usbIsSuspended();

  logPrintf("[  ] qmkInit()\n");
  logPrintf("     MATRIX_ROWS : %d\n", MATRIX_ROWS);
  logPrintf("     MATRIX_COLS : %d\n", MATRIX_COLS);
  logPrintf("     DEBOUNCE    : %d\n", DEBOUNCE);

  if (xTaskCreate(qmkThread, "qmkThread", _HW_DEF_RTOS_THREAD_MEM_QMK, NULL, _HW_DEF_RTOS_THREAD_PRI_QMK, NULL) != pdPASS)
  {
    logPrintf("[NG] qmkThread()\n");   
  }  

  if (xTaskCreate(usbThread, "usbThread", _HW_DEF_RTOS_THREAD_MEM_USB, NULL, _HW_DEF_RTOS_THREAD_PRI_USB, NULL) != pdPASS)
  {
    logPrintf("[NG] qmkThread()\n");   
  }  

  cliAdd("qmk", cliQmk);



  return true;
}

void qmkUpdate(void)
{
  keyboard_task();
  eeprom_task();
  idle_task();
}

void qmkThread(void *args)
{
  while(1)
  {
    qmkUpdate();
    delay(1);
  }
}

void usbThread(void *args)
{
  while(1)
  {
    tud_task();
    delay(1);
  }
}

void keyboard_post_init_user(void)
{
#ifdef KILL_SWITCH_ENABLE
  kill_switch_init();
#endif
#ifdef KKUK_ENABLE
  kkuk_init();
#endif
}

bool process_record_user(uint16_t keycode, keyrecord_t *record)
{
#ifdef KILL_SWITCH_ENABLE
  kill_switch_process(keycode, record);
#endif
#ifdef KKUK_ENABLE
  kkuk_process(keycode, record);
#endif
  return true;
}

void idle_task(void)
{
  bool is_suspended_cur;

  is_suspended_cur = usbIsSuspended();
  if (is_suspended_cur != is_suspended)
  {
    if (is_suspended_cur)
    {
      suspend_power_down();
      logPrintf("Enter Suspend\n");
    }
    else
    {
      logPrintf("Exit Suspend\n");
      suspend_wakeup_init();
    }

    is_suspended = is_suspended_cur;
  }

#ifdef KKUK_ENABLE
  kkuk_idle();
#endif
}

void cliQmk(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 2 && args->isStr(0, "clear") && args->isStr(1, "eeprom"))
  {
    eeconfig_init();
    cliPrintf("Clearing EEPROM\n");
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("qmk info\n");
    cliPrintf("qmk clear eeprom\n");
  }
}