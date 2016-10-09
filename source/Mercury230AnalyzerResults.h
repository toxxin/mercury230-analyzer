#ifndef MERCURY230_ANALYZER_RESULTS
#define MERCURY230_ANALYZER_RESULTS

#include <AnalyzerResults.h>

#include <stdio.h>
#include <string.h>

#define FRAMING_ERROR_FLAG ( 1 << 0 )
#define PARITY_ERROR_FLAG ( 1 << 1 )
#define MP_MODE_ADDRESS_FLAG ( 1 << 2 )

class Mercury230Analyzer;
class Mercury230AnalyzerSettings;

class Mercury230AnalyzerResults : public AnalyzerResults
{
public:
	Mercury230AnalyzerResults( Mercury230Analyzer* analyzer, Mercury230AnalyzerSettings* settings );
	virtual ~Mercury230AnalyzerResults();

	virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
	virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base );
	virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
	virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

protected: //functions

protected:  //vars
	Mercury230AnalyzerSettings* mSettings;
	Mercury230Analyzer* mAnalyzer;
};

#endif //MERCURY230_ANALYZER_RESULTS
