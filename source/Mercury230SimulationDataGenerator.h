#ifndef MERCURY230_SIMULATION_DATA_GENERATOR
#define MERCURY230_SIMULATION_DATA_GENERATOR

#include <AnalyzerHelpers.h>

#include <queue>
#include <stdio.h>
#include <string.h>

#include "Mercury230AnalyzerExtension.h"

class Mercury230AnalyzerSettings;

class Mercury230SimulationDataGenerator
{
public:
	Mercury230SimulationDataGenerator();
	~Mercury230SimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, Mercury230AnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );

protected:
	Mercury230AnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;
	BitState mBitLow;
	BitState mBitHigh;
	U64 mValue;

	//U64 mMpModeAddressMask;
	//U64 mMpModeDataMask;
	U64 mNumBitsMask;

protected: //Mercury230 specific

	void CreateMercury230Byte( U64 value );
	ClockGenerator mClockGenerator;
	SimulationChannelDescriptor mMercury230SimulationData;  //if we had more than one channel to simulate, they would need to be in an array

	//used for doing CRC calculations
	U16   crc_tab16[256];
	void init_crc16_tab( void );
	U16 update_CRC( U16 crc, U8 c );

	void SendPacket(std::queue<U8> &q);

	void CheckChannel(U8 DeviceID);
	void OpenChannel(U8 DeviceID);
	void CloseChannel(U8 DeviceID);
	void ReadCurrentTime(U8 DeviceID);
	void ReadOnOffTime(U8 DeviceID, U8 RecNumber);
	void ReadEnergy(U8 DeviceID, ReadEnergyEnum param, U8 MonthNumber, U8 TarifNumber);
	void ReadParamms(U8 DeviceID, U8 param);
	void ReadExtendedParams(U8 DeviceID, U8 BWRI);

};
#endif //MERCURY230_SIMULATION_DATA_GENERATOR
