/*=============================================================================
	IAkStreamMgr.h - Wwise 2012.1 streaming interfaces.

	DISHONORED(layout): every vtable below is the 2012 PDB's synthesized *_vtbl record, slot for slot:
	  AK::IAkStreamMgr_vtbl          (28 bytes = dtor, Destroy, GetStreamMgrProfile, CreateStd x2, CreateAuto x2)
	  AK::IAkStdStream_vtbl          (52 bytes)
	  AK::IAkAutoStream_vtbl         (68 bytes)
	  AK::StreamMgr::IAkLowLevelIOHook_vtbl    (20 bytes = dtor, Close, GetBlockSize, GetDeviceDesc, GetDeviceData)
	  AK::StreamMgr::IAkIOHookBlocking_vtbl    (28 bytes = the five above + Read + Write)
	  AK::StreamMgr::IAkFileLocationResolver_vtbl (12 bytes = dtor, Open(AkFileID), Open(const AkOSChar*))
	The parameter types come from the demangled signatures of Arkane's own overrides, e.g.
	  public: virtual enum AKRESULT __thiscall CAkUnrealIOHookBlocking::Open(unsigned long, enum AkOpenMode,
	      struct AkFileSystemFlags *, bool &, struct AkFileDesc &)                      2013 rva 0x5af410
	  public: virtual enum AKRESULT __thiscall CAkUnrealIOHookBlocking::Read(struct AkFileDesc &,
	      struct AkIoHeuristics const &, void *, struct AkIOTransferInfo &)             2013 rva 0x5b0c20
	so the reference/pointer shape of each parameter is retail's, not a guess.
=============================================================================*/
#ifndef _IAK_STREAM_MGR_H_
#define _IAK_STREAM_MGR_H_

#include <AK/SoundEngine/Common/AkTypes.h>

#if defined(_MSC_VER)
#pragma pack (push, 8)
#endif

/** DISHONORED(layout): 20 bytes; bIsFromRSX / bIsAutomaticStream are PS3-era flags Wwise still sets. */
struct AkFileSystemFlags
{
	AkUInt32	uCompanyID;
	AkUInt32	uCodecID;
	AkUInt32	uCustomParamSize;
	void *		pCustomParam;
	bool		bIsLanguageSpecific;
	bool		bIsFromRSX;
	bool		bIsAutomaticStream;
};

/** DISHONORED(layout): 32 bytes, align 8 (iFileSize is AkInt64). */
struct AkFileDesc
{
	AkInt64			iFileSize;
	AkUInt32		uSector;
	AkUInt32		uCustomParamSize;
	void *			pCustomParam;
	AkFileHandle	hFile;
	AkDeviceID		deviceID;
};

/** DISHONORED(layout): 8 bytes; priority is AkPriority (char). */
struct AkIoHeuristics
{
	AkReal32	fDeadline;
	AkPriority	priority;
};

/** DISHONORED(layout): 16 bytes, align 8. */
struct AkIOTransferInfo
{
	AkUInt64	uFilePosition;
	AkUInt32	uBufferSize;
	AkUInt32	uRequestedSize;
};

struct AkAsyncIOTransferInfo;
typedef void ( *AkIOCallback )( AkAsyncIOTransferInfo * in_pTransferInfo, AKRESULT in_eResult );

/** DISHONORED(layout): 32 bytes, derives AkIOTransferInfo (deferred hook only; retail uses the blocking one). */
struct AkAsyncIOTransferInfo : public AkIOTransferInfo
{
	void *			pBuffer;
	AkIOCallback	pCallback;
	void *			pCookie;
	void *			pUserData;
};

/** DISHONORED(layout): 44 bytes; szDeviceName is a 16-wchar_t inline buffer. */
struct AkDeviceDesc
{
	AkDeviceID	deviceID;
	bool		bCanWrite;
	bool		bCanRead;
	AkOSChar	szDeviceName[16];
	AkUInt32	uStringSize;
};

/** DISHONORED(layout): 24 bytes, align 8. */
struct AkStreamInfo
{
	AkDeviceID			deviceID;
	const AkOSChar *	pszName;
	AkUInt64			uSize;
	bool				bIsOpen;
};

/** DISHONORED(layout): 16 bytes; uMinNumBuffers is AkUInt8 and priority AkPriority. */
struct AkAutoStmHeuristics
{
	AkReal32	fThroughput;
	AkUInt32	uLoopStart;
	AkUInt32	uLoopEnd;
	AkUInt8		uMinNumBuffers;
	AkPriority	priority;
};

/** DISHONORED(layout): 12 bytes. */
struct AkAutoStmBufSettings
{
	AkUInt32	uBufferSize;
	AkUInt32	uMinBufferSize;
	AkUInt32	uBlockSize;
};

#if defined(_MSC_VER)
#pragma pack (pop)
#endif

namespace AK
{
	class IAkStreamMgrProfile;
	class IAkStreamProfile;

	/** DISHONORED(layout): AK::IAkStdStream_vtbl, 13 slots after the destructor slot. */
	class IAkStdStream
	{
	protected:
		virtual ~IAkStdStream() {}
	public:
		virtual void Destroy() = 0;
		virtual void GetInfo( AkStreamInfo & out_info ) = 0;
		virtual void * GetFileDescriptor() = 0;
		virtual AKRESULT SetStreamName( const AkOSChar * in_pszStreamName ) = 0;
		virtual AkUInt32 GetBlockSize() = 0;
		virtual AKRESULT Read( void * in_pBuffer, AkUInt32 in_uReqSize, bool in_bWait, AkPriority in_priority, AkReal32 in_fDeadline, AkUInt32 & out_uSize ) = 0;
		virtual AKRESULT Write( void * in_pBuffer, AkUInt32 in_uReqSize, bool in_bWait, AkPriority in_priority, AkReal32 in_fDeadline, AkUInt32 & out_uSize ) = 0;
		virtual AkUInt64 GetPosition( bool * out_pbEndOfStream ) = 0;
		virtual AKRESULT SetPosition( AkInt64 in_iMoveOffset, AkMoveMethod in_eMoveMethod, AkInt64 * out_piRealOffset ) = 0;
		virtual void Cancel() = 0;
		virtual void * GetData( AkUInt32 & out_uSize ) = 0;
		virtual AkStmStatus GetStatus() = 0;
	};

	/** DISHONORED(layout): AK::IAkAutoStream_vtbl, 17 slots after the destructor slot. */
	class IAkAutoStream
	{
	protected:
		virtual ~IAkAutoStream() {}
	public:
		virtual void Destroy() = 0;
		virtual void GetInfo( AkStreamInfo & out_info ) = 0;
		virtual void * GetFileDescriptor() = 0;
		virtual void GetHeuristics( AkAutoStmHeuristics & out_heuristics ) = 0;
		virtual AKRESULT SetHeuristics( const AkAutoStmHeuristics & in_heuristics ) = 0;
		virtual AKRESULT SetMinimalBufferSize( AkUInt32 in_uMinBufferSize ) = 0;
		virtual AKRESULT SetStreamName( const AkOSChar * in_pszStreamName ) = 0;
		virtual AkUInt32 GetBlockSize() = 0;
		virtual AKRESULT QueryBufferingStatus( AkUInt32 & out_uNumBytesAvailable ) = 0;
		virtual AkUInt32 GetNominalBuffering() = 0;
		virtual AKRESULT Start() = 0;
		virtual AKRESULT Stop() = 0;
		virtual AkUInt64 GetPosition( bool * out_pbEndOfStream ) = 0;
		virtual AKRESULT SetPosition( AkInt64 in_iMoveOffset, AkMoveMethod in_eMoveMethod, AkInt64 * out_piRealOffset ) = 0;
		virtual AKRESULT GetBuffer( void *& out_pBuffer, AkUInt32 & out_uSize, bool in_bWait ) = 0;
		virtual AKRESULT ReleaseBuffer() = 0;
	};

	/**
	 * The stream manager singleton. DISHONORED(layout): m_pStreamMgr is a real static of the retail exe
	 * (UAkAudioDevice::Teardown reads AK::IAkStreamMgr::m_pStreamMgr and calls slot 1, Destroy). It is a
	 * plain static with one definition in AkSilentStreamMgr.cpp, never an inline static: this header is
	 * pulled into AkAudio, Engine and DishonoredGame and the Engine link is close to its size limit.
	 */
	class IAkStreamMgr
	{
	public:
		static IAkStreamMgr * Get() { return m_pStreamMgr; }

	protected:
		virtual ~IAkStreamMgr() {}
	public:
		virtual void Destroy() = 0;
		virtual IAkStreamMgrProfile * GetStreamMgrProfile() = 0;
		virtual AKRESULT CreateStd( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream, bool in_bSyncOpen ) = 0;
		virtual AKRESULT CreateStd( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream, bool in_bSyncOpen ) = 0;
		virtual AKRESULT CreateAuto( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream, bool in_bSyncOpen ) = 0;
		virtual AKRESULT CreateAuto( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream, bool in_bSyncOpen ) = 0;

	protected:
		static IAkStreamMgr * m_pStreamMgr;
	};

	namespace StreamMgr
	{
		/** DISHONORED(layout): IAkFileLocationResolver_vtbl, two Open overloads after the destructor. */
		class IAkFileLocationResolver
		{
		protected:
			virtual ~IAkFileLocationResolver() {}
		public:
			virtual AKRESULT Open( AkFileID in_fileID, AkOpenMode in_eOpenMode, AkFileSystemFlags * in_pFlags, bool & io_bSyncOpen, AkFileDesc & out_fileDesc ) = 0;
			virtual AKRESULT Open( const AkOSChar * in_pszFileName, AkOpenMode in_eOpenMode, AkFileSystemFlags * in_pFlags, bool & io_bSyncOpen, AkFileDesc & out_fileDesc ) = 0;
		};

		/** DISHONORED(layout): IAkLowLevelIOHook_vtbl, four slots after the destructor. */
		class IAkLowLevelIOHook
		{
		protected:
			virtual ~IAkLowLevelIOHook() {}
		public:
			virtual AKRESULT Close( AkFileDesc & in_fileDesc ) = 0;
			virtual AkUInt32 GetBlockSize( AkFileDesc & in_fileDesc ) = 0;
			virtual void GetDeviceDesc( AkDeviceDesc & out_deviceDesc ) = 0;
			virtual AkUInt32 GetDeviceData() = 0;
		};

		/** DISHONORED(layout): IAkIOHookBlocking_vtbl = IAkLowLevelIOHook + Read + Write. Retail's hook. */
		class IAkIOHookBlocking : public IAkLowLevelIOHook
		{
		public:
			virtual AKRESULT Read( AkFileDesc & in_fileDesc, const AkIoHeuristics & in_heuristics, void * out_pBuffer, AkIOTransferInfo & io_transferInfo ) = 0;
			virtual AKRESULT Write( AkFileDesc & in_fileDesc, const AkIoHeuristics & in_heuristics, void * in_pData, AkIOTransferInfo & io_transferInfo ) = 0;
		};

		/** DISHONORED(layout): IAkIOHookDeferred_vtbl = IAkLowLevelIOHook + Read + Write + Cancel. Unused by retail. */
		class IAkIOHookDeferred : public IAkLowLevelIOHook
		{
		public:
			virtual AKRESULT Read( AkFileDesc & in_fileDesc, const AkIoHeuristics & in_heuristics, AkAsyncIOTransferInfo & io_transferInfo ) = 0;
			virtual AKRESULT Write( AkFileDesc & in_fileDesc, const AkIoHeuristics & in_heuristics, AkAsyncIOTransferInfo & io_transferInfo ) = 0;
			virtual void Cancel( AkFileDesc & in_fileDesc, AkAsyncIOTransferInfo & io_transferInfo, bool & io_bCancelAllTransfersForThisFile ) = 0;
		};
	}
}

#if defined(__cplusplus) && !defined(AK_NO_LAYOUT_CHECKS)
static_assert(sizeof(AkFileSystemFlags) == 20, "AkFileSystemFlags: 2012 PDB sizeof 20");
static_assert(sizeof(AkFileDesc) == 32, "AkFileDesc: 2012 PDB sizeof 32");
static_assert(sizeof(AkIoHeuristics) == 8, "AkIoHeuristics: 2012 PDB sizeof 8");
static_assert(sizeof(AkIOTransferInfo) == 16, "AkIOTransferInfo: 2012 PDB sizeof 16");
static_assert(sizeof(AkAsyncIOTransferInfo) == 32, "AkAsyncIOTransferInfo: 2012 PDB sizeof 32");
static_assert(sizeof(AkDeviceDesc) == 44, "AkDeviceDesc: 2012 PDB sizeof 44");
static_assert(sizeof(AkStreamInfo) == 24, "AkStreamInfo: 2012 PDB sizeof 24");
static_assert(sizeof(AkAutoStmHeuristics) == 16, "AkAutoStmHeuristics: 2012 PDB sizeof 16");
static_assert(sizeof(AkAutoStmBufSettings) == 12, "AkAutoStmBufSettings: 2012 PDB sizeof 12");
#endif

#endif // _IAK_STREAM_MGR_H_
