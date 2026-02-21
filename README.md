# INA219 C++ Driver

A lightweight, modern C++ driver for the Texas Instruments INA219 current / power monitor, designed for embedded projects and easy cross-platform integration via a small “provider” abstraction.

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
- [Provider abstraction](#provider-abstraction)
  - [What is `ina219::Provider`?](#what-is-ina219provider)
  - [Minimal custom Provider example](#minimal-custom-provider-example)
  - [Notes on I2C transactions](#notes-on-i2c-transactions)
- [Building and running examples](#building-and-running-examples)
  - [Using the build script](#using-the-build-script)
  - [Manual CMake (fallback)](#manual-cmake-fallback)
- [Integrating the driver in your project](#integrating-the-driver-in-your-project)
  - [Option 1: CMake FetchContent (recommended)](#option-1-cmake-fetchcontent-recommended)
  - [Option 2: add_subdirectory (submodule or vendored)](#option-2-add_subdirectory-submodule-or-vendored)
- [Typical usage scenarios (wiring + API)](#typical-usage-scenarios-wiring--api)
  - [Scenario A: Single sensor, default address](#scenario-a-single-sensor-default-address)
  - [Scenario B: Multiple INA219 devices on one I2C bus](#scenario-b-multiple-ina219-devices-on-one-i2c-bus)
  - [Scenario C: Measure only bus voltage (no current)](#scenario-c-measure-only-bus-voltage-no-current)
  - [Scenario D: Current + power monitoring (typical)](#scenario-d-current--power-monitoring-typical)
  - [Scenario E: Low power / sleep](#scenario-e-low-power--sleep)
- [License](#license)

## Project overview

This repository provides a single-header-friendly INA219 driver API (configuration, calibration, and measurement reads) plus buildable platform examples.  
The driver is platform-agnostic: all I2C, timing, and optional logging are supplied by a user-implemented `ina219::Provider`.

## Features

- Type-safe configuration enums for bus range, PGA gain, ADC averaging/resolution, and operating mode.
- Fluent configuration API: `sensor.configure().busRange(...).pgaGain(...).operatingMode(...);` with auto-apply when the builder goes out of scope.
- Calibration helpers based on shunt resistor and either `currentLsb` or a max expected current.
- Direct measurement helpers: bus voltage (mV), shunt voltage (µV), current (mA), and power (mW).
- Address selection via a dedicated `ina219::Address` enum mapping A0/A1 pin wiring to the 7-bit I2C address.
- CMake integration examples (FetchContent / add_subdirectory).

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
├── scripts/
│   └── build.sh
└── README.md
```

The public API is exposed through `#include <ina219/ina219.hpp>`.

## API overview

### Constructing the driver

The driver is constructed from a `Provider` implementation and an optional I2C address (default is `Address::A0_GND_A1_GND` → 0x40).

> Details on the `Provider` abstraction and how to implement it for your platform are covered in the [Provider abstraction](#provider-abstraction) section below.

```cpp
#include <ina219/ina219.hpp>

MyProvider provider;               // You implement this
ina219::Ina219 sensor{provider};   // Default address 0x40
```

### Configuring the sensor (fluent API)

`configure()` returns a `ConfigBuilder` that applies the configuration in its destructor when the full expression ends (i.e., at the `;`).

```cpp
sensor.configure()
    .busRange(ina219::BusRange::V32)
    .pgaGain(ina219::PgaGain::Gain8)
    .busAdcMode(ina219::AdcMode::Adc12bit128Samples)
    .shuntAdcMode(ina219::AdcMode::Adc12bit128Samples)
    .operatingMode(ina219::Mode::ShuntBusVoltageContinuous);
```

If you want the driver to wait after applying config, keep the default `configure(true)` delay behavior.

### Calibration (current/power scaling)

You can calibrate explicitly with `(rShunt, currentLsb)` using `setCalibration()`, where `rShunt` is the value of your shunt resistor in ohms, and `currentLsb` is the desired current resolution in amps per bit (e.g., 0.0001 for 100µA/bit).

If you prefer to specify a max expected current with your already-chosen shunt resistor, use `setCalibrationMaxCurrent(rShunt, maxExpectedCurrent)`.

> The current LSB is calculated as `currentLsb = maxExpectedCurrent / 2^15`.

Also, you can set the shunt resistor and let the driver pick a matching current LSB automatically using `setShuntResistor()`.

> The current LSB is calculated as `currentLsb = maxExpectedCurrent / 2^15`, where `maxExpectedCurrent = 320mV / rShunt` is the maximum current that can be measured without overflow for the given shunt resistor with a max shunt voltage of 320mV.

```cpp
// Easiest: board-specific shunt resistor only
sensor.setShuntResistor(0.1);  // 100 mΩ

// Or full control:
sensor.setCalibration(0.1, 0.0001);           // rShunt=0.1Ω, currentLsb=100µA/bit
sensor.setCalibrationMaxCurrent(0.1, 3.2);    // rShunt=0.1Ω, Imax=3.2A (auto currentLsb)
```

You can retrieve the active calibration parameters with `getShuntResistor()` and `getCurrentLsb()`.

### Reading measurements

Bus voltage is provided via `readBusVoltageMv(uint16_t& mv)`.  
Shunt voltage is provided via `readShuntVoltageUv(int32_t& uv)`.  
Current is provided via `readCurrentMa(double& ma)` (or `readCurrentRaw(int16_t&)`).  
Power is provided via `readPowerMw(double& mw)` (or `readPowerRaw(int16_t&)`).

```cpp
uint16_t bus_mv = 0;
int32_t shunt_uv = 0;
double current_ma = 0;
double power_mw = 0;

sensor.readBusVoltageMv(bus_mv);      // mV
sensor.readShuntVoltageUv(shunt_uv);  // µV
sensor.readCurrentMa(current_ma);     // mA (requires calibration)
sensor.readPowerMw(power_mw);         // mW (requires calibration)
```

### Reset and address management

You can reset the sensor with `reset()` (optional delay enabled by default).  
You can change and query the active address at runtime via `setAddress()` / `getAddress()`.

```cpp
sensor.reset();  // resets config to default state
sensor.setAddress(ina219::Address::A0_VS_A1_GND);
```

## Provider abstraction

### What is `ina219::Provider`?

`ina219::Provider` is an abstract interface that supplies:

- I2C write/read primitives (`i2cWrite`, `i2cRead`) with an optional `nostop` parameter for repeated-start sequences.
- A millisecond delay function (`delayMs`).
- Optional printf-style logging entry points (`logDebug`, `logInfo`, `logWarning`, `logError`) receiving a `va_list` (default no-op in the base class).

### Minimal custom Provider example

Implement only what you need for your platform: I2C + delay are mandatory.

```cpp
#include <ina219/ina219.hpp>
#include <cstdarg>
#include <cstdio>

class MyProvider : public ina219::Provider {
public:
    bool i2cWrite(uint8_t addr,
                  const uint8_t* data,
                  std::size_t len,
                  bool nostop = false) override
    {
        // Implement using your SDK/HAL (addr is 7-bit).
        return true;
    }

    bool i2cRead(uint8_t addr,
                 uint8_t* data,
                 std::size_t len,
                 bool nostop = false) override
    {
        // Implement using your SDK/HAL.
        return true;
    }

    void delayMs(uint32_t ms) override
    {
        // Implement using your RTOS/HAL timing.
    }

    // Optional: override logging
    void logDebug(const char* fmt, std::va_list args) override
    {
        std::vprintf(fmt, args);
        std::printf("\n");
    }
};
```

## Building and running examples

### Using the build script

Use the repo build helper script from the repository root:

```bash
./scripts/build.sh
```

> By default, it builds a static library on the host platform with Debug configuration.

Build for a specific example using the `--example` flag, e.g., for the Pico example:

```bash
./scripts/build.sh --example pico
```

> You can use the `--clean` flag to remove the build directory before building.

### Manual CMake (fallback)

You can also build examples directly with CMake, as shown in the previous README.  
For example, to build the Pico example:

```bash
cmake -S ./examples/pico -B build
cmake --build build --target ina219_pico
```

Refer to each `examples/<platform>/README.md` for flashing instructions.

## Integrating the driver in your project

### Option 1: CMake FetchContent (recommended)

```cmake
include(FetchContent)

FetchContent_Declare(
    ina219
    GIT_REPOSITORY https://github.com/tblanpied/ina219-cpp-driver.git
    GIT_TAG main
)
FetchContent_MakeAvailable(ina219)

target_link_libraries(your_app PRIVATE ina219::ina219)
```

> You can specify a log level for the driver with `set(INA219_LOG_LEVEL <log_level>)` (0=none, 1=error, 2=warning, 3=info, 4=debug) before `FetchContent_MakeAvailable()`. The default log level is 0 (no logs).

### Option 2: add_subdirectory (submodule or vendored)

```bash
git submodule add https://github.com/tblanpied/ina219-cpp-driver.git external/ina219
```

```cmake
add_subdirectory(external/ina219)
target_link_libraries(your_app PRIVATE ina219::ina219)
```

## Typical usage scenarios (wiring + API)

### Scenario A: Single sensor, default address

Wiring: leave A0=GND and A1=GND to use address 0x40.  
API: use the default constructor address and configure + calibrate.

```cpp
ina219::Ina219 sensor{provider};          // defaults to Address::A0_GND_A1_GND
sensor.setShuntResistor(0.1);             // board shunt (Ω)
sensor.configure().operatingMode(ina219::Mode::ShuntBusVoltageContinuous);
```

### Scenario B: Multiple INA219 devices on one I2C bus

Wiring: set A0/A1 to different combinations (GND / VS+ / SDA / SCL) to select unique addresses via `ina219::Address`.  
API: construct each instance with its address (or call `setAddress()` before use).

```cpp
ina219::Ina219 s1{provider, ina219::Address::A0_GND_A1_GND};
ina219::Ina219 s2{provider, ina219::Address::A0_VS_A1_GND};
```

### Scenario C: Measure only bus voltage (no current)

Wiring: connect INA219 bus sense pins appropriately for your measurement point, and connect SDA/SCL.  
API: configure bus ADC/mode as needed, then call `readBusVoltageMv()`.

```cpp
sensor.configure()
    .busRange(ina219::BusRange::V32)
    .operatingMode(ina219::Mode::BusVoltageContinuous);

uint16_t mv = 0;
sensor.readBusVoltageMv(mv);
```

### Scenario D: Current + power monitoring (typical)

Wiring: place a known shunt resistor in series with the load so INA219 can measure the shunt voltage drop.  
API: calibrate first (shunt resistor + LSB/max current), then read `readCurrentMa()` and `readPowerMw()`.

```cpp
sensor.setCalibrationMaxCurrent(0.1, 3.2);  // rShunt=0.1Ω, Imax=3.2A

double ma = 0;
double mw = 0;
sensor.readCurrentMa(ma);
sensor.readPowerMw(mw);
```

### Scenario E: Low power / sleep

Wiring: unchanged.  
API: set `operatingMode(Mode::PowerDown)` when you want to stop conversions.

```cpp
sensor.configure().operatingMode(ina219::Mode::PowerDown);
```

## License

MIT License (see `LICENSE` file).
