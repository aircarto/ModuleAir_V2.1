/*****************************************************************
 * OTA (Over-The-Air) Update System for ModuleAir
 *
 * Refactored version with improved memory management and non-blocking design.
 *
 * Server requirements:
 * - Host a version.json file at http://aircarto.fr/ota/version.json with
 *format:
 *   {"version": "V2-012026", "url": "firmware.bin", "md5": "abc123..."}
 * - Host the firmware.bin file
 *
 * Features:
 * - Manual update trigger from web interface at /update
 * - MD5 checksum verification
 * - HTTP (no HTTPS to save ~40KB RAM)
 * - Non-blocking check design
 *
 *****************************************************************/

#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

// OTA Server Configuration
// Change these values to point to your firmware server
#define OTA_SERVER_HOST "aircarto.fr"
#define OTA_SERVER_PORT 80
#define OTA_VERSION_PATH "/ota/version.json"
#define OTA_BASE_PATH "/ota/"

// OTA check timeout and retry settings
#define OTA_CHECK_TIMEOUT_MS 15000
#define OTA_DOWNLOAD_TIMEOUT_MS 60000
#define OTA_BUFFER_SIZE 1024

// OTA State - holds all information about the update status
struct OTAState {
  bool updateAvailable;
  bool updateInProgress;
  bool updateSuccess;
  bool updateFailed;
  bool checkInProgress;
  String availableVersion;
  String firmwareUrl;
  String firmwareMd5;
  String lastError;
  int updateProgress;
  unsigned long lastCheckTime;
};

// Global OTA state
extern OTAState otaState;

// Forward declarations
void ota_init();
bool ota_check_for_update();
bool ota_perform_update();
void ota_reset_state();
String ota_get_current_version();

/*****************************************************************
 * OTA Implementation
 *****************************************************************/

OTAState otaState = {
    false, // updateAvailable
    false, // updateInProgress
    false, // updateSuccess
    false, // updateFailed
    false, // checkInProgress
    "",    // availableVersion
    "",    // firmwareUrl
    "",    // firmwareMd5
    "",    // lastError
    0,     // updateProgress
    0      // lastCheckTime
};

// Initialize OTA system
void ota_init() {
  ota_reset_state();
  Debug.println(F("[OTA] System initialized"));
}

// Get current firmware version (extract version part after "ModuleAirV2-")
String ota_get_current_version() {
  String fullVersion = SOFTWARE_VERSION_STR;
  int dashPos = fullVersion.indexOf('-');
  if (dashPos > 0) {
    return fullVersion.substring(dashPos + 1);
  }
  return fullVersion;
}

// Compare version strings
// Returns: 1 if v1 > v2, -1 if v1 < v2, 0 if equal
static int ota_compare_versions(const String &v1, const String &v2) {
  if (v1 == v2)
    return 0;
  return (v1 > v2) ? 1 : -1;
}

// Check for available updates - lightweight HTTP version
bool ota_check_for_update() {
  // Prevent concurrent checks
  if (otaState.checkInProgress) {
    Debug.println(F("[OTA] Check already in progress"));
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    otaState.lastError = "WiFi not connected";
    Debug.println(F("[OTA] WiFi not connected, skipping update check"));
    return false;
  }

  otaState.checkInProgress = true;
  otaState.lastError = "";

  Debug.println(F("[OTA] Checking for firmware updates..."));
  Debug.printf("[OTA] Free heap before check: %d bytes\n", ESP.getFreeHeap());

  // Allow pending tasks to complete
  yield();
  delay(50);

  // Use WiFiClientSecure for HTTPS (server redirects HTTP to HTTPS)
  // setInsecure() skips certificate validation to save memory
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  String url = "https://";
  url += OTA_SERVER_HOST;
  url += OTA_VERSION_PATH;

  Debug.print(F("[OTA] Fetching: "));
  Debug.println(url);

  http.begin(client, url);
  http.setTimeout(OTA_CHECK_TIMEOUT_MS);
  http.addHeader("User-Agent", "ModuleAir-OTA/2.0");
  http.addHeader("Connection", "close");

  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    otaState.lastError = "HTTP error: " + String(httpCode);
    Debug.printf("[OTA] HTTP error: %d\n", httpCode);
    http.end();
    otaState.checkInProgress = false;
    otaState.lastCheckTime = millis(); // Record check time even on failure
    return false;
  }

  String payload = http.getString();
  http.end();

  // Free up memory immediately
  yield();

  Debug.print(F("[OTA] Response: "));
  Debug.println(payload);

  // Parse JSON response with small buffer
  StaticJsonDocument<384> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    otaState.lastError = "JSON parse error: " + String(error.c_str());
    Debug.printf("[OTA] JSON parse error: %s\n", error.c_str());
    otaState.checkInProgress = false;
    return false;
  }

  // Extract version info
  String serverVersion = doc["version"] | "";
  String firmwareUrl = doc["url"] | "";
  String firmwareMd5 = doc["md5"] | "";

  if (serverVersion.isEmpty() || firmwareUrl.isEmpty()) {
    otaState.lastError = "Invalid version.json format";
    Debug.println(F("[OTA] Invalid version.json format"));
    otaState.checkInProgress = false;
    return false;
  }

  // Compare versions
  String currentVersion = ota_get_current_version();
  Debug.printf("[OTA] Current version: %s\n", currentVersion.c_str());
  Debug.printf("[OTA] Server version: %s\n", serverVersion.c_str());

  if (ota_compare_versions(serverVersion, currentVersion) > 0) {
    otaState.updateAvailable = true;
    otaState.availableVersion = serverVersion;

    // Build full firmware URL if it's relative
    if (firmwareUrl.startsWith("http")) {
      otaState.firmwareUrl = firmwareUrl;
    } else {
      otaState.firmwareUrl = "https://";
      otaState.firmwareUrl += OTA_SERVER_HOST;
      if (!firmwareUrl.startsWith("/")) {
        otaState.firmwareUrl += OTA_BASE_PATH;
      }
      otaState.firmwareUrl += firmwareUrl;
    }

    otaState.firmwareMd5 = firmwareMd5;
    otaState.lastCheckTime = millis();

    Debug.println(F("[OTA] Update available!"));
    Debug.printf("[OTA] Firmware URL: %s\n", otaState.firmwareUrl.c_str());

    otaState.checkInProgress = false;
    return true;
  } else {
    otaState.updateAvailable = false;
    otaState.lastCheckTime = millis();
    Debug.println(F("[OTA] Firmware is up to date"));

    otaState.checkInProgress = false;
    return false;
  }
}

// Update progress callback
static void ota_progress_callback(size_t current, size_t total) {
  if (total > 0) {
    otaState.updateProgress = (current * 100) / total;
  }

  // Print progress every 10%
  static int lastPrinted = -1;
  int currentPercent = (otaState.updateProgress / 10) * 10;
  if (currentPercent != lastPrinted) {
    Debug.printf("[OTA] Progress: %d%%\n", otaState.updateProgress);
    lastPrinted = currentPercent;
  }
}

// Perform the actual firmware update
bool ota_perform_update() {
  if (!otaState.updateAvailable || otaState.firmwareUrl.isEmpty()) {
    otaState.lastError = "No update available";
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) {
    otaState.lastError = "WiFi not connected";
    return false;
  }

  Debug.println(F("[OTA] Starting firmware update..."));
  Debug.printf("[OTA] URL: %s\n", otaState.firmwareUrl.c_str());
  Debug.printf("[OTA] Free heap: %d bytes\n", ESP.getFreeHeap());

  otaState.updateInProgress = true;
  otaState.updateProgress = 0;
  otaState.updateFailed = false;
  otaState.updateSuccess = false;
  otaState.lastError = "";

  // Allow pending tasks to complete
  yield();
  delay(100);

  // Use WiFiClientSecure for HTTPS (firmware URL uses HTTPS)
  WiFiClientSecure client;
  client.setInsecure(); // Skip certificate validation

  HTTPClient http;

  http.begin(client, otaState.firmwareUrl);
  http.setTimeout(OTA_DOWNLOAD_TIMEOUT_MS);
  http.addHeader("User-Agent", "ModuleAir-OTA/2.0");

  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    otaState.lastError = "Download failed: HTTP " + String(httpCode);
    otaState.updateInProgress = false;
    otaState.updateFailed = true;
    Debug.printf("[OTA] Download failed: %d\n", httpCode);
    http.end();
    return false;
  }

  int contentLength = http.getSize();
  Debug.printf("[OTA] Firmware size: %d bytes\n", contentLength);

  if (contentLength <= 0) {
    otaState.lastError = "Invalid content length";
    otaState.updateInProgress = false;
    otaState.updateFailed = true;
    http.end();
    return false;
  }

  // Check if there's enough space
  if (!Update.begin(contentLength)) {
    otaState.lastError = "Not enough space for update";
    otaState.updateInProgress = false;
    otaState.updateFailed = true;
    Debug.println(F("[OTA] Not enough space"));
    http.end();
    return false;
  }

  // Set MD5 if provided for verification
  if (!otaState.firmwareMd5.isEmpty()) {
    Update.setMD5(otaState.firmwareMd5.c_str());
    Debug.printf("[OTA] MD5: %s\n", otaState.firmwareMd5.c_str());
  }

  // Get the stream
  WiFiClient *stream = http.getStreamPtr();

  // Write firmware with progress tracking
  Debug.println(F("[OTA] Writing firmware..."));

  size_t written = 0;
  size_t total = contentLength;
  uint8_t buff[OTA_BUFFER_SIZE];

  while (http.connected() && (contentLength > 0 || contentLength == -1)) {
    size_t available = stream->available();

    if (available) {
      size_t toRead = (available > sizeof(buff)) ? sizeof(buff) : available;
      size_t bytesRead = stream->readBytes(buff, toRead);

      if (bytesRead > 0) {
        size_t bytesWritten = Update.write(buff, bytesRead);

        if (bytesWritten != bytesRead) {
          otaState.lastError = "Write error";
          otaState.updateInProgress = false;
          otaState.updateFailed = true;
          Debug.println(F("[OTA] Write error"));
          Update.abort();
          http.end();
          return false;
        }

        written += bytesWritten;

        if (contentLength > 0) {
          contentLength -= bytesRead;
        }

        // Update progress
        ota_progress_callback(written, total);
      }
    }

    yield();
    delay(1); // Small delay to prevent watchdog
  }

  http.end();

  // Finalize update
  if (Update.end(true)) {
    if (Update.isFinished()) {
      otaState.updateSuccess = true;
      otaState.updateInProgress = false;
      otaState.updateAvailable = false;
      otaState.updateProgress = 100;
      Debug.println(F("[OTA] Update successful! Rebooting..."));
      return true;
    } else {
      otaState.lastError = "Update not finished";
      otaState.updateFailed = true;
      otaState.updateInProgress = false;
      Debug.println(F("[OTA] Update not finished"));
      return false;
    }
  } else {
    otaState.lastError = "Update error: " + String(Update.getError());
    otaState.updateFailed = true;
    otaState.updateInProgress = false;
    Debug.printf("[OTA] Update error: %d\n", Update.getError());
    return false;
  }
}

// Reset OTA state (useful after failed update or to prepare for new check)
void ota_reset_state() {
  otaState.updateAvailable = false;
  otaState.updateInProgress = false;
  otaState.updateSuccess = false;
  otaState.updateFailed = false;
  otaState.checkInProgress = false;
  otaState.availableVersion = "";
  otaState.firmwareUrl = "";
  otaState.firmwareMd5 = "";
  otaState.lastError = "";
  otaState.updateProgress = 0;
  otaState.lastCheckTime = 0;
}

#endif // OTA_UPDATE_H
