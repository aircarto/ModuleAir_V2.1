# Gemini Code Understanding

## Project Overview

This project, ModuleAir V2.1, is an open-source air quality sensor built on the ESP32 platform using the Arduino framework and PlatformIO.

The device is designed to:
-   Measure various air quality metrics using a range of sensors:
    -   Particulate Matter (PM1, PM2.5, PM10) via Nova PM SDS011 or NextPM.
    -   CO2 via SensAir S88, MH-Z16, or MH-Z19.
    -   Volatile Organic Compounds (VOC) via CCS811.
    -   Temperature, Humidity, and Pressure via BME280.
-   Display the collected data on either a 64x32 RGB LED matrix or an SSD1306 OLED screen.
-   Transmit data via WiFi or LoRaWAN to various platforms, including AirCarto, AtmoSud, and Sensor.Community.
-   Provide a web interface for on-device configuration.

## Building and Running

The project is managed using PlatformIO.

### Build Environments

The `platformio.ini` file defines several build environments, with the primary ones being:
-   `esp32dev_fr`: Builds the firmware with French language support.
-   `esp32dev_en`: Builds the firmware with English language support.
-   `esp32doit-devkit-v1`: A configuration for a specific board variant.

### Commands

-   **Build:** To compile the code for a specific environment:
    ```bash
    pio run -e esp32dev_fr
    ```
-   **Upload:** To compile and upload the firmware to a connected device:
    ```bash
    pio run -e esp32dev_fr --target upload
    ```
-   **Monitor:** To view serial output from the device:
    ```bash
    pio device monitor
    ```

### ⚠️ Critical Pre-build Step

This project requires manual modification of PlatformIO's installed library files to function correctly. These changes are necessary to enable the use of two separate SPI buses on the ESP32.

1.  **In `SPI.cpp`** (within the project's `.pio/libdeps/.../` directory):
    -   Modify the `SPIClass` instantiation to create two instances, `SPI` and `SPI_H`.
2.  **In `SPI.h`**:
    -   Add `extern SPIClass SPI_H;`
3.  **In `PxMatrix.h`**:
    -   Replace all instances of `SPI.` with `SPI_H.` (except for the `#include <SPI.h>`).

These steps are documented in `README.md` and `moduleair.cpp`.

## Development Conventions

### File Structure & Configuration

The project uses a multi-layered configuration approach:

-   **`platformio.ini`**: Defines build environments, libraries, and core build flags. The source directory is the project root (`.`), not `/src`.
-   **Compile-Time Configuration**:
    -   `ext_def.h`: The primary file for enabling or disabling hardware (sensors, displays) and software features (APIs) using `#define` flags.
    -   `defines.h`: Defines constants, GPIO pin mappings, and sensor sampling intervals.
    -   `configuration.h`: Configures which data screens and logos are shown on the display.
-   **Run-Time Configuration**:
    -   `moduleair.cpp`: Contains the main application logic, including the web server for configuration.
    -   `html-content.h`: Contains the HTML and JavaScript for the web configuration portal.
    -   When the user saves settings via the web portal, they are written to a `config.json` file on the device's SPIFFS filesystem. `moduleair-cfg.h` handles reading from and writing to this file.

### Code Flow

1.  **`setup()`**: Initializes serial communication, reads the config from SPIFFS, starts WiFi (or AP mode), initializes displays and sensors, and starts the web server.
2.  **`loop()`**:
    -   Handles web server client requests.
    -   Periodically reads sensor data, accumulating sums for averaging.
    -   Updates the display every 5 seconds.
    -   Every 2 minutes (`SENDING_INTERVALL_MS`), it calculates averages, builds a JSON payload, and sends the data to the configured endpoints (WiFi APIs or LoRaWAN).

### Code Style

-   The presence of an `astyle.rc` file indicates that the `astyle` tool is used for C++ code formatting.

### Internationalization (i18n)

-   The application supports multiple languages (English and French).
-   The active language is determined at compile time by build flags in `platformio.ini` (`-DINTL_FR` or `-DINTL_EN`).
-   These flags control which language-specific header (`intl_fr.h` or `intl_en.h`) is used to provide the UI strings.
