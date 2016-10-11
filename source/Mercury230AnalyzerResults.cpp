#include "Mercury230AnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "Mercury230Analyzer.h"
#include "Mercury230AnalyzerSettings.h"
#include <iostream>
#include <sstream>


#pragma warning(disable: 4996) //warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.

Mercury230AnalyzerResults::Mercury230AnalyzerResults( Mercury230Analyzer* analyzer, Mercury230AnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

Mercury230AnalyzerResults::~Mercury230AnalyzerResults()
{
}

void Mercury230AnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& /*channel*/, DisplayBase display_base )  //unrefereced vars commented out to remove warnings.
{
	//we only need to pay attention to 'channel' if we're making bubbles for more than one channel (as set by AddChannelBubblesWillAppearOn)
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );

	bool framing_error = false;
	if( ( frame.mFlags & FRAMING_ERROR_FLAG ) != 0 )
		framing_error = true;

	bool parity_error = false;
	if( ( frame.mFlags & PARITY_ERROR_FLAG ) != 0 )
		parity_error = true;

	U32 bits_per_transfer = mSettings->mBitsPerTransfer;

	if (mSettings->mMercury230Mode==Mercury230AnalyzerEnums::Mercury230Master || mSettings->mMercury230Mode==Mercury230AnalyzerEnums::Mercury230Slave)
	{
		char DeviceAddrStr[128];
		U8 DeviceAddr = (frame.mData1 & 0xFF00000000000000)>>56;
		AnalyzerHelpers::GetNumberString( DeviceAddr, display_base, bits_per_transfer, DeviceAddrStr, 128 );

		char FunctionCodeStr[128];
		U8 FunctionCode = (frame.mData1 & 0x00FF000000000000)>>48;
		AnalyzerHelpers::GetNumberString( FunctionCode, display_base, bits_per_transfer, FunctionCodeStr, 128 );

		char Param1Str[128];
		U8 Param1 = (frame.mData1 & 0x0000FF0000000000)>>40;
		AnalyzerHelpers::GetNumberString( Param1, display_base, bits_per_transfer, Param1Str, 128 );

		char Param2Str[128];
		U8 Param2 = (frame.mData1 & 0x000000FF00000000)>>32;
		AnalyzerHelpers::GetNumberString( Param2, display_base, bits_per_transfer, Param2Str, 128 );

		char Payload1Str[128];
		U16 Payload1 = (frame.mData1 & 0x0000FFFF00000000)>>32;
		AnalyzerHelpers::GetNumberString( Payload1, display_base, bits_per_transfer, Payload1Str, 128 );

		char Payload2Str[128];
		U16 Payload2 = (frame.mData1 & 0x00000000FFFF0000)>>16;
		AnalyzerHelpers::GetNumberString( Payload2, display_base, bits_per_transfer, Payload2Str, 128 );

		char Payload3Str[128];
		U16 Payload3 = (frame.mData2 & 0x000000000000FFFF);
		AnalyzerHelpers::GetNumberString( Payload3, display_base, bits_per_transfer, Payload3Str, 128 );

		char Payload4Str[128];
		U16 Payload4 = (frame.mData2 & 0x00000000FFFF0000)>>16;
		AnalyzerHelpers::GetNumberString( Payload4, display_base, bits_per_transfer, Payload4Str, 128 );

		char Password1Str[128];
		U16 Password1 = (frame.mData2 & 0xFFFF000000000000)>>54;
		AnalyzerHelpers::GetNumberString( Password1, display_base, bits_per_transfer, Password1Str, 128 );

		char Password2Str[128];
		U16 Password2 = (frame.mData2 & 0x0000FFFF00000000)>>32;
		AnalyzerHelpers::GetNumberString( Password1, display_base, bits_per_transfer, Password1Str, 128 );

		char Password3Str[128];
		U16 Password3 = (frame.mData2 & 0x00000000FFFF0000)>>16;
		AnalyzerHelpers::GetNumberString( Password1, display_base, bits_per_transfer, Password3Str, 128 );


		char ChecksumStr[128];
		U16 Checksum = (frame.mData1 & 0x000000000000FFFF);
		AnalyzerHelpers::GetNumberString( Checksum, display_base, bits_per_transfer, ChecksumStr, 128 );

		char result_str[256];
		char Error_str[128];

		if(frame.mFlags&FLAG_REQUEST_FRAME)
		{
			AddResultString( FunctionCodeStr);
			switch(FunctionCode)
			{
				case REQCODE_CHECK_CHANNEL:
					AddResultString( "Check channel" );
					sprintf( result_str, "DeviceID: %s, Func: Check channel (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, ChecksumStr );
					break;
				case REQCODE_OPEN_CHANNEL:
					AddResultString( "Open channel" );
					sprintf( result_str, "DeviceID: %s, Func: Open channel (%s), Access: %s, Password: %s, ChkSum: %s", DeviceAddrStr, FunctionCodeStr, "1", "123456", ChecksumStr );
					break;
				case REQCODE_CLOSE_CHANNEL:
					AddResultString( "Close channel" );
					sprintf( result_str, "DeviceID: %s, Func: Close channel (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, ChecksumStr );
					break;
				case REQCODE_READ_DATA_TIME:
					switch(Param1) {
						case READ_PARAM_TIME_CURENT_TIME:
							AddResultString( "Read current time" );
							sprintf( result_str, "DeviceID: %s, Func: Read current time (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, ChecksumStr );
							break;
						case READ_PARAM_TIME_ON_OFF:
							AddResultString( "Read on/off time" );
							sprintf( result_str, "DeviceID: %s, Func: Read on/off time (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, ChecksumStr );
							break;
						// case READ_PARAM_TIME_
					}
					break;
				case REQCODE_READ_DATA_ENERGY:
					AddResultString( "Read energy" );
					sprintf( result_str, "DeviceID: %s, Func: Read energy (%s), Params: %s %s, ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, Param2Str, ChecksumStr );
					break;
				case REQCODE_READ_DATA_PARAMS:
					switch(Param1) {
						case READ_PARAM_PARAM_SN:
							AddResultString( "Read params: serial number" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), SN: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_FAST_READ:
							AddResultString( "Read params: fast read" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Fast read: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_T_RATIO:
							AddResultString( "Read params: transformation ratio" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Transformation ratio: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_VERSION:
							AddResultString( "Read params: software version" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Software version: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_TIMEOUT:
							AddResultString( "Read params: timeout" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Timeout: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_ADDRESS:
							AddResultString( "Read params: address" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Address: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_SEASON_TIME:
							AddResultString( "Read params: season time" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Season time: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_OVERLIMIT:
							AddResultString( "Read params: overlimit" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Overlimit: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_FLAGS:
							AddResultString( "Read params: flags" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Flags: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_STATE:
							AddResultString( "Read params: state" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), State: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_LOCATION:
							AddResultString( "Read params: location" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Location: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_SCHEDULE_MAX_POWER:
							AddResultString( "Read params: schedule max power" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Schedule max power: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_VALUE_MAX_POWER:
							AddResultString( "Read params: value max power" );
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Value max power: (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
					}
					break;
			}
		}

		if(frame.mFlags&FLAG_CHECKSUM_ERROR)
			sprintf( result_str, "%s (Invalid Checksum!)", result_str );

		AddResultString( result_str );
	}
	else
	{
		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, bits_per_transfer, number_str, 128 );

		char result_str[128];

		//MP mode address case:
		bool mp_mode_address_flag = false;
		if( ( frame.mFlags & MP_MODE_ADDRESS_FLAG ) != 0 )
		{
			mp_mode_address_flag = true;

			AddResultString( "A" );
			AddResultString( "Addr" );

			if( framing_error == false )
			{
				sprintf( result_str, "Addr: %s", number_str );
				AddResultString( result_str );

				sprintf( result_str, "Address: %s", number_str );
				AddResultString( result_str );

			}else
			{
				sprintf( result_str, "Addr: %s (framing error)", number_str );
				AddResultString( result_str );

				sprintf( result_str, "Address: %s (framing error)", number_str );
				AddResultString( result_str );
			}
			return;
		}

		//normal case:
		if( ( parity_error == true ) || ( framing_error == true ) )
		{
			AddResultString( "!" );

			sprintf( result_str, "%s (error)", number_str );
			AddResultString( result_str );

			if( parity_error == true && framing_error == false )
				sprintf( result_str, "%s (parity error)", number_str );
			else
			if( parity_error == false && framing_error == true )
				sprintf( result_str, "%s (framing error)", number_str );
			else
				sprintf( result_str, "%s (framing error & parity error)", number_str );

			AddResultString( result_str );

		}else
		{
			AddResultString( number_str );
		}
	}
}

void Mercury230AnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 /*export_type_user_id*/ )
{
}

void Mercury230AnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
	Frame frame = GetFrame( frame_index );
    ClearTabularText();

	bool framing_error = false;
	if( ( frame.mFlags & FRAMING_ERROR_FLAG ) != 0 )
		framing_error = true;

	bool parity_error = false;
	if( ( frame.mFlags & PARITY_ERROR_FLAG ) != 0 )
		parity_error = true;


    U32 bits_per_transfer = mSettings->mBitsPerTransfer;

    if (mSettings->mMercury230Mode==Mercury230AnalyzerEnums::Mercury230Master || mSettings->mMercury230Mode==Mercury230AnalyzerEnums::Mercury230Slave)
    {
        char DeviceAddrStr[128];
        U8 DeviceAddr = (frame.mData1 & 0xFF00000000000000)>>56;
        AnalyzerHelpers::GetNumberString( DeviceAddr, display_base, bits_per_transfer, DeviceAddrStr, 128 );

        char FunctionCodeStr[128];
        U8 FunctionCode = (frame.mData1 & 0x00FF000000000000)>>48;
        AnalyzerHelpers::GetNumberString( FunctionCode, display_base, bits_per_transfer, FunctionCodeStr, 128 );

		char Param1Str[128];
		U8 Param1 = (frame.mData1 & 0x0000FF0000000000)>>40;
		AnalyzerHelpers::GetNumberString( Param1, display_base, bits_per_transfer, Param1Str, 128 );

		char Param2Str[128];
		U8 Param2 = (frame.mData1 & 0x000000FF00000000)>>32;
		AnalyzerHelpers::GetNumberString( Param2, display_base, bits_per_transfer, Param2Str, 128 );

        char Payload1Str[128];
        U16 Payload1 = (frame.mData1 & 0x0000FFFF00000000)>>32;
        AnalyzerHelpers::GetNumberString( Payload1, display_base, bits_per_transfer, Payload1Str, 128 );

        char Payload2Str[128];
        U16 Payload2 = (frame.mData1 & 0x00000000FFFF0000)>>16;
        AnalyzerHelpers::GetNumberString( Payload2, display_base, bits_per_transfer, Payload2Str, 128 );

        char Payload3Str[128];
        U16 Payload3 = (frame.mData2 & 0x000000000000FFFF);
        AnalyzerHelpers::GetNumberString( Payload3, display_base, bits_per_transfer, Payload3Str, 128 );

        char Payload4Str[128];
        U16 Payload4 = (frame.mData2 & 0x00000000FFFF0000)>>16;
        AnalyzerHelpers::GetNumberString( Payload4, display_base, bits_per_transfer, Payload4Str, 128 );

        char ChecksumStr[128];
        U16 Checksum = (frame.mData1 & 0x000000000000FFFF);
        AnalyzerHelpers::GetNumberString( Checksum, display_base, bits_per_transfer, ChecksumStr, 128 );

        char result_str[256];
        char Error_str[128];

        if(frame.mFlags&FLAG_REQUEST_FRAME)
        {
            switch(FunctionCode)
            {
				case REQCODE_CHECK_CHANNEL:
					sprintf( result_str, "DeviceID: %s, Func: Check channel (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, ChecksumStr );
					break;

				case REQCODE_OPEN_CHANNEL:
					sprintf( result_str, "DeviceID: %s, Func: Open channel (%s), Access: %s, Password: %s, ChkSum: %s", DeviceAddrStr, FunctionCodeStr, "1", "123456", ChecksumStr );
					break;

				case REQCODE_CLOSE_CHANNEL:
					sprintf( result_str, "DeviceID: %s, Func: Close channel (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, ChecksumStr );
					break;

				case REQCODE_READ_DATA_TIME:
					switch(Param1) {
						case READ_PARAM_TIME_CURENT_TIME:
							sprintf( result_str, "DeviceID: %s, Func: Read current time (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, ChecksumStr );
							break;
						case READ_PARAM_TIME_ON_OFF:
							sprintf( result_str, "DeviceID: %s, Func: Read on/off time (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, ChecksumStr );
							break;
						// case READ_PARAM_TIME_
					}
					break;

				case REQCODE_READ_DATA_ENERGY:
					sprintf( result_str, "DeviceID: %s, Func: Read energy (%s), Params: %s %s, ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, Param2Str, ChecksumStr );
					break;

				case REQCODE_READ_DATA_PARAMS:
					switch(Param1) {
						case READ_PARAM_PARAM_SN:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), SN (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						// TODO: extended params
						case READ_PARAM_PARAM_FAST_READ:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Fast read (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_T_RATIO:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Transformation ratio (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_VERSION:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Software version (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_TIMEOUT:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Timeout (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_ADDRESS:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Address (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						// TODO: extended params
						case READ_PARAM_PARAM_DISPLAY_MODE:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Display mode (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_SEASON_TIME:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Season time (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_OVERLIMIT:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Overlimit (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_FLAGS:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Flags (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_STATE:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), State (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_LOCATION:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Location (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_SCHEDULE_MAX_POWER:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Schedule max power (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;
						case READ_PARAM_PARAM_VALUE_MAX_POWER:
							sprintf( result_str, "DeviceID: %s, Func: Read params (%s), Value max power (%s), ChkSum: %s", DeviceAddrStr, FunctionCodeStr, Param1Str, ChecksumStr );
							break;	
					}
					break;
            }
        }
        else if(frame.mFlags&FLAG_RESPONSE_FRAME)
        {
        	//response handler
        }
        else if(frame.mFlags&FLAG_EXCEPTION_FRAME)
        {
        	//exception frame
        }
        else if(frame.mFlags&FLAG_FILE_SUBREQ)
        {
            sprintf( result_str, "SubRequest - RefType: %s, FileNum: %s, RecordNum: %s, RecordLen: %s", FunctionCodeStr, Payload1Str, Payload2Str, ChecksumStr );
        }
        else if(frame.mFlags&FLAG_DATA_FRAME)
        {
            sprintf( result_str, "Value: %s", Payload1Str );
        }
        else if(frame.mFlags&FLAG_END_FRAME)
        {
            sprintf( result_str, " Checksum: %s", ChecksumStr );
        }

        if(frame.mFlags&FLAG_CHECKSUM_ERROR)
            sprintf( result_str, "%s (Invalid Checksum!)", result_str );

        AddTabularText( result_str );
    }
    else
    {
        char number_str[128];
        AnalyzerHelpers::GetNumberString( frame.mData1, display_base, bits_per_transfer, number_str, 128 );

        char result_str[128];

        //MP mode address case:
        bool mp_mode_address_flag = false;
        if( ( frame.mFlags & MP_MODE_ADDRESS_FLAG ) != 0 )
        {
            mp_mode_address_flag = true;
            if( framing_error == false )
            {
               sprintf( result_str, "Address: %s", number_str );

            }else
            {
                sprintf( result_str, "Address: %s (framing error)", number_str );
            }
            return;
        }

        //normal case:
        if( ( parity_error == true ) || ( framing_error == true ) )
        {
            sprintf( result_str, "%s (error)", number_str );
            AddTabularText( result_str );

            if( parity_error == true && framing_error == false )
                sprintf( result_str, "%s (parity error)", number_str );
            else
            if( parity_error == false && framing_error == true )
                sprintf( result_str, "%s (framing error)", number_str );
            else
                sprintf( result_str, "%s (framing error & parity error)", number_str );

            AddTabularText( result_str );

        }else
        {
            AddTabularText( number_str );
        }
    }
}

void Mercury230AnalyzerResults::GeneratePacketTabularText( U64 /*packet_id*/, DisplayBase /*display_base*/ )  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

void Mercury230AnalyzerResults::GenerateTransactionTabularText( U64 /*transaction_id*/, DisplayBase /*display_base*/ )  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString( "not supported" );
}
