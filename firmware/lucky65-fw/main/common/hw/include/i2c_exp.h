#ifndef I2C_EXP_H_
#define I2C_EXP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"

#ifdef _USE_HW_I2C_EXP


typedef enum
{
  I2C_REG_VERSION       = 0x00,
  I2C_REG_POWER_BTN     = 0x01,
  I2C_REG_CHARGE_FLAG   = 0x02,
  I2C_REG_TOUCH_OUT     = 0x03, 
  I2C_REG_I2S_SD_MODE   = 0x04,
  I2C_REG_LCD_TS_RST    = 0x05,
  I2C_REG_LCD_BL        = 0x06,
  I2C_REG_LCD_WR_CMD    = 0x07,
  I2C_REG_LCD_WR_DAT    = 0x08,
  I2C_REG_LCD_BL_PWM    = 0x09,
  I2C_REG_BAT_ADC_UP    = 0x0A,
  I2C_REG_BAT_ADC_L     = 0x0B,
  I2C_REG_BAT_ADC_H     = 0x0C,

} I2cExpAddr_t;


bool i2cExpInit(void);
bool i2cExpIsInit(void);
bool i2cExpRead(I2cExpAddr_t addr, uint8_t *p_data, uint8_t length);
bool i2cExpWrite(I2cExpAddr_t addr, uint8_t *p_data, uint8_t length);


#endif

#ifdef __cplusplus
}
#endif

#endif
