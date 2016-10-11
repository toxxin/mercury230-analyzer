#include "Mercury230Analyzer.h"
#include "Mercury230AnalyzerSettings.h"
#include <AnalyzerChannelData.h>


Mercury230Analyzer::Mercury230Analyzer()
:	Analyzer2(),
	mSettings( new Mercury230AnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );

	//used for calculating Mercury230 Checksum values
	init_crc16_tab();
}

Mercury230Analyzer::~Mercury230Analyzer()
{
	KillThread();
}

void Mercury230Analyzer::ComputeSampleOffsets()
{
	ClockGenerator clock_generator;
	clock_generator.Init( mSettings->mBitRate, mSampleRateHz );

	mSampleOffsets.clear();

	U32 num_bits = mSettings->mBitsPerTransfer;

	mSampleOffsets.push_back( clock_generator.AdvanceByHalfPeriod( 1.5 ) );  //point to the center of the 1st bit (past the start bit)
	num_bits--;  //we just added the first bit.

	for( U32 i=0; i<num_bits; i++ )
	{
		mSampleOffsets.push_back( clock_generator.AdvanceByHalfPeriod() );
	}

	if( mSettings->mParity != AnalyzerEnums::None )
		mParityBitOffset = clock_generator.AdvanceByHalfPeriod();

	//to check for framing errors, we also want to check
	//1/2 bit after the beginning of the stop bit
	mStartOfStopBitOffset = clock_generator.AdvanceByHalfPeriod( 1.0 );  //i.e. moving from the center of the last data bit (where we left off) to 1/2 period into the stop bit

	//and 1/2 bit before end of the stop bit period
	mEndOfStopBitOffset = clock_generator.AdvanceByHalfPeriod( mSettings->mStopBits - 1.0 );  //if stopbits == 1.0, this will be 0
}


void Mercury230Analyzer::SetupResults()
{
	//Unlike the worker thread, this function is called from the GUI thread
	//we need to reset the Results object here because it is exposed for direct access by the GUI, and it can't be deleted from the WorkerThread

	mResults.reset( new Mercury230AnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void Mercury230Analyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();
	ComputeSampleOffsets();
	U32 num_bits = mSettings->mBitsPerTransfer;

	if( mSettings->mInverted == false )
	{
		mBitHigh = BIT_HIGH;
		mBitLow = BIT_LOW;
	}else
	{
		mBitHigh = BIT_LOW;
		mBitLow = BIT_HIGH;
	}

	U64 bit_mask = 0;
	U64 mask = 0x1ULL;
	for( U32 i=0; i<num_bits; i++ )
	{
		bit_mask |= mask;
		mask <<= 1;
	}

	mMercury230 = GetAnalyzerChannelData( mSettings->mInputChannel );
	mMercury230->TrackMinimumPulseWidth();

	if( mMercury230->GetBitState() == mBitLow )
		mMercury230->AdvanceToNextEdge();

	for( ; ; )
	{
		Frame frame;
		U64 starting_frame;
		U64 ending_frame;
		U64 Payload1[2];
		U64 Payload2[2];
		U64 Payload3[2];
		U64 Payload4[2];
		U64 RecChecksum[2];
		U64 ByteCount[2];
		U64 Checksum;
		U64 param;
		U64 access;

		//the frame begins here with the Device Address
		U64 devaddr = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
		frame.mStartingSampleInclusive = starting_frame;

		//Then comes the Function Code
		U64 funccode = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

		//Now we'll process the rest of the data based on whether the transmission is coming from the master or a slave device
		if (mSettings->mMercury230Mode==Mercury230AnalyzerEnums::Mercury230Master)
		{
			frame.mFlags = FLAG_REQUEST_FRAME;

			//It's a master device doing the talking
			switch(funccode)
			{
				case REQCODE_CHECK_CHANNEL:
					RecChecksum[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					RecChecksum[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

					Checksum = 0xFFFF; //Mercury230/RTU uses CRC-16, calls for initialization to 0xFFFF
					Checksum = update_CRC( Checksum, devaddr );
					Checksum = update_CRC( Checksum, funccode );

					if((((Checksum&0xFF00)>>8)!=RecChecksum[1]) || ((Checksum&0x00FF)!=RecChecksum[0]))
					{
						frame.mFlags = frame.mFlags | FLAG_CHECKSUM_ERROR;	//error!
					}

					frame.mData1 = (devaddr << 56) + (funccode << 48) + (RecChecksum[1] << 8) + RecChecksum[0];

					break;

				case REQCODE_OPEN_CHANNEL:
					access = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					Payload1[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					Payload1[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					Payload2[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					Payload2[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					Payload3[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					Payload3[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

					RecChecksum[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					RecChecksum[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

					Checksum = 0xFFFF; //Mercury230/RTU uses CRC-16, calls for initialization to 0xFFFF
					Checksum = update_CRC( Checksum, devaddr );
					Checksum = update_CRC( Checksum, funccode );
					Checksum = update_CRC(Checksum, access);
					Checksum = update_CRC(Checksum, 0x31);
					Checksum = update_CRC(Checksum, 0x32);
					Checksum = update_CRC(Checksum, 0x33);
					Checksum = update_CRC(Checksum, 0x34);
					Checksum = update_CRC(Checksum, 0x35);
					Checksum = update_CRC(Checksum, 0x36);

					if((((Checksum&0xFF00)>>8)!=RecChecksum[1]) || ((Checksum&0x00FF)!=RecChecksum[0]))
					{
						frame.mFlags = frame.mFlags | FLAG_CHECKSUM_ERROR;	//error!
					}

					frame.mData1 = (devaddr << 56) + (funccode << 48) + (access << 40) + (RecChecksum[1] << 8) + RecChecksum[0];
					frame.mData2 = (Payload1[0] << 56) + (Payload1[1] << 48) + (Payload2[0] << 40) + (Payload2[1] << 32) + (Payload3[0] << 24) + (Payload3[1] << 16);

					break;

				case REQCODE_CLOSE_CHANNEL:
					RecChecksum[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					RecChecksum[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

					Checksum = 0xFFFF; //Mercury230/RTU uses CRC-16, calls for initialization to 0xFFFF
					Checksum = update_CRC( Checksum, devaddr );
					Checksum = update_CRC( Checksum, funccode );

					if((((Checksum&0xFF00)>>8)!=RecChecksum[1]) || ((Checksum&0x00FF)!=RecChecksum[0]))
					{
						frame.mFlags = frame.mFlags | FLAG_CHECKSUM_ERROR;	//error!
					}

					frame.mData1 = (devaddr << 56) + (funccode << 48) + (RecChecksum[1] << 8) + RecChecksum[0];

					break;

				case REQCODE_READ_DATA_TIME:
					param = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					switch(param) {
						case READ_PARAM_TIME_CURENT_TIME:
							RecChecksum[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
							RecChecksum[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

							Checksum = 0xFFFF; //Mercury230/RTU uses CRC-16, calls for initialization to 0xFFFF
							Checksum = update_CRC( Checksum, devaddr );
							Checksum = update_CRC( Checksum, funccode );
							Checksum = update_CRC( Checksum, param );

							if((((Checksum&0xFF00)>>8)!=RecChecksum[1]) || ((Checksum&0x00FF)!=RecChecksum[0]))
							{
								frame.mFlags = frame.mFlags | FLAG_CHECKSUM_ERROR;	//error!
							}

							frame.mData1 = (devaddr << 56) + (funccode << 48) + (param << 40) + (RecChecksum[1] << 8) + RecChecksum[0];

							break;

						default:
							break;
					}
					break;

				case REQCODE_READ_DATA_ENERGY:
					Payload1[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					Payload1[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

					RecChecksum[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					RecChecksum[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

					Checksum = 0xFFFF; //Mercury230/RTU uses CRC-16, calls for initialization to 0xFFFF
					Checksum = update_CRC( Checksum, devaddr );
					Checksum = update_CRC( Checksum, funccode );
					Checksum = update_CRC( Checksum, Payload1[0] );
					Checksum = update_CRC( Checksum, Payload1[1] );

					if((((Checksum&0xFF00)>>8)!=RecChecksum[1]) || ((Checksum&0x00FF)!=RecChecksum[0]))
					{
						frame.mFlags = frame.mFlags | FLAG_CHECKSUM_ERROR;	//error!
					}

					frame.mData1 = (devaddr << 56) + (funccode << 48) + (Payload1[0] << 40) + (Payload1[1] << 32) + (RecChecksum[1] << 8) + RecChecksum[0];
					break;

				case REQCODE_READ_DATA_PARAMS:
					param = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
					switch(param) {
						case READ_PARAM_PARAM_SN:
						case READ_PARAM_PARAM_T_RATIO:
						case READ_PARAM_PARAM_VERSION:
						case READ_PARAM_PARAM_TIMEOUT:
						case READ_PARAM_PARAM_ADDRESS:
						case READ_PARAM_PARAM_SEASON_TIME:
						case READ_PARAM_PARAM_OVERLIMIT:
						case READ_PARAM_PARAM_FLAGS:
						case READ_PARAM_PARAM_STATE:
						case READ_PARAM_PARAM_LOCATION:
						case READ_PARAM_PARAM_VALUE_MAX_POWER:

							RecChecksum[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
							RecChecksum[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

							Checksum = 0xFFFF; //Mercury230/RTU uses CRC-16, calls for initialization to 0xFFFF
							Checksum = update_CRC( Checksum, devaddr );
							Checksum = update_CRC( Checksum, funccode );
							Checksum = update_CRC( Checksum, param );

							if((((Checksum&0xFF00)>>8)!=RecChecksum[1]) || ((Checksum&0x00FF)!=RecChecksum[0]))
							{
								frame.mFlags = frame.mFlags | FLAG_CHECKSUM_ERROR;	//error!
							}

							frame.mData1 = (devaddr << 56) + (funccode << 48) + (param << 40) + (RecChecksum[1] << 8) + RecChecksum[0];
							break;

						case READ_PARAM_PARAM_FAST_READ:
						case READ_PARAM_PARAM_DISPLAY_MODE:
						case READ_PARAM_PARAM_SCHEDULE_MAX_POWER:

							RecChecksum[0] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);
							RecChecksum[1] = GetNextByteMercury230(num_bits, bit_mask, starting_frame, ending_frame);

							Checksum = 0xFFFF; //Mercury230/RTU uses CRC-16, calls for initialization to 0xFFFF
							Checksum = update_CRC( Checksum, devaddr );
							Checksum = update_CRC( Checksum, funccode );
							Checksum = update_CRC( Checksum, param );

							if((((Checksum&0xFF00)>>8)!=RecChecksum[1]) || ((Checksum&0x00FF)!=RecChecksum[0]))
							{
								frame.mFlags = frame.mFlags | FLAG_CHECKSUM_ERROR;	//error!
							}

							frame.mData1 = (devaddr << 56) + (funccode << 48) + (param << 40) + (RecChecksum[1] << 8) + RecChecksum[0];
							break;
					}
					break;

				//use fall through to process similar requests with the same code
			default:
				break;
			}
		}
		else
		{
			//slave mode
		}

		//the frame ends here
		frame.mEndingSampleInclusive = ending_frame;
		mResults->AddFrame( frame );
		mResults->CommitResults();

		ReportProgress( frame.mEndingSampleInclusive );
		CheckIfThreadShouldExit();
	}
}

bool Mercury230Analyzer::NeedsRerun()
{
	if( mSettings->mUseAutobaud == false )
		return false;

	//ok, lets see if we should change the bit rate, base on mShortestActivePulse

	U64 shortest_pulse = mMercury230->GetMinimumPulseWidthSoFar();

	if( shortest_pulse == 0 )
		AnalyzerHelpers::Assert( "Alg problem, shortest_pulse was 0" );

	U32 computed_bit_rate = U32( double( mSampleRateHz ) / double( shortest_pulse ) );

	if( computed_bit_rate > mSampleRateHz )
		AnalyzerHelpers::Assert( "Alg problem, computed_bit_rate is higer than sample rate" );  //just checking the obvious...

	if( computed_bit_rate > (mSampleRateHz / 4) )
		return false; //the baud rate is too fast.
	if( computed_bit_rate == 0 )
	{
		//bad result, this is not good data, don't bother to re-run.
		return false;
	}

	U32 specified_bit_rate = mSettings->mBitRate;

	double error = double( AnalyzerHelpers::Diff32( computed_bit_rate, specified_bit_rate ) ) / double( specified_bit_rate );

	if( error > 0.1 )
	{
		mSettings->mBitRate = computed_bit_rate;
		mSettings->UpdateInterfacesFromSettings();
		return true;
	}else
	{
		return false;
	}
}

U32 Mercury230Analyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 Mercury230Analyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4;
}

const char* Mercury230Analyzer::GetAnalyzerName() const
{
	return "Mercury230";
}

const char* GetAnalyzerName()
{
	return "Mercury230";
}

Analyzer* CreateAnalyzer()
{
	return new Mercury230Analyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}

U64 Mercury230Analyzer::GetNextByteMercury230(U32 num_bits, U64 bit_mask, U64 &frame_starting_sample, U64 &frame_ending_sample)
{
	mMercury230->AdvanceToNextEdge();

	//we're now at the beginning of the start bit.  We can start collecting the data.
	frame_starting_sample = mMercury230->GetSampleNumber();

	U64 data = 0;
	bool parity_error = false;
	bool framing_error = false;
	bool mp_is_address = false;

	DataBuilder data_builder;
	data_builder.Reset( &data, mSettings->mShiftOrder, num_bits );
	U64 marker_location = frame_starting_sample;

	for( U32 i=0; i<num_bits; i++ )
	{
		mMercury230->Advance( mSampleOffsets[i] );
		data_builder.AddBit( mMercury230->GetBitState() );

		marker_location += mSampleOffsets[i];
		mResults->AddMarker( marker_location, AnalyzerResults::Dot, mSettings->mInputChannel );
	}

	if( mSettings->mInverted == true )
		data = (~data) & bit_mask;

	parity_error = false;

	if( mSettings->mParity != AnalyzerEnums::None )
	{
		mMercury230->Advance( mParityBitOffset );
		bool is_even = AnalyzerHelpers::IsEven( AnalyzerHelpers::GetOnesCount( data ) );

		if( mSettings->mParity == AnalyzerEnums::Even )
		{
			if( is_even == true )
			{
				if( mMercury230->GetBitState() != mBitLow ) //we expect a low bit, to keep the parity even.
					parity_error = true;
			}else
			{
				if( mMercury230->GetBitState() != mBitHigh ) //we expect a high bit, to force parity even.
					parity_error = true;
			}
		}else  //if( mSettings->mParity == AnalyzerEnums::Odd )
		{
			if( is_even == false )
			{
				if( mMercury230->GetBitState() != mBitLow ) //we expect a low bit, to keep the parity odd.
					parity_error = true;
			}else
			{
				if( mMercury230->GetBitState() != mBitHigh ) //we expect a high bit, to force parity odd.
					parity_error = true;
			}
		}

		marker_location += mParityBitOffset;
		mResults->AddMarker( marker_location, AnalyzerResults::Square, mSettings->mInputChannel );
	}

	mMercury230->Advance( mStartOfStopBitOffset );

	frame_ending_sample = mMercury230->GetSampleNumber();

	return data;
}


U16 Mercury230Analyzer::update_CRC( U16 crc, U8 c )
{
    U16 tmp, short_c;

    short_c = 0x00ff & (U16) c;

    //if ( ! crc_tab16_init ) init_crc16_tab();

    tmp =  crc       ^ short_c;
    crc = (crc >> 8) ^ crc_tab16[ tmp & 0xff ];

    return crc;
}

void Mercury230Analyzer::init_crc16_tab( void )
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
