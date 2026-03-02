# INA219 Arduino (ATmega328P) example

This example shows how to use the **ina219-cpp-driver** on an ATmega328P-based Arduino (Nano/Uno) via I2C, and how to build/flash it using **CMake + Arduino CLI** (no Arduino IDE required).

It demonstrates:

- Abstracting the platform layer (I2C read/write, delay, logging) so the same INA219 driver can run on different MCUs.
- Reading bus voltage, shunt voltage, current, and power in a loop and printing results over UART/USB serial.

> Note: This example targets C++17 (not C++20) because full C++20 support is not reliably available with the default Arduino AVR toolchain. It also adds avr-libstdcpp (modm-io/avr-libstdcpp), a partial libstdc++ port for AVR, to provide missing standard C++ wrapper headers / std:: definitions (e.g., headers like <cstdint> map types into std::).

## Hardware required

- Arduino Nano/Uno (ATmega328P).
- INA219 current/power monitor breakout/module.
- Jumper wires.

### Wiring / pinout

I2C wiring (Uno/Nano):

- INA219 **VCC** → Arduino **5V** (or 3.3V if your breakout supports it)
- INA219 **GND** → Arduino **GND**
- INA219 **SDA** → Arduino **A4** (SDA)
- INA219 **SCL** → Arduino **A5** (SCL)

Power measurement wiring (high-side current sense):

- INA219 **VIN+** goes to the supply side of the load.
- INA219 **VIN-** goes to the load side.

> Tip: Always share a common ground between Arduino and the measured system.

## Software required

- CMake (>= 3.20)
- A build tool (Ninja or Make)
- `arduino-cli`
- Arduino AVR core installed via `arduino-cli`

### Install Arduino CLI + AVR core

1. Install Arduino CLI (see official installation methods).
2. Install the AVR core:

```bash
arduino-cli core update-index
arduino-cli core install arduino:avr
```

3. Verify the board shows up:

```bash
arduino-cli board list
```

This prints the connected board/port (e.g. `/dev/ttyACM0`).

## Example layout

Example layout (typical):

```
examples/arduino/
  CMakeLists.txt
  src/
    src.ino
```

`src.ino` uses:

- `Wire` for I2C (SDA=A4, SCL=A5 on Uno/Nano).
- `Serial` for logging/printing.

## Build and flash (CMake + Arduino CLI)

### Configure

From the repo root:

```bash
cmake -S examples/arduino -B build -DFQBN=arduino:avr:nano -DPORT=/dev/ttyACM0
```

Notes:

- For Uno use `-DFQBN=arduino:avr:uno`.
- Change `/dev/ttyACM0` with the result of the command `arduino-cli board list`.
- For some Nano clones you may need the “Old Bootloader” option (`cpu=atmega328old`). Arduino CLI exposes this as a board option for `arduino:avr:nano` (see CMakeLists.txt).

### Build

From the repo root:

```bash
cmake --build build
```

### Upload

From the repo root:

```bash
cmake --build build --target upload
```

The upload uses `arduino-cli upload` with `--fqbn` and `-p/--port`.

If you have a Nano clone that needs it, the upload target uses:

- `--board-options cpu=atmega328old`

## Serial monitor

After flashing, open a serial monitor at **115200** baud (the value used in `Serial.begin(...)`).

Example with Arduino CLI monitor (if you use it) or any terminal tool:

```bash
minicom -D /dev/ttyACM0 -b 115200
```

You should see periodic readings (bus voltage, shunt voltage, current, power).
