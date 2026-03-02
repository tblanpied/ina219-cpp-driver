cmake_minimum_required(VERSION 3.20)
project(ina219_arduino NONE)

set(SKETCH_DIR "${CMAKE_CURRENT_LIST_DIR}/src" CACHE PATH "Folder containing .ino")
set(FQBN "arduino:avr:nano" CACHE STRING "Arduino FQBN")
set(PORT "/dev/ttyACM0" CACHE STRING "Serial port, e.g. /dev/ttyACM0")
set(CPP_STD "gnu++17" CACHE STRING "C++ dialect for Arduino build")

# ==============================================================================
# Dependencies (FetchContent)
# ==============================================================================
include(FetchContent)

# --- INA219 driver
FetchContent_Declare(ina219_driver
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.."   # repo root
)
FetchContent_GetProperties(ina219_driver)
if(NOT ina219_driver_POPULATED)
  FetchContent_Populate(ina219_driver)
endif()
set(INA219_INCLUDE_DIR "${ina219_driver_SOURCE_DIR}/include")

# --- avr-libstdcpp (headers)
FetchContent_Declare(avr_libstdcpp
  GIT_REPOSITORY https://github.com/modm-io/avr-libstdcpp.git
  GIT_TAG master
)
FetchContent_GetProperties(avr_libstdcpp)
if(NOT avr_libstdcpp_POPULATED)
  FetchContent_Populate(avr_libstdcpp)
endif()
set(AVR_LIBSTDCPP_INCLUDE_DIR "${avr_libstdcpp_SOURCE_DIR}/include")

# ==============================================================================
# Arduino CLI build (default)
# ==============================================================================

add_custom_target(compile ALL
  COMMAND arduino-cli compile
          --fqbn ${FQBN}
          --build-path "${CMAKE_BINARY_DIR}"
          --build-property "compiler.cpp.extra_flags=-I${INA219_INCLUDE_DIR} -I${AVR_LIBSTDCPP_INCLUDE_DIR} -std=${CPP_STD}"
          "${SKETCH_DIR}"
  USES_TERMINAL
  VERBATIM
)

# ==============================================================================
# Upload target
# ==============================================================================

add_custom_target(upload
  COMMAND arduino-cli upload
          --fqbn ${FQBN}
          --board-options cpu=atmega328old
          -p ${PORT}
          "${SKETCH_DIR}"
  DEPENDS compile
  USES_TERMINAL
  VERBATIM
)
