// WwiseSmoke: agent IH1's checks of the ported Wwise 2012.1 bank side (memory manager, stream manager over a
// loose-file blocking hook, CAkBankMgr + the HIRC hierarchy), against the retail content and against
// resources/tools/wwise_bank_dump.py's independent reading of it.
//
// The retail CookedPCConsole folder is read in place; the python counts come from
//   python resources/tools/wwise_bank_dump.py --csv <file>   (WWISE_SMOKE_CSV, default ../agentIH1/hirc_counts.csv)
// Every check that needs a missing input is reported as SKIP, not FAIL.
#if DISHONORED_WITH_WWISE

#define WIN32_LEAN_AND_MEAN
#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)

#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/SoundEngine/Common/AkQueryParameters.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <string>
#include <vector>

void AkAudioLibIndexCounts( AkUInt32 out_counts[ 10 ] );
void AkAudioLibLastBankCounts( AkUInt32 out_counts[ 20 ], AkUInt32 out_failures[ 20 ], AkUInt32 out_misparsed[ 20 ], AkUInt32 & out_uBytes );

typedef void ( *WwiseReportFunc )( bool Passed, const char * Name, const char * Detail );
typedef void ( *WwiseSkipFunc )( const char * Name, const char * Reason );

namespace
{
const wchar_t * const CookedDir = L"D:\\RecompileDishonored\\Dishonored_Latest2026\\DishonoredGame\\CookedPCConsole\\";

const char * const HircNames[ 20 ] =
{
	"?", "State", "Sound", "Action", "Event", "RanSeqCntr", "SwitchCntr", "ActorMixer", "Bus", "LayerCntr",
	"MusicSegment", "MusicTrack", "MusicSwitchCntr", "MusicRanSeqCntr", "Attenuation", "DialogueEvent",
	"FeedbackBus", "FeedbackNode", "FxShareSet", "FxCustom"
};

WwiseReportFunc GReport = NULL;

void CheckEq( const char * Name, long long Actual, long long Expected )
{
	char Detail[ 256 ];
	snprintf( Detail, sizeof( Detail ), "%lld, expected %lld", Actual, Expected );
	GReport( Actual == Expected, Name, Detail );
}

/** The loose-file blocking hook: Open(name) under CookedPCConsole, byte-granular blocking reads. */
class FSmokeHook : public AK::StreamMgr::IAkFileLocationResolver, public AK::StreamMgr::IAkIOHookBlocking
{
public:
	AkDeviceID DeviceID = AK_INVALID_DEVICE_ID;
	int Opens = 0;

	virtual AKRESULT Open( AkFileID, AkOpenMode, AkFileSystemFlags *, bool &, AkFileDesc & out_fileDesc )
	{
		memset( &out_fileDesc, 0, sizeof( out_fileDesc ) );
		return AK_FileNotFound;
	}

	virtual AKRESULT Open( const AkOSChar * in_pszFileName, AkOpenMode in_eOpenMode, AkFileSystemFlags *, bool & io_bSyncOpen, AkFileDesc & out_fileDesc )
	{
		memset( &out_fileDesc, 0, sizeof( out_fileDesc ) );
		if( in_eOpenMode != AK_OpenModeRead )
		{
			return AK_NotImplemented;
		}
		const std::wstring Path = std::wstring( CookedDir ) + in_pszFileName;
		HANDLE File = CreateFileW( Path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( File == INVALID_HANDLE_VALUE )
		{
			return AK_FileNotFound;
		}
		LARGE_INTEGER Size;
		GetFileSizeEx( File, &Size );
		out_fileDesc.iFileSize = Size.QuadPart;
		out_fileDesc.hFile = File;
		out_fileDesc.deviceID = DeviceID;
		io_bSyncOpen = true;
		++Opens;
		return AK_Success;
	}

	virtual AKRESULT Read( AkFileDesc & in_fileDesc, const AkIoHeuristics &, void * out_pBuffer, AkIOTransferInfo & io_transferInfo )
	{
		LARGE_INTEGER Position;
		Position.QuadPart = (LONGLONG)io_transferInfo.uFilePosition;
		SetFilePointerEx( (HANDLE)in_fileDesc.hFile, Position, NULL, FILE_BEGIN );
		DWORD Read = 0;
		if( !ReadFile( (HANDLE)in_fileDesc.hFile, out_pBuffer, io_transferInfo.uRequestedSize, &Read, NULL ) )
		{
			return AK_Fail;
		}
		if( Read < io_transferInfo.uRequestedSize )
		{
			memset( (char *)out_pBuffer + Read, 0, io_transferInfo.uRequestedSize - Read );
		}
		return AK_Success;
	}

	virtual AKRESULT Write( AkFileDesc &, const AkIoHeuristics &, void *, AkIOTransferInfo & ) { return AK_NotImplemented; }

	virtual AKRESULT Close( AkFileDesc & in_fileDesc )
	{
		CloseHandle( (HANDLE)in_fileDesc.hFile );
		return AK_Success;
	}

	virtual AkUInt32 GetBlockSize( AkFileDesc & ) { return 1; }

	virtual void GetDeviceDesc( AkDeviceDesc & out_deviceDesc ) { memset( &out_deviceDesc, 0, sizeof( out_deviceDesc ) ); }

	virtual AkUInt32 GetDeviceData() { return 0; }
};

std::vector<unsigned char> ReadWholeFile( const std::wstring & Path )
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

struct FCsvBank
{
	std::string Label;
	unsigned BankID = 0;
	unsigned Size = 0;
	unsigned Counts[ 20 ] = {};
};

std::vector<FCsvBank> ReadCsv( const char * Path )
{
	std::vector<FCsvBank> Rows;
	FILE * File = fopen( Path, "r" );
	if( !File )
	{
		return Rows;
	}
	char Line[ 4096 ];
	std::vector<std::string> Header;
	auto Split = []( const char * In ) {
		std::vector<std::string> Out;
		std::string Cur;
		bool Quoted = false;
		for( const char * P = In; *P && *P != '\n' && *P != '\r'; ++P )
		{
			if( *P == '"' )
			{
				Quoted = !Quoted;
			}
			else if( *P == ',' && !Quoted )
			{
				Out.push_back( Cur );
				Cur.clear();
			}
			else
			{
				Cur += *P;
			}
		}
		Out.push_back( Cur );
		return Out;
	};
	if( fgets( Line, sizeof( Line ), File ) )
	{
		Header = Split( Line );
	}
	while( fgets( Line, sizeof( Line ), File ) )
	{
		const std::vector<std::string> Cells = Split( Line );
		FCsvBank Row;
		for( size_t i = 0; i < Cells.size() && i < Header.size(); ++i )
		{
			if( Header[ i ] == "bank" )
			{
				Row.Label = Cells[ i ];
			}
			else if( Header[ i ] == "bank_id" )
			{
				Row.BankID = (unsigned)strtoul( Cells[ i ].c_str(), NULL, 16 );
			}
			else if( Header[ i ] == "size" )
			{
				Row.Size = (unsigned)strtoul( Cells[ i ].c_str(), NULL, 10 );
			}
			for( int t = 1; t < 20; ++t )
			{
				if( Header[ i ] == HircNames[ t ] )
				{
					Row.Counts[ t ] = (unsigned)strtoul( Cells[ i ].c_str(), NULL, 10 );
				}
			}
		}
		Rows.push_back( Row );
	}
	fclose( File );
	return Rows;
}

/** The bank inside an AKPK (the python tool's Package.banks), for the in-memory sweep. */
bool FindPackagedBank( const std::vector<unsigned char> & Pck, unsigned BankID, size_t & OutOffset, size_t & OutSize )
{
	if( Pck.size() < 28 || memcmp( Pck.data(), "AKPK", 4 ) != 0 )
	{
		return false;
	}
	const unsigned * Header = (const unsigned *)Pck.data();
	const unsigned LangMapSize = Header[ 3 ];
	const size_t TableAt = 28 + LangMapSize;
	if( TableAt + 4 > Pck.size() )
	{
		return false;
	}
	const unsigned NumFiles = *(const unsigned *)( Pck.data() + TableAt );
	for( unsigned i = 0; i < NumFiles; ++i )
	{
		const unsigned * Entry = (const unsigned *)( Pck.data() + TableAt + 4 + 20 * i );
		if( Entry[ 0 ] == BankID )
		{
			OutOffset = (size_t)Entry[ 3 ] * ( Entry[ 1 ] ? Entry[ 1 ] : 1 );
			OutSize = Entry[ 2 ];
			return OutOffset + OutSize <= Pck.size();
		}
	}
	return false;
}

unsigned TotalOf( const AkUInt32 * Counts )
{
	unsigned Total = 0;
	for( int t = 1; t < 20; ++t )
	{
		Total += Counts[ t ];
	}
	return Total;
}
}

void TestWwise( WwiseReportFunc InReport, WwiseSkipFunc InSkip )
{
	GReport = InReport;

	CheckEq( "Wwise GetIDFromString(\"Init\") is Init.bnk's BKHD id", AK::SoundEngine::GetIDFromString( "Init" ), 0x50c63a23 );
	CheckEq( "Wwise GetIDFromString lowercases A-Z only", AK::SoundEngine::GetIDFromString( L"INIT" ), 0x50c63a23 );

	if( GetFileAttributesW( ( std::wstring( CookedDir ) + L"Init.bnk" ).c_str() ) == INVALID_FILE_ATTRIBUTES )
	{
		InSkip( "Wwise bank loading", "retail CookedPCConsole not found" );
		return;
	}

	AkMemSettings MemSettings;
	MemSettings.uMaxNumPools = 384;
	GReport( AK::MemoryMgr::Init( &MemSettings ) == AK_Success, "Wwise AK::MemoryMgr::Init", NULL );
	AkStreamMgrSettings StreamSettings;
	AK::StreamMgr::GetDefaultSettings( StreamSettings );
	GReport( AK::StreamMgr::Create( StreamSettings ) != NULL, "Wwise AK::StreamMgr::Create", NULL );
	static FSmokeHook Hook;
	AkDeviceSettings DeviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings( DeviceSettings );
	DeviceSettings.uSchedulerTypeFlags = AK_SCHEDULER_BLOCKING;
	Hook.DeviceID = AK::StreamMgr::CreateDevice( DeviceSettings, &Hook );
	GReport( Hook.DeviceID != AK_INVALID_DEVICE_ID, "Wwise blocking device + \"AK::IOThread\"", NULL );
	AK::StreamMgr::SetFileLocationResolver( &Hook );
	GReport( AK::SoundEngine::Init( NULL, NULL ) == AK_Success, "Wwise AK::SoundEngine::Init (bank manager thread)", NULL );

	AkBankID InitID = 0;
	const AKRESULT InitResult = AK::SoundEngine::LoadBank( L"Init.bnk", AK_DEFAULT_POOL_ID, InitID );
	CheckEq( "Wwise LoadBank(Init.bnk) through the stream manager", InitResult, AK_Success );
	CheckEq( "Wwise LoadBank(Init.bnk) bank id", InitID, 0x50c63a23 );
	AkUInt32 Counts[ 20 ], Failures[ 20 ], Misparsed[ 20 ], Bytes = 0;
	AkAudioLibLastBankCounts( Counts, Failures, Misparsed, Bytes );
	CheckEq( "Wwise Init.bnk size read", Bytes, 14052 );
	CheckEq( "Wwise Init.bnk HIRC total (python: 95)", TotalOf( Counts ), 95 );
	CheckEq( "Wwise Init.bnk HIRC State (python: 13)", Counts[ 1 ], 13 );
	CheckEq( "Wwise Init.bnk HIRC Bus (python: 27)", Counts[ 8 ], 27 );
	CheckEq( "Wwise Init.bnk HIRC FxShareSet (python: 55)", Counts[ 18 ], 55 );
	CheckEq( "Wwise Init.bnk objects that failed to load", TotalOf( Failures ), 0 );
	CheckEq( "Wwise Init.bnk objects that did not read their whole section", TotalOf( Misparsed ), 0 );
	AkUInt32 Index[ 10 ];
	AkAudioLibIndexCounts( Index );
	CheckEq( "Wwise index: 27 buses", Index[ 0 ], 27 );
	CheckEq( "Wwise index: 13 custom states", Index[ 1 ], 13 );
	CheckEq( "Wwise index: 55 FX share sets", Index[ 8 ], 55 );
	CheckEq( "Wwise LoadBank(Init.bnk) again is AK_BankAlreadyLoaded", AK::SoundEngine::LoadBank( L"Init.bnk", AK_DEFAULT_POOL_ID, InitID ), AK_BankAlreadyLoaded );

	const char * CsvPath = getenv( "WWISE_SMOKE_CSV" ) ? getenv( "WWISE_SMOKE_CSV" ) : "../agentIH1/hirc_counts.csv";
	const std::vector<FCsvBank> Rows = ReadCsv( CsvPath );
	if( Rows.empty() )
	{
		InSkip( "Wwise in-memory sweep of every packaged bank", "no python CSV (run wwise_bank_dump.py --csv)" );
	}
	else
	{
		int Compared = 0, CountMismatch = 0, Loaded = 0, Leaks = 0;
		unsigned long long Objects = 0, FailedObjects = 0, MisparsedObjects = 0;
		unsigned FailedByType[ 20 ] = {}, MisparsedByType[ 20 ] = {};
		std::string FirstMismatch;
		for( const FCsvBank & Row : Rows )
		{
			const size_t Hash = Row.Label.find( '#' );
			if( Hash == std::string::npos )
			{
				continue;
			}
			const std::string PckName = Row.Label.substr( 0, Hash );
			const std::vector<unsigned char> Pck = ReadWholeFile( std::wstring( CookedDir ) + std::wstring( PckName.begin(), PckName.end() ) );
			size_t Offset = 0, Size = 0;
			if( !FindPackagedBank( Pck, Row.BankID, Offset, Size ) )
			{
				continue;
			}
			void * Memory = _aligned_malloc( Size, 16 );
			memcpy( Memory, Pck.data() + Offset, Size );
			AkBankID BankID = 0;
			const AKRESULT Result = AK::SoundEngine::LoadBank( Memory, (AkUInt32)Size, BankID );
			AkAudioLibLastBankCounts( Counts, Failures, Misparsed, Bytes );
			++Compared;
			if( Result == AK_Success )
			{
				++Loaded;
			}
			bool Same = Bytes == Row.Size;
			for( int t = 1; t < 20; ++t )
			{
				Same = Same && Counts[ t ] == Row.Counts[ t ];
				FailedByType[ t ] += Failures[ t ];
				MisparsedByType[ t ] += Misparsed[ t ];
			}
			Objects += TotalOf( Counts );
			FailedObjects += TotalOf( Failures );
			MisparsedObjects += TotalOf( Misparsed );
			if( !Same )
			{
				++CountMismatch;
				if( FirstMismatch.empty() )
				{
					FirstMismatch = Row.Label;
				}
			}
			AK::SoundEngine::UnloadBank( BankID, NULL );
			AkUInt32 After[ 10 ];
			AkAudioLibIndexCounts( After );
			for( int i = 0; i < 10; ++i )
			{
				Leaks += After[ i ] != Index[ i ];
			}
			_aligned_free( Memory );
		}
		char Detail[ 512 ];
		snprintf( Detail, sizeof( Detail ), "%d banks compared, %d loaded, %d count mismatches (first %s)", Compared, Loaded, CountMismatch, FirstMismatch.c_str() );
		GReport( Compared > 300 && CountMismatch == 0, "Wwise every packaged bank: CAkBankMgr HIRC counts by type == wwise_bank_dump.py", Detail );
		snprintf( Detail, sizeof( Detail ), "%d of %d", Loaded, Compared );
		GReport( Loaded == Compared, "Wwise every packaged bank loads from memory (AK_Success)", Detail );
		std::string ByType;
		for( int t = 1; t < 20; ++t )
		{
			if( FailedByType[ t ] || MisparsedByType[ t ] )
			{
				char Part[ 64 ];
				snprintf( Part, sizeof( Part ), " %s=%u/%u", HircNames[ t ], FailedByType[ t ], MisparsedByType[ t ] );
				ByType += Part;
			}
		}
		snprintf( Detail, sizeof( Detail ), "%llu objects, %llu failed, %llu misparsed (failed/misparsed by type:%s)", Objects, FailedObjects, MisparsedObjects, ByType.c_str() );
		printf( "      %s\n", Detail );
		GReport( MisparsedObjects == 0, "Wwise every created object read exactly its HIRC section", Detail );
		GReport( Leaks == 0, "Wwise UnloadBank returns every index to its Init.bnk-only population", NULL );
	}

	AkPositioningInfo Info;
	CheckEq( "Wwise Query::GetPositioningInfo(Init.bnk bus 0x26df8722) is AK_NotCompatible", AK::SoundEngine::Query::GetPositioningInfo( 0x26df8722u, Info ), AK_NotCompatible );
	AkUInt32 NumObjects = 0;
	CheckEq( "Wwise Query::QueryAudioObjectIDs(unknown event) is AK_IDNotFound", AK::SoundEngine::Query::QueryAudioObjectIDs( 0x12345678u, NumObjects, NULL ), AK_IDNotFound );

	CheckEq( "Wwise UnloadBank(Init.bnk)", AK::SoundEngine::UnloadBank( L"Init.bnk", NULL ), AK_Success );
	AkAudioLibIndexCounts( Index );
	CheckEq( "Wwise index empty after the last unload", (long long)Index[ 0 ] + Index[ 1 ] + Index[ 2 ] + Index[ 3 ] + Index[ 8 ], 0 );
	AK::SoundEngine::Term();
	AK::StreamMgr::DestroyDevice( Hook.DeviceID );
	AK::IAkStreamMgr::Get()->Destroy();
	AK::MemoryMgr::Term();
	GReport( !AK::MemoryMgr::IsInitialized(), "Wwise orderly shutdown (sound engine, device, stream manager, memory manager)", NULL );
}

#endif
