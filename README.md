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
  - [Building](#building)
  - [Flashing](#flashing)
- [Integrating the driver in your project](#integrating-the-driver-in-your-project)
  - [Option 1: Copy the header into your project](#option-1-copy-the-header-into-your-project)
  - [Option 2: CMake FetchContent](#option-2-cmake-fetchcontent)
  - [Option 3: Git submodule](#option-3-git-submodule)
- [License](#license)

## Project overview

This repository provides a single-header INA219 driver API (configuration, calibration, measurement reads) plus buildable platform examples.

The driver is platform-agnostic: all I2C access, timing, and optional logging are supplied by a user-implemented `Platform` type that is checked at compile time.

> C++ standard: the project uses **C++20** (concepts). If you need an older standard, you can remove/replace the concept checks (e.g., via `static_assert` + detection idiom), but C++20 is the supported baseline.

## Features

- Single-header integration (`include/ina219/ina219.hpp`).
- Platform-agnostic via a templated `Platform` type (no virtual interface required).
- Type-safe enums for bus range, PGA gain, ADC settings, and operating mode.
- Fluent configuration API: `sensor.configure().busRange(...).pgaGain(...).operatingMode(...);`
  - Auto-apply in the builder destructor when the full expression ends (`;`).
- Calibration helpers based on shunt resistor and either `currentLsb` or a max expected current.
- Measurement helpers:
  - Bus voltage (mV)
  - Shunt voltage (µV)
  - Current (mA) / raw current
  - Power (mW) / raw power
- Address selection via `ina219::Address` mapping A0/A1 wiring to the 7-bit I2C address.
- CMake integration (FetchContent, submodule, or copy header).

## Repository structure

Typical layout (may evolve as examples are added):

```
.
├── CMakeLists.txt
├── include/
│   └── ina219/
│       └── ina219.hpp
├── examples/
│   ├── pico/
│   ├── esp32/
│   └── ...
└── README.md
```

Public API:

```cpp
#include <ina219/ina219.hpp>
```

## API overview

### Constructing the driver

Construct the driver using a `Platform` implementation and an optional I2C address
(default: `Address::A0GndA1Gnd` → `0x40`).

```cpp
#include <ina219/ina219.hpp>

// If your Platform is default-constructible:
ina219::Ina219<MyPlatform> sensor;   // Default address 0x40

// Otherwise provide a pre-configured platform instance (owned by the driver):
ina219::Ina219<MyPlatform> sensor{MyPlatform{ /* ... */ }};
```

### Configuring the sensor (fluent API)

`configure()` returns a `ConfigBuilder`. The configuration is applied when the builder
is destroyed (end of the full expression).

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

**Note**: when reading the bus voltage, the INA219 bus-voltage status flags (CNVR / OVF)
are also read. The driver exposes getters like `lastConversionReady()` and
`lastMathOverflow()`, so you can use them to decide whether data is fresh and whether
current/power results should be trusted.

### Reset and address management

- Reset: `reset()` (optional delay enabled by default)
- Address: `setAddress()` / `getAddress()`

```cpp
sensor.reset();
sensor.setAddress(ina219::Address::A0VsA1Gnd);
```

## Platform abstraction

### How the platform is abstracted

The driver is fully static and uses a class template parameter (`Platform`) to bind
hardware access at compile time.

A C++20 concept is used to describe the platform services required by the driver.
Any type that satisfies the concept can be used as the platform/policy type.

The platform type is responsible for:

- Performing I2C write/read transactions: `i2cWrite`, `i2cRead`
- Providing a millisecond delay: `delayMs`
- Logging (printf-like `fmt` + `va_list`):
  `logDebug`, `logInfo`, `logWarning`, `logError`

### Minimal custom Platform type example

```cpp
#include <cstdarg>
#include <cstddef>
#include <cstdint>

class MyPlatform {
public:
    bool i2cWrite(uint8_t addr, const uint8_t* data, std::size_t len, bool nostop = false) {
        // Implement using your SDK/HAL (addr is 7-bit).
        return true;
    }

    bool i2cRead(uint8_t addr, uint8_t* data, std::size_t len, bool nostop = false) {
        // Implement using your SDK/HAL.
        return true;
    }

    void delayMs(uint32_t ms) {
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

### Building

From the repo root, configure and build an example directory. For the Pico example:

```bash
cmake -S examples/pico -B build
cmake --build build
```

### Flashing

Most embedded examples expose a `flash` target:

```bash
cmake --build build --target flash
```

> You must install the required SDK/toolchain for each platform. See `examples/*/README.md`.

## Integrating the driver in your project

### Option 1: Copy the header into your project

Copy:

- `include/ina219/ina219.hpp`

You can change log level via the `INA219_LOG_LEVEL` macro/definition.

### Option 2: CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(
  ina219
  GIT_REPOSITORY https://github.com/tblanpied/ina219-cpp-driver.git
  GIT_TAG main
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
