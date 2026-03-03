idf_component_register(
  SRCS "main.cpp"
  INCLUDE_DIRS "."
  REQUIRES esp_driver_i2c
)

include(FetchContent)
set(INA219_LOG_LEVEL 3) # info level logging for the example
FetchContent_Declare(ina219
  SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../.."   # points to the driver repo root
)
FetchContent_MakeAvailable(ina219)

# Link the driver library
target_link_libraries(${COMPONENT_LIB} PRIVATE ina219)
