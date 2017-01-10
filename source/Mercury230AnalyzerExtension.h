#ifndef MERCURY230_ANALYZER_EXTENSION
#define MERCURY230_ANALYZER_EXTENSION

//Request Codes
#define REQCODE_CHECK_CHANNEL                   0x00
#define REQCODE_OPEN_CHANNEL                    0x01
#define REQCODE_CLOSE_CHANNEL                   0x02
#define REQCODE_WRITE_DATA                      0x03
#define REQCODE_READ_DATA_TIME                  0x04
#define REQCODE_READ_DATA_ENERGY				0x05
#define REQCODE_READ_DATA_PARAMS				0x08

//Flags used by AnalyzerResults
#define FLAG_CHECKSUM_ERROR						0x80
#define FLAG_REQUEST_FRAME						0x40
#define FLAG_RESPONSE_FRAME						0x02
#define FLAG_EXCEPTION_FRAME					0x04
#define FLAG_DATA_FRAME							0x08
#define FLAG_END_FRAME							0x01
#define FLAG_FILE_SUBREQ						0x20

/* Read request time params */
#define READ_PARAM_TIME_CURENT_TIME				0x00
#define READ_PARAM_TIME_ON_OFF					0x01

#define READ_PARAM_TIME_ON_OFF_PHASE1			0x03
#define READ_PARAM_TIME_ON_OFF_PHASE2			0x04
#define READ_PARAM_TIME_ON_OFF_PHASE3			0x05

/* Read request energy params */
// #define READ_PARAM_ENERGY_FROM_RESET			0x00
// #define READ_PARAM_ENERGY_CUR_YEAR			0x01
// #define READ_PARAM_ENERGY_LAST_YEAR			0x02
// #define READ_PARAM_ENERGY_CUR_MONTH			0x03
// #define READ_PARAM_ENERGY_CUR_DAY			0x04
// #define READ_PARAM_ENERGY_LAST_DAY			0x05
// #define READ_PARAM_ENERGY_CUR_YEAR_BEGIN		0x09

//Sub-request codes
#define READ_PARAM_PARAM_SN						0x00
#define READ_PARAM_PARAM_FAST_READ				0x01
#define READ_PARAM_PARAM_T_RATIO				0x02
#define READ_PARAM_PARAM_VERSION				0x03
#define READ_PARAM_PARAM_TIMEOUT				0x04
#define READ_PARAM_PARAM_ADDRESS				0x05
#define READ_PARAM_PARAM_DISPLAY_MODE			0x06
#define READ_PARAM_PARAM_SEASON_TIME			0x07
#define READ_PARAM_PARAM_OVERLIMIT				0x08
#define READ_PARAM_PARAM_FLAGS					0x09
#define READ_PARAM_PARAM_STATE					0x0A
#define READ_PARAM_PARAM_LOCATION				0x0B
#define READ_PARAM_PARAM_SCHEDULE_MAX_POWER		0x0C
#define READ_PARAM_PARAM_VALUE_MAX_POWER		0x0D
#define READ_PARAM_PARAM_EXTENED				0x11

enum ReadEnergyEnum {
	FromReset = 0x00, CurentYear = 0x01, LastYear = 0x02, CurMonth = 0x03, LastMonth = 0x04
};

#endif //MERCURY230_ANALYZER_EXTENSION