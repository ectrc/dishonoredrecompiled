/*=============================================================================
	akunrealiohookblocking.cpp - Arkane's Wwise low-level IO: the file-location resolver, the blocking IO
	hook and the AKPK file-package layer on top of them.

	These three classes are NOT part of the Wwise libraries: Audiokinetic ships them as samples
	(SDK/samples/SoundEngine/...) and every licensee compiles its own copy, which is why the 2012 PDB
	attributes them to Arkane's own units. Retail's set, with the 2013 rvas this port is written from:
	  CAkUnrealIOHookBlocking::Open(AkFileID, ...)            0x5af410
	  CAkUnrealIOHookBlocking::Open(const AkOSChar*, ...)     0x5b3760
	  CAkUnrealIOHookBlocking::Read                           0x5b0c20
	  CAkUnrealIOHookBlocking::Close                          0x5b0d00
	  CAkUnrealIOHookBlocking::FillFileDescriptorHelper       0x5af3a0
	  CAkFilePackageLowLevelIO<..>::LoadFilePackage           0x5b2400   (_LoadFilePackage 0x5b19b0)
	  CAkFilePackageLowLevelIO<..>::UnloadFilePackage         0x5b1940
	  CAkFilePackageLowLevelIO<..>::UnloadAllFilePackages     0x5b4af0
	  CAkFilePackageLowLevelIO<..>::Open x2                   0x5b4b60 / 0x5b4c80
	  CAkFilePackageLowLevelIO<..>::FindPackagedFile x2        0x5b49e0 (AkFileID) / 0x5b4a70 (AkUInt64)
	  CAkFilePackageLowLevelIO<..>::Close / GetBlockSize      0x5b49c0 / 0x5b49a0
	  CAkFilePackageLowLevelIO<..>::Term                      0x5b0e10
	  CAkFilePackageLowLevelIO<..>::LanguageChangeHandler      0x5b0e90  (OnLanguageChange 0x5b00e0)
	Retail instantiates the template as CAkFilePackageLowLevelIO<CAkUnrealIOHookBlocking, CAkDiskPackage>;
	there is exactly one instantiation, so this port flattens it into one class and says so.

	DISHONORED(written): the AKPK container layout is read off the retail packages rather than taken from a
	header - see FAkFilePackage::Load below for what was measured and how.
=============================================================================*/
#include "AkAudio.h"

#if DISHONORED_WITH_WWISE

#include "akunrealiohookblocking.h"

/*-----------------------------------------------------------------------------
	CAkFileLocationBase
-----------------------------------------------------------------------------*/

void CAkFileLocationBase::SetBasePath( const TCHAR* InBasePath )
{
	m_BasePath = InBasePath;
}

void CAkFileLocationBase::AddBasePath( const FString& InPath )
{
	m_AdditionalPaths.AddUniqueItem( InPath );
}

void CAkFileLocationBase::RemoveAllAdditionalPaths()
{
	m_AdditionalPaths.Empty();
}

// DISHONORED(port): 2013 rva 0x5af3a0 (CAkUnrealIOHookBlocking::FillFileDescriptorHelper): the base path
// first, then every additional directory in the order they were added; the first file that exists wins.
UBOOL CAkFileLocationBase::ResolvePath( const TCHAR* InFileName, FString& OutFullPath ) const
{
	OutFullPath = m_BasePath + InFileName;
	if( GFileManager->FileSize( *OutFullPath ) >= 0 )
	{
		return TRUE;
	}
	for( INT PathIndex = 0; PathIndex < m_AdditionalPaths.Num(); ++PathIndex )
	{
		const FString Candidate = m_AdditionalPaths( PathIndex ) + InFileName;
		if( GFileManager->FileSize( *Candidate ) >= 0 )
		{
			OutFullPath = Candidate;
			return TRUE;
		}
	}
	return FALSE;
}

/*-----------------------------------------------------------------------------
	FAkFilePackage: one AKPK container
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(written): the AKPK header, measured on the retail packages with build/agentAN/bank_stid.py and
 * the AKPK probe in agentAN.md (311 CookedPCConsole\*.pck and 670 Bk_*.<language>):
 *   char 'AKPK', AkUInt32 uHeaderSize, AkUInt32 uVersion (= 1), then the four table sizes
 *   (language map, soundbanks, streamed files, externals), then the four tables back to back.
 * Each of the soundbank and streamed-file tables is AkUInt32 uNumFiles followed by 20-byte entries
 * { AkFileID fileID; AkUInt32 uBlockSize; AkUInt32 uFileSize; AkUInt32 uStartBlock; AkUInt32 uLanguageID; }.
 * Data begins at 8 + uHeaderSize and a file's offset inside the package is uStartBlock * uBlockSize; every
 * retail package uses uBlockSize 1, so uStartBlock is a plain byte offset (verified: 'BKHD' at uStartBlock
 * for every bank entry, 'RIFF' for every streamed entry). The externals table is always empty (uNumFiles 0).
 */
UBOOL FAkFilePackage::Load( const TCHAR* InFullPath, AkUInt32 InPackageID )
{
	PackageID = InPackageID;
	FullPath = InFullPath;
	Reader = GFileManager->CreateFileReader( InFullPath );
	if( !Reader )
	{
		return FALSE;
	}

	AkUInt32 Header[7];
	if( Reader->TotalSize() < (INT)sizeof(Header) )
	{
		CloseReader();
		return FALSE;
	}
	Reader->Serialize( Header, sizeof(Header) );
	if( appMemcmp( Header, "AKPK", 4 ) != 0 )
	{
		CloseReader();
		return FALSE;
	}
	const AkUInt32 HeaderSize = Header[1];
	Version = Header[2];
	const AkUInt32 LanguageMapSize = Header[3];
	const AkUInt32 SoundBanksSize = Header[4];
	const AkUInt32 StreamedFilesSize = Header[5];
	if( 8 + HeaderSize > (AkUInt32)Reader->TotalSize() )
	{
		CloseReader();
		return FALSE;
	}

	ReadTable( 28 + LanguageMapSize, SoundBanksSize, SoundBanks );
	ReadTable( 28 + LanguageMapSize + SoundBanksSize, StreamedFilesSize, StreamedFiles );
	return TRUE;
}

void FAkFilePackage::ReadTable( AkUInt32 TableOffset, AkUInt32 TableSize, TArray<FAkPackagedFile>& OutFiles )
{
	if( TableSize < sizeof(AkUInt32) || TableOffset + TableSize > (AkUInt32)Reader->TotalSize() )
	{
		return;
	}
	Reader->Seek( TableOffset );
	AkUInt32 NumFiles = 0;
	Reader->Serialize( &NumFiles, sizeof(NumFiles) );
	const AkUInt32 Available = ( TableSize - sizeof(AkUInt32) ) / 20;
	NumFiles = Min<AkUInt32>( NumFiles, Available );
	for( AkUInt32 FileIndex = 0; FileIndex < NumFiles; ++FileIndex )
	{
		AkUInt32 Entry[5];
		Reader->Serialize( Entry, sizeof(Entry) );
		FAkPackagedFile File;
		File.FileID = Entry[0];
		File.BlockSize = Entry[1] ? Entry[1] : 1;
		File.FileSize = Entry[2];
		File.Offset = (AkInt64)Entry[3] * (AkInt64)File.BlockSize;
		File.LanguageID = Entry[4];
		OutFiles.AddItem( File );
	}
}

const FAkPackagedFile* FAkFilePackage::Find( AkFileID InFileID, UBOOL bStreamed ) const
{
	const TArray<FAkPackagedFile>& Files = bStreamed ? StreamedFiles : SoundBanks;
	for( INT FileIndex = 0; FileIndex < Files.Num(); ++FileIndex )
	{
		if( Files( FileIndex ).FileID == InFileID )
		{
			return &Files( FileIndex );
		}
	}
	return NULL;
}

void FAkFilePackage::CloseReader()
{
	delete Reader;
	Reader = NULL;
}

/*-----------------------------------------------------------------------------
	CAkFilePackageLowLevelIO
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x5b1430's CAkDefaultIOHookBlocking::Init call site: create the streaming
// device with this object as the hook and register it as the file-location resolver, plus the
// language-change observer CAkFilePackageLowLevelIO installs (0x5b0e90).
AKRESULT CAkFilePackageLowLevelIO::Init( const AkDeviceSettings& InSettings )
{
	if( m_DeviceID != AK_INVALID_DEVICE_ID )
	{
		return AK_Success;
	}
	if( !( InSettings.uSchedulerTypeFlags & AK_SCHEDULER_BLOCKING ) )
	{
		return AK_NotCompatible;
	}
	m_DeviceID = AK::StreamMgr::CreateDevice( InSettings, this );
	if( m_DeviceID == AK_INVALID_DEVICE_ID )
	{
		return AK_Fail;
	}
	AK::StreamMgr::SetFileLocationResolver( this );
	AK::StreamMgr::AddLanguageChangeObserver( LanguageChangeHandler, this );
	return AK_Success;
}

// DISHONORED(port): 2013 rva 0x5b0e10
void CAkFilePackageLowLevelIO::Term()
{
	UnloadAllFilePackages();
	AK::StreamMgr::RemoveLanguageChangeObserver( this );
	if( AK::StreamMgr::GetFileLocationResolver() == this )
	{
		AK::StreamMgr::SetFileLocationResolver( NULL );
	}
	if( m_DeviceID != AK_INVALID_DEVICE_ID )
	{
		AK::StreamMgr::DestroyDevice( m_DeviceID );
		m_DeviceID = AK_INVALID_DEVICE_ID;
	}
}

// DISHONORED(port): 2013 rva 0x5b0e90 -> OnLanguageChange 0x5b00e0. Retail reopens the language-specific
// packages; with one bank per package and the language baked into the package's file name
// (Bk_<hash>.<language>), the packages themselves are reloaded by UAkBank::Load and nothing is cached here,
// so the handler only records the change.
void CAkFilePackageLowLevelIO::LanguageChangeHandler( const AkOSChar* const InLanguageName, void* InCookie )
{
	CAkFilePackageLowLevelIO* Self = (CAkFilePackageLowLevelIO*)InCookie;
	if( Self )
	{
		Self->m_Language = InLanguageName ? InLanguageName : TEXT("");
	}
}

// DISHONORED(port): 2013 rva 0x5b2400 -> _LoadFilePackage 0x5b19b0. The package id retail hands back is the
// AKPK's own position in its list; ours is a monotonic counter, which UAkBank stores in m_AkPackageID and
// only ever passes back to UnloadFilePackage.
AKRESULT CAkFilePackageLowLevelIO::LoadFilePackage( const TCHAR* InPackageName, AkUInt32& OutPackageID )
{
	OutPackageID = 0;
	FString FullPath;
	if( !ResolvePath( InPackageName, FullPath ) )
	{
		return AK_FileNotFound;
	}
	// Already loaded: hand the caller the same id instead of a second reader on the same file.
	for( INT PackageIndex = 0; PackageIndex < m_Packages.Num(); ++PackageIndex )
	{
		if( m_Packages( PackageIndex )->FullPath == FullPath )
		{
			OutPackageID = m_Packages( PackageIndex )->PackageID;
			return AK_BankAlreadyLoaded;
		}
	}

	FAkFilePackage* Package = new FAkFilePackage();
	if( !Package->Load( *FullPath, ++m_NextPackageID ) )
	{
		delete Package;
		return AK_InvalidFile;
	}
	m_Packages.AddItem( Package );
	OutPackageID = Package->PackageID;
	debugf( TEXT("Wwise: file package %s: AKPK v%u, %d banks, %d streamed files"), InPackageName,
		Package->Version, Package->SoundBanks.Num(), Package->StreamedFiles.Num() );
	return AK_Success;
}

// DISHONORED(port): 2013 rva 0x5b1940
AKRESULT CAkFilePackageLowLevelIO::UnloadFilePackage( AkUInt32 InPackageID )
{
	for( INT PackageIndex = 0; PackageIndex < m_Packages.Num(); ++PackageIndex )
	{
		if( m_Packages( PackageIndex )->PackageID != InPackageID )
		{
			continue;
		}
		m_Packages( PackageIndex )->CloseReader();
		delete m_Packages( PackageIndex );
		m_Packages.Remove( PackageIndex );
		return AK_Success;
	}
	return AK_Fail;
}

// DISHONORED(port): 2013 rva 0x5b4af0
AKRESULT CAkFilePackageLowLevelIO::UnloadAllFilePackages()
{
	for( INT PackageIndex = 0; PackageIndex < m_Packages.Num(); ++PackageIndex )
	{
		m_Packages( PackageIndex )->CloseReader();
		delete m_Packages( PackageIndex );
	}
	m_Packages.Empty();
	return AK_Success;
}

// DISHONORED(port): 2013 rva 0x5b49e0 (FindPackagedFile<AkFileID>): every loaded package in turn.
const FAkPackagedFile* CAkFilePackageLowLevelIO::FindPackagedFile( AkFileID InFileID, UBOOL bStreamed, FAkFilePackage*& OutPackage ) const
{
	for( INT PackageIndex = 0; PackageIndex < m_Packages.Num(); ++PackageIndex )
	{
		const FAkPackagedFile* File = m_Packages( PackageIndex )->Find( InFileID, bStreamed );
		if( File )
		{
			OutPackage = m_Packages( PackageIndex );
			return File;
		}
	}
	OutPackage = NULL;
	return NULL;
}

/**
 * DISHONORED(port): 2013 rva 0x5b4c80 (the string form) on top of 0x5b3760. A bank's id inside an AKPK is
 * the Wwise name hash of the bank name without its extension - verified against the retail content: every
 * one of the 311 .pck and 670 Bk_* packages holds exactly one bank whose table id equals
 * AK::SoundEngine::GetIDFromString of the package's own base name (build/agentAN/bank_stid.py). A name that
 * no package holds falls through to the loose file on disk, which is how Init.bnk loads.
 */
AKRESULT CAkFilePackageLowLevelIO::Open( const AkOSChar* InFileName, AkOpenMode InOpenMode, AkFileSystemFlags* InFlags, bool& IoSyncOpen, AkFileDesc& OutFileDesc )
{
	if( InOpenMode != AK_OpenModeRead || !InFileName )
	{
		return AK_NotImplemented;
	}
	const UBOOL bStreamed = InFlags && InFlags->uCodecID != AKCODECID_BANK;

	FString BaseName = InFileName;
	const INT DotIndex = BaseName.InStr( TEXT("."), TRUE );
	if( DotIndex != INDEX_NONE )
	{
		BaseName = BaseName.Left( DotIndex );
	}
	FAkFilePackage* Package = NULL;
	const FAkPackagedFile* File = FindPackagedFile( AK::SoundEngine::GetIDFromString( *BaseName ), bStreamed, Package );
	if( File )
	{
		appMemzero( &OutFileDesc, sizeof(OutFileDesc) );
		OutFileDesc.iFileSize = (AkInt64)File->FileSize;
		OutFileDesc.uSector = 0;
		OutFileDesc.deviceID = m_DeviceID;
		OutFileDesc.hFile = MakeHandle( Package->Reader, FALSE, File->Offset );
		IoSyncOpen = true;
		return AK_Success;
	}
	return CAkUnrealIOHookBlocking::Open( InFileName, InOpenMode, InFlags, IoSyncOpen, OutFileDesc );
}

// DISHONORED(port): 2013 rva 0x5b4b60 (the AkFileID form). Wwise only asks by id for streamed media.
AKRESULT CAkFilePackageLowLevelIO::Open( AkFileID InFileID, AkOpenMode InOpenMode, AkFileSystemFlags* InFlags, bool& IoSyncOpen, AkFileDesc& OutFileDesc )
{
	if( InOpenMode != AK_OpenModeRead )
	{
		return AK_NotImplemented;
	}
	const UBOOL bStreamed = !InFlags || InFlags->uCodecID != AKCODECID_BANK;
	FAkFilePackage* Package = NULL;
	const FAkPackagedFile* File = FindPackagedFile( InFileID, bStreamed, Package );
	if( !File )
	{
		return CAkUnrealIOHookBlocking::Open( InFileID, InOpenMode, InFlags, IoSyncOpen, OutFileDesc );
	}
	appMemzero( &OutFileDesc, sizeof(OutFileDesc) );
	OutFileDesc.iFileSize = (AkInt64)File->FileSize;
	OutFileDesc.deviceID = m_DeviceID;
	OutFileDesc.hFile = MakeHandle( Package->Reader, FALSE, File->Offset );
	IoSyncOpen = true;
	return AK_Success;
}

/*-----------------------------------------------------------------------------
	CAkUnrealIOHookBlocking: the loose-file half
-----------------------------------------------------------------------------*/

void* CAkUnrealIOHookBlocking::MakeHandle( FArchive* InReader, UBOOL bOwnsReader, AkInt64 InBaseOffset )
{
	FAkOpenFile* Handle = new FAkOpenFile();
	Handle->Reader = InReader;
	Handle->bOwnsReader = bOwnsReader;
	Handle->BaseOffset = InBaseOffset;
	return Handle;
}

// DISHONORED(port): 2013 rva 0x5b3760 on top of FillFileDescriptorHelper 0x5af3a0
AKRESULT CAkUnrealIOHookBlocking::Open( const AkOSChar* InFileName, AkOpenMode InOpenMode, AkFileSystemFlags* /*InFlags*/, bool& IoSyncOpen, AkFileDesc& OutFileDesc )
{
	appMemzero( &OutFileDesc, sizeof(OutFileDesc) );
	if( InOpenMode != AK_OpenModeRead || !InFileName )
	{
		return AK_NotImplemented;
	}
	FString FullPath;
	if( !ResolvePath( InFileName, FullPath ) )
	{
		return AK_FileNotFound;
	}
	FArchive* Reader = GFileManager->CreateFileReader( *FullPath );
	if( !Reader )
	{
		return AK_FileNotFound;
	}
	OutFileDesc.iFileSize = (AkInt64)Reader->TotalSize();
	OutFileDesc.deviceID = m_DeviceID;
	OutFileDesc.hFile = MakeHandle( Reader, TRUE, 0 );
	IoSyncOpen = true;
	return AK_Success;
}

// DISHONORED(bringup): 2013 rva 0x5af410. Wwise resolves a streamed file by id only when the game keeps a
// filename look-up table of its own; retail's resolver has no such table outside the file packages (the
// package layer above answers every id form), so a bare id reaches disk nowhere and the caller is told so.
AKRESULT CAkUnrealIOHookBlocking::Open( AkFileID /*InFileID*/, AkOpenMode /*InOpenMode*/, AkFileSystemFlags* /*InFlags*/, bool& /*IoSyncOpen*/, AkFileDesc& OutFileDesc )
{
	appMemzero( &OutFileDesc, sizeof(OutFileDesc) );
	return AK_FileNotFound;
}

// DISHONORED(port): 2013 rva 0x5b0c20. The transfer is relative to the file, the reader is positioned at
// the file's base inside its container.
AKRESULT CAkUnrealIOHookBlocking::Read( AkFileDesc& InFileDesc, const AkIoHeuristics& /*InHeuristics*/, void* OutBuffer, AkIOTransferInfo& IoTransferInfo )
{
	FAkOpenFile* Handle = (FAkOpenFile*)InFileDesc.hFile;
	if( !Handle || !Handle->Reader || !OutBuffer )
	{
		return AK_Fail;
	}
	const AkInt64 Position = Handle->BaseOffset + (AkInt64)IoTransferInfo.uFilePosition;
	AkUInt32 Size = IoTransferInfo.uRequestedSize;
	if( Position + (AkInt64)Size > Handle->BaseOffset + InFileDesc.iFileSize )
	{
		Size = (AkUInt32)Max<AkInt64>( 0, Handle->BaseOffset + InFileDesc.iFileSize - Position );
	}
	if( Position + (AkInt64)Size > (AkInt64)Handle->Reader->TotalSize() )
	{
		return AK_Fail;
	}
	Handle->Reader->Seek( (INT)Position );
	if( Size )
	{
		Handle->Reader->Serialize( OutBuffer, (INT)Size );
	}
	// The caller sized its buffer to a block multiple; anything past the file is zero, as Wwise expects.
	if( Size < IoTransferInfo.uRequestedSize )
	{
		appMemzero( (BYTE*)OutBuffer + Size, IoTransferInfo.uRequestedSize - Size );
	}
	return Handle->Reader->IsError() ? AK_Fail : AK_Success;
}

// DISHONORED(bringup): retail's hook is read-only in the shipping build too (the write path is the Wwise
// authoring profiler's), so Write refuses instead of pretending.
AKRESULT CAkUnrealIOHookBlocking::Write( AkFileDesc& /*InFileDesc*/, const AkIoHeuristics& /*InHeuristics*/, void* /*InData*/, AkIOTransferInfo& /*IoTransferInfo*/ )
{
	return AK_NotImplemented;
}

// DISHONORED(port): 2013 rva 0x5b0d00
AKRESULT CAkUnrealIOHookBlocking::Close( AkFileDesc& InFileDesc )
{
	FAkOpenFile* Handle = (FAkOpenFile*)InFileDesc.hFile;
	if( Handle )
	{
		if( Handle->bOwnsReader )
		{
			delete Handle->Reader;
		}
		delete Handle;
		InFileDesc.hFile = NULL;
	}
	return AK_Success;
}

// DISHONORED(retail): every retail package uses uBlockSize 1 and the loose banks are unaligned, so one byte
// is the device's block size (CAkFilePackageLowLevelIO::GetBlockSize 0x5b49a0 returns the entry's).
AkUInt32 CAkUnrealIOHookBlocking::GetBlockSize( AkFileDesc& /*InFileDesc*/ )
{
	return 1;
}

void CAkUnrealIOHookBlocking::GetDeviceDesc( AkDeviceDesc& OutDeviceDesc )
{
	appMemzero( &OutDeviceDesc, sizeof(OutDeviceDesc) );
	OutDeviceDesc.deviceID = m_DeviceID;
	OutDeviceDesc.bCanRead = true;
	OutDeviceDesc.bCanWrite = false;
	appStrncpy( OutDeviceDesc.szDeviceName, TEXT("Unreal"), ARRAY_COUNT(OutDeviceDesc.szDeviceName) );
	OutDeviceDesc.uStringSize = appStrlen( OutDeviceDesc.szDeviceName ) + 1;
}

AkUInt32 CAkUnrealIOHookBlocking::GetDeviceData()
{
	return 0;
}

#endif // DISHONORED_WITH_WWISE
