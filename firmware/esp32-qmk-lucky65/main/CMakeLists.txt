


file(GLOB SRC_FILES 
  ap/*.c
  ap/*.cpp
)

file(GLOB_RECURSE SRC_FILES_RECURSE
  bsp/*.c
  common/*.c 
  hw/*.c
  hw/*.cpp;
)

set(requires 
  "spi_flash"
  "esp_adc"
  "esp_timer"
  "esp_psram"
  "vfs"
  "nvs_flash"
  "esp_hid"
  "esp_mm"
  "tinyusb"
)

idf_component_register(
  SRCS "main.cpp"
  
  SRCS ${SRC_FILES}
  SRCS ${SRC_FILES_RECURSE}
  SRCS ${QMK_SRC_FILES}

  INCLUDE_DIRS ""
  INCLUDE_DIRS "ap"
  INCLUDE_DIRS "ap/modules"
  INCLUDE_DIRS "bsp"
  INCLUDE_DIRS "common"
  INCLUDE_DIRS "common/core"
  INCLUDE_DIRS "common/hw/include"
  INCLUDE_DIRS "hw"
  INCLUDE_DIRS "hw/driver"
  INCLUDE_DIRS "hw/driver/usb"
  INCLUDE_DIRS "hw/driver/ble"
  INCLUDE_DIRS "lib" 

  INCLUDE_DIRS ${QMK_INC_DIR}

  PRIV_REQUIRES soc driver freertos

  REQUIRES "${requires}"
  REQUIRES driver
  )

  target_compile_options(${COMPONENT_LIB} PRIVATE 
  "-Wno-format"
  "-Wno-expansion-to-defined"
  )