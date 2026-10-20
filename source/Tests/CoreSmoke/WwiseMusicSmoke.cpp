// WwiseMusicSmoke: agent IH4 fork M's checks of the ported interactive music engine (wave 26), called from TestWwise after
// TestGraph with Init.bnk loaded and the sound engine running. The music engine stays initialized for the in-memory sweep
// of every packaged bank that follows (so HIRC types 10..13 are created for real there) and is terminated by
// TermWwiseMusic before AK::SoundEngine::Term.
//
// Expected values come from build/agentIH4/fk_M_tools/music_parse.py (an independent python reader of the music HIRC
// objects, 2461 objects in every package read to their section end):
//   Bank_Music_Tower_Intro.pck (bank 0x0dc219e2): event 0xb970f036 stops 0x3ffbf5b5 and plays segment 0x2bbc989d,
//     duration 6025.714286 ms (entry 0, exit 6025.714 ms), meter 120 bpm 4/4 (own), one track 0x31a25401 whose clip plays
//     streamed file 0x0baad2fc (265734 frames at 44.1 kHz) from 0 for 6025.714 ms.
//   Bank_Music_Streets.pck (bank 0xeea32b06): event 0x87084726 plays music switch 0xc5cc052 (state group 0x9cb5d15f,
//     meter 134 bpm 4/4 for all its children). State 0xa4ad5e0e selects sequence 0x24aed7c5 (segment 0x16595496, 12 bars);
//     state 0xd9f6455e selects 0x3002b749 and the rule 0x24aed7c5 -> 0x3002b749 syncs to the next bar (sync type 2) with a
//     transition segment 0x2e961c9d. A bar is 4 * 60 / 134 s.
// Streamed music media is opened inside its package: the test's resolver answers Open(file id) with the package file and
// the file's offset as uSector (the smoke device's block size is 1).
#if DISHONORED_WITH_WWISE

#define WIN32_LEAN_AND_MEAN
#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/SoundEngine/Common/AkCallback.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <DishonoredWwiseSilent.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <map>
#include <string>
#include <vector>

namespace AkAudioLibSettings
{
	extern AkUInt32 g_pipelineCoreFrequency;
}

typedef void ( *WwiseReportFunc )( bool Passed, const char * Name, const char * Detail );
typedef void ( *WwiseSkipFunc )( const char * Name, const char * Reason );

namespace
{
const wchar_t * const MusicCookedDir = L"D:\\RecompileDishonored\\Dishonored_Latest2026\\DishonoredGame\\CookedPCConsole\\";

std::vector<unsigned char> ReadMusicFile( const std::wstring & Path )
{
	std::vector<unsigned char> Bytes;
	FILE * File = _wfopen( Path.c_str(), L"rb" );
	if( !File )
	{
		return Bytes;
	}
	fseek( File, 0, SEEK_END );
	const long Size = ftell( File );
	fseek( File, 0, SEEK_SET );
	Bytes.resize( Size > 0 ? (size_t)Size : 0 );
	if( Size > 0 )
	{
		fread( Bytes.data(), 1, (size_t)Size, File );
	}
	fclose( File );
	return Bytes;
}

struct FPackagedFile
{
	std::wstring Package;
	AkUInt32 Offset = 0;
	AkUInt32 Size = 0;
};

/** The AKPK header (tag, header size, version, language map, bank LUT, streamed file LUT, externals LUT sizes), then the
    maps in that order; LUT entries are { id, block size, size, start block, language }. */
struct FPackage
{
	std::wstring Name;
	std::vector<unsigned char> Bytes;
	std::map<AkUInt32, std::pair<size_t, size_t>> Banks;
	std::map<AkUInt32, std::pair<size_t, size_t>> Files;

	bool Load( const wchar_t * InName )
	{
		Name = InName;
		Bytes = ReadMusicFile( std::wstring( MusicCookedDir ) + InName );
		if( Bytes.size() < 28 || memcmp( Bytes.data(), "AKPK", 4 ) != 0 )
		{
			return false;
		}
		const AkUInt32 * Header = (const AkUInt32 *)Bytes.data();
		ReadLut( 28 + Header[ 3 ], Banks );
		ReadLut( 28 + Header[ 3 ] + Header[ 4 ], Files );
		return true;
	}

	void ReadLut( size_t At, std::map<AkUInt32, std::pair<size_t, size_t>> & Out )
	{
		const AkUInt32 Count = *(const AkUInt32 *)( Bytes.data() + At );
		for( AkUInt32 i = 0; i < Count; ++i )
		{
			const AkUInt32 * Entry = (const AkUInt32 *)( Bytes.data() + At + 4 + 20 * i );
			Out[ Entry[ 0 ] ] = std::make_pair( (size_t)Entry[ 3 ] * ( Entry[ 1 ] ? Entry[ 1 ] : 1 ), (size_t)Entry[ 2 ] );
		}
	}
};

class FMusicResolver : public AK::StreamMgr::IAkFileLocationResolver
{
public:
	AK::StreamMgr::IAkFileLocationResolver * Previous = NULL;
	std::map<AkFileID, FPackagedFile> Files;
	volatile LONG StreamOpens = 0;

	virtual AKRESULT Open( AkFileID in_fileID, AkOpenMode in_eOpenMode, AkFileSystemFlags * in_pFlags, bool & io_bSyncOpen, AkFileDesc & out_fileDesc )
	{
		const std::map<AkFileID, FPackagedFile>::const_iterator It = Files.find( in_fileID );
		if( It == Files.end() )
		{
			return Previous->Open( in_fileID, in_eOpenMode, in_pFlags, io_bSyncOpen, out_fileDesc );
		}
		const AKRESULT Result = Previous->Open( It->second.Package.c_str(), in_eOpenMode, in_pFlags, io_bSyncOpen, out_fileDesc );
		if( Result == AK_Success )
		{
			out_fileDesc.uSector = It->second.Offset;
			out_fileDesc.iFileSize = It->second.Size;
			InterlockedIncrement( &StreamOpens );
		}
		return Result;
	}

	virtual AKRESULT Open( const AkOSChar * in_pszFileName, AkOpenMode in_eOpenMode, AkFileSystemFlags * in_pFlags, bool & io_bSyncOpen, AkFileDesc & out_fileDesc )
	{
		return Previous->Open( in_pszFileName, in_eOpenMode, in_pFlags, io_bSyncOpen, out_fileDesc );
	}
};

FMusicResolver GResolver;
bool GMusicInit = false;

/** Audio frames rendered, counted by a global callback that runs after the music renderer's behavioural extension. */
volatile LONG GFrames = 0;

void FrameTick( bool )
{
	InterlockedIncrement( &GFrames );
}

struct FMusicEvents
{
	volatile LONG Entries = 0;
	volatile LONG Exits = 0;
	volatile LONG Bars = 0;
	volatile LONG Started = 0;
	volatile LONG Ended = 0;
	LONG EntryFrames[ 16 ] = {};
	LONG ExitFrames[ 16 ] = {};
	LONG BarFrames[ 64 ] = {};
	LONG EndFrame = 0;
	AkReal32 BarDuration = 0.f;
	AkReal32 BeatDuration = 0.f;
};

FMusicEvents GMusic;

void MusicCallback( AkCallbackType in_eType, AkCallbackInfo * in_pInfo )
{
	const LONG Frame = GFrames;
	switch( in_eType )
	{
	case AK_MusicSyncEntry:
	{
		const LONG i = InterlockedIncrement( &GMusic.Entries ) - 1;
		if( i < 16 )
		{
			GMusic.EntryFrames[ i ] = Frame;
		}
		break;
	}
	case AK_MusicSyncExit:
	{
		const LONG i = InterlockedIncrement( &GMusic.Exits ) - 1;
		if( i < 16 )
		{
			GMusic.ExitFrames[ i ] = Frame;
		}
		break;
	}
	case AK_MusicSyncBar:
	{
		const LONG i = InterlockedIncrement( &GMusic.Bars ) - 1;
		if( i < 64 )
		{
			GMusic.BarFrames[ i ] = Frame;
		}
		GMusic.BarDuration = ( (AkMusicSyncCallbackInfo *)in_pInfo )->fBarDuration;
		GMusic.BeatDuration = ( (AkMusicSyncCallbackInfo *)in_pInfo )->fBeatDuration;
		break;
	}
	case AK_MusicPlayStarted:
		InterlockedIncrement( &GMusic.Started );
		break;
	case AK_EndOfEvent:
		GMusic.EndFrame = Frame;
		InterlockedIncrement( &GMusic.Ended );
		break;
	default:
		break;
	}
}

void MusicSettle( int Milliseconds )
{
	for( int Waited = 0; Waited < Milliseconds; Waited += 10 )
	{
		AK::SoundEngine::RenderAudio();
		Sleep( 10 );
	}
}

template <class TCondition>
bool MusicWaitFor( int Milliseconds, TCondition Condition )
{
	for( int Waited = 0; Waited < Milliseconds; Waited += 10 )
	{
		if( Condition() )
		{
			return true;
		}
		AK::SoundEngine::RenderAudio();
		Sleep( 10 );
	}
	return Condition();
}

void ResetMusicEvents()
{
	memset( (void *)&GMusic, 0, sizeof( GMusic ) );
}

void * LoadPackagedBank( const FPackage & Package, AkUInt32 BankID, AkBankID & OutBankID, AKRESULT & OutResult )
{
	const std::map<AkUInt32, std::pair<size_t, size_t>>::const_iterator It = Package.Banks.find( BankID );
	if( It == Package.Banks.end() )
	{
		OutResult = AK_FileNotFound;
		return NULL;
	}
	void * Memory = _aligned_malloc( It->second.second, 16 );
	memcpy( Memory, Package.Bytes.data() + It->second.first, It->second.second );
	OutResult = AK::SoundEngine::LoadBank( Memory, (AkUInt32)It->second.second, OutBankID );
	return Memory;
}

void RegisterStreamedFiles( const FPackage & Package )
{
	for( const auto & File : Package.Files )
	{
		FPackagedFile & Entry = GResolver.Files[ File.first ];
		Entry.Package = Package.Name;
		Entry.Offset = (AkUInt32)File.second.first;
		Entry.Size = (AkUInt32)File.second.second;
	}
}

std::wstring NextToExe( const wchar_t * Name )
{
	wchar_t Path[ MAX_PATH ];
	const DWORD Length = GetModuleFileNameW( NULL, Path, MAX_PATH );
	std::wstring Out( Path, Length );
	return Out.substr( 0, Out.find_last_of( L'\\' ) + 1 ) + Name;
}

void MusicLog( const AkOSChar * in_pszMessage )
{
	wprintf( L"      %s\n", in_pszMessage );
}
}

void TestWwiseMusic( WwiseReportFunc InReport, WwiseSkipFunc InSkip )
{
	char Detail[ 512 ];
	if( getenv( "WWISE_SMOKE_DIAG" ) )
	{
		DishonoredWwise::SetLogHook( MusicLog );
		DishonoredWwise::SetDiagnostics( true );
	}
	AkMusicSettings Settings;
	AK::MusicEngine::GetDefaultInitSettings( Settings );
	GMusicInit = AK::MusicEngine::Init( &Settings ) == AK_Success;
	InReport( GMusicInit, "Wwise music: AK::MusicEngine::Init (renderer, behavioural extension, state and bank handlers)", NULL );
	if( !GMusicInit )
	{
		return;
	}
	AK::SoundEngine::RegisterGlobalCallback( FrameTick );
	GResolver.Previous = AK::StreamMgr::GetFileLocationResolver();
	AK::StreamMgr::SetFileLocationResolver( &GResolver );

	const double Rate = (double)AkAudioLibSettings::g_pipelineCoreFrequency;
	const double Frame = 1024.0;
	const AkGameObjectID GameObj = 300;
	AK::SoundEngine::RegisterGameObj( GameObj, "IH4 music smoke" );

	// 1. Bank_Music_Tower_Intro: event 0xb970f036 plays segment 0x2bbc989d (6025.714286 ms, 120 bpm 4/4).
	FPackage TowerIntro;
	if( !TowerIntro.Load( L"Bank_Music_Tower_Intro.pck" ) )
	{
		InSkip( "Wwise music: a bank segment plays its track", "Bank_Music_Tower_Intro.pck not found" );
	}
	else
	{
		RegisterStreamedFiles( TowerIntro );
		AkBankID BankID = 0;
		AKRESULT Result = AK_Fail;
		void * Memory = LoadPackagedBank( TowerIntro, 0x0dc219e2, BankID, Result );
		snprintf( Detail, sizeof( Detail ), "result %d", (int)Result );
		InReport( Result == AK_Success, "Wwise music: LoadBank(Bank_Music_Tower_Intro 0x0dc219e2) creates its segments, tracks and containers", Detail );

		const double SegmentMs = 6025.714285714286;
		const double ExpectedSamples = SegmentMs * Rate / 1000.0;
		const std::wstring WavName = NextToExe( L"ih4_music_segment.wav" );
		ResetMusicEvents();
		DishonoredWwise::SetWavCapture( WavName.c_str() );
		const LONG StreamOpensBefore = GResolver.StreamOpens;
		const AkPlayingID PlayingID = AK::SoundEngine::PostEvent( 0xb970f036, GameObj, AK_MusicSyncEntry | AK_MusicSyncExit | AK_MusicSyncBar | AK_MusicPlayStarted | AK_EndOfEvent, MusicCallback, NULL );
		InReport( PlayingID != AK_INVALID_PLAYING_ID, "Wwise music: PostEvent(0xb970f036, plays segment 0x2bbc989d) returns a playing id", NULL );
		const bool Ended = MusicWaitFor( 12000, [] { return GMusic.Ended != 0; } );
		MusicSettle( 200 );
		DishonoredWwise::SetWavCapture( NULL );

		snprintf( Detail, sizeof( Detail ), "play started %ld, entries %ld, exits %ld, end of event %ld, streamed opens %ld", GMusic.Started, GMusic.Entries, GMusic.Exits, GMusic.Ended,
			GResolver.StreamOpens - StreamOpensBefore );
		printf( "      %s\n", Detail );
		InReport( Ended && GMusic.Started == 1 && GMusic.Entries == 1 && GResolver.StreamOpens > StreamOpensBefore,
			"Wwise music: AK_MusicPlayStarted, one entry, AK_EndOfEvent; the track streams its media from the package", Detail );

		const double EntryToEnd = ( GMusic.EndFrame - GMusic.EntryFrames[ 0 ] ) * Frame;
		snprintf( Detail, sizeof( Detail ), "entry -> end of event %.0f samples (%ld frames), bank segment %.0f samples (%.3f ms at %.0f Hz), difference %.0f samples", EntryToEnd,
			GMusic.EndFrame - GMusic.EntryFrames[ 0 ], ExpectedSamples, SegmentMs, Rate, EntryToEnd - ExpectedSamples );
		printf( "      %s\n", Detail );
		InReport( EntryToEnd >= ExpectedSamples - Frame && EntryToEnd <= ExpectedSamples + 2 * Frame,
			"Wwise music: the segment plays from its entry to its exit cue (bank duration within one audio frame; the end of event follows in the next frame)", Detail );

		const std::vector<unsigned char> Wav = ReadMusicFile( WavName );
		if( Wav.size() <= 44 )
		{
			InReport( false, "Wwise music: the segment's track reaches the sink", "ih4_music_segment.wav not captured" );
		}
		else
		{
			const unsigned Channels = *(const unsigned short *)( Wav.data() + 22 );
			const unsigned WavRate = *(const unsigned *)( Wav.data() + 24 );
			const short * Samples = (const short *)( Wav.data() + 44 );
			const size_t NumFrames = ( Wav.size() - 44 ) / ( 2 * Channels );
			size_t First = NumFrames, Last = 0;
			int Peak = 0;
			for( size_t f = 0; f < NumFrames; ++f )
			{
				for( unsigned c = 0; c < Channels; ++c )
				{
					const int Value = abs( (int)Samples[ f * Channels + c ] );
					if( Value > 1 )
					{
						First = f < First ? f : First;
						Last = f;
					}
					Peak = Value > Peak ? Value : Peak;
				}
			}
			const double Span = Last >= First ? (double)( Last - First + 1 ) : 0.0;
			const double Expected = SegmentMs * WavRate / 1000.0;
			snprintf( Detail, sizeof( Detail ), "peak %d, audible span %.0f samples (%.4f s) at %u Hz, bank clip %.0f samples (%.4f s), difference %.0f samples", Peak, Span,
				Span / WavRate, WavRate, Expected, Expected / WavRate, Span - Expected );
			printf( "      %s\n", Detail );
			InReport( Peak > 500, "Wwise music: the segment's track reaches the sink (not silent)", Detail );
			InReport( Span <= Expected + Frame && Span >= Expected - 0.7 * WavRate, "Wwise music: the audible span lies within the segment's clip (the asset decays to digital silence over its last 0.6 s)", Detail );
		}

		const int ExpectedBars = (int)floor( SegmentMs / 2000.0 ) + 1;
		snprintf( Detail, sizeof( Detail ), "%ld AK_MusicSyncBar, bank meter 120 bpm 4/4 over %.3f ms: %d bars; bar duration %.4f s (expected 2.0000)", GMusic.Bars, SegmentMs, ExpectedBars, GMusic.BarDuration );
		printf( "      %s\n", Detail );
		InReport( GMusic.Bars == ExpectedBars && fabs( GMusic.BarDuration - 2.0 ) < 1e-4, "Wwise music: AK_MusicSyncBar once per bar of the bank's meter", Detail );

		AK::SoundEngine::UnloadBank( BankID, NULL );
		_aligned_free( Memory );
	}

	// 2. Bank_Music_Streets: switch 0xc5cc052 on state group 0x9cb5d15f; 0x24aed7c5 -> 0x3002b749 at the next bar (134 bpm 4/4).
	FPackage Streets;
	if( !Streets.Load( L"Bank_Music_Streets.pck" ) )
	{
		InSkip( "Wwise music: a state change transitions at the next bar", "Bank_Music_Streets.pck not found" );
	}
	else
	{
		RegisterStreamedFiles( Streets );
		AkBankID BankID = 0;
		AKRESULT Result = AK_Fail;
		void * Memory = LoadPackagedBank( Streets, 0xeea32b06, BankID, Result );
		snprintf( Detail, sizeof( Detail ), "result %d", (int)Result );
		InReport( Result == AK_Success, "Wwise music: LoadBank(Bank_Music_Streets 0xeea32b06)", Detail );

		const double BarSamples = 4.0 * 60.0 / 134.0 * Rate;
		AK::SoundEngine::SetState( 0x9cb5d15f, 0xa4ad5e0e );
		MusicSettle( 100 );
		ResetMusicEvents();
		const AkPlayingID PlayingID = AK::SoundEngine::PostEvent( 0x87084726, GameObj, AK_MusicSyncEntry | AK_MusicSyncBar | AK_MusicPlayStarted | AK_EndOfEvent, MusicCallback, NULL );
		InReport( PlayingID != AK_INVALID_PLAYING_ID, "Wwise music: PostEvent(0x87084726, plays music switch 0xc5cc052) returns a playing id", NULL );
		const bool Entered = MusicWaitFor( 5000, [] { return GMusic.Entries > 0; } );
		MusicSettle( 3100 );
		const LONG StateFrame = GFrames;
		const LONG BarsBefore = GMusic.Bars;
		AK::SoundEngine::SetState( 0x9cb5d15f, 0xd9f6455e );
		const bool Transitioned = MusicWaitFor( 6000, [] { return GMusic.Entries > 1; } );
		const LONG F0 = GMusic.EntryFrames[ 0 ];
		const LONG F1 = GMusic.EntryFrames[ 1 ];
		LONG BarsInSource = 0;
		for( LONG i = 0; i < GMusic.Bars && i < 64; ++i )
		{
			BarsInSource += GMusic.BarFrames[ i ] >= F0 && GMusic.BarFrames[ i ] < F1;
		}
		const double Transition = ( F1 - F0 ) * Frame;
		const double Bars = Transition / BarSamples;
		const double Nearest = floor( Bars + 0.5 );
		const double StateAt = ( StateFrame - F0 ) * Frame;
		snprintf( Detail, sizeof( Detail ), "source entry frame %ld, SetState at frame %ld (%.0f samples in), destination entry frame %ld: %.0f samples = %.4f bars of %.1f samples (bank 134 bpm 4/4), "
			"off the bar by %.0f samples; %ld AK_MusicSyncBar in the source (bar %.4f s)", F0, StateFrame, StateAt, F1, Transition, Bars, BarSamples,
			Transition - Nearest * BarSamples, BarsInSource, GMusic.BarDuration );
		printf( "      %s\n", Detail );
		InReport( Entered && Transitioned && fabs( Transition - Nearest * BarSamples ) <= Frame, "Wwise music: the state change transitions on a bar boundary of the bank's meter (within one audio frame)", Detail );
		InReport( Transitioned && Nearest * BarSamples >= StateAt && Nearest * BarSamples <= StateAt + BarSamples + 0.1 * Rate + 2 * Frame,
			"Wwise music: the transition takes the first bar after the state change (allowing the destination's 100 ms look-ahead)", Detail );
		InReport( Transitioned && BarsInSource == (LONG)Nearest && BarsBefore > 0, "Wwise music: one AK_MusicSyncBar per source bar before the transition", Detail );

		AK::SoundEngine::StopAll( GameObj );
		const bool Stopped = MusicWaitFor( 5000, [] { return GMusic.Ended != 0; } );
		InReport( Stopped, "Wwise music: StopAll ends the music playing id (AK_EndOfEvent)", NULL );
		AK::SoundEngine::SetState( 0x9cb5d15f, 0 );
		MusicSettle( 100 );
		AK::SoundEngine::UnloadBank( BankID, NULL );
		_aligned_free( Memory );
	}
	AK::SoundEngine::UnregisterGameObj( GameObj );
	MusicSettle( 50 );
}

void TermWwiseMusic()
{
	if( !GMusicInit )
	{
		return;
	}
	AK::MusicEngine::Term();
	AK::SoundEngine::UnregisterGlobalCallback( FrameTick );
	AK::StreamMgr::SetFileLocationResolver( GResolver.Previous );
	GMusicInit = false;
}

#endif
