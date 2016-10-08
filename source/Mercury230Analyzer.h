#ifndef MERCURY230_ANALYZER_H
#define MERCURY230_ANALYZER_H

#include <Analyzer.h>
#include "Mercury230AnalyzerResults.h"
#include "Mercury230SimulationDataGenerator.h"
#include "Mercury230AnalyzerExtension.h"

#include <stdio.h>
#include <string.h>

class Mercury230AnalyzerSettings;
class ANALYZER_EXPORT Mercury230Analyzer : public Analyzer2
{
public:
	Mercury230Analyzer();
	virtual ~Mercury230Analyzer();
	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'Mercury230Analyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class

protected: //functions
	void ComputeSampleOffsets();
	U64 GetNextByteMercury230(U32 num_bits, U64 bit_mask, U64 &frame_starting_sample, U64 &frame_ending_sample);

protected: //vars
	std::auto_ptr< Mercury230AnalyzerSettings > mSettings;
	std::auto_ptr< Mercury230AnalyzerResults > mResults;
	AnalyzerChannelData* mMercury230;

	Mercury230SimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Mercury230 analysis vars:
	U32 mSampleRateHz;
	std::vector<U32> mSampleOffsets;
	U32 mParityBitOffset;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
	BitState mBitLow;
	BitState mBitHigh;

	//Checksum caluclations for Mercury230
	U16   crc_tab16[256];
	void init_crc16_tab( void );
	U16 update_CRC( U16 crc, U8 c );

#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //MERCURY230_ANALYZER_H
