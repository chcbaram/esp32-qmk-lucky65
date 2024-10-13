#include "reset.h"


#ifdef _USE_HW_RESET
#include "rtc.h"
#include "cli.h"
#include "soc/rtc_cntl_reg.h"

#if CLI_USE(HW_RESET)
static void cliReset(cli_args_t *args);
#endif

static const char *reset_bit_str[] = 
  {
    "RESET_BIT_POWER",
    "RESET_BIT_PIN",
    "RESET_BIT_WDG",
    "RESET_BIT_SOFT",
  };

// static const char *mode_bit_str[] = 
//   {
//     "MODE_BIT_BOOT",
//     "MODE_BIT_UPDATE",
//   };

static bool     is_init    = false;
static uint32_t reset_bits = 0;





bool resetInit(void)
{
  bool ret;


  is_init = true;

#if CLI_USE(HW_RESET)  
  cliAdd("reset", cliReset);
#endif

  ret = is_init;
  return ret;
}

void resetLog(void)
{

}

void resetToBoot(void)
{
  REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
  resetToReset();
}

void resetToReset(void)
{
  esp_restart();
}



#if CLI_USE(HW_RESET)
void cliReset(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info"))
  {
    cliPrintf("Reset Bits\n");
    for (int i=0; i<RESET_BIT_MAX; i++)
    {
      if (reset_bits & (1<<i))
      {
        cliPrintf("      %s\n", reset_bit_str[i]);
      }
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "boot"))
  {
    resetToBoot();
    ret = true;
  }

  // if (args->argc == 1 && args->isStr(0, "update"))
  // {
  //   resetSetBootMode(1<<MODE_BIT_UPDATE);
  //   resetToReset();
  //   ret = true;
  // }

  if (args->argc == 1 && args->isStr(0, "reset"))
  {
    resetToReset();
    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("reset info\n");
    cliPrintf("reset boot\n");
    cliPrintf("reset update\n");
    cliPrintf("reset reset\n");
  }
}
#endif

#endif


