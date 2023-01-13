

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

enum ConfigShapeId {
	Config_has_wifi,
	Config_has_lora,
	Config_appeui,
	Config_deveui,
	Config_appkey,
	Config_current_lang,
	Config_wlanssid,
	Config_wlanpwd,
	Config_www_username,
	Config_www_password,
	Config_fs_ssid,
	Config_fs_pwd,
	Config_www_basicauth_enabled,
	Config_sds_read,
	Config_npm_read,
	Config_bmx280_read,
	Config_height_above_sealevel,
	Config_mhz16_read,
	Config_mhz19_read,
	Config_ccs811_read,
	Config_latitude,
	Config_longitude,
	Config_send2dusti,
	Config_ssl_dusti,
	Config_send2madavi,
	Config_ssl_madavi,
	Config_send2csv,
	Config_has_ssd1306,
	Config_has_matrix,
	Config_display_measure,
	Config_display_forecast,
	Config_display_wifi_info,
	Config_display_lora_info,
	Config_display_device_info,
	Config_debug,
	Config_sending_intervall_ms,
	Config_time_for_wifi_config,
	Config_send2custom,
	Config_host_custom,
	Config_url_custom,
	Config_port_custom,
	Config_user_custom,
	Config_pwd_custom,
	Config_ssl_custom,
	Config_send2custom2,
	Config_host_custom2,
	Config_url_custom2,
	Config_port_custom2,
	Config_user_custom2,
	Config_pwd_custom2,
	Config_ssl_custom2,
};
static constexpr char CFG_KEY_HAS_WIFI[] PROGMEM = "has_wifi";
static constexpr char CFG_KEY_HAS_LORA[] PROGMEM = "has_lora";
static constexpr char CFG_KEY_APPEUI[] PROGMEM = "appeui";
static constexpr char CFG_KEY_DEVEUI[] PROGMEM = "deveui";
static constexpr char CFG_KEY_APPKEY[] PROGMEM = "appkey";
static constexpr char CFG_KEY_CURRENT_LANG[] PROGMEM = "current_lang";
static constexpr char CFG_KEY_WLANSSID[] PROGMEM = "wlanssid";
static constexpr char CFG_KEY_WLANPWD[] PROGMEM = "wlanpwd";
static constexpr char CFG_KEY_WWW_USERNAME[] PROGMEM = "www_username";
static constexpr char CFG_KEY_WWW_PASSWORD[] PROGMEM = "www_password";
static constexpr char CFG_KEY_FS_SSID[] PROGMEM = "fs_ssid";
static constexpr char CFG_KEY_FS_PWD[] PROGMEM = "fs_pwd";
static constexpr char CFG_KEY_WWW_BASICAUTH_ENABLED[] PROGMEM = "www_basicauth_enabled";
static constexpr char CFG_KEY_SDS_READ[] PROGMEM = "sds_read";
static constexpr char CFG_KEY_NPM_READ[] PROGMEM = "npm_read";
static constexpr char CFG_KEY_BMX280_READ[] PROGMEM = "bmx280_read";
static constexpr char CFG_KEY_HEIGHT_ABOVE_SEALEVEL[] PROGMEM = "height_above_sealevel";
static constexpr char CFG_KEY_MHZ16_READ[] PROGMEM = "mhz16_read";
static constexpr char CFG_KEY_MHZ19_READ[] PROGMEM = "mhz19_read";
static constexpr char CFG_KEY_CCS811_READ[] PROGMEM = "ccs811_read";
static constexpr char CFG_KEY_LATITUDE[] PROGMEM = "latitude";
static constexpr char CFG_KEY_LONGITUDE[] PROGMEM = "longitude";
static constexpr char CFG_KEY_SEND2DUSTI[] PROGMEM = "send2dusti";
static constexpr char CFG_KEY_SSL_DUSTI[] PROGMEM = "ssl_dusti";
static constexpr char CFG_KEY_SEND2MADAVI[] PROGMEM = "send2madavi";
static constexpr char CFG_KEY_SSL_MADAVI[] PROGMEM = "ssl_madavi";
static constexpr char CFG_KEY_SEND2CSV[] PROGMEM = "send2csv";
static constexpr char CFG_KEY_HAS_SSD1306[] PROGMEM = "has_ssd1306";
static constexpr char CFG_KEY_HAS_MATRIX[] PROGMEM = "has_matrix";
static constexpr char CFG_KEY_DISPLAY_MEASURE[] PROGMEM = "display_measure";
static constexpr char CFG_KEY_DISPLAY_FORECAST[] PROGMEM = "display_forecast";
static constexpr char CFG_KEY_DISPLAY_WIFI_INFO[] PROGMEM = "display_wifi_info";
static constexpr char CFG_KEY_DISPLAY_LORA_INFO[] PROGMEM = "display_lora_info";
static constexpr char CFG_KEY_DISPLAY_DEVICE_INFO[] PROGMEM = "display_device_info";
static constexpr char CFG_KEY_DEBUG[] PROGMEM = "debug";
static constexpr char CFG_KEY_SENDING_INTERVALL_MS[] PROGMEM = "sending_intervall_ms";
static constexpr char CFG_KEY_TIME_FOR_WIFI_CONFIG[] PROGMEM = "time_for_wifi_config";
static constexpr char CFG_KEY_SEND2CUSTOM[] PROGMEM = "send2custom";
static constexpr char CFG_KEY_HOST_CUSTOM[] PROGMEM = "host_custom";
static constexpr char CFG_KEY_URL_CUSTOM[] PROGMEM = "url_custom";
static constexpr char CFG_KEY_PORT_CUSTOM[] PROGMEM = "port_custom";
static constexpr char CFG_KEY_USER_CUSTOM[] PROGMEM = "user_custom";
static constexpr char CFG_KEY_PWD_CUSTOM[] PROGMEM = "pwd_custom";
static constexpr char CFG_KEY_SSL_CUSTOM[] PROGMEM = "ssl_custom";
static constexpr char CFG_KEY_SEND2CUSTOM2[] PROGMEM = "send2custom2";
static constexpr char CFG_KEY_HOST_CUSTOM2[] PROGMEM = "host_custom2";
static constexpr char CFG_KEY_URL_CUSTOM2[] PROGMEM = "url_custom2";
static constexpr char CFG_KEY_PORT_CUSTOM2[] PROGMEM = "port_custom2";
static constexpr char CFG_KEY_USER_CUSTOM2[] PROGMEM = "user_custom2";
static constexpr char CFG_KEY_PWD_CUSTOM2[] PROGMEM = "pwd_custom2";
static constexpr char CFG_KEY_SSL_CUSTOM2[] PROGMEM = "ssl_custom2";
static constexpr ConfigShapeEntry configShape[] PROGMEM = {
	{ Config_Type_Bool, 0, CFG_KEY_HAS_WIFI, &cfg::has_wifi },
	{ Config_Type_Bool, 0, CFG_KEY_HAS_LORA, &cfg::has_lora },
	{ Config_Type_Hex, sizeof(cfg::appeui)-1, CFG_KEY_APPEUI, cfg::appeui },
	{ Config_Type_Hex, sizeof(cfg::deveui)-1, CFG_KEY_DEVEUI, cfg::deveui },
	{ Config_Type_Hex, sizeof(cfg::appkey)-1, CFG_KEY_APPKEY, cfg::appkey },
	{ Config_Type_String, sizeof(cfg::current_lang)-1, CFG_KEY_CURRENT_LANG, cfg::current_lang },
	{ Config_Type_String, sizeof(cfg::wlanssid)-1, CFG_KEY_WLANSSID, cfg::wlanssid },
	{ Config_Type_Password, sizeof(cfg::wlanpwd)-1, CFG_KEY_WLANPWD, cfg::wlanpwd },
	{ Config_Type_String, sizeof(cfg::www_username)-1, CFG_KEY_WWW_USERNAME, cfg::www_username },
	{ Config_Type_Password, sizeof(cfg::www_password)-1, CFG_KEY_WWW_PASSWORD, cfg::www_password },
	{ Config_Type_String, sizeof(cfg::fs_ssid)-1, CFG_KEY_FS_SSID, cfg::fs_ssid },
	{ Config_Type_Password, sizeof(cfg::fs_pwd)-1, CFG_KEY_FS_PWD, cfg::fs_pwd },
	{ Config_Type_Bool, 0, CFG_KEY_WWW_BASICAUTH_ENABLED, &cfg::www_basicauth_enabled },
	{ Config_Type_Bool, 0, CFG_KEY_SDS_READ, &cfg::sds_read },
	{ Config_Type_Bool, 0, CFG_KEY_NPM_READ, &cfg::npm_read },
	{ Config_Type_Bool, 0, CFG_KEY_BMX280_READ, &cfg::bmx280_read },
	{ Config_Type_String, sizeof(cfg::height_above_sealevel)-1, CFG_KEY_HEIGHT_ABOVE_SEALEVEL, cfg::height_above_sealevel },
	{ Config_Type_Bool, 0, CFG_KEY_MHZ16_READ, &cfg::mhz16_read },
	{ Config_Type_Bool, 0, CFG_KEY_MHZ19_READ, &cfg::mhz19_read },
	{ Config_Type_Bool, 0, CFG_KEY_CCS811_READ, &cfg::ccs811_read },
	{ Config_Type_String, sizeof(cfg::latitude)-1, CFG_KEY_LATITUDE, cfg::latitude },
	{ Config_Type_String, sizeof(cfg::longitude)-1, CFG_KEY_LONGITUDE, cfg::longitude },
	{ Config_Type_Bool, 0, CFG_KEY_SEND2DUSTI, &cfg::send2dusti },
	{ Config_Type_Bool, 0, CFG_KEY_SSL_DUSTI, &cfg::ssl_dusti },
	{ Config_Type_Bool, 0, CFG_KEY_SEND2MADAVI, &cfg::send2madavi },
	{ Config_Type_Bool, 0, CFG_KEY_SSL_MADAVI, &cfg::ssl_madavi },
	{ Config_Type_Bool, 0, CFG_KEY_SEND2CSV, &cfg::send2csv },
	{ Config_Type_Bool, 0, CFG_KEY_HAS_SSD1306, &cfg::has_ssd1306 },
	{ Config_Type_Bool, 0, CFG_KEY_HAS_MATRIX, &cfg::has_matrix },
	{ Config_Type_Bool, 0, CFG_KEY_DISPLAY_MEASURE, &cfg::display_measure },
	{ Config_Type_Bool, 0, CFG_KEY_DISPLAY_FORECAST, &cfg::display_forecast },
	{ Config_Type_Bool, 0, CFG_KEY_DISPLAY_WIFI_INFO, &cfg::display_wifi_info },
	{ Config_Type_Bool, 0, CFG_KEY_DISPLAY_LORA_INFO, &cfg::display_lora_info },
	{ Config_Type_Bool, 0, CFG_KEY_DISPLAY_DEVICE_INFO, &cfg::display_device_info },
	{ Config_Type_UInt, 0, CFG_KEY_DEBUG, &cfg::debug },
	{ Config_Type_Time, 0, CFG_KEY_SENDING_INTERVALL_MS, &cfg::sending_intervall_ms },
	{ Config_Type_Time, 0, CFG_KEY_TIME_FOR_WIFI_CONFIG, &cfg::time_for_wifi_config },
	{ Config_Type_Bool, 0, CFG_KEY_SEND2CUSTOM, &cfg::send2custom },
	{ Config_Type_String, sizeof(cfg::host_custom)-1, CFG_KEY_HOST_CUSTOM, cfg::host_custom },
	{ Config_Type_String, sizeof(cfg::url_custom)-1, CFG_KEY_URL_CUSTOM, cfg::url_custom },
	{ Config_Type_UInt, 0, CFG_KEY_PORT_CUSTOM, &cfg::port_custom },
	{ Config_Type_String, sizeof(cfg::user_custom)-1, CFG_KEY_USER_CUSTOM, cfg::user_custom },
	{ Config_Type_Password, sizeof(cfg::pwd_custom)-1, CFG_KEY_PWD_CUSTOM, cfg::pwd_custom },
	{ Config_Type_Bool, 0, CFG_KEY_SSL_CUSTOM, &cfg::ssl_custom },
	{ Config_Type_Bool, 0, CFG_KEY_SEND2CUSTOM2, &cfg::send2custom2 },
	{ Config_Type_String, sizeof(cfg::host_custom2)-1, CFG_KEY_HOST_CUSTOM2, cfg::host_custom2 },
	{ Config_Type_String, sizeof(cfg::url_custom2)-1, CFG_KEY_URL_CUSTOM2, cfg::url_custom2 },
	{ Config_Type_UInt, 0, CFG_KEY_PORT_CUSTOM2, &cfg::port_custom2 },
	{ Config_Type_String, sizeof(cfg::user_custom2)-1, CFG_KEY_USER_CUSTOM2, cfg::user_custom2 },
	{ Config_Type_Password, sizeof(cfg::pwd_custom2)-1, CFG_KEY_PWD_CUSTOM2, cfg::pwd_custom2 },
	{ Config_Type_Bool, 0, CFG_KEY_SSL_CUSTOM2, &cfg::ssl_custom2 },
};
