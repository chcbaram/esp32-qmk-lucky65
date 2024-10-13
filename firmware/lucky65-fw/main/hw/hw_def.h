#ifndef HW_HW_DEF_H_
#define HW_HW_DEF_H_


#include "def.h"
#include "bsp.h"
#include QMK_KEYMAP_CONFIG_H


#define _DEF_FIRMWATRE_VERSION    "V241010R1"
#define _DEF_BOARD_NAME           "ESP32-QMK-LUCKY65"


#define _HW_DEF_RTOS_THREAD_PRI_CLI           5
#define _HW_DEF_RTOS_THREAD_PRI_QMK           5
#define _HW_DEF_RTOS_THREAD_PRI_KEYS          5
#define _HW_DEF_RTOS_THREAD_PRI_HID           5
#define _HW_DEF_RTOS_THREAD_PRI_BATTERY       5
#define _HW_DEF_RTOS_THREAD_PRI_USB           5

#define _HW_DEF_RTOS_THREAD_MEM_CLI           (8*1024)
#define _HW_DEF_RTOS_THREAD_MEM_QMK           (8*1024)
#define _HW_DEF_RTOS_THREAD_MEM_KEYS          (4*1024)
#define _HW_DEF_RTOS_THREAD_MEM_HID           (8*1024)
#define _HW_DEF_RTOS_THREAD_MEM_BATTERY       (2*1024)
#define _HW_DEF_RTOS_THREAD_MEM_USB           (4*1024)


#define _USE_HW_RTOS
#define _USE_HW_NVS
#define _USE_HW_USB
#define _USE_HW_EEPROM
#define _USE_HW_RESET
#define _USE_HW_BLE
#define _USE_HW_BLE_HID
#define _USE_HW_BATTERY
#define _USE_HW_CDC


#define _USE_HW_LED
#define      HW_LED_MAX_CH          1

#define _USE_HW_UART
#define      HW_UART_MAX_CH         2
#define      HW_UART_CH_CLI         _DEF_UART2

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    32
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    8
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_CLI_GUI
#define      HW_CLI_GUI_WIDTH       80
#define      HW_CLI_GUI_HEIGHT      24

#define _USE_HW_LOG
#define      HW_LOG_CH              _DEF_UART1
#define      HW_LOG_BOOT_BUF_MAX    1024
#define      HW_LOG_LIST_BUF_MAX    1024

#define _USE_HW_I2C
#define      HW_I2C_MAX_CH          1

#define _USE_HW_KEYS
#define      HW_KEYS_PRESS_MAX      6

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         4

#define _USE_HW_ADC                 
#define      HW_ADC_MAX_CH          1


//-- CLI
//
#define _USE_CLI_HW_EEPROM          1
#define _USE_CLI_HW_I2C             1
#define _USE_CLI_HW_KEYS            1
#define _USE_CLI_HW_RESET           1



#define __WEAK                      __attribute__((weak))


#endif 
