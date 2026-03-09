# INA219 C++ Driver

A lightweight, modern C++ driver for the Texas Instruments INA219 current/power monitor, designed for embedded projects and easy cross-platform integration via a compile-time platform abstraction.

## Table of Contents

- [Project overview](#project-overview)
- [Features](#features)
- [Repository structure](#repository-structure)
- [API overview](#api-overview)
  - [Constructing the driver](#constructing-the-driver)
  - [Configuring the sensor (fluent API)](#configuring-the-sensor-fluent-api)
  - [Calibration (current/power scaling)](#calibration-currentpower-scaling)
  - [Reading measurements](#reading-measurements)
  - [Reset and address management](#reset-and-address-management)
- [Platform abstraction](#platform-abstraction)
  - [How the platform is abstracted](#how-the-platform-is-abstracted)
  - [Minimal custom Platform type example](#minimal-custom-platform-type-example)
- [Building and running examples](#building-and-running-examples)
- [Integrating the driver in your project](#integrating-the-driver-in-your-project)
  - [Option 1: Copy the headers into your project](#option-1-copy-the-headers-into-your-project)
  - [Option 2: CMake FetchContent](#option-2-cmake-fetchcontent)
  - [Option 3: Git submodule](#option-3-git-submodule)
- [License](#license)

## Project overview

This repository provides a header-only INA219 driver API (configuration, calibration, measurement reads) plus buildable platform examples.

The driver is platform-agnostic: all I2C access, timing, and optional logging are supplied by a user-implemented `Platform` type that is checked at compile time.

> C++ standard: the project uses **C++20** (concepts). A pre-C++20 fallback (detection idiom / type traits) is also provided, but C++20 remains the supported baseline. If your platform doesn't support C++20, you can fallback to C++17.

## Features

- Header-only integration:
  - Public header: `include/ina219/ina219.hpp`
  - Internal implementation details: `include/ina219/details.hpp`
- Platform-agnostic via a templated `Platform` type (no virtual interface required).
- Type-safe enums for bus range, PGA gain, ADC settings, and operating mode.
- Fluent configuration API:
  - `sensor.configure().busRange(...).pgaGain(...).operatingMode(...);`
  - Auto-apply at end of full expression (builder destruction).
- Calibration helpers based on shunt resistor and either `currentLsb` or a max expected current.
- Measurement helpers:
  - Bus voltage (mV)
  - Shunt voltage (µV)
  - Current (mA) / raw current
  - Power (mW) / raw power
- Address selection via `ina219::Address` mapping A0/A1 wiring to the 7-bit I2C address.
- CMake integration (FetchContent, submodule, or copy headers).

## Repository structure

Typical layout (may evolve as examples are added):

```
.
├── CMakeLists.txt
├── include/
│   └── ina219/
│       ├── ina219.hpp
│       └── details.hpp
├── examples/
│   ├── pico_sdk/
│   ├── esp_idf/
│   ├── arduino/
│   ├── stm32/
│   └── ...
└── README.md
```

Public include:

```cpp
#include <ina219/ina219.hpp>
```

> `ina219.hpp` includes `details.hpp` internally. You generally only include `ina219.hpp` in application code.

## API overview

### Constructing the driver

Construct the driver using a `Platform` implementation and an optional I2C address (default: `Address::A0GndA1Gnd` → `0x40`).

```cpp
#include <ina219/ina219.hpp>

// If your Platform is default-constructible:
ina219::Ina219<MyPlatform> sensor;   // Default address 0x40

// Otherwise provide a pre-configured platform instance (owned by the driver):
ina219::Ina219<MyPlatform> sensor{MyPlatform{ /* ... */ }};
```

### Configuring the sensor (fluent API)

`configure()` returns a `ConfigBuilder`. The configuration is applied when the builder is destroyed (end of the full expression).

```cpp
sensor.configure()
    .busRange(ina219::BusRange::V32)
    .pgaGain(ina219::PgaGain::Gain8)
    .busAdcMode(ina219::AdcMode::Adc12bit128Samples)
    .shuntAdcMode(ina219::AdcMode::Adc12bit128Samples)
    .operatingMode(ina219::Mode::ShuntBusVoltageContinuous);
```

If you want the driver to wait after applying config, keep the default `configure(true)` behavior.

### Calibration (current/power scaling)

You can calibrate explicitly with `(rShunt, currentLsb)` using `setCalibration()`, where:

- `rShunt` is the shunt resistor value in ohms (e.g., `0.1` for 100 mΩ)
- `currentLsb` is the desired current resolution in amperes per bit (e.g., `0.0001` for 100 µA/bit)

If you prefer to specify a max expected current, use `setCalibrationMaxCurrent(rShunt, maxExpectedCurrent)`.

Also, you can set the shunt resistor and let the driver pick a matching current LSB automatically using `setShuntResistor()`.

```cpp
// Easiest: board-specific shunt resistor only
sensor.setShuntResistor(0.1);  // 100 mΩ

// Or full control:
sensor.setCalibration(0.1, 0.0001);           // rShunt=0.1Ω, currentLsb=100µA/bit
sensor.setCalibrationMaxCurrent(0.1, 3.2);    // rShunt=0.1Ω, Imax=3.2A (auto currentLsb)
```

You can retrieve the active calibration parameters with:

- `getShuntResistor()`
- `getCurrentLsb()`

### Reading measurements

- Bus voltage: `readBusVoltageMv(uint16_t& mv)`
- Shunt voltage: `readShuntVoltageUv(int32_t& uv)`
- Current: `readCurrentMa(double& ma)` (or `readCurrentRaw(int16_t&)`)
- Power: `readPowerMw(double& mw)` (or `readPowerRaw(int16_t&)`)

```cpp
uint16_t bus_mv = 0;
int32_t  shunt_uv = 0;
double   current_ma = 0;
double   power_mw = 0;

sensor.readBusVoltageMv(bus_mv);      // mV
sensor.readShuntVoltageUv(shunt_uv);  // µV
sensor.readCurrentMa(current_ma);     // mA (requires calibration)
sensor.readPowerMw(power_mw);         // mW (requires calibration)
```

**Note**: when reading the bus voltage, the INA219 bus-voltage status flags (CNVR / OVF) are also read. The driver exposes getters like `lastConversionReady()` and `lastMathOverflow()`, so you can use them to decide whether data is fresh and whether current/power results should be trusted.

### Reset and address management

- Reset: `reset()` (optional delay enabled by default)
- Address: `setAddress()` / `getAddress()`

```cpp
sensor.reset();
sensor.setAddress(ina219::Address::A0VsA1Gnd);
```

## Platform abstraction

### How the platform is abstracted

The driver is fully static and uses a class template parameter (`Platform`) to bind hardware access at compile time.

A C++20 concept describes the platform services required by the driver (with a pre-C++20 fallback trait). Any type that satisfies the required interface can be used as the platform/policy type.

The platform type is responsible for:

- Performing I2C transactions:
  - `i2cWrite(addr, tx, tx_len)` for write-only transactions
  - `i2cWriteRead(addr, tx, tx_len, rx, rx_len)` for the common “register read” pattern (write register pointer then read bytes, typically with repeated-start)
- Providing a millisecond delay: `delayMs(ms)`
- Logging (printf-like `fmt` + `va_list`):
  `logDebug`, `logInfo`, `logWarning`, `logError`

### Minimal custom Platform type example

```cpp
#include <cstdarg>
#include <cstddef>
#include <cstdint>

class MyPlatform {
public:
    bool i2cWrite(std::uint8_t addr, const std::uint8_t* tx, std::size_t tx_len) {
        // Implement using your SDK/HAL (addr is 7-bit).
        (void)addr; (void)tx; (void)tx_len;
        return true;
    }

    bool i2cWriteRead(std::uint8_t addr,
                      const std::uint8_t* tx, std::size_t tx_len,
                      std::uint8_t* rx, std::size_t rx_len) {
        // Implement as a combined transaction (write-then-read),
        // typically with a repeated-start between phases.
        (void)addr; (void)tx; (void)tx_len; (void)rx; (void)rx_len;
        return true;
    }

    void delayMs(std::uint32_t ms) {
        // Implement using your RTOS/HAL timing.
        (void)ms;
    }

    void logDebug(const char* fmt, std::va_list args)   { (void)fmt; (void)args; }
    void logInfo(const char* fmt, std::va_list args)    { (void)fmt; (void)args; }
    void logWarning(const char* fmt, std::va_list args) { (void)fmt; (void)args; }
    void logError(const char* fmt, std::va_list args)   { (void)fmt; (void)args; }
};
```

Usage:

```cpp
ina219::Ina219<MyPlatform> sensor;            // if default-constructible
// or
ina219::Ina219<MyPlatform> sensor(MyPlatform{/*...*/});
```

## Building and running examples

> Brefore, you must install the required SDK/toolchain and tools for each platform. See `examples/*/README.md`.

From the repo root, configure, build and flash using the following commands.

- **pico_sdk**:

  ```bash
  # Build
  cmake -S examples/pico_sdk -B build
  cmake --build build
  # Flash
  cmake --build build --target flash
  ```

- **esp_idf**:

  ```bash
  # Build
  idf.py -B build/ -C examples/esp_idf/ build
  # Flash
  idf.py -B build/ -C examples/esp_idf/ flash
  ```

- **arduino**:

  ```bash
  # Build
  cmake -S examples/arduino -B build -DPORT=/dev/ttyACM0
  cmake --build build
  # Flash
  cmake --build build --target upload
  ```

- **stm32**:

  ```bash
  # Build
  cmake -S examples/stm32 -B build
  cmake --build build
  # Flash
  cmake --build build --target flash
  ```

## Integrating the driver in your project

### Option 1: Copy the headers into your project

Copy the folder `include/ina219` containing the headers:

- `include/ina219/ina219.hpp`
- `include/ina219/details.hpp`

You can change log level via the `INA219_LOG_LEVEL` macro/definition.

### Option 2: CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
  ina219
  GIT_REPOSITORY https://github.com/tblanpied/ina219-cpp-driver.git
  GIT_TAG v2.0.0
)

# Optional: configure the driver before MakeAvailable
set(INA219_LOG_LEVEL 0) # 0=none, 1=error, 2=warning, 3=info, 4=debug

FetchContent_MakeAvailable(ina219)

target_link_libraries(your_app PRIVATE ina219::ina219)
```

### Option 3: Git submodule

```bash
git submodule add https://github.com/tblanpied/ina219-cpp-driver.git external/ina219
```

```cmake
add_subdirectory(external/ina219)
target_link_libraries(your_app PRIVATE ina219::ina219)
```

## License

MIT License (see `LICENSE`).
