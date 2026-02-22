cmake_minimum_required(VERSION 3.20)

# ------------------------------------------------------------------------------
# Project
# ------------------------------------------------------------------------------
project(ina219 VERSION 0.1.0 LANGUAGES CXX)

# Standard settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(INA219_LOG_LEVEL "Set the log level for the INA219 driver (0=none, 1=error, 2=warning, 3=info, 4=debug)" 0)

# ------------------------------------------------------------------------------
# Driver library target
# Layout:
#   include/ina219/...   public headers
#   src/ina219.cpp       implementation
# ------------------------------------------------------------------------------
add_library(ina219 STATIC
  src/ina219.cpp
)

# Nice, consistent consumer-facing target name
add_library(ina219::ina219 ALIAS ina219)

# Public headers are under include/
target_include_directories(ina219
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compile-time definitions for the driver, including log level
target_compile_definitions(ina219 PUBLIC
  INA219_LOG_LEVEL=${INA219_LOG_LEVEL}
)

target_compile_options(ina219 PRIVATE -Wall -Wextra -Wpedantic -Weffc++ -Wconversion -Wsign-conversion -Werror)
