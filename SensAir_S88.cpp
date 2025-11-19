/*
 * SensAir S88 CO2 Sensor Library
 * Communication via Modbus RTU protocol over UART
 */

#include "SensAir_S88.h"

// Modbus command to read CO2:
// 0xFE = sensor address
// 0x04 = function "read input register"
// 0x00 0x03 = register address (0x0003 = CO2)
// 0x00 0x01 = number of registers to read (1)
// 0xD5 0xC5 = CRC
const uint8_t SensAir_S88::READ_CO2_CMD[] = {0xFE, 0x04, 0x00, 0x03, 0x00, 0x01, 0xD5, 0xC5};

SensAir_S88::SensAir_S88() : _serial(nullptr), _lastError(0) {
    memset(_response, 0, sizeof(_response));
}

void SensAir_S88::begin(HardwareSerial &serial) {
    _serial = &serial;
    _lastError = 0;
}

int SensAir_S88::getCO2() {
    if (_serial == nullptr) {
        _lastError = 1; // Serial not initialized
        return -1;
    }

    // Clear the buffer (discard old data)
    while (_serial->available() > 0) {
        _serial->read();
    }

    // Send the command to the sensor
    _serial->write(READ_CO2_CMD, CMD_LENGTH);
    _serial->flush(); // Wait for transmission to complete

    // Wait for sensor response (200ms)
    delay(200);

    // Read the response
    if (!readResponse()) {
        _lastError = 2; // Timeout or incomplete response
        return -1;
    }

    // Verify response validity
    if (!isResponseValid()) {
        _lastError = 3; // Invalid response
        return -1;
    }

    // Extract CO2 value from bytes 3 and 4
    // Example: 0x02 0x4D = (2 × 256) + 77 = 589 ppm
    int co2 = (_response[3] << 8) | _response[4];

    _lastError = 0; // Success
    return co2;
}

bool SensAir_S88::readResponse() {
    int bytesRead = 0;
    unsigned long startTime = millis();

    // Read up to RESPONSE_LENGTH bytes with 1 second timeout
    while (bytesRead < RESPONSE_LENGTH && (millis() - startTime) < 1000) {
        if (_serial->available() > 0) {
            _response[bytesRead] = _serial->read();
            bytesRead++;
        }
    }

    // Check if we received the expected number of bytes
    return (bytesRead >= RESPONSE_LENGTH);
}

bool SensAir_S88::isResponseValid() {
    // Check header bytes
    // Expected: 0xFE (address) and 0x04 (function code)
    if (_response[0] != 0xFE || _response[1] != 0x04) {
        return false;
    }

    // Check data length byte
    // Expected: 0x02 (2 bytes of data)
    if (_response[2] != 0x02) {
        return false;
    }

    return true;
}

uint8_t SensAir_S88::getLastError() {
    return _lastError;
}
