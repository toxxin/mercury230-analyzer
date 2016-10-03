#include "Mercury230AnalyzerSettings.h"

#include <AnalyzerHelpers.h>
#include <sstream>

#ifndef __GNUC__
	#pragma warning(disable: 4800) //warning C4800: 'U32' : forcing value to bool 'true' or 'false' (performance warning)
#endif


Mercury230AnalyzerSettings::Mercury230AnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mBitRate( 9600 ),
	mBitsPerTransfer( 8 ),
	mShiftOrder( AnalyzerEnums::LsbFirst ),
	mStopBits( 1.0 ),
	mParity( AnalyzerEnums::None ),
	mInverted( false ),
	mUseAutobaud( false ),
	mMercury230Mode( Mercury230AnalyzerEnums::Mercury230Master )
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "Mercury230", "Mercury230" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mMercury230ModeInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mMercury230ModeInterface->SetTitleAndTooltip( "Mercury230 Mode", "Specify which mode of Mercury230 this is" );
	mMercury230ModeInterface->AddNumber( Mercury230AnalyzerEnums::Mercury230Master, "Mercury230 - Master", "(messages are transmitted in binary)" );
	mMercury230ModeInterface->AddNumber( Mercury230AnalyzerEnums::Mercury230Slave, "Mercury230 - Slave", "(messages are transmitted in binary)" );
	mMercury230ModeInterface->SetNumber( mMercury230Mode );

	mBitRateInterface.reset( new AnalyzerSettingInterfaceInteger() );
	mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/s)",  "Specify the bit rate in bits per second." );
	mBitRateInterface->SetMax( 6000000 );
	mBitRateInterface->SetMin( 1 );
	mBitRateInterface->SetInteger( mBitRate );

	mInvertedInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mInvertedInterface->SetTitleAndTooltip( "", "Specify if the serial signal is inverted" );
	mInvertedInterface->AddNumber( false, "Non Inverted (Standard)", "" );
	mInvertedInterface->AddNumber( true, "Inverted", "" );
	mInvertedInterface->SetNumber( mInverted );
	enum Mode { Normal, MpModeRightZeroMeansAddress, MpModeRightOneMeansAddress, MpModeLeftZeroMeansAddress, MpModeLeftOneMeansAddress };	// FIXME: unused?

	AddInterface( mInputChannelInterface.get() );
	AddInterface( mMercury230ModeInterface.get() );
	AddInterface( mBitRateInterface.get() );
	AddInterface( mInvertedInterface.get() );

	//AddExportOption( 0, "Export as text/csv file", "text (*.txt);;csv (*.csv)" );
	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "Mercury230", false );
}

Mercury230AnalyzerSettings::~Mercury230AnalyzerSettings()
{
}

bool Mercury230AnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	mBitRate = mBitRateInterface->GetInteger();
	mInverted = bool( U32( mInvertedInterface->GetNumber() ) );
	mMercury230Mode = Mercury230AnalyzerEnums::Mode( U32( mMercury230ModeInterface->GetNumber() ) );

	ClearChannels();
	AddChannel( mInputChannel, "Mercury230", true );

	return true;
}

void Mercury230AnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mBitRateInterface->SetInteger( mBitRate );
	mInvertedInterface->SetNumber( mInverted );
	mMercury230ModeInterface->SetNumber( mMercury230Mode );
}

void Mercury230AnalyzerSettings::LoadSettings( const char* settings )
{
	// Example: $3 = 0x1647478 "22 serialization::archive 5 22 Mercury230AnalyzerSettings 0 8 1324695933 0 9600"
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	const char* name_string;	//the first thing in the archive is the name of the protocol analyzer that the data belongs to.
	text_archive >> &name_string;
	if( strcmp( name_string, "Mercury230AnalyzerSettings" ) != 0 ) {
		if( strcmp( name_string, "SaleaeAsyncMercury230Analyzer" ) != 0 ) {	// The old string; treat them the same for now.
			AnalyzerHelpers::Assert( "Mercury230AnalyzerSettings: Provided with a settings string that doesn't belong to us;" );
		}
	}
	text_archive >> mInputChannel;
	text_archive >> mBitRate;
	text_archive >> mInverted;

	Mercury230AnalyzerEnums::Mode mode;
	if( text_archive >> *(U32*)&mode )
		mMercury230Mode = mode;

	ClearChannels();
	AddChannel( mInputChannel, "Mercury230", true );

	UpdateInterfacesFromSettings();
}

const char* Mercury230AnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << "Mercury230AnalyzerSettings";
	text_archive << mInputChannel;
	text_archive << mBitRate;
	text_archive << mInverted;

	text_archive << mMercury230Mode;

	return SetReturnString( text_archive.GetString() );
}
