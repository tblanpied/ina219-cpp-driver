/**
 * Copyright (c) 2026 - present Timothée Blanpied
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file ina219.cpp
 * @brief Source file for the INA219 sensor C++ driver.
 * @version 1.0.0
 * @author Timothée Blanpied
 * @date 2026-02-21
 */

#include "ina219/ina219.hpp"

using namespace ina219;

Ina219::ConfigBuilder::ConfigBuilder(Ina219 &ref, bool delay) : config_(), ref_(ref), delay_(delay)
{
    // Read current config to preserve unchanged fields
    uint16_t current_config;
    if (ref_.readRegister(RegisterAddress::Config, current_config))
    {
        config_.raw = current_config;
        ref_.logDebug("ConfigBuilder: Read current configuration: 0x%04X", current_config);
    }
    else
    {
        ref_.logError("ConfigBuilder: Failed to read current configuration, using defaults");
        config_.raw = DEFAULT_CONFIG; // Use default config if read fails
    }
}

Ina219::ConfigBuilder &Ina219::ConfigBuilder::busRange(BusRange range) noexcept
{
    config_.bits.brng = static_cast<uint16_t>(range) & ((0x1u << ConfigRegister::BRNG_BITS) - 1);
    return *this;
}

Ina219::ConfigBuilder &Ina219::ConfigBuilder::pgaGain(PgaGain gain) noexcept
{
    config_.bits.pg = static_cast<uint16_t>(gain) & ((0x1u << ConfigRegister::PG_BITS) - 1);
    return *this;
}

Ina219::ConfigBuilder &Ina219::ConfigBuilder::busAdcMode(AdcMode mode) noexcept
{
    config_.bits.badc = static_cast<uint16_t>(mode) & ((0x1u << ConfigRegister::BADC_BITS) - 1);
    return *this;
}

Ina219::ConfigBuilder &Ina219::ConfigBuilder::shuntAdcMode(AdcMode mode) noexcept
{
    config_.bits.sadc = static_cast<uint16_t>(mode) & ((0x1u << ConfigRegister::SADC_BITS) - 1);
    return *this;
}

Ina219::ConfigBuilder &Ina219::ConfigBuilder::operatingMode(Mode mode) noexcept
{
    config_.bits.mode = static_cast<uint16_t>(mode) & ((0x1u << ConfigRegister::MODE_BITS) - 1);
    return *this;
}

Ina219::ConfigBuilder::~ConfigBuilder()
{
    // Apply configuration to sensor.
    bool result = ref_.writeRegister(RegisterAddress::Config, config_.raw);
    if (!result)
    {
        ref_.logError("ConfigBuilder: Failed to apply configuration");
    }
    else
    {
        if (delay_)
        {
            ref_.provider_.delayMs(CONFIG_WAIT_MS); // Wait for configuration to take effect
        }
        ref_.logInfo("ConfigBuilder: Applied configuration: ");
        ref_.logInfo("ConfigBuilder:   Bus Range: 0x%02X", config_.bits.brng);
        ref_.logInfo("ConfigBuilder:   PGA Gain: 0x%02X", config_.bits.pg);
        ref_.logInfo("ConfigBuilder:   Bus ADC Mode: 0x%02X", config_.bits.badc);
        ref_.logInfo("ConfigBuilder:   Shunt ADC Mode: 0x%02X", config_.bits.sadc);
        ref_.logInfo("ConfigBuilder:   Operating Mode: 0x%02X", config_.bits.mode);
    }
}

bool Ina219::writeRegister(RegisterAddress reg, uint16_t value)
{
    uint8_t data[3];
    data[0] = static_cast<uint8_t>(reg);
    data[1] = static_cast<uint8_t>((value >> 8) & 0xFF); // High byte
    data[2] = static_cast<uint8_t>(value & 0xFF);        // Low byte
    bool result = provider_.i2cWrite(static_cast<uint8_t>(addr_), data, sizeof(data));
    if (result)
    {
        logDebug("writeRegister: reg=0x%02X, value=0x%04X", reg, value);
    }
    else
    {
        logError("writeRegister: Failed to write register 0x%02X with value 0x%04X", reg, value);
    }
    return result;
}

bool Ina219::readRegister(RegisterAddress reg, uint16_t &value)
{
    uint8_t data[2];
    if (!provider_.i2cWrite(static_cast<uint8_t>(addr_), reinterpret_cast<const uint8_t *>(&reg), 1, false))
    {
        logError("readRegister: Failed to write register address 0x%02X", reg);
        return false; // Write reg address failed
    }
    if (!provider_.i2cRead(static_cast<uint8_t>(addr_), data, sizeof(data)))
    {
        logError("readRegister: Failed to read register 0x%02X", reg);
        return false; // Read data failed
    }
    value = (data[0] << 8) | data[1];
    logDebug("readRegister: reg=0x%02X, value=0x%04X", reg, value);
    return true;
}

void Ina219::logDebug(const char *fmt, ...) noexcept
{
#if INA219_LOG_LEVEL >= 4
    std::va_list args;
    va_start(args, fmt);
    provider_.logDebug(fmt, args);
    va_end(args);
#else
    (void)fmt;
#endif
}

void Ina219::logInfo(const char *fmt, ...) noexcept
{
#if INA219_LOG_LEVEL >= 3
    std::va_list args;
    va_start(args, fmt);
    provider_.logInfo(fmt, args);
    va_end(args);
#else
    (void)fmt;
#endif
}

void Ina219::logWarning(const char *fmt, ...) noexcept
{
#if INA219_LOG_LEVEL >= 2
    std::va_list args;
    va_start(args, fmt);
    provider_.logWarning(fmt, args);
    va_end(args);
#else
    (void)fmt;
#endif
}

void Ina219::logError(const char *fmt, ...) noexcept
{
#if INA219_LOG_LEVEL >= 1
    std::va_list args;
    va_start(args, fmt);
    provider_.logError(fmt, args);
    va_end(args);
#else
    (void)fmt;
#endif
}

Ina219::Ina219(Provider &provider, Address address)
    : provider_(provider), addr_(address)
{
}

Ina219::ConfigBuilder Ina219::configure(bool delay) noexcept
{
    return ConfigBuilder(*this, delay);
}

bool Ina219::reset(bool delay) noexcept
{
    ConfigRegister config;
    config.bits.rst = 1; // Set RST bit
    bool result = writeRegister(RegisterAddress::Config, config.raw);
    if (result)
    {
        logDebug("reset: Reset command sent successfully");
    }
    else
    {
        logError("reset: Failed to send sensor reset command");
    }
    if (delay && result)
    {
        logDebug("reset: Waiting for %d ms", RESET_WAIT_MS);
        provider_.delayMs(RESET_WAIT_MS);
    }
    return result;
}

bool Ina219::readBusVoltageMv(uint16_t &mv) noexcept
{
    BusVoltageRegister reg;
    if (!readRegister(RegisterAddress::BusVoltage, reg.raw))
    {
        logError("readBusVoltageMv: Failed to read bus voltage");
        return false; // Read failed, return false as error indicator
    }
    mv = reg.bits.voltage * BUS_VOLTAGE_LSB_MV; // Convert to mV (LSB = 4mV)
    logDebug("readBusVoltageMv: voltage = %d mV, raw = 0x%04X", mv, reg.raw);
    return true;
}

bool Ina219::setShuntResistor(double rShunt, bool delay) noexcept
{
    if (rShunt <= 0)
    {
        logError("setShuntResistor: Invalid shunt resistor value: %f", rShunt);
        return false;
    }

    // Auto‑LSB: max resolution for this R_SHUNT
    double maxCurrentSafe = MAX_V_SHUNT / rShunt; // I = V/R, max current that won't exceed shunt voltage limit
    double currentLsb = maxCurrentSafe / MAX_CURRENT_RAW_VALUE;

    logInfo("setShuntResistor: Calculated current LSB: %f A/bit for R_SHUNT: %f", currentLsb, rShunt);

    return setCalibration(rShunt, currentLsb, delay);
}

bool Ina219::setCalibration(double rShunt, double currentLsb, bool delay) noexcept
{
    if (rShunt <= 0 || currentLsb <= 0)
    {
        logError("setCalibration: Invalid parameters. R_SHUNT: %f, Current LSB: %f", rShunt, currentLsb);
        return false;
    }

    uint16_t cal = static_cast<uint16_t>(CAL_CONSTANT / (rShunt * currentLsb));

    logDebug("setCalibration: Calculated CAL value: %u for R_SHUNT: %f, Current LSB: %f", cal, rShunt, currentLsb);

    rShunt_ = rShunt;
    currentLsb_ = currentLsb;

    bool result = writeRegister(RegisterAddress::Calibration, cal);
    if (delay && result)
    {
        logDebug("setCalibration: Waiting for %d ms for calibration to take effect", CALIBRATION_WAIT_MS);
        provider_.delayMs(CALIBRATION_WAIT_MS);
    }
    return result;
}

bool Ina219::setCalibrationMaxCurrent(double rShunt, double maxExpectedCurrent, bool delay) noexcept
{
    if (rShunt <= 0 || maxExpectedCurrent <= 0)
    {
        logError("setCalibrationMaxCurrent: Invalid parameters. R_SHUNT: %f, Max Expected Current: %f", rShunt, maxExpectedCurrent);
        return false;
    }

    // Compute safe LSB for this max current
    double currentLsb = maxExpectedCurrent / MAX_CURRENT_RAW_VALUE;

    logInfo("setCalibrationMaxCurrent: Calculated current LSB: %f A/bit for R_SHUNT: %f, Max Expected Current: %f", currentLsb, rShunt, maxExpectedCurrent);

    return setCalibration(rShunt, currentLsb, delay);
}

double Ina219::getShuntResistor() const noexcept { return rShunt_; }

double Ina219::getCurrentLsb() const noexcept { return currentLsb_; }

bool Ina219::readShuntVoltageUv(int32_t &uv) noexcept
{
    uint16_t rawValue;
    if (!readRegister(RegisterAddress::ShuntVoltage, rawValue))
    {
        logError("readShuntVoltageUv: Failed to read shunt voltage");
        return false;
    }

    // Two's complement signed value (sign already extended in the 16-bit word)
    const int16_t rawSigned = static_cast<int16_t>(rawValue);
    uv = static_cast<int32_t>(rawSigned) * SHUNT_VOLTAGE_LSB_UV; // Convert to µV (LSB = 10µV)

    logDebug("readShuntVoltageUv: shunt voltage = %d µV, raw = 0x%04X", uv, rawValue);

    return true;
}

bool Ina219::readCurrentRaw(int16_t &raw) noexcept
{
    uint16_t rawValue;
    if (!readRegister(RegisterAddress::Current, rawValue))
    {
        logError("readCurrentRaw: Failed to read raw current");
        return false; // Read failed, return false as error indicator
    }

    raw = static_cast<int16_t>(rawValue); // Current register is signed 16-bit

    logDebug("readCurrentRaw: raw = 0x%04X", rawValue);

    return true;
}

bool Ina219::readCurrentMa(double &ma) noexcept
{
    int16_t rawCurrent;
    if (!readCurrentRaw(rawCurrent))
    {
        logError("readCurrentMa: Failed to read current");
        return false;
    }

    ma = rawCurrent * currentLsb_ * 1000.0; // Convert to mA

    logDebug("readCurrentMa: current = %f mA, raw = 0x%04X", ma, rawCurrent);

    return true;
}

bool Ina219::readPowerRaw(int16_t &raw) noexcept
{
    uint16_t rawValue;
    if (!readRegister(RegisterAddress::Power, rawValue))
    {
        logError("readPowerRaw: Failed to read raw power");
        return false; // Read failed, return false as error indicator
    }

    raw = static_cast<int16_t>(rawValue); // Power register is signed 16-bit

    logDebug("readPowerRaw: raw = 0x%04X", rawValue);

    return true;
}

bool Ina219::readPowerMw(double &mw) noexcept
{
    int16_t rawPower;
    if (!readPowerRaw(rawPower))
    {
        logError("readPowerMw: Failed to read power in milliwatts");
        return false;
    }

    // Power LSB is 20 times current LSB according to datasheet
    mw = rawPower * currentLsb_ * 20.0 * 1000.0;

    logDebug("readPowerMw: power = %f mW, raw = 0x%04X", mw, rawPower);

    return true;
}

void Ina219::setAddress(Address address) noexcept
{
    addr_ = address;
}

Address Ina219::getAddress() const noexcept
{
    return addr_;
}
