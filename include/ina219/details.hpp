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
 * @file details.hpp
 * @brief Header file containing implementation details for the INA219 sensor C++ driver.
 * @version 1.0.0
 * @author Timothée Blanpied
 * @date 2026-03-02
 */

#include <cstdarg>
#include <cstdint>
#include <limits>

#if defined( __cpp_concepts ) && ( __cpp_concepts >= 201'907L )
#define INA219_HAS_CONCEPTS 1
#else
#define INA219_HAS_CONCEPTS 0
#endif

#if INA219_HAS_CONCEPTS
#include <concepts>

namespace ina219::details {

/**
 * @brief Concept describing the platform services required by the INA219 driver.
 *
 * This concept defines a *non-virtual*, compile-time interface (a “platform/policy”
 * type) used by the INA219 driver template.
 *
 * A conforming platform type is responsible for:
 * - I2C write-only transactions (register writes, configuration writes, etc.).
 * - I2C write-then-read transactions (typical register reads using repeated-start).
 * - Millisecond delays.
 * - Logging functions (printf-style format string + `va_list`).
 *
 * @note The driver assumes 7-bit I2C addresses (without the R/W bit).
 *
 * @tparam P Platform type.
 */
template<typename P>
concept Platform = requires(
    P& p,
    std::uint8_t addr,
    const std::uint8_t* tx,
    std::size_t tx_len,
    std::uint8_t* rx,
    std::size_t rx_len,
    std::uint32_t ms,
    const char* fmt,
    va_list args ) {
    /**
     * @brief Write bytes on the I2C bus (write-only transaction).
     *
     * Typical usage:
     * - Write a register address + payload.
     * - Write configuration/calibration values.
     *
     * @param addr   7-bit I2C address.
     * @param tx     Pointer to TX buffer.
     * @param tx_len Number of bytes to write.
     * @return true on success, false on failure (NACK/bus error/etc.).
     */
    { p.i2cWrite( addr, tx, tx_len ) } -> std::convertible_to<bool>;

    /**
     * @brief Write then read on the I2C bus (combined transaction).
     *
     * Typical usage:
     * - Write a register pointer (often 1 byte).
     * - Then read back N bytes from that register.
     *
     * This primitive is intended to represent the common I2C “register read”
     * pattern in a single call, so platforms can map it to their optimal backend
     * (e.g., a dedicated write+read function).
     *
     * @param addr   7-bit I2C address.
     * @param tx     Pointer to TX buffer (e.g., register address bytes).
     * @param tx_len Number of bytes to write.
     * @param rx     Pointer to RX buffer to fill.
     * @param rx_len Number of bytes to read.
     * @return true on success, false on failure.
     */
    { p.i2cWriteRead( addr, tx, tx_len, rx, rx_len ) } -> std::convertible_to<bool>;

    /**
     * @brief Delay execution for at least @p ms milliseconds.
     *
     * Used to wait for reset/configuration/calibration effects to settle.
     *
     * @param ms Milliseconds to delay.
     */
    { p.delayMs( ms ) } -> std::same_as<void>;

    /**
     * @brief Log a debug message.
     * @param fmt  printf-style format string.
     * @param args `va_list` argument list.
     */
    { p.logDebug( fmt, args ) } -> std::same_as<void>;

    /**
     * @brief Log an informational message.
     * @param fmt  printf-style format string.
     * @param args `va_list` argument list.
     */
    { p.logInfo( fmt, args ) } -> std::same_as<void>;

    /**
     * @brief Log a warning message.
     * @param fmt  printf-style format string.
     * @param args `va_list` argument list.
     */
    { p.logWarning( fmt, args ) } -> std::same_as<void>;

    /**
     * @brief Log an error message.
     * @param fmt  printf-style format string.
     * @param args `va_list` argument list.
     */
    { p.logError( fmt, args ) } -> std::same_as<void>;
};

}    // namespace ina219::details
#else

#include <utility>

namespace ina219::details {

/**
 * @brief Helper alias used to implement a “detection idiom” (SFINAE) pre-C++20 concepts.
 *
 * This alias becomes `void` if all provided expressions are well-formed; otherwise the
 * specialization is discarded.
 */
template<typename...>
using void_t = void;

/**
 * @brief Trait that evaluates to `true` when a type satisfies the INA219 platform API.
 *
 * This is the fallback for environments where C++20 concepts are not enabled.
 *
 * The required API mirrors the `Platform` concept:
 * - `bool i2cWrite(uint8_t, const uint8_t*, size_t)`
 * - `bool i2cWriteRead(uint8_t, const uint8_t*, size_t, uint8_t*, size_t)`
 * - `void delayMs(uint32_t)`
 * - `void logDebug(const char*, va_list)` and friends
 *
 * @tparam P Candidate platform type.
 * @tparam Enable Internal SFINAE parameter (do not specify).
 */
template<typename P, typename Enable = void>
struct is_platform : std::false_type {};

/**
 * @brief `is_platform` specialization enabled when all required expressions are valid.
 *
 * This specialization:
 * 1) Checks that the required member functions exist (are well-formed).
 * 2) Checks return types: I2C methods convertible to `bool`, others exactly `void`.
 *
 * @tparam P Candidate platform type.
 */
template<typename P>
struct is_platform<
    P,
    void_t<
        // Existence checks (well-formedness)
        decltype( std::declval<P&>().i2cWrite( std::declval<std::uint8_t>(),
                                               std::declval<const std::uint8_t*>(),
                                               std::declval<std::size_t>() ) ),

        decltype( std::declval<P&>().i2cWriteRead( std::declval<std::uint8_t>(),
                                                   std::declval<const std::uint8_t*>(),
                                                   std::declval<std::size_t>(),
                                                   std::declval<std::uint8_t*>(),
                                                   std::declval<std::size_t>() ) ),

        decltype( std::declval<P&>().delayMs( std::declval<std::uint32_t>() ) ),

        decltype( std::declval<P&>().logDebug(
            std::declval<const char*>(), std::declval<std::va_list>() ) ),
        decltype( std::declval<P&>().logInfo(
            std::declval<const char*>(), std::declval<std::va_list>() ) ),
        decltype( std::declval<P&>().logWarning(
            std::declval<const char*>(), std::declval<std::va_list>() ) ),
        decltype( std::declval<P&>().logError(
            std::declval<const char*>(), std::declval<std::va_list>() ) )>> :
    std::bool_constant<
        // Return-type checks
        std::is_convertible_v<decltype( std::declval<P&>().i2cWrite(
                                  std::declval<std::uint8_t>(),
                                  std::declval<const std::uint8_t*>(),
                                  std::declval<std::size_t>() ) ),
                              bool>
        && std::is_convertible_v<decltype( std::declval<P&>().i2cWriteRead(
                                     std::declval<std::uint8_t>(),
                                     std::declval<const std::uint8_t*>(),
                                     std::declval<std::size_t>(),
                                     std::declval<std::uint8_t*>(),
                                     std::declval<std::size_t>() ) ),
                                 bool>
        && std::
            is_same_v<decltype( std::declval<P&>().delayMs( std::declval<std::uint32_t>() ) ), void>
        && std::is_same_v<decltype( std::declval<P&>().logDebug(
                              std::declval<const char*>(), std::declval<std::va_list>() ) ),
                          void>
        && std::is_same_v<decltype( std::declval<P&>().logInfo(
                              std::declval<const char*>(), std::declval<std::va_list>() ) ),
                          void>
        && std::is_same_v<decltype( std::declval<P&>().logWarning(
                              std::declval<const char*>(), std::declval<std::va_list>() ) ),
                          void>
        && std::is_same_v<decltype( std::declval<P&>().logError(
                              std::declval<const char*>(), std::declval<std::va_list>() ) ),
                          void>> {};

}    // namespace ina219::details
#endif

namespace ina219::details {

/**
 * @brief INA219 scaling constants.
 * @details Values reflect datasheet LSB definitions and driver assumptions.
 */
inline constexpr std::uint8_t kBusVoltageLsbMv = 4;       ///< Bus voltage LSB = 4 mV/bit.
inline constexpr std::int32_t kShuntVoltageLsbUv = 10;    ///< Shunt voltage LSB = 10 µV/bit.

/**
 * @brief Default INA219 configuration register value used by the driver.
 * @details BRNG=32V, PG=Gain8, BADC=12-bit, SADC=12-bit, MODE=Shunt+Bus continuous.
 */
inline constexpr std::uint16_t kDefaultConfig = 0x39'9F;

/** @brief INA219 calibration formula constant. */
inline constexpr double kCalConstant = 0.04096;

/** @brief Maximum allowed calibration register value. */
inline constexpr std::uint16_t kMaxCalValue = 0xFF'FE;

/** @brief Maximum shunt voltage for Gain8 (±320 mV). */
inline constexpr double kMaxVShuntV = 0.32;

/** @brief Settling delays. */
inline constexpr std::uint32_t kConfigWaitMs = 10;
inline constexpr std::uint32_t kResetWaitMs = 10;
inline constexpr std::uint32_t kCalibrationWaitMs = 10;

/** @brief Maximum raw signed current value for a 15-bit signed representation. */
inline constexpr double kMaxCurrentRawValue = ( 1 << 15 ) - 1;

/** @brief Power_LSB = 20 * Current_LSB (datasheet). */
inline constexpr double kPowerLsbMultiplier = 20.0;

/** @brief Byte/scale helpers. */
inline constexpr std::uint8_t kByteShift = 8;
inline constexpr std::uint16_t kByteMask = 0x00'FF;
inline constexpr double kToMilli = 1000.0;

/**
 * @brief Enumeration of INA219 register addresses for I2C communication.
 */
enum class RegisterAddress : std::uint8_t {
    Config = 0x00,          ///< Configuration register
    ShuntVoltage = 0x01,    ///< Shunt voltage register
    BusVoltage = 0x02,      ///< Bus voltage register
    Power = 0x03,           ///< Power register
    Current = 0x04,         ///< Current register
    Calibration = 0x05      ///< Calibration register
};

inline constexpr std::uint8_t kReg16Bits = std::numeric_limits<std::uint16_t>::digits;    // 16
inline constexpr std::uint8_t kReg16MaxShift = kReg16Bits - 1;                            // 15
inline constexpr std::uint8_t kFieldMinWidth = 1;

template<std::uint8_t Shift, std::uint8_t Width>
struct RegisterField16 {
    static_assert( Width >= kFieldMinWidth && Width <= kReg16Bits, "Width must be in [1..16]" );
    static_assert( Shift <= kReg16MaxShift, "Shift must be in [0..15]" );
    static_assert( Shift + Width <= kReg16Bits, "Field must fit in a 16-bit register" );

    using reg_t = std::uint16_t;

    static constexpr reg_t widthMask() {
        return static_cast<reg_t>( ( 1U << Width ) - 1U );
    }

    static constexpr reg_t mask() {
        return static_cast<reg_t>( static_cast<unsigned>( widthMask() ) << Shift );
    }

    [[nodiscard]] static constexpr reg_t get( reg_t reg ) noexcept {
        return static_cast<reg_t>( ( reg & mask() ) >> Shift );
    }

    [[nodiscard]] static constexpr reg_t set( reg_t reg, reg_t value ) noexcept {
        constexpr reg_t kMask = mask();
        reg = static_cast<reg_t>( reg & static_cast<reg_t>( ~kMask ) );
        value = static_cast<reg_t>( value & widthMask() );    // clamp to field width
        reg = static_cast<reg_t>( reg | static_cast<reg_t>( value << Shift ) );
        return reg;
    }
};

/**
 * @name Configuration register fields (0x00)
 * @details 16-bit register layout (bit 15..0):
 * @code{.txt}
 *    15    14     13     12 11    10 9 8 7   6 5 4 3    2 1 0
 *  +-----+-----+------+---------+----------+----------+---------+
 *  | RST |  0  | BRNG |   PG    |   BADC   |   SADC   |  MODE   |
 *  +-----+-----+------+---------+----------+----------+---------+
 *   bit15 bit14 bit13  bits12-11  bits10-7   bits6-3    bits2-0
 * @endcode
 * - MODE: Operating mode selection (3 bits).
 * - SADC: Shunt ADC resolution/averaging (4 bits).
 * - BADC: Bus ADC resolution/averaging (4 bits).
 * - PG  : PGA gain / shunt voltage range (2 bits).
 * - BRNG: Bus voltage range (1 bit): 0=16V, 1=32V.
 * - RST : Reset bit (1 bit).
 */
struct ConfigReg {
    static constexpr std::uint8_t kModeShift = 0;
    static constexpr std::uint8_t kModeWidth = 3;

    static constexpr std::uint8_t kSadcShift = 3;
    static constexpr std::uint8_t kSadcWidth = 4;

    static constexpr std::uint8_t kBadcShift = 7;
    static constexpr std::uint8_t kBadcWidth = 4;

    static constexpr std::uint8_t kPgShift = 11;
    static constexpr std::uint8_t kPgWidth = 2;

    static constexpr std::uint8_t kBrngShift = 13;
    static constexpr std::uint8_t kBrngWidth = 1;

    static constexpr std::uint8_t kRstShift = 15;
    static constexpr std::uint8_t kRstWidth = 1;

    using Mode = RegisterField16<kModeShift, kModeWidth>;
    using Sadc = RegisterField16<kSadcShift, kSadcWidth>;
    using Badc = RegisterField16<kBadcShift, kBadcWidth>;
    using Pg = RegisterField16<kPgShift, kPgWidth>;
    using Brng = RegisterField16<kBrngShift, kBrngWidth>;
    using Rst = RegisterField16<kRstShift, kRstWidth>;
};

/**
 * @name Bus Voltage register fields (0x02)
 * @details 16-bit register layout (bit 15..0):
 * @code{.txt}
 *   15 14 13 12 11 10 9 8 7 6 5 4 3  2    1      0
 *  +-------------------------------+---+------+-----+
 *  |                BD             | - | CNVR | OVF |
 *  +-------------------------------+---+------+-----+
 *                bits15-3                bit1   bit2
 * @endcode
 * - BD: Bus voltage data (13 bits).
 * - CNVR: Conversion ready flag.
 * - OVF: Math overflow flag.
 */
struct BusVoltageReg {
    static constexpr std::uint8_t kBdShift = 3;
    static constexpr std::uint8_t kBdWidth = 13;

    static constexpr std::uint8_t kCnvrShift = 1;
    static constexpr std::uint8_t kCnvrWidth = 1;

    static constexpr std::uint8_t kOvfShift = 0;
    static constexpr std::uint8_t kOvfWidth = 1;

    using Bd = RegisterField16<kBdShift, kBdWidth>;
    using Cnvr = RegisterField16<kCnvrShift, kCnvrWidth>;
    using Ovf = RegisterField16<kOvfShift, kOvfWidth>;
};

}    // namespace ina219::details
