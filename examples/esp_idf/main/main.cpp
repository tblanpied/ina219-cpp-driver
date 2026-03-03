#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ina219/ina219.hpp>

#include <array>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <mutex>

inline constexpr i2c_port_num_t I2C_PORT = I2C_NUM_0;
inline constexpr gpio_num_t I2C_SDA = GPIO_NUM_4;
inline constexpr gpio_num_t I2C_SCL = GPIO_NUM_5;
inline constexpr std::uint32_t I2C_FREQ_HZ = 400'000;
inline constexpr int I2C_TIMEOUT_MS = 100;

static void printReadings(
    std::uint32_t busVoltage_mV,
    std::int32_t shuntVoltage_uV,
    double current_mA,
    double power_mW ) {
    static int first = 1;
    if ( !first ) {
        std::printf( "\033[6A" );
    }
    first = 0;

    std::printf( "\033[2K\r#############################\n" );
    std::printf( "\033[2K\r# Bus Voltage  : %7lu mV #\n", busVoltage_mV );
    std::printf( "\033[2K\r# Shunt Voltage: %7ld uV #\n", shuntVoltage_uV );
    std::printf( "\033[2K\r# Current      : %7.2f mA #\n", current_mA );
    std::printf( "\033[2K\r# Power        : %7.2f mW #\n", power_mW );
    std::printf( "\033[2K\r#############################\n" );
    std::fflush( stdout );
}

class Esp32Platform {
  public:
    Esp32Platform() {
        init_bus_once();
    }

    bool i2cWrite( std::uint8_t addr, const std::uint8_t* data, std::size_t len ) {
        auto dev = get_dev( addr );
        if ( !dev ) {
            return false;
        }
        return i2c_master_transmit( dev, data, len, I2C_TIMEOUT_MS ) == ESP_OK;
    }

    bool i2cWriteRead( std::uint8_t addr,
                       const std::uint8_t* wdata,
                       std::size_t wlen,
                       std::uint8_t* rdata,
                       std::size_t rlen ) {
        auto dev = get_dev( addr );
        if ( !dev ) {
            return false;
        }
        return i2c_master_transmit_receive( dev, wdata, wlen, rdata, rlen, I2C_TIMEOUT_MS )
               == ESP_OK;
    }

    void delayMs( std::uint32_t ms ) {
        vTaskDelay( pdMS_TO_TICKS( ms ) );
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
    inline static std::once_flag s_once;
    inline static i2c_master_bus_handle_t s_bus = nullptr;

    i2c_master_dev_handle_t _dev{};    // per-address cache

    static void init_bus_once() {
        std::call_once( s_once, [] {
            i2c_master_bus_config_t bus_cfg = {};
            bus_cfg.i2c_port = I2C_PORT;
            bus_cfg.sda_io_num = I2C_SDA;
            bus_cfg.scl_io_num = I2C_SCL;
            bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
            bus_cfg.glitch_ignore_cnt = 7;
            bus_cfg.intr_priority = 0;
            bus_cfg.trans_queue_depth = 0;
            bus_cfg.flags.enable_internal_pullup = 1;

            ESP_ERROR_CHECK( i2c_new_master_bus( &bus_cfg, &s_bus ) );
        } );
    }

    i2c_master_dev_handle_t get_dev( std::uint8_t addr ) {
        if ( !s_bus ) {
            return nullptr;
        }
        if ( _dev ) {
            return _dev;
        }

        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = addr;
        dev_cfg.scl_speed_hz = I2C_FREQ_HZ;
        dev_cfg.scl_wait_us = 0;
        dev_cfg.flags.disable_ack_check = 0;

        if ( i2c_master_bus_add_device( s_bus, &dev_cfg, &_dev ) != ESP_OK ) {
            return nullptr;
        }
        return _dev;
    }

    void _log( const char* fmt, std::va_list args ) {
        std::printf( "[INA219] " );
        std::vprintf( fmt, args );
        std::printf( "\n" );
    }
};

extern "C" void app_main( void ) {
    vTaskDelay( pdMS_TO_TICKS( 3000 ) );
    std::printf( "INA219 example on ESP32-S3 (ESP-IDF new I2C driver)\n" );

    ina219::Ina219<Esp32Platform> ina219;

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
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }
}
