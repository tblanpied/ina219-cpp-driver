#pragma once
#include <string>
#include <cstdint>
#include <cstdarg>

namespace ina219
{
    /**
     * @brief INA219 I2C address selection based on A0/A1 pin states (7‑bit)
     *
     * The INA219 sensor allows you to set its I2C address using the A0 and A1 pins.
     * Each pin can be connected to GND, VS+, SDA, or SCL, resulting in 16 possible addresses.
     * This enum maps the combinations of A0 and A1 connections to their corresponding 7-bit I2C addresses.
     * The default address is 0x40 when both A0 and A1 are connected to GND.
     * Refer to the INA219 datasheet for more details on address selection.
     */
    enum class Address : uint8_t
    {
        A0_GND_A1_GND = 0x40, ///< A1=GND, A0=GND     (default)
        A0_VS_A1_GND = 0x41,  ///< A1=GND, A0=VS+
        A0_SDA_A1_GND = 0x44, ///< A1=GND, A0=SDA
        A0_SCL_A1_GND = 0x45, ///< A1=GND, A0=SCL
        A0_GND_A1_VS = 0x42,  ///< A1=VS+, A0=GND
        A0_VS_A1_VS = 0x43,   ///< A1=VS+, A0=VS+
        A0_SDA_A1_VS = 0x46,  ///< A1=VS+, A0=SDA
        A0_SCL_A1_VS = 0x47,  ///< A1=VS+, A0=SCL
        A0_GND_A1_SDA = 0x48, ///< A1=SDA, A0=GND
        A0_VS_A1_SDA = 0x49,  ///< A1=SDA, A0=VS+
        A0_SDA_A1_SDA = 0x4A, ///< A1=SDA, A0=SDA
        A0_SCL_A1_SDA = 0x4B, ///< A1=SDA, A0=SCL
        A0_GND_A1_SCL = 0x4C, ///< A1=SCL, A0=GND
        A0_VS_A1_SCL = 0x4D,  ///< A1=SCL, A0=VS+
        A0_SDA_A1_SCL = 0x4E, ///< A1=SCL, A0=SDA
        A0_SCL_A1_SCL = 0x4F  ///< A1=SCL, A0=SCL
    };

    /**
     * @brief Bus voltage range selection for the INA219 sensor (0-16V or 0-32V).
     *
     * The default value is V32 (0-32V range).
     */
    enum class BusRange : uint8_t
    {
        V16 = 0x0, ///< 0-16V range
        V32 = 0x1  ///< 0-32V range (default)
    };

    /**
     * @brief Programmable Gain Amplifier (PGA) gain settings for the INA219 sensor,
     * which determine the shunt voltage measurement range.
     *
     * The default value is Gain8 (±320mV range).
     */
    enum class PgaGain : uint8_t
    {
        Gain1 = 0x0, ///< /1, ±40mV range
        Gain2 = 0x1, ///< /2, ±80mV range
        Gain4 = 0x2, ///< /4, ±160mV range
        Gain8 = 0x3, ///< /8, ±320mV range (default)
    };

    /**
     * @brief ADC resolution and averaging settings for the INA219 sensor, which affect conversion time and noise performance.
     *
     * Used for both bus voltage and shunt voltage measurements.
     * Higher resolution and more averaging result in longer conversion times but better accuracy.
     * The default value is Adc12bit1Sample (12-bit resolution, single sample, 532µs conversion time).
     */
    enum class AdcMode : uint8_t
    {
        Adc9bit1Sample = 0x0,    ///< 9-bit resolution, 84µs conversion time
        Adc10bit1Sample = 0x1,   ///< 10-bit resolution, 148µs conversion time
        Adc11bit1Sample = 0x2,   ///< 11-bit resolution, 276µs conversion time
        Adc12bit1Sample = 0x3,   ///< 12-bit resolution, 532µs conversion time (default)
        Adc12bit2Samples = 0x9,  ///< 2 samples averaged, 1.06ms conversion time
        Adc12bit4Samples = 0xA,  ///< 4 samples averaged, 2.13ms conversion time
        Adc12bit8Samples = 0xB,  ///< 8 samples averaged, 4.26ms conversion time
        Adc12bit16Samples = 0xC, ///< 16 samples averaged, 8.51ms conversion time
        Adc12bit32Samples = 0xD, ///< 32 samples averaged, 17.02ms conversion time
        Adc12bit64Samples = 0xE, ///< 64 samples averaged, 34.05ms conversion time
        Adc12bit128Samples = 0xF ///< 128 samples averaged, 68.10ms conversion time
    };

    /**
     * @brief Operating modes for the INA219 sensor, which control when measurements are taken and how the sensor behaves.
     *
     * The default mode is ShuntBusVoltageContinuous, which continuously measures both shunt and bus voltage.
     */
    enum class Mode : uint8_t
    {
        PowerDown = 0x0,                ///< Power‑down
        ShuntVoltageTriggered = 0x1,    ///< Triggered measurement of shunt voltage
        BusVoltageTriggered = 0x2,      ///< Triggered measurement of bus voltage
        ShuntBusVoltageTriggered = 0x3, ///< Triggered measurement of both shunt and bus voltage
        AdcOff = 0x4,                   ///< ADC off (no measurements)
        ShuntVoltageContinuous = 0x5,   ///< Continuous measurement of shunt voltage
        BusVoltageContinuous = 0x6,     ///< Continuous measurement of bus voltage
        ShuntBusVoltageContinuous = 0x7 ///< Continuous measurement of both shunt and bus voltage (default)
    };

    /**
     * @brief Abstract interface for platform-specific I2C communication, timing and optional debugging,
     * which the INA219 driver relies on to interact with the sensor.
     */
    class Provider
    {
    public:
        /**
         * @brief Virtual destructor for the Provider interface.
         */
        virtual ~Provider() = default;

        /**
         * @brief I2C write operation.
         *
         * @param addr The 7-bit I2C address of the device.
         * @param data Pointer to the data buffer to be sent.
         * @param len The length of the data buffer in bytes.
         * @param nostop If true, do not send a stop condition after the write (for repeated start scenarios).
         * @return true if the write operation was successful, false otherwise.
         */
        virtual bool i2cWrite(uint8_t addr,
                              const uint8_t *data,
                              std::size_t len,
                              bool nostop = false) = 0;

        /**
         * @brief I2C read operation.
         *
         * @param addr The 7-bit I2C address of the device.
         * @param data Pointer to the buffer where received data will be stored.
         * @param len The length of the data buffer in bytes.
         * @param nostop If true, do not send a stop condition after the read (for repeated start scenarios).
         * @return true if the read operation was successful, false otherwise.
         */
        virtual bool i2cRead(uint8_t addr,
                             uint8_t *data,
                             std::size_t len,
                             bool nostop = false) = 0;

        /**
         * @brief Delay execution for a specified number of milliseconds.
         *
         * @param ms The number of milliseconds to delay.
         */
        virtual void delayMs(uint32_t ms) = 0;

        virtual void logDebug(const char *fmt, std::va_list args)
        {
            (void)fmt;
            (void)args;
        } // default no-op

        virtual void logInfo(const char *fmt, std::va_list args)
        {
            (void)fmt;
            (void)args;
        } // default no-op

        virtual void logWarning(const char *fmt, std::va_list args)
        {
            (void)fmt;
            (void)args;
        } // default no-op

        virtual void logError(const char *fmt, std::va_list args)
        {
            (void)fmt;
            (void)args;
        } // default no-op
    };

    /**
     * @brief C++ driver class for the INA219 current and power monitor sensor.
     *
     * This class provides methods to configure the sensor, read voltage measurements, and manage the I2C communication.
     */
    class Ina219
    {
    private:
        static constexpr uint8_t BUS_VOLTAGE_LSB_MV = 4;               ///< Each bit in bus voltage register represents 4mV
        static constexpr int32_t SHUNT_VOLTAGE_LSB_UV = 10;            ///< Each bit in shunt voltage register represents 10µV
        static constexpr uint16_t DEFAULT_CONFIG = 0x399F;             ///< Default configuration: BRNG=V32, PG=Gain8, BADC=12-bit, SADC=12-bit, MODE=ShuntBusVoltageContinuous
        static constexpr double CAL_CONSTANT = 0.04096;                ///< Calibration constant for calculating CAL value
        static constexpr uint16_t MAX_CAL_VALUE = 0xFFFE;              ///< Maximum allowed value for the calibration register
        static constexpr double MAX_V_SHUNT = 0.32;                    ///< Maximum shunt voltage for Gain8 (±320mV)
        static constexpr uint8_t CONFIG_WAIT_MS = 10;                  ///< Delay in milliseconds to wait after applying configuration for it to take effect
        static constexpr uint8_t RESET_WAIT_MS = 10;                   ///< Delay in milliseconds to wait after resetting the sensor for it to stabilize
        static constexpr uint8_t CALIBRATION_WAIT_MS = 10;             ///< Delay in milliseconds to wait after writing calibration for it to take effect
        static constexpr double MAX_CURRENT_RAW_VALUE = (1 << 15) - 1; ///< Maximum raw current value based on 15-bit signed representation

        /**
         * @brief Enumeration of INA219 register addresses for I2C communication.
         */
        enum class RegisterAddress : uint8_t
        {
            Config = 0x00,       ///< Configuration register
            ShuntVoltage = 0x01, ///< Shunt voltage register
            BusVoltage = 0x02,   ///< Bus voltage register
            Power = 0x03,        ///< Power register
            Current = 0x04,      ///< Current register
            Calibration = 0x05   ///< Calibration register
        };

        /**
         * @brief Helper union to represent the configuration register with bit fields for easy manipulation of individual settings.
         */
        union ConfigRegister
        {
            static constexpr uint8_t MODE_BITS = 3;
            static constexpr uint8_t SADC_BITS = 4;
            static constexpr uint8_t BADC_BITS = 4;
            static constexpr uint8_t PG_BITS = 2;
            static constexpr uint8_t BRNG_BITS = 1;

            uint16_t raw;
            struct
            {
                uint16_t mode : MODE_BITS;
                uint16_t sadc : SADC_BITS;
                uint16_t badc : BADC_BITS;
                uint16_t pg : PG_BITS;
                uint16_t brng : BRNG_BITS;
                uint16_t reserved : 1;
                uint16_t rst : 1;
            } bits;

            constexpr ConfigRegister() : raw(0) {}
            explicit constexpr ConfigRegister(uint16_t value) : raw(value) {}
        };

        /**
         * @brief Helper union to represent the bus voltage register with bit fields for easy access to voltage and status flags.
         */
        union BusVoltageRegister
        {
            uint16_t raw;
            struct
            {
                uint16_t ovf : 1;  ///< Math overflow flag
                uint16_t cnvr : 1; ///< Conversion ready flag
                uint16_t reserved : 1;
                uint16_t voltage : 13; ///< Bus voltage value (LSB = 4mV)
            } bits;

            constexpr BusVoltageRegister() : raw(0) {}
            explicit constexpr BusVoltageRegister(uint16_t value) : raw(value) {}
        };

        /**
         * @brief Builder class for configuring the INA219 sensor using a fluent API,
         * allowing users to set various configuration options in a readable and convenient way.
         * The configuration is applied to the sensor when the builder goes out of scope (in the destructor).
         * This design ensures that the configuration is always applied.
         */
        class ConfigBuilder
        {
        public:
            /**
             * @brief Constructor that initializes the ConfigBuilder with a reference to the parent Ina219 instance.
             * Reads the current configuration from the sensor to preserve unchanged fields, or uses defaults if the read fails.
             */
            explicit ConfigBuilder(Ina219 &ref, bool delay = true);

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
            ConfigBuilder &busRange(BusRange range) noexcept;

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
            ConfigBuilder &pgaGain(PgaGain gain) noexcept;

            /**
             * @brief Set the ADC resolution and averaging mode for bus voltage measurements on the INA219 sensor.
             * Higher resolution and more averaging result in longer conversion times but better accuracy.
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
            ConfigBuilder &busAdcMode(AdcMode mode) noexcept;

            /**
             * @brief Set the ADC resolution and averaging mode for shunt voltage measurements on the INA219 sensor.
             * Higher resolution and more averaging result in longer conversion times but better accuracy.
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
            ConfigBuilder &shuntAdcMode(AdcMode mode) noexcept;

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
             * - `Mode::ShuntAndBusVoltageTriggered`: Both shunt and bus voltage measurements are triggered.
             *
             * - `Mode::AdcOff`: ADC is turned off (no measurements).
             *
             * - `Mode::ShuntVoltageContinuous`: Shunt voltage measurement is continuous.
             *
             * - `Mode::BusVoltageContinuous`: Bus voltage measurement is continuous.
             *
             * - `Mode::ShuntAndBusVoltageContinuous`: Both shunt and bus voltage measurements are continuous.
             *
             * @param mode The desired operating mode (enum `Mode`).
             * @return Reference to the ConfigBuilder for method chaining.
             */
            ConfigBuilder &operatingMode(Mode mode) noexcept;

            /**
             * @brief Destructor that applies the configuration to the INA219 sensor when the builder goes out of scope.
             */
            ~ConfigBuilder();

        private:
            ConfigRegister config_; ///< Local copy of the configuration register being built
            Ina219 &ref_;           ///< Reference to the parent Ina219 instance to apply the configuration
            bool delay_;            ///< Whether to delay after applying configuration to allow it to take effect
        };

        Provider &provider_; ///< Reference to the platform-specific provider for I2C communication and timing
        Address addr_;       ///< I2C address of the INA219 sensor (based on A0/A1 pin configuration)

        double rShunt_ = 0.0;     ///< Shunt resistor value in ohms (for calibration)
        double currentLsb_ = 0.0; ///< Current LSB in amperes (for calibration)

        /**
         * @brief Write a 16-bit value to a specified INA219 register over I2C.
         *
         * @param reg The register address to write to (enum `RegisterAddress`).
         * @param value The 16-bit value to write to the register.
         * @return true if the write operation was successful; false otherwise.
         */
        bool writeRegister(RegisterAddress reg, uint16_t value);

        /**
         * @brief Read a 16-bit value from a specified INA219 register over I2C.
         *
         * @param reg The register address to read from (enum `RegisterAddress`).
         * @param value Reference to a variable where the read 16-bit value will be stored.
         * @return true if the read operation was successful and the value was stored; false otherwise
         */
        bool readRegister(RegisterAddress reg, uint16_t &value);

        /**
         * @brief Helper method for logging debug messages using the provider's logging mechanism.
         *
         * @param fmt The format string (printf-style) for the debug message.
         * @param ... Additional arguments for the format string.
         */
        void logDebug(const char *fmt, ...) noexcept;

        /**
         * @brief Helper method for logging informational messages using the provider's logging mechanism.
         *
         * @param fmt The format string (printf-style) for the informational message.
         * @param ... Additional arguments for the format string.
         */
        void logInfo(const char *fmt, ...) noexcept;

        /**
         * @brief Helper method for logging warning messages using the provider's logging mechanism.
         *
         * @param fmt The format string (printf-style) for the warning message.
         * @param ... Additional arguments for the format string.
         */
        void logWarning(const char *fmt, ...) noexcept;

        /**
         * @brief Helper method for logging error messages using the provider's logging mechanism.
         *
         * @param fmt The format string (printf-style) for the error message.
         * @param ... Additional arguments for the format string.
         */
        void logError(const char *fmt, ...) noexcept;

    public:
        /**
         * @brief Constructor for the Ina219 class, which initializes the driver
         * with a reference to a platform-specific provider and an optional I2C address.
         *
         * @param provider Reference to an implementation of the `Provider` interface for I2C communication and timing.
         * @param address The I2C address of the INA219 sensor (default is `Address::A0_GND_A1_GND` which corresponds to 0x40).
         */
        Ina219(Provider &provider, Address address = Address::A0_GND_A1_GND);

        /**
         * @brief Start configuring the INA219 sensor using the ConfigBuilder, which provides a fluent API
         * for setting various configuration options.
         * The configuration will be applied to the sensor when the ConfigBuilder goes out of scope (in its destructor).
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

         * @param delay If true, the ConfigBuilder will introduce a delay (`10` milliseconds) after applying
         * the configuration to allow the sensor to stabilize. Default is true.
         * @return A ConfigBuilder instance for setting configuration options in a fluent manner.
         */
        ConfigBuilder configure(bool delay = true) noexcept;

        /**
         * @brief Reset the INA219 sensor to its default state by setting the RST bit in the configuration register.
         *
         * @param delay If true, the method will introduce a delay (`10` milliseconds) after sending the reset command.
         * @return true if the reset command was successfully sent to the sensor; false otherwise.
         */
        bool reset(bool delay = true) noexcept;

        /**
         * @brief Read the bus voltage from the INA219 sensor and convert it to millivolts (mV).
         *
         * The bus voltage is read from the bus voltage register, which provides a raw value that needs to be multiplied
         * by the LSB value (4mV) to get the actual voltage in millivolts.
         *
         * @param mv Reference to a variable where the bus voltage in millivolts (mV) will be stored.
         * @return true if the read operation was successful and the value was stored; false otherwise.
         */
        bool readBusVoltageMv(uint16_t &mv) noexcept;

        /**
         * @brief Sets the shunt resistor value and automatically calculates the appropriate current LSB for calibration.
         *
         * @param rShunt The value of the shunt resistor in ohms (e.g., 0.1 for 100mΩ). Must be greater than 0.
         * @param delay If true, the method will introduce a delay (`10` milliseconds) after writing the calibration
         * value to allow the sensor to stabilize. Default is true.
         * @return true if the shunt resistor value is valid and calibration was successful; false otherwise.
         */
        bool setShuntResistor(double rShunt, bool delay = true) noexcept;

        /**
         * @brief Sets the calibration register based on the shunt resistor value and desired current LSB,
         * ensuring that the resulting current and power measurements are within the sensor's limits.
         *
         * @param rShunt The value of the shunt resistor in ohms (e.g., 0.1 for 100mΩ).
         * @param currentLsb The desired current resolution in amperes per bit (e.g., 0.0001 for 100µA/bit).
         * @param delay If true, the method will introduce a delay (`10` milliseconds) after writing the calibration
         * value to allow the sensor to stabilize. Default is true.
         * @return true if the calculated calibration value is within the sensor's limits and was successfully
         * written to the calibration register; false otherwise.
         */
        bool setCalibration(double rShunt, double currentLsb, bool delay = true) noexcept;

        /**
         * @brief Sets the calibration register based on the shunt resistor value and maximum expected current,
         * automatically calculating the appropriate current LSB to ensure that measurements are within the sensor's limits.
         *
         * @param rShunt The value of the shunt resistor in ohms (e.g., 0.1 for 100mΩ).
         * @param maxExpectedCurrent The maximum expected current in amperes (e.g., 2.0 for 2A). Must be greater than 0.
         * @param delay If true, the method will introduce a delay (`10` milliseconds) after writing the calibration
         * value to allow the sensor to stabilize. Default is true.
         * @return true if the calculated calibration value is within the sensor's limits and was successfully
         * written to the calibration register; false otherwise.
         */
        bool setCalibrationMaxCurrent(double rShunt, double maxExpectedCurrent, bool delay = true) noexcept;

        /**
         * @brief Get the currently set shunt resistor value in ohms.
         *
         * @return The shunt resistor value in ohms. Returns 0 if the shunt resistor has not been set or if the value is invalid.
         */
        double getShuntResistor() const noexcept;

        /**
         * @brief Get the currently set current LSB value in amperes per bit.
         *
         * Set using `setCalibration` or is automatically calculated in `setShuntResistor` and `setCalibrationMaxCurrent`.
         *
         * @return The current LSB value in amperes per bit.
         * Returns 0 if the current LSB has not been set or if the value is invalid.
         */
        double getCurrentLsb() const noexcept;

        /**
         * @brief Read the shunt voltage from the INA219 sensor and convert it to microvolts (µV).
         *
         * The shunt voltage is read from the shunt voltage register, which provides a raw value that needs to be multiplied
         * by the LSB value (10µV) to get the actual voltage in microvolts.
         *
         * @param uv Reference to a variable where the shunt voltage in microvolts (µV) will be stored.
         * @return true if the read operation was successful and the value was stored; false otherwise.
         */
        bool readShuntVoltageUv(int32_t &uv) noexcept;

        /**
         * @brief Read the raw current value from the INA219 sensor's current register, which is a signed 16-bit integer.
         * The raw value needs to be multiplied by the current LSB (set in calibration) to get the actual current in amperes.
         *
         * @param raw Reference to a variable where the raw current value will be stored.
         * @return true if the read operation was successful and the value was stored; false otherwise.
         */
        bool readCurrentRaw(int16_t &raw) noexcept;

        /**
         * @brief Read the current from the INA219 sensor and convert it to milliamperes (mA) using
         * the current LSB set in calibration.
         *
         * @param ma Reference to a variable where the current in milliamperes (mA) will be stored.
         * @return true if the read operation was successful and the value was stored; false otherwise.
         */
        bool readCurrentMa(double &ma) noexcept;

        /**
         * @brief Read the raw power value from the INA219 sensor's power register, which is a signed 16-bit integer.
         * The raw value needs to be multiplied by the power LSB (20 times the current LSB) to get the actual power in watts.
         *
         * @param raw Reference to a variable where the raw power value will be stored.
         * @return true if the read operation was successful and the value was stored; false otherwise.
         */
        bool readPowerRaw(int16_t &raw) noexcept;

        /**
         * @brief Read the power from the INA219 sensor and convert it to milliwatts (mW) using the current LSB set in calibration.
         *
         * @param mw Reference to a variable where the power in milliwatts (mW) will be stored.
         * @return true if the read operation was successful and the value was stored; false otherwise.
         */
        bool readPowerMw(double &mw) noexcept;

        /**
         * @brief Set the I2C address of the INA219 sensor based on the A0/A1 pin configuration.
         *
         * @param address The desired I2C address (enum `Address`) corresponding to the A0/A1 pin states.
         */
        void setAddress(Address address) noexcept;

        /**
         * @brief Get the currently set I2C address of the INA219 sensor.
         *
         * @return The I2C address (enum `Address`) currently set for the sensor.
         */
        Address getAddress() const noexcept;
    };

} // namespace ina219
