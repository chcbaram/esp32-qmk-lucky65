# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)


set(KEYBOARD_PATH "/keyboards/baram/Lucky65")

include(main/ap/modules/qmk/CMakeLists.txt)

project(lucky65-fw)


