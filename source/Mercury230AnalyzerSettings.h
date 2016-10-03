#ifndef MERCURY230_ANALYZER_SETTINGS
#define MERCURY230_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

#ifdef __GNUC__
	#include <stdio.h>
	#include <string.h>
#endif

namespace Mercury230AnalyzerEnums	// Note: There is another definition of enum Mode in the cpp file.
{
	enum Mode { Mercury230Master, Mercury230Slave };
}

class Mercury230AnalyzerSettings : public AnalyzerSettings
{
public:
	Mercury230AnalyzerSettings();
	virtual ~Mercury230AnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	Channel mInputChannel;
	U32 mBitRate;
	U32 mBitsPerTransfer;
	AnalyzerEnums::ShiftOrder mShiftOrder;
	double mStopBits;
	AnalyzerEnums::Parity mParity;
	bool mInverted;
	bool mUseAutobaud;
	Mercury230AnalyzerEnums::Mode mMercury230Mode;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mBitsPerTransferInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mShiftOrderInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mStopBitsInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mParityInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mInvertedInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool >		mUseAutobaudInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mMercury230ModeInterface;
};

#endif //MERCURY230_ANALYZER_SETTINGS
