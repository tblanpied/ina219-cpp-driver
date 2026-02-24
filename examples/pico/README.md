# Raspberry Pi Pico example (INA219)

This example shows how to use the **INA219 C++ driver** on a Raspberry Pi Pico (RP2040) using the Pico SDK I2C API, with a Pico-specific platform wrapper and periodic measurements printed over USB serial.

## What this example demonstrates

- Implementing a Pico platform/provider using `i2c_write_blocking()`, `i2c_read_blocking()`, and `sleep_ms()`.
- Resetting, configuring, calibrating, and reading bus voltage, shunt voltage, current, and power in a loop.
- Using the driver’s logging hooks (debug/info/warning/error) to print formatted logs over stdio.

## Required hardware

- Raspberry Pi Pico (or Pico W) running the Pico SDK.
- INA219 breakout/module (default I2C address `0x40` when A0=A1=GND).
- Jumper wires and a USB cable for power + serial output.
- A load and supply for real current measurement (recommended).

## Required software and toolchain

- Raspberry Pi Pico SDK + its standard build prerequisites (CMake, ARM GCC toolchain, etc.).
- A serial terminal (e.g. `minicom`, `screen`, PuTTY) to view USB CDC output.

## Pinout configuration

This example uses **I2C0** at **400 kHz**:

- SDA: GPIO4
- SCL: GPIO5

The code enables the Pico’s internal pull-ups on GPIO4/GPIO5 via `gpio_pull_up()`, but you typically also want external I2C pull-ups on the bus (many INA219 breakouts already include them).

## Build

From the repo root:

```bash
cmake -S examples/pico -B build
cmake --build build
```

## Flash

The build generates a `flash` target, you can run:

```bash
cmake --build build --target flash
```

This relies on **picotool** being installed and discoverable in `PATH`.

You can always drag-and-drop the generated `.uf2` file onto the Pico’s mass-storage device (BOOTSEL).

## What the example does (step-by-step)

1. Initializes USB stdio with `stdio_init_all()`.
2. Initializes I2C0 at 400 kHz with `i2c_init()`, configures GPIO4/GPIO5 for I2C, and enables pull-ups.
3. Creates `PicoPlatform`, a small adapter that implements the driver’s expected platform API using Pico SDK calls (I2C + delay + logging).
4. Constructs the sensor driver, resets the INA219, applies a configuration (BusRange V16, Gain8, heavy averaging on both ADCs, continuous shunt+bus), then calibrates using `setShuntResistor(0.1)`.
5. In an infinite loop, reads bus voltage, shunt voltage, current, and power every 500 ms and prints them using a small “terminal UI” (ANSI escape sequences) that overwrites the previous block.

## Driver API exercised

- `reset()`
- `configure()` and builder methods: `busRange()`, `pgaGain()`, `busAdcMode()`, `shuntAdcMode()`, `operatingMode()`
- `setShuntResistor()`
- `readBusVoltageMv()`
- `readShuntVoltageUv()`
- `readCurrentMa()`
- `readPowerMw()`
