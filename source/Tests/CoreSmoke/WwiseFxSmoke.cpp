// WwiseFxSmoke: agent IH4 fork F1's checks of the ported effect plug-ins and the effect paths of the lower engine,
// called from TestWwise after TestGraph with Init.bnk loaded and the sound engine running.
//   (a) RoomVerb: an impulse through CAkRoomVerbFX built from Init.bnk's own share set 0x23ea221c (decay 1.8 s, HF damping
//       2.7, dry 0 dB, ER -23 dB, reverb -23 dB, pattern 23 "Large Room", room size 15, reverb delay 35 ms, 12 units; read
//       out of the bank file here, cross-checked with build/agentIH4/fk_F1_tools/fx_params.py), compared with references
//       derived from those parameters: the dry level, the first reflection's time and level, the 250 Hz decay time.
//   (b) ParametricEQ: steady sines through CAkParametricEQFX against the RBJ magnitude computed here from the band.
//   (c) The engine: the voice line 0x57447bbe (mixing bus 0x28eb7999, Init.bnk's environmental bus) with the game object
//       sending to the RoomVerb environment produces a tail after the dry media ends; without the send it does not.
#if DISHONORED_WITH_WWISE

#define WIN32_LEAN_AND_MEAN
#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/SoundEngine/Common/AkCallback.h>
#include <AK/Plugin/AkRoomVerbFXFactory.h>
#include <AK/Plugin/AkParametricEQFXFactory.h>
#include <AK/Plugin/AkSilenceSourceFactory.h>
#include <DishonoredWwiseSilent.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <malloc.h>

typedef void ( *WwiseReportFunc )( bool Passed, const char * Name, const char * Detail );
typedef void ( *WwiseSkipFunc )( const char * Name, const char * Reason );

namespace
{
const wchar_t * const FxCookedDir = L"D:\\RecompileDishonored\\Dishonored_Latest2026\\DishonoredGame\\CookedPCConsole\\";
const AkUniqueID RoomVerbShareSet = 0x23ea221c;
const AkUInt32 SampleRate = 48000;
const double Pi = 3.14159265358979323846;

class FTestAllocator : public AK::IAkPluginMemAlloc
{
public:
	virtual void * Malloc( size_t in_uSize ) { return _aligned_malloc( in_uSize, 16 ); }
	virtual void Free( void * in_pMemAddress ) { _aligned_free( in_pMemAddress ); }
};

class FTestEffectContext : public AK::IAkEffectPluginContext
{
public:
	explicit FTestEffectContext( bool in_bSend ) : bSend( in_bSend ) {}
	virtual bool IsSendModeEffect() const { return bSend; }
	virtual AK::IAkStreamMgr * GetStreamMgr() const { return NULL; }
	virtual AkPlayingID GetPlayingID() const { return 0; }
	virtual AkUInt16 GetMaxBufferLength() const { return 1024; }
	virtual AKRESULT PostMonitorData( void *, AkUInt32 ) { return AK_Fail; }
	virtual bool CanPostMonitorData() { return false; }
	virtual void GetPluginMedia( AkUInt32, AkUInt8 *& out_rpData, AkUInt32 & out_rDataSize ) { out_rpData = NULL; out_rDataSize = 0; }
	virtual AK::IAkProcessorFeatures * GetProcessorFeatures() { return NULL; }
	bool bSend;
};

std::vector<unsigned char> ReadFxFile( const std::wstring & Path )
{
	std::vector<unsigned char> Data;
	FILE * File = _wfopen( Path.c_str(), L"rb" );
	if( File )
	{
		fseek( File, 0, SEEK_END );
		Data.resize( ftell( File ) );
		fseek( File, 0, SEEK_SET );
		fread( Data.data(), 1, Data.size(), File );
		fclose( File );
	}
	return Data;
}

/** The parameter block of an FX share set / custom of a .bnk file: HIRC type 18 / 19 body = { id, plug-in id, size, block }. */
bool FindFxBlock( const std::vector<unsigned char> & Bank, AkUniqueID FxID, AkUInt32 & OutPluginID, std::vector<unsigned char> & OutBlock )
{
	size_t Pos = 0;
	while( Pos + 8 <= Bank.size() )
	{
		const unsigned Size = *(const unsigned *)&Bank[ Pos + 4 ];
		if( !memcmp( &Bank[ Pos ], "HIRC", 4 ) )
		{
			const unsigned Count = *(const unsigned *)&Bank[ Pos + 8 ];
			size_t p = Pos + 12;
			for( unsigned i = 0; i < Count; ++i )
			{
				const unsigned char Type = Bank[ p ];
				const unsigned ObjSize = *(const unsigned *)&Bank[ p + 1 ];
				const unsigned char * Body = &Bank[ p + 5 ];
				if( ( Type == 18 || Type == 19 ) && *(const unsigned *)Body == FxID )
				{
					OutPluginID = *(const unsigned *)( Body + 4 );
					const unsigned BlockSize = *(const unsigned *)( Body + 8 );
					OutBlock.assign( Body + 12, Body + 12 + BlockSize );
					return true;
				}
				p += 5 + ObjSize;
			}
		}
		Pos += 8 + Size;
	}
	return false;
}

double dB( double Linear )
{
	return Linear > 0.0 ? 20.0 * log10( Linear ) : -400.0;
}

/** One stereo float buffer of 1024 frames, channel 1 after channel 0 (the pipeline's deinterleaved layout). */
struct FStereoBuffer
{
	float Data[ 2 * 1024 ];
	AkAudioBuffer Buffer;

	FStereoBuffer()
	{
		memset( Data, 0, sizeof( Data ) );
		Buffer.pData = Data;
		Buffer.uChannelMask = AK_SPEAKER_SETUP_STEREO;
		Buffer.eState = AK_DataReady;
		Buffer.uMaxFrames = 1024;
		Buffer.uValidFrames = 1024;
	}
};

/** RBJ cookbook magnitude of a peaking / shelf band (shelf slope 1) at a frequency, computed here in double. */
double RbjMagnitudedB( int Type, double Fs, double F0, double GaindB, double Q, double F )
{
	const double A = pow( 10.0, GaindB / 40.0 );
	const double W0 = 2.0 * Pi * F0 / Fs;
	const double Cw = cos( W0 ), Sw = sin( W0 );
	double b0, b1, b2, a0, a1, a2;
	if( Type == 6 )
	{
		const double Alpha = Sw / ( 2.0 * Q );
		b0 = 1 + Alpha * A; b1 = -2 * Cw; b2 = 1 - Alpha * A; a0 = 1 + Alpha / A; a1 = -2 * Cw; a2 = 1 - Alpha / A;
	}
	else
	{
		const double TwoSqrtAAlpha = 2.0 * sqrt( A ) * ( Sw / 2.0 * sqrt( 2.0 ) );
		if( Type == 4 )
		{
			b0 = A * ( ( A + 1 ) - ( A - 1 ) * Cw + TwoSqrtAAlpha ); b1 = 2 * A * ( ( A - 1 ) - ( A + 1 ) * Cw ); b2 = A * ( ( A + 1 ) - ( A - 1 ) * Cw - TwoSqrtAAlpha );
			a0 = ( A + 1 ) + ( A - 1 ) * Cw + TwoSqrtAAlpha; a1 = -2 * ( ( A - 1 ) + ( A + 1 ) * Cw ); a2 = ( A + 1 ) + ( A - 1 ) * Cw - TwoSqrtAAlpha;
		}
		else
		{
			b0 = A * ( ( A + 1 ) + ( A - 1 ) * Cw + TwoSqrtAAlpha ); b1 = -2 * A * ( ( A - 1 ) + ( A + 1 ) * Cw ); b2 = A * ( ( A + 1 ) + ( A - 1 ) * Cw - TwoSqrtAAlpha );
			a0 = ( A + 1 ) - ( A - 1 ) * Cw + TwoSqrtAAlpha; a1 = 2 * ( ( A - 1 ) - ( A + 1 ) * Cw ); a2 = ( A + 1 ) - ( A - 1 ) * Cw - TwoSqrtAAlpha;
		}
	}
	const double W = 2.0 * Pi * F / Fs;
	const double NumRe = b0 + b1 * cos( W ) + b2 * cos( 2 * W ), NumIm = -( b1 * sin( W ) + b2 * sin( 2 * W ) );
	const double DenRe = a0 + a1 * cos( W ) + a2 * cos( 2 * W ), DenIm = -( a1 * sin( W ) + a2 * sin( 2 * W ) );
	return 10.0 * log10( ( NumRe * NumRe + NumIm * NumIm ) / ( DenRe * DenRe + DenIm * DenIm ) );
}

/** A 1 s sine through the EQ, the steady-state gain over its last half second. */
double EqSineGaindB( AK::IAkInPlaceEffectPlugin * Effect, double Frequency )
{
	Effect->Reset();
	double InEnergy = 0.0, OutEnergy = 0.0;
	AkUInt32 Frame = 0;
	for( int Block = 0; Block < 47; ++Block )
	{
		FStereoBuffer Buffer;
		for( int i = 0; i < 1024; ++i, ++Frame )
		{
			Buffer.Data[ i ] = Buffer.Data[ 1024 + i ] = (float)( 0.25 * sin( 2.0 * Pi * Frequency * Frame / SampleRate ) );
		}
		std::vector<float> In( Buffer.Data, Buffer.Data + 1024 );
		Effect->Execute( &Buffer.Buffer );
		if( Block >= 23 )
		{
			for( int i = 0; i < 1024; ++i )
			{
				InEnergy += (double)In[ i ] * In[ i ];
				OutEnergy += (double)Buffer.Data[ i ] * Buffer.Data[ i ];
			}
		}
	}
	return 10.0 * log10( OutEnergy / InEnergy );
}

WwiseReportFunc GFxReport = NULL;

void Report( bool Passed, const char * Name, const char * Detail )
{
	printf( "      %s\n", Detail );
	GFxReport( Passed, Name, Detail );
}

void TestRoomVerb( WwiseSkipFunc InSkip )
{
	const std::vector<unsigned char> Init = ReadFxFile( std::wstring( FxCookedDir ) + L"Init.bnk" );
	AkUInt32 PluginID = 0;
	std::vector<unsigned char> Block;
	if( !FindFxBlock( Init, RoomVerbShareSet, PluginID, Block ) )
	{
		InSkip( "Wwise RoomVerb", "Init.bnk share set 0x23ea221c not found" );
		return;
	}
	char Detail[ 512 ];
	FTestAllocator Allocator;
	FTestEffectContext Context( false );
	AK::IAkPluginParam * Params = CreateRoomVerbFXParams( &Allocator );
	AK::IAkInPlaceEffectPlugin * Reverb = static_cast<AK::IAkInPlaceEffectPlugin *>( CreateRoomVerbFX( &Allocator ) );
	GFxReport( PluginID == 0x760003 && Params && Reverb, "Wwise RoomVerb: plug-in 118 (0x760003) instance and parameters from CreateRoomVerbFX / CreateRoomVerbFXParams", NULL );
	GFxReport( Params->Init( &Allocator, Block.data(), (AkUInt32)Block.size() ) == AK_Success, "Wwise RoomVerb: SetParamsBlock of Init.bnk share set 0x23ea221c (186 bytes)", NULL );
	AkAudioFormat Format;
	Format.SetAll( SampleRate, AK_SPEAKER_SETUP_STEREO, 32, 8, 1 /* AK_FLOAT */, 1 /* AK_NONINTERLEAVED */ );
	GFxReport( Reverb->Init( &Allocator, &Context, Params, Format ) == AK_Success && Reverb->Reset() == AK_Success, "Wwise RoomVerb: Init + Reset on 48 kHz stereo", NULL );

	// The parameters as the block holds them (fx_params.py prints the same).
	float DecayTime, DryDB, ERDB, RoomSize, StereoWidth;
	unsigned Pattern;
	memcpy( &DecayTime, &Block[ 0 ], 4 );
	memcpy( &StereoWidth, &Block[ 12 ], 4 );
	memcpy( &DryDB, &Block[ 68 ], 4 );
	memcpy( &ERDB, &Block[ 72 ], 4 );
	memcpy( &Pattern, &Block[ 81 ], 4 );
	memcpy( &RoomSize, &Block[ 89 ], 4 );

	// An impulse on the left channel, then the tail (the input ends after the first buffer).
	const AkUInt32 NumBlocks = (AkUInt32)( ( DecayTime + 1.5 ) * SampleRate / 1024 );
	std::vector<float> Left, Right;
	for( AkUInt32 b = 0; b < NumBlocks; ++b )
	{
		FStereoBuffer Buffer;
		if( b == 0 )
		{
			Buffer.Data[ 0 ] = 1.f;
		}
		else
		{
			Buffer.Buffer.uValidFrames = 0;
			Buffer.Buffer.eState = AK_NoMoreData;
		}
		Reverb->Execute( &Buffer.Buffer );
		Left.insert( Left.end(), Buffer.Data, Buffer.Data + Buffer.Buffer.uValidFrames );
		Right.insert( Right.end(), Buffer.Data + Buffer.Buffer.uMaxFrames, Buffer.Data + Buffer.Buffer.uMaxFrames + Buffer.Buffer.uValidFrames );
	}

	// Dry: sample 0 is the input times the dry level (no reflection reaches the output before the ER delay).
	snprintf( Detail, sizeof( Detail ), "dry: output %.3f dB at the impulse, parameter %.3f dB", dB( fabs( Left[ 0 ] ) ), DryDB );
	Report( fabs( dB( fabs( Left[ 0 ] ) ) - DryDB ) < 0.1, "Wwise RoomVerb: dry level within 0.1 dB of the share set's", Detail );

	// First reflection: the pattern's earliest tap scaled by 2^(room size / 100) is the ER delay; it arrives with the ER
	// level times the constant-power mono sum of a left-only impulse (sqrt(1/2)) times the left output's stereo-width
	// mix of the two first taps (g1 = 0.7071 + 0.2929 width / 180 on the left tap, g2 = sqrt(1 - g1^2) on the right one).
	static const float LargeRoomTap0L[ 2 ] = { 27.75f, 0.3888793885707855f };
	static const float LargeRoomTap0R[ 2 ] = { 27.75f, 0.5332006812095642f };
	const double Scale = pow( 2.0, RoomSize * 0.0099999998f );
	const AkUInt32 ExpectedOnset = (AkUInt32)(AkInt64)( (double)SampleRate * (float)( LargeRoomTap0L[ 0 ] * (float)Scale ) * 0.001 );
	AkUInt32 Onset = 1;
	while( Onset < Left.size() && fabs( Left[ Onset ] ) < 1e-6f )
	{
		++Onset;
	}
	const double Gain1 = StereoWidth * 0.0055555557 * 0.29289401 + 0.70710599;
	const double Gain2 = 1.0 - Gain1 * Gain1 > 0.0 ? sqrt( 1.0 - Gain1 * Gain1 ) : 0.0;
	const double ExpectedER = pow( 10.0, ERDB / 20.0 ) * sqrt( 0.5 ) * ( LargeRoomTap0L[ 1 ] * Gain1 + LargeRoomTap0R[ 1 ] * Gain2 );
	snprintf( Detail, sizeof( Detail ), "pattern %u, room size %.0f, width %.0f: first reflection at sample %u (expected %u = 27.75 ms x %.4f), level %.3f dB (expected %.3f dB)",
		Pattern, RoomSize, StereoWidth, Onset, ExpectedOnset, Scale, dB( fabs( Left[ Onset ] ) ), dB( ExpectedER ) );
	Report( Pattern == 23 && Onset == ExpectedOnset && fabs( dB( fabs( Left[ Onset ] ) ) - dB( ExpectedER ) ) < 0.1, "Wwise RoomVerb: first reflection time and level from the ER pattern, room size and ER level", Detail );

	// Decay: 250 Hz band (RBJ band pass, Q 1.41) of the left output, Schroeder backward integration, slope fitted between
	// -5 and -25 dB, extrapolated to 60 dB. Each FDN line's loop gain is 10^(-3 L / (T fs)) at DC and its damping pole
	// changes it by under 0.05 dB per pass at 250 Hz, so the band decays at the share set's decay time; the tolerance
	// (10 %) covers the modal fluctuation of a 48-line network over a 20 dB fit range.
	{
		const double W0 = 2.0 * Pi * 250.0 / SampleRate, Alpha = sin( W0 ) / ( 2.0 * 1.41 );
		const double b0 = Alpha, b2 = -Alpha, a0 = 1 + Alpha, a1 = -2 * cos( W0 ), a2 = 1 - Alpha;
		std::vector<double> Band( Left.size() );
		double x1 = 0, x2 = 0, y1 = 0, y2 = 0;
		for( size_t i = 0; i < Left.size(); ++i )
		{
			const double x = i ? Left[ i ] : 0.0;
			const double y = ( b0 * x + b2 * x2 - a1 * y1 - a2 * y2 ) / a0;
			x2 = x1; x1 = x; y2 = y1; y1 = y;
			Band[ i ] = y;
		}
		std::vector<double> Edc( Band.size() + 1, 0.0 );
		for( size_t i = Band.size(); i-- > 0; )
		{
			Edc[ i ] = Edc[ i + 1 ] + Band[ i ] * Band[ i ];
		}
		size_t Start = 0, End = 0;
		for( size_t i = 0; i < Band.size(); ++i )
		{
			const double Level = 10.0 * log10( Edc[ i ] / Edc[ 0 ] );
			if( !Start && Level <= -5.0 )
			{
				Start = i;
			}
			if( Level <= -25.0 )
			{
				End = i;
				break;
			}
		}
		double Sx = 0, Sy = 0, Sxx = 0, Sxy = 0;
		const double N = (double)( End - Start + 1 );
		for( size_t i = Start; i <= End && End; ++i )
		{
			const double t = (double)i / SampleRate, Level = 10.0 * log10( Edc[ i ] / Edc[ 0 ] );
			Sx += t; Sy += Level; Sxx += t * t; Sxy += t * Level;
		}
		const double Slope = ( N * Sxy - Sx * Sy ) / ( N * Sxx - Sx * Sx );
		const double RT60 = Slope < 0.0 ? -60.0 / Slope : 0.0;
		snprintf( Detail, sizeof( Detail ), "250 Hz decay: T20-fitted RT60 %.3f s, share set DecayTime %.3f s (%.1f %%)", RT60, DecayTime, 100.0 * ( RT60 / DecayTime - 1.0 ) );
		Report( End && fabs( RT60 / DecayTime - 1.0 ) < 0.10, "Wwise RoomVerb: the low band decays at the share set's decay time (within 10 %)", Detail );
	}
	// The tail handler keeps the effect producing for (reverb delay + decay time) after the input ended.
	double LateEnergy = 0.0;
	for( size_t i = Left.size() - 4800; i < Left.size(); ++i )
	{
		LateEnergy += (double)Left[ i ] * Left[ i ];
	}
	snprintf( Detail, sizeof( Detail ), "%zu frames produced after a 1024-frame input (%.2f s), last 0.1 s at %.1f dB rms", Left.size(), (double)Left.size() / SampleRate, dB( sqrt( LateEnergy / 4800 ) ) );
	Report( Left.size() > (size_t)( DecayTime * SampleRate ), "Wwise RoomVerb: AkFXTailHandler extends the output past the input by the tail length", Detail );
	Reverb->Term( &Allocator );
	Params->Term( &Allocator );
}

void TestParametricEQ()
{
	char Detail[ 512 ];
	FTestAllocator Allocator;
	FTestEffectContext Context( false );
	AK::IAkPluginParam * Params = CreateParametricEQFXParams( &Allocator );
	AK::IAkInPlaceEffectPlugin * Eq = static_cast<AK::IAkInPlaceEffectPlugin *>( CreateParametricEQFX( &Allocator ) );
	Params->Init( &Allocator, NULL, 0 );
	// Band 1 low shelf 200 Hz +9 dB, band 2 peaking 1 kHz +6 dB Q 2, band 3 off; output 0 dB (the SetParam ids 0..16).
	const float LowShelf = 4.f, ShelfFreq = 200.f, ShelfGain = 9.f;
	const float Peaking = 6.f, PeakFreq = 1000.f, PeakGain = 6.f, PeakQ = 2.f, Off = 0.f, Zero = 0.f;
	Params->SetParam( 0, &LowShelf, 4 );
	Params->SetParam( 1, &ShelfGain, 4 );
	Params->SetParam( 2, &ShelfFreq, 4 );
	Params->SetParam( 5, &Peaking, 4 );
	Params->SetParam( 6, &PeakGain, 4 );
	Params->SetParam( 7, &PeakFreq, 4 );
	Params->SetParam( 8, &PeakQ, 4 );
	Params->SetParam( 14, &Off, 4 );
	Params->SetParam( 15, &Zero, 4 );
	AkAudioFormat Format;
	Format.SetAll( SampleRate, AK_SPEAKER_SETUP_STEREO, 32, 8, 1 /* AK_FLOAT */, 1 /* AK_NONINTERLEAVED */ );
	GFxReport( Eq->Init( &Allocator, &Context, Params, Format ) == AK_Success, "Wwise ParametricEQ: CreateParametricEQFX + Init on 48 kHz stereo", NULL );
	const double Centre = EqSineGaindB( Eq, PeakFreq );
	const double ExpectedCentre = RbjMagnitudedB( 6, SampleRate, PeakFreq, PeakGain, PeakQ, PeakFreq ) + RbjMagnitudedB( 4, SampleRate, ShelfFreq, ShelfGain, 5.0, PeakFreq );
	snprintf( Detail, sizeof( Detail ), "1 kHz (peaking band centre, +6 dB): measured %.3f dB, bands %.3f dB (peak %.3f + shelf skirt %.3f)", Centre, ExpectedCentre,
		RbjMagnitudedB( 6, SampleRate, PeakFreq, PeakGain, PeakQ, PeakFreq ), RbjMagnitudedB( 4, SampleRate, ShelfFreq, ShelfGain, 5.0, PeakFreq ) );
	Report( fabs( Centre - ExpectedCentre ) < 0.1 && fabs( RbjMagnitudedB( 6, SampleRate, PeakFreq, PeakGain, PeakQ, PeakFreq ) - PeakGain ) < 0.01, "Wwise ParametricEQ: gain at the peaking band centre within 0.1 dB", Detail );
	const double Low = EqSineGaindB( Eq, 40.0 );
	const double ExpectedLow = RbjMagnitudedB( 4, SampleRate, ShelfFreq, ShelfGain, 5.0, 40.0 ) + RbjMagnitudedB( 6, SampleRate, PeakFreq, PeakGain, PeakQ, 40.0 );
	snprintf( Detail, sizeof( Detail ), "40 Hz (low shelf 200 Hz +9 dB): measured %.3f dB, RBJ %.3f dB", Low, ExpectedLow );
	Report( fabs( Low - ExpectedLow ) < 0.1, "Wwise ParametricEQ: low-shelf gain below its corner within 0.1 dB", Detail );
	const double Edge = EqSineGaindB( Eq, 1250.0 );
	const double ExpectedEdge = RbjMagnitudedB( 6, SampleRate, PeakFreq, PeakGain, PeakQ, 1250.0 ) + RbjMagnitudedB( 4, SampleRate, ShelfFreq, ShelfGain, 5.0, 1250.0 );
	snprintf( Detail, sizeof( Detail ), "1.25 kHz (peaking skirt): measured %.3f dB, RBJ %.3f dB", Edge, ExpectedEdge );
	Report( fabs( Edge - ExpectedEdge ) < 0.1, "Wwise ParametricEQ: the peaking band's skirt follows its Q within 0.1 dB", Detail );
	Eq->Term( &Allocator );
	Params->Term( &Allocator );
}

struct FFxCallbacks
{
	volatile LONG EndOfEvent = 0;
};

FFxCallbacks GFxCallbacks;

void FxCallback( AkCallbackType in_eType, AkCallbackInfo * in_pInfo )
{
	if( in_eType == AK_EndOfEvent )
	{
		InterlockedIncrement( &GFxCallbacks.EndOfEvent );
	}
}

/** The voice line played through the sink with -akwav's capture; the 16-bit capture's rms over [from, to) seconds after
    the post-time first audible frame. */
struct FTailCapture
{
	double DryRms = 0.0;
	double TailRms = 0.0;
	bool Ended = false;
};

FTailCapture PlayVoiceForTail( AkGameObjectID GameObj, const wchar_t * Name, double MediaSeconds )
{
	FTailCapture Out;
	wchar_t WavPath[ MAX_PATH ];
	const DWORD ExeLength = GetModuleFileNameW( NULL, WavPath, MAX_PATH );
	std::wstring WavName( WavPath, ExeLength );
	WavName = WavName.substr( 0, WavName.find_last_of( L'\\' ) + 1 ) + Name;
	GFxCallbacks.EndOfEvent = 0;
	DishonoredWwise::SetWavCapture( WavName.c_str() );
	const AkPlayingID PlayingID = AK::SoundEngine::PostEvent( 0x57447bbe, GameObj, AK_EndOfEvent, FxCallback, NULL );
	for( int i = 0; PlayingID != AK_INVALID_PLAYING_ID && i < 1000 && !GFxCallbacks.EndOfEvent; ++i )
	{
		AK::SoundEngine::RenderAudio();
		Sleep( 10 );
	}
	Out.Ended = GFxCallbacks.EndOfEvent != 0;
	for( int i = 0; i < 150; ++i )
	{
		AK::SoundEngine::RenderAudio();
		Sleep( 10 );
	}
	DishonoredWwise::SetWavCapture( NULL );
	const std::vector<unsigned char> Wav = ReadFxFile( WavName );
	if( Wav.size() <= 44 )
	{
		return Out;
	}
	const unsigned Channels = *(const unsigned short *)( Wav.data() + 22 );
	const unsigned Rate = *(const unsigned *)( Wav.data() + 24 );
	const short * Samples = (const short *)( Wav.data() + 44 );
	const size_t Frames = ( Wav.size() - 44 ) / ( 2 * Channels );
	size_t First = 0;
	while( First < Frames && abs( Samples[ First * Channels ] ) <= 1 && abs( Samples[ First * Channels + Channels - 1 ] ) <= 1 )
	{
		++First;
	}
	const size_t DryEnd = First + (size_t)( MediaSeconds * Rate );
	const size_t TailStart = DryEnd + Rate / 20;
	const size_t TailEnd = TailStart + Rate / 2 < Frames ? TailStart + Rate / 2 : Frames;
	double DryEnergy = 0.0, TailEnergy = 0.0;
	for( size_t f = First; f < DryEnd && f < Frames; ++f )
	{
		for( unsigned c = 0; c < Channels; ++c )
		{
			DryEnergy += (double)Samples[ f * Channels + c ] * Samples[ f * Channels + c ];
		}
	}
	for( size_t f = TailStart; f < TailEnd; ++f )
	{
		for( unsigned c = 0; c < Channels; ++c )
		{
			TailEnergy += (double)Samples[ f * Channels + c ] * Samples[ f * Channels + c ];
		}
	}
	Out.DryRms = DryEnd > First ? sqrt( DryEnergy / ( (double)( DryEnd - First ) * Channels ) ) : 0.0;
	Out.TailRms = TailEnd > TailStart ? sqrt( TailEnergy / ( (double)( TailEnd - TailStart ) * Channels ) ) : 0.0;
	return Out;
}

void TestEnvironmentSend( WwiseSkipFunc InSkip )
{
	const std::vector<unsigned char> Pck = ReadFxFile( std::wstring( FxCookedDir ) + L"Bk_E43C0E7F49D6DA8943110CA3F7328499.INT" );
	// The AKPK bank LUT: the voice bank 0x87628435 (same lookup as WwiseSmoke.cpp's FindPackagedBank).
	size_t Offset = 0, Size = 0;
	if( Pck.size() > 28 && !memcmp( Pck.data(), "AKPK", 4 ) )
	{
		const unsigned LangMapSize = *(const unsigned *)( Pck.data() + 12 );
		const unsigned char * Lut = Pck.data() + 28 + LangMapSize;
		const unsigned Num = *(const unsigned *)Lut;
		for( unsigned i = 0; i < Num; ++i )
		{
			const unsigned * Entry = (const unsigned *)( Lut + 4 + 20 * i );
			if( Entry[ 0 ] == 0x87628435 )
			{
				Offset = (size_t)Entry[ 3 ] * Entry[ 1 ];
				Size = Entry[ 2 ];
			}
		}
	}
	if( !Size )
	{
		InSkip( "Wwise environment send", "voice bank 0x87628435 not found" );
		return;
	}
	void * Memory = _aligned_malloc( Size, 16 );
	memcpy( Memory, Pck.data() + Offset, Size );
	AkBankID BankID = 0;
	GFxReport( AK::SoundEngine::LoadBank( Memory, (AkUInt32)Size, BankID ) == AK_Success, "Wwise environment send: LoadBank(voice bank 0x87628435)", NULL );
	const AkGameObjectID GameObj = 102;
	AK::SoundEngine::RegisterGameObj( GameObj, "IH4 smoke" );
	AkSoundPosition Position;
	memset( &Position, 0, sizeof( Position ) );
	Position.Position.Z = 1.f;
	Position.Orientation.Z = -1.f;
	AK::SoundEngine::SetPosition( GameObj, Position );
	const double MediaSeconds = 117526.0 / 44100.0;
	const FTailCapture Dry = PlayVoiceForTail( GameObj, L"ih4_env_dry.wav", MediaSeconds );
	AkEnvironmentValue Send;
	Send.EnvID = RoomVerbShareSet;
	Send.fControlValue = 1.f;
	AK::SoundEngine::SetGameObjectEnvironmentsValues( GameObj, &Send, 1 );
	for( int i = 0; i < 10; ++i )
	{
		AK::SoundEngine::RenderAudio();
		Sleep( 10 );
	}
	const FTailCapture Wet = PlayVoiceForTail( GameObj, L"ih4_env_wet.wav", MediaSeconds );
	AK::SoundEngine::SetGameObjectEnvironmentsValues( GameObj, NULL, 0 );
	char Detail[ 512 ];
	snprintf( Detail, sizeof( Detail ), "voice 0x57447bbe dry play: rms %.1f during the line, %.2f in 0.05..0.55 s after it; with a send to RoomVerb 0x23ea221c at 1.0: rms %.1f, tail %.1f (%.1f dB under the line)",
		Dry.DryRms, Dry.TailRms, Wet.DryRms, Wet.TailRms, dB( Wet.TailRms / ( Wet.DryRms > 0 ? Wet.DryRms : 1 ) ) );
	Report( Dry.Ended && Wet.Ended && Dry.TailRms < 2.0 && Wet.TailRms > 8.0 && Wet.TailRms > 4.0 * ( Dry.TailRms + 0.5 ),
		"Wwise environment send: the voice's aux send to a RoomVerb environment leaves a reverb tail after the line ends", Detail );
	AK::SoundEngine::UnregisterGameObj( GameObj );
	for( int i = 0; i < 20; ++i )
	{
		AK::SoundEngine::RenderAudio();
		Sleep( 10 );
	}
	AK::SoundEngine::UnloadBank( BankID, NULL );
	_aligned_free( Memory );
}
}

/** The registrations AkAudio's UAkAudioDevice::EnsureInitialized makes (2013 0x5b1430) for the plug-ins fork F1 ported;
    made before Init.bnk loads so its FX share sets get their parameter objects. */
void RegisterWwiseFxPlugins()
{
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, 0, 105, CreateParametricEQFX, CreateParametricEQFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, 0, 118, CreateRoomVerbFX, CreateRoomVerbFXParams );
	AK::SoundEngine::RegisterPlugin( AkPluginTypeSource, 0, 101, CreateSilenceSource, CreateSilenceSourceParams );
}

void TestWwiseFx( WwiseReportFunc InReport, WwiseSkipFunc InSkip )
{
	GFxReport = InReport;
	TestRoomVerb( InSkip );
	TestParametricEQ();
	TestEnvironmentSend( InSkip );
}

#endif
