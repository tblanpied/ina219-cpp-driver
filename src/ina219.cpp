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

namespace ina219 {

/* Helper Constants */

static constexpr uint8_t BYTE_SHIFT = 8;
static constexpr uint16_t BYTE_MASK = 0xFF;
static constexpr double TO_MILLI = 1000.0;

/* Configuration Builder */

Ina219::ConfigBuilder::ConfigBuilder( Ina219& ref, bool delay ) : _ref( ref ), _delay( delay ) {
    // Read current config to preserve unchanged fields
    if ( _ref._readRegister( RegisterAddress::Config, _config ) ) {
        _ref._logDebug( "ConfigBuilder: Read current configuration: 0x%04X", _config );
    } else {
        _ref._logError( "ConfigBuilder: Failed to read current configuration, using defaults" );
        _config = DEFAULT_CONFIG;    // Use default config if read fails
    }
}

Ina219::ConfigBuilder& Ina219::ConfigBuilder::busRange( BusRange range ) noexcept {
    _config = CONFIG_BRNG.set( _config, static_cast<uint16_t>( range ) );
    return *this;
}

Ina219::ConfigBuilder& Ina219::ConfigBuilder::pgaGain( PgaGain gain ) noexcept {
    _config = CONFIG_PG.set( _config, static_cast<uint16_t>( gain ) );
    return *this;
}

Ina219::ConfigBuilder& Ina219::ConfigBuilder::busAdcMode( AdcMode mode ) noexcept {
    _config = CONFIG_BADC.set( _config, static_cast<uint16_t>( mode ) );
    return *this;
}

Ina219::ConfigBuilder& Ina219::ConfigBuilder::shuntAdcMode( AdcMode mode ) noexcept {
    _config = CONFIG_SADC.set( _config, static_cast<uint16_t>( mode ) );
    return *this;
}

Ina219::ConfigBuilder& Ina219::ConfigBuilder::operatingMode( Mode mode ) noexcept {
    _config = CONFIG_MODE.set( _config, static_cast<uint16_t>( mode ) );
    return *this;
}

Ina219::ConfigBuilder::~ConfigBuilder() {
    // Apply configuration to sensor.
    bool result = _ref._writeRegister( RegisterAddress::Config, _config );
    if ( !result ) {
        _ref._logError( "ConfigBuilder: Failed to apply configuration" );
    } else {
        if ( _delay ) {
            _ref._provider.delayMs( CONFIG_WAIT_MS );    // Wait for configuration to take effect
        }
        _ref._logInfo( "ConfigBuilder: Applied configuration: 0x%04X", _config );
        _ref._logInfo( "ConfigBuilder:   Bus Range: 0x%02X", CONFIG_BRNG.get( _config ) );
        _ref._logInfo( "ConfigBuilder:   PGA Gain: 0x%02X", CONFIG_PG.get( _config ) );
        _ref._logInfo( "ConfigBuilder:   Bus ADC Mode: 0x%02X", CONFIG_BADC.get( _config ) );
        _ref._logInfo( "ConfigBuilder:   Shunt ADC Mode: 0x%02X", CONFIG_SADC.get( _config ) );
        _ref._logInfo( "ConfigBuilder:   Operating Mode: 0x%02X", CONFIG_MODE.get( _config ) );
    }
}

/* Ina219 private methods */

bool Ina219::_writeRegister( RegisterAddress reg, uint16_t value ) {
    uint8_t data[3];
    data[0] = static_cast<uint8_t>( reg );
    data[1] = static_cast<uint8_t>( ( value >> BYTE_SHIFT ) & BYTE_MASK );    // High byte
    data[2] = static_cast<uint8_t>( value & BYTE_MASK );                      // Low byte
    bool result = _provider.i2cWrite(
        static_cast<uint8_t>( _addr ), static_cast<const uint8_t*>( data ), sizeof( data ), false );
    if ( result ) {
        _logDebug( "writeRegister: reg=0x%02X, value=0x%04X", reg, value );
    } else {
        _logError( "writeRegister: Failed to write register 0x%02X with value 0x%04X", reg, value );
    }
    return result;
}

bool Ina219::_readRegister( RegisterAddress reg, uint16_t& value ) {
    uint8_t data[2];
    auto regAddr = static_cast<uint8_t>( reg );
    if ( !_provider.i2cWrite( static_cast<uint8_t>( _addr ), &regAddr, 1, true ) ) {
        _logError( "readRegister: Failed to write register address 0x%02X", reg );
        return false;    // Write reg address failed
    }
    if ( !_provider.i2cRead( static_cast<uint8_t>( _addr ),
                             static_cast<uint8_t*>( data ),
                             sizeof( data ),
                             false ) ) {
        _logError( "readRegister: Failed to read register 0x%02X", reg );
        return false;    // Read data failed
    }
    value = ( data[0] << BYTE_SHIFT ) | data[1];
    _logDebug( "readRegister: reg=0x%02X, value=0x%04X", reg, value );
    return true;
}

void Ina219::_logDebug( const char* fmt, ... ) noexcept {
#if INA219_LOG_LEVEL >= 4
    std::va_list args;
    va_start( args, fmt );
    _provider.logDebug( fmt, args );
    va_end( args );
#else
    (void)fmt;
#endif
}

void Ina219::_logInfo( const char* fmt, ... ) noexcept {
#if INA219_LOG_LEVEL >= 3
    std::va_list args;
    va_start( args, fmt );
    _provider.logInfo( fmt, args );
    va_end( args );
#else
    (void)fmt;
#endif
}

void Ina219::_logWarning( const char* fmt, ... ) noexcept {
#if INA219_LOG_LEVEL >= 2
    std::va_list args;
    va_start( args, fmt );
    _provider.logWarning( fmt, args );
    va_end( args );
#else
    (void)fmt;
#endif
}

void Ina219::_logError( const char* fmt, ... ) noexcept {
#if INA219_LOG_LEVEL >= 1
    std::va_list args;
    va_start( args, fmt );
    _provider.logError( fmt, args );
    va_end( args );
#else
    (void)fmt;
#endif
}

/* Ina219 public methods */

Ina219::Ina219( Provider& provider, Address address ) : _provider( provider ), _addr( address ) {
}

Ina219::ConfigBuilder Ina219::configure( bool delay ) noexcept {
    return ConfigBuilder( *this, delay );
}

bool Ina219::reset( bool delay ) noexcept {
    bool result = _writeRegister( RegisterAddress::Config, CONFIG_RST.set( 0, 1 ) );
    if ( result ) {
        _logDebug( "reset: Reset command sent successfully" );
    } else {
        _logError( "reset: Failed to send sensor reset command" );
    }
    if ( delay && result ) {
        _logDebug( "reset: Waiting for %d ms", RESET_WAIT_MS );
        _provider.delayMs( RESET_WAIT_MS );
    }
    return result;
}

bool Ina219::readBusVoltageMv( uint16_t& mv ) noexcept {
    uint16_t rawValue{};
    if ( !_readRegister( RegisterAddress::BusVoltage, rawValue ) ) {
        _logError( "readBusVoltageMv: Failed to read bus voltage" );
        return false;    // Read failed, return false as error indicator
    }
    mv = BUS_VOLTAGE_BD.get( rawValue ) * BUS_VOLTAGE_LSB_MV;    // Convert to mV (LSB = 4mV)
    _logDebug( "readBusVoltageMv: voltage = %d mV, raw = 0x%04X", mv, rawValue );
    return true;
}

bool Ina219::setShuntResistor( double rShunt, bool delay ) noexcept {
    if ( rShunt <= 0 ) {
        _logError( "setShuntResistor: Invalid shunt resistor value: %f", rShunt );
        return false;
    }

    // Auto‑LSB: max resolution for this R_SHUNT
    double maxCurrentSafe
        = MAX_V_SHUNT / rShunt;    // I = V/R, max current that won't exceed shunt voltage limit
    double currentLsb = maxCurrentSafe / MAX_CURRENT_RAW_VALUE;

    _logInfo(
        "setShuntResistor: Calculated current LSB: %f A/bit for R_SHUNT: %f", currentLsb, rShunt );

    return setCalibration( rShunt, currentLsb, delay );
}

bool Ina219::setCalibration( double rShunt, double currentLsb, bool delay ) noexcept {
    if ( rShunt <= 0 || currentLsb <= 0 ) {
        _logError( "setCalibration: Invalid parameters. R_SHUNT: %f, Current LSB: %f",
                   rShunt,
                   currentLsb );
        return false;
    }

    auto cal = static_cast<uint16_t>( CAL_CONSTANT / ( rShunt * currentLsb ) );

    _logDebug( "setCalibration: Calculated CAL value: %u for R_SHUNT: %f, Current LSB: %f",
               cal,
               rShunt,
               currentLsb );

    _rShunt = rShunt;
    _currentLsb = currentLsb;

    bool result = _writeRegister( RegisterAddress::Calibration, cal );
    if ( delay && result ) {
        _logDebug( "setCalibration: Waiting for %d ms for calibration to take effect",
                   CALIBRATION_WAIT_MS );
        _provider.delayMs( CALIBRATION_WAIT_MS );
    }
    return result;
}

bool Ina219::setCalibrationMaxCurrent(
    double rShunt, double maxExpectedCurrent, bool delay ) noexcept {
    if ( rShunt <= 0 || maxExpectedCurrent <= 0 ) {
        _logError(
            "setCalibrationMaxCurrent: Invalid parameters. R_SHUNT: %f, Max Expected Current: %f",
            rShunt,
            maxExpectedCurrent );
        return false;
    }

    // Compute safe LSB for this max current
    double currentLsb = maxExpectedCurrent / MAX_CURRENT_RAW_VALUE;

    _logInfo(
        "setCalibrationMaxCurrent: Calculated current LSB: %f A/bit for R_SHUNT: %f, Max " "Ex" "pe" "ct" "e" "d " "Current" ": %f",
        currentLsb,
        rShunt,
        maxExpectedCurrent );

    return setCalibration( rShunt, currentLsb, delay );
}

double Ina219::getShuntResistor() const noexcept {
    return _rShunt;
}

double Ina219::getCurrentLsb() const noexcept {
    return _currentLsb;
}

bool Ina219::readShuntVoltageUv( int32_t& uv ) noexcept {
    uint16_t rawValue{};
    if ( !_readRegister( RegisterAddress::ShuntVoltage, rawValue ) ) {
        _logError( "readShuntVoltageUv: Failed to read shunt voltage" );
        return false;
    }

    // Two's complement signed value (sign already extended in the 16-bit word)
    const auto rawSigned = static_cast<int16_t>( rawValue );
    uv = static_cast<int32_t>( rawSigned ) * SHUNT_VOLTAGE_LSB_UV;    // Convert to µV (LSB = 10µV)

    _logDebug( "readShuntVoltageUv: shunt voltage = %d µV, raw = 0x%04X", uv, rawValue );

    return true;
}

bool Ina219::readCurrentRaw( int16_t& raw ) noexcept {
    uint16_t rawValue{};
    if ( !_readRegister( RegisterAddress::Current, rawValue ) ) {
        _logError( "readCurrentRaw: Failed to read raw current" );
        return false;    // Read failed, return false as error indicator
    }

    raw = static_cast<int16_t>( rawValue );    // Current register is signed 16-bit

    _logDebug( "readCurrentRaw: raw = 0x%04X", rawValue );

    return true;
}

bool Ina219::readCurrentMa( double& ma ) noexcept {
    int16_t rawCurrent{};
    if ( !readCurrentRaw( rawCurrent ) ) {
        _logError( "readCurrentMa: Failed to read current" );
        return false;
    }

    ma = rawCurrent * _currentLsb * TO_MILLI;    // Convert to mA

    _logDebug( "readCurrentMa: current = %f mA, raw = 0x%04X", ma, rawCurrent );

    return true;
}

bool Ina219::readPowerRaw( int16_t& raw ) noexcept {
    uint16_t rawValue{};
    if ( !_readRegister( RegisterAddress::Power, rawValue ) ) {
        _logError( "readPowerRaw: Failed to read raw power" );
        return false;    // Read failed, return false as error indicator
    }

    raw = static_cast<int16_t>( rawValue );    // Power register is signed 16-bit

    _logDebug( "readPowerRaw: raw = 0x%04X", rawValue );

    return true;
}

bool Ina219::readPowerMw( double& mw ) noexcept {
    int16_t rawPower{};
    if ( !readPowerRaw( rawPower ) ) {
        _logError( "readPowerMw: Failed to read power in milliwatts" );
        return false;
    }

    mw = rawPower * _currentLsb * POWER_LSB_MULTIPLIER * TO_MILLI;

    _logDebug( "readPowerMw: power = %f mW, raw = 0x%04X", mw, rawPower );

    return true;
}

void Ina219::setAddress( Address address ) noexcept {
    _addr = address;
}

Address Ina219::getAddress() const noexcept {
    return _addr;
}

}    // namespace ina219
