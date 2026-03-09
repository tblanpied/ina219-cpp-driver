cmake_minimum_required(VERSION 3.22)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gcc-arm-none-eabi.cmake"
        CACHE FILEPATH "ARM GNU toolchain file"
    )
endif()

project(ina219_example LANGUAGES C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

add_executable(${PROJECT_NAME})

add_subdirectory(cmake/stm32cubemx)

target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/App/Src/ina219_example.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/App/Inc"
)

target_link_options(${PROJECT_NAME} PRIVATE
    -u _printf_float
)

list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)
list(REMOVE_ITEM CMAKE_CXX_IMPLICIT_LINK_LIBRARIES ob)

include(FetchContent)
set(INA219_LOG_LEVEL 3) # info level logging for the example
FetchContent_Declare(ina219
    SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../.."
)

# Example for pulling the driver from Git instead:
# FetchContent_Declare(ina219
#     GIT_REPOSITORY https://github.com/tblanpied/ina219-cpp-driver.git
#     GIT_TAG        v2.0.0
# )

FetchContent_MakeAvailable(ina219)

target_link_libraries(${PROJECT_NAME}
    stm32cubemx
    ina219
)

set(STM32_PROGRAMMER_CLI_DIR
    "$ENV{HOME}/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin"
    CACHE PATH "Directory containing STM32_Programmer_CLI"
)

find_program(STM32_PROGRAMMER_CLI
    NAMES STM32_Programmer_CLI STM32_Programmer_CLI.exe
    HINTS "${STM32_PROGRAMMER_CLI_DIR}"
    DOC "Path to STM32_Programmer_CLI executable"
)

if(STM32_PROGRAMMER_CLI)
    message(STATUS "STM32_Programmer_CLI found: ${STM32_PROGRAMMER_CLI}")

    add_custom_target(flash
        COMMAND ${STM32_PROGRAMMER_CLI}
                -c port=swd
                -w $<TARGET_FILE:${PROJECT_NAME}>
                -v
                -rst
        DEPENDS ${PROJECT_NAME}
        USES_TERMINAL
        VERBATIM
        COMMENT "Flashing ${PROJECT_NAME} with STM32_Programmer_CLI"
    )
else()
    message(STATUS
        "STM32_Programmer_CLI not found. "
        "Set STM32_PROGRAMMER_CLI_DIR or STM32_PROGRAMMER_CLI in CMake cache to enable the flash target."
    )
endif()
