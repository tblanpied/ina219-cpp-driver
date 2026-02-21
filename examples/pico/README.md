# Raspberry Pi Pico example (INA219)

This example shows how to use the INA219 driver on a Raspberry Pi Pico (RP2040) using the Pico SDK I2C API, with a custom `ina219::Provider` implementation and periodic measurements printed over USB serial. 

## What this example demonstrates

- How to implement a minimal Pico-specific `ina219::Provider` (`Ina219PicoProvider`) using `i2c_write_blocking()`, `i2c_read_blocking()`, and `sleep_ms()`. 
- How to reset, configure, calibrate, and continuously read bus voltage, shunt voltage, current, and power. 
- How to use the provider logging hooks (`logDebug`, `logInfo`, `logWarning`, `logError`) to print formatted logs. 

## Required hardware

- Raspberry Pi Pico (or Pico W) running the Pico SDK. 
- One INA219 breakout/module (default I2C address 0x40 if A0=A1=GND). 
- Jumper wires and a USB cable for serial output + power. 
- A load and supply for “real” current measurement (optional but recommended). 

## Required software and toolchain

- Raspberry Pi Pico SDK environment (CMake-based Pico toolchain). 
- A serial terminal (e.g., `minicom`, `screen`, PuTTY) to view USB CDC output from `stdio_init_all()`. 

## Dependencies to install

- Pico SDK and its standard build prerequisites for your OS (CMake, a C/C++ toolchain, etc.). 
- No additional libraries are required beyond what the Pico SDK already provides for I2C and stdio in this example. 

## Pinout configuration

This example uses **I2C0** at **400 kHz**, with: 

- SDA: GPIO4
- SCL: GPIO5

The code enables internal pull-ups on GPIO4/GPIO5 via `gpio_pull_up()`, but in many real setups you should also have external I2C pull-ups on the bus (often already present on INA219 breakout boards). 

## How to flash

A helper script is provided at: `./examples/pico/scripts/flash.sh`.

> Should be ran from the project root (not from `examples/pico`), and assumes you have already built the example with `./scripts/build --example pico`.

```bash
./examples/pico/scripts/flash.sh
```

## What the example is doing internally

1. Initializes USB stdio with `stdio_init_all()`, then sets up I2C0 at 400 kHz and configures GPIO4/GPIO5 for I2C with pull-ups. 
2. Creates `Ina219PicoProvider`, which adapts Pico SDK I2C + delay + logging to the driver’s `ina219::Provider` interface. 
3. Creates `ina219::Ina219 sensor(provider)`, resets the sensor, applies a configuration (BusRange V16, Gain8, heavy averaging on both ADCs, continuous shunt+bus), then calibrates using `setShuntResistor(0.1)`. 
4. In an infinite loop, reads bus voltage, shunt voltage, current, and power, and prints the values every 500 ms using a small terminal “UI” that rewrites the same block using ANSI escape sequences. 

## Which parts of the API it exercises

From `ina219::Ina219`: 

- `reset()`
- `configure()` + `ConfigBuilder::busRange()`, `pgaGain()`, `busAdcMode()`, `shuntAdcMode()`, `operatingMode()` (fluent API)
- `setShuntResistor()`
- `readBusVoltageMv()`
- `readShuntVoltageUv()`
- `readCurrentMa()`
- `readPowerMw()`

From `ina219::Provider` (implemented by `Ina219PicoProvider`): 

- `i2cWrite()`, `i2cRead()`, `delayMs()`
- `logDebug()`, `logInfo()`, `logWarning()`, `logError()` (all routed to a shared `log()` function)
