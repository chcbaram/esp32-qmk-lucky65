#include "keys.h"


#ifdef _USE_HW_KEYS
#include "qbuffer.h"
#include "cli.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "soc/io_mux_reg.h"


#define MATRIX_ROWS                 5
#define MATRIX_COLS                 16

#define KEYS_ROWS   MATRIX_ROWS
#define KEYS_COLS   MATRIX_COLS


#define lock()       xSemaphoreTake(mutex_lock, portMAX_DELAY);
#define unLock()     xSemaphoreGive(mutex_lock);
#define lockGpio()   xSemaphoreTake(mutex_gpio, portMAX_DELAY);
#define unLockGpio() xSemaphoreGive(mutex_gpio);

#if CLI_USE(HW_KEYS)
static void cliCmd(cli_args_t *args);
#endif
static void keysThread(void *args);
static bool keysInitGpio(void);


static uint8_t cols_buf[KEYS_COLS];
static SemaphoreHandle_t mutex_lock;
static SemaphoreHandle_t mutex_gpio;

static gpio_num_t rows_gpio_tbl[KEYS_ROWS] = {GPIO_NUM_4,
                                              GPIO_NUM_5,
                                              GPIO_NUM_6,
                                              GPIO_NUM_7,
                                              GPIO_NUM_8,
                                              };
static gpio_num_t cols_gpio_tbl[KEYS_COLS] = {GPIO_NUM_16,
                                              GPIO_NUM_17,
                                              GPIO_NUM_18,
                                              GPIO_NUM_21,
                                              GPIO_NUM_47,
                                              GPIO_NUM_33,
                                              GPIO_NUM_34,
                                              GPIO_NUM_48,
                                              GPIO_NUM_35,
                                              GPIO_NUM_36,
                                              GPIO_NUM_37,
                                              GPIO_NUM_38,
                                              GPIO_NUM_39,
                                              GPIO_NUM_40,
                                              GPIO_NUM_41,
                                              GPIO_NUM_42,
                                              };


static bool is_ready = false;




bool keysInit(void)
{
  mutex_lock = xSemaphoreCreateMutex();
  mutex_gpio = xSemaphoreCreateMutex();


  keysInitGpio();
  keysScan();

  if (xTaskCreate(keysThread, "keysThread", _HW_DEF_RTOS_THREAD_MEM_KEYS, NULL, _HW_DEF_RTOS_THREAD_PRI_KEYS, NULL) != pdPASS)
  {
    logPrintf("[NG] keysThread()\n");   
  }  

#if CLI_USE(HW_KEYS)
  cliAdd("keys", cliCmd);
#endif

  return true;
}

bool keysInitGpio(void)
{
  lockGpio();

  gpio_iomux_out(GPIO_NUM_39, FUNC_MTCK_GPIO39, false);
  gpio_iomux_out(GPIO_NUM_40, FUNC_MTDO_GPIO40, false);
  gpio_iomux_out(GPIO_NUM_41, FUNC_MTDI_GPIO41, false);
  gpio_iomux_out(GPIO_NUM_42, FUNC_MTMS_GPIO42, false);


  for (int i=0; i<KEYS_ROWS; i++)
  {
    gpio_pullup_en(rows_gpio_tbl[i]);
    gpio_pulldown_dis(rows_gpio_tbl[i]);
    gpio_set_direction(rows_gpio_tbl[i], GPIO_MODE_INPUT);
  }

  for (int i=0; i<KEYS_COLS; i++)
  {
    gpio_pullup_dis(cols_gpio_tbl[i]);
    gpio_pulldown_dis(cols_gpio_tbl[i]);
    gpio_set_direction(cols_gpio_tbl[i], GPIO_MODE_OUTPUT);

    gpio_set_level(cols_gpio_tbl[i], _DEF_HIGH);
  }

  unLockGpio();

  return true;
}

bool keysEnterSleep(void)
{
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  gpio_config_t config = {
    .pin_bit_mask = BIT64(GPIO_NUM_18),
    .mode         = GPIO_MODE_INPUT,
    .pull_down_en = true,
    .pull_up_en   = false,
    .intr_type    = GPIO_INTR_DISABLE};

  for (int i=0; i<KEYS_COLS; i++)
  {
    config.pin_bit_mask = BIT64(cols_gpio_tbl[i]);
    gpio_config(&config);
    gpio_wakeup_enable(cols_gpio_tbl[i], GPIO_INTR_HIGH_LEVEL);
  }

  esp_sleep_enable_gpio_wakeup();  
  return true;
}

bool keysExitSleep(void)
{
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  keysInitGpio();
  return true;
}

bool keysIsBusy(void)
{
  return false;
}

bool keysIsReady(void)
{
  bool ret = is_ready;

  is_ready = false;

  return ret;
}

bool keysUpdate(void)
{
  return true;
}

bool keysReadBuf(uint8_t *p_data, uint32_t length)
{
  lock();
  memcpy(p_data, cols_buf, length);
  unLock();
  return true;
}

bool keysGetPressed(uint16_t row, uint16_t col)
{
  bool    ret = false;
  uint8_t row_bit;

  row_bit = cols_buf[col];

  if (row_bit & (1<<row))
  {
    ret = true;
  }

  return ret;
}

void keysScan(void)
{
  uint8_t scan_buf[KEYS_COLS];


  memset(scan_buf, 0, sizeof(scan_buf));

  // lockGpio();
  for (int cols_i = 0; cols_i < KEYS_COLS; cols_i++)
  {
    gpio_set_level(cols_gpio_tbl[cols_i], _DEF_LOW);
    esp_rom_delay_us(10);
    for (int rows_i = 0; rows_i < KEYS_ROWS; rows_i++)
    {
      if (gpio_get_level(rows_gpio_tbl[rows_i]) == 0)
      {
        scan_buf[cols_i] |= (1<<rows_i);
      }
    }
    gpio_set_level(cols_gpio_tbl[cols_i], _DEF_HIGH);
  }
  // unLockGpio();

  lock();
  memcpy(cols_buf, scan_buf, sizeof(scan_buf));
  unLock();

  is_ready = true;
}

void keysThread(void *args)
{
  while(1)
  {
    keysScan();
    delay(1);
  }
}

#if CLI_USE(HW_KEYS)
void cliCmd(cli_args_t *args)
{
  bool ret = false;



  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliShowCursor(false);


    while(cliKeepLoop())
    {
      keysUpdate();
      delay(10);

      cliPrintf("     ");
      for (int cols=0; cols<MATRIX_COLS; cols++)
      {
        cliPrintf("%02d ", cols);
      }
      cliPrintf("\n");

      for (int rows=0; rows<MATRIX_ROWS; rows++)
      {
        cliPrintf("%02d : ", rows);

        for (int cols=0; cols<MATRIX_COLS; cols++)
        {
          if (keysGetPressed(rows, cols))
            cliPrintf("O  ");
          else
            cliPrintf("_  ");
        }
        cliPrintf("\n");
      }
      cliMoveUp(MATRIX_ROWS+1);
    }
    cliMoveDown(MATRIX_ROWS+1);

    cliShowCursor(true);
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("keys info\n");
  }
}
#endif

#endif