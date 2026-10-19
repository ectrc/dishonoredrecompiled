/*=============================================================================
	AkBankMgr.cpp - the bank manager thread, its queue, bank loading / preparing / unloading, the HIRC
	readers, the media index and the bank callback manager.

	DISHONORED(port): agent IH1 (wave 26). Function-by-function from the 2012 decompile; each definition names
	its rva. Deviations are listed in AkBankMgr.h.
=============================================================================*/
#include "AkBankMgr.h"
#include "AkAudioMgr.h"

AkThread		CAkBankMgr::m_BankMgrThread = NULL;
AkThreadID		CAkBankMgr::m_idThread = 0;
CAkLock			CAkBankList::m_BankListLock;
AkMemPoolId		AkMediaEntry::m_uPrepareEventMemoryPoolID = AK_INVALID_POOL_ID;

static const wchar_t * HircTypeName( AkUInt32 in_eType )
{
	static const wchar_t * s_names[ AkBank::HIRCType_Count ] =
	{
		L"?", L"State", L"Sound", L"Action", L"Event", L"RanSeqCntr", L"SwitchCntr", L"ActorMixer", L"Bus",
		L"LayerCntr", L"MusicSegment", L"MusicTrack", L"MusicSwitchCntr", L"MusicRanSeqCntr", L"Attenuation",
		L"DialogueEvent", L"FeedbackBus", L"FeedbackNode", L"FxShareSet", L"FxCustom"
	};
	return in_eType < AkBank::HIRCType_Count ? s_names[ in_eType ] : L"?";
}

// ------------------------------------------------------------------------------------------------------------
// CAkUsageSlot
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x92a3f0. */
CAkUsageSlot::CAkUsageSlot( AkBankID in_BankID, AkMemPoolId in_memPoolId, AkInt32 in_mainRefCount, AkInt32 in_prepareEventRefCount, AkInt32 in_prepareBankRefCount )
	: key( in_BankID )
	, pNextItem( NULL )
	, m_BankID( in_BankID )
	, m_pData( NULL )
	, m_uLoadedDataSize( 0 )
	, m_memPoolId( in_memPoolId )
	, m_bIsInternalPool( false )
	, m_uNumLoadedItems( 0 )
	, m_uIndexSize( 0 )
	, m_paLoadedMedia( NULL )
	, m_pfnBankCallback( NULL )
	, m_pCookie( NULL )
	, m_iRefCount( in_mainRefCount )
	, m_iPrepareRefCount( in_prepareBankRefCount + in_prepareEventRefCount )
	, m_bWasLoadedAsABank( in_mainRefCount != 0 )
	, m_bWasIndexAllocated( 0 )
	, m_bIsMediaPrepared( 0 )
	, m_bUsageProhibited( 0 )
	, m_iWasPreparedAsABankCounter( in_prepareBankRefCount )
{
}

/** 2012 0x929870: the DATA block goes back to its pool; an internal pool is destroyed with it. */
void CAkUsageSlot::Unload()
{
	if( !m_pData )
	{
		return;
	}
	if( AK::MemoryMgr::GetPoolAttributes( m_memPoolId ) & AkFixedSizeBlocksMode )
	{
		AK::MemoryMgr::ReleaseBlock( m_memPoolId, m_pData );
	}
	else
	{
		AK::MemoryMgr::Free( m_memPoolId, m_pData );
	}
	m_pData = NULL;
	if( m_bIsInternalPool )
	{
		AK::MemoryMgr::DestroyPool( m_memPoolId );
		m_memPoolId = AK_INVALID_POOL_ID;
	}
}

/** 2012 0x9298d0. */
void CAkUsageSlot::UnloadCompletionNotification()
{
	if( m_pfnBankCallback )
	{
		g_pBankManager->m_CallbackMgr.DoCallback( m_pfnBankCallback, m_BankID, AK_Success, m_memPoolId, m_pCookie );
		m_pfnBankCallback = NULL;
	}
}

/** 2012 0x92a880: release every object the bank created, a few at a time under g_csMain. */
void CAkUsageSlot::RemoveContent()
{
	CAkIndexable ** it = m_listLoadedItem.m_pItems;
	while( it != m_listLoadedItem.m_pItemsEnd )
	{
		AkUInt8 uBatch = 1;
		AkAutoLock<CAkLock> gate( g_csMain );
		for( ; it != m_listLoadedItem.m_pItemsEnd && uBatch; ++uBatch, ++it )
		{
			( *it )->Release();
		}
	}
	m_listLoadedItem.Term();
}

/** The tail Release / ReleasePrepare share: free the media index if the slot owns it, then the slot. */
void CAkUsageSlot::FreeAndDelete()
{
	if( m_bWasIndexAllocated )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, m_paLoadedMedia );
		m_bWasIndexAllocated = 0;
	}
	m_paLoadedMedia = NULL;
	this->~CAkUsageSlot();
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

/** 2012 0x92cbb0. */
void CAkUsageSlot::Release( bool in_bSkipNotification )
{
	CAkBankList::m_BankListLock.Lock();
	if( InterlockedDecrement( &m_iRefCount ) > 0 )
	{
		CAkBankList::m_BankListLock.Unlock();
		return;
	}
	Unload();
	g_pBankManager->UnloadMedia( this );
	const bool bStillPrepared = m_iPrepareRefCount > 0;
	CAkBankList::m_BankListLock.Unlock();
	if( !in_bSkipNotification )
	{
		UnloadCompletionNotification();
	}
	if( !bStillPrepared )
	{
		FreeAndDelete();
	}
}

/** 2012 0x92cc60. */
void CAkUsageSlot::ReleasePrepare( bool in_bIsFinal )
{
	CAkBankList::m_BankListLock.Lock();
	if( in_bIsFinal )
	{
		m_iPrepareRefCount = 0;
	}
	else if( InterlockedDecrement( &m_iPrepareRefCount ) > 0 )
	{
		CAkBankList::m_BankListLock.Unlock();
		return;
	}
	g_pBankManager->UnPrepareMedia( this );
	if( m_iRefCount > 0 )
	{
		CAkBankList::m_BankListLock.Unlock();
		return;
	}
	g_pBankManager->m_BankList.Remove( m_BankID );
	CAkBankList::m_BankListLock.Unlock();
	RemoveContent();
	Unload();
	FreeAndDelete();
}

// ------------------------------------------------------------------------------------------------------------
// CAkBankList (akbanklist.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x94d560. */
void CAkBankList::Set( AkBankID in_BankID, CAkUsageSlot * in_pSlot )
{
	AkAutoLock<CAkLock> gate( m_BankListLock );
	if( !m_ListLoadedBanks.Exists( in_BankID ) )
	{
		in_pSlot->key = in_BankID;
		m_ListLoadedBanks.Set( in_pSlot );
	}
}

/** 2012 0x94d5d0. */
CAkUsageSlot * CAkBankList::Get( AkBankID in_BankID )
{
	AkAutoLock<CAkLock> gate( m_BankListLock );
	return m_ListLoadedBanks.Exists( in_BankID );
}

/** 2012 0x94d530. */
void CAkBankList::Remove( AkBankID in_BankID )
{
	AkAutoLock<CAkLock> gate( m_BankListLock );
	m_ListLoadedBanks.Unset( in_BankID );
}

// ------------------------------------------------------------------------------------------------------------
// CAkBankCallbackMgr (akbankcallbackmgr.cpp)
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x91ebb0: the "callback active" event is manual-reset and starts non-signalled. */
CAkBankCallbackMgr::CAkBankCallbackMgr()
{
	m_CallbackActiveEvent = CreateEventW( NULL, TRUE, FALSE, NULL );
}

/** 2012 0x91ec40. */
CAkBankCallbackMgr::~CAkBankCallbackMgr()
{
	m_ListCookies.Term();
	if( m_CallbackActiveEvent )
	{
		CloseHandle( m_CallbackActiveEvent );
	}
}

/** 2012 0x91ec90. */
AKRESULT CAkBankCallbackMgr::AddCookie( void * in_cookie )
{
	AkAutoLock<CAkLock> gate( m_csLock );
	BankCallbackItem * pItem = m_ListCookies.Exists( in_cookie );
	if( pItem )
	{
		++pItem->m_cookieCount;
		return AK_Success;
	}
	return m_ListCookies.Set( in_cookie ) ? AK_Success : AK_InsufficientMemory;
}

/** 2012 0x91ed00. */
void CAkBankCallbackMgr::RemoveOneCookie( void * in_cookie )
{
	{
		AkAutoLock<CAkLock> gate( m_csLock );
		BankCallbackItem * pItem = m_ListCookies.Exists( in_cookie );
		if( pItem )
		{
			if( pItem->m_cookieCount > 1 )
			{
				--pItem->m_cookieCount;
			}
			else
			{
				m_ListCookies.Unset( in_cookie );
			}
		}
	}
	if( CAkBankMgr::m_idThread != GetCurrentThreadId() )
	{
		WaitForSingleObject( m_CallbackActiveEvent, INFINITE );
	}
}

/** 2012 0x91ed70: a cancelled cookie consumes its callback without calling it. */
void CAkBankCallbackMgr::DoCallback( AkBankCallbackFunc in_pfnBankCallback, AkBankID in_bankID, AKRESULT in_eLoadResult, AkMemPoolId in_memPoolId, void * in_pCookie )
{
	if( !in_pfnBankCallback )
	{
		return;
	}
	m_csLock.Lock();
	BankCallbackItem * pItem = m_ListCookies.Exists( in_pCookie );
	if( !pItem )
	{
		m_csLock.Unlock();
		return;
	}
	const bool bSkip = pItem->m_toSkipCount != 0;
	if( pItem->m_cookieCount > 1 )
	{
		--pItem->m_cookieCount;
		if( pItem->m_toSkipCount )
		{
			--pItem->m_toSkipCount;
		}
	}
	else
	{
		m_ListCookies.Unset( in_pCookie );
	}
	ResetEvent( m_CallbackActiveEvent );
	m_csLock.Unlock();
	if( !bSkip )
	{
		in_pfnBankCallback( in_bankID, in_eLoadResult, in_memPoolId, in_pCookie );
	}
	SetEvent( m_CallbackActiveEvent );
}

/** 2012 0x91ee20. */
void CAkBankCallbackMgr::CancelCookie( void * in_cookie )
{
	{
		AkAutoLock<CAkLock> gate( m_csLock );
		BankCallbackItem * pItem = m_ListCookies.Exists( in_cookie );
		if( pItem )
		{
			pItem->m_toSkipCount = pItem->m_cookieCount;
		}
	}
	if( CAkBankMgr::m_idThread != GetCurrentThreadId() )
	{
		WaitForSingleObject( m_CallbackActiveEvent, INFINITE );
	}
}

// ------------------------------------------------------------------------------------------------------------
// AkMediaEntry
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x92a5f0: a reference on the entry (CAkBankMgr::ReleaseMedia drops it); prepared data wins, else the first
    bank slot that is not being unloaded, which is referenced too. Nothing found drops the reference again.
    DISHONORED(port, agent IH2): IH1 took neither reference and did not skip a slot being unloaded. */
void AkMediaEntry::GetMedia( AkMediaInfo & out_mediaInfo, CAkUsageSlot *& out_pUsageSlot )
{
	++uRefCount;
	if( m_preparedMediaInfo.pInMemoryData )
	{
		out_mediaInfo = m_preparedMediaInfo;
		return;
	}
	for( AkUInt32 i = 0; i < m_BankSlots.Length(); ++i )
	{
		if( !m_BankSlots.m_pItems[ i ].key->m_bUsageProhibited )
		{
			out_pUsageSlot = m_BankSlots.m_pItems[ i ].key;
			out_mediaInfo = m_BankSlots.m_pItems[ i ].item;
			InterlockedIncrement( &out_pUsageSlot->m_iRefCount );
			return;
		}
	}
	out_mediaInfo.pInMemoryData = NULL;
	out_mediaInfo.uInMemoryDataSize = 0;
	if( uRefCount-- == 1 && m_preparedMediaInfo.pInMemoryData )
	{
		AK::MemoryMgr::Free( m_uPrepareEventMemoryPoolID, m_preparedMediaInfo.pInMemoryData );
		m_preparedMediaInfo.pInMemoryData = NULL;
		m_preparedMediaInfo.uInMemoryDataSize = 0;
	}
}

// ------------------------------------------------------------------------------------------------------------
// CAkBankMgr
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x92a900. */
CAkBankMgr::CAkBankMgr()
	: m_bStopThread( false )
	, m_bIsFirstBusLoaded( false )
	, m_bFeedbackInBank( false )
	, m_bAccumulating( false )
	, m_eventQueue( NULL )
	, m_uLoadBytes( 0 )
{
	memset( m_aHircCounts, 0, sizeof( m_aHircCounts ) );
	memset( m_aHircFailures, 0, sizeof( m_aHircFailures ) );
	memset( m_aHircMisparsed, 0, sizeof( m_aHircMisparsed ) );
	m_BankMgrThread = NULL;
}

/** 2012 0x92a460. */
CAkBankMgr::~CAkBankMgr()
{
}

/** 2012 0x92e690. */
AKRESULT CAkBankMgr::Init()
{
	AkMediaEntry::m_uPrepareEventMemoryPoolID = g_settings.uPrepareEventMemoryPoolID == AK_INVALID_POOL_ID ? g_DefaultPoolId : g_settings.uPrepareEventMemoryPoolID;
	m_BankList.Init();
	AKRESULT eResult = m_BankReader.Init();
	if( eResult != AK_Success )
	{
		return eResult;
	}
	m_MediaHashTable.Init( g_DefaultPoolId );
	m_BankIDToFileName.Init( g_DefaultPoolId );
	eResult = m_BankQueue.Init( 10, (AkUInt32)-1 );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	return StartThread();
}

/** 2012 0x92e630. */
AKRESULT CAkBankMgr::StartThread()
{
	if( m_BankMgrThread )
	{
		return AK_Fail;
	}
	m_bStopThread = false;
	m_eventQueue = CreateEventW( NULL, FALSE, FALSE, NULL );
	if( !m_eventQueue )
	{
		return AK_Fail;
	}
	AKPLATFORM::AkCreateThread( BankThreadFunc, this, g_PDSettings.threadBankManager, &m_BankMgrThread, "AK::BankManager" );
	return m_BankMgrThread ? AK_Success : AK_Fail;
}

/** 2012 0x929910. */
void CAkBankMgr::StopThread()
{
	m_bStopThread = true;
	if( m_BankMgrThread )
	{
		SetEvent( m_eventQueue );
		WaitForSingleObject( m_BankMgrThread, INFINITE );
		CloseHandle( m_BankMgrThread );
		m_BankMgrThread = NULL;
	}
	if( m_eventQueue )
	{
		CloseHandle( m_eventQueue );
	}
	m_eventQueue = NULL;
}

/** 2012 0x92e5e0. */
DWORD WINAPI CAkBankMgr::BankThreadFunc( LPVOID lpParameter )
{
	CAkBankMgr * pBankMgr = (CAkBankMgr *)lpParameter;
	m_idThread = GetCurrentThreadId();
	WaitForSingleObject( pBankMgr->m_eventQueue, INFINITE );
	while( !pBankMgr->m_bStopThread )
	{
		pBankMgr->ExecuteCommand();
		WaitForSingleObject( pBankMgr->m_eventQueue, INFINITE );
	}
	return 0;
}

/** 2012 0x92e010. */
AKRESULT CAkBankMgr::Term()
{
	while( m_BankQueue.Length() )
	{
		const AkBankQueueItem Item = m_BankQueue.First();
		m_BankQueue.RemoveFirst();
		if( ( Item.eType == QueueItemLoadMediaFile || Item.eType == QueueItemLoadMediaFileSwap ) && Item.prepare.eventID )
		{
			AK::MemoryMgr::Free( g_DefaultPoolId, Item.prepare.pEventID );
		}
	}
	m_BankQueue.Term();
	UnloadAll();
	m_BankList.Term();
	m_MediaHashTable.Term();
	m_PreparationAccumulator.Term();
	FlushFileNameTable();
	m_BankReader.Term();
	return AK_Success;
}

/** 2012 0x92b060. */
AKRESULT CAkBankMgr::QueueBankCommand( AkBankQueueItem in_Item )
{
	AkAutoLock<CAkLock> gate( m_queueLock );
	AKRESULT eResult = AK_Success;
	if( in_Item.callbackInfo.pfnBankCallback )
	{
		eResult = m_CallbackMgr.AddCookie( in_Item.callbackInfo.pCookie );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	eResult = m_BankQueue.AddLast( in_Item ) ? AK_Success : AK_Fail;
	if( eResult == AK_Success )
	{
		SetEvent( m_eventQueue );
	}
	else if( in_Item.callbackInfo.pfnBankCallback )
	{
		m_CallbackMgr.RemoveOneCookie( in_Item.callbackInfo.pCookie );
	}
	return eResult;
}

/** 2012 0x92e3d0. */
AKRESULT CAkBankMgr::ExecuteCommand()
{
	AKRESULT eResult = AK_Success;
	m_queueLock.Lock();
	while( m_BankQueue.Length() )
	{
		AkBankQueueItem Item = m_BankQueue.First();
		m_BankQueue.RemoveFirst();
		m_queueLock.Unlock();
		switch( Item.eType )
		{
		case QueueItemLoad:
			eResult = LoadBankPre( Item );
			break;
		case QueueItemUnload:
			eResult = UnloadBankPre( Item );
			break;
		case QueueItemPrepareEvent:
			eResult = PrepareEvents( Item );
			break;
		case QueueItemUnprepareEvent:
			eResult = UnprepareEvents( Item );
			break;
		case QueueItemSupportedGameSync:
			AKDIAG( L"akdiag: PrepareGameSyncs (group 0x%08x, %u syncs): not ported (game syncs are IH3), completed", Item.gameSync.uGroupID, Item.gameSync.uNumGameSync );
			NotifyCompletion( Item, AK_Success );
			eResult = AK_Success;
			break;
		case QueueItemUnprepareAllEvents:
			ClearPreparedEvents();
			NotifyCompletion( Item, AK_Success );
			eResult = AK_Success;
			break;
		case QueueItemPrepareBank:
			eResult = PrepareBank( Item );
			break;
		case QueueItemUnprepareBank:
			eResult = UnPrepareBank( Item );
			break;
		case QueueItemClearBanks:
			eResult = ClearBanksInternal( Item );
			break;
		case QueueItemLoadMediaFile:
		case QueueItemLoadMediaFileSwap:
		case QueueItemUnloadMediaFile:
			eResult = AK_Success;
			break;
		default:
			break;
		}
		m_queueLock.Lock();
	}
	m_queueLock.Unlock();
	return eResult;
}

/** 2012 0x929970: the bank id / pool id the callback gets depends on the item type. */
void CAkBankMgr::NotifyCompletion( AkBankQueueItem & in_rItem, AKRESULT in_OperationResult )
{
	AkMemPoolId memPoolId = AK_INVALID_POOL_ID;
	AkUInt32 uID = 0;
	switch( in_rItem.eType )
	{
	case QueueItemLoad:
	case QueueItemUnload:
		memPoolId = (AkMemPoolId)in_rItem.prepare.eventID;
		uID = in_rItem.prepare.numEvents;
		break;
	case QueueItemSupportedGameSync:
		break;
	case QueueItemPrepareBank:
	case QueueItemUnprepareBank:
		uID = in_rItem.prepare.numEvents;
		break;
	default:
		uID = in_rItem.prepare.numEvents == 1 ? in_rItem.prepare.eventID : 0;
		break;
	}
	m_CallbackMgr.DoCallback( in_rItem.callbackInfo.pfnBankCallback, uID, in_OperationResult, memPoolId, in_rItem.callbackInfo.pCookie );
}

/** 2012 0x9299e0: the bank id is the fourth dword of a BKHD-first bank. */
AkBankID CAkBankMgr::GetBankIDFromInMemorySpace( const void * in_pData, AkUInt32 in_uSize )
{
	return in_uSize < 0x18 ? 0 : ( (const AkUInt32 *)in_pData )[ 3 ];
}

/** 2012 0x92acd0: "name" -> "name.bnk", kept until the bank manager terminates. */
void CAkBankMgr::UpdateBankName( AkBankID in_bankID, const char * in_pStringWithoutExtension )
{
	if( m_BankIDToFileName.Exists( in_bankID ) )
	{
		return;
	}
	const size_t uLen = strlen( in_pStringWithoutExtension );
	char * pszName = (char *)AK::MemoryMgr::Malloc( g_DefaultPoolId, (AkUInt32)uLen + 5 );
	if( !pszName )
	{
		return;
	}
	memcpy( pszName, in_pStringWithoutExtension, uLen );
	memcpy( pszName + uLen, ".bnk", 5 );
	char ** ppEntry = m_BankIDToFileName.Set( in_bankID );
	if( ppEntry )
	{
		*ppEntry = pszName;
	}
	else
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, pszName );
	}
}

const char * CAkBankMgr::BankFileName( AkBankID in_bankID )
{
	char ** ppName = m_BankIDToFileName.Exists( in_bankID );
	return ppName ? *ppName : NULL;
}

/** 2012 0x92a990. */
void CAkBankMgr::FlushFileNameTable()
{
	if( m_BankIDToFileName.m_MemPoolId == AK_INVALID_POOL_ID )
	{
		return;
	}
	for( AkHashList<AkBankID, char *, 31>::Iterator it = m_BankIDToFileName.Begin(); it != m_BankIDToFileName.End(); ++it )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, ( *it ).item );
	}
	m_BankIDToFileName.Term();
}

/** 2012 0x92ad70. */
AkMediaInfo CAkBankMgr::GetMedia( AkUniqueID in_mediaId, CAkUsageSlot *& out_pUsageSlot )
{
	AkMediaInfo mediaInfo;
	mediaInfo.pInMemoryData = NULL;
	mediaInfo.uInMemoryDataSize = 0;
	AkAutoLock<CAkLock> bankListGate( CAkBankList::m_BankListLock );
	AkAutoLock<CAkLock> mediaGate( m_MediaLock );
	AkMediaEntry * pEntry = m_MediaHashTable.Exists( in_mediaId );
	if( pEntry )
	{
		pEntry->GetMedia( mediaInfo, out_pUsageSlot );
	}
	return mediaInfo;
}

/** 2012 0x92a490: {plug-in id, stream type, source id, file id, [offset, size if not streamed], flags, [plug-in params]}. */
AKRESULT CAkBankMgr::LoadSource( AkUInt8 *& io_pData, AkUInt32 & io_ulDataSize, AkBankSourceData & out_rSource )
{
	memset( &out_rSource, 0, sizeof( out_rSource ) );
	out_rSource.m_PluginID = *(AkUInt32 *)io_pData;
	io_pData += 4;
	const AkUInt32 StreamType = *(AkUInt32 *)io_pData;
	io_pData += 4;
	out_rSource.m_MediaInfo.sourceID = *(AkUInt32 *)io_pData;
	io_pData += 4;
	out_rSource.m_MediaInfo.uFileID = *(AkUInt32 *)io_pData;
	io_pData += 4;
	if( StreamType == 1 )
	{
		out_rSource.m_MediaInfo.uFileOffset = 0;
		out_rSource.m_MediaInfo.uInMemoryMediaSize = 0;
	}
	else
	{
		out_rSource.m_MediaInfo.uFileOffset = *(AkUInt32 *)io_pData;
		io_pData += 4;
		out_rSource.m_MediaInfo.uInMemoryMediaSize = *(AkUInt32 *)io_pData;
		io_pData += 4;
	}
	const AkUInt8 uSourceBits = *io_pData++;
	out_rSource.m_MediaInfo.bIsLanguageSpecific = uSourceBits & 1;
	out_rSource.m_MediaInfo.bPrefetch = StreamType == 2;
	out_rSource.m_MediaInfo.bIsFromRSX = ( uSourceBits & 2 ) != 0;
	const AkUInt32 uPluginType = out_rSource.m_PluginID & AkPluginTypeMask;
	if( uPluginType == AkPluginTypeCodec )
	{
		if( StreamType == 0 )
		{
			out_rSource.m_MediaInfo.Type = 3;
			return AK_Success;
		}
		if( StreamType == 1 || StreamType == 2 )
		{
			out_rSource.m_MediaInfo.Type = 1;
			return AK_Success;
		}
		return AK_Fail;
	}
	if( uPluginType == AkPluginTypeMotionSource || uPluginType == AkPluginTypeSource )
	{
		out_rSource.m_uSize = *(AkUInt32 *)io_pData;
		io_pData += 4;
		out_rSource.m_pParam = io_pData;
		io_pData += out_rSource.m_uSize;
		io_ulDataSize -= out_rSource.m_uSize;
	}
	else if( uPluginType )
	{
		return AK_Fail;
	}
	return AK_Success;
}

/** 2012 0x929a00: BKHD must come first; version 65 only. */
AKRESULT CAkBankMgr::ProcessBankHeader( AkBank::AkBankHeader & in_rBankHeader )
{
	AkBank::AkSubchunkHeader SubChunkHeader;
	AKRESULT eResult = m_BankReader.FillDataEx( &SubChunkHeader, sizeof( SubChunkHeader ) );
	if( eResult != AK_Success || SubChunkHeader.dwTag != AkBank::BankHeaderChunkID )
	{
		m_bFeedbackInBank = in_rBankHeader.bFeedbackInBank != 0;
		return AK_InvalidFile;
	}
	m_uLoadBytes += sizeof( SubChunkHeader ) + SubChunkHeader.dwChunkSize;
	eResult = m_BankReader.FillDataEx( &in_rBankHeader, sizeof( in_rBankHeader ) );
	if( eResult == AK_Success )
	{
		if( SubChunkHeader.dwChunkSize != sizeof( in_rBankHeader ) )
		{
			const AkUInt32 uToSkip = SubChunkHeader.dwChunkSize - sizeof( in_rBankHeader );
			AkUInt32 uSkipped = 0;
			eResult = m_BankReader.Skip( uToSkip, uSkipped );
			if( eResult == AK_Success && uSkipped != uToSkip )
			{
				eResult = AK_BankReadError;
			}
		}
		if( eResult == AK_Success && in_rBankHeader.dwBankGeneratorVersion != AK_BANK_READER_VERSION )
		{
			eResult = AK_WrongBankVersion;
		}
	}
	m_bFeedbackInBank = in_rBankHeader.bFeedbackInBank != 0;
	return eResult;
}

/** 2012 0x929a90: the DATA chunk goes into the slot's pool (an internal fixed-block pool if none was given). */
AKRESULT CAkBankMgr::ProcessDataChunk( AkUInt32 in_dwDataChunkSize, CAkUsageSlot * in_pUsageSlot )
{
	if( !in_dwDataChunkSize )
	{
		return AK_Success;
	}
	if( in_pUsageSlot->m_memPoolId == AK_INVALID_POOL_ID )
	{
		in_pUsageSlot->m_memPoolId = AK::MemoryMgr::CreatePool( NULL, in_dwDataChunkSize, in_dwDataChunkSize, AkMalloc | AkFixedSizeBlocksMode, 16 );
		if( in_pUsageSlot->m_memPoolId == AK_INVALID_POOL_ID )
		{
			return AK_InsufficientMemory;
		}
		in_pUsageSlot->m_bIsInternalPool = true;
	}
	AKRESULT eResult = AK::MemoryMgr::CheckPoolId( in_pUsageSlot->m_memPoolId );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	if( AK::MemoryMgr::GetPoolAttributes( in_pUsageSlot->m_memPoolId ) & AkFixedSizeBlocksMode )
	{
		if( AK::MemoryMgr::GetBlockSize( in_pUsageSlot->m_memPoolId ) >= in_dwDataChunkSize )
		{
			in_pUsageSlot->m_pData = (AkUInt8 *)AK::MemoryMgr::GetBlock( in_pUsageSlot->m_memPoolId );
		}
	}
	else
	{
		in_pUsageSlot->m_pData = (AkUInt8 *)AK::MemoryMgr::Malloc( in_pUsageSlot->m_memPoolId, in_dwDataChunkSize );
	}
	if( !in_pUsageSlot->m_pData )
	{
		return AK_InsufficientMemory;
	}
	in_pUsageSlot->m_uLoadedDataSize = in_dwDataChunkSize;
	AkUInt32 ulSizeRead = 0;
	eResult = m_BankReader.FillData( in_pUsageSlot->m_pData, in_dwDataChunkSize, ulSizeRead );
	if( eResult == AK_Success && ulSizeRead != in_dwDataChunkSize )
	{
		return AK_InvalidFile;
	}
	return eResult;
}

/** 2012 0x929b60: volume threshold, voice limit, state groups + transitions, switch->RTPC maps, RTPC defaults. */
AKRESULT CAkBankMgr::ProcessGlobalSettingsChunk( AkUInt32 in_dwDataChunkSize, CAkUsageSlot * )
{
	if( !in_dwDataChunkSize )
	{
		return AK_Success;
	}
	AkReal32 fVolumeThreshold = 0.f;
	AKRESULT eResult = m_BankReader.FillDataEx( &fVolumeThreshold, 4 );
	AK::SoundEngine::SetVolumeThresholdInternal( fVolumeThreshold, AK::SoundEngine::AkCommandPriority_InitDefault );
	AkUInt16 uMaxVoices = 0;
	if( eResult == AK_Success )
	{
		eResult = m_BankReader.FillDataEx( &uMaxVoices, 2 );
		AK::SoundEngine::SetMaxNumVoicesLimitInternal( uMaxVoices, AK::SoundEngine::AkCommandPriority_InitDefault );
	}

	AkUInt32 ulNumStateGroups = 0;
	if( eResult == AK_Success )
	{
		eResult = m_BankReader.FillDataEx( &ulNumStateGroups, 4 );
	}
	for( AkUInt32 i = 0; eResult == AK_Success && i < ulNumStateGroups; ++i )
	{
		AkStateGroupID StateGroupID = 0;
		AkTimeMs DefaultTransitionTime = 0;
		AkUInt32 ulNumTransitions = 0;
		eResult = m_BankReader.FillDataEx( &StateGroupID, 4 );
		if( eResult == AK_Success )
		{
			eResult = m_BankReader.FillDataEx( &DefaultTransitionTime, 4 );
		}
		if( eResult == AK_Success )
		{
			eResult = g_pStateMgr->AddStateGroup( StateGroupID ) ? AK_Success : AK_Fail;
		}
		if( eResult == AK_Success )
		{
			eResult = g_pStateMgr->SetdefaultTransitionTime( StateGroupID, DefaultTransitionTime );
		}
		if( eResult == AK_Success )
		{
			eResult = m_BankReader.FillDataEx( &ulNumTransitions, 4 );
		}
		for( AkUInt32 j = 0; eResult == AK_Success && j < ulNumTransitions; ++j )
		{
			AkStateID StateFrom = 0;
			AkStateID StateTo = 0;
			AkTimeMs TransitionTime = 0;
			eResult = m_BankReader.FillDataEx( &StateFrom, 4 );
			if( eResult == AK_Success )
			{
				eResult = m_BankReader.FillDataEx( &StateTo, 4 );
			}
			if( eResult == AK_Success )
			{
				eResult = m_BankReader.FillDataEx( &TransitionTime, 4 );
			}
			if( eResult == AK_Success )
			{
				eResult = g_pStateMgr->AddStateTransition( StateGroupID, StateFrom, StateTo, TransitionTime, false );
			}
		}
	}

	AkUInt32 ulNumSwitchGroups = 0;
	if( eResult == AK_Success )
	{
		eResult = m_BankReader.FillDataEx( &ulNumSwitchGroups, 4 );
	}
	for( AkUInt32 i = 0; eResult == AK_Success && i < ulNumSwitchGroups; ++i )
	{
		AkSwitchGroupID SwitchGroupID = 0;
		AkRtpcID RTPC_ID = 0;
		AkUInt32 ulSize = 0;
		eResult = m_BankReader.FillDataEx( &SwitchGroupID, 4 );
		if( eResult == AK_Success )
		{
			eResult = m_BankReader.FillDataEx( &RTPC_ID, 4 );
		}
		if( eResult == AK_Success )
		{
			eResult = m_BankReader.FillDataEx( &ulSize, 4 );
		}
		if( eResult == AK_Success && ulSize )
		{
			const AkUInt32 uBytes = sizeof( AkRTPCGraphPointBase<AkUInt32> ) * ulSize;
			AkRTPCGraphPointBase<AkUInt32> * pGraphPoints = (AkRTPCGraphPointBase<AkUInt32> *)AK::MemoryMgr::Malloc( g_DefaultPoolId, uBytes );
			if( !pGraphPoints )
			{
				eResult = AK_InsufficientMemory;
				break;
			}
			eResult = m_BankReader.FillDataEx( pGraphPoints, uBytes );
			if( eResult == AK_Success )
			{
				eResult = g_pRTPCMgr->AddSwitchRTPC( SwitchGroupID, RTPC_ID, pGraphPoints, ulSize );
			}
			AK::MemoryMgr::Free( g_DefaultPoolId, pGraphPoints );
		}
	}

	AkUInt32 ulNumParams = 0;
	if( eResult == AK_Success )
	{
		eResult = m_BankReader.FillDataEx( &ulNumParams, 4 );
	}
	for( ; eResult == AK_Success && ulNumParams; --ulNumParams )
	{
		AkRtpcID RTPC_ID = 0;
		AkReal32 fValue = 0.f;
		eResult = m_BankReader.FillDataEx( &RTPC_ID, 4 );
		if( eResult == AK_Success )
		{
			eResult = m_BankReader.FillDataEx( &fValue, 4 );
		}
		if( eResult == AK_Success )
		{
			g_pRTPCMgr->SetDefaultParamValue( RTPC_ID, fValue );
		}
	}
	AKDIAG( L"akdiag:   STMG: volume threshold %.2f dB, max voices %u, %u state groups, %u switch->RTPC groups",
		fVolumeThreshold, (AkUInt32)uMaxVoices, ulNumStateGroups, ulNumSwitchGroups );
	return eResult;
}

/** 2012 0x929e90: the obstruction / occlusion volume and LPF curves. */
AKRESULT CAkBankMgr::ProcessEnvSettingsChunk( AkUInt32 in_dwDataChunkSize )
{
	if( !g_pStateMgr || !in_dwDataChunkSize )
	{
		return AK_Fail;
	}
	for( AkUInt32 i = 0; i < CAkEnvironmentsMgr::MAX_CURVE_X_TYPES; ++i )
	{
		for( AkUInt32 j = 0; j < CAkEnvironmentsMgr::MAX_CURVE_Y_TYPES; ++j )
		{
			AkUInt8 bCurveEnabled = 0;
			AKRESULT eResult = m_BankReader.FillDataEx( &bCurveEnabled, 1 );
			if( eResult != AK_Success )
			{
				return eResult;
			}
			g_pEnvironmentMgr->m_bCurveEnabled[ i ][ j ] = bCurveEnabled != 0;
			AkUInt8 eCurveScaling = 0;
			eResult = m_BankReader.FillDataEx( &eCurveScaling, 1 );
			if( eResult != AK_Success )
			{
				return eResult;
			}
			AkUInt16 ulCurveSize = 0;
			eResult = m_BankReader.FillDataEx( &ulCurveSize, 2 );
			if( eResult != AK_Success )
			{
				return eResult;
			}
			AkRTPCGraphPoint * pPoints = (AkRTPCGraphPoint *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkRTPCGraphPoint ) * ulCurveSize );
			if( !pPoints )
			{
				return AK_InsufficientMemory;
			}
			eResult = m_BankReader.FillDataEx( pPoints, sizeof( AkRTPCGraphPoint ) * ulCurveSize );
			if( eResult == AK_Success )
			{
				g_pEnvironmentMgr->SetObsOccCurve( (CAkEnvironmentsMgr::eCurveXType)i, (CAkEnvironmentsMgr::eCurveYType)j, ulCurveSize, pPoints, (AkCurveScaling)eCurveScaling );
			}
			AK::MemoryMgr::Free( g_DefaultPoolId, pPoints );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
	}
	return AK_Success;
}

/** 2012 0x92ab10: STID, bank id -> "name.bnk" for the banks a PlayAction can reference. */
AKRESULT CAkBankMgr::ProcessStringMappingChunk( AkUInt32 in_dwDataChunkSize, CAkUsageSlot * )
{
	AKRESULT eResult = AK_Success;
	AkUInt32 uLeft = in_dwDataChunkSize;
	while( uLeft && eResult == AK_Success )
	{
		AkBank::AKBKHashHeader hdr;
		eResult = m_BankReader.FillDataEx( &hdr, sizeof( hdr ) );
		if( eResult != AK_Success )
		{
			break;
		}
		uLeft -= sizeof( hdr );
		for( AkUInt32 stringNumber = 0; stringNumber < hdr.uiSize; ++stringNumber )
		{
			AkBankID bankID = 0;
			eResult = m_BankReader.FillDataEx( &bankID, 4 );
			if( eResult != AK_Success )
			{
				break;
			}
			uLeft -= 4;
			AkUInt8 uStringSize = 0;
			eResult = m_BankReader.FillDataEx( &uStringSize, 1 );
			if( eResult != AK_Success )
			{
				break;
			}
			--uLeft;
			if( m_BankIDToFileName.Exists( bankID ) )
			{
				AkUInt32 uIgnoredSkippedSize = 0;
				m_BankReader.Skip( uStringSize, uIgnoredSkippedSize );
				uLeft -= uStringSize;
				continue;
			}
			char * pszName = (char *)AK::MemoryMgr::Malloc( g_DefaultPoolId, uStringSize + 5 );
			if( !pszName )
			{
				eResult = AK_InsufficientMemory;
				break;
			}
			memcpy( pszName + uStringSize, ".bnk", 5 );
			eResult = m_BankReader.FillDataEx( pszName, uStringSize );
			if( eResult != AK_Success )
			{
				AK::MemoryMgr::Free( g_DefaultPoolId, pszName );
				break;
			}
			uLeft -= uStringSize;
			char ** ppEntry = m_BankIDToFileName.Set( bankID );
			if( !ppEntry )
			{
				AK::MemoryMgr::Free( g_DefaultPoolId, pszName );
				eResult = AK_InsufficientMemory;
				break;
			}
			*ppEntry = pszName;
		}
	}
	return eResult;
}

/** 2012 0x929fe0: DIDX, one MediaHeader per media; kept in place for in-memory banks, copied otherwise. */
AKRESULT CAkBankMgr::LoadMediaIndex( CAkUsageSlot * in_pCurrentSlot, AkUInt32 in_uIndexChunkSize, bool in_bIsLoadedFromMemory )
{
	if( in_pCurrentSlot->m_uNumLoadedItems )
	{
		AkUInt32 uSkipped = 0;
		m_BankReader.Skip( in_uIndexChunkSize, uSkipped );
		return AK_Success;
	}
	const AkUInt32 uIndexBytes = sizeof( AkBank::MediaHeader ) * ( in_uIndexChunkSize / sizeof( AkBank::MediaHeader ) );
	AKRESULT eResult = AK_Success;
	if( in_bIsLoadedFromMemory )
	{
		const void * pData = m_BankReader.GetData( uIndexBytes );
		if( !in_pCurrentSlot->m_paLoadedMedia )
		{
			in_pCurrentSlot->m_paLoadedMedia = (AkBank::MediaHeader *)pData;
		}
		m_BankReader.ReleaseData();
	}
	else
	{
		in_pCurrentSlot->m_paLoadedMedia = (AkBank::MediaHeader *)AK::MemoryMgr::Malloc( g_DefaultPoolId, uIndexBytes );
		if( !in_pCurrentSlot->m_paLoadedMedia )
		{
			eResult = AK_InsufficientMemory;
		}
		else
		{
			in_pCurrentSlot->m_bWasIndexAllocated = 1;
			m_BankReader.FillDataEx( in_pCurrentSlot->m_paLoadedMedia, uIndexBytes );
		}
	}
	if( in_pCurrentSlot->m_paLoadedMedia )
	{
		in_pCurrentSlot->m_uIndexSize = in_uIndexChunkSize / sizeof( AkBank::MediaHeader );
	}
	return eResult;
}

/** 2012 0x92cd30: every DIDX media gets a media-table entry pointing into the slot's DATA block. */
AKRESULT CAkBankMgr::LoadMedia( AkUInt8 * in_pDataBank, CAkUsageSlot * in_pCurrentSlot, bool )
{
	if( in_pCurrentSlot->m_uNumLoadedItems || !in_pCurrentSlot->m_uIndexSize )
	{
		return AK_Success;
	}
	if( !in_pCurrentSlot->m_paLoadedMedia )
	{
		UnloadMedia( in_pCurrentSlot );
		return AK_InsufficientMemory;
	}
	AKRESULT eResult = AK_InsufficientMemory;
	AkUInt32 nNumLoadedMedias = 0;
	for( ; nNumLoadedMedias < in_pCurrentSlot->m_uIndexSize; ++nNumLoadedMedias )
	{
		const AkBank::MediaHeader & header = in_pCurrentSlot->m_paLoadedMedia[ nNumLoadedMedias ];
		if( header.id )
		{
			AkAutoLock<CAkLock> gate( m_MediaLock );
			bool bAlreadyThere = false;
			AkMediaEntry * pEntry = m_MediaHashTable.Set( header.id, bAlreadyThere );
			if( !pEntry )
			{
				break;
			}
			if( !( bAlreadyThere && pEntry->m_preparedMediaInfo.pInMemoryData ) )
			{
				pEntry->m_sourceID = header.id;
				AkMediaInfo mediaInfo;
				mediaInfo.pInMemoryData = in_pDataBank + header.uOffset;
				mediaInfo.uInMemoryDataSize = header.uSize;
				if( !pEntry->m_BankSlots.Set( in_pCurrentSlot, mediaInfo ) )
				{
					eResult = AK_InsufficientMemory;
					m_MediaHashTable.Unset( header.id );
					break;
				}
				eResult = AK_Success;
			}
			if( bAlreadyThere )
			{
				++pEntry->uRefCount;
			}
		}
		++in_pCurrentSlot->m_uNumLoadedItems;
	}
	if( in_pCurrentSlot->m_uIndexSize == nNumLoadedMedias )
	{
		return AK_Success;
	}
	UnloadMedia( in_pCurrentSlot );
	return eResult;
}

/** 2012 0x92c770. */
void CAkBankMgr::UnloadMedia( CAkUsageSlot * in_pCurrentSlot )
{
	if( !in_pCurrentSlot->m_paLoadedMedia )
	{
		return;
	}
	AkAutoLock<CAkLock> gate( m_MediaLock );
	for( AkUInt32 i = 0; i < in_pCurrentSlot->m_uNumLoadedItems; ++i )
	{
		const AkUniqueID mediaID = in_pCurrentSlot->m_paLoadedMedia[ i ].id;
		if( !mediaID )
		{
			continue;
		}
		AkHashList<AkUniqueID, AkMediaEntry, 193>::IteratorEx it = m_MediaHashTable.FindEx( mediaID );
		if( !it.pItem )
		{
			continue;
		}
		AkMediaEntry & entry = it.pItem->Assoc.item;
		entry.m_BankSlots.UnsetSwap( in_pCurrentSlot );
		if( entry.uRefCount-- == 1 && entry.m_preparedMediaInfo.pInMemoryData )
		{
			AK::MemoryMgr::Free( AkMediaEntry::m_uPrepareEventMemoryPoolID, entry.m_preparedMediaInfo.pInMemoryData );
			entry.m_preparedMediaInfo.pInMemoryData = NULL;
			entry.m_preparedMediaInfo.uInMemoryDataSize = 0;
		}
		if( !entry.uRefCount )
		{
			m_MediaHashTable.Erase( it );
		}
	}
}

/** 2012 0x92c840: prepared media (never created: see the header) would be released here. */
void CAkBankMgr::UnPrepareMedia( CAkUsageSlot * in_pCurrentSlot )
{
	in_pCurrentSlot->m_bIsMediaPrepared = 0;
}

/** 2012 0x92ae00: the slot is released on the audio thread (message 26) after every voice playing its media stopped. */
AKRESULT CAkBankMgr::KillSlot( CAkUsageSlot * in_pUsageSlot, AkBankCallbackFunc in_pCallBack, void * in_pCookie )
{
	in_pUsageSlot->RemoveContent();
	in_pUsageSlot->m_bUsageProhibited = 1;
	in_pUsageSlot->m_pfnBankCallback = in_pCallBack;
	in_pUsageSlot->m_pCookie = in_pCookie;
	AkQueuedMsg Item;
	Item.type = QueuedMsgType_KillBank;
	Item.killbank.pUsageSlot = in_pUsageSlot;
	g_pAudioMgr->Enqueue( Item, AkQueuedMsg::SizeofBody<AkQueuedMsg_KillBank>() );
	return AK::SoundEngine::RenderAudio();
}

/** 2012 0x92c910: the reference GetMedia took; prepared media is freed with its last reference. */
void CAkBankMgr::ReleaseMedia( AkUniqueID in_mediaId )
{
	AkAutoLock<CAkLock> bankListGate( CAkBankList::m_BankListLock );
	ReleaseSingleMedia( in_mediaId );
}

/** 2012 0x92cb20. */
void CAkBankMgr::ReleaseSingleMedia( AkUniqueID in_SourceID )
{
	AkAutoLock<CAkLock> mediaGate( m_MediaLock );
	AkHashList<AkUniqueID, AkMediaEntry, 193>::IteratorEx it = m_MediaHashTable.FindEx( in_SourceID );
	if( !it.pItem )
	{
		return;
	}
	AkMediaEntry & entry = it.pItem->Assoc.item;
	if( entry.uRefCount-- == 1 && entry.m_preparedMediaInfo.pInMemoryData )
	{
		AK::MemoryMgr::Free( AkMediaEntry::m_uPrepareEventMemoryPoolID, entry.m_preparedMediaInfo.pInMemoryData );
		entry.m_preparedMediaInfo.pInMemoryData = NULL;
		entry.m_preparedMediaInfo.uInMemoryDataSize = 0;
	}
	if( !entry.uRefCount )
	{
		m_MediaHashTable.Erase( it );
	}
}

/** 2012 0x92b340. */
void CAkBankMgr::AddLoadedItem( CAkUsageSlot * in_pUsageSlot, CAkIndexable * in_pIndexable )
{
	in_pUsageSlot->m_listLoadedItem.AddLast( in_pIndexable );
}

/** See g_pHircParseEnd (AkHierarchy.h): a freshly created object must have read its section exactly. */
void CAkBankMgr::CheckParseEnd( const AkBank::AKBKSubHircSection & in_rSection, const AkUInt8 * in_pData )
{
	if( g_pHircParseEnd && g_pHircParseEnd != in_pData + in_rSection.dwSectionSize && in_rSection.eHircType < AkBank::HIRCType_Count )
	{
		if( !m_aHircMisparsed[ in_rSection.eHircType ]++ )
		{
			AKDIAG( L"akdiag:   misparsed %s 0x%08x: read %d of %u bytes", HircTypeName( in_rSection.eHircType ), *(const AkUInt32 *)in_pData,
				(AkInt32)( g_pHircParseEnd - in_pData ), in_rSection.dwSectionSize );
		}
	}
	g_pHircParseEnd = NULL;
}

AKRESULT CAkBankMgr::SkipSection( const AkBank::AKBKSubHircSection & in_rSection )
{
	AkUInt32 uSkipped = 0;
	m_BankReader.Skip( in_rSection.dwSectionSize, uSkipped );
	return uSkipped == in_rSection.dwSectionSize ? AK_Success : AK_InvalidFile;
}

/** 2012 0x92b6b0 .. 0x92bdb0: StdBankRead<T>: reuse the indexed object if present, else Create + SetInitialValues. */
template <class T>
AKRESULT CAkBankMgr::StdBankRead( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot, T * ( *in_pfnCreate )( AkUniqueID ), CAkIndexItemBase & in_rIndex )
{
	AKRESULT eResult = AK_Success;
	AkUInt8 * pData = (AkUInt8 *)m_BankReader.GetData( in_rSection.dwSectionSize );
	if( !pData )
	{
		return AK_Fail;
	}
	const AkUniqueID ulID = *(AkUInt32 *)pData;
	T * pObject = static_cast<T *>( in_rIndex.GetPtrAndAddRefBase( ulID ) );
	if( !pObject )
	{
		AkAutoLock<CAkLock> gate( g_csMain );
		pObject = in_pfnCreate( ulID );
		if( pObject )
		{
			g_pHircParseEnd = NULL;
			eResult = pObject->SetInitialValues( pData, in_rSection.dwSectionSize );
			if( eResult != AK_Success )
			{
				pObject->Release();
			}
			else
			{
				CheckParseEnd( in_rSection, pData );
			}
		}
		else
		{
			eResult = AK_Fail;
		}
	}
	if( eResult == AK_Success )
	{
		AddLoadedItem( in_pUsageSlot, pObject );
	}
	m_BankReader.ReleaseData();
	return eResult;
}

static CAkRanSeqCntr * CreateRanSeqCntr( AkUniqueID in_ulID )
{
	return CAkRanSeqCntr::Create( in_ulID, ContainerMode_Sequence );
}

/** 2012 0x92b590: a sound that already holds a bank source keeps it; one without gets a partial load. */
AKRESULT CAkBankMgr::ReadSourceParentSound( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot )
{
	AKRESULT eResult = AK_Success;
	AkUInt8 * pData = (AkUInt8 *)m_BankReader.GetData( in_rSection.dwSectionSize );
	if( !pData )
	{
		return AK_Fail;
	}
	const AkUniqueID ulID = *(AkUInt32 *)pData;
	CAkSound * pSound = static_cast<CAkSound *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( ulID ) );
	if( pSound )
	{
		if( !pSound->HasBankSource() )
		{
			AkAutoLock<CAkLock> gate( g_csMain );
			eResult = pSound->SetInitialValues( pData, in_rSection.dwSectionSize, in_pUsageSlot, true );
			if( eResult != AK_Success )
			{
				pSound->Release();
			}
		}
	}
	else
	{
		AkAutoLock<CAkLock> gate( g_csMain );
		pSound = CAkSound::Create( ulID );
		if( pSound )
		{
			g_pHircParseEnd = NULL;
			eResult = pSound->SetInitialValues( pData, in_rSection.dwSectionSize, in_pUsageSlot, false );
			if( eResult != AK_Success )
			{
				pSound->Release();
			}
			else
			{
				CheckParseEnd( in_rSection, pData );
			}
		}
		else
		{
			eResult = AK_Fail;
		}
	}
	if( eResult == AK_Success )
	{
		AddLoadedItem( in_pUsageSlot, pSound );
	}
	m_BankReader.ReleaseData();
	return eResult;
}

/** 2012 0x92be70. */
AKRESULT CAkBankMgr::ReadState( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot )
{
	return StdBankRead<CAkState>( in_rSection, in_pUsageSlot, &CAkState::Create, g_pIndex->m_idxCustomStates );
}

/** 2012 0x92bf40: the first bus of the first bank loaded replaces the master bus. */
AKRESULT CAkBankMgr::ReadBus( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot )
{
	AKRESULT eResult = AK_Success;
	AkUInt8 * pData = (AkUInt8 *)m_BankReader.GetData( in_rSection.dwSectionSize );
	if( !pData )
	{
		return AK_Fail;
	}
	const AkUniqueID ulID = *(AkUInt32 *)pData;
	CAkBus * pBus = static_cast<CAkBus *>( g_pIndex->m_idxAudioNode.GetPtrAndAddRef( ulID ) );
	if( !pBus )
	{
		if( !m_bIsFirstBusLoaded )
		{
			CAkBus::ClearMasterBus();
		}
		AkAutoLock<CAkLock> gate( g_csMain );
		pBus = CAkBus::Create( ulID );
		if( pBus )
		{
			g_pHircParseEnd = NULL;
			eResult = pBus->SetInitialValues( pData, in_rSection.dwSectionSize );
			if( eResult != AK_Success )
			{
				pBus->Release();
			}
			else
			{
				CheckParseEnd( in_rSection, pData );
			}
		}
		else
		{
			eResult = AK_Fail;
		}
	}
	if( eResult == AK_Success )
	{
		AddLoadedItem( in_pUsageSlot, pBus );
		m_bIsFirstBusLoaded = true;
	}
	m_BankReader.ReleaseData();
	return eResult;
}

/** 2012 0x92c030: an existing Play action that was not loaded from a bank (bank id unknown) is re-read. */
AKRESULT CAkBankMgr::ReadAction( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot )
{
	AKRESULT eResult = AK_Success;
	AkUInt8 * pData = (AkUInt8 *)m_BankReader.GetData( in_rSection.dwSectionSize );
	if( !pData )
	{
		return AK_Fail;
	}
	const AkUniqueID ulID = *(AkUInt32 *)pData;
	const AkUInt32 ulActionType = *(AkUInt16 *)( pData + 4 );
	CAkAction * pAction = g_pIndex->m_idxActions.GetPtrAndAddRef( ulID );
	if( pAction )
	{
		if( ulActionType == AkActionType_Play && !pAction->m_bWasLoadedFromBank )
		{
			AkAutoLock<CAkLock> gate( g_csMain );
			eResult = pAction->SetInitialValues( pData, in_rSection.dwSectionSize );
			if( eResult != AK_Success )
			{
				pAction->Release();
			}
		}
	}
	else
	{
		AkAutoLock<CAkLock> gate( g_csMain );
		pAction = CAkAction::Create( ulActionType, ulID );
		if( pAction )
		{
			g_pHircParseEnd = NULL;
			eResult = pAction->SetInitialValues( pData, in_rSection.dwSectionSize );
			if( eResult != AK_Success )
			{
				pAction->Release();
			}
			else
			{
				CheckParseEnd( in_rSection, pData );
			}
		}
		else
		{
			eResult = AK_Fail;
		}
	}
	if( eResult == AK_Success )
	{
		AddLoadedItem( in_pUsageSlot, pAction );
	}
	m_BankReader.ReleaseData();
	return eResult;
}

/** 2012 0x92c150: an event joins the index only once its action list is complete. */
AKRESULT CAkBankMgr::ReadEvent( const AkBank::AKBKSubHircSection & in_rSection, CAkUsageSlot * in_pUsageSlot )
{
	AKRESULT eResult = AK_Success;
	AkUInt8 * pData = (AkUInt8 *)m_BankReader.GetData( in_rSection.dwSectionSize );
	if( !pData )
	{
		return AK_Fail;
	}
	const AkUniqueID ulID = *(AkUInt32 *)pData;
	CAkEvent * pEvent = g_pIndex->m_idxEvents.GetPtrAndAddRef( ulID );
	if( !pEvent )
	{
		pEvent = CAkEvent::CreateNoIndex( ulID );
		if( !pEvent )
		{
			m_BankReader.ReleaseData();
			return AK_Fail;
		}
		AkAutoLock<CAkLock> gate( g_csMain );
		g_pHircParseEnd = NULL;
		eResult = pEvent->SetInitialValues( pData, in_rSection.dwSectionSize );
		if( eResult == AK_Success )
		{
			CheckParseEnd( in_rSection, pData );
			pEvent->AddToIndex();
		}
		else
		{
			pEvent->Release();
		}
	}
	if( eResult == AK_Success )
	{
		AddLoadedItem( in_pUsageSlot, pEvent );
	}
	m_BankReader.ReleaseData();
	return eResult;
}

/** 2012 0x92c320. */
AKRESULT CAkBankMgr::ProcessHircChunk( CAkUsageSlot * in_pUsageSlot, AkUInt32 in_dwBankID )
{
	AkUInt32 l_NumReleasableHircItem = 0;
	AKRESULT eResult = m_BankReader.FillDataEx( &l_NumReleasableHircItem, 4 );
	if( eResult == AK_Success && l_NumReleasableHircItem )
	{
		eResult = in_pUsageSlot->m_listLoadedItem.GrowArray( l_NumReleasableHircItem ) ? AK_Success : AK_Fail;
	}
	for( AkUInt32 i = 0; i < l_NumReleasableHircItem; ++i )
	{
		if( eResult != AK_Success )
		{
			return eResult;
		}
		AkBank::AKBKSubHircSection Section;
		eResult = m_BankReader.FillDataEx( &Section, sizeof( Section ) );
		if( eResult != AK_Success )
		{
			continue;
		}
		switch( Section.eHircType )
		{
		case AkBank::HIRCType_State:
			eResult = ReadState( Section, in_pUsageSlot );
			break;
		case AkBank::HIRCType_Sound:
			eResult = ReadSourceParentSound( Section, in_pUsageSlot );
			break;
		case AkBank::HIRCType_Action:
			eResult = ReadAction( Section, in_pUsageSlot );
			break;
		case AkBank::HIRCType_Event:
			eResult = ReadEvent( Section, in_pUsageSlot );
			break;
		case AkBank::HIRCType_RanSeqCntr:
			eResult = StdBankRead<CAkRanSeqCntr>( Section, in_pUsageSlot, &CreateRanSeqCntr, g_pIndex->m_idxAudioNode );
			break;
		case AkBank::HIRCType_SwitchCntr:
			eResult = StdBankRead<CAkSwitchCntr>( Section, in_pUsageSlot, &CAkSwitchCntr::Create, g_pIndex->m_idxAudioNode );
			break;
		case AkBank::HIRCType_ActorMixer:
			eResult = StdBankRead<CAkActorMixer>( Section, in_pUsageSlot, &CAkActorMixer::Create, g_pIndex->m_idxAudioNode );
			break;
		case AkBank::HIRCType_Bus:
			eResult = ReadBus( Section, in_pUsageSlot );
			break;
		case AkBank::HIRCType_LayerCntr:
			eResult = StdBankRead<CAkLayerCntr>( Section, in_pUsageSlot, &CAkLayerCntr::Create, g_pIndex->m_idxAudioNode );
			break;
		case AkBank::HIRCType_Attenuation:
			eResult = StdBankRead<CAkAttenuation>( Section, in_pUsageSlot, &CAkAttenuation::Create, g_pIndex->m_idxAttenuations );
			break;
		case AkBank::HIRCType_FxShareSet:
			eResult = StdBankRead<CAkFxShareSet>( Section, in_pUsageSlot, &CAkFxShareSet::Create, g_pIndex->m_idxFxShareSets );
			break;
		case AkBank::HIRCType_FxCustom:
			eResult = StdBankRead<CAkFxCustom>( Section, in_pUsageSlot, &CAkFxCustom::Create, g_pIndex->m_idxFxCustom );
			break;
		case AkBank::HIRCType_DialogueEvent:
		case AkBank::HIRCType_FeedbackBus:
		case AkBank::HIRCType_FeedbackNode:
			eResult = SkipSection( Section );
			break;
		default:
			if( g_pExternalBankHandlerCallback )
			{
				eResult = g_pExternalBankHandlerCallback( Section, in_pUsageSlot, in_dwBankID );
				if( eResult != AK_PartialSuccess )
				{
					break;
				}
			}
			eResult = SkipSection( Section );
			break;
		}
		if( Section.eHircType < AkBank::HIRCType_Count )
		{
			++m_aHircCounts[ Section.eHircType ];
			if( eResult != AK_Success )
			{
				++m_aHircFailures[ Section.eHircType ];
			}
		}
	}
	return eResult;
}

/** 2012 0x92d2c0. */
AKRESULT CAkBankMgr::LoadBank( AkBankQueueItem in_Item, CAkUsageSlot *& out_pUsageSlot, AkLoadBankDataMode in_eLoadMode, bool in_bIsFromPrepareBank )
{
	memset( m_aHircCounts, 0, sizeof( m_aHircCounts ) );
	memset( m_aHircFailures, 0, sizeof( m_aHircFailures ) );
	memset( m_aHircMisparsed, 0, sizeof( m_aHircMisparsed ) );
	m_uLoadBytes = 0;

	const AkBankID bankID = in_Item.prepare.numEvents;
	CAkUsageSlot * pSlot = m_BankList.Get( bankID );
	if( pSlot && pSlot->m_bWasLoadedAsABank )
	{
		return AK_BankAlreadyLoaded;
	}
	AKRESULT eResult = AK_Success;
	if( pSlot )
	{
		out_pUsageSlot = pSlot;
		if( in_bIsFromPrepareBank )
		{
			++pSlot->m_iWasPreparedAsABankCounter;
		}
	}
	else
	{
		AkInt32 mainRefCount = 0;
		AkInt32 prepareEventRefCount = 0;
		AkInt32 prepareBankRefCount = 0;
		AkMemPoolId memPoolId = g_settings.uPrepareEventMemoryPoolID;
		if( in_eLoadMode == AkLoadBankDataMode_OneBlock )
		{
			memPoolId = in_Item.load.memPoolId;
			mainRefCount = 1;
		}
		else if( in_bIsFromPrepareBank )
		{
			prepareBankRefCount = 1;
		}
		else
		{
			prepareEventRefCount = 1;
		}
		void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkUsageSlot ) );
		out_pUsageSlot = pMem ? AkPlacementNew( pMem ) CAkUsageSlot( bankID, memPoolId, mainRefCount, prepareEventRefCount, prepareBankRefCount ) : NULL;
		if( !out_pUsageSlot )
		{
			eResult = AK_InsufficientMemory;
		}
	}

	m_BankReader.Reset();
	const bool bIsLoadedFromMemory = in_Item.bankLoadFlag == AkBankLoadFlag_InMemory;
	if( eResult == AK_Success )
	{
		if( in_eLoadMode == AkLoadBankDataMode_OneBlock && in_Item.bankLoadFlag == AkBankLoadFlag_InMemory )
		{
			eResult = m_BankReader.SetFile( in_Item.load.pInMemoryBank, in_Item.load.ui32InMemoryBankSize );
		}
		else if( in_eLoadMode == AkLoadBankDataMode_OneBlock && in_Item.bankLoadFlag == AkBankLoadFlag_UsingFileID )
		{
			eResult = m_BankReader.SetFile( bankID, 0, 0, in_Item.callbackInfo.pCookie, true );
		}
		else
		{
			const char * pszFileName = BankFileName( bankID );
			eResult = pszFileName ? m_BankReader.SetFile( pszFileName, 0, in_Item.callbackInfo.pCookie ) : m_BankReader.SetFile( bankID, 0, 0, in_Item.callbackInfo.pCookie, true );
		}
	}

	AkBank::AkBankHeader BankHeaderInfo;
	memset( &BankHeaderInfo, 0, sizeof( BankHeaderInfo ) );
	if( eResult == AK_Success )
	{
		eResult = ProcessBankHeader( BankHeaderInfo );
	}

	while( eResult == AK_Success )
	{
		AkBank::AkSubchunkHeader SubChunkHeader;
		AkUInt32 ulSizeRead = 0;
		eResult = m_BankReader.FillData( &SubChunkHeader, sizeof( SubChunkHeader ), ulSizeRead );
		if( eResult != AK_Success || !ulSizeRead )
		{
			break;
		}
		if( ulSizeRead != sizeof( SubChunkHeader ) )
		{
			eResult = AK_InvalidFile;
			break;
		}
		m_uLoadBytes += sizeof( SubChunkHeader ) + SubChunkHeader.dwChunkSize;
		bool bSkip = false;
		switch( SubChunkHeader.dwTag )
		{
		case AkBank::BankStateMgrChunkID:
			eResult = ProcessGlobalSettingsChunk( SubChunkHeader.dwChunkSize, out_pUsageSlot );
			break;
		case AkBank::BankEnvSettingChunkID:
			eResult = ProcessEnvSettingsChunk( SubChunkHeader.dwChunkSize );
			break;
		case AkBank::BankDataIndexChunkID:
			if( in_eLoadMode == AkLoadBankDataMode_Structure )
			{
				bSkip = true;
			}
			else
			{
				eResult = LoadMediaIndex( out_pUsageSlot, SubChunkHeader.dwChunkSize, bIsLoadedFromMemory );
			}
			break;
		case AkBank::BankDataChunkID:
			if( in_eLoadMode == AkLoadBankDataMode_OneBlock )
			{
				AkUInt8 * pData = NULL;
				if( bIsLoadedFromMemory )
				{
					pData = (AkUInt8 *)m_BankReader.GetData( SubChunkHeader.dwChunkSize );
					m_BankReader.ReleaseData();
				}
				else
				{
					eResult = ProcessDataChunk( SubChunkHeader.dwChunkSize, out_pUsageSlot );
					pData = out_pUsageSlot->m_pData;
				}
				if( eResult == AK_Success )
				{
					eResult = LoadMedia( pData, out_pUsageSlot, bIsLoadedFromMemory );
				}
			}
			else if( in_eLoadMode == AkLoadBankDataMode_Structure )
			{
				bSkip = true;
			}
			else
			{
				AKDIAG( L"akdiag:   DATA chunk of %u bytes not prepared: PrepareBank(AllInclusive) media preparation is not ported (see AkBankMgr.h)", SubChunkHeader.dwChunkSize );
				bSkip = true;
			}
			break;
		case AkBank::BankHierarchyChunkID:
			eResult = ProcessHircChunk( out_pUsageSlot, bankID );
			break;
		case AkBank::BankStrMapChunkID:
			eResult = ProcessStringMappingChunk( SubChunkHeader.dwChunkSize, out_pUsageSlot );
			break;
		default:
			bSkip = true;
			break;
		}
		if( bSkip )
		{
			AkUInt32 uSkipped = 0;
			m_BankReader.Skip( SubChunkHeader.dwChunkSize, uSkipped );
			if( uSkipped != SubChunkHeader.dwChunkSize )
			{
				eResult = AK_InvalidFile;
			}
		}
	}
	m_BankReader.CloseFile();
	return eResult;
}

void CAkBankMgr::LogBank( const wchar_t * in_szWhat, AkBankID in_bankID, AKRESULT in_eResult )
{
	if( !DishonoredWwise::IsDiagnostics() )
	{
		return;
	}
	const char * pszName = BankFileName( in_bankID );
	wchar_t szName[ AK_DEFAULT_PATH_LENGTH ] = L"<by id>";
	if( pszName )
	{
		AKPLATFORM::AkCharToWideChar( pszName, AK_DEFAULT_PATH_LENGTH, szName );
	}
	AkUInt32 uTotal = 0;
	AkUInt32 uFailures = 0;
	AkUInt32 uMisparsed = 0;
	wchar_t szCounts[ 1024 ] = L"";
	size_t uLen = 0;
	for( AkUInt32 i = 0; i < AkBank::HIRCType_Count; ++i )
	{
		uTotal += m_aHircCounts[ i ];
		uFailures += m_aHircFailures[ i ];
		uMisparsed += m_aHircMisparsed[ i ];
		if( m_aHircCounts[ i ] && uLen < 900 )
		{
			uLen += swprintf_s( szCounts + uLen, 1024 - uLen, L" %s=%u", HircTypeName( i ), m_aHircCounts[ i ] );
		}
	}
	AkSilent::Logf( L"akdiag: %s bank %s (id 0x%08x): result %d, %u bytes, HIRC %u objects (%u failed, %u misparsed):%s",
		in_szWhat, szName, in_bankID, (AkInt32)in_eResult, m_uLoadBytes, uTotal, uFailures, uMisparsed, szCounts );
}

/** 2012 0x92d6a0. */
AKRESULT CAkBankMgr::LoadBankPre( AkBankQueueItem & in_rItem )
{
	AKRESULT eNotify = AK_Success;
	CAkUsageSlot * pUsageSlot = NULL;
	AKRESULT eResult = LoadBank( in_rItem, pUsageSlot, AkLoadBankDataMode_OneBlock, false );
	if( eResult == AK_BankAlreadyLoaded )
	{
		eResult = AK_Success;
		eNotify = AK_BankAlreadyLoaded;
	}
	else if( eResult == AK_Success )
	{
		pUsageSlot->m_bWasLoadedAsABank = 1;
		m_BankList.Set( in_rItem.prepare.numEvents, pUsageSlot );
	}
	else
	{
		if( pUsageSlot )
		{
			pUsageSlot->RemoveContent();
			pUsageSlot->Release( true );
		}
		eNotify = eResult;
	}
	LogBank( L"LoadBank", in_rItem.prepare.numEvents, eNotify );
	NotifyCompletion( in_rItem, eNotify );
	return eResult;
}

/** 2012 0x92b0e0. */
AKRESULT CAkBankMgr::UnloadBankPre( AkBankQueueItem in_Item )
{
	const AkBankID bankID = in_Item.prepare.numEvents;
	CAkBankList::m_BankListLock.Lock();
	CAkUsageSlot * pSlot = m_BankList.Get( bankID );
	if( pSlot )
	{
		m_BankList.Remove( bankID );
		if( pSlot->m_bWasLoadedAsABank )
		{
			CAkBankList::m_BankListLock.Unlock();
			AKDIAG( L"akdiag: UnloadBank 0x%08x (%S)", bankID, BankFileName( bankID ) ? BankFileName( bankID ) : "<by id>" );
			return KillSlot( pSlot, in_Item.callbackInfo.pfnBankCallback, in_Item.callbackInfo.pCookie );
		}
	}
	CAkBankList::m_BankListLock.Unlock();
	NotifyCompletion( in_Item, AK_Success );
	return AK_Success;
}

/** 2012 0x92b190: every loaded (not merely prepared) bank is killed, newest first, each waited on. */
AKRESULT CAkBankMgr::ClearBanksInternal( AkBankQueueItem in_Item )
{
	AkArray<CAkUsageSlot *, CAkUsageSlot *, ArrayPoolDefault, 8> slots;
	{
		AkAutoLock<CAkLock> gate( CAkBankList::m_BankListLock );
		AkHashListBare<AkBankID, CAkUsageSlot, 31>::IteratorEx it = m_BankList.m_ListLoadedBanks.BeginEx();
		while( it.pItem )
		{
			if( it.pItem->m_bWasLoadedAsABank )
			{
				slots.AddLast( it.pItem );
				it = m_BankList.m_ListLoadedBanks.Erase( it );
			}
			else
			{
				++it;
			}
		}
	}
	AKDIAG( L"akdiag: ClearBanks: %u loaded banks", slots.Length() );
	for( AkInt32 i = (AkInt32)slots.Length() - 1; i >= 0; --i )
	{
		KillSlot( slots[ i ], NULL, NULL );
	}
	slots.Term();
	NotifyCompletion( in_Item, AK_Success );
	return AK_Success;
}

/** 2012 0x92db00. */
AKRESULT CAkBankMgr::PrepareBankInternal( AkBankQueueItem in_Item, AkBankID in_FileID, AkLoadBankDataMode in_LoadBankMode, bool in_bIsFromPrepareBank )
{
	CAkBankList::m_BankListLock.Lock();
	CAkUsageSlot * pSlot = m_BankList.Get( in_FileID );
	AkLoadBankDataMode eMode = in_LoadBankMode;
	if( pSlot )
	{
		InterlockedIncrement( &pSlot->m_iPrepareRefCount );
		++pSlot->m_iWasPreparedAsABankCounter;
		if( in_LoadBankMode == AkLoadBankDataMode_Structure || pSlot->m_bIsMediaPrepared )
		{
			CAkBankList::m_BankListLock.Unlock();
			return AK_Success;
		}
		eMode = AkLoadBankDataMode_Media;
	}
	CAkBankList::m_BankListLock.Unlock();
	in_Item.prepare.numEvents = in_FileID;
	CAkUsageSlot * pUsageSlot = NULL;
	const AKRESULT eResult = LoadBank( in_Item, pUsageSlot, eMode, in_bIsFromPrepareBank );
	if( eResult == AK_Success )
	{
		m_BankList.Set( in_FileID, pUsageSlot );
		return AK_Success;
	}
	if( eResult != AK_BankAlreadyLoaded && pUsageSlot )
	{
		m_BankList.Remove( in_FileID );
		pUsageSlot->ReleasePrepare( false );
	}
	return eResult;
}

/** 2012 0x92de50: AkBankContent_All prepares media and structure, StructureOnly only the structure. */
AKRESULT CAkBankMgr::PrepareBank( AkBankQueueItem in_Item )
{
	const AkLoadBankDataMode eMode = in_Item.prepare.eventID != AK::SoundEngine::AkBankContent_All ? AkLoadBankDataMode_Structure : AkLoadBankDataMode_MediaAndStructure;
	const AKRESULT eResult = PrepareBankInternal( in_Item, in_Item.prepare.numEvents, eMode, true );
	LogBank( L"PrepareBank", in_Item.prepare.numEvents, eResult );
	NotifyCompletion( in_Item, eResult );
	return eResult;
}

/** 2012 0x92d9b0. */
AKRESULT CAkBankMgr::UnPrepareBank( AkBankQueueItem in_Item )
{
	CAkUsageSlot * pSlot = m_BankList.Get( in_Item.prepare.numEvents );
	if( pSlot )
	{
		if( pSlot->m_iWasPreparedAsABankCounter )
		{
			--pSlot->m_iWasPreparedAsABankCounter;
		}
		AKDIAG( L"akdiag: UnprepareBank 0x%08x (%S)", in_Item.prepare.numEvents, BankFileName( in_Item.prepare.numEvents ) ? BankFileName( in_Item.prepare.numEvents ) : "<by id>" );
		pSlot->ReleasePrepare( false );
	}
	NotifyCompletion( in_Item, AK_Success );
	return AK_Success;
}

/** 2012 0x92da00. */
void CAkBankMgr::UnPrepareAllBank()
{
	AkArray<AkBankID, AkBankID, ArrayPoolDefault, 8> bankIDs;
	{
		AkAutoLock<CAkLock> gate( CAkBankList::m_BankListLock );
		for( AkHashListBare<AkBankID, CAkUsageSlot, 31>::Iterator it = m_BankList.m_ListLoadedBanks.Begin(); it != m_BankList.m_ListLoadedBanks.End(); ++it )
		{
			if( ( *it )->m_iWasPreparedAsABankCounter )
			{
				bankIDs.AddLast( ( *it )->key );
			}
		}
	}
	for( AkInt32 i = (AkInt32)bankIDs.Length() - 1; i >= 0; --i )
	{
		CAkUsageSlot * pSlot = m_BankList.Get( bankIDs[ i ] );
		if( pSlot && pSlot->m_iWasPreparedAsABankCounter )
		{
			--pSlot->m_iWasPreparedAsABankCounter;
			pSlot->ReleasePrepare( true );
		}
	}
	bankIDs.Term();
}

/** 2012 0x92a6c0: a copy of the media out of the first bank that holds it. */
AKRESULT AkMediaEntry::PrepareFromBank( AkUInt8 *& out_pAllocatedData, AkUInt32 & out_uMediaSize )
{
	out_uMediaSize = m_BankSlots.m_pItems[ 0 ].item.uInMemoryDataSize;
	out_pAllocatedData = (AkUInt8 *)AK::MemoryMgr::Malign( m_uPrepareEventMemoryPoolID, out_uMediaSize, 16 );
	if( !out_pAllocatedData )
	{
		return AK_InsufficientMemory;
	}
	memcpy( out_pAllocatedData, m_BankSlots.m_pItems[ 0 ].item.pInMemoryData, out_uMediaSize );
	return AK_Success;
}

/** 2012 0x92aa10: a media inside a bank file is read through the bank's file name when the manager knows it. */
AKRESULT CAkBankMgr::SetFileReader( AkFileID in_FileID, AkUInt32 in_uFileOffset, AkUInt32 in_codecID, void * in_pCookie, bool in_bIsLanguageSpecific )
{
	if( !in_uFileOffset && in_codecID )
	{
		return m_BankReader.SetFile( in_FileID, 0, in_codecID, in_pCookie, in_bIsLanguageSpecific );
	}
	const char * pszFileName = BankFileName( in_FileID );
	if( pszFileName )
	{
		return m_BankReader.SetFile( pszFileName, in_uFileOffset, in_pCookie );
	}
	return m_BankReader.SetFile( in_FileID, in_uFileOffset, 0, in_pCookie, true );
}

/** 2012 0x92aa90: the media bytes at their offset in the file that holds them. */
AKRESULT CAkBankMgr::LoadSoundFromFile( AkSrcTypeInfo & in_rMediaInfo, AkUInt8 * io_pData )
{
	m_BankReader.Reset();
	AKRESULT eResult = SetFileReader( in_rMediaInfo.mediaInfo.uFileID, in_rMediaInfo.mediaInfo.uFileOffset, in_rMediaInfo.dwID >> 16, NULL, in_rMediaInfo.mediaInfo.bIsLanguageSpecific );
	AkUInt32 uReadSize = 0;
	if( eResult == AK_Success )
	{
		eResult = m_BankReader.FillData( io_pData, in_rMediaInfo.mediaInfo.uInMemoryMediaSize, uReadSize );
		if( eResult == AK_Success && in_rMediaInfo.mediaInfo.uInMemoryMediaSize != uReadSize )
		{
			eResult = AK_Fail;
		}
	}
	m_BankReader.CloseFile();
	return eResult;
}

/** 2012 0x92c9b0: one more reference on the media; the first one copies it out of a loaded bank or reads it from
    its file into the prepared-event pool. */
AKRESULT CAkBankMgr::LoadSingleMedia( AkSrcTypeInfo & in_rMediaInfo )
{
	AkUInt32 uMediaSize = in_rMediaInfo.mediaInfo.uInMemoryMediaSize;
	if( !uMediaSize )
	{
		return AK_Success;
	}
	m_MediaLock.Lock();
	const AkUniqueID sourceID = in_rMediaInfo.mediaInfo.sourceID;
	AkMediaEntry * pEntry = m_MediaHashTable.Exists( sourceID );
	if( pEntry )
	{
		++pEntry->uRefCount;
		if( pEntry->m_preparedMediaInfo.pInMemoryData )
		{
			m_MediaLock.Unlock();
			return AK_Success;
		}
	}
	else
	{
		pEntry = m_MediaHashTable.Set( sourceID );
		if( !pEntry )
		{
			m_MediaLock.Unlock();
			return AK_Fail;
		}
		pEntry->m_sourceID = sourceID;
	}
	AkUInt8 * pData = NULL;
	AKRESULT eResult;
	if( pEntry->m_BankSlots.Length() )
	{
		eResult = pEntry->PrepareFromBank( pData, uMediaSize );
	}
	else
	{
		pData = (AkUInt8 *)AK::MemoryMgr::Malign( AkMediaEntry::m_uPrepareEventMemoryPoolID, uMediaSize, 16 );
		if( !pData )
		{
			eResult = AK_InsufficientMemory;
		}
		else
		{
			m_MediaLock.Unlock();
			eResult = LoadSoundFromFile( in_rMediaInfo, pData );
			m_MediaLock.Lock();
		}
	}
	if( eResult == AK_Success )
	{
		pEntry->m_preparedMediaInfo.uInMemoryDataSize = uMediaSize;
		pEntry->m_preparedMediaInfo.pInMemoryData = pData;
		m_MediaLock.Unlock();
		AKDIAG( L"akdiag: media 0x%08x prepared: %u bytes from %s 0x%08x", sourceID, uMediaSize, pEntry->m_BankSlots.Length() ? L"loaded bank" : L"file", in_rMediaInfo.mediaInfo.uFileID );
		return AK_Success;
	}
	if( pData )
	{
		AK::MemoryMgr::Free( AkMediaEntry::m_uPrepareEventMemoryPoolID, pData );
	}
	if( !--pEntry->uRefCount )
	{
		m_MediaHashTable.Unset( sourceID );
	}
	m_MediaLock.Unlock();
	AKDIAG( L"akdiag: media 0x%08x NOT prepared (%d): file 0x%08x offset %u size %u", sourceID, (AkInt32)eResult, in_rMediaInfo.mediaInfo.uFileID, in_rMediaInfo.mediaInfo.uFileOffset, uMediaSize );
	return eResult;
}

/** 2012 0x92d1d0: during PrepareEvent(s) the media are only collected (one entry per source), then loaded together. */
AKRESULT CAkBankMgr::PrepareSingleMedia( AkSrcTypeInfo & in_rMediaInfo )
{
	if( !m_bAccumulating )
	{
		return LoadSingleMedia( in_rMediaInfo );
	}
	AkUInt32 i = 0;
	for( ; i < m_PreparationAccumulator.Length(); ++i )
	{
		if( m_PreparationAccumulator[ i ].mediaInfo.sourceID >= in_rMediaInfo.mediaInfo.sourceID )
		{
			break;
		}
	}
	if( i < m_PreparationAccumulator.Length() && m_PreparationAccumulator[ i ].mediaInfo.sourceID == in_rMediaInfo.mediaInfo.sourceID )
	{
		m_PreparationAccumulator[ i ] = in_rMediaInfo;
		return AK_Success;
	}
	AkSrcTypeInfo * pItem = m_PreparationAccumulator.Insert( i );
	if( !pItem )
	{
		return AK_InsufficientMemory;
	}
	*pItem = in_rMediaInfo;
	return AK_Success;
}

/** 2012 0x92d240. */
void CAkBankMgr::UnprepareSingleMedia( AkUniqueID in_SourceID )
{
	if( !m_bAccumulating )
	{
		ReleaseSingleMedia( in_SourceID );
	}
}

/** 2012 0x92d260. */
AKRESULT CAkBankMgr::ProcessAccumulated()
{
	for( AkUInt32 i = 0; i < m_PreparationAccumulator.Length(); ++i )
	{
		const AKRESULT eResult = LoadSingleMedia( m_PreparationAccumulator[ i ] );
		if( eResult != AK_Success )
		{
			while( i )
			{
				ReleaseSingleMedia( m_PreparationAccumulator[ --i ].mediaInfo.sourceID );
			}
			return eResult;
		}
	}
	return AK_Success;
}

/** 2012 0x92dcc0: each Play action loads its bank structure (the action's file id) and prepares its node's media; a
    failure undoes the actions already prepared. The event is referenced while prepared. */
AKRESULT CAkBankMgr::PrepareEvent( AkBankQueueItem in_Item, AkUniqueID in_EventID )
{
	CAkEvent * pEvent = g_pIndex->m_idxEvents.GetPtrAndAddRef( in_EventID );
	if( !pEvent )
	{
		return AK_IDNotFound;
	}
	AKRESULT eResult = AK_Success;
	if( !pEvent->m_iPreparationCount )
	{
		CAkAction * pFailed = NULL;
		for( CAkAction * pAction = pEvent->m_actions.m_pFirst; pAction; pAction = pAction->pNextLightItem )
		{
			if( pAction->m_eActionType != AkActionType_Play )
			{
				continue;
			}
			const AkFileID fileID = static_cast<CAkActionPlay *>( pAction )->m_fileID;
			eResult = PrepareBankInternal( in_Item, fileID, AkLoadBankDataMode_Structure, false );
			if( eResult == AK_Success )
			{
				eResult = CAkParameterNodeBase::PrepareNodeData( pAction->m_ulElementID );
				if( eResult != AK_Success )
				{
					CAkUsageSlot * pSlot = m_BankList.Get( fileID );
					if( pSlot )
					{
						pSlot->ReleasePrepare( false );
					}
				}
			}
			if( eResult != AK_Success )
			{
				pFailed = pAction;
				break;
			}
		}
		for( CAkAction * pAction = pEvent->m_actions.m_pFirst; pFailed && pAction != pFailed; pAction = pAction->pNextLightItem )
		{
			if( pAction->m_eActionType == AkActionType_Play )
			{
				CAkParameterNodeBase::UnPrepareNodeData( pAction->m_ulElementID );
				CAkUsageSlot * pSlot = m_BankList.Get( static_cast<CAkActionPlay *>( pAction )->m_fileID );
				if( pSlot )
				{
					pSlot->ReleasePrepare( false );
				}
			}
		}
		if( eResult == AK_Success )
		{
			pEvent->AddRef();
		}
	}
	if( eResult == AK_Success )
	{
		++pEvent->m_iPreparationCount;
	}
	pEvent->Release();
	AKDIAG( L"akdiag: PrepareEvent 0x%08x: result %d, preparation count %u", in_EventID, (AkInt32)eResult, pEvent->m_iPreparationCount );
	return eResult;
}

/** 2012 0x92d740: the last unprepare releases every Play action's node data and bank structure. */
void CAkBankMgr::UnprepareEvent( CAkEvent * in_pEvent, bool in_bCompleteUnprepare )
{
	if( !in_pEvent->m_iPreparationCount )
	{
		return;
	}
	if( in_bCompleteUnprepare )
	{
		in_pEvent->m_iPreparationCount = 0;
	}
	else
	{
		--in_pEvent->m_iPreparationCount;
	}
	if( !in_pEvent->m_iPreparationCount )
	{
		for( CAkAction * pAction = in_pEvent->m_actions.m_pFirst; pAction; pAction = pAction->pNextLightItem )
		{
			if( pAction->m_eActionType == AkActionType_Play )
			{
				CAkParameterNodeBase::UnPrepareNodeData( pAction->m_ulElementID );
				CAkUsageSlot * pSlot = m_BankList.Get( static_cast<CAkActionPlay *>( pAction )->m_fileID );
				if( pSlot )
				{
					pSlot->ReleasePrepare( false );
				}
			}
		}
		in_pEvent->Release();
	}
}

/** 2012 0x92de00. */
AKRESULT CAkBankMgr::UnprepareEvent( AkUniqueID in_EventID )
{
	CAkEvent * pEvent = g_pIndex->m_idxEvents.GetPtrAndAddRef( in_EventID );
	if( !pEvent )
	{
		return AK_IDNotFound;
	}
	UnprepareEvent( pEvent, false );
	pEvent->Release();
	return AK_Success;
}

/** 2012 0x92e130. */
AKRESULT CAkBankMgr::PrepareEvents( AkBankQueueItem in_Item )
{
	AKRESULT eResult = AK_Success;
	m_bAccumulating = true;
	if( in_Item.prepare.numEvents == 1 )
	{
		eResult = PrepareEvent( in_Item, in_Item.prepare.eventID );
		if( eResult == AK_Success )
		{
			eResult = ProcessAccumulated();
			if( eResult != AK_Success )
			{
				UnprepareEvent( in_Item.prepare.eventID );
			}
		}
	}
	else
	{
		AkUInt32 i = 0;
		for( ; i < in_Item.prepare.numEvents; ++i )
		{
			eResult = PrepareEvent( in_Item, in_Item.prepare.pEventID[ i ] );
			if( eResult != AK_Success )
			{
				break;
			}
		}
		if( eResult != AK_Success )
		{
			while( i )
			{
				UnprepareEvent( in_Item.prepare.pEventID[ --i ] );
			}
		}
		else
		{
			eResult = ProcessAccumulated();
			if( eResult != AK_Success )
			{
				for( i = 0; i < in_Item.prepare.numEvents; ++i )
				{
					UnprepareEvent( in_Item.prepare.pEventID[ i ] );
				}
			}
		}
		AK::MemoryMgr::Free( g_DefaultPoolId, in_Item.prepare.pEventID );
		in_Item.prepare.eventID = 0;
	}
	m_bAccumulating = false;
	m_PreparationAccumulator.RemoveAll();
	NotifyCompletion( in_Item, eResult );
	return eResult;
}

/** 2012 0x92e310. */
AKRESULT CAkBankMgr::UnprepareEvents( AkBankQueueItem in_Item )
{
	if( in_Item.prepare.numEvents == 1 )
	{
		const AKRESULT eResult = UnprepareEvent( in_Item.prepare.eventID );
		NotifyCompletion( in_Item, eResult );
		return eResult;
	}
	AKRESULT eResult = AK_Success;
	for( AkUInt32 i = 0; i < in_Item.prepare.numEvents; ++i )
	{
		eResult = UnprepareEvent( in_Item.prepare.pEventID[ i ] );
		if( eResult != AK_Success )
		{
			break;
		}
	}
	AK::MemoryMgr::Free( g_DefaultPoolId, in_Item.prepare.pEventID );
	in_Item.prepare.eventID = 0;
	NotifyCompletion( in_Item, eResult );
	return eResult;
}

/** 2012 0x92dbf0. */
void CAkBankMgr::ClearPreparedEvents()
{
	AkAutoLock<CAkLock> mainGate( g_csMain );
	AkAutoLock<CAkLock> indexGate( g_pIndex->m_idxEvents.m_IndexLock );
	AkHashListBare<AkUniqueID, CAkIndexable, 193>::Iterator it = g_pIndex->m_idxEvents.m_mapIDToPtr.Begin();
	while( it.pItem )
	{
		CAkEvent * pEvent = static_cast<CAkEvent *>( it.pItem );
		++it;
		if( pEvent->m_iPreparationCount )
		{
			pEvent->AddRef();
			UnprepareEvent( pEvent, true );
			pEvent->Release();
		}
	}
}

/** 2012 0x92dea0. */
void CAkBankMgr::UnloadAll()
{
	ClearPreparedEvents();
	UnPrepareAllBank();
	AkHashListBare<AkBankID, CAkUsageSlot, 31>::IteratorEx it = m_BankList.m_ListLoadedBanks.BeginEx();
	while( it.pItem )
	{
		CAkUsageSlot * pSlot = it.pItem;
		it = m_BankList.m_ListLoadedBanks.Erase( it );
		pSlot->RemoveContent();
		pSlot->Release( true );
	}
}
