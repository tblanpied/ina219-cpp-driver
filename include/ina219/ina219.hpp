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
 * @file ina219.hpp
 * @brief Header file containing the class template for the INA219 sensor C++ driver.
 * @version 1.0.0
 * @author Timothée Blanpied
 * @date 2026-03-02
 */

#pragma once
#include "details.hpp"

#include <cstdarg>
#include <cstdint>
#include <utility>

namespace ina219 {

/**
 * @brief INA219 I2C address selection based on A0/A1 pin states (7‑bit)
 *
 * The INA219 sensor allows you to set its I2C address using the A0 and A1 pins.
 * Each pin can be connected to GND, VS+, SDA, or SCL, resulting in 16 possible addresses.
 * This enum maps the combinations of A0 and A1 connections to their corresponding 7-bit I2C
 * addresses. The default address is 0x40 when both A0 and A1 are connected to GND. Refer to the
 * INA219 datasheet for more details on address selection.
 */
enum class Address : std::uint8_t {
    A0GndA1Gnd = 0x40,    ///< A1=GND, A0=GND     (default)
    A0VsA1Gnd = 0x41,     ///< A1=GND, A0=VS+
    A0SdaA1Gnd = 0x44,    ///< A1=GND, A0=SDA
    A0SclA1Gnd = 0x45,    ///< A1=GND, A0=SCL
    A0GndA1Vs = 0x42,     ///< A1=VS+, A0=GND
    A0VsA1Vs = 0x43,      ///< A1=VS+, A0=VS+
    A0SdaA1Vs = 0x46,     ///< A1=VS+, A0=SDA
    A0SclA1Vs = 0x47,     ///< A1=VS+, A0=SCL
    A0GndA1Sda = 0x48,    ///< A1=SDA, A0=GND
    A0VsA1Sda = 0x49,     ///< A1=SDA, A0=VS+
    A0SdaA1Sda = 0x4A,    ///< A1=SDA, A0=SDA
    A0SclA1Sda = 0x4B,    ///< A1=SDA, A0=SCL
    A0GndA1Scl = 0x4C,    ///< A1=SCL, A0=GND
    A0VsA1Scl = 0x4D,     ///< A1=SCL, A0=VS+
    A0SdaA1Scl = 0x4E,    ///< A1=SCL, A0=SDA
    A0SclA1Scl = 0x4F     ///< A1=SCL, A0=SCL
};

/**
 * @brief Bus voltage range selection for the INA219 sensor (0-16V or 0-32V).
 *
 * The default value is V32 (0-32V range).
 */
enum class BusRange : std::uint8_t {
    V16 = 0x0,    ///< 0-16V range
    V32 = 0x1     ///< 0-32V range (default)
};

/**
 * @brief Programmable Gain Amplifier (PGA) gain settings for the INA219 sensor,
 * which determine the shunt voltage measurement range.
 *
 * The default value is Gain8 (±320mV range).
 */
enum class PgaGain : std::uint8_t {
    Gain1 = 0x0,    ///< /1, ±40mV range
    Gain2 = 0x1,    ///< /2, ±80mV range
    Gain4 = 0x2,    ///< /4, ±160mV range
    Gain8 = 0x3,    ///< /8, ±320mV range (default)
};

/**
 * @brief ADC resolution and averaging settings for the INA219 sensor, which affect conversion time
 * and noise performance.
 *
 * Used for both bus voltage and shunt voltage measurements.
 * Higher resolution and more averaging result in longer conversion times but better accuracy.
 * The default value is Adc12bit1Sample (12-bit resolution, single sample, 532µs conversion time).
 */
enum class AdcMode : std::uint8_t {
    Adc9bit1Sample = 0x0,       ///< 9-bit resolution, 84µs conversion time
    Adc10bit1Sample = 0x1,      ///< 10-bit resolution, 148µs conversion time
    Adc11bit1Sample = 0x2,      ///< 11-bit resolution, 276µs conversion time
    Adc12bit1Sample = 0x3,      ///< 12-bit resolution, 532µs conversion time (default)
    Adc12bit2Samples = 0x9,     ///< 2 samples averaged, 1.06ms conversion time
    Adc12bit4Samples = 0xA,     ///< 4 samples averaged, 2.13ms conversion time
    Adc12bit8Samples = 0xB,     ///< 8 samples averaged, 4.26ms conversion time
    Adc12bit16Samples = 0xC,    ///< 16 samples averaged, 8.51ms conversion time
    Adc12bit32Samples = 0xD,    ///< 32 samples averaged, 17.02ms conversion time
    Adc12bit64Samples = 0xE,    ///< 64 samples averaged, 34.05ms conversion time
    Adc12bit128Samples = 0xF    ///< 128 samples averaged, 68.10ms conversion time
};

/**
 * @brief Operating modes for the INA219 sensor, which control when measurements are taken and how
 * the sensor behaves.
 *
 * The default mode is ShuntBusVoltageContinuous, which continuously measures both shunt and bus
 * voltage.
 */
enum class Mode : std::uint8_t {
    PowerDown = 0x0,                   ///< Power‑down
    ShuntVoltageTriggered = 0x1,       ///< Triggered measurement of shunt voltage
    BusVoltageTriggered = 0x2,         ///< Triggered measurement of bus voltage
    ShuntBusVoltageTriggered = 0x3,    ///< Triggered measurement of both shunt and bus voltage
    AdcOff = 0x4,                      ///< ADC off (no measurements)
    ShuntVoltageContinuous = 0x5,      ///< Continuous measurement of shunt voltage
    BusVoltageContinuous = 0x6,        ///< Continuous measurement of bus voltage
    ShuntBusVoltageContinuous = 0x7    ///< Continuous measurement of both shunt and bus voltage
                                       ///< (default)
};

/**
 * @brief C++ driver class for the INA219 current and power monitor sensor.
 *
 * This class provides methods to configure the sensor, read voltage measurements, and manage the
 * I2C communication.
 */
#if INA219_HAS_CONCEPTS
template<details::Platform P>
class Ina219 {
#else
template<typename P>
class Ina219 {
    static_assert(
        details::is_platform<P>::value, "P does not satisfy the INA219 Platform requirements" );
#endif

  public:
/**
 * @brief Construct a driver instance using a default-constructed platform and an optional I2C
 * address.
 *
 * This constructor is enabled only if the platform type `P` is default-constructible.
 * It is useful when your platform configuration is fixed at compile time or can be
 * obtained without parameters (e.g., hard-coded I2C peripheral/pins/baud inside `P`).
 *
 * @tparam Q Helper template parameter used for SFINAE enable/disable.
 * @param address The I2C address of the INA219 sensor (default is `Address::A0GndA1Gnd`,
 * which corresponds to 0x40).
 */
#if INA219_HAS_CONCEPTS
    explicit Ina219( Address address = Address::A0GndA1Gnd )
    requires std::is_default_constructible_v<P>
        : _addr( address ) {
    }
#else
    template<typename Q = P,
             typename std::enable_if_t<std::is_default_constructible<Q>::value, int> = 0>
    explicit Ina219( Address address = Address::A0GndA1Gnd ) : _addr( address ) {
    }
#endif

    /**
     * @brief Construct a driver instance by taking ownership of a pre-configured platform and with
     * an optional I2C address.
     *
     * The platform is passed by value and moved into the driver. This lets the caller
     * build a platform object with specific hardware settings (I2C instance, pins, baudrate,
     * logging sink, etc.) and transfer it into the driver.
     *
     * @param platform Platform object.
     * @param address  The I2C address of the INA219 sensor (default is `Address::A0GndA1Gnd`,
     * which corresponds to 0x40).
     */
    explicit Ina219( P platform, Address address = Address::A0GndA1Gnd ) :
        _platform( std::move( platform ) ), _addr( address ) {
    }

    /**
     * @brief Builder class for configuring the INA219 sensor using a fluent API,
     * allowing users to set various configuration options in a readable and convenient way.
     * The configuration is applied to the sensor when the builder goes out of scope (in the
     * destructor). This design ensures that the configuration is always applied.
     */
    class ConfigBuilder {
      public:
        // Delete copy and move constructors and assignment operators to prevent misuse of the
        // builder
        ConfigBuilder( const ConfigBuilder& ) = delete;
        ConfigBuilder& operator=( const ConfigBuilder& ) = delete;
        ConfigBuilder( ConfigBuilder&& ) = delete;
        ConfigBuilder& operator=( ConfigBuilder&& ) = delete;

        /**
         * @brief Set the bus voltage range for the INA219 sensor.
         *
         * Values:
         *
         *  - `BusRange::V16`: 0-16V range
         *
         *  - `BusRange::V32`: 0-32V range
         *
         * @param range The desired bus voltage range (enum `BusRange`).
         * @return Reference to the ConfigBuilder for method chaining.
         */
        ConfigBuilder& busRange( BusRange range ) noexcept {
            _config = details::ConfigReg::Brng::set( _config, static_cast<std::uint16_t>( range ) );
            return *this;
        }

        /**
         * @brief Set the Programmable Gain Amplifier (PGA) gain for the INA219 sensor,
         * which determines the shunt voltage measurement range.
         *
         * Values:
         *
         * - `PgaGain::Gain1`: /1, ±40mV range
         *
         * - `PgaGain::Gain2`: /2, ±80mV range
         *
         * - `PgaGain::Gain4`: /4, ±160mV range
         *
         * - `PgaGain::Gain8`: /8, ±320mV range (default)
         *
         * @param gain The desired PGA gain setting (enum `PgaGain`).
         * @return Reference to the ConfigBuilder for method chaining.
         */
        ConfigBuilder& pgaGain( PgaGain gain ) noexcept {
            _config = details::ConfigReg::Pg::set( _config, static_cast<std::uint16_t>( gain ) );
            return *this;
        }

        /**
         * @brief Set the ADC resolution and averaging mode for bus voltage measurements on the
         * INA219 sensor. Higher resolution and more averaging result in longer conversion times but
         * better accuracy.
         *
         * Values:
         *
         * - `AdcMode::Adc9bit1Sample`: 9-bit resolution, 84µs conversion time
         *
         * - `AdcMode::Adc10bit1Sample`: 10-bit resolution, 148µs conversion time
         *
         * - `AdcMode::Adc11bit1Sample`: 11-bit resolution, 276µs conversion time
         *
         * - `AdcMode::Adc12bit1Sample`: 12-bit resolution, 532µs conversion time
         *
         * - `AdcMode::Adc12bit2Samples`: 2 samples averaged, 1.06ms conversion time
         *
         * - `AdcMode::Adc12bit4Samples`: 4 samples averaged, 2.13ms conversion time
         *
         * - `AdcMode::Adc12bit8Samples`: 8 samples averaged, 4.26ms conversion time
         *
         * - `AdcMode::Adc12bit16Samples`: 16 samples averaged, 8.51ms conversion time
         *
         * - `AdcMode::Adc12bit32Samples`: 32 samples averaged, 17.02ms conversion time
         *
         * - `AdcMode::Adc12bit64Samples`: 64 samples averaged, 34.05ms conversion time
         *
         * - `AdcMode::Adc12bit128Samples`: 128 samples averaged, 68.10ms conversion time
         *
         * @param mode The desired ADC mode for bus voltage measurements (enum `AdcMode`).
         * @return Reference to the ConfigBuilder for method chaining.
         */
        ConfigBuilder& busAdcMode( AdcMode mode ) noexcept {
            _config = details::ConfigReg::Badc::set( _config, static_cast<std::uint16_t>( mode ) );
            return *this;
        }

        /**
         * @brief Set the ADC resolution and averaging mode for shunt voltage measurements on the
         * INA219 sensor. Higher resolution and more averaging result in longer conversion times but
         * better accuracy.
         *
         * Values:
         *
         * - `AdcMode::Adc9bit1Sample`: 9-bit resolution, 84µs conversion time
         *
         * - `AdcMode::Adc10bit1Sample`: 10-bit resolution, 148µs conversion time
         *
         * - `AdcMode::Adc11bit1Sample`: 11-bit resolution, 276µs conversion time
         *
         * - `AdcMode::Adc12bit1Sample`: 12-bit resolution, 532µs conversion time
         *
         * - `AdcMode::Adc12bit2Samples`: 2 samples averaged, 1.06ms conversion time
         *
         * - `AdcMode::Adc12bit4Samples`: 4 samples averaged, 2.13ms conversion time
         *
         * - `AdcMode::Adc12bit8Samples`: 8 samples averaged, 4.26ms conversion time
         *
         * - `AdcMode::Adc12bit16Samples`: 16 samples averaged, 8.51ms conversion time
         *
         * - `AdcMode::Adc12bit32Samples`: 32 samples averaged, 17.02ms conversion time
         *
         * - `AdcMode::Adc12bit64Samples`: 64 samples averaged, 34.05ms conversion time
         *
         * - `AdcMode::Adc12bit128Samples`: 128 samples averaged, 68.10ms conversion time
         *
         * @param mode The desired ADC mode for shunt voltage measurements (enum `AdcMode`).
         * @return Reference to the ConfigBuilder for method chaining.
         */
        ConfigBuilder& shuntAdcMode( AdcMode mode ) noexcept {
            _config = details::ConfigReg::Sadc::set( _config, static_cast<std::uint16_t>( mode ) );
            return *this;
        }

        /**
         * @brief Set the operating mode for the INA219 sensor, which controls when measurements
         * are taken and how the sensor behaves.
         *
         * Values:
         *
         * - `Mode::PowerDown`: Sensor is powered down.
         *
         * - `Mode::ShuntVoltageTriggered`: Shunt voltage measurement is triggered.
         *
         * - `Mode::BusVoltageTriggered`: Bus voltage measurement is triggered.
         *
         * - `Mode::ShuntAndBusVoltageTriggered`: Both shunt and bus voltage measurements are
         * triggered.
         *
         * - `Mode::AdcOff`: ADC is turned off (no measurements).
         *
         * - `Mode::ShuntVoltageContinuous`: Shunt voltage measurement is continuous.
         *
         * - `Mode::BusVoltageContinuous`: Bus voltage measurement is continuous.
         *
         * - `Mode::ShuntAndBusVoltageContinuous`: Both shunt and bus voltage measurements are
         * continuous.
         *
         * @param mode The desired operating mode (enum `Mode`).
         * @return Reference to the ConfigBuilder for method chaining.
         */
        ConfigBuilder& operatingMode( Mode mode ) noexcept {
            _config = details::ConfigReg::Mode::set( _config, static_cast<std::uint16_t>( mode ) );
            return *this;
        }

        /**
         * @brief Destructor that applies the configuration to the INA219 sensor when the builder
         * goes out of scope.
         */
        ~ConfigBuilder() {
            bool result{ false };
            // Apply configuration to sensor.
            result = _ref._writeRegister( details::RegisterAddress::Config, _config );
            if ( !result ) {
                _ref._logError( "ConfigBuilder: Failed to apply configuration" );
            } else {
                if ( _delay ) {
                    _ref._platform.delayMs(
                        details::kConfigWaitMs );    // Wait for configuration to take effect
                }
                _ref._logInfo( "ConfigBuilder: Applied configuration: 0x%04X", _config );
                _ref._logInfo( "ConfigBuilder:   Bus Range: 0x%02X",
                               details::ConfigReg::Brng::get( _config ) );
                _ref._logInfo(
                    "ConfigBuilder:   PGA Gain: 0x%02X", details::ConfigReg::Pg::get( _config ) );
                _ref._logInfo( "ConfigBuilder:   Bus ADC Mode: 0x%02X",
                               details::ConfigReg::Badc::get( _config ) );
                _ref._logInfo( "ConfigBuilder:   Shunt ADC Mode: 0x%02X",
                               details::ConfigReg::Sadc::get( _config ) );
                _ref._logInfo( "ConfigBuilder:   Operating Mode: 0x%02X",
                               details::ConfigReg::Mode::get( _config ) );
            }
        }

      private:
        friend class Ina219;

        /// Local copy of the configuration register being built
        std::uint16_t _config{};
        /// Reference to the parent Ina219 instance to apply the configuration
        Ina219& _ref;
        /// Whether to delay after applying configuration to allow it to take effect
        bool _delay{ true };

        /**
         * @brief Constructor that initializes the ConfigBuilder with a reference to the parent
         * Ina219 instance. Reads the current configuration from the sensor to preserve unchanged
         * fields, or uses defaults if the read fails.
         */
        explicit ConfigBuilder( Ina219& ref, bool delay = true ) : _ref( ref ), _delay( delay ) {
            // Read current config to preserve unchanged fields
            if ( _ref._readRegister( details::RegisterAddress::Config, _config ) ) {
                _ref._logDebug( "ConfigBuilder: Read current configuration: 0x%04X", _config );
            } else {
                _ref._logError(
                    "ConfigBuilder: Failed to read current configuration, using defaults" );
                _config = details::kDefaultConfig;    // Use default config if read fails
            }
        }
    };

    /**
     * @brief Start configuring the INA219 sensor using the ConfigBuilder, which provides a fluent
     * API for setting various configuration options.
     * The configuration will be applied to the sensor when the ConfigBuilder goes out of scope (in
     * its destructor).
     *
     * Example usage:
     *
     * @code{.cpp}
    sensor.configure()
        .busRange(ina219::BusRange::V32)
        .pgaGain(ina219::PgaGain::Gain8)
        .busAdcMode(ina219::AdcMode::Adc12bit128Samples)
        .shuntAdcMode(ina219::AdcMode::Adc12bit128Samples)
        .operatingMode(ina219::Mode::ShuntBusVoltageContinuous);
      @endcode

     * @param delay If true, the ConfigBuilder will introduce a delay (`10` milliseconds) after
     * applying the configuration to allow the sensor to stabilize. Default is true.
     * @return A ConfigBuilder instance for setting configuration options in a fluent manner.
     */
    ConfigBuilder configure( bool delay = true ) noexcept {
        return ConfigBuilder( *this, delay );
    }

    /**
     * @brief Reset the INA219 sensor to its default state by setting the RST bit in the
     * configuration register.
     *
     * @param delay If true, the method will introduce a delay (`10` milliseconds) after sending the
     * reset command.
     * @return true if the reset command was successfully sent to the sensor; false otherwise.
     */
    bool reset( bool delay = true ) noexcept {
        bool result{};
        result = _writeRegister(
            details::RegisterAddress::Config, details::ConfigReg::Rst::set( 0, 1 ) );
        if ( result ) {
            _logDebug( "reset: Reset command sent successfully" );
        } else {
            _logError( "reset: Failed to send sensor reset command" );
        }
        if ( delay && result ) {
            _logDebug( "reset: Waiting for %d ms", details::kResetWaitMs );
            _platform.delayMs( details::kResetWaitMs );
        }
        return result;
    }

    /**
     * @brief Read the bus voltage from the INA219 sensor and convert it to millivolts (mV).
     *
     * The bus voltage is read from the bus voltage register, which provides a raw value that needs
     * to be multiplied by the LSB value (4mV) to get the actual voltage in millivolts.
     *
     * @param mv Reference to a variable where the bus voltage in millivolts (mV) will be stored.
     * @return true if the read operation was successful and the value was stored; false otherwise.
     */
    bool readBusVoltageMv( std::uint16_t& mv ) noexcept {
        std::uint16_t rawValue{};
        if ( !_readRegister( details::RegisterAddress::BusVoltage, rawValue ) ) {
            _logError( "readBusVoltageMv: Failed to read bus voltage" );
            return false;    // Read failed, return false as error indicator
        }

        _lastBusCnvr = ( details::BusVoltageReg::Cnvr::get( rawValue ) != 0 );
        _lastBusOvf = ( details::BusVoltageReg::Ovf::get( rawValue ) != 0 );

        mv = details::BusVoltageReg::Bd::get( rawValue )
             * details::kBusVoltageLsbMv;    // Convert to mV (LSB = 4mV)

        _logDebug( "readBusVoltageMv: voltage=%u mV, CNVR=%d, OVF=%d, raw=0x%04X",
                   mv,
                   _lastBusCnvr,
                   _lastBusOvf,
                   rawValue );
        return true;
    }

    /**
     * @brief Sets the shunt resistor value and automatically calculates the appropriate current LSB
     * for calibration.
     *
     * @param rShunt The value of the shunt resistor in ohms (e.g., 0.1 for 100mΩ). Must be greater
     * than 0.
     * @param delay If true, the method will introduce a delay (`10` milliseconds) after writing the
     * calibration value to allow the sensor to stabilize. Default is true.
     * @return true if the shunt resistor value is valid and calibration was successful; false
     * otherwise.
     */
    bool setShuntResistor( double rShunt, bool delay = true ) noexcept {
        if ( rShunt <= 0 ) {
            _logError( "setShuntResistor: Invalid shunt resistor value: %f", rShunt );
            return false;
        }

        // Auto‑LSB: max resolution for this R_SHUNT
        // I = V/R, max current that won't exceed shunt voltage limit
        double maxCurrentSafe = details::kMaxVShuntV / rShunt;
        double currentLsb = maxCurrentSafe / details::kMaxCurrentRawValue;

        _logInfo( "setShuntResistor: Calculated current LSB: %f A/bit for R_SHUNT: %f",
                  currentLsb,
                  rShunt );

        return setCalibration( rShunt, currentLsb, delay );
    }

    /**
     * @brief Sets the calibration register based on the shunt resistor value and desired current
     * LSB, ensuring that the resulting current and power measurements are within the sensor's
     * limits.
     *
     * @param rShunt The value of the shunt resistor in ohms (e.g., 0.1 for 100mΩ).
     * @param currentLsb The desired current resolution in amperes per bit (e.g., 0.0001 for
     * 100µA/bit).
     * @param delay If true, the method will introduce a delay (`10` milliseconds) after writing the
     * calibration value to allow the sensor to stabilize. Default is true.
     * @return true if the calculated calibration value is within the sensor's limits and was
     * successfully written to the calibration register; false otherwise.
     */
    bool setCalibration( double rShunt, double currentLsb, bool delay = true ) noexcept {
        if ( rShunt <= 0 || currentLsb <= 0 ) {
            _logError( "setCalibration: Invalid parameters. R_SHUNT: %f, Current LSB: %f",
                       rShunt,
                       currentLsb );
            return false;
        }

        auto cal = static_cast<std::uint16_t>( details::kCalConstant / ( rShunt * currentLsb ) );

        _logDebug( "setCalibration: Calculated CAL value: %u for R_SHUNT: %f, Current LSB: %f",
                   cal,
                   rShunt,
                   currentLsb );

        _rShunt = rShunt;
        _currentLsb = currentLsb;

        bool result{};
        result = _writeRegister( details::RegisterAddress::Calibration, cal );
        if ( delay && result ) {
            _logDebug( "setCalibration: Waiting for %d ms for calibration to take effect",
                       details::kCalibrationWaitMs );
            _platform.delayMs( details::kCalibrationWaitMs );
        }
        return result;
    }

    /**
     * @brief Sets the calibration register based on the shunt resistor value and maximum expected
     * current, automatically calculating the appropriate current LSB to ensure that measurements
     * are within the sensor's limits.
     *
     * @param rShunt The value of the shunt resistor in ohms (e.g., 0.1 for 100mΩ).
     * @param maxExpectedCurrent The maximum expected current in amperes (e.g., 2.0 for 2A). Must be
     * greater than 0.
     * @param delay If true, the method will introduce a delay (`10` milliseconds) after writing the
     * calibration value to allow the sensor to stabilize. Default is true.
     * @return true if the calculated calibration value is within the sensor's limits and was
     * successfully written to the calibration register; false otherwise.
     */
    bool setCalibrationMaxCurrent(
        double rShunt, double maxExpectedCurrent, bool delay = true ) noexcept {
        if ( rShunt <= 0 || maxExpectedCurrent <= 0 ) {
            _logError(
                "setCalibrationMaxCurrent: Invalid parameters. R_SHUNT: %f, Max Expected " "Current" ": %f",
                rShunt,
                maxExpectedCurrent );
            return false;
        }

        // Compute safe LSB for this max current
        double currentLsb = maxExpectedCurrent / details::kMaxCurrentRawValue;

        _logInfo(
            "setCalibrationMaxCurrent: Calculated current LSB: %f A/bit for R_SHUNT: %f, Max " "Ex" "pe" "ct" "e" "d " "Current" ": %f",
            currentLsb,
            rShunt,
            maxExpectedCurrent );

        return setCalibration( rShunt, currentLsb, delay );
    }

    /**
     * @brief Get the currently set shunt resistor value in ohms.
     *
     * @return The shunt resistor value in ohms. Returns 0 if the shunt resistor has not been set or
     * if the value is invalid.
     */
    [[nodiscard]] double getShuntResistor() const noexcept {
        return _rShunt;
    }

    /**
     * @brief Get the currently set current LSB value in amperes per bit.
     *
     * Set using `setCalibration` or is automatically calculated in `setShuntResistor` and
     * `setCalibrationMaxCurrent`.
     *
     * @return The current LSB value in amperes per bit.
     * Returns 0 if the current LSB has not been set or if the value is invalid.
     */
    [[nodiscard]] double getCurrentLsb() const noexcept {
        return _currentLsb;
    }

    /**
     * @brief Read the shunt voltage from the INA219 sensor and convert it to microvolts (µV).
     *
     * The shunt voltage is read from the shunt voltage register, which provides a raw value that
     * needs to be multiplied by the LSB value (10µV) to get the actual voltage in microvolts.
     *
     * @param uv Reference to a variable where the shunt voltage in microvolts (µV) will be stored.
     * @return true if the read operation was successful and the value was stored; false otherwise.
     */
    bool readShuntVoltageUv( int32_t& uv ) noexcept {
        std::uint16_t rawValue{};
        if ( !_readRegister( details::RegisterAddress::ShuntVoltage, rawValue ) ) {
            _logError( "readShuntVoltageUv: Failed to read shunt voltage" );
            return false;
        }

        // Two's complement signed value (sign already extended in the 16-bit word)
        const auto kRawSigned = static_cast<int16_t>( rawValue );
        uv = static_cast<int32_t>( kRawSigned )
             * details::kShuntVoltageLsbUv;    // Convert to µV (LSB = 10µV)

        _logDebug( "readShuntVoltageUv: shunt voltage = %d µV, raw = 0x%04X", uv, rawValue );

        return true;
    }

    /**
     * @brief Read the raw current value from the INA219 sensor's current register, which is a
     * signed 16-bit integer. The raw value needs to be multiplied by the current LSB (set in
     * calibration) to get the actual current in amperes.
     *
     * @param raw Reference to a variable where the raw current value will be stored.
     * @return true if the read operation was successful and the value was stored; false otherwise.
     */
    bool readCurrentRaw( std::int16_t& raw ) noexcept {
        std::uint16_t rawValue{};
        if ( !_readRegister( details::RegisterAddress::Current, rawValue ) ) {
            _logError( "readCurrentRaw: Failed to read raw current" );
            return false;    // Read failed, return false as error indicator
        }

        raw = static_cast<int16_t>( rawValue );    // Current register is signed 16-bit

        _logDebug( "readCurrentRaw: raw = 0x%04X", rawValue );

        return true;
    }

    /**
     * @brief Read the current from the INA219 sensor and convert it to milliamperes (mA) using
     * the current LSB set in calibration.
     *
     * @param ma Reference to a variable where the current in milliamperes (mA) will be stored.
     * @return true if the read operation was successful and the value was stored; false otherwise.
     */
    bool readCurrentMa( double& ma ) noexcept {
        std::int16_t rawCurrent{};
        if ( !readCurrentRaw( rawCurrent ) ) {
            _logError( "readCurrentMa: Failed to read current" );
            return false;
        }

        ma = rawCurrent * _currentLsb * details::kToMilli;    // Convert to mA

        _logDebug( "readCurrentMa: current = %f mA, raw = 0x%04X", ma, rawCurrent );

        return true;
    }

    /**
     * @brief Read the raw power value from the INA219 sensor's power register, which is a signed
     * 16-bit integer. The raw value needs to be multiplied by the power LSB (20 times the current
     * LSB) to get the actual power in watts.
     *
     * @param raw Reference to a variable where the raw power value will be stored.
     * @return true if the read operation was successful and the value was stored; false otherwise.
     */
    bool readPowerRaw( std::int16_t& raw ) noexcept {
        std::uint16_t rawValue{};
        if ( !_readRegister( details::RegisterAddress::Power, rawValue ) ) {
            _logError( "readPowerRaw: Failed to read raw power" );
            return false;    // Read failed, return false as error indicator
        }

        raw = static_cast<int16_t>( rawValue );    // Power register is signed 16-bit

        _logDebug( "readPowerRaw: raw = 0x%04X", rawValue );

        return true;
    }

    /**
     * @brief Read the power from the INA219 sensor and convert it to milliwatts (mW) using the
     * current LSB set in calibration.
     *
     * @param mw Reference to a variable where the power in milliwatts (mW) will be stored.
     * @return true if the read operation was successful and the value was stored; false otherwise.
     */
    bool readPowerMw( double& mw ) noexcept {
        std::int16_t rawPower{};
        if ( !readPowerRaw( rawPower ) ) {
            _logError( "readPowerMw: Failed to read power in milliwatts" );
            return false;
        }

        mw = rawPower * _currentLsb * details::kPowerLsbMultiplier * details::kToMilli;

        _logDebug( "readPowerMw: power = %f mW, raw = 0x%04X", mw, rawPower );

        return true;
    }

    /**
     * @brief Get the last Conversion Ready (CNVR) flag observed.
     *
     * This value is only refreshed when you call `readBusVoltageMv()` (or any other function
     * that reads the Bus Voltage register).
     *
     * @return True if the last Bus Voltage register read indicated conversion-ready.
     *
     * @note CNVR is mainly useful in triggered/one-shot modes to know when a
     * fresh conversion result is ready; in continuous mode you may ignore
     */
    [[nodiscard]] bool lastConversionReady() const noexcept {
        return _lastBusCnvr;
    }

    /**
     * @brief Get the last Math Overflow (OVF) flag observed.
     *
     * This value is only refreshed when you call readBusVoltageMv() (or any other function that
     * reads the Bus Voltage register).
     *
     * @return True if the last Bus Voltage register read indicated math overflow.
     *
     * @note If OVF is true, current and power readings may be meaningless because the device's math
     * overflowed. You may want to treat this as an error, so you don't take into account the
     * current / power.
     */
    [[nodiscard]] bool lastMathOverflow() const noexcept {
        return _lastBusOvf;
    }

    /**
     * @brief Set the I2C address of the INA219 sensor based on the A0/A1 pin configuration.
     *
     * @param address The desired I2C address (enum `Address`) corresponding to the A0/A1 pin
     * states.
     */
    void setAddress( Address address ) noexcept {
        _addr = address;
    }

    /**
     * @brief Get the currently set I2C address of the INA219 sensor.
     *
     * @return The I2C address (enum `Address`) currently set for the sensor.
     */
    [[nodiscard]] Address getAddress() const noexcept {
        return _addr;
    }

    /**
     * @brief Access the owned platform instance.
     * @return Reference to the platform object used for I2C, delays and logging.
     */
    [[nodiscard]] P& getPlatformInstance() const noexcept {
        return _platform;
    }

  private:
    /// I2C address of the INA219 sensor (based on A0/A1 pin configuration)
    Address _addr{ Address::A0GndA1Gnd };
    /// Platform implementation used by this driver instance.
    P _platform{};

    /// Shunt resistor value in ohms (for calibration)
    double _rShunt = 0.0;
    /// Current LSB in amperes (for calibration)
    double _currentLsb = 0.0;

    /// Last observed CNVR (Conversion Ready) flag from the Bus Voltage register, updated each time
    /// `readBusVoltageMv()` is called
    bool _lastBusCnvr{ false };
    /// Last observed OVF flag from the Bus Voltage register, updated each time `readBusVoltageMv()`
    /// is called
    bool _lastBusOvf{ false };

    /**
     * @brief Write a 16-bit value to a specified INA219 register over I2C.
     *
     * @param reg The register address to write to (enum `details::RegisterAddress`).
     * @param value The 16-bit value to write to the register.
     * @return true if the write operation was successful; false otherwise.
     */
    bool _writeRegister( details::RegisterAddress reg, std::uint16_t value ) {
        std::uint8_t data[3];
        data[0] = static_cast<std::uint8_t>( reg );
        data[1] = static_cast<std::uint8_t>(
            ( value >> details::kByteShift ) & details::kByteMask );          // High byte
        data[2] = static_cast<std::uint8_t>( value & details::kByteMask );    // Low byte
        bool result = _platform.i2cWrite( static_cast<std::uint8_t>( _addr ),
                                          static_cast<const std::uint8_t*>( data ),
                                          sizeof( data ) );
        if ( result ) {
            _logDebug( "writeRegister: reg=0x%02X, value=0x%04X", reg, value );
        } else {
            _logError(
                "writeRegister: Failed to write register 0x%02X with value 0x%04X", reg, value );
        }
        return result;
    }

    /**
     * @brief Read a 16-bit value from a specified INA219 register over I2C.
     *
     * @param reg The register address to read from (enum `details::RegisterAddress`).
     * @param value Reference to a variable where the read 16-bit value will be stored.
     * @return true if the read operation was successful and the value was stored; false otherwise
     */
    bool _readRegister( details::RegisterAddress reg, std::uint16_t& value ) {
        std::uint8_t data[2];
        auto regAddr = static_cast<std::uint8_t>( reg );
        if ( !_platform.i2cWriteRead( static_cast<std::uint8_t>( _addr ),
                                      &regAddr,
                                      1,
                                      static_cast<std::uint8_t*>( data ),
                                      sizeof( data ) ) ) {
            _logError( "readRegister: Failed to read register address 0x%02X", reg );
            return false;
        }
        value = ( data[0] << details::kByteShift ) | data[1];
        _logDebug( "readRegister: reg=0x%02X, value=0x%04X", reg, value );
        return true;
    }

    /**
     * @brief Helper method for logging debug messages using the provider's logging mechanism.
     *
     * @param fmt The format string (printf-style) for the debug message.
     * @param ... Additional arguments for the format string.
     */
    void _logDebug( const char* fmt, ... ) noexcept {
#if INA219_LOG_LEVEL >= 4
        va_list args;
        va_start( args, fmt );
        _platform.logDebug( fmt, args );
        va_end( args );
#else
        (void)fmt;
#endif
    }

    /**
     * @brief Helper method for logging informational messages using the provider's logging
     * mechanism.
     *
     * @param fmt The format string (printf-style) for the informational message.
     * @param ... Additional arguments for the format string.
     */
    void _logInfo( const char* fmt, ... ) noexcept {
#if INA219_LOG_LEVEL >= 3
        std::va_list args;
        va_start( args, fmt );
        _platform.logInfo( fmt, args );
        va_end( args );
#else
        (void)fmt;
#endif
    }

    /**
     * @brief Helper method for logging warning messages using the provider's logging mechanism.
     *
     * @param fmt The format string (printf-style) for the warning message.
     * @param ... Additional arguments for the format string.
     */
    void _logWarning( const char* fmt, ... ) noexcept {
#if INA219_LOG_LEVEL >= 2
        std::va_list args;
        va_start( args, fmt );
        _platform.logWarning( fmt, args );
        va_end( args );
#else
        (void)fmt;
#endif
    }

    /**
     * @brief Helper method for logging error messages using the provider's logging mechanism.
     *
     * @param fmt The format string (printf-style) for the error message.
     * @param ... Additional arguments for the format string.
     */
    void _logError( const char* fmt, ... ) noexcept {
#if INA219_LOG_LEVEL >= 1
        std::va_list args;
        va_start( args, fmt );
        _platform.logError( fmt, args );
        va_end( args );
#else
        (void)fmt;
#endif
    }
};

}    // namespace ina219
