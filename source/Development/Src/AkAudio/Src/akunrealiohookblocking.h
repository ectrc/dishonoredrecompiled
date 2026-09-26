/*=============================================================================
	akunrealiohookblocking.h - declarations of Arkane's Wwise low-level IO classes.

	DISHONORED(written): retail has no such header (the classes live in the .cpp and the template header of
	the Wwise samples); this one exists so akaudiodevice.cpp can hold the g_lowLevelIO global the 2012 PDB
	names (_dynamic_initializer_for__g_lowLevelIO__, 0xba2fa0, attributed to akaudiodevice.cpp).

	Retail's inheritance, from the three vftable symbols of CAkUnrealIOHookBlocking (2013 rvas 0xcbb150,
	0xcbb16c, 0xcbb178):
	  CAkUnrealIOHookBlocking : AK::StreamMgr::IAkIOHookBlocking, AK::StreamMgr::IAkFileLocationResolver,
	                            CAkFileLocationBase
	and CAkFilePackageLowLevelIO<CAkUnrealIOHookBlocking, CAkDiskPackage> on top of it. The template has one
	instantiation in the exe, so it is flattened into a plain derived class here.

	CAkUnrealIOHookBlocking is 1068 bytes in the 2012 PDB (CAkDefaultIOHookBlocking plus an FEvent* at 1064);
	that layout is Wwise-sample internal, not an interface Wwise reads, so it is not reproduced - our members
	are FString / TArray and the synchronisation event is unnecessary because nothing here is asynchronous.
=============================================================================*/
#pragma once

#if DISHONORED_WITH_WWISE

#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>

/** Where a bank or a media file may be found: the cooked directory plus the DLC directories. */
class CAkFileLocationBase
{
public:
	virtual ~CAkFileLocationBase() {}

	void SetBasePath( const TCHAR* InBasePath );
	void AddBasePath( const FString& InPath );
	void RemoveAllAdditionalPaths();
	const FString& GetBasePath() const { return m_BasePath; }

	/** TRUE and OutFullPath set when the file exists under the base path or one of the additional ones. */
	UBOOL ResolvePath( const TCHAR* InFileName, FString& OutFullPath ) const;

protected:
	FString			m_BasePath;
	TArray<FString>	m_AdditionalPaths;
};

/** What AkFileDesc::hFile points at: a reader plus where this file starts inside it. */
struct FAkOpenFile
{
	FArchive*	Reader;
	UBOOL		bOwnsReader;		///< FALSE while the reader belongs to a still-loaded file package
	AkInt64		BaseOffset;
};

/** One entry of an AKPK soundbank or streamed-file table (see FAkFilePackage::Load for the evidence). */
struct FAkPackagedFile
{
	AkFileID	FileID;
	AkUInt32	BlockSize;
	AkUInt32	FileSize;
	AkInt64		Offset;
	AkUInt32	LanguageID;
};

/** One loaded AKPK container. */
struct FAkFilePackage
{
	FAkFilePackage() : Reader(NULL), PackageID(0), Version(0) {}
	~FAkFilePackage() { CloseReader(); }

	UBOOL Load( const TCHAR* InFullPath, AkUInt32 InPackageID );
	const FAkPackagedFile* Find( AkFileID InFileID, UBOOL bStreamed ) const;
	void CloseReader();

	FArchive*					Reader;
	FString						FullPath;
	AkUInt32					PackageID;
	AkUInt32					Version;
	TArray<FAkPackagedFile>		SoundBanks;
	TArray<FAkPackagedFile>		StreamedFiles;

private:
	void ReadTable( AkUInt32 TableOffset, AkUInt32 TableSize, TArray<FAkPackagedFile>& OutFiles );
};

/** Blocking IO over loose files, through GFileManager. */
class CAkUnrealIOHookBlocking : public AK::StreamMgr::IAkIOHookBlocking, public AK::StreamMgr::IAkFileLocationResolver, public CAkFileLocationBase
{
public:
	CAkUnrealIOHookBlocking() : m_DeviceID(AK_INVALID_DEVICE_ID) {}
	virtual ~CAkUnrealIOHookBlocking() {}

	// AK::StreamMgr::IAkFileLocationResolver
	virtual AKRESULT Open( AkFileID InFileID, AkOpenMode InOpenMode, AkFileSystemFlags* InFlags, bool& IoSyncOpen, AkFileDesc& OutFileDesc );
	virtual AKRESULT Open( const AkOSChar* InFileName, AkOpenMode InOpenMode, AkFileSystemFlags* InFlags, bool& IoSyncOpen, AkFileDesc& OutFileDesc );

	// AK::StreamMgr::IAkIOHookBlocking
	virtual AKRESULT Read( AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, void* OutBuffer, AkIOTransferInfo& IoTransferInfo );
	virtual AKRESULT Write( AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics, void* InData, AkIOTransferInfo& IoTransferInfo );
	virtual AKRESULT Close( AkFileDesc& InFileDesc );
	virtual AkUInt32 GetBlockSize( AkFileDesc& InFileDesc );
	virtual void GetDeviceDesc( AkDeviceDesc& OutDeviceDesc );
	virtual AkUInt32 GetDeviceData();

protected:
	static void* MakeHandle( FArchive* InReader, UBOOL bOwnsReader, AkInt64 InBaseOffset );

	AkDeviceID m_DeviceID;
};

/** The AKPK layer: banks and media come out of the packages first, loose files second. */
class CAkFilePackageLowLevelIO : public CAkUnrealIOHookBlocking
{
public:
	CAkFilePackageLowLevelIO() : m_NextPackageID(0) {}
	virtual ~CAkFilePackageLowLevelIO() { UnloadAllFilePackages(); }

	AKRESULT Init( const AkDeviceSettings& InSettings );
	void Term();

	AKRESULT LoadFilePackage( const TCHAR* InPackageName, AkUInt32& OutPackageID );
	AKRESULT UnloadFilePackage( AkUInt32 InPackageID );
	AKRESULT UnloadAllFilePackages();

	virtual AKRESULT Open( AkFileID InFileID, AkOpenMode InOpenMode, AkFileSystemFlags* InFlags, bool& IoSyncOpen, AkFileDesc& OutFileDesc );
	virtual AKRESULT Open( const AkOSChar* InFileName, AkOpenMode InOpenMode, AkFileSystemFlags* InFlags, bool& IoSyncOpen, AkFileDesc& OutFileDesc );

	INT GetNumFilePackages() const { return m_Packages.Num(); }

private:
	static void LanguageChangeHandler( const AkOSChar* const InLanguageName, void* InCookie );
	const FAkPackagedFile* FindPackagedFile( AkFileID InFileID, UBOOL bStreamed, FAkFilePackage*& OutPackage ) const;

	TArray<FAkFilePackage*>	m_Packages;
	AkUInt32				m_NextPackageID;
	FString					m_Language;
};

#endif // DISHONORED_WITH_WWISE
