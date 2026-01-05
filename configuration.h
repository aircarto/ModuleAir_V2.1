// Allow online acces / not used yet

bool online_acces = false;

// Fill with values from 1 to 6 and 0 if no display

const int cfg_logo_moduleair = 1;
const int cfg_logo_aircarto = 2;
const int cfg_logo_atmo = 0;
const int cfg_logo_region = 0;
const int cfg_logo_custom1 = 3; //see logos-custom.h
const int cfg_logo_custom2 = 0; 

// Screen selection has been moved to the cfg namespace in moduleair.cpp
// These settings are now configurable via the web interface under the "Écrans" tab
// Default values are set in moduleair.cpp (lines 197-211)
// and can be modified through the web configuration page
