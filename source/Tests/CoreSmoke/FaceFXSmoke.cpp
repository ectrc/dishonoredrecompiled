// FaceFXSmoke: agent FX1's checks of the reconstructed FaceFX 1.7.3.1 core (memory, names, classes, archive),
// against retail FaceFX archives and against resources/tools/facefx_archive_dump.py's independent reading of them.
//
// Inputs: a folder of head archives (<asset>.fxa, the RawFaceFXActorBytes of a cooked UFaceFXAsset) each with the
// <asset>.txt that `python resources/tools/facefx_archive_dump.py --smoke <dir>/*.fxa` writes next to it.
// FACEFX_CORE_SMOKE_DIR names the folder (default build/agentFX1/fxa, then ../agentFX1/fxa, from the tree root);
// `python resources/tools/facefx_archive_dump.py --extract <dir> L_Tower_Script.upk` fills it from the retail
// CookedPCConsole. Every check that needs a missing input is reported as SKIP, not FAIL.
#if DISHONORED_WITH_FACEFX_SDK

#define WIN32_LEAN_AND_MEAN
#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)

#include <FxSDK.h>
#include <FxArchive.h>
#include <FxBone.h>
#include <FxKey.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using namespace OC3Ent::Face;

typedef void ( *FaceFXReportFunc )( bool Passed, const char * Name, const char * Detail );
typedef void ( *FaceFXSkipFunc )( const char * Name, const char * Reason );

namespace
{
FaceFXReportFunc GReport = NULL;
FaceFXSkipFunc GSkip = NULL;

long long GUserBytes = 0;
long long GUserAllocs = 0;

void* SmokeAllocate( FxSize NumBytes )
{
	++GUserAllocs;
	GUserBytes += NumBytes;
	return malloc( NumBytes );
}

void* SmokeAllocateDebug( FxSize NumBytes, const FxChar* )
{
	return SmokeAllocate( NumBytes );
}

void SmokeFree( void* Ptr, FxSize NumBytes )
{
	if( Ptr )
	{
		--GUserAllocs;
		GUserBytes -= NumBytes;
	}
	free( Ptr );
}

void CheckEq( const char * Name, long long Actual, long long Expected )
{
	char Detail[ 256 ];
	snprintf( Detail, sizeof( Detail ), "%lld, expected %lld", Actual, Expected );
	GReport( Actual == Expected, Name, Detail );
}

void CheckStr( const char * Name, const char * Actual, const char * Expected )
{
	char Detail[ 512 ];
	snprintf( Detail, sizeof( Detail ), "\"%s\", expected \"%s\"", Actual ? Actual : "(null)", Expected );
	GReport( Actual && strcmp( Actual, Expected ) == 0, Name, Detail );
}

struct FArchiveExpectation
{
	struct FClass
	{
		FxSize Bin;
		FxSize Version;
		std::string Name;
	};
	struct FName
	{
		FxSize Bin;
		std::string Name;
	};
	std::string File;
	FxSize Size = 0;
	FxSize Sdk = 0;
	FxSize Format = 0;
	std::string Licensee;
	std::string Project;
	FxSize LicVersion = 0;
	FxSize DirVersion = 0;
	std::vector< FClass > Classes;
	std::vector< FName > Names;
	FxSize NameBinsUsed = 0;
	FxSize NameInsertions = 0;
	FxSize NameCollisions = 0;
	FxSize Objects = 0;
	FxSize DirEnd = 0;
	FxSize ActorName = 0;
};

bool ReadExpectation( const std::string & Path, FArchiveExpectation & Out )
{
	FILE * File = NULL;
	if( fopen_s( &File, Path.c_str(), "rb" ) != 0 || !File )
	{
		return false;
	}
	char Line[ 1024 ];
	while( fgets( Line, sizeof( Line ), File ) )
	{
		size_t Len = strlen( Line );
		while( Len && ( Line[ Len - 1 ] == '\n' || Line[ Len - 1 ] == '\r' ) )
		{
			Line[ --Len ] = 0;
		}
		char * Space = strchr( Line, ' ' );
		if( !Space )
		{
			continue;
		}
		*Space = 0;
		const char * Key = Line;
		const char * Rest = Space + 1;
		if( !strcmp( Key, "file" ) ) Out.File = Rest;
		else if( !strcmp( Key, "size" ) ) Out.Size = strtoul( Rest, NULL, 10 );
		else if( !strcmp( Key, "sdk" ) ) Out.Sdk = strtoul( Rest, NULL, 10 );
		else if( !strcmp( Key, "format" ) ) Out.Format = strtoul( Rest, NULL, 10 );
		else if( !strcmp( Key, "licensee" ) ) Out.Licensee = Rest;
		else if( !strcmp( Key, "project" ) ) Out.Project = Rest;
		else if( !strcmp( Key, "licversion" ) ) Out.LicVersion = strtoul( Rest, NULL, 10 );
		else if( !strcmp( Key, "dirversion" ) ) Out.DirVersion = strtoul( Rest, NULL, 10 );
		else if( !strcmp( Key, "objects" ) ) Out.Objects = strtoul( Rest, NULL, 10 );
		else if( !strcmp( Key, "dirend" ) ) Out.DirEnd = strtoul( Rest, NULL, 10 );
		else if( !strcmp( Key, "actorname" ) ) Out.ActorName = strtoul( Rest, NULL, 10 );
		else if( !strcmp( Key, "namehash" ) )
		{
			char * End = NULL;
			Out.NameBinsUsed = strtoul( Rest, &End, 10 );
			Out.NameInsertions = strtoul( End, &End, 10 );
			Out.NameCollisions = strtoul( End, &End, 10 );
		}
		else if( !strcmp( Key, "class" ) )
		{
			char * End = NULL;
			FArchiveExpectation::FClass Class;
			Class.Bin = strtoul( Rest, &End, 10 );
			Class.Version = strtoul( End, &End, 10 );
			Class.Name = End + 1;
			Out.Classes.push_back( Class );
		}
		else if( !strcmp( Key, "name" ) )
		{
			char * End = NULL;
			strtoul( Rest, &End, 10 );
			FArchiveExpectation::FName Name;
			Name.Bin = strtoul( End, &End, 10 );
			Name.Name = End + 1;
			Out.Names.push_back( Name );
		}
	}
	fclose( File );
	return true;
}

std::vector< FxByte > ReadBytes( const std::string & Path )
{
	std::vector< FxByte > Bytes;
	FILE * File = NULL;
	if( fopen_s( &File, Path.c_str(), "rb" ) == 0 && File )
	{
		fseek( File, 0, SEEK_END );
		long Length = ftell( File );
		fseek( File, 0, SEEK_SET );
		Bytes.resize( Length > 0 ? Length : 0 );
		if( Length > 0 )
		{
			fread( Bytes.data(), 1, Length, File );
		}
		fclose( File );
	}
	return Bytes;
}

std::string FindArchiveDir()
{
	char Env[ MAX_PATH ];
	DWORD EnvLen = GetEnvironmentVariableA( "FACEFX_CORE_SMOKE_DIR", Env, MAX_PATH );
	if( EnvLen && EnvLen < MAX_PATH )
	{
		return Env;
	}
	const char * Candidates[] = { "build/agentFX1/fxa", "../agentFX1/fxa" };
	for( const char * Candidate : Candidates )
	{
		DWORD Attributes = GetFileAttributesA( Candidate );
		if( Attributes != INVALID_FILE_ATTRIBUTES && ( Attributes & FILE_ATTRIBUTE_DIRECTORY ) )
		{
			return Candidate;
		}
	}
	return std::string();
}

/** The fixed-size pools (8..128 bytes): blocks come back in LIFO order and the chunk is reused. */
void TestMemoryPools()
{
	FxMemoryAllocationPolicy Policy( MAT_Custom, FxTrue, SmokeAllocate, SmokeAllocateDebug, SmokeFree );
	FxMemoryStartup( Policy );
	CheckEq( "FaceFX: pools - fixed-size allocators", FxGetNumFixedSizeAllocators(), 16 );
	void * A = FxAllocate( 20 );
	void * B = FxAllocate( 20 );
	CheckEq( "FaceFX: pools - 20-byte blocks are 24 apart", static_cast< FxByte * >( B ) - static_cast< FxByte * >( A ), 24 );
	FxFixedSizeAllocatorInfo Info = FxGetFixedSizeAllocatorInfo( 2 );
	CheckEq( "FaceFX: pools - 24-byte pool blocks per chunk", Info.numBlocksPerChunk, 170 );
	FxFree( B, 20 );
	void * C = FxAllocate( 17 );
	GReport( C == B, "FaceFX: pools - freed block is handed out again", "" );
	void * Big = FxAllocate( 200 );
	GReport( Big != NULL, "FaceFX: pools - 200 bytes go to the user allocator", "" );
	FxFree( Big, 200 );
	FxFree( C, 17 );
	FxFree( A, 20 );
	FxMemoryShutdown();
	CheckEq( "FaceFX: pools - every chunk returned", GUserAllocs, 0 );
}

void TestNames()
{
	FxName A( "Lip Corner|L" );
	CheckStr( "FaceFX: FxName proofs ' ' and '|'", A.GetAsCstr(), "Lip_Corner_L" );
	FxName B( "Lip_Corner_L" );
	GReport( A == B, "FaceFX: FxName equal spellings share one string", "" );
	FxSize Index = FxName::Find( "Lip_Corner_L" );
	GReport( Index != FxInvalidIndex, "FaceFX: FxName::Find", "" );
	CheckEq( "FaceFX: FxName reference count", Index != FxInvalidIndex ? FxName::GetNumReferences( Index ) : 0, 2 );
	FxName Quoted( "say \"ah\"" );
	CheckStr( "FaceFX: FxName proofs '\"'", Quoted.GetAsCstr(), "say_'ah'" );
	GReport( FxName().GetAsString().Length() == 0 && FxName() == FxName::NullName, "FaceFX: default FxName is NullName (\"\")", "" );
	FxSize NamesBefore = FxName::GetNumNames();
	{
		FxName Temp( "FX1_temporary_name" );
		CheckEq( "FaceFX: FxName adds a table entry", FxName::GetNumNames(), NamesBefore + 1 );
	}
	CheckEq( "FaceFX: last FxName reference removes the entry", FxName::GetNumNames(), NamesBefore );
	FxName Renamed( "FX1_rename_me" );
	FxName Alias( Renamed );
	GReport( Renamed.Rename( "FX1 renamed" ) && Alias == FxName( "FX1_renamed" ), "FaceFX: FxName::Rename is seen by every sharer", Alias.GetAsCstr() );
	GReport( !B.Rename( "FX1_renamed" ), "FaceFX: FxName::Rename refuses a spelling in use", "" );
	CheckStr( "FaceFX: FxSDKConvertVersionToString(1731)", FxSDKGetVersionString().GetData(), "1.7.3.1" );
	CheckStr( "FaceFX: FxSDKGetBuildId", FxSDKGetBuildId().GetData(), "1731100780" );
	GReport( FxClass::FindClassDesc( FxName( "FxBone" ) ) == FxBone::StaticClass() && FxBone::StaticClass()->IsKindOf( FxNamedObject::StaticClass() ),
		"FaceFX: FxBone registered, derives from FxNamedObject", "" );
}

/** Retail's hashing: the class bins stored in the archive, and the 12-bit FxName table over every archive name. */
void TestArchive( const std::string & Dir, const FArchiveExpectation & E )
{
	std::string Prefix = "FaceFX: " + E.File + " - ";
	std::vector< FxByte > Bytes = ReadBytes( Dir + "/" + E.File );
	CheckEq( ( Prefix + "bytes" ).c_str(), static_cast< long long >( Bytes.size() ), E.Size );
	if( Bytes.empty() )
	{
		return;
	}

	FxStringHash< FxSize, 12, FxList > NameHash;
	bool BinsMatch = true;
	for( const FArchiveExpectation::FName & Name : E.Names )
	{
		FxName Proofed( Name.Name.c_str() );
		NameHash.Insert( Proofed.GetAsCstr(), 0 );
		BinsMatch &= NameHash.GetBinIndex( Proofed.GetAsCstr() ) == Name.Bin;
	}
	GReport( BinsMatch, ( Prefix + "every name in its python djb2 bin (12 bits)" ).c_str(), "" );
	CheckEq( ( Prefix + "name hash bins in use" ).c_str(), NameHash.GetNumBinsInUse(), E.NameBinsUsed );
	CheckEq( ( Prefix + "name hash insertions" ).c_str(), NameHash.GetNumInsertions(), E.NameInsertions );
	CheckEq( ( Prefix + "name hash collisions" ).c_str(), NameHash.GetNumCollisions(), E.NameCollisions );

	{
		FxArchive Arc( FxArchiveStoreMemoryNoCopy::Create( Bytes.data(), static_cast< FxSize >( Bytes.size() ) ), FxArchive::AM_LinearLoad );
		Arc.Open();
		GReport( Arc.IsValid() != 0, ( Prefix + "FxArchive opens" ).c_str(), "" );
		if( !Arc.IsValid() )
		{
			return;
		}
		CheckEq( ( Prefix + "sdk version" ).c_str(), Arc.GetSDKVersion(), E.Sdk );
		CheckStr( ( Prefix + "sdk version string" ).c_str(), Arc.GetSDKVersionString(), "1.7.3.1" );
		CheckEq( ( Prefix + "file format version" ).c_str(), Arc.GetFileFormatVersion(), E.Format );
		CheckStr( ( Prefix + "licensee" ).c_str(), Arc.GetLicenseeName(), E.Licensee.c_str() );
		CheckStr( ( Prefix + "licensee project" ).c_str(), Arc.GetLicenseeProjectName(), E.Project.c_str() );
		CheckEq( ( Prefix + "licensee version" ).c_str(), Arc.GetLicenseeVersion(), E.LicVersion );
		const FxArchiveDirectory & Directory = Arc.GetInternalData()->Directory;
		CheckEq( ( Prefix + "class versions in the directory" ).c_str(), Directory.ClassVersions.GetNumBinsInUse(), E.Classes.size() );
		bool ClassesMatch = true;
		std::string ClassList;
		for( const FArchiveExpectation::FClass & Class : E.Classes )
		{
			FxUInt16 Version = 0xFFFF;
			bool Found = Directory.ClassVersions.Find( Class.Name.c_str(), Version ) != 0;
			bool InBin = false;
			const FxArray< FxStringHash< FxUInt16, 7, FxArray >::FxHashBinEntry > & Bin = Directory.ClassVersions.GetBin( Class.Bin );
			for( FxSize i = 0; i < Bin.Length(); ++i )
			{
				InBin |= Bin[ i ].key == Class.Name.c_str();
			}
			ClassesMatch &= Found && InBin && Version == Class.Version && Directory.ClassVersions.GetBinIndex( Class.Name.c_str() ) == Class.Bin;
			ClassList += Class.Name + " ";
		}
		GReport( ClassesMatch, ( Prefix + "class names, versions and retail bins (7 bits)" ).c_str(), ClassList.c_str() );
		CheckEq( ( Prefix + "archive names" ).c_str(), Directory.Names.Length(), E.Names.size() );
		bool NamesMatch = Directory.Names.Length() == E.Names.size();
		for( FxSize i = 0; NamesMatch && i < Directory.Names.Length(); ++i )
		{
			NamesMatch &= Directory.Names[ i ].NameStr == E.Names[ i ].Name.c_str();
			NamesMatch &= Directory.Names[ i ].Name == FxName( E.Names[ i ].Name.c_str() );
		}
		GReport( NamesMatch, ( Prefix + "archive names are live FxNames" ).c_str(), "" );
		CheckEq( ( Prefix + "object pointers" ).c_str(), Directory.NumObjectPointers, E.Objects );
		CheckEq( ( Prefix + "directory end" ).c_str(), Arc.Tell(), E.DirEnd );

		FxArchiveStoreMemory * Store = FxArchiveStoreMemory::Create( NULL, 0 );
		{
			FxArchive Resave( Store, FxArchive::AM_Save );
			Resave.Open();
			Resave.SetInternalDataState( Arc.GetInternalData() );
			bool Identical = Store->GetSize() == E.DirEnd && memcmp( Store->GetMemory(), Bytes.data(), E.DirEnd ) == 0;
			char Detail[ 128 ];
			snprintf( Detail, sizeof( Detail ), "%u bytes written", Store->GetSize() );
			GReport( Identical, ( Prefix + "header + directory re-saved byte for byte" ).c_str(), Detail );
		}

		FxNamedObject Actor;
		Actor.Serialize( Arc );
		std::string Stem = E.File.substr( 0, E.File.rfind( '.' ) );
		CheckStr( ( Prefix + "FxNamedObject::Serialize reads the actor name" ).c_str(), Actor.GetNameAsCstr(),
			E.ActorName < E.Names.size() ? E.Names[ E.ActorName ].Name.c_str() : "?" );
		GReport( Stem == Actor.GetNameAsCstr(), ( Prefix + "actor named after its asset" ).c_str(), Actor.GetNameAsCstr() );
	}
}

/** CreateDirectory pass + save (as FxSaveActorToMemory does) and load back through the object-pointer protocol. */
void TestRoundTrip( FxArchive::FxArchiveByteOrder ByteOrder, const char * Label )
{
	std::string Prefix = std::string( "FaceFX: round trip " ) + Label + " - ";
	FxBone * Bone = new FxBone( FxName( "jaw_jnt" ), FxVec3( 1.5f, -2.0f, 3.25f ), FxQuat( 0.5f, 0.5f, -0.5f, 0.5f ), FxVec3( 1.0f, 1.0f, 1.0f ) );
	FxAnimKey Key( 0.25f, 0.75f, -1.0f, 2.0f );
	FxString Text( "Unreal Engine 3 Licensee" );
	FxBool Flag = FxTrue;

	FxArchive Directory( FxArchiveStoreNull::Create(), FxArchive::AM_CreateDirectory );
	Directory.Open();
	Directory << Bone << Bone << Key << Text << Flag;

	FxArchiveStoreMemory * Store = FxArchiveStoreMemory::Create( NULL, 0 );
	std::vector< FxByte > Saved;
	{
		FxArchive Save( Store, FxArchive::AM_Save, ByteOrder );
		Save.Open();
		GReport( Save.IsValid() != 0, ( Prefix + "save archive opens" ).c_str(), "" );
		Save.SetInternalDataState( Directory.GetInternalData() );
		Save << Bone << Bone << Key << Text << Flag;
		Saved.assign( Store->GetMemory(), Store->GetMemory() + Store->GetSize() );
	}
	delete Bone;
	CheckEq( ( Prefix + "magic" ).c_str(), Saved.size() > 3 ? Saved[ 3 ] : 0, ByteOrder == FxArchive::ABO_BigEndian ? 'B' : 'E' );

	FxArchive Load( FxArchiveStoreMemoryNoCopy::Create( Saved.data(), static_cast< FxSize >( Saved.size() ) ), FxArchive::AM_LinearLoad );
	Load.Open();
	GReport( Load.IsValid() != 0, ( Prefix + "load archive opens" ).c_str(), "" );
	CheckEq( ( Prefix + "sdk version" ).c_str(), Load.GetSDKVersion(), 1731 );
	CheckEq( ( Prefix + "directory object pointers" ).c_str(), Load.GetInternalData()->Directory.NumObjectPointers, 1 );
	CheckEq( ( Prefix + "FxBone class version recorded" ).c_str(), Load.GetInternalData()->Directory.FindClassVersion( "FxBone" ), 0 );
	FxBone * First = NULL;
	FxBone * Second = NULL;
	FxAnimKey LoadedKey;
	FxString LoadedText;
	FxBool LoadedFlag = FxFalse;
	Load << First << Second << LoadedKey << LoadedText << LoadedFlag;
	GReport( First && First == Second, ( Prefix + "second pointer resolves to the first object" ).c_str(), "" );
	if( First )
	{
		GReport( First->GetClassDesc() == FxBone::StaticClass(), ( Prefix + "constructed through FxClass" ).c_str(), "" );
		CheckStr( ( Prefix + "bone name" ).c_str(), First->GetNameAsCstr(), "jaw_jnt" );
		GReport( First->GetPos().x == 1.5f && First->GetPos().y == -2.0f && First->GetPos().z == 3.25f
			&& First->GetRot().w == 0.5f && First->GetRot().y == -0.5f && First->GetScale().z == 1.0f,
			( Prefix + "bone transform" ).c_str(), "" );
		delete First;
	}
	GReport( LoadedKey.GetTime() == 0.25f && LoadedKey.GetValue() == 0.75f && LoadedKey.GetSlopeIn() == -1.0f && LoadedKey.GetSlopeOut() == 2.0f,
		( Prefix + "FxAnimKey" ).c_str(), "" );
	GReport( LoadedText == "Unreal Engine 3 Licensee" && LoadedFlag, ( Prefix + "string and bool" ).c_str(), LoadedText.GetData() );
	CheckEq( ( Prefix + "every byte consumed" ).c_str(), Load.Tell(), static_cast< long long >( Saved.size() ) );

	char TempPath[ MAX_PATH ];
	char TempFile[ MAX_PATH ];
	GetTempPathA( MAX_PATH, TempPath );
	GetTempFileNameA( TempPath, "fx1", 0, TempFile );
	{
		FILE * File = NULL;
		if( fopen_s( &File, TempFile, "wb" ) == 0 && File )
		{
			fwrite( Saved.data(), 1, Saved.size(), File );
			fclose( File );
		}
	}
	for( int Fast = 0; Fast < 2; ++Fast )
	{
		FxArchiveStore * FileStore = Fast ? static_cast< FxArchiveStore * >( FxArchiveStoreFileFast::Create( TempFile ) )
			: static_cast< FxArchiveStore * >( FxArchiveStoreFile::Create( TempFile ) );
		FxArchive FromFile( FileStore, FxArchive::AM_LinearLoad );
		FromFile.Open();
		FxBone * FileBone = NULL;
		FromFile << FileBone;
		GReport( FromFile.IsValid() && FileBone && strcmp( FileBone->GetNameAsCstr(), "jaw_jnt" ) == 0,
			( Prefix + ( Fast ? "FxArchiveStoreFileFast" : "FxArchiveStoreFile" ) ).c_str(), "" );
		delete FileBone;
	}
	DeleteFileA( TempFile );
}
} // namespace

void TestFaceFX( FaceFXReportFunc InReport, FaceFXSkipFunc InSkip )
{
	GReport = InReport;
	GSkip = InSkip;
	printf( "\nFaceFX 1.7.3.1 core (agent FX1)\n" );
	TestMemoryPools();

	// The SDK stays up with the default policy (FxSDKStartup(), as FaceFXGraphSmoke calls it next): retail never
	// re-starts it, and FxSDKShutdown would leave every class's cached descriptor dangling.
	FxSDKStartup();
	TestNames();
	TestRoundTrip( FxArchive::ABO_LittleEndian, "little endian" );
	TestRoundTrip( FxArchive::ABO_BigEndian, "big endian" );

	std::string Dir = FindArchiveDir();
	int NumArchives = 0;
	if( !Dir.empty() )
	{
		WIN32_FIND_DATAA Found;
		HANDLE Find = FindFirstFileA( ( Dir + "/*.txt" ).c_str(), &Found );
		if( Find != INVALID_HANDLE_VALUE )
		{
			do
			{
				FArchiveExpectation Expectation;
				if( ReadExpectation( Dir + "/" + Found.cFileName, Expectation ) && !Expectation.File.empty() )
				{
					TestArchive( Dir, Expectation );
					++NumArchives;
				}
			}
			while( FindNextFileA( Find, &Found ) );
			FindClose( Find );
		}
	}
	if( !NumArchives )
	{
		GSkip( "FaceFX: retail head archives", "no <asset>.fxa + <asset>.txt in FACEFX_CORE_SMOKE_DIR / build/agentFX1/fxa" );
	}
	printf( "FaceFX: %d retail archives checked\n", NumArchives );
}

#endif
