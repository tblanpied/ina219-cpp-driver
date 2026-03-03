# INA219 ESP-IDF Example (ESP32‑S3)

This example shows how to use the **ina219-cpp-driver** on an ESP32‑S3 using ESP‑IDF and the I2C master driver API (`driver/i2c_master.h`).

It demonstrates:

- Creating a small ESP‑IDF “platform” wrapper (`Esp32Platform`) that satisfies the driver’s compile-time Platform API.
- Using the ESP‑IDF I2C bus/device model (allocate bus once, attach a device handle, perform transactions).
- Reading bus voltage, shunt voltage, current, and power continuously and printing a live-updating terminal “dashboard”.

## Hardware required

- ESP32‑S3 development board (this example is configured for ESP32‑S3, and `sdkconfig.defaults` sets `CONFIG_IDF_TARGET="esp32s3"`).
- INA219 breakout/module connected over I2C.

## Wiring / pinout

This example uses:

- SDA: **GPIO4**
- SCL: **GPIO5**
- I2C frequency: **400 kHz**

Connect:

- ESP32‑S3 3V3 → INA219 VCC
- ESP32‑S3 GND → INA219 GND
- ESP32‑S3 GPIO4 (SDA) → INA219 SDA
- ESP32‑S3 GPIO5 (SCL) → INA219 SCL

## Software required (Ubuntu)

### 1) Install ESP‑IDF prerequisites

On Ubuntu/Debian, Espressif lists these packages as prerequisites for building ESP‑IDF projects:

```bash
sudo apt-get update
sudo apt-get install -y \
  git wget flex bison gperf \
  python3 python3-pip python3-venv \
  cmake ninja-build ccache \
  libffi-dev libssl-dev \
  dfu-util libusb-1.0-0
```

### 2) Install ESP‑IDF (example for v5.5.3)

Espressif’s “Get ESP‑IDF” + “install tools” flow is: clone ESP‑IDF, run `install.sh` for the target(s), then source `export.sh` to set the environment.

Example:

```bash
mkdir -p ~/esp
cd ~/esp
git clone -b v5.5.3 --recursive https://github.com/espressif/esp-idf.git

cd ~/esp/esp-idf
./install.sh esp32s3
. ./export.sh
```

## Build / flash / monitor

`idf.py` is the ESP‑IDF front-end for configuring, building, flashing and monitoring.
It supports:

- `-C <dir>` to select the project directory
- `-B <dir>` to select the build output directory instead of the default `build/` subdir

### Build

From the repository root, run:

```bash
idf.py -B build/ -C examples/esp_idf/ build
```

### Flash

```bash
idf.py -B build/ -C examples/esp_idf/ flash
```

You can specify the serial port with `-p`.

### Monitor

```bash
idf.py -B build/ -C examples/esp_idf/ monitor
```

## Files in this example

- `CMakeLists.txt`: ESP‑IDF project entry.
- `main/CMakeLists.txt`: registers the `main` component, depends on `esp_driver_i2c`, and uses `FetchContent` to build the INA219 driver from the repo root.
- `main/main.cpp`: implements `Esp32Platform` using ESP‑IDF I2C master driver and continuously prints INA219 readings.
- `sdkconfig.defaults`: minimal config (target + flash size).
