/*****************************************************************
 * NVS Backup System - Preserve critical data during OTA updates
 *
 * This module backs up critical SPIFFS data to NVS (Non-Volatile Storage)
 * before OTA updates. NVS is never erased during OTA, unlike SPIFFS.
 *
 * Backed up data:
 *   - Custom logos (logo_custom1, logo_custom2) - split into chunks
 *   - Screen configuration (which screens to display)
 *   - Other critical settings
 *
 * Workflow:
 *   1. Before OTA: Call nvs_backup_all() to save everything to NVS
 *   2. After OTA/boot: Call nvs_restore_if_needed() to restore if SPIFFS was formatted
 *****************************************************************/

#ifndef NVS_BACKUP_H
#define NVS_BACKUP_H

#include <Preferences.h>

// NVS namespace for backups
#define NVS_BACKUP_NAMESPACE "ota_backup"

// Logo backup settings
#define LOGO_SIZE 2048                              // 2048 uint16_t values
#define LOGO_FILE_SIZE (LOGO_SIZE * sizeof(uint16_t)) // 4096 bytes
#define NVS_CHUNK_SIZE 1800                         // Safe chunk size for NVS (< 4000 bytes limit)
#define LOGO_CHUNKS ((LOGO_FILE_SIZE + NVS_CHUNK_SIZE - 1) / NVS_CHUNK_SIZE) // 3 chunks per logo

// NVS keys
#define NVS_KEY_BACKUP_VALID "backup_ok"
#define NVS_KEY_LOGO1_PREFIX "logo1_"
#define NVS_KEY_LOGO2_PREFIX "logo2_"
#define NVS_KEY_SCREEN_PM01 "scr_pm01"
#define NVS_KEY_SCREEN_PM25 "scr_pm25"
#define NVS_KEY_SCREEN_PM10 "scr_pm10"
#define NVS_KEY_SCREEN_CO2 "scr_co2"
#define NVS_KEY_SCREEN_COV "scr_cov"
#define NVS_KEY_SCREEN_TEMP "scr_temp"
#define NVS_KEY_SCREEN_HUMI "scr_humi"
#define NVS_KEY_SCREEN_PRESS "scr_press"
#define NVS_KEY_SCREEN_ATMO_PM10 "scr_apm10"
#define NVS_KEY_SCREEN_ATMO_PM25 "scr_apm25"
#define NVS_KEY_SCREEN_ATMO_INDEX "scr_aidx"
#define NVS_KEY_SCREEN_ATMO_O3 "scr_ao3"
#define NVS_KEY_SCREEN_ATMO_NO2 "scr_ano2"
#define NVS_KEY_SCREEN_ATMO_SO2 "scr_aso2"

// External references to screen configuration variables (from moduleair.cpp)
extern bool screen_pm01;
extern bool screen_pm25;
extern bool screen_pm10;
extern bool screen_co2;
extern bool screen_cov;
extern bool screen_temp;
extern bool screen_humi;
extern bool screen_press;
extern bool screen_atmo_pm10;
extern bool screen_atmo_pm25;
extern bool screen_atmo_index;
extern bool screen_atmo_o3;
extern bool screen_atmo_no2;
extern bool screen_atmo_so2;

/**
 * Backup a logo to NVS in chunks
 * @param prefs Preferences object (already opened)
 * @param logo_num 1 or 2
 * @param logo_data Pointer to logo data (2048 uint16_t values)
 * @return true if successful
 */
bool nvs_backup_logo(Preferences &prefs, int logo_num, const uint16_t *logo_data) {
  const char *prefix = (logo_num == 1) ? NVS_KEY_LOGO1_PREFIX : NVS_KEY_LOGO2_PREFIX;
  const uint8_t *data = (const uint8_t *)logo_data;

  Serial.printf("[NVS Backup] Backing up logo %d (%d bytes in %d chunks)...\n",
                logo_num, LOGO_FILE_SIZE, LOGO_CHUNKS);

  for (int chunk = 0; chunk < LOGO_CHUNKS; chunk++) {
    char key[16];
    snprintf(key, sizeof(key), "%s%d", prefix, chunk);

    size_t offset = chunk * NVS_CHUNK_SIZE;
    size_t size = NVS_CHUNK_SIZE;

    // Last chunk may be smaller
    if (offset + size > LOGO_FILE_SIZE) {
      size = LOGO_FILE_SIZE - offset;
    }

    if (prefs.putBytes(key, data + offset, size) != size) {
      Serial.printf("[NVS Backup] Failed to write chunk %d of logo %d\n", chunk, logo_num);
      return false;
    }
  }

  Serial.printf("[NVS Backup] Logo %d backed up successfully\n", logo_num);
  return true;
}

/**
 * Restore a logo from NVS chunks
 * @param prefs Preferences object (already opened)
 * @param logo_num 1 or 2
 * @param logo_buffer Destination buffer (2048 uint16_t values)
 * @return true if successful
 */
bool nvs_restore_logo(Preferences &prefs, int logo_num, uint16_t *logo_buffer) {
  const char *prefix = (logo_num == 1) ? NVS_KEY_LOGO1_PREFIX : NVS_KEY_LOGO2_PREFIX;
  uint8_t *data = (uint8_t *)logo_buffer;

  Serial.printf("[NVS Backup] Restoring logo %d from NVS...\n", logo_num);

  for (int chunk = 0; chunk < LOGO_CHUNKS; chunk++) {
    char key[16];
    snprintf(key, sizeof(key), "%s%d", prefix, chunk);

    size_t offset = chunk * NVS_CHUNK_SIZE;
    size_t size = NVS_CHUNK_SIZE;

    // Last chunk may be smaller
    if (offset + size > LOGO_FILE_SIZE) {
      size = LOGO_FILE_SIZE - offset;
    }

    size_t read = prefs.getBytes(key, data + offset, size);
    if (read != size) {
      Serial.printf("[NVS Backup] Failed to read chunk %d of logo %d (expected %d, got %d)\n",
                    chunk, logo_num, size, read);
      return false;
    }
  }

  Serial.printf("[NVS Backup] Logo %d restored successfully\n", logo_num);
  return true;
}

/**
 * Backup screen configuration to NVS
 * @param prefs Preferences object (already opened)
 * @return true if successful
 */
bool nvs_backup_screen_config(Preferences &prefs) {
  Serial.println("[NVS Backup] Backing up screen configuration...");

  prefs.putBool(NVS_KEY_SCREEN_PM01, screen_pm01);
  prefs.putBool(NVS_KEY_SCREEN_PM25, screen_pm25);
  prefs.putBool(NVS_KEY_SCREEN_PM10, screen_pm10);
  prefs.putBool(NVS_KEY_SCREEN_CO2, screen_co2);
  prefs.putBool(NVS_KEY_SCREEN_COV, screen_cov);
  prefs.putBool(NVS_KEY_SCREEN_TEMP, screen_temp);
  prefs.putBool(NVS_KEY_SCREEN_HUMI, screen_humi);
  prefs.putBool(NVS_KEY_SCREEN_PRESS, screen_press);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_PM10, screen_atmo_pm10);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_PM25, screen_atmo_pm25);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_INDEX, screen_atmo_index);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_O3, screen_atmo_o3);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_NO2, screen_atmo_no2);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_SO2, screen_atmo_so2);

  Serial.println("[NVS Backup] Screen configuration backed up successfully");
  return true;
}

/**
 * Restore screen configuration from NVS
 * @param prefs Preferences object (already opened)
 * @return true if successful
 */
bool nvs_restore_screen_config(Preferences &prefs) {
  Serial.println("[NVS Backup] Restoring screen configuration from NVS...");

  screen_pm01 = prefs.getBool(NVS_KEY_SCREEN_PM01, screen_pm01);
  screen_pm25 = prefs.getBool(NVS_KEY_SCREEN_PM25, screen_pm25);
  screen_pm10 = prefs.getBool(NVS_KEY_SCREEN_PM10, screen_pm10);
  screen_co2 = prefs.getBool(NVS_KEY_SCREEN_CO2, screen_co2);
  screen_cov = prefs.getBool(NVS_KEY_SCREEN_COV, screen_cov);
  screen_temp = prefs.getBool(NVS_KEY_SCREEN_TEMP, screen_temp);
  screen_humi = prefs.getBool(NVS_KEY_SCREEN_HUMI, screen_humi);
  screen_press = prefs.getBool(NVS_KEY_SCREEN_PRESS, screen_press);
  screen_atmo_pm10 = prefs.getBool(NVS_KEY_SCREEN_ATMO_PM10, screen_atmo_pm10);
  screen_atmo_pm25 = prefs.getBool(NVS_KEY_SCREEN_ATMO_PM25, screen_atmo_pm25);
  screen_atmo_index = prefs.getBool(NVS_KEY_SCREEN_ATMO_INDEX, screen_atmo_index);
  screen_atmo_o3 = prefs.getBool(NVS_KEY_SCREEN_ATMO_O3, screen_atmo_o3);
  screen_atmo_no2 = prefs.getBool(NVS_KEY_SCREEN_ATMO_NO2, screen_atmo_no2);
  screen_atmo_so2 = prefs.getBool(NVS_KEY_SCREEN_ATMO_SO2, screen_atmo_so2);

  Serial.println("[NVS Backup] Screen configuration restored successfully");
  return true;
}

/**
 * Backup ALL critical data to NVS before OTA
 * This should be called BEFORE starting the OTA update
 * @return true if backup was successful
 */
bool nvs_backup_all() {
  Serial.println("[NVS Backup] ========================================");
  Serial.println("[NVS Backup] Starting full backup to NVS...");
  Serial.println("[NVS Backup] ========================================");

  Preferences prefs;
  if (!prefs.begin(NVS_BACKUP_NAMESPACE, false)) {
    Serial.println("[NVS Backup] ERROR: Failed to open NVS namespace");
    return false;
  }

  bool success = true;

  // Backup logos from SPIFFS
  uint16_t temp_logo_buffer[LOGO_SIZE];

  // Backup Logo 1
  if (SPIFFS.exists(LOGO_CUSTOM1_PATH)) {
    File file = SPIFFS.open(LOGO_CUSTOM1_PATH, "r");
    if (file && file.size() == LOGO_FILE_SIZE) {
      file.read((uint8_t *)temp_logo_buffer, LOGO_FILE_SIZE);
      file.close();
      success = nvs_backup_logo(prefs, 1, temp_logo_buffer) && success;
    } else {
      Serial.println("[NVS Backup] WARNING: Could not read logo 1 from SPIFFS");
      if (file) file.close();
    }
  } else {
    Serial.println("[NVS Backup] WARNING: Logo 1 not found on SPIFFS, backing up from buffer");
    // Backup from RAM buffer if file doesn't exist
    extern uint16_t logo_buffer1[];
    success = nvs_backup_logo(prefs, 1, logo_buffer1) && success;
  }

  // Backup Logo 2
  if (SPIFFS.exists(LOGO_CUSTOM2_PATH)) {
    File file = SPIFFS.open(LOGO_CUSTOM2_PATH, "r");
    if (file && file.size() == LOGO_FILE_SIZE) {
      file.read((uint8_t *)temp_logo_buffer, LOGO_FILE_SIZE);
      file.close();
      success = nvs_backup_logo(prefs, 2, temp_logo_buffer) && success;
    } else {
      Serial.println("[NVS Backup] WARNING: Could not read logo 2 from SPIFFS");
      if (file) file.close();
    }
  } else {
    Serial.println("[NVS Backup] WARNING: Logo 2 not found on SPIFFS, backing up from buffer");
    // Backup from RAM buffer if file doesn't exist
    extern uint16_t logo_buffer2[];
    success = nvs_backup_logo(prefs, 2, logo_buffer2) && success;
  }

  // Backup screen configuration
  success = nvs_backup_screen_config(prefs) && success;

  // Mark backup as valid
  if (success) {
    prefs.putBool(NVS_KEY_BACKUP_VALID, true);
    Serial.println("[NVS Backup] ========================================");
    Serial.println("[NVS Backup] Full backup completed successfully!");
    Serial.println("[NVS Backup] ========================================");
  } else {
    Serial.println("[NVS Backup] ========================================");
    Serial.println("[NVS Backup] ERROR: Backup completed with errors");
    Serial.println("[NVS Backup] ========================================");
  }

  prefs.end();
  return success;
}

/**
 * Check if NVS backup exists and is valid
 * @return true if valid backup exists
 */
bool nvs_has_valid_backup() {
  Preferences prefs;
  if (!prefs.begin(NVS_BACKUP_NAMESPACE, true)) {
    return false;
  }

  bool valid = prefs.getBool(NVS_KEY_BACKUP_VALID, false);
  prefs.end();
  return valid;
}

/**
 * Restore critical data from NVS if needed
 * Call this after SPIFFS.begin() and before logo_init()
 *
 * This will restore data if:
 *   - Valid NVS backup exists
 *   - SPIFFS appears to be freshly formatted (logos missing)
 *
 * @return true if restoration was performed
 */
bool nvs_restore_if_needed() {
  // Check if we have a valid backup
  if (!nvs_has_valid_backup()) {
    Serial.println("[NVS Backup] No valid backup found in NVS");
    return false;
  }

  // Check if SPIFFS has been formatted (logo files missing)
  bool needs_restore = !SPIFFS.exists(LOGO_CUSTOM1_PATH) || !SPIFFS.exists(LOGO_CUSTOM2_PATH);

  if (!needs_restore) {
    Serial.println("[NVS Backup] SPIFFS data intact, no restore needed");
    return false;
  }

  Serial.println("[NVS Backup] ========================================");
  Serial.println("[NVS Backup] SPIFFS appears to be formatted!");
  Serial.println("[NVS Backup] Restoring from NVS backup...");
  Serial.println("[NVS Backup] ========================================");

  Preferences prefs;
  if (!prefs.begin(NVS_BACKUP_NAMESPACE, true)) {
    Serial.println("[NVS Backup] ERROR: Failed to open NVS namespace");
    return false;
  }

  bool success = true;
  uint16_t temp_logo_buffer[LOGO_SIZE];

  // Restore Logo 1
  if (nvs_restore_logo(prefs, 1, temp_logo_buffer)) {
    // Save to SPIFFS
    File file = SPIFFS.open(LOGO_CUSTOM1_PATH, "w");
    if (file) {
      file.write((uint8_t *)temp_logo_buffer, LOGO_FILE_SIZE);
      file.close();
      Serial.println("[NVS Backup] Logo 1 restored to SPIFFS");
    } else {
      Serial.println("[NVS Backup] ERROR: Could not write logo 1 to SPIFFS");
      success = false;
    }
  } else {
    success = false;
  }

  // Restore Logo 2
  if (nvs_restore_logo(prefs, 2, temp_logo_buffer)) {
    // Save to SPIFFS
    File file = SPIFFS.open(LOGO_CUSTOM2_PATH, "w");
    if (file) {
      file.write((uint8_t *)temp_logo_buffer, LOGO_FILE_SIZE);
      file.close();
      Serial.println("[NVS Backup] Logo 2 restored to SPIFFS");
    } else {
      Serial.println("[NVS Backup] ERROR: Could not write logo 2 to SPIFFS");
      success = false;
    }
  } else {
    success = false;
  }

  // Restore screen configuration
  success = nvs_restore_screen_config(prefs) && success;

  prefs.end();

  if (success) {
    Serial.println("[NVS Backup] ========================================");
    Serial.println("[NVS Backup] Restoration completed successfully!");
    Serial.println("[NVS Backup] ========================================");
  } else {
    Serial.println("[NVS Backup] ========================================");
    Serial.println("[NVS Backup] ERROR: Restoration completed with errors");
    Serial.println("[NVS Backup] ========================================");
  }

  return success;
}

/**
 * Clear NVS backup (call after successful restore if desired)
 */
void nvs_clear_backup() {
  Preferences prefs;
  if (prefs.begin(NVS_BACKUP_NAMESPACE, false)) {
    prefs.clear();
    prefs.end();
    Serial.println("[NVS Backup] Backup cleared from NVS");
  }
}

#endif // NVS_BACKUP_H
