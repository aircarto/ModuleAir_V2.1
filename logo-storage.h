/*****************************************************************
 * Logo Storage - SPIFFS persistence for custom logos
 *
 * This module stores custom logos (logo_custom1, logo_custom2) on SPIFFS
 * so they are preserved after OTA firmware updates.
 *
 * On first boot (no logo files exist):
 *   - Copies compiled logo arrays to SPIFFS
 *
 * On subsequent boots / after OTA:
 *   - Loads logos from SPIFFS files (ignores compiled arrays)
 *****************************************************************/

#ifndef LOGO_STORAGE_H
#define LOGO_STORAGE_H

// Note: SPIFFS.h is already included in moduleair.cpp before this header
// Note: logos-custom.h is already included via intl.h -> intl_fr.h

// Logo dimensions: 64x32 pixels = 2048 pixels, 2 bytes each (uint16_t RGB565)
#define LOGO_SIZE 2048
#define LOGO_FILE_SIZE (LOGO_SIZE * sizeof(uint16_t)) // 4096 bytes

// File paths on SPIFFS
#define LOGO_CUSTOM1_PATH "/logo_custom1.bin"
#define LOGO_CUSTOM2_PATH "/logo_custom2.bin"

// External references to compiled logo arrays (defined in logos-custom.h)
// These are already included via intl.h -> intl_fr.h -> logos-custom.h
extern uint16_t logo_custom1[];
extern uint16_t logo_custom2[];

// RAM buffers for active logos (loaded from SPIFFS)
uint16_t logo_buffer1[LOGO_SIZE];
uint16_t logo_buffer2[LOGO_SIZE];

// Flags to track if logos were loaded successfully
bool logo1_loaded = false;
bool logo2_loaded = false;

/**
 * Save a logo array to SPIFFS
 * @param logo_num 1 or 2 for custom logo 1 or 2
 * @param logo_data Pointer to the logo data array (2048 uint16_t elements)
 * @return true if save was successful
 */
bool logo_save(int logo_num, const uint16_t *logo_data) {
  const char *path = (logo_num == 1) ? LOGO_CUSTOM1_PATH : LOGO_CUSTOM2_PATH;

  File file = SPIFFS.open(path, "w");
  if (!file) {
    Serial.printf("[Logo Storage] Failed to create file: %s\n", path);
    return false;
  }

  size_t written = file.write((const uint8_t *)logo_data, LOGO_FILE_SIZE);
  file.close();

  if (written != LOGO_FILE_SIZE) {
    Serial.printf("[Logo Storage] Write error: expected %d bytes, wrote %d\n",
                  LOGO_FILE_SIZE, written);
    return false;
  }

  Serial.printf("[Logo Storage] Saved logo %d to %s (%d bytes)\n", logo_num,
                path, written);
  return true;
}

/**
 * Load a logo from SPIFFS into a buffer
 * @param logo_num 1 or 2 for custom logo 1 or 2
 * @param buffer Destination buffer (must be at least LOGO_SIZE elements)
 * @return true if load was successful
 */
bool logo_load(int logo_num, uint16_t *buffer) {
  const char *path = (logo_num == 1) ? LOGO_CUSTOM1_PATH : LOGO_CUSTOM2_PATH;

  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.printf("[Logo Storage] Failed to open file: %s\n", path);
    return false;
  }

  size_t file_size = file.size();
  if (file_size != LOGO_FILE_SIZE) {
    Serial.printf("[Logo Storage] Invalid file size: expected %d, got %d\n",
                  LOGO_FILE_SIZE, file_size);
    file.close();
    return false;
  }

  size_t read = file.read((uint8_t *)buffer, LOGO_FILE_SIZE);
  file.close();

  if (read != LOGO_FILE_SIZE) {
    Serial.printf("[Logo Storage] Read error: expected %d bytes, read %d\n",
                  LOGO_FILE_SIZE, read);
    return false;
  }

  Serial.printf("[Logo Storage] Loaded logo %d from %s (%d bytes)\n", logo_num,
                path, read);
  return true;
}

/**
 * Check if a logo file exists on SPIFFS
 * @param logo_num 1 or 2 for custom logo 1 or 2
 * @return true if file exists
 */
bool logo_exists(int logo_num) {
  const char *path = (logo_num == 1) ? LOGO_CUSTOM1_PATH : LOGO_CUSTOM2_PATH;
  return SPIFFS.exists(path);
}

/**
 * Initialize logo storage system
 *
 * For each custom logo:
 *   - If no file exists on SPIFFS: copy from compiled array and save
 *   - Load from SPIFFS into RAM buffer
 *
 * Call this AFTER SPIFFS.begin() in setup()
 */
void logo_init() {
  Serial.println("[Logo Storage] Initializing logo storage...");

  // Logo Custom 1
  if (!logo_exists(1)) {
    Serial.println(
        "[Logo Storage] Logo 1 not found on SPIFFS - saving from firmware...");
    if (logo_save(1, logo_custom1)) {
      Serial.println("[Logo Storage] Logo 1 saved to SPIFFS");
    } else {
      Serial.println("[Logo Storage] Failed to save Logo 1");
    }
  } else {
    Serial.println("[Logo Storage] Logo 1 found on SPIFFS (preserved from "
                   "previous firmware)");
  }

  // Load Logo 1 into buffer
  if (logo_load(1, logo_buffer1)) {
    logo1_loaded = true;
    Serial.println("[Logo Storage] Logo 1 loaded successfully");
  } else {
    // Fallback: use compiled array
    Serial.println("[Logo Storage] Fallback: using compiled Logo 1");
    memcpy(logo_buffer1, logo_custom1, LOGO_FILE_SIZE);
    logo1_loaded = true;
  }

  // Logo Custom 2
  if (!logo_exists(2)) {
    Serial.println(
        "[Logo Storage] Logo 2 not found on SPIFFS - saving from firmware...");
    if (logo_save(2, logo_custom2)) {
      Serial.println("[Logo Storage] Logo 2 saved to SPIFFS");
    } else {
      Serial.println("[Logo Storage] Failed to save Logo 2");
    }
  } else {
    Serial.println("[Logo Storage] Logo 2 found on SPIFFS (preserved from "
                   "previous firmware)");
  }

  // Load Logo 2 into buffer
  if (logo_load(2, logo_buffer2)) {
    logo2_loaded = true;
    Serial.println("[Logo Storage] Logo 2 loaded successfully");
  } else {
    // Fallback: use compiled array
    Serial.println("[Logo Storage] Fallback: using compiled Logo 2");
    memcpy(logo_buffer2, logo_custom2, LOGO_FILE_SIZE);
    logo2_loaded = true;
  }

  Serial.println("[Logo Storage] Initialization complete");
}

/**
 * Force update logo on SPIFFS from new data
 * Useful for updating logos via web interface in the future
 * @param logo_num 1 or 2 for custom logo 1 or 2
 * @param logo_data New logo data
 * @return true if update was successful
 */
bool logo_update(int logo_num, const uint16_t *logo_data) {
  if (logo_save(logo_num, logo_data)) {
    // Also update RAM buffer
    uint16_t *buffer = (logo_num == 1) ? logo_buffer1 : logo_buffer2;
    memcpy(buffer, logo_data, LOGO_FILE_SIZE);
    return true;
  }
  return false;
}

#endif // LOGO_STORAGE_H
