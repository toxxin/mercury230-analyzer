#include "Mercury230SimulationDataGenerator.h"
#include "Mercury230AnalyzerSettings.h"
#include "Mercury230AnalyzerExtension.h"

Mercury230SimulationDataGenerator::Mercury230SimulationDataGenerator()
{
}

Mercury230SimulationDataGenerator::~Mercury230SimulationDataGenerator()
{
}

void Mercury230SimulationDataGenerator::Initialize( U32 simulation_sample_rate, Mercury230AnalyzerSettings* settings )
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	mClockGenerator.Init( mSettings->mBitRate, simulation_sample_rate );
	mMercury230SimulationData.SetChannel( mSettings->mInputChannel );
	mMercury230SimulationData.SetSampleRate( simulation_sample_rate );

	if( mSettings->mInverted == false )
	{
		mBitLow = BIT_LOW;
		mBitHigh = BIT_HIGH;
	}
	else
	{
		mBitLow = BIT_HIGH;
		mBitHigh = BIT_LOW;
	}

	mMercury230SimulationData.SetInitialBitState( mBitHigh );
	mMercury230SimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( 10.0 ) );  //insert 10 bit-periods of idle

	mValue = 0;
	mNumBitsMask = 0;

	U32 num_bits = mSettings->mBitsPerTransfer;
	for( U32 i = 0; i < num_bits; i++ )
	{
		mNumBitsMask <<= 1;
		mNumBitsMask |= 0x1;
	}

	//used for calculating Mercury230 Checksum values
	init_crc16_tab();
}


U32 Mercury230SimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	while( mMercury230SimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
	{
			CheckChannel(0x01);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			OpenChannel(0x80);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			CloseChannel(0x80);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadCurrentTime(0x00);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadEnergy(0x80, CurMonth, 1, 7);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_SN);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			// extended params
			// ReadParamms(0x80, READ_PARAM_PARAM_FAST_READ);
			// mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_T_RATIO);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_VERSION);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_TIMEOUT);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_ADDRESS);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			// extended params
			// ReadParamms(0x80, READ_PARAM_PARAM_DISPLAY_MODE, NULL);
			// mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_SEASON_TIME);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_OVERLIMIT);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_FLAGS);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_STATE);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );

			ReadParamms(0x80, READ_PARAM_PARAM_LOCATION);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );
			
			// extended params
			// ReadParamms(0x80, READ_PARAM_PARAM_SCHEDULE_MAX_POWER);
			// mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );
			
			ReadParamms(0x80, READ_PARAM_PARAM_VALUE_MAX_POWER);
			mMercury230SimulationData.Advance( mClockGenerator.AdvanceByTimeS( .015) );
	}
	*simulation_channels = &mMercury230SimulationData;

	return 1;  // we are retuning the size of the SimulationChannelDescriptor array.  In our case, the "array" is length 1.
}


void Mercury230SimulationDataGenerator::CreateMercury230Byte( U64 value )
{
	//assume we start high
	mMercury230SimulationData.Transition();  //low-going edge for start bit
	mMercury230SimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod() );  //add start bit time

	if( mSettings->mInverted == true )
		value = ~value;

	U32 num_bits = mSettings->mBitsPerTransfer;

	BitExtractor bit_extractor( value, mSettings->mShiftOrder, num_bits );

	for( U32 i=0; i<num_bits; i++ )
	{
		mMercury230SimulationData.TransitionIfNeeded( bit_extractor.GetNextBit() );
		mMercury230SimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod() );
	}

	if( mSettings->mParity == AnalyzerEnums::Even )
	{
		if( AnalyzerHelpers::IsEven( AnalyzerHelpers::GetOnesCount( value ) ) == true )
			mMercury230SimulationData.TransitionIfNeeded( mBitLow ); //we want to add a zero bit
		else
			mMercury230SimulationData.TransitionIfNeeded( mBitHigh ); //we want to add a one bit

		mMercury230SimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod() );
	}else if( mSettings->mParity == AnalyzerEnums::Odd )
	{
		if( AnalyzerHelpers::IsOdd( AnalyzerHelpers::GetOnesCount( value ) ) == true )
			mMercury230SimulationData.TransitionIfNeeded( mBitLow ); //we want to add a zero bit
		else
			mMercury230SimulationData.TransitionIfNeeded( mBitHigh );

		mMercury230SimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod() );
	}

	mMercury230SimulationData.TransitionIfNeeded( mBitHigh ); //we need to end high

	//lets pad the end a bit for the stop bit:
	mMercury230SimulationData.Advance( mClockGenerator.AdvanceByHalfPeriod( mSettings->mStopBits ) );
}


void Mercury230SimulationDataGenerator::SendPacket(std::queue<U8> &q) {
	U16 CRCValue = 0xFFFF;
	while (!q.empty()) {
		CreateMercury230Byte(q.front());
		CRCValue = update_CRC(CRCValue, q.front());
		q.pop();
	}
	CreateMercury230Byte((CRCValue&0x00FF));
	CreateMercury230Byte(((CRCValue&0xFF00) >> 8));
}


void Mercury230SimulationDataGenerator::CheckChannel(U8 DeviceID) {
	std::queue<U8> q;
	q.push(DeviceID);
	q.push(REQCODE_CHECK_CHANNEL);
	SendPacket(q);
}


void Mercury230SimulationDataGenerator::OpenChannel(U8 DeviceID) {
	std::queue<U8> q;
	q.push(DeviceID);
	q.push(REQCODE_OPEN_CHANNEL);
	q.push(0x01); //access level 1
	// passwrod "123456"
	for (U8 i = 0x31; i <= 0x36; ++i)
		q.push(i);
	SendPacket(q);
}


void Mercury230SimulationDataGenerator::CloseChannel(U8 DeviceID) {
	std::queue<U8> q;
	q.push(DeviceID);
	q.push(REQCODE_CLOSE_CHANNEL);
	SendPacket(q);
}


void Mercury230SimulationDataGenerator::ReadCurrentTime(U8 DeviceID) {
	std::queue<U8> q;
	q.push(DeviceID);
	q.push(REQCODE_READ_DATA_TIME);
	q.push(READ_PARAM_TIME_CURENT_TIME);
	SendPacket(q);
}


void Mercury230SimulationDataGenerator::ReadOnOffTime(U8 DeviceID, U8 RecNumber) {
	std::queue<U8> q;
	q.push(DeviceID);
	q.push(REQCODE_READ_DATA_TIME);
	q.push(READ_PARAM_TIME_ON_OFF);
	q.push(RecNumber);
	SendPacket(q);
}


void Mercury230SimulationDataGenerator::ReadEnergy(U8 DeviceID, ReadEnergyEnum param, U8 MonthNumber, U8 TarifNumber) {
	std::queue<U8> q;
	q.push(DeviceID);
	q.push(REQCODE_READ_DATA_ENERGY);
	q.push((param << 4) | MonthNumber);
	q.push(TarifNumber);
	SendPacket(q);
}

void Mercury230SimulationDataGenerator::ReadParamms(U8 DeviceID, U8 param) {
	std::queue<U8> q;
	q.push(DeviceID);
	q.push(REQCODE_READ_DATA_PARAMS);
	q.push(param);
	SendPacket(q);
}

void Mercury230SimulationDataGenerator::ReadExtendedParams(U8 DeviceID, U8 BWRI) {
	std::queue<U8> q;
	q.push(DeviceID);
	q.push(REQCODE_READ_DATA_PARAMS);
	q.push(READ_PARAM_PARAM_EXTENED);
	q.push(BWRI);
}

U16 Mercury230SimulationDataGenerator::update_CRC( U16 crc, U8 c )
{

    U16 tmp, short_c;

    short_c = 0x00ff & (U16) c;

    //if ( ! crc_tab16_init ) init_crc16_tab();

    tmp =  crc       ^ short_c;
    crc = (crc >> 8) ^ crc_tab16[ tmp & 0xff ];

    return crc;

}

void Mercury230SimulationDataGenerator::init_crc16_tab( void )
{

    int i, j;
    U16 crc, c;

    for (i=0; i<256; i++) {

        crc = 0;
        c   = (U16) i;

        for (j=0; j<8; j++) {

            if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ 0xA001;
            else                      crc =   crc >> 1;

            c = c >> 1;
        }

        crc_tab16[i] = crc;
    }

}
