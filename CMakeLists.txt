cmake_minimum_required(VERSION 3.20)

project(ina219 VERSION 1.0.0 LANGUAGES CXX)

# Options
option(INA219_LOG_LEVEL "Set the log level for the INA219 driver (0=none, 1=error, 2=warning, 3=info, 4=debug)" 0)

# Driver library target
add_library(ina219 INTERFACE)
add_library(ina219::ina219 ALIAS ina219)

# Public headers are under include/
target_include_directories(ina219 INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Compile-time definitions for the driver, including log level
target_compile_definitions(ina219 INTERFACE
  INA219_LOG_LEVEL=${INA219_LOG_LEVEL}
)

# The driver is using C++20
target_compile_features(ina219 INTERFACE cxx_std_20)
