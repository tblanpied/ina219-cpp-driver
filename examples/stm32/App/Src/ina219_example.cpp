#include "ina219_example.hpp"

#include "i2c.h"
#include "usart.h"

#include <ina219/ina219.hpp>

#include <cstdarg>
#include <cstdint>
#include <cstdio>

void uartVPrint( const char* fmt, std::va_list args ) {
    char buffer[256];
    int len = vsnprintf( buffer, sizeof( buffer ), fmt, args );
    if ( len <= 0 ) {
        return;
    }

    if ( len > static_cast<int>( sizeof( buffer ) ) ) {
        len = sizeof( buffer );
    }

    HAL_UART_Transmit( &huart1,
                       reinterpret_cast<std::uint8_t*>( buffer ),
                       static_cast<std::uint16_t>( len ),
                       HAL_MAX_DELAY );
}

void uartPrint( const char* fmt, ... ) {
    std::va_list args;
    va_start( args, fmt );
    uartVPrint( fmt, args );
    va_end( args );
}

class Stm32Platform {
  public:
    explicit Stm32Platform( I2C_HandleTypeDef* i2c = &hi2c1 ) : _i2c( i2c ) {
    }

    bool i2cWrite( std::uint8_t addr, const std::uint8_t* data, std::size_t len ) {
        return HAL_I2C_Master_Transmit(
                   _i2c,
                   static_cast<std::uint16_t>( addr << 1 ),
                   const_cast<std::uint8_t*>( data ),
                   static_cast<std::uint16_t>( len ),
                   HAL_MAX_DELAY )
               == HAL_OK;
    }

    bool i2cWriteRead( std::uint8_t addr,
                       const std::uint8_t* wdata,
                       std::size_t wlen,
                       std::uint8_t* rdata,
                       std::size_t rlen ) {
        if ( wlen == 1 ) {
            return HAL_I2C_Mem_Read(
                       _i2c,
                       static_cast<std::uint16_t>( addr << 1 ),
                       wdata[0],
                       I2C_MEMADD_SIZE_8BIT,
                       rdata,
                       static_cast<std::uint16_t>( rlen ),
                       HAL_MAX_DELAY )
                   == HAL_OK;
        }

        if ( HAL_I2C_Master_Transmit( _i2c,
                                      static_cast<std::uint16_t>( addr << 1 ),
                                      const_cast<std::uint8_t*>( wdata ),
                                      static_cast<std::uint16_t>( wlen ),
                                      HAL_MAX_DELAY )
             != HAL_OK ) {
            return false;
        }

        return HAL_I2C_Master_Receive(
                   _i2c,
                   static_cast<std::uint16_t>( addr << 1 ),
                   rdata,
                   static_cast<std::uint16_t>( rlen ),
                   HAL_MAX_DELAY )
               == HAL_OK;
    }

    void delayMs( std::uint32_t ms ) {
        HAL_Delay( ms );
    }

    void logDebug( const char* fmt, std::va_list args ) {
        log( "[INA219] ", fmt, args );
    }

    void logInfo( const char* fmt, std::va_list args ) {
        log( "[INA219] ", fmt, args );
    }

    void logWarning( const char* fmt, std::va_list args ) {
        log( "[INA219] ", fmt, args );
    }

    void logError( const char* fmt, std::va_list args ) {
        log( "[INA219] ", fmt, args );
    }

  private:
    I2C_HandleTypeDef* _i2c;

    void log( const char* prefix, const char* fmt, std::va_list args ) {
        uartPrint( "%s", prefix );
        uartVPrint( fmt, args );
        uartPrint( "\r\n" );
    }
};

void printReadings( std::uint32_t busVoltage_mV,
                    std::int32_t shuntVoltage_uV,
                    double current_mA,
                    double power_mW ) {
    static int first = 1;
    if ( first == 0 ) {
        uartPrint( "\033[6A" );
    }
    first = 0;

    uartPrint( "\033[2K\r#############################\r\n" );
    uartPrint(
        "\033[2K\r# Bus Voltage  : %7lu mV #\r\n", static_cast<unsigned long>( busVoltage_mV ) );
    uartPrint( "\033[2K\r# Shunt Voltage: %7ld uV #\r\n", static_cast<long>( shuntVoltage_uV ) );
    uartPrint( "\033[2K\r# Current      : %7.2f mA #\r\n", current_mA );
    uartPrint( "\033[2K\r# Power        : %7.2f mW #\r\n", power_mW );
    uartPrint( "\033[2K\r#############################\r\n" );
}

extern "C" void INA219_Example_Run( void ) {
    HAL_Delay( 300 );
    uartPrint( "INA219 example on STM32\r\n" );

    ina219::Ina219<Stm32Platform> ina219;

    ina219.reset();
    ina219.configure()
        .busRange( ina219::BusRange::V16 )
        .pgaGain( ina219::PgaGain::Gain8 )
        .busAdcMode( ina219::AdcMode::Adc12bit128Samples )
        .shuntAdcMode( ina219::AdcMode::Adc12bit128Samples )
        .operatingMode( ina219::Mode::ShuntBusVoltageContinuous );
    ina219.setShuntResistor( 0.1 );

    while ( true ) {
        std::uint16_t busVoltage{};
        std::int32_t shuntVoltage{};
        double currentMa{}, powerMw{};

        ina219.readBusVoltageMv( busVoltage );
        ina219.readShuntVoltageUv( shuntVoltage );
        ina219.readCurrentMa( currentMa );
        ina219.readPowerMw( powerMw );

        printReadings( busVoltage, shuntVoltage, currentMa, powerMw );
        HAL_Delay( 500 );
    }
}
