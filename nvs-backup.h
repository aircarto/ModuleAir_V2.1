/*****************************************************************
 * NVS Backup System - Preserve critical data during OTA updates
 *
 * This module backs up critical SPIFFS data to NVS (Non-Volatile Storage)
 * before OTA updates. NVS is never erased during OTA, unlike SPIFFS.
 *
 * Backed up data:
 *   - Custom logos (logo_custom1, logo_custom2) - split into chunks
 *   - Screen configuration (which screens to display)
 *   - config.json (full configuration file)
 *
 * Workflow:
 *   1. Before OTA: Call nvs_backup_all() to save everything to NVS
 *   2. After OTA/boot: Call nvs_restore_if_needed() to restore if SPIFFS was
 *formatted
 *****************************************************************/

#ifndef NVS_BACKUP_H
#define NVS_BACKUP_H

#include "utils.h"
#include <Preferences.h>

// NVS namespace for backups
#define NVS_BACKUP_NAMESPACE "ota_backup"
#define NVS_CONFIG_NAMESPACE "ota_config"

// Logo backup settings
#define LOGO_SIZE 2048                                // 2048 uint16_t values
#define LOGO_FILE_SIZE (LOGO_SIZE * sizeof(uint16_t)) // 4096 bytes
#define NVS_CHUNK_SIZE 1800 // Safe chunk size for NVS (< 4000 bytes limit)
#define LOGO_CHUNKS                                                            \
  ((LOGO_FILE_SIZE + NVS_CHUNK_SIZE - 1) / NVS_CHUNK_SIZE) // 3 chunks per logo

// Config.json max size for NVS backup (putString limit ~4000 chars)
// We split into chunks if needed
#define CONFIG_JSON_CHUNK_SIZE 3900
#define CONFIG_JSON_MAX_CHUNKS 4 // Supports up to ~15.6KB config

// NVS keys
#define NVS_KEY_BACKUP_VALID "backup_ok"
#define NVS_KEY_LOGO1_PREFIX "logo1_"
#define NVS_KEY_LOGO2_PREFIX "logo2_"
#define NVS_KEY_CONFIG_PREFIX "cfg_"
#define NVS_KEY_CONFIG_CHUNKS "cfg_chunks"
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
// These are inside namespace cfg in moduleair.cpp
namespace cfg {
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
} // namespace cfg

/**
 * Backup a logo to NVS in chunks
 */
bool nvs_backup_logo(Preferences &prefs, int logo_num,
                     const uint16_t *logo_data) {
  const char *prefix =
      (logo_num == 1) ? NVS_KEY_LOGO1_PREFIX : NVS_KEY_LOGO2_PREFIX;
  const uint8_t *data = (const uint8_t *)logo_data;

  Debug.printf("[NVS Backup] Backing up logo %d (%d bytes in %d chunks)...\n",
               logo_num, LOGO_FILE_SIZE, LOGO_CHUNKS);

  for (int chunk = 0; chunk < LOGO_CHUNKS; chunk++) {
    char key[16];
    snprintf(key, sizeof(key), "%s%d", prefix, chunk);

    size_t offset = chunk * NVS_CHUNK_SIZE;
    size_t size = NVS_CHUNK_SIZE;

    if (offset + size > LOGO_FILE_SIZE) {
      size = LOGO_FILE_SIZE - offset;
    }

    if (prefs.putBytes(key, data + offset, size) != size) {
      Debug.printf("[NVS Backup] Failed to write chunk %d of logo %d\n", chunk,
                   logo_num);
      return false;
    }
  }

  Debug.printf("[NVS Backup] Logo %d backed up successfully\n", logo_num);
  return true;
}

/**
 * Restore a logo from NVS chunks
 */
bool nvs_restore_logo(Preferences &prefs, int logo_num, uint16_t *logo_buffer) {
  const char *prefix =
      (logo_num == 1) ? NVS_KEY_LOGO1_PREFIX : NVS_KEY_LOGO2_PREFIX;
  uint8_t *data = (uint8_t *)logo_buffer;

  Debug.printf("[NVS Backup] Restoring logo %d from NVS...\n", logo_num);

  for (int chunk = 0; chunk < LOGO_CHUNKS; chunk++) {
    char key[16];
    snprintf(key, sizeof(key), "%s%d", prefix, chunk);

    size_t offset = chunk * NVS_CHUNK_SIZE;
    size_t size = NVS_CHUNK_SIZE;

    if (offset + size > LOGO_FILE_SIZE) {
      size = LOGO_FILE_SIZE - offset;
    }

    size_t read = prefs.getBytes(key, data + offset, size);
    if (read != size) {
      Debug.printf("[NVS Backup] Failed to read chunk %d of logo %d (expected "
                   "%d, got %d)\n",
                   chunk, logo_num, size, read);
      return false;
    }
  }

  Debug.printf("[NVS Backup] Logo %d restored successfully\n", logo_num);
  return true;
}

/**
 * Backup screen configuration to NVS
 */
bool nvs_backup_screen_config(Preferences &prefs) {
  Debug.println("[NVS Backup] Backing up screen configuration...");

  prefs.putBool(NVS_KEY_SCREEN_PM01, cfg::screen_pm01);
  prefs.putBool(NVS_KEY_SCREEN_PM25, cfg::screen_pm25);
  prefs.putBool(NVS_KEY_SCREEN_PM10, cfg::screen_pm10);
  prefs.putBool(NVS_KEY_SCREEN_CO2, cfg::screen_co2);
  prefs.putBool(NVS_KEY_SCREEN_COV, cfg::screen_cov);
  prefs.putBool(NVS_KEY_SCREEN_TEMP, cfg::screen_temp);
  prefs.putBool(NVS_KEY_SCREEN_HUMI, cfg::screen_humi);
  prefs.putBool(NVS_KEY_SCREEN_PRESS, cfg::screen_press);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_PM10, cfg::screen_atmo_pm10);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_PM25, cfg::screen_atmo_pm25);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_INDEX, cfg::screen_atmo_index);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_O3, cfg::screen_atmo_o3);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_NO2, cfg::screen_atmo_no2);
  prefs.putBool(NVS_KEY_SCREEN_ATMO_SO2, cfg::screen_atmo_so2);

  Debug.println("[NVS Backup] Screen configuration backed up successfully");
  return true;
}

/**
 * Restore screen configuration from NVS
 */
bool nvs_restore_screen_config(Preferences &prefs) {
  Debug.println("[NVS Backup] Restoring screen configuration from NVS...");

  cfg::screen_pm01 = prefs.getBool(NVS_KEY_SCREEN_PM01, cfg::screen_pm01);
  cfg::screen_pm25 = prefs.getBool(NVS_KEY_SCREEN_PM25, cfg::screen_pm25);
  cfg::screen_pm10 = prefs.getBool(NVS_KEY_SCREEN_PM10, cfg::screen_pm10);
  cfg::screen_co2 = prefs.getBool(NVS_KEY_SCREEN_CO2, cfg::screen_co2);
  cfg::screen_cov = prefs.getBool(NVS_KEY_SCREEN_COV, cfg::screen_cov);
  cfg::screen_temp = prefs.getBool(NVS_KEY_SCREEN_TEMP, cfg::screen_temp);
  cfg::screen_humi = prefs.getBool(NVS_KEY_SCREEN_HUMI, cfg::screen_humi);
  cfg::screen_press = prefs.getBool(NVS_KEY_SCREEN_PRESS, cfg::screen_press);
  cfg::screen_atmo_pm10 =
      prefs.getBool(NVS_KEY_SCREEN_ATMO_PM10, cfg::screen_atmo_pm10);
  cfg::screen_atmo_pm25 =
      prefs.getBool(NVS_KEY_SCREEN_ATMO_PM25, cfg::screen_atmo_pm25);
  cfg::screen_atmo_index =
      prefs.getBool(NVS_KEY_SCREEN_ATMO_INDEX, cfg::screen_atmo_index);
  cfg::screen_atmo_o3 =
      prefs.getBool(NVS_KEY_SCREEN_ATMO_O3, cfg::screen_atmo_o3);
  cfg::screen_atmo_no2 =
      prefs.getBool(NVS_KEY_SCREEN_ATMO_NO2, cfg::screen_atmo_no2);
  cfg::screen_atmo_so2 =
      prefs.getBool(NVS_KEY_SCREEN_ATMO_SO2, cfg::screen_atmo_so2);

  Debug.println("[NVS Backup] Screen configuration restored successfully");
  return true;
}

/**
 * Backup config.json to NVS
 * Reads the file from SPIFFS and stores it in NVS chunks
 */
bool nvs_backup_config_json() {
  Debug.println("[NVS Backup] Backing up config.json...");

  if (!SPIFFS.exists(F("/config.json"))) {
    Debug.println("[NVS Backup] WARNING: config.json not found on SPIFFS");
    return false;
  }

  File configFile = SPIFFS.open(F("/config.json"), "r");
  if (!configFile) {
    Debug.println("[NVS Backup] ERROR: Could not open config.json");
    return false;
  }

  String configContent = configFile.readString();
  configFile.close();

  size_t totalLen = configContent.length();
  Debug.printf("[NVS Backup] config.json size: %d bytes\n", totalLen);

  if (totalLen == 0) {
    Debug.println("[NVS Backup] WARNING: config.json is empty");
    return false;
  }

  // Store in a separate NVS namespace to avoid key conflicts
  Preferences prefs;
  if (!prefs.begin(NVS_CONFIG_NAMESPACE, false)) {
    Debug.println("[NVS Backup] ERROR: Failed to open config NVS namespace");
    return false;
  }

  // Calculate number of chunks needed
  int numChunks =
      (totalLen + CONFIG_JSON_CHUNK_SIZE - 1) / CONFIG_JSON_CHUNK_SIZE;
  if (numChunks > CONFIG_JSON_MAX_CHUNKS) {
    Debug.printf(
        "[NVS Backup] ERROR: config.json too large (%d bytes, max %d)\n",
        totalLen, CONFIG_JSON_CHUNK_SIZE * CONFIG_JSON_MAX_CHUNKS);
    prefs.end();
    return false;
  }

  // Store chunk count
  prefs.putInt(NVS_KEY_CONFIG_CHUNKS, numChunks);

  // Store each chunk
  bool success = true;
  for (int i = 0; i < numChunks; i++) {
    char key[16];
    snprintf(key, sizeof(key), "%s%d", NVS_KEY_CONFIG_PREFIX, i);

    size_t start = i * CONFIG_JSON_CHUNK_SIZE;
    size_t len = CONFIG_JSON_CHUNK_SIZE;
    if (start + len > totalLen) {
      len = totalLen - start;
    }

    String chunk = configContent.substring(start, start + len);
    if (prefs.putString(key, chunk) == 0) {
      Debug.printf("[NVS Backup] ERROR: Failed to write config chunk %d\n", i);
      success = false;
      break;
    }
  }

  prefs.end();

  if (success) {
    Debug.printf("[NVS Backup] config.json backed up (%d chunks)\n", numChunks);
  }
  return success;
}

/**
 * Restore config.json from NVS to SPIFFS
 */
bool nvs_restore_config_json() {
  Debug.println("[NVS Backup] Restoring config.json from NVS...");

  Preferences prefs;
  if (!prefs.begin(NVS_CONFIG_NAMESPACE, true)) {
    Debug.println("[NVS Backup] ERROR: Failed to open config NVS namespace");
    return false;
  }

  int numChunks = prefs.getInt(NVS_KEY_CONFIG_CHUNKS, 0);
  if (numChunks <= 0 || numChunks > CONFIG_JSON_MAX_CHUNKS) {
    Debug.println("[NVS Backup] No config.json backup found in NVS");
    prefs.end();
    return false;
  }

  // Reconstruct config string from chunks
  String configContent;
  for (int i = 0; i < numChunks; i++) {
    char key[16];
    snprintf(key, sizeof(key), "%s%d", NVS_KEY_CONFIG_PREFIX, i);

    String chunk = prefs.getString(key, "");
    if (chunk.length() == 0) {
      Debug.printf("[NVS Backup] ERROR: Failed to read config chunk %d\n", i);
      prefs.end();
      return false;
    }
    configContent += chunk;
  }

  prefs.end();

  // Write to SPIFFS
  File configFile = SPIFFS.open(F("/config.json"), "w");
  if (!configFile) {
    Debug.println("[NVS Backup] ERROR: Could not create config.json on SPIFFS");
    return false;
  }

  configFile.print(configContent);
  configFile.close();

  Debug.printf("[NVS Backup] config.json restored to SPIFFS (%d bytes)\n",
               configContent.length());
  return true;
}

/**
 * Backup ALL critical data to NVS before OTA
 * This should be called BEFORE starting the OTA update
 */
bool nvs_backup_all() {
  Debug.println("[NVS Backup] ========================================");
  Debug.println("[NVS Backup] Starting full backup to NVS...");
  Debug.println("[NVS Backup] ========================================");

  Preferences prefs;
  if (!prefs.begin(NVS_BACKUP_NAMESPACE, false)) {
    Debug.println("[NVS Backup] ERROR: Failed to open NVS namespace");
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
      Debug.println("[NVS Backup] WARNING: Could not read logo 1 from SPIFFS");
      if (file)
        file.close();
    }
  } else {
    Debug.println("[NVS Backup] WARNING: Logo 1 not found on SPIFFS, backing "
                  "up from buffer");
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
      Debug.println("[NVS Backup] WARNING: Could not read logo 2 from SPIFFS");
      if (file)
        file.close();
    }
  } else {
    Debug.println("[NVS Backup] WARNING: Logo 2 not found on SPIFFS, backing "
                  "up from buffer");
    extern uint16_t logo_buffer2[];
    success = nvs_backup_logo(prefs, 2, logo_buffer2) && success;
  }

  // Backup screen configuration
  success = nvs_backup_screen_config(prefs) && success;

  // Mark backup as valid
  if (success) {
    prefs.putBool(NVS_KEY_BACKUP_VALID, true);
  }

  prefs.end();

  // Backup config.json (uses separate NVS namespace)
  success = nvs_backup_config_json() && success;

  if (success) {
    Debug.println("[NVS Backup] ========================================");
    Debug.println("[NVS Backup] Full backup completed successfully!");
    Debug.println("[NVS Backup] ========================================");
  } else {
    Debug.println("[NVS Backup] ========================================");
    Debug.println("[NVS Backup] ERROR: Backup completed with errors");
    Debug.println("[NVS Backup] ========================================");
  }

  return success;
}

/**
 * Check if NVS backup exists and is valid
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
 *   - SPIFFS appears to be freshly formatted (config.json or logos missing)
 *
 * @return true if restoration was performed
 */
bool nvs_restore_if_needed() {
  // Check if we have a valid backup
  if (!nvs_has_valid_backup()) {
    Debug.println("[NVS Backup] No valid backup found in NVS");
    return false;
  }

  // Check if SPIFFS has been formatted (config.json OR logo files missing)
  bool config_missing = !SPIFFS.exists(F("/config.json"));
  bool logos_missing =
      !SPIFFS.exists(LOGO_CUSTOM1_PATH) || !SPIFFS.exists(LOGO_CUSTOM2_PATH);

  bool needs_restore = config_missing || logos_missing;

  if (!needs_restore) {
    Debug.println("[NVS Backup] SPIFFS data intact, no restore needed");
    return false;
  }

  Debug.println("[NVS Backup] ========================================");
  Debug.println("[NVS Backup] SPIFFS appears to be formatted!");
  Debug.printf("[NVS Backup]   config.json missing: %s\n",
               config_missing ? "YES" : "no");
  Debug.printf("[NVS Backup]   logos missing: %s\n",
               logos_missing ? "YES" : "no");
  Debug.println("[NVS Backup] Restoring from NVS backup...");
  Debug.println("[NVS Backup] ========================================");

  bool success = true;

  // Restore config.json FIRST (most critical)
  if (config_missing) {
    success = nvs_restore_config_json() && success;
  }

  // Restore logos and screen config from main backup namespace
  Preferences prefs;
  if (!prefs.begin(NVS_BACKUP_NAMESPACE, true)) {
    Debug.println("[NVS Backup] ERROR: Failed to open NVS namespace");
    return success;
  }

  if (logos_missing) {
    uint16_t temp_logo_buffer[LOGO_SIZE];

    // Restore Logo 1
    if (nvs_restore_logo(prefs, 1, temp_logo_buffer)) {
      File file = SPIFFS.open(LOGO_CUSTOM1_PATH, "w");
      if (file) {
        file.write((uint8_t *)temp_logo_buffer, LOGO_FILE_SIZE);
        file.close();
        Debug.println("[NVS Backup] Logo 1 restored to SPIFFS");
      } else {
        Debug.println("[NVS Backup] ERROR: Could not write logo 1 to SPIFFS");
        success = false;
      }
    } else {
      success = false;
    }

    // Restore Logo 2
    if (nvs_restore_logo(prefs, 2, temp_logo_buffer)) {
      File file = SPIFFS.open(LOGO_CUSTOM2_PATH, "w");
      if (file) {
        file.write((uint8_t *)temp_logo_buffer, LOGO_FILE_SIZE);
        file.close();
        Debug.println("[NVS Backup] Logo 2 restored to SPIFFS");
      } else {
        Debug.println("[NVS Backup] ERROR: Could not write logo 2 to SPIFFS");
        success = false;
      }
    } else {
      success = false;
    }
  }

  // Always restore screen configuration when any restore is needed
  success = nvs_restore_screen_config(prefs) && success;

  prefs.end();

  if (success) {
    Debug.println("[NVS Backup] ========================================");
    Debug.println("[NVS Backup] Restoration completed successfully!");
    Debug.println("[NVS Backup] ========================================");
  } else {
    Debug.println("[NVS Backup] ========================================");
    Debug.println("[NVS Backup] ERROR: Restoration completed with errors");
    Debug.println("[NVS Backup] ========================================");
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
    Debug.println("[NVS Backup] Backup cleared from NVS");
  }
  if (prefs.begin(NVS_CONFIG_NAMESPACE, false)) {
    prefs.clear();
    prefs.end();
  }
}

#endif // NVS_BACKUP_H
