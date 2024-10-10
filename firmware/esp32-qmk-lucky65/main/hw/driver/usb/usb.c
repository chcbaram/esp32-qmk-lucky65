#include "usb.h"


#ifdef _USE_HW_USB
#include "usb_hid/usbd_hid.h"
#include "cli.h"
#include "tinyusb.h"
#include "class/hid/hid_device.h"


static bool is_init = false;


#ifdef _USE_HW_CLI
static void cliCmd(cli_args_t *args);
#endif




bool usbInit(void)
{
  bool ret;


  logPrintf("[  ] usbInit()\n");   

  ret = usbHidInit();
  is_init = ret;

  logPrintf("[%s] usbInit()\n", is_init?"OK":"E_");   

#ifdef _USE_HW_CLI
  cliAdd("usb", cliCmd);
#endif
  return ret;
}

bool usbIsOpen(void)
{
  return true;
}

bool usbIsConnect(void)
{
  bool ret = false;

  if (tud_connected() && !tud_suspended())
  {
    ret = true;
  }

  return ret;
}

bool usbIsSuspended(void)
{
  return tud_suspended();
}


#ifdef _USE_HW_CLI
void cliCmd(cli_args_t *args)
{
  bool ret = false;

  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    // uint16_t vid = 0;
    // uint16_t pid = 0;
    // uint8_t *p_data;
    // uint16_t len = 0;


    // if (p_desc != NULL)
    // {
    //   p_data = p_desc->GetDeviceDescriptor(USBD_SPEED_HIGH, &len);
    //   vid = (p_data[ 9]<<8)|(p_data[ 8]<<0);
    //   pid = (p_data[11]<<8)|(p_data[10]<<0);
    // }

    // cliPrintf("USB PID     : 0x%04X\n", vid);
    // cliPrintf("USB VID     : 0x%04X\n", pid);

    while(cliKeepLoop())
    {
      cliPrintf("mounted   : %d\n", tud_mounted());
      cliPrintf("connedted : %d\n", tud_connected());
      cliPrintf("tud_suspended : %d\n", tud_suspended());

      cliPrintf("USB Connect : %d\n", usbIsConnect());
      cliPrintf("USB Open    : %d\n", usbIsOpen());
      cliMoveUp(5);
      delay(100);
    }
    cliMoveDown(5);

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "test"))
  {
    uint8_t keycode[6] = { 0 };
    keycode[0] = HID_KEY_A;

    tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, keycode);
    delay(100);

    keycode[0] = 0;
    tud_hid_keyboard_report(HID_ITF_PROTOCOL_KEYBOARD, 0, keycode);

    ret = true;
  }

  if (ret == false)
  {
    cliPrintf("usb info\n");
  }
}
#endif

#endif