/*=============================================================================
	AkBankMgr.h - CAkBankMgr and what it is made of: CAkBankReader, CAkUsageSlot, CAkBankList,
	CAkBankCallbackMgr, AkMediaEntry.

	DISHONORED(port): agent IH1 (wave 26); wwise\sdk\source\soundengine\akaudiolib\common\akbankmgr.cpp
	(2012 0x929870..0x92e690), akbankreader.cpp (0x91e240..0x91e9c0), akbanklist.cpp (0x94d4b0..0x94d5d0),
	akbankcallbackmgr.cpp (0x91ea80..0x91ee20). Layouts are the PDB's (CAkUsageSlot 0x4C, CAkBankReader 0x28,
	AkBankQueueItem 36 bytes); the bank manager runs its own thread "AK::BankManager" with the properties
	in g_PDSettings.threadBankManager, and every synchronous API call queues an item and waits on it exactly
	as retail (AK::SoundEngine::AkSyncCaller, DefaultBankCallbackFunc).

	Deviations (named):
	  * KillSlot hands the slot to the audio manager (message 26: CAkURenderer::StopAllPBIs + Release) so the
	    unload happens between two audio frames; the audio manager is IH2's, so the slot is released directly
	    (no PBI can reference it before IH2).
	  * Media preparation (PrepareMedia / LoadSingleMedia / PrepareEvent's PrepareNodeData) reads streamed
	    media into the prepared-event pool for PrepareBank(AllInclusive) and PrepareEvent. Dishonored only
	    prepares banks StructureOnly (Engine akbank.cpp: AkBankContent_StructureOnly) and never calls
	    PrepareEvent (measured: UAkAudioDevice::PrepareEvent has no caller); those paths queue and complete
	    with AK_Success after the structure part, and -akdiag says so.
	  * The feedback (motion) master bus swap around a load and CAkFeedbackBus are not ported: no Dishonored
	    bank has bFeedbackInBank set (wwise_bank_dump.py --all).
	  * -akdiag: every load / prepare / unload logs bank name, id, size, result and HIRC counts by type.
=============================================================================*/
#pragma once

#include "AkHierarchy.h"
#include <AK/SoundEngine/Common/IAkStreamMgr.h>

class CAkUsageSlot;

/** 0x28 bytes. */
class CAkBankReader
{
public:
	CAkBankReader();

	AKRESULT Init();
	AKRESULT Term();
	void Reset();
	AKRESULT SetBankLoadIOSettings( AkReal32 in_fThroughput, AkPriority in_priority );
	AKRESULT SetFile( const void * in_pInMemoryFile, AkUInt32 in_ui32FileSize );
	AKRESULT SetFile( const char * in_pszFilename, AkUInt32 in_uFileOffset, void * in_pCookie );
	AKRESULT SetFile( AkFileID in_FileID, AkUInt32 in_uFileOffset, AkUInt32 in_codecID, void * in_pCookie, bool in_bIsLanguageSpecific );
	AKRESULT CloseFile();
	const void * GetData( AkUInt32 in_uSize );
	void ReleaseData();
	AKRESULT FillData( void * in_pBufferToFill, AkUInt32 in_ulSizeToRead, AkUInt32 & out_rulSizeRead );
	AKRESULT FillDataEx( void * in_pBufferToFill, AkUInt32 in_ulSizeToRead );
	AKRESULT Skip( AkUInt32 in_ulNumBytesToSkip, AkUInt32 & out_rulSizeSkipped );

	AkUInt8 *			m_pBuffer;
	AkUInt8 *			m_pReadBufferPtr;
	AkUInt32			m_ulRemainingBytes;
	AkUInt32			m_ulBufferSize;
	AkUInt32			m_ulDeviceBlockSize;
	void *				m_pUserReadBuffer;
	const AkUInt8 *		m_pInMemoryBankReaderPtr;
	AK::IAkStdStream *	m_pStream;
	AkReal32			m_fThroughput;
	AkPriority			m_priority;
	bool				m_bIsInitDone;

private:
	AKRESULT OpenedStream();
};

struct AkMediaInfo
{
	AkUInt8 *	pInMemoryData;
	AkUInt32	uInMemoryDataSize;
};

/** 0x4C bytes. */
class CAkUsageSlot
{
public:
	CAkUsageSlot( AkBankID in_BankID, AkMemPoolId in_memPoolId, AkInt32 in_mainRefCount, AkInt32 in_prepareEventRefCount, AkInt32 in_prepareBankRefCount );

	void Unload();
	void UnloadCompletionNotification();
	void RemoveContent();
	void Release( bool in_bSkipNotification );
	void ReleasePrepare( bool in_bIsFinal );
	void FreeAndDelete();

	AkBankID										key;
	CAkUsageSlot *									pNextItem;
	AkBankID										m_BankID;
	AkUInt8 *										m_pData;
	AkUInt32										m_uLoadedDataSize;
	AkMemPoolId										m_memPoolId;
	bool											m_bIsInternalPool;
	AkUInt32										m_uNumLoadedItems;
	AkUInt32										m_uIndexSize;
	AkBank::MediaHeader *							m_paLoadedMedia;
	AkBankCallbackFunc								m_pfnBankCallback;
	void *											m_pCookie;
	AkArray<CAkIndexable *, CAkIndexable *, ArrayPoolDefault, 0>	m_listLoadedItem;
	volatile LONG									m_iRefCount;
	volatile LONG									m_iPrepareRefCount;
	AkUInt8											m_bWasLoadedAsABank : 1;
	AkUInt8											m_bWasIndexAllocated : 1;
	AkUInt8											m_bIsMediaPrepared : 1;
	AkUInt8											m_bUsageProhibited : 1;
	AkInt32											m_iWasPreparedAsABankCounter;
};

class CAkBankList
{
public:
	void Init() { m_ListLoadedBanks.Init(); }
	void Term() { m_ListLoadedBanks.Term(); }
	void Set( AkBankID in_BankID, CAkUsageSlot * in_pSlot );
	CAkUsageSlot * Get( AkBankID in_BankID );
	void Remove( AkBankID in_BankID );

	static CAkLock m_BankListLock;
	AkHashListBare<AkBankID, CAkUsageSlot, 31> m_ListLoadedBanks;
};

class CAkBankCallbackMgr
{
public:
	struct BankCallbackItem
	{
		BankCallbackItem() : m_cookieCount( 1 ), m_toSkipCount( 0 ) {}
		AkUInt32 m_cookieCount;
		AkUInt32 m_toSkipCount;
	};

	CAkBankCallbackMgr();
	~CAkBankCallbackMgr();

	AKRESULT AddCookie( void * in_cookie );
	void RemoveOneCookie( void * in_cookie );
	void DoCallback( AkBankCallbackFunc in_pfnBankCallback, AkBankID in_bankID, AKRESULT in_eLoadResult, AkMemPoolId in_memPoolId, void * in_pCookie );
	void CancelCookie( void * in_cookie );

	CAkKeyArray<void *, BankCallbackItem, 5>	m_ListCookies;
	CAkLock										m_csLock;
	HANDLE										m_CallbackActiveEvent;
};

class AkMediaEntry
{
public:
	AkMediaEntry() : uRefCount( 1 ), m_sourceID( AK_INVALID_UNIQUE_ID )
	{
		m_preparedMediaInfo.pInMemoryData = NULL;
		m_preparedMediaInfo.uInMemoryDataSize = 0;
	}
	~AkMediaEntry() { m_BankSlots.Term(); }

	void GetMedia( AkMediaInfo & out_mediaInfo, CAkUsageSlot *& out_pUsageSlot );

	static AkMemPoolId m_uPrepareEventMemoryPoolID;

	AkMediaInfo									m_preparedMediaInfo;
	CAkKeyArray<CAkUsageSlot *, AkMediaInfo, 1>	m_BankSlots;
	AkUInt32									uRefCount;
	AkUniqueID									m_sourceID;
};

class CAkBankMgr
{
public:
	enum AkBankQueueItemType
	{
		QueueItemLoad					= 0,
		QueueItemUnload					= 1,
		QueueItemPrepareEvent			= 2,
		QueueItemUnprepareEvent			= 3,
		QueueItemSupportedGameSync		= 4,
		QueueItemUnprepareAllEvents		= 5,
		QueueItemPrepareBank			= 6,
		QueueItemUnprepareBank			= 7,
		QueueItemClearBanks				= 8,
		QueueItemLoadMediaFile			= 9,
		QueueItemUnloadMediaFile		= 10,
		QueueItemLoadMediaFileSwap		= 11
	};

	enum AkLoadBankDataMode
	{
		AkLoadBankDataMode_OneBlock				= 0,
		AkLoadBankDataMode_MediaAndStructure	= 1,
		AkLoadBankDataMode_Structure			= 2,
		AkLoadBankDataMode_Media				= 3
	};

	enum AkBankLoadFlag
	{
		AkBankLoadFlag_None			= 0,
		AkBankLoadFlag_InMemory		= 1,
		AkBankLoadFlag_UsingFileID	= 2
	};

	struct AkBankCompletionNotifInfo
	{
		AkBankCallbackFunc	pfnBankCallback;
		void *				pCookie;
	};

	/** 36 bytes; the union overlays at +16 exactly as the PDB's anonymous union. */
	struct AkBankQueueItem
	{
		AkBankQueueItemType			eType;
		AkBankCompletionNotifInfo	callbackInfo;
		AkBankLoadFlag				bankLoadFlag;
		union
		{
			struct
			{
				AkUInt32		numEvents;
				union
				{
					AkUniqueID		eventID;
					AkUniqueID *	pEventID;
				};
			} prepare;
			struct
			{
				AkBankID		bankID;
				AkMemPoolId		memPoolId;
				const void *	pInMemoryBank;
				AkUInt32		ui32InMemoryBankSize;
			} load;
			struct
			{
				AkUInt32		eGroupType;
				AkUInt32		uGroupID;
				bool			bSupported;
				AkUInt32		uNumGameSync;
				AkUInt32		uGameSyncID;
			} gameSync;
		};
	};

	CAkBankMgr();
	~CAkBankMgr();

	AKRESULT Init();
	AKRESULT Term();
	void StopThread();

	AKRESULT QueueBankCommand( AkBankQueueItem in_Item );
	void NotifyCompletion( AkBankQueueItem & in_rItem, AKRESULT in_OperationResult );
	void UpdateBankName( AkBankID in_bankID, const char * in_pStringWithoutExtension );
	AkMediaInfo GetMedia( AkUniqueID in_mediaId, CAkUsageSlot *& out_pUsageSlot );
	void UnloadMedia( CAkUsageSlot * in_pCurrentSlot );
	void UnPrepareMedia( CAkUsageSlot * in_pCurrentSlot );

	static AKRESULT LoadSource( AkUInt8 *& io_pData, AkUInt32 & io_ulDataSize, AkBankSourceData & out_rSource );
	static AkBankID GetBankIDFromInMemorySpace( const void * in_pData, AkUInt32 in_uSize );

	const char * BankFileName( AkBankID in_bankID );

	CAkBankReader										m_BankReader;
	bool												m_bStopThread;
	CAkLock												m_queueLock;
	CAkLock												m_MediaLock;
	CAkList2<AkBankQueueItem, const AkBankQueueItem &, 1>	m_BankQueue;
	bool												m_bIsFirstBusLoaded;
	AkHashList<AkUniqueID, AkMediaEntry, 193>			m_MediaHashTable;
	CAkBankList											m_BankList;
	AkHashList<AkBankID, char *, 31>					m_BankIDToFileName;
	bool												m_bFeedbackInBank;
	CAkBankCallbackMgr									m_CallbackMgr;
	bool												m_bAccumulating;
	HANDLE												m_eventQueue;

	static AkThread		m_BankMgrThread;
	static AkThreadID	m_idThread;

	/** -akdiag: what the HIRC chunk of the bank being loaded contained, by HIRC type, and the bytes read. */
	AkUInt32	m_aHircCounts[ AkBank::HIRCType_Count ];
	AkUInt32	m_aHircFailures[ AkBank::HIRCType_Count ];
	AkUInt32	m_aHircMisparsed[ AkBank::HIRCType_Count ];
	AkUInt32	m_uLoadBytes;

private:
	static DWORD WINAPI BankThreadFunc( LPVOID lpParameter );
	AKRESULT StartThread();
	AKRESULT ExecuteCommand();

	AKRESULT LoadBankPre( AkBankQueueItem & in_rItem );
	AKRESULT UnloadBankPre( AkBankQueueItem in_Item );
	AKRESULT ClearBanksInternal( AkBankQueueItem in_Item );
	AKRESULT PrepareEvents( AkBankQueueItem in_Item );
	AKRESULT UnprepareEvents( AkBankQueueItem in_Item );
	AKRESULT PrepareEvent( AkBankQueueItem in_Item, AkUniqueID in_EventID );
	AKRESULT UnprepareEvent( AkUniqueID in_EventID );
	void UnprepareEvent( CAkEvent * in_pEvent, bool in_bCompleteUnprepare );
	AKRESULT PrepareBank( AkBankQueueItem in_Item );
	AKRESULT UnPrepareBank( AkBankQueueItem in_Item );
	AKRESULT PrepareBankInternal( AkBankQueueItem in_Item, AkBankID in_FileID, AkLoadBankDataMode in_LoadBankMode, bool in_bIsFromPrepareBank );
	void UnPrepareAllBank();
	void ClearPreparedEvents();
	void UnloadAll();
	void FlushFileNameTable();

	AKRESULT LoadBank( AkBankQueueItem in_Item, CAkUsageSlot *& out_pUsageSlot, AkLoadBankDataMode in_eLoadMode, bool in_bIsFromPrepareBank );
	AKRESULT ProcessBankHeader( AkBank::AkBankHeader & in_rBankHeader );
	AKRESULT ProcessDataChunk( AkUInt32 in_dwDataChunkSize, CAkUsageSlot * in_pUsageSlot );
	AKRESULT ProcessGlobalSettingsChunk( AkUInt32 in_dwDataChunkSize, CAkUsageSlot * in_pUsageSlot );
	AKRESULT ProcessEnvSettingsChunk( AkUInt32 in_dwDataChunkSize );
	AKRESULT ProcessHircChunk( CAkUsageSlot * in_pUsageSlot, AkUInt32 in_dwBankID );
	AKRESULT ProcessStringMappingChunk( AkUInt32 in_dwDataChunkSize, CAkUsageSlot * in_pUsageSlot );
	AKRESULT LoadMediaIndex( CAkUsageSlot * in_pCurrentSlot, AkUInt32 in_uIndexChunkSize, bool in_bIsLoadedFromMemory );
	AKRESULT LoadMedia( AkUInt8 * in_pDataBank, CAkUsageSlot * in_pCurrentSlot, bool in_bIsLoadedFromMemory );
	AKRESULT KillSlot( CAkUsageSlot * in_pUsageSlot, AkBankCallbackFunc in_pCallBack, void * in_pCookie );

	template <class T> AKRESULT StdBankRead( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot, T * ( *in_pfnCreate )( AkUniqueID ), CAkIndexItemBase & in_rIndex );
	AKRESULT ReadSourceParentSound( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot );
	AKRESULT ReadState( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot );
	AKRESULT ReadBus( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot );
	AKRESULT ReadAction( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot );
	AKRESULT ReadEvent( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot );
	AKRESULT SkipSection( const AkBank::AKBKSubHircSection & in_rSection );
	void AddLoadedItem( CAkUsageSlot * in_pUsageSlot, CAkIndexable * in_pIndexable );
	void CheckParseEnd( const AkBank::AKBKSubHircSection & in_rSection, const AkUInt8 * in_pData );
	void LogBank( const wchar_t * in_szWhat, AkBankID in_bankID, AKRESULT in_eResult );
};
