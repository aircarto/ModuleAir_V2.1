#ifndef utils_h
#define utils_h

#include <WString.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HardwareSerial.h>
#include <esp32/sha.h>
#include <freertos/queue.h>

constexpr unsigned SMALL_STR = 64-1;
constexpr unsigned MED_STR = 256-1;
constexpr unsigned LARGE_STR = 512-1;
constexpr unsigned XLARGE_STR = 1024-1;

//Answers Next PM
const uint8_t constexpr answer_stop[4] = {0x81, 0x15, 0x01, 0x69};
const uint8_t constexpr answer_start[4] = {0x81, 0x15, 0x00, 0x6A};
const uint8_t constexpr answer_sleep[4] = {0x81, 0x16, 0x01, 0x68};

#define RESERVE_STRING(name, size) String name((const char*)nullptr); name.reserve(size)

#define UPDATE_MIN(MIN, SAMPLE) if (SAMPLE < MIN) { MIN = SAMPLE; }
#define UPDATE_MAX(MAX, SAMPLE) if (SAMPLE > MAX) { MAX = SAMPLE; }
#define UPDATE_MIN_MAX(MIN, MAX, SAMPLE) { UPDATE_MIN(MIN, SAMPLE); UPDATE_MAX(MAX, SAMPLE); }

extern String sha1Hex(const String& s);
extern String hmac1(const String& secret, const String& s);

extern String tmpl(const __FlashStringHelper* patt, const String& value);

extern void add_table_row_from_value(String& page_content, const __FlashStringHelper* sensor, const __FlashStringHelper* param, const String& value, const String& unit);
extern void add_table_row_from_value(String& page_content, const __FlashStringHelper* param, const String& value, const char* unit = nullptr);

extern int32_t calcWiFiSignalQuality(int32_t rssi);

extern String add_sensor_type(const String& sensor_text);
extern String wlan_ssid_to_table_row(const String& ssid, const String& encryption, int32_t rssi);
extern String delayToString(unsigned time_ms);

extern String check_display_value(double value, double undef, uint8_t len, uint8_t str_len);
extern void add_Value2Json(String& res, const __FlashStringHelper* type, const String& value);
extern void add_Value2Json(String& res, const __FlashStringHelper* type, const __FlashStringHelper* debug_type, const float& value);

extern float readCorrectionOffset(const char* correction);

namespace cfg {
	extern unsigned debug;
}


#define serialSDS (Serial1)
#define serialNPM (Serial1) 
#define serialMHZ (Serial2)

enum class PmSensorCmd {
	Start,
	Stop,
	ContinuousMode
};

enum class PmSensorCmd2 { // for NPM
	State,
	Change,
	Concentration,
	Version,
	Speed,
	Temphumi
};

/*****************************************************************
 * Debug output                                                  *
 *****************************************************************/

class LoggingSerial : public HardwareSerial {

public:
	LoggingSerial();
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;
	String popLines();

private:
	QueueHandle_t m_buffer;
};

extern class LoggingSerial Debug;

extern void debug_out(const String& text, unsigned int level);
extern void debug_out(const __FlashStringHelper* text, unsigned int level);
extern void debug_outln(const String& text, unsigned int level);
extern void debug_outln_info(const String& text);
extern void debug_outln_verbose(const String& text);
extern void debug_outln_error(const __FlashStringHelper* text);
extern void debug_outln_info(const __FlashStringHelper* text);
extern void debug_outln_verbose(const __FlashStringHelper* text);
extern void debug_outln_info(const __FlashStringHelper* text, const String& option);
extern void debug_outln_info(const __FlashStringHelper* text, float value);
extern void debug_outln_verbose(const __FlashStringHelper* text, const String& option);
extern void debug_outln_info_bool(const __FlashStringHelper* text, const bool option);


extern bool SDS_checksum_valid(const uint8_t (&data)[8]);
extern void SDS_rawcmd(const uint8_t cmd_head1, const uint8_t cmd_head2, const uint8_t cmd_head3);
extern bool SDS_cmd(PmSensorCmd cmd);
extern bool PMS_cmd(PmSensorCmd cmd);
extern bool HPM_cmd(PmSensorCmd cmd);
extern void NPM_cmd(PmSensorCmd2 cmd);
extern bool NPM_checksum_valid_4(const uint8_t (&data)[4]);
extern bool NPM_checksum_valid_5(const uint8_t (&data)[5]);
extern bool NPM_checksum_valid_6(const uint8_t (&data)[6]);
extern bool NPM_checksum_valid_8(const uint8_t (&data)[8]);
extern bool NPM_checksum_valid_16(const uint8_t (&data)[16]);
extern void NPM_data_reader(uint8_t data[], size_t size);
extern String NPM_state(uint8_t bytedata);

extern bool isNumeric(const String& str);

extern const __FlashStringHelper* loggerDescription(unsigned i);

#endif
