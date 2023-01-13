#!/usr/bin/env python3

configshape_in = """
Bool        has_wifi
Bool        has_lora
Hex		    appeui
Hex 		deveui
Hex 		appkey
String		current_lang
String      wlanssid
Password    wlanpwd
String		www_username
Password	www_password
String		fs_ssid
Password	fs_pwd
Bool		www_basicauth_enabled
Bool		sds_read
Bool		npm_read
Bool		bmx280_read
String		height_above_sealevel
Bool		mhz16_read
Bool		mhz19_read
Bool		ccs811_read
String		latitude
String		longitude
Bool		send2dusti
Bool		ssl_dusti
Bool		send2madavi
Bool		ssl_madavi
Bool		send2csv
Bool		has_ssd1306
Bool		has_matrix
Bool		display_measure
Bool		display_forecast
Bool		display_wifi_info
Bool		display_lora_info
Bool		display_device_info
UInt		debug
Time		sending_intervall_ms
Time		time_for_wifi_config
Bool		send2custom
String		host_custom
String		url_custom
UInt		port_custom
String		user_custom
Password	pwd_custom
Bool		ssl_custom
Bool		send2custom2
String		host_custom2
String		url_custom2
UInt		port_custom2
String		user_custom2
Password	pwd_custom2
Bool		ssl_custom2
"""

with open("moduleair-cfg.h", "w") as h:
    print("""

// This file is generated, please do not edit.
// Change moduleair-cfg.h.py instead.

enum ConfigEntryType : unsigned short {
	Config_Type_Bool,
	Config_Type_UInt,
	Config_Type_Time,
	Config_Type_String,
	Config_Type_Password,
    Config_Type_Hex
};

struct ConfigShapeEntry {
	enum ConfigEntryType cfg_type;
	unsigned short cfg_len;
	const char* _cfg_key;
	union {
		void* as_void;
		bool* as_bool;
		unsigned int* as_uint;
		char* as_str;
	} cfg_val;
	const __FlashStringHelper* cfg_key() const { return FPSTR(_cfg_key); }
};

enum ConfigShapeId {""", file=h)

    for cfgentry in configshape_in.strip().split('\n'):
        print("\tConfig_", cfgentry.split()[1], ",", sep='', file=h)
    print("};", file=h)

    for cfgentry in configshape_in.strip().split('\n'):
        _, cfgkey = cfgentry.split()
        print("static constexpr char CFG_KEY_", cfgkey.upper(),
              "[] PROGMEM = \"", cfgkey, "\";", sep='', file=h)

    print("static constexpr ConfigShapeEntry configShape[] PROGMEM = {",
          file=h)
    for cfgentry in configshape_in.strip().split('\n'):
        cfgtype, cfgkey = cfgentry.split()
        print("\t{ Config_Type_", cfgtype,
              ", sizeof(cfg::" + cfgkey + ")-1" if cfgtype in ('String', 'Password', 'Hex') else ", 0",
              ", CFG_KEY_", cfgkey.upper(),
              ", ", "" if cfgtype in ('String', 'Password', 'Hex') else "&",
              "cfg::", cfgkey, " },", sep='', file=h)
    print("};", file=h)
