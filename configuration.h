// Allow online acces / not used yet

bool online_acces = false;

// Default values for logo display positions (1 to 6, 0 = hidden)
// These are now stored in config.json and preserved across OTA updates
// The values below are used as compile-time defaults for ext_def.h
#define CFG_LOGO_MODULEAIR 1
#define CFG_LOGO_AIRCARTO 2
#define CFG_LOGO_ATMO 0
#define CFG_LOGO_REGION 0
#define CFG_LOGO_CUSTOM1 0 // see logos-custom.h
#define CFG_LOGO_CUSTOM2 0

// Screen selection has been moved to the cfg namespace in moduleair.cpp
// These settings are now configurable via the web interface under the "Écrans"
// tab Default values are set in moduleair.cpp (lines 197-211) and can be
// modified through the web configuration page
