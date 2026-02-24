cmake_minimum_required(VERSION 3.20)

# ==============================================================================
# Raspberry Pi Pico SDK
# ==============================================================================

# Select the board (can be overridden: -DPICO_BOARD=pico_w, etc.)
set(PICO_BOARD pico CACHE STRING "Pico board type")

# Pull in the Pico SDK.
include(pico_sdk_import.cmake)

# ==============================================================================
# Project
# ==============================================================================

project(ina219_pico VERSION 0.1 LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Dependencies (INA219 driver)
# ==============================================================================

include(FetchContent)

set(INA219_LOG_LEVEL 3) # info level logging for the example
FetchContent_Declare(ina219
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.."   # points to the driver repo root
)

# --- Use the following for your project
# FetchContent_Declare(ina219
#   GIT_REPOSITORY https://github.com/tblanpied/ina219-cpp-driver.git
#   GIT_TAG        main
# )

FetchContent_MakeAvailable(ina219)

# ==============================================================================
# Pico SDK init
# ==============================================================================

pico_sdk_init()

# ==============================================================================
# Example application
# ==============================================================================

add_executable(ina219_pico
  src/main.cpp
)

pico_set_program_name(ina219_pico "ina219_pico")
pico_set_program_version(ina219_pico "${PROJECT_VERSION}")

# stdio routing
pico_enable_stdio_usb(ina219_pico 1)
pico_enable_stdio_uart(ina219_pico 0)

# Link Pico libs + driver target (exported by the driver (root) CMakeLists)
target_link_libraries(ina219_pico PRIVATE
  ina219
  pico_stdlib
  hardware_i2c
)

# Produce .uf2/.bin/.hex etc.
pico_add_extra_outputs(ina219_pico)

# Flash command using picotool
add_custom_target(flash
  COMMAND picotool load ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.uf2 -f
  DEPENDS ${CMAKE_PROJECT_NAME}
  USES_TERMINAL
)
