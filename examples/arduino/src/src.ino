#include <Arduino.h>
#include <Wire.h>
#include <ina219/ina219.hpp>

#include <cstdarg>
#include <cstdint>
#include <cstdio>

class ArduinoPlatform {
  public:
    ArduinoPlatform() = default;

    bool i2cWrite( std::uint8_t addr, const std::uint8_t* data, std::size_t len ) {
        Wire.beginTransmission( addr );
        Wire.write( data, len );
        std::uint8_t status = Wire.endTransmission( true );
        return status == 0;
    }

    bool i2cWriteRead( std::uint8_t addr,
                       const std::uint8_t* wdata,
                       std::size_t wlen,
                       std::uint8_t* rdata,
                       std::size_t rlen ) {
        Wire.beginTransmission( addr );
        Wire.write( wdata, wlen );

        // false = repeated start (no STOP) between write and read
        std::uint8_t status = Wire.endTransmission( false );
        if ( status != 0 ) {
            return false;
        }

        // true = send STOP after the read (typical behavior)
        std::uint8_t got = Wire.requestFrom(
            static_cast<int>( addr ), static_cast<int>( rlen ), static_cast<int>( true ) );
        if ( got != static_cast<std::uint8_t>( rlen ) ) {
            return false;
        }

        for ( std::size_t i = 0; i < rlen; i++ ) {
            int c = Wire.read();
            if ( c < 0 ) {
                return false;
            }
            rdata[i] = static_cast<std::uint8_t>( c );
        }
        return true;
    }

    void delayMs( std::uint32_t ms ) {
        delay( ms );
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
    void _log( const char* fmt, std::va_list args ) {
        char buf[96];
        vsnprintf( buf, sizeof( buf ), fmt, args );
        Serial.print( F( "[INA219] " ) );
        Serial.println( buf );
    }
};

static void printReadings(
    std::uint32_t busVoltage_mV, std::int32_t shuntVoltage_uV, float current_mA, float power_mW ) {
    Serial.println( F( "#############################" ) );
    Serial.print( F( "# Bus Voltage  : " ) );
    Serial.print( busVoltage_mV );
    Serial.println( F( " mV #" ) );
    Serial.print( F( "# Shunt Voltage: " ) );
    Serial.print( shuntVoltage_uV );
    Serial.println( F( " uV #" ) );
    Serial.print( F( "# Current      : " ) );
    Serial.print( current_mA, 2 );
    Serial.println( F( " mA #" ) );
    Serial.print( F( "# Power        : " ) );
    Serial.print( power_mW, 2 );
    Serial.println( F( " mW #" ) );
    Serial.println( F( "#############################" ) );
    Serial.println();
}

ina219::Ina219<ArduinoPlatform> sensor;

void setup() {
    Serial.begin( 115'200 );
    delay( 1500 );

    Wire.begin();                // Uno/Nano: SDA=A4, SCL=A5
    Wire.setClock( 400'000 );    // 400 kHz like your Pico example

    Serial.println( F( "INA219 example using Arduino" ) );

    sensor.reset();
    sensor.configure()
        .busRange( ina219::BusRange::V16 )
        .pgaGain( ina219::PgaGain::Gain8 )
        .busAdcMode( ina219::AdcMode::Adc12bit128Samples )
        .shuntAdcMode( ina219::AdcMode::Adc12bit128Samples )
        .operatingMode( ina219::Mode::ShuntBusVoltageContinuous );

    sensor.setShuntResistor( 0.1 );    // 0.1 ohm
}

void loop() {
    std::uint16_t busVoltage = 0;
    std::int32_t shuntVoltage = 0;
    double currentMa_d = 0.0, powerMw_d = 0.0;

    sensor.readBusVoltageMv( busVoltage );
    sensor.readShuntVoltageUv( shuntVoltage );
    sensor.readCurrentMa( currentMa_d );
    sensor.readPowerMw( powerMw_d );

    float currentMa = static_cast<float>( currentMa_d );
    float powerMw = static_cast<float>( powerMw_d );

    printReadings( busVoltage, shuntVoltage, currentMa, powerMw );

    delay( 500 );
}
