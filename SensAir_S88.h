/*
 * SensAir S88 CO2 Sensor Library
 * Communication via Modbus RTU protocol over UART
 */

#ifndef SENSAIR_S88_H
#define SENSAIR_S88_H

#include <Arduino.h>
#include <HardwareSerial.h>

class SensAir_S88 {
public:
    SensAir_S88();

    // Initialize the sensor with serial port
    void begin(HardwareSerial &serial);

    // Read CO2 value in ppm
    // Returns CO2 value or -1 on error
    int getCO2();

    // Get last error (for debugging)
    uint8_t getLastError();

private:
    HardwareSerial *_serial;
    uint8_t _lastError;

    // Modbus command to read CO2
    static const uint8_t READ_CO2_CMD[];
    static const uint8_t CMD_LENGTH = 8;
    static const uint8_t RESPONSE_LENGTH = 7;

    // Response buffer
    uint8_t _response[10];

    // Helper function to read sensor response
    bool readResponse();

    // Verify response validity
    bool isResponseValid();
};

#endif // SENSAIR_S88_H
