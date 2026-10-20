// WwiseFx2Smoke: agent IH4 fork F2's checks of the ported Matrix Reverb, Delay, Tremolo and Pitch Shifter effect
// plug-ins. Each plug-in is created through its retail factory with a parameter block read out of Init.bnk (the FX
// share sets the game ships: HIRC type 18), driven with a test signal through Execute in 1024-frame buffers at 48 kHz,
// and its output measured against what the parameters imply.
#if DISHONORED_WITH_WWISE

#define WIN32_LEAN_AND_MEAN
#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/AkDelayFXFactory.h>
#include <AK/Plugin/AkMatrixReverbFXFactory.h>
#include <AK/Plugin/AkTremoloFXFactory.h>
#include <AK/Plugin/AkPitchShifterFXFactory.h>
#include <AK/Plugin/AkTimeStretchFXFactory.h>
#include <AK/Plugin/AkStereoDelayFXFactory.h>
#include <AK/Plugin/AkHarmonizerFXFactory.h>

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <malloc.h>
#include <string>
#include <vector>

typedef void ( *WwiseReportFunc )( bool Passed, const char * Name, const char * Detail );
typedef void ( *WwiseSkipFunc )( const char * Name, const char * Reason );

namespace
{
const wchar_t * const Fx2CookedDir = L"D:\\RecompileDishonored\\Dishonored_Latest2026\\DishonoredGame\\CookedPCConsole\\";
const AkUInt32 SampleRate = 48000;
const AkUInt16 FrameSize = 1024;

class FFx2Allocator : public AK::IAkPluginMemAlloc
{
public:
	int Outstanding = 0;
	virtual void * Malloc( size_t in_uSize )
	{
		++Outstanding;
		return _aligned_malloc( in_uSize ? in_uSize : 1, 16 );
	}
	virtual void Free( void * in_pMemAddress )
	{
		--Outstanding;
		_aligned_free( in_pMemAddress );
	}
};

class FFx2Context : public AK::IAkEffectPluginContext
{
public:
	virtual bool IsSendModeEffect() const { return false; }
	virtual AK::IAkStreamMgr * GetStreamMgr() const { return NULL; }
	virtual AkPlayingID GetPlayingID() const { return 0; }
	virtual AkUInt16 GetMaxBufferLength() const { return FrameSize; }
	virtual AKRESULT PostMonitorData( void *, AkUInt32 ) { return AK_Fail; }
	virtual bool CanPostMonitorData() { return false; }
	virtual void GetPluginMedia( AkUInt32, AkUInt8 * & out_rpData, AkUInt32 & out_rDataSize ) { out_rpData = NULL; out_rDataSize = 0; }
	virtual AK::IAkProcessorFeatures * GetProcessorFeatures() { return NULL; }
};

std::vector<unsigned char> Fx2ReadFile( const std::wstring & Path )
{
	std::vector<unsigned char> Data;
	FILE * File = _wfopen( Path.c_str(), L"rb" );
	if( File )
	{
		fseek( File, 0, SEEK_END );
		Data.resize( (size_t)ftell( File ) );
		fseek( File, 0, SEEK_SET );
		if( !Data.empty() && fread( Data.data(), 1, Data.size(), File ) != Data.size() )
		{
			Data.clear();
		}
		fclose( File );
	}
	return Data;
}

/** The parameter block of the FX share set (HIRC type 18) in_uID: body = u32 id, u32 plug-in id, u32 size, params. */
bool Fx2FindParams( const std::vector<unsigned char> & Bank, AkUInt32 in_uID, AkUInt32 & out_uPluginID, std::vector<unsigned char> & out_Params )
{
	size_t Pos = 0;
	while( Pos + 8 <= Bank.size() )
	{
		const AkUInt32 Size = *(const AkUInt32 *)&Bank[ Pos + 4 ];
		if( !memcmp( &Bank[ Pos ], "HIRC", 4 ) )
		{
			const AkUInt32 Count = *(const AkUInt32 *)&Bank[ Pos + 8 ];
			size_t Obj = Pos + 12;
			for( AkUInt32 i = 0; i < Count && Obj + 5 <= Bank.size(); ++i )
			{
				const unsigned char Type = Bank[ Obj ];
				const AkUInt32 ObjSize = *(const AkUInt32 *)&Bank[ Obj + 1 ];
				const unsigned char * Body = &Bank[ Obj + 5 ];
				if( ( Type == 18 || Type == 19 ) && *(const AkUInt32 *)Body == in_uID )
				{
					out_uPluginID = *(const AkUInt32 *)( Body + 4 );
					const AkUInt32 BlockSize = *(const AkUInt32 *)( Body + 8 );
					out_Params.assign( Body + 12, Body + 12 + BlockSize );
					return true;
				}
				Obj += 5 + ObjSize;
			}
		}
		Pos += 8 + Size;
	}
	return false;
}

struct FFx2Buffer
{
	std::vector<float> Data;
	AkAudioBuffer Buffer;

	explicit FFx2Buffer( AkChannelMask Mask )
	{
		Data.resize( (size_t)FrameSize * AkGetNumChannels( Mask ) + 4 );
		Buffer.pData = (void *)( ( (uintptr_t)Data.data() + 15 ) & ~(uintptr_t)15 );
		Buffer.uChannelMask = Mask;
		Buffer.eState = AK_DataReady;
		Buffer.uMaxFrames = FrameSize;
		Buffer.uValidFrames = FrameSize;
	}
	void Prepare()
	{
		Buffer.eState = AK_DataReady;
		Buffer.uValidFrames = FrameSize;
	}
};

/** Runs in_uNumFrames frames through the in-place effect; in_Input( frame, channel ) gives the input, the output is
	appended per channel. */
template < class InputFunc >
void Fx2Run( AK::IAkInPlaceEffectPlugin * Effect, AkChannelMask Mask, AkUInt32 NumFrames, InputFunc Input, std::vector< std::vector<float> > & Output, AK::IAkPluginParam * Params = NULL, AkUInt32 SetParamAtFrame = (AkUInt32)-1, AkPluginParamID SetParamID = 0, float SetParamValue = 0.f )
{
	const AkUInt32 NumChannels = AkGetNumChannels( Mask );
	Output.assign( NumChannels, std::vector<float>() );
	FFx2Buffer Buf( Mask );
	for( AkUInt32 Start = 0; Start < NumFrames; Start += FrameSize )
	{
		if( Params && Start <= SetParamAtFrame && SetParamAtFrame < Start + FrameSize )
		{
			Params->SetParam( SetParamID, &SetParamValue, sizeof( SetParamValue ) );
		}
		Buf.Prepare();
		for( AkUInt32 c = 0; c < NumChannels; ++c )
		{
			float * Channel = Buf.Buffer.GetChannel( c );
			for( AkUInt32 f = 0; f < FrameSize; ++f )
			{
				Channel[ f ] = Input( Start + f, c );
			}
		}
		Effect->Execute( &Buf.Buffer );
		for( AkUInt32 c = 0; c < NumChannels; ++c )
		{
			const float * Channel = Buf.Buffer.GetChannel( c );
			Output[ c ].insert( Output[ c ].end(), Channel, Channel + Buf.Buffer.uValidFrames );
		}
	}
}

double Fx2Db( double Ratio )
{
	return 20.0 * log10( fabs( Ratio ) > 1e-30 ? fabs( Ratio ) : 1e-30 );
}

/** The frequency of the strongest bin of a Hann-windowed DFT around in_fGuess (1 Hz steps, parabolic refinement). */
double Fx2PeakFrequency( const std::vector<float> & Signal, size_t Start, size_t Length, double LowHz, double HighHz )
{
	std::vector<double> Window( Length );
	for( size_t n = 0; n < Length; ++n )
	{
		Window[ n ] = Signal[ Start + n ] * ( 0.5 - 0.5 * cos( 2.0 * 3.14159265358979 * n / ( Length - 1 ) ) );
	}
	double BestHz = LowHz, BestMag = -1.0;
	std::vector<double> Mags;
	for( double Hz = LowHz; Hz <= HighHz; Hz += 1.0 )
	{
		double Re = 0.0, Im = 0.0;
		const double W = 2.0 * 3.14159265358979 * Hz / SampleRate;
		for( size_t n = 0; n < Length; ++n )
		{
			Re += Window[ n ] * cos( W * n );
			Im -= Window[ n ] * sin( W * n );
		}
		const double Mag = sqrt( Re * Re + Im * Im );
		Mags.push_back( Mag );
		if( Mag > BestMag )
		{
			BestMag = Mag;
			BestHz = Hz;
		}
	}
	const size_t Index = (size_t)( BestHz - LowHz );
	if( Index > 0 && Index + 1 < Mags.size() )
	{
		const double A = Mags[ Index - 1 ], B = Mags[ Index ], C = Mags[ Index + 1 ];
		const double Denominator = A - 2.0 * B + C;
		if( Denominator != 0.0 )
		{
			BestHz += 0.5 * ( A - C ) / Denominator;
		}
	}
	return BestHz;
}

WwiseReportFunc Fx2Report = NULL;

void Fx2Check( bool Passed, const char * Name, const char * Format, ... )
{
	char Detail[ 512 ];
	va_list Args;
	va_start( Args, Format );
	vsnprintf( Detail, sizeof( Detail ), Format, Args );
	va_end( Args );
	printf( "      %s: %s\n", Name, Detail );
	Fx2Report( Passed, Name, Detail );
}

struct FFx2Plugin
{
	FFx2Allocator & Allocator;
	AK::IAkPluginParam * Params = NULL;
	AK::IAkInPlaceEffectPlugin * Effect = NULL;
	AKRESULT InitResult = AK_Fail;

	FFx2Plugin( FFx2Allocator & InAllocator, AK::IAkPlugin * ( *CreateFX )( AK::IAkPluginMemAlloc * ), AK::IAkPluginParam * ( *CreateParams )( AK::IAkPluginMemAlloc * ), const std::vector<unsigned char> & Block, AkChannelMask Mask )
		: Allocator( InAllocator )
	{
		static FFx2Context Context;
		Params = CreateParams( &Allocator );
		Effect = static_cast<AK::IAkInPlaceEffectPlugin *>( CreateFX( &Allocator ) );
		if( Params && Effect && Params->Init( &Allocator, Block.data(), (AkUInt32)Block.size() ) == AK_Success )
		{
			AkAudioFormat Format;
			Format.SetAll( SampleRate, Mask, 32, 4 * AkGetNumChannels( Mask ), 1, 0 );
			InitResult = Effect->Init( &Allocator, &Context, Params, Format );
			if( InitResult == AK_Success )
			{
				Effect->Reset();
			}
		}
	}
	~FFx2Plugin()
	{
		if( Effect )
		{
			Effect->Term( &Allocator );
		}
		if( Params )
		{
			Params->Term( &Allocator );
		}
	}
};

void TestDelay( const std::vector<unsigned char> & Bank )
{
	AkUInt32 PluginID = 0;
	std::vector<unsigned char> Block;
	if( !Fx2FindParams( Bank, 0x29e9c8cb, PluginID, Block ) )
	{
		Fx2Check( false, "Wwise Delay: Init.bnk share set 0x29e9c8cb found", "missing" );
		return;
	}
	const float DelayTime = *(const float *)&Block[ 0 ];
	const float FeedbackPercent = *(const float *)&Block[ 4 ];
	const float WetPercent = *(const float *)&Block[ 8 ];
	const float LevelDb = *(const float *)&Block[ 12 ];
	// The bank's share set has its feedback (15 %) disabled (byte 16 = 0); retail's Process zeroes the shared feedback
	// value while it is disabled, so the check enables it in the block before Init to measure the feedback path.
	Block[ 16 ] = 1;
	FFx2Allocator Allocator;
	std::vector< std::vector<float> > Out;
	{
		FFx2Plugin Plugin( Allocator, CreateDelayFX, CreateDelayFXParams, Block, AK_SPEAKER_SETUP_STEREO );
		Fx2Check( PluginID >> 16 == AKEFFECTID_DELAY && Plugin.InitResult == AK_Success, "Wwise Delay (106): Init from Init.bnk 0x29e9c8cb", "plug-in %u, block %u bytes, delay %.3f s, feedback %.1f %% (enabled %u), wet %.0f %%, level %.1f dB, Init %d",
			PluginID >> 16, (unsigned)Block.size(), DelayTime, FeedbackPercent, Block[ 16 ], WetPercent, LevelDb, Plugin.InitResult );
		if( Plugin.InitResult != AK_Success )
		{
			return;
		}
		const AkUInt32 DelayFrames = ( (AkUInt32)( SampleRate * DelayTime ) + 3 ) & ~3u;
		Fx2Run( Plugin.Effect, AK_SPEAKER_SETUP_STEREO, 3 * DelayFrames + 4 * FrameSize, []( AkUInt32 Frame, AkUInt32 ) { return Frame == 0 ? 1.f : 0.f; }, Out );
		const std::vector<float> & Left = Out[ 0 ];
		size_t FirstEcho = 0;
		float FirstPeak = 0.f;
		for( size_t i = 1; i < 2 * (size_t)DelayFrames - 8 && i < Left.size(); ++i )
		{
			if( fabs( Left[ i ] ) > fabs( FirstPeak ) )
			{
				FirstPeak = Left[ i ];
				FirstEcho = i;
			}
		}
		const double ExpectedGain = ( WetPercent * 0.01 ) * pow( 10.0, LevelDb / 20.0 );
		Fx2Check( FirstEcho == DelayFrames && fabs( Fx2Db( FirstPeak / ExpectedGain ) ) < 0.1, "Wwise Delay: impulse echo at DelayTime (wet x level)",
			"echo at frame %u (expected %u), gain %.3f dB (expected %.3f dB), dry at frame 0: %.4f", (unsigned)FirstEcho, DelayFrames, Fx2Db( FirstPeak ), Fx2Db( ExpectedGain ), Left[ 0 ] );
		const float SecondPeak = 2 * (size_t)DelayFrames < Left.size() ? Left[ 2 * (size_t)DelayFrames ] : 0.f;
		const double Ratio = Fx2Db( SecondPeak / FirstPeak );
		const double ExpectedRatio = Fx2Db( FeedbackPercent * 0.01 );
		Fx2Check( fabs( Ratio - ExpectedRatio ) < 0.1, "Wwise Delay: feedback (enabled in the block) second echo / first echo",
			"%.3f dB at frame %u (expected %.3f dB = %.1f %% feedback)", Ratio, 2 * DelayFrames, ExpectedRatio, FeedbackPercent );
	}
	Fx2Check( Allocator.Outstanding == 0, "Wwise Delay: every allocation released at Term", "%d outstanding", Allocator.Outstanding );
}

void TestTremolo( const std::vector<unsigned char> & Bank )
{
	FFx2Allocator Allocator;
	{
		AkUInt32 PluginID = 0;
		std::vector<unsigned char> Block;
		if( !Fx2FindParams( Bank, 0xbf997a5a, PluginID, Block ) )
		{
			Fx2Check( false, "Wwise Tremolo: Init.bnk share set 0xbf997a5a found", "missing" );
			return;
		}
		const float Frequency = *(const float *)&Block[ 4 ];
		const AkUInt32 Waveform = *(const AkUInt32 *)&Block[ 8 ];
		const float PWM = *(const float *)&Block[ 16 ] * 0.01f;
		FFx2Plugin Plugin( Allocator, CreateTremoloFX, CreateTremoloFXParams, Block, AK_SPEAKER_SETUP_STEREO );
		Fx2Check( PluginID >> 16 == AKEFFECTID_TREMOLO && Plugin.InitResult == AK_Success, "Wwise Tremolo (131): Init from Init.bnk 0xbf997a5a", "waveform %u, %.2f Hz, depth %.0f %%, PWM %.0f %%, Init %d",
			Waveform, Frequency, *(const float *)&Block[ 0 ], PWM * 100.f, Plugin.InitResult );
		if( Plugin.InitResult != AK_Success )
		{
			return;
		}
		std::vector< std::vector<float> > Out;
		const AkUInt32 NumFrames = 10 * SampleRate;
		Fx2Run( Plugin.Effect, AK_SPEAKER_SETUP_STEREO, NumFrames, []( AkUInt32, AkUInt32 ) { return 1.f; }, Out );
		const std::vector<float> & Gain = Out[ 0 ];
		std::vector<size_t> Rising;
		size_t HighFrames = 0;
		for( size_t i = 1; i < Gain.size(); ++i )
		{
			if( Gain[ i - 1 ] < 0.5f && Gain[ i ] >= 0.5f )
			{
				Rising.push_back( i );
			}
			HighFrames += Gain[ i ] >= 0.5f;
		}
		const double Measured = Rising.size() >= 2 ? (double)( Rising.size() - 1 ) * SampleRate / (double)( Rising.back() - Rising.front() ) : 0.0;
		Fx2Check( fabs( Measured - Frequency ) < 0.01 * Frequency, "Wwise Tremolo: square LFO rate", "%.4f Hz from %u cycles (expected %.4f Hz); duty %.1f %% (PWM %.0f %%)",
			Measured, (unsigned)( Rising.size() ? Rising.size() - 1 : 0 ), Frequency, 100.0 * HighFrames / Gain.size(), PWM * 100.f );
	}
	{
		AkUInt32 PluginID = 0;
		std::vector<unsigned char> Block;
		if( !Fx2FindParams( Bank, 0xbe158cfc, PluginID, Block ) )
		{
			Fx2Check( false, "Wwise Tremolo: Init.bnk share set 0xbe158cfc found", "missing" );
			return;
		}
		const float Depth = *(const float *)&Block[ 0 ] * 0.01f;
		const float Frequency = *(const float *)&Block[ 4 ];
		FFx2Plugin Plugin( Allocator, CreateTremoloFX, CreateTremoloFXParams, Block, AK_SPEAKER_SETUP_STEREO );
		if( Plugin.InitResult != AK_Success )
		{
			Fx2Check( false, "Wwise Tremolo (131): Init from Init.bnk 0xbe158cfc", "Init %d", Plugin.InitResult );
			return;
		}
		std::vector< std::vector<float> > Out;
		const AkUInt32 NumFrames = (AkUInt32)( 2.0 * SampleRate / Frequency ) + FrameSize;
		Fx2Run( Plugin.Effect, AK_SPEAKER_SETUP_STEREO, NumFrames, []( AkUInt32, AkUInt32 ) { return 1.f; }, Out );
		float Min = 2.f, Max = -2.f;
		for( float Value : Out[ 0 ] )
		{
			Min = Value < Min ? Value : Min;
			Max = Value > Max ? Value : Max;
		}
		Fx2Check( fabs( ( Max - Min ) - Depth ) < 0.01f && fabs( Min - ( 1.f - Depth ) ) < 0.01f, "Wwise Tremolo: sine LFO depth", "gain %.4f..%.4f (expected %.4f..1.0000, depth %.0f %%, %.2f Hz)",
			Min, Max, 1.f - Depth, Depth * 100.f, Frequency );
	}
	Fx2Check( Allocator.Outstanding == 0, "Wwise Tremolo: every allocation released at Term", "%d outstanding", Allocator.Outstanding );
}

void TestPitchShifter( const std::vector<unsigned char> & Bank )
{
	FFx2Allocator Allocator;
	{
		AkUInt32 PluginID = 0;
		std::vector<unsigned char> Block;
		if( !Fx2FindParams( Bank, 0x933614a5, PluginID, Block ) )
		{
			Fx2Check( false, "Wwise Pitch Shifter: Init.bnk share set 0x933614a5 found", "missing" );
			return;
		}
		float Cents;
		memcpy( &Cents, &Block[ 18 ], sizeof( Cents ) );
		FFx2Plugin Plugin( Allocator, CreatePitchShifterFX, CreatePitchShifterFXParams, Block, AK_SPEAKER_SETUP_STEREO );
		Fx2Check( PluginID >> 16 == AKEFFECTID_PITCHSHIFTER && Plugin.InitResult == AK_Success, "Wwise Pitch Shifter (136): Init from Init.bnk 0x933614a5",
			"pitch %.0f cents, delay %.0f ms, dry %.0f dB, wet %.0f dB, Init %d", Cents, *(const float *)&Block[ 12 ], *(const float *)&Block[ 4 ], *(const float *)&Block[ 8 ], Plugin.InitResult );
		if( Plugin.InitResult != AK_Success )
		{
			return;
		}
		const double InputHz = 1000.0;
		std::vector< std::vector<float> > Out;
		Fx2Run( Plugin.Effect, AK_SPEAKER_SETUP_STEREO, 2 * SampleRate, [InputHz]( AkUInt32 Frame, AkUInt32 ) { return (float)( 0.5 * sin( 2.0 * 3.14159265358979 * InputHz * Frame / SampleRate ) ); }, Out );
		const double Expected = InputHz * pow( 2.0, Cents / 1200.0 );
		const double Measured = Fx2PeakFrequency( Out[ 0 ], SampleRate / 2, 16384, Expected * 0.5, Expected * 1.5 );
		Fx2Check( fabs( Measured - Expected ) < 0.01 * Expected, "Wwise Pitch Shifter: 1 kHz sine shifted by the bank's cents", "%.2f Hz (expected %.2f Hz = 1000 x 2^(%.0f/1200))", Measured, Expected, Cents );
	}
	Fx2Check( Allocator.Outstanding == 0, "Wwise Pitch Shifter: every allocation released at Term", "%d outstanding", Allocator.Outstanding );
}

/** Schroeder RT60 (-5..-25 dB slope x 3) of L+R after an optional 2nd-order Butterworth low-pass (in_fLowPassHz > 0)
	or high-pass (in_fLowPassHz < 0, cutoff -in_fLowPassHz). */
double Fx2RT60( const std::vector< std::vector<float> > & Out, double in_fFilterHz )
{
	std::vector<double> Power( Out[ 0 ].size(), 0.0 );
	for( size_t c = 0; c < Out.size(); ++c )
	{
		double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
		if( in_fFilterHz != 0.0 )
		{
			const double K = tan( 3.14159265358979 * fabs( in_fFilterHz ) / SampleRate );
			const double Norm = 1.0 / ( 1.0 + sqrt( 2.0 ) * K + K * K );
			if( in_fFilterHz > 0.0 )
			{
				b0 = K * K * Norm; b1 = 2.0 * b0; b2 = b0;
			}
			else
			{
				b0 = Norm; b1 = -2.0 * b0; b2 = b0;
			}
			a1 = 2.0 * ( K * K - 1.0 ) * Norm;
			a2 = ( 1.0 - sqrt( 2.0 ) * K + K * K ) * Norm;
		}
		double x1 = 0.0, x2 = 0.0, y1 = 0.0, y2 = 0.0;
		for( size_t i = 1; i < Out[ c ].size(); ++i )
		{
			const double x = Out[ c ][ i ];
			const double y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
			x2 = x1; x1 = x; y2 = y1; y1 = y;
			Power[ i ] += y * y;
		}
	}
	std::vector<double> Energy( Power.size() + 1, 0.0 );
	for( size_t i = Power.size(); i-- > 1; )
	{
		Energy[ i ] = Energy[ i + 1 ] + Power[ i ];
	}
	size_t At5 = 0, At25 = 0;
	for( size_t i = 1; i < Power.size(); ++i )
	{
		const double Db = 10.0 * log10( Energy[ i ] / Energy[ 1 ] + 1e-30 );
		if( !At5 && Db <= -5.0 )
		{
			At5 = i;
		}
		if( Db <= -25.0 )
		{
			At25 = i;
			break;
		}
	}
	return At25 > At5 ? 3.0 * (double)( At25 - At5 ) / SampleRate : 0.0;
}

void TestMatrixReverb( const std::vector<unsigned char> & Bank )
{
	FFx2Allocator Allocator;
	AkUInt32 PluginID = 0;
	std::vector<unsigned char> Block;
	if( !Fx2FindParams( Bank, 0x1feda2f4, PluginID, Block ) )
	{
		Fx2Check( false, "Wwise Matrix Reverb: Init.bnk share set 0x1feda2f4 found", "missing" );
		return;
	}
	const float ReverbTime = *(const float *)&Block[ 0 ];
	const float HFRatio = *(const float *)&Block[ 4 ];
	const AkUInt32 NumDelays = *(const AkUInt32 *)&Block[ 8 ];
	const AkUInt32 NumFrames = (AkUInt32)( 3.0 * ReverbTime * SampleRate );
	{
		FFx2Plugin Plugin( Allocator, CreateMatrixReverbFX, CreateMatrixReverbFXParams, Block, AK_SPEAKER_SETUP_STEREO );
		Fx2Check( PluginID >> 16 == AKEFFECTID_MATRIXREVERB && Plugin.InitResult == AK_Success, "Wwise Matrix Reverb (115): Init from Init.bnk 0x1feda2f4",
			"reverb time %.2f s, HF ratio %.2f, %u delays, wet %.0f dB, Init %d", ReverbTime, HFRatio, NumDelays, *(const float *)&Block[ 16 ], Plugin.InitResult );
		if( Plugin.InitResult != AK_Success )
		{
			return;
		}
		std::vector< std::vector<float> > Out;
		Fx2Run( Plugin.Effect, AK_SPEAKER_SETUP_STEREO, NumFrames, []( AkUInt32 Frame, AkUInt32 ) { return Frame == 0 ? 1.f : 0.f; }, Out );
		const double LowRT = Fx2RT60( Out, 250.0 );
		const double HighRT = Fx2RT60( Out, -8000.0 );
		// Derived from the parameters: every line's loop gain is 10^(-3 L T / RT) (-60 dB/s) up to 250 Hz, except the
		// longest one: with HF ratio 4.5 retail's stability clamp (scale = 1 / log10(g_max) ln10/4 when the product exceeds
		// 1) puts that line's damping pole exactly at 1 with a zero input gain, so 15 of 16 lines carry the tail and it is
		// shorter than the reverb time; with HF ratio 1 (below) no line is damped and the tail is the reverb time.
		Fx2Check( LowRT > 0.7 * ReverbTime && LowRT < 1.0 * ReverbTime, "Wwise Matrix Reverb: low band (< 250 Hz) RT60, 15 of 16 lines live (HF clamp)",
			"%.3f s (reverb time %.2f s)", LowRT, ReverbTime );
		Fx2Check( HighRT > 0.0 && LowRT / HighRT > 2.0, "Wwise Matrix Reverb: the HF ratio damps the highs (> 8 kHz RT60 shorter)",
			"%.3f s, low / high %.2f (HF ratio %.2f)", HighRT, HighRT > 0.0 ? LowRT / HighRT : 0.0, HFRatio );
	}
	{
		// With the HF ratio at 1 the damping filter is a plain gain (retail: scale 1 - 1/(1/HF)^2 = 0), so the whole
		// spectrum decays at the reverb time.
		const float HFRatioOne = 1.f;
		memcpy( &Block[ 4 ], &HFRatioOne, sizeof( HFRatioOne ) );
		FFx2Plugin Plugin( Allocator, CreateMatrixReverbFX, CreateMatrixReverbFXParams, Block, AK_SPEAKER_SETUP_STEREO );
		std::vector< std::vector<float> > Out;
		if( Plugin.InitResult == AK_Success )
		{
			Fx2Run( Plugin.Effect, AK_SPEAKER_SETUP_STEREO, NumFrames, []( AkUInt32 Frame, AkUInt32 ) { return Frame == 0 ? 1.f : 0.f; }, Out );
		}
		const double RT60 = Out.empty() ? 0.0 : Fx2RT60( Out, 0.0 );
		Fx2Check( RT60 > 0.9 * ReverbTime && RT60 < 1.1 * ReverbTime, "Wwise Matrix Reverb: broadband RT60 with HF ratio 1 (Schroeder -5..-25 dB x 3)",
			"%.3f s (reverb time %.2f s)", RT60, ReverbTime );
	}
	Fx2Check( Allocator.Outstanding == 0, "Wwise Matrix Reverb: every allocation released at Term", "%d outstanding", Allocator.Outstanding );
}

/** The plug-ins no Dishonored bank uses (Time Stretch is registered, Stereo Delay and Harmonizer are not), on their
	defaults plus one parameter each. */
void TestUnusedPlugins()
{
	FFx2Allocator Allocator;
	{
		AK::IAkPluginParam * Params = CreateTimeStretchFXParams( &Allocator );
		AK::IAkOutOfPlaceEffectPlugin * Effect = static_cast<AK::IAkOutOfPlaceEffectPlugin *>( CreateTimeStretchFX( &Allocator ) );
		static FFx2Context Context;
		Params->Init( &Allocator, NULL, 0 );
		const float Stretch = 200.f;
		Params->SetParam( 1, &Stretch, sizeof( Stretch ) );
		AkAudioFormat Format;
		Format.SetAll( SampleRate, AK_SPEAKER_SETUP_MONO, 32, 4, 1, 0 );
		const AKRESULT InitResult = Effect->Init( &Allocator, &Context, Params, Format );
		Effect->Reset();
		FFx2Buffer In( AK_SPEAKER_SETUP_MONO ), Out( AK_SPEAKER_SETUP_MONO );
		std::vector<float> Output;
		AkUInt32 InputFrames = 0, Calls = 0;
		In.Buffer.uValidFrames = 0;
		Out.Buffer.uValidFrames = 0;
		while( InputFrames < 4 * SampleRate && Calls < 100000 && InitResult == AK_Success )
		{
			++Calls;
			if( !In.Buffer.uValidFrames )
			{
				float * Channel = In.Buffer.GetChannel( 0 );
				for( AkUInt32 f = 0; f < FrameSize; ++f )
				{
					Channel[ f ] = (float)( 0.5 * sin( 2.0 * 3.14159265358979 * 1000.0 * ( InputFrames + f ) / SampleRate ) );
				}
				InputFrames += FrameSize;
				In.Buffer.uValidFrames = FrameSize;
				In.Buffer.eState = AK_DataReady;
			}
			Effect->Execute( &In.Buffer, FrameSize - In.Buffer.uValidFrames, &Out.Buffer );
			if( Out.Buffer.uValidFrames == Out.Buffer.uMaxFrames )
			{
				Output.insert( Output.end(), Out.Buffer.GetChannel( 0 ), Out.Buffer.GetChannel( 0 ) + FrameSize );
				Out.Buffer.uValidFrames = 0;
			}
		}
		const double Ratio = (double)Output.size() / InputFrames;
		const double Hz = Output.size() > SampleRate + 16384 ? Fx2PeakFrequency( Output, SampleRate, 16384, 900.0, 1100.0 ) : 0.0;
		Fx2Check( InitResult == AK_Success && fabs( Ratio - 2.0 ) < 0.05 && fabs( Hz - 1000.0 ) < 10.0, "Wwise Time Stretch (130, no bank uses it): 200 % stretch of a 1 kHz sine",
			"Init %d, %u frames in, %u out (ratio %.3f, expected 2), output peak %.1f Hz (expected 1000)", InitResult, InputFrames, (unsigned)Output.size(), Ratio, Hz );
		Effect->Term( &Allocator );
		Params->Term( &Allocator );
	}
	{
		std::vector<unsigned char> NoBlock;
		FFx2Plugin Plugin( Allocator, CreateStereoDelayFX, CreateStereoDelayFXParams, NoBlock, AK_SPEAKER_SETUP_STEREO );
		std::vector< std::vector<float> > Out;
		if( Plugin.InitResult == AK_Success )
		{
			Fx2Run( Plugin.Effect, AK_SPEAKER_SETUP_STEREO, SampleRate, []( AkUInt32 Frame, AkUInt32 Channel ) { return Frame == 0 && Channel == 0 ? 1.f : 0.f; }, Out );
		}
		size_t Echo = 0;
		for( size_t i = 1; !Out.empty() && i < Out[ 0 ].size(); ++i )
		{
			if( fabs( Out[ 0 ][ i ] ) > 0.5f )
			{
				Echo = i;
				break;
			}
		}
		Fx2Check( Plugin.InitResult == AK_Success && Echo == SampleRate / 2 && Out[ 1 ][ Echo ] == 0.f, "Wwise Stereo Delay (135, not registered by the glue): default 0.5 s left echo",
			"Init %d, left echo at frame %u (expected %u), right %.3f", Plugin.InitResult, (unsigned)Echo, SampleRate / 2, Out.empty() ? 0.f : Out[ 1 ][ Echo ] );
	}
	{
		std::vector<unsigned char> NoBlock;
		AK::IAkPluginParam * Params = CreateHarmonizerFXParams( &Allocator );
		const AkUInt32 On = 1;
		Params->SetParam( 6, &On, sizeof( On ) );
		const float Octave = 1200.f, Silent = -96.f;
		Params->SetParam( 7, &Octave, sizeof( Octave ) );
		Params->SetParam( 3, &Silent, sizeof( Silent ) );
		AK::IAkInPlaceEffectPlugin * Effect = static_cast<AK::IAkInPlaceEffectPlugin *>( CreateHarmonizerFX( &Allocator ) );
		static FFx2Context Context;
		AkAudioFormat Format;
		Format.SetAll( SampleRate, AK_SPEAKER_SETUP_MONO, 32, 4, 1, 0 );
		const AKRESULT InitResult = Effect->Init( &Allocator, &Context, Params, Format );
		Effect->Reset();
		std::vector< std::vector<float> > Out;
		if( InitResult == AK_Success )
		{
			Fx2Run( Effect, AK_SPEAKER_SETUP_MONO, 2 * SampleRate, []( AkUInt32 Frame, AkUInt32 ) { return (float)( 0.5 * sin( 2.0 * 3.14159265358979 * 500.0 * Frame / SampleRate ) ); }, Out );
		}
		const double Hz = Out.empty() ? 0.0 : Fx2PeakFrequency( Out[ 0 ], SampleRate / 2, 16384, 600.0, 1400.0 );
		Fx2Check( InitResult == AK_Success && fabs( Hz - 1000.0 ) < 10.0, "Wwise Harmonizer (139, not registered by the glue): voice 1 at +1200 cents on a 500 Hz sine",
			"Init %d, %.2f Hz (expected 1000)", InitResult, Hz );
		Effect->Term( &Allocator );
		Params->Term( &Allocator );
	}
	Fx2Check( Allocator.Outstanding == 0, "Wwise Time Stretch / Stereo Delay / Harmonizer: every allocation released at Term", "%d outstanding", Allocator.Outstanding );
}
}

void TestWwiseFxPlugins2( WwiseReportFunc InReport, WwiseSkipFunc InSkip )
{
	Fx2Report = InReport;
	const std::vector<unsigned char> Bank = Fx2ReadFile( std::wstring( Fx2CookedDir ) + L"Init.bnk" );
	if( Bank.empty() )
	{
		InSkip( "Wwise effect plug-ins (Delay, Matrix Reverb, Tremolo, Pitch Shifter)", "retail Init.bnk not found" );
		return;
	}
	TestDelay( Bank );
	TestTremolo( Bank );
	TestPitchShifter( Bank );
	TestMatrixReverb( Bank );
	TestUnusedPlugins();
}

#endif
