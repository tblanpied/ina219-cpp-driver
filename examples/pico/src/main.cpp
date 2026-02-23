#include <hardware/i2c.h>
#include <ina219/ina219.hpp>
#include <pico/stdlib.h>

#include <cstdint>
#include <cstdio>

class PicoPlatform {
  public:
    explicit PicoPlatform( i2c_inst* i2c_inst = i2c0 ) : _i2cInst( i2c_inst ) {
    }

    bool i2cWrite( uint8_t addr, const uint8_t* data, std::size_t len, bool nostop = false ) {
        int result = i2c_write_blocking( _i2cInst, addr, data, len, nostop );
        return result == static_cast<int>( len );
    }

    bool i2cRead( uint8_t addr, uint8_t* data, std::size_t len, bool nostop = false ) {
        int result = i2c_read_blocking( _i2cInst, addr, data, len, nostop );
        return result == static_cast<int>( len );
    }

    void delayMs( uint32_t ms ) {
        sleep_ms( ms );
    }

    void logDebug( const char* fmt, std::va_list args ) {
        _log( fmt, args );
    }

    void logInfo( const char* fmt, std::va_list args ) {
        _log( fmt, args );
    }

    void logWarning( const char* fmt, std::va_list args ) {
        _log( fmt, args );
    }

    void logError( const char* fmt, std::va_list args ) {
        _log( fmt, args );
    }

  private:
    i2c_inst* _i2cInst;

    void _log( const char* fmt, std::va_list args ) {
        printf( "[INA219] " );
        vprintf( fmt, args );
        printf( "\n" );
    }
};

static void printReadings(
    uint32_t busVoltage_mV, int32_t shuntVoltage_uV, double current_mA, double power_mW ) {
    // First call prints normally; subsequent calls move up 4 lines.
    static int first = 1;
    if ( first == 0 ) {
        // Move cursor up 6 lines
        printf( "\033[6A" );    // ESC[6A
    }
    first = 0;

    // For each line: clear line, return to start, print fixed-width fields
    printf( "\033[2K\r#############################\n" );
    printf( "\033[2K\r# Bus Voltage  : %7lu mV #\n", busVoltage_mV );
    printf( "\033[2K\r# Shunt Voltage: %7ld uV #\n", shuntVoltage_uV );
    printf( "\033[2K\r# Current      : %7.2f mA #\n", current_mA );
    printf( "\033[2K\r# Power        : %7.2f mW #\n", power_mW );
    printf( "\033[2K\r#############################\n" );

    fflush( stdout );
}

int main() {
    stdio_init_all();

    // Initialize I2C bus and timer
    i2c_init( i2c0, 400 * 1000 );             // Initialize I2C at 400kHz
    gpio_set_function( 4, GPIO_FUNC_I2C );    // SDA on GPIO4
    gpio_set_function( 5, GPIO_FUNC_I2C );    // SCL on GPIO5
    gpio_pull_up( 4 );                        // Enable pull-up for SDA
    gpio_pull_up( 5 );                        // Enable pull-up for SCL

    sleep_ms( 3000 );
    printf( "INA219 example on Raspberry Pi Pico\n" );

    ina219::Ina219<PicoPlatform> ina219;

    // If the 'PicoPlatform' constructor was not default constructible:
    // ina219::Ina219<PicoPlatform> ina219(PicoPlatform{i2c0});

    ina219.reset();    // Reset sensor to default state
    ina219.configure()
        .busRange( ina219::BusRange::V16 )
        .pgaGain( ina219::PgaGain::Gain8 )
        .busAdcMode( ina219::AdcMode::Adc12bit128Samples )
        .shuntAdcMode( ina219::AdcMode::Adc12bit128Samples )
        .operatingMode( ina219::Mode::ShuntBusVoltageContinuous );
    ina219.setShuntResistor( 0.1 );

    while ( true ) {
        uint16_t busVoltage{};
        int32_t shuntVoltage{};
        double currentMa{}, powerMw{};

        ina219.readBusVoltageMv( busVoltage );
        ina219.readShuntVoltageUv( shuntVoltage );
        ina219.readCurrentMa( currentMa );
        ina219.readPowerMw( powerMw );

        printReadings( busVoltage, shuntVoltage, currentMa, powerMw );

        sleep_ms( 500 );
    }

    return 0;
}
