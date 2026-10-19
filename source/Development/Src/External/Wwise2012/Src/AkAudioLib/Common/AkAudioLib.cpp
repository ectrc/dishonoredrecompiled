/*=============================================================================
	AkAudioLib.cpp - AK::SoundEngine's engine lifetime (the IH1 half), banks, name hashing and the Query
	functions that answer from the bank hierarchy and the game object registry.

	DISHONORED(port): agent IH1 (wave 26); wwise\sdk\source\soundengine\akaudiolib\win32\akaudiolib.cpp.
	  GetDefaultInitSettings 0x916e00, GetDefaultPlatformInitSettings 0x916e50 (via CAkLEngine 0x9212e0)
	  Init 0x91ad50 -> CreateDefaultMemPools 0x918270 + PreInit 0x919cc0 (index, RTPC, environments, bank
	  manager, registry, state manager in that order); Term 0x91a090 (bank thread stopped first, managers
	  terminated in retail order, temp objects released, default pool destroyed)
	  LoadBank x8 0x917cf0..0x918c60, UnloadBank x6, PrepareBank x6, PrepareEvent x6, PrepareGameSyncs x4,
	  ClearBanks 0x9189e0, ClearPreparedEvents 0x9180d0, CancelBankCallbackCookie 0x917fc0,
	  SetBankLoadIOSettings 0x917a90, DefaultBankCallbackFunc 0x917a70, AkSyncCaller::Wait 0x916da0,
	  GetIDFromString 0x918570, GetBankIDFromString 0x918a70
	  Query::QueryAudioObjectIDs 0x91b720, GetPositioningInfo 0x91b7e0, GetPosition 0x91b250,
	  GetActiveListeners 0x91b2f0, GetIsGameObjectActive 0x91b120, GetGameObjectEnvironmentsValues 0x91b540,
	  GetGameObjectDryLevelValue 0x91b610, GetObjectObstructionAndOcclusion 0x91b680, GetState 0x91af90
	The playback half of Init/Term (CAkURenderer, CAkAudioMgr, CAkLEngine, the playing / transition / path
	managers) is IH2's; AkSilentSoundEngine.cpp keeps the silent playback until then and calls
	AkAudioLibInit / AkAudioLibTerm from AK::SoundEngine::Init / Term.
	-akdiag logs every GetIDFromString with its answer, every Query::QueryAudioObjectIDs / GetPositioningInfo
	with its result, and (in AkBankMgr.cpp) every bank operation.
=============================================================================*/
#include "AkBankMgr.h"
#include "AkRegistryMgr.h"

#include <AK/SoundEngine/Common/AkQueryParameters.h>

AkMemPoolId						g_DefaultPoolId = AK_INVALID_POOL_ID;
AkInitSettings					g_settings;
AkPlatformInitSettings			g_PDSettings;
CAkLock							g_csMain;
CAkAudioLibIndex *				g_pIndex = NULL;
CAkBankMgr *					g_pBankManager = NULL;
CAkRegistryMgr *				g_pRegistryMgr = NULL;
CAkStateMgr *					g_pStateMgr = NULL;
CAkRTPCMgr *					g_pRTPCMgr = NULL;
CAkEnvironmentsMgr *			g_pEnvironmentMgr = NULL;
AkExternalBankHandlerCallback	g_pExternalBankHandlerCallback = NULL;
AkSoundPosition					g_DefaultSoundPosition = { { 0.f, 0.f, 1.f }, { 0.f, 0.f, -1.f } };

namespace AK
{
	AkReal32 g_fFreqRatio = 0.f;
}

namespace AkAudioLibSettings
{
	AkUInt32 g_pipelineCoreFrequency = 48000;
}

static AK::SoundEngine::AkCommandPriority g_eVolumeThresholdPriority = AK::SoundEngine::AkCommandPriority_None;
static AK::SoundEngine::AkCommandPriority g_eNumVoicesPriority = AK::SoundEngine::AkCommandPriority_None;

namespace AkSilent
{
	void ApplyVolumeThreshold( AkReal32 in_fVolumeThresholdDB );
	void ApplyMaxNumVoices( AkUInt16 in_maxNumberVoices );
	AkUInt32 RegisterName( const wchar_t * name );
	AkUInt32 RegisterName( const char * name );
	DishonoredWwise::Stats & MutableStats();
}

template <class T>
static T * NewManager()
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( T ) );
	return pMem ? AkPlacementNew( pMem ) T() : NULL;
}

template <class T>
static void DeleteManager( T *& io_pManager )
{
	if( io_pManager )
	{
		io_pManager->~T();
		AK::MemoryMgr::Free( g_DefaultPoolId, io_pManager );
		io_pManager = NULL;
	}
}

/** AK::SoundEngine::Init's bank-side work, from the settings copy to PreInit. */
AKRESULT AkAudioLibInit( AkInitSettings * in_pSettings, AkPlatformInitSettings * in_pPlatformSettings )
{
	g_eVolumeThresholdPriority = AK::SoundEngine::AkCommandPriority_None;
	g_eNumVoicesPriority = AK::SoundEngine::AkCommandPriority_None;
	if( in_pSettings )
	{
		g_settings = *in_pSettings;
	}
	else
	{
		AK::SoundEngine::GetDefaultInitSettings( g_settings );
	}
	if( in_pPlatformSettings )
	{
		g_PDSettings = *in_pPlatformSettings;
	}
	else
	{
		AK::SoundEngine::GetDefaultPlatformInitSettings( g_PDSettings );
	}
	if( g_PDSettings.uNumRefillsInVoice < 2 )
	{
		g_PDSettings.uNumRefillsInVoice = 4;
	}

	if( g_DefaultPoolId == AK_INVALID_POOL_ID )
	{
		const AkUInt32 uDefaultPoolSize = g_settings.uDefaultPoolSize <= 64 ? 0x1000000 : g_settings.uDefaultPoolSize;
		g_DefaultPoolId = AK::MemoryMgr::CreatePool( NULL, uDefaultPoolSize, 64, AkMalloc, 0 );
		if( g_DefaultPoolId == AK_INVALID_POOL_ID )
		{
			return AK_InsufficientMemory;
		}
	}

	AkInt64 iFrequency = 0;
	AKPLATFORM::PerformanceFrequency( &iFrequency );
	AK::g_fFreqRatio = (AkReal32)( iFrequency / 1000 );

	AKRESULT eResult = AK_Success;
	if( !g_pIndex )
	{
		g_pIndex = NewManager<CAkAudioLibIndex>();
		if( !g_pIndex )
		{
			return AK_InsufficientMemory;
		}
		g_pIndex->Init();
	}
	if( !g_pRTPCMgr )
	{
		g_pRTPCMgr = NewManager<CAkRTPCMgr>();
		eResult = g_pRTPCMgr ? g_pRTPCMgr->Init() : AK_InsufficientMemory;
	}
	if( eResult == AK_Success && !g_pEnvironmentMgr )
	{
		g_pEnvironmentMgr = NewManager<CAkEnvironmentsMgr>();
		eResult = g_pEnvironmentMgr ? g_pEnvironmentMgr->Init() : AK_InsufficientMemory;
	}
	if( eResult == AK_Success && !g_pBankManager )
	{
		g_pBankManager = NewManager<CAkBankMgr>();
		eResult = g_pBankManager ? g_pBankManager->Init() : AK_InsufficientMemory;
	}
	if( eResult == AK_Success && !g_pRegistryMgr )
	{
		g_pRegistryMgr = NewManager<CAkRegistryMgr>();
		eResult = g_pRegistryMgr ? g_pRegistryMgr->Init() : AK_InsufficientMemory;
	}
	if( eResult == AK_Success && !g_pStateMgr )
	{
		g_pStateMgr = NewManager<CAkStateMgr>();
		eResult = g_pStateMgr ? g_pStateMgr->Init() : AK_InsufficientMemory;
	}
	if( in_pSettings )
	{
		*in_pSettings = g_settings;
	}
	AKDIAG( L"akdiag: SoundEngine::Init: default pool %d (%u bytes), bank manager thread %s, result %d",
		g_DefaultPoolId, g_settings.uDefaultPoolSize, CAkBankMgr::m_BankMgrThread ? L"started" : L"missing", (AkInt32)eResult );
	return eResult;
}

/** AK::SoundEngine::Term's bank-side work, in retail order. */
void AkAudioLibTerm()
{
	if( g_pBankManager )
	{
		g_pBankManager->StopThread();
		g_pBankManager->Term();
		DeleteManager( g_pBankManager );
	}
	if( g_pStateMgr )
	{
		g_pStateMgr->Term();
		DeleteManager( g_pStateMgr );
	}
	if( g_pRegistryMgr )
	{
		g_pRegistryMgr->Term();
		DeleteManager( g_pRegistryMgr );
	}
	if( g_pEnvironmentMgr )
	{
		g_pEnvironmentMgr->Term();
		DeleteManager( g_pEnvironmentMgr );
	}
	if( g_pRTPCMgr )
	{
		g_pRTPCMgr->Term();
		DeleteManager( g_pRTPCMgr );
	}
	if( g_pIndex )
	{
		g_pIndex->Term();
		DeleteManager( g_pIndex );
	}
	if( g_DefaultPoolId != AK_INVALID_POOL_ID )
	{
		AK::MemoryMgr::DestroyPool( g_DefaultPoolId );
		g_DefaultPoolId = AK_INVALID_POOL_ID;
	}
}

/** CAkAudioLibIndex::Init 0x9347e0. */
void CAkAudioLibIndex::Init()
{
	m_idxAudioNode.m_mapIDToPtr.Init();
	m_idxCustomStates.m_mapIDToPtr.Init();
	m_idxEvents.m_mapIDToPtr.Init();
	m_idxActions.m_mapIDToPtr.Init();
	m_idxLayers.m_mapIDToPtr.Init();
	m_idxAttenuations.m_mapIDToPtr.Init();
	m_idxDynamicSequences.m_mapIDToPtr.Init();
	m_idxDialogueEvents.m_mapIDToPtr.Init();
	m_idxFxShareSets.m_mapIDToPtr.Init();
	m_idxFxCustom.m_mapIDToPtr.Init();
}

/** CAkAudioLibIndex::Term: the maps are intrusive, the objects were released with their banks. */
void CAkAudioLibIndex::Term()
{
	ReleaseTempObjects();
}

/** 2012 0x934710: objects with a "temporary" id (high bit set, created by the authoring tool) are released. */
void CAkAudioLibIndex::ReleaseTempObjects()
{
	{
		AkAutoLock<CAkLock> gate( m_idxAudioNode.m_IndexLock );
		AkHashListBare<AkUniqueID, CAkIndexable, 193>::Iterator it = m_idxAudioNode.m_mapIDToPtr.Begin();
		while( it.pItem )
		{
			CAkIndexable * pItem = it.pItem;
			++it;
			if( (AkInt32)pItem->key < 0 )
			{
				pItem->Release();
			}
		}
	}
	AkHashListBare<AkUniqueID, CAkIndexable, 193>::Iterator it = m_idxFxCustom.m_mapIDToPtr.Begin();
	while( it.pItem )
	{
		CAkIndexable * pItem = it.pItem;
		++it;
		if( (AkInt32)pItem->key < 0 )
		{
			pItem->Release();
		}
	}
}

namespace AK
{
	namespace SoundEngine
	{
		/** 2012 0x916e00. */
		void GetDefaultInitSettings( AkInitSettings & out_settings )
		{
			out_settings.uMaxNumPaths = 255;
			out_settings.uMaxNumTransitions = 255;
			out_settings.pfnAssertHook = NULL;
			out_settings.uDefaultPoolSize = 0x1000000;
			out_settings.fDefaultPoolRatioThreshold = 1.f;
			out_settings.uCommandQueueSize = 0x40000;
			out_settings.uPrepareEventMemoryPoolID = AK_INVALID_POOL_ID;
			out_settings.bEnableGameSyncPreparation = false;
			out_settings.uContinuousPlaybackLookAhead = 1;
			out_settings.uMonitorPoolSize = 0x40000;
			out_settings.uMonitorQueuePoolSize = 0x10000;
		}

		/** 2012 0x916e50 -> CAkLEngine::GetDefaultPlatformInitSettings 0x9212e0. */
		void GetDefaultPlatformInitSettings( AkPlatformInitSettings & out_settings )
		{
			memset( &out_settings, 0, sizeof( out_settings ) );
			out_settings.hWnd = GetForegroundWindow();
			out_settings.threadLEngine.nPriority = THREAD_PRIORITY_ABOVE_NORMAL;
			out_settings.threadLEngine.dwAffinityMask = 0;
			out_settings.threadLEngine.uStackSize = 0x2000;
			out_settings.threadBankManager.nPriority = THREAD_PRIORITY_NORMAL;
			out_settings.threadBankManager.dwAffinityMask = 0;
			out_settings.threadBankManager.uStackSize = 0x2000;
			out_settings.uLEngineDefaultPoolSize = 0x1000000;
			out_settings.fLEngineDefaultPoolRatioThreshold = 1.f;
			out_settings.uNumRefillsInVoice = 4;
			out_settings.eAudioQuality = AkSoundQuality_High;
			out_settings.uChannelMask = 0;
			out_settings.bGlobalFocus = true;
			out_settings.threadMonitor.nPriority = THREAD_PRIORITY_ABOVE_NORMAL;
			out_settings.threadMonitor.dwAffinityMask = 0;
			out_settings.threadMonitor.uStackSize = 0x2000;
		}

		/** AK::SoundEngine::SetVolumeThresholdInternal: a lower (stronger) priority source wins. */
		AKRESULT SetVolumeThresholdInternal( AkReal32 in_fVolumeThresholdDB, AkCommandPriority in_eCommandPriority )
		{
			if( in_eCommandPriority > g_eVolumeThresholdPriority )
			{
				return AK_Success;
			}
			g_eVolumeThresholdPriority = in_eCommandPriority;
			AkSilent::ApplyVolumeThreshold( in_fVolumeThresholdDB );
			return AK_Success;
		}

		AKRESULT SetMaxNumVoicesLimitInternal( AkUInt16 in_maxNumberVoices, AkCommandPriority in_eCommandPriority )
		{
			if( in_eCommandPriority > g_eNumVoicesPriority )
			{
				return AK_Success;
			}
			g_eNumVoicesPriority = in_eCommandPriority;
			AkSilent::ApplyMaxNumVoices( in_maxNumberVoices );
			return AK_Success;
		}

		AKRESULT SetVolumeThreshold( AkReal32 in_fVolumeThresholdDB )
		{
			return SetVolumeThresholdInternal( in_fVolumeThresholdDB, AkCommandPriority_Game );
		}

		AKRESULT SetMaxNumVoicesLimit( AkUInt16 in_maxNumberVoices )
		{
			return SetMaxNumVoicesLimitInternal( in_maxNumberVoices, AkCommandPriority_Game );
		}

		// -- name hashing --------------------------------------------------------------------------

		/** 2012 0x918570 (char form inlined into the wide one): ANSI, A-Z lowered, 32-bit FNV-1. */
		AkUInt32 GetIDFromString( const char * in_pszString )
		{
			if( !in_pszString )
			{
				return 0;
			}
			char szString[ AK_DEFAULT_PATH_LENGTH ];
			AKPLATFORM::SafeStrCpy( szString, in_pszString, AK_DEFAULT_PATH_LENGTH );
			const size_t uLen = strlen( szString );
			for( size_t i = 0; i < uLen; ++i )
			{
				if( szString[ i ] >= 'A' && szString[ i ] <= 'Z' )
				{
					szString[ i ] += 'a' - 'A';
				}
			}
			AkUInt32 uHash = 2166136261u;
			for( size_t i = 0; i < uLen; ++i )
			{
				uHash = ( uHash * 16777619u ) ^ (AkUInt8)szString[ i ];
			}
			return uHash;
		}

		AkUInt32 GetIDFromString( const AkOSChar * in_pszString )
		{
			if( !in_pszString )
			{
				return 0;
			}
			char szString[ AK_DEFAULT_PATH_LENGTH ];
			AKPLATFORM::AkWideCharToChar( in_pszString, AK_DEFAULT_PATH_LENGTH - 1, szString );
			szString[ AK_DEFAULT_PATH_LENGTH - 1 ] = 0;
			const AkUInt32 uID = GetIDFromString( szString );
			AkSilent::RegisterName( in_pszString );
			AKDIAG( L"akdiag: GetIDFromString(\"%s\") = 0x%08x", in_pszString, uID );
			return uID;
		}

		/** 2012 0x918a70: the name up to its first '.', hashed, remembered as "name.bnk" for the bank reader. */
		static AkBankID GetBankIDFromStringInternal( const char * in_pszString )
		{
			char szStringWithoutExtension[ AK_DEFAULT_PATH_LENGTH ];
			AKPLATFORM::SafeStrCpy( szStringWithoutExtension, in_pszString, AK_DEFAULT_PATH_LENGTH - 1 );
			char * pDot = strchr( szStringWithoutExtension, '.' );
			if( pDot )
			{
				*pDot = 0;
			}
			const AkBankID bankID = GetIDFromString( szStringWithoutExtension );
			if( g_pBankManager )
			{
				g_pBankManager->UpdateBankName( bankID, szStringWithoutExtension );
			}
			AkSilent::RegisterName( szStringWithoutExtension );
			return bankID;
		}

		static AkBankID GetBankIDFromStringWide( const AkOSChar * in_pszString )
		{
			char szAnsi[ AK_DEFAULT_PATH_LENGTH ];
			AKPLATFORM::AkWideCharToChar( in_pszString, AK_DEFAULT_PATH_LENGTH - 1, szAnsi );
			szAnsi[ AK_DEFAULT_PATH_LENGTH - 1 ] = 0;
			return GetBankIDFromStringInternal( szAnsi );
		}

		AkBankID GetBankIDFromString( const AkOSChar * in_pszString )
		{
			return GetBankIDFromStringWide( in_pszString );
		}

		// -- the synchronous bank calls ------------------------------------------------------------

		/** AkSyncCaller / AkBankSyncLoader: {result, event, pool id}; DefaultBankCallbackFunc fills it. */
		struct AkBankSyncLoader
		{
			AKRESULT	m_eResult;
			HANDLE		m_hEvent;
			AkMemPoolId	memPoolId;

			AkBankSyncLoader() : m_eResult( AK_Fail ), m_hEvent( NULL ), memPoolId( AK_INVALID_POOL_ID ) {}

			/** 2012 0x916da0. */
			AKRESULT Wait( AKRESULT in_eResult )
			{
				if( in_eResult == AK_Success )
				{
					WaitForSingleObject( m_hEvent, INFINITE );
					CloseHandle( m_hEvent );
					m_hEvent = NULL;
					return m_eResult;
				}
				if( m_hEvent )
				{
					CloseHandle( m_hEvent );
				}
				m_hEvent = NULL;
				return in_eResult;
			}
		};

		/** 2012 0x917a70. */
		void DefaultBankCallbackFunc( AkBankID, AKRESULT in_eLoadResult, AkMemPoolId in_memPoolId, void * in_pCookie )
		{
			AkBankSyncLoader * pLoader = (AkBankSyncLoader *)in_pCookie;
			pLoader->m_eResult = in_eLoadResult;
			pLoader->memPoolId = in_memPoolId;
			SetEvent( pLoader->m_hEvent );
		}

		static CAkBankMgr::AkBankQueueItem MakeItem( CAkBankMgr::AkBankQueueItemType in_eType, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, CAkBankMgr::AkBankLoadFlag in_eFlag )
		{
			CAkBankMgr::AkBankQueueItem item;
			memset( &item, 0, sizeof( item ) );
			item.eType = in_eType;
			item.callbackInfo.pfnBankCallback = in_pfnBankCallback;
			item.callbackInfo.pCookie = in_pCookie;
			item.bankLoadFlag = in_eFlag;
			return item;
		}

		static AKRESULT QueueAndWait( CAkBankMgr::AkBankQueueItem in_Item, AkBankSyncLoader & io_loader )
		{
			if( !g_pBankManager )
			{
				return AK_Fail;
			}
			io_loader.m_hEvent = CreateEventW( NULL, FALSE, FALSE, NULL );
			if( !io_loader.m_hEvent )
			{
				return AK_Fail;
			}
			in_Item.callbackInfo.pfnBankCallback = DefaultBankCallbackFunc;
			in_Item.callbackInfo.pCookie = &io_loader;
			return io_loader.Wait( g_pBankManager->QueueBankCommand( in_Item ) );
		}

		static AKRESULT Queue( const CAkBankMgr::AkBankQueueItem & in_Item )
		{
			return g_pBankManager ? g_pBankManager->QueueBankCommand( in_Item ) : AK_Fail;
		}

		// -- LoadBank ------------------------------------------------------------------------------

		/** 2012 0x918b10. */
		AKRESULT LoadBank( const AkOSChar * in_pszString, AkMemPoolId in_memPoolId, AkBankID & out_bankID )
		{
			out_bankID = GetBankIDFromStringWide( in_pszString );
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemLoad, NULL, NULL, CAkBankMgr::AkBankLoadFlag_None );
			item.load.bankID = out_bankID;
			item.load.memPoolId = in_memPoolId;
			AkBankSyncLoader loader;
			const AKRESULT eResult = QueueAndWait( item, loader );
			DishonoredWwise::Stats & stats = AkSilent::MutableStats();
			if( eResult == AK_Success || eResult == AK_BankAlreadyLoaded )
			{
				++stats.BanksLoaded;
			}
			else
			{
				++stats.BanksMissing;
			}
			return eResult;
		}

		/** 2012 0x918bd0. */
		AKRESULT LoadBank( const char * in_pszString, AkMemPoolId in_memPoolId, AkBankID & out_bankID )
		{
			out_bankID = GetBankIDFromStringInternal( in_pszString );
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemLoad, NULL, NULL, CAkBankMgr::AkBankLoadFlag_None );
			item.load.bankID = out_bankID;
			item.load.memPoolId = in_memPoolId;
			AkBankSyncLoader loader;
			return QueueAndWait( item, loader );
		}

		/** 2012 0x917cf0. */
		AKRESULT LoadBank( AkBankID in_bankID, AkMemPoolId in_memPoolId )
		{
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemLoad, NULL, NULL, CAkBankMgr::AkBankLoadFlag_UsingFileID );
			item.load.bankID = in_bankID;
			item.load.memPoolId = in_memPoolId;
			AkBankSyncLoader loader;
			return QueueAndWait( item, loader );
		}

		/** 2012 0x917d80. */
		AKRESULT LoadBank( const void * in_pInMemoryBankPtr, AkUInt32 in_uInMemoryBankSize, AkBankID & out_bankID )
		{
			if( ( (AkUIntPtr)in_pInMemoryBankPtr & 0xF ) || !in_pInMemoryBankPtr || in_uInMemoryBankSize < 0x18 )
			{
				return AK_InvalidParameter;
			}
			out_bankID = CAkBankMgr::GetBankIDFromInMemorySpace( in_pInMemoryBankPtr, in_uInMemoryBankSize );
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemLoad, NULL, NULL, CAkBankMgr::AkBankLoadFlag_InMemory );
			item.load.bankID = out_bankID;
			item.load.memPoolId = AK_INVALID_POOL_ID;
			item.load.pInMemoryBank = in_pInMemoryBankPtr;
			item.load.ui32InMemoryBankSize = in_uInMemoryBankSize;
			AkBankSyncLoader loader;
			return QueueAndWait( item, loader );
		}

		/** 2012 0x918c60. */
		AKRESULT LoadBank( const AkOSChar * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkMemPoolId in_memPoolId, AkBankID & out_bankID )
		{
			out_bankID = GetBankIDFromStringWide( in_pszString );
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemLoad, in_pfnBankCallback, in_pCookie, CAkBankMgr::AkBankLoadFlag_None );
			item.load.bankID = out_bankID;
			item.load.memPoolId = in_memPoolId;
			++AkSilent::MutableStats().BanksLoaded;
			return Queue( item );
		}

		AKRESULT LoadBank( const char * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkMemPoolId in_memPoolId, AkBankID & out_bankID )
		{
			out_bankID = GetBankIDFromStringInternal( in_pszString );
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemLoad, in_pfnBankCallback, in_pCookie, CAkBankMgr::AkBankLoadFlag_None );
			item.load.bankID = out_bankID;
			item.load.memPoolId = in_memPoolId;
			return Queue( item );
		}

		/** 2012 0x917e10. */
		AKRESULT LoadBank( AkBankID in_bankID, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkMemPoolId in_memPoolId )
		{
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemLoad, in_pfnBankCallback, in_pCookie, CAkBankMgr::AkBankLoadFlag_UsingFileID );
			item.load.bankID = in_bankID;
			item.load.memPoolId = in_memPoolId;
			return Queue( item );
		}

		/** 2012 0x917e70. */
		AKRESULT LoadBank( const void * in_pInMemoryBankPtr, AkUInt32 in_uInMemoryBankSize, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankID & out_bankID )
		{
			if( ( (AkUIntPtr)in_pInMemoryBankPtr & 0xF ) || !in_pInMemoryBankPtr || in_uInMemoryBankSize < 0x18 )
			{
				return AK_InvalidParameter;
			}
			out_bankID = CAkBankMgr::GetBankIDFromInMemorySpace( in_pInMemoryBankPtr, in_uInMemoryBankSize );
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemLoad, in_pfnBankCallback, in_pCookie, CAkBankMgr::AkBankLoadFlag_InMemory );
			item.load.bankID = out_bankID;
			item.load.memPoolId = AK_INVALID_POOL_ID;
			item.load.pInMemoryBank = in_pInMemoryBankPtr;
			item.load.ui32InMemoryBankSize = in_uInMemoryBankSize;
			return Queue( item );
		}

		// -- UnloadBank ----------------------------------------------------------------------------

		/** 2012 0x917ec0. */
		AKRESULT UnloadBank( AkBankID in_bankID, AkMemPoolId * out_pMemPoolId )
		{
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemUnload, NULL, NULL, CAkBankMgr::AkBankLoadFlag_None );
			item.load.bankID = in_bankID;
			item.load.memPoolId = AK_INVALID_POOL_ID;
			AkBankSyncLoader loader;
			const AKRESULT eResult = QueueAndWait( item, loader );
			if( out_pMemPoolId )
			{
				*out_pMemPoolId = loader.memPoolId;
			}
			return eResult;
		}

		/** 2012 0x918d70. */
		AKRESULT UnloadBank( const AkOSChar * in_pszString, AkMemPoolId * out_pMemPoolId )
		{
			return UnloadBank( GetBankIDFromStringWide( in_pszString ), out_pMemPoolId );
		}

		AKRESULT UnloadBank( const char * in_pszString, AkMemPoolId * out_pMemPoolId )
		{
			return UnloadBank( GetBankIDFromStringInternal( in_pszString ), out_pMemPoolId );
		}

		/** 2012 0x917f60. */
		AKRESULT UnloadBank( AkBankID in_bankID, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemUnload, in_pfnBankCallback, in_pCookie, CAkBankMgr::AkBankLoadFlag_None );
			item.load.bankID = in_bankID;
			item.load.memPoolId = AK_INVALID_POOL_ID;
			return Queue( item );
		}

		/** 2012 0x918de0. */
		AKRESULT UnloadBank( const AkOSChar * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			return UnloadBank( GetBankIDFromStringWide( in_pszString ), in_pfnBankCallback, in_pCookie );
		}

		AKRESULT UnloadBank( const char * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			return UnloadBank( GetBankIDFromStringInternal( in_pszString ), in_pfnBankCallback, in_pCookie );
		}

		/** 2012 0x917fc0. */
		void CancelBankCallbackCookie( void * in_pCookie )
		{
			if( g_pBankManager )
			{
				g_pBankManager->m_CallbackMgr.CancelCookie( in_pCookie );
			}
		}

		/** 2012 0x917a90. */
		AKRESULT SetBankLoadIOSettings( AkReal32 in_fThroughput, AkPriority in_priority )
		{
			return g_pBankManager ? g_pBankManager->m_BankReader.SetBankLoadIOSettings( in_fThroughput, in_priority ) : AK_Fail;
		}

		/** 2012 0x9180d0. */
		AKRESULT ClearPreparedEvents()
		{
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemUnprepareAllEvents, NULL, NULL, CAkBankMgr::AkBankLoadFlag_None );
			AkBankSyncLoader loader;
			return QueueAndWait( item, loader );
		}

		/** 2012 0x9189e0. */
		AKRESULT ClearBanks()
		{
			if( !g_pBankManager )
			{
				return AK_Fail;
			}
			AKRESULT eResult = ClearPreparedEvents();
			if( eResult != AK_Success )
			{
				return eResult;
			}
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemClearBanks, NULL, NULL, CAkBankMgr::AkBankLoadFlag_None );
			AkBankSyncLoader loader;
			return QueueAndWait( item, loader );
		}

		// -- PrepareBank ---------------------------------------------------------------------------

		/** 2012 0x917fe0: queue type 6 (load) or 7 (unload); prepare = {bank id, content flags}. */
		AKRESULT PrepareBank( PreparationType in_PreparationType, AkBankID in_bankID, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankContent in_uFlags )
		{
			CAkBankMgr::AkBankQueueItem item = MakeItem( in_PreparationType == Preparation_Load ? CAkBankMgr::QueueItemPrepareBank : CAkBankMgr::QueueItemUnprepareBank, in_pfnBankCallback, in_pCookie, CAkBankMgr::AkBankLoadFlag_None );
			item.prepare.numEvents = in_bankID;
			item.prepare.eventID = in_uFlags;
			return Queue( item );
		}

		/** 2012 0x918ee0. */
		AKRESULT PrepareBank( PreparationType in_PreparationType, AkBankID in_bankID, AkBankContent in_uFlags )
		{
			if( !g_pBankManager )
			{
				return AK_Fail;
			}
			AkBankSyncLoader loader;
			loader.m_hEvent = CreateEventW( NULL, FALSE, FALSE, NULL );
			if( !loader.m_hEvent )
			{
				return AK_Fail;
			}
			const AKRESULT eResult = loader.Wait( PrepareBank( in_PreparationType, in_bankID, DefaultBankCallbackFunc, &loader, in_uFlags ) );
			if( in_PreparationType == Preparation_Load )
			{
				DishonoredWwise::Stats & stats = AkSilent::MutableStats();
				if( eResult == AK_Success )
				{
					++stats.BanksPrepared;
				}
				else
				{
					++stats.BanksMissing;
				}
			}
			return eResult;
		}

		/** 2012 0x919a50. */
		AKRESULT PrepareBank( PreparationType in_PreparationType, const AkOSChar * in_pszString, AkBankContent in_uFlags )
		{
			return PrepareBank( in_PreparationType, GetBankIDFromStringWide( in_pszString ), in_uFlags );
		}

		AKRESULT PrepareBank( PreparationType in_PreparationType, const char * in_pszString, AkBankContent in_uFlags )
		{
			return PrepareBank( in_PreparationType, GetBankIDFromStringInternal( in_pszString ), in_uFlags );
		}

		/** 2012 0x918f40. */
		AKRESULT PrepareBank( PreparationType in_PreparationType, const AkOSChar * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankContent in_uFlags )
		{
			return PrepareBank( in_PreparationType, GetBankIDFromStringWide( in_pszString ), in_pfnBankCallback, in_pCookie, in_uFlags );
		}

		AKRESULT PrepareBank( PreparationType in_PreparationType, const char * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankContent in_uFlags )
		{
			return PrepareBank( in_PreparationType, GetBankIDFromStringInternal( in_pszString ), in_pfnBankCallback, in_pCookie, in_uFlags );
		}

		// -- PrepareEvent --------------------------------------------------------------------------

		/** 2012 0x917b20 (PrepareEventInternal): one id travels in the item, several in a copied array. */
		static AKRESULT PrepareEventInternal( AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, PreparationType in_PreparationType, const AkUniqueID * in_pEventID, AkUInt32 in_uNumEvent )
		{
			if( !in_uNumEvent )
			{
				return AK_InvalidParameter;
			}
			CAkBankMgr::AkBankQueueItem item = MakeItem( in_PreparationType == Preparation_Load ? CAkBankMgr::QueueItemPrepareEvent : CAkBankMgr::QueueItemUnprepareEvent, in_pfnBankCallback, in_pCookie, CAkBankMgr::AkBankLoadFlag_None );
			item.prepare.numEvents = in_uNumEvent;
			if( in_uNumEvent == 1 )
			{
				item.prepare.eventID = in_pEventID[ 0 ];
			}
			else
			{
				item.prepare.pEventID = (AkUniqueID *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkUniqueID ) * in_uNumEvent );
				if( !item.prepare.pEventID )
				{
					return AK_InsufficientMemory;
				}
				memcpy( item.prepare.pEventID, in_pEventID, sizeof( AkUniqueID ) * in_uNumEvent );
			}
			const AKRESULT eResult = Queue( item );
			if( eResult != AK_Success && in_uNumEvent != 1 )
			{
				AK::MemoryMgr::Free( g_DefaultPoolId, item.prepare.pEventID );
			}
			return eResult;
		}

		/** 2012 0x918040. */
		AKRESULT PrepareEvent( PreparationType in_PreparationType, AkUniqueID * in_pEventID, AkUInt32 in_uNumEvent )
		{
			if( !g_pBankManager )
			{
				return AK_Fail;
			}
			AkBankSyncLoader loader;
			loader.m_hEvent = CreateEventW( NULL, FALSE, FALSE, NULL );
			if( !loader.m_hEvent )
			{
				return AK_Fail;
			}
			return loader.Wait( PrepareEventInternal( DefaultBankCallbackFunc, &loader, in_PreparationType, in_pEventID, in_uNumEvent ) );
		}

		/** 2012 0x9180a0. */
		AKRESULT PrepareEvent( PreparationType in_PreparationType, AkUniqueID * in_pEventID, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			return PrepareEventInternal( in_pfnBankCallback, in_pCookie, in_PreparationType, in_pEventID, in_uNumEvent );
		}

		template <class CHAR>
		static AKRESULT PrepareEventByName( PreparationType in_PreparationType, const CHAR ** in_ppszString, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, bool in_bSync )
		{
			if( !in_uNumEvent )
			{
				return AK_InvalidParameter;
			}
			AkUniqueID * pIDs = (AkUniqueID *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkUniqueID ) * in_uNumEvent );
			if( !pIDs )
			{
				return AK_InsufficientMemory;
			}
			for( AkUInt32 i = 0; i < in_uNumEvent; ++i )
			{
				pIDs[ i ] = GetIDFromString( in_ppszString[ i ] );
			}
			const AKRESULT eResult = in_bSync ? PrepareEvent( in_PreparationType, pIDs, in_uNumEvent ) : PrepareEvent( in_PreparationType, pIDs, in_uNumEvent, in_pfnBankCallback, in_pCookie );
			AK::MemoryMgr::Free( g_DefaultPoolId, pIDs );
			return eResult;
		}

		/** 2012 0x918fb0. */
		AKRESULT PrepareEvent( PreparationType in_PreparationType, const AkOSChar ** in_ppszString, AkUInt32 in_uNumEvent )
		{
			return PrepareEventByName( in_PreparationType, in_ppszString, in_uNumEvent, NULL, NULL, true );
		}

		AKRESULT PrepareEvent( PreparationType in_PreparationType, const char ** in_ppszString, AkUInt32 in_uNumEvent )
		{
			return PrepareEventByName( in_PreparationType, in_ppszString, in_uNumEvent, NULL, NULL, true );
		}

		AKRESULT PrepareEvent( PreparationType in_PreparationType, const AkOSChar ** in_ppszString, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			return PrepareEventByName( in_PreparationType, in_ppszString, in_uNumEvent, in_pfnBankCallback, in_pCookie, false );
		}

		AKRESULT PrepareEvent( PreparationType in_PreparationType, const char ** in_ppszString, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			return PrepareEventByName( in_PreparationType, in_ppszString, in_uNumEvent, in_pfnBankCallback, in_pCookie, false );
		}

		// -- PrepareGameSyncs (queued; the preparation itself is IH3's, see AkBankMgr.cpp) ----------

		/** 2012 0x917c00 (PrepareGameSyncsInternal). */
		static AKRESULT PrepareGameSyncsInternal( PreparationType in_PreparationType, AkGroupType in_eGameSyncType, AkUInt32 in_GroupID, const AkUInt32 * in_paGameSyncID, AkUInt32 in_uNumGameSyncs, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			if( !in_uNumGameSyncs || !in_paGameSyncID )
			{
				return AK_InvalidParameter;
			}
			CAkBankMgr::AkBankQueueItem item = MakeItem( CAkBankMgr::QueueItemSupportedGameSync, in_pfnBankCallback, in_pCookie, CAkBankMgr::AkBankLoadFlag_None );
			item.gameSync.eGroupType = in_eGameSyncType;
			item.gameSync.uGroupID = in_GroupID;
			item.gameSync.bSupported = in_PreparationType == Preparation_Load;
			item.gameSync.uNumGameSync = in_uNumGameSyncs;
			item.gameSync.uGameSyncID = in_paGameSyncID[ 0 ];
			return Queue( item );
		}

		/** 2012 0x918190. */
		AKRESULT PrepareGameSyncs( PreparationType in_PreparationType, AkGroupType in_eGameSyncType, AkUInt32 in_GroupID, AkUInt32 * in_paGameSyncID, AkUInt32 in_uNumGameSyncs )
		{
			if( !g_pBankManager )
			{
				return AK_Fail;
			}
			AkBankSyncLoader loader;
			loader.m_hEvent = CreateEventW( NULL, FALSE, FALSE, NULL );
			if( !loader.m_hEvent )
			{
				return AK_Fail;
			}
			return loader.Wait( PrepareGameSyncsInternal( in_PreparationType, in_eGameSyncType, in_GroupID, in_paGameSyncID, in_uNumGameSyncs, DefaultBankCallbackFunc, &loader ) );
		}

		AKRESULT PrepareGameSyncs( PreparationType in_PreparationType, AkGroupType in_eGameSyncType, const AkOSChar * in_pszGroupName, const AkOSChar ** in_ppszGameSyncName, AkUInt32 in_uNumGameSyncs )
		{
			if( !in_uNumGameSyncs || !in_ppszGameSyncName )
			{
				return AK_InvalidParameter;
			}
			AkUInt32 aIDs[ 64 ];
			const AkUInt32 uNum = in_uNumGameSyncs < 64 ? in_uNumGameSyncs : 64;
			for( AkUInt32 i = 0; i < uNum; ++i )
			{
				aIDs[ i ] = GetIDFromString( in_ppszGameSyncName[ i ] );
			}
			return PrepareGameSyncs( in_PreparationType, in_eGameSyncType, GetIDFromString( in_pszGroupName ), aIDs, uNum );
		}

		AKRESULT PrepareGameSyncs( PreparationType in_PreparationType, AkGroupType in_eGameSyncType, const char * in_pszGroupName, const char ** in_ppszGameSyncName, AkUInt32 in_uNumGameSyncs )
		{
			if( !in_uNumGameSyncs || !in_ppszGameSyncName )
			{
				return AK_InvalidParameter;
			}
			AkUInt32 aIDs[ 64 ];
			const AkUInt32 uNum = in_uNumGameSyncs < 64 ? in_uNumGameSyncs : 64;
			for( AkUInt32 i = 0; i < uNum; ++i )
			{
				aIDs[ i ] = GetIDFromString( in_ppszGameSyncName[ i ] );
			}
			return PrepareGameSyncs( in_PreparationType, in_eGameSyncType, GetIDFromString( in_pszGroupName ), aIDs, uNum );
		}

		// -- Query -----------------------------------------------------------------------------------

		namespace Query
		{
			/** 2012 0x91b720. */
			AKRESULT QueryAudioObjectIDs( AkUniqueID in_eventID, AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos )
			{
				if( io_ruNumItems && !out_aObjectInfos )
				{
					return AK_InvalidParameter;
				}
				if( !g_pIndex )
				{
					io_ruNumItems = 0;
					return AK_IDNotFound;
				}
				const AkUInt32 uAsked = io_ruNumItems;
				AkAutoLock<CAkLock> gate( g_csMain );
				CAkEvent * pEvent = g_pIndex->m_idxEvents.GetPtrAndAddRef( in_eventID );
				if( !pEvent )
				{
					AKDIAG( L"akdiag: Query::QueryAudioObjectIDs(0x%08x, %u) = AK_IDNotFound (event not loaded)", in_eventID, uAsked );
					return AK_IDNotFound;
				}
				const AKRESULT eResult = pEvent->QueryAudioObjectIDs( io_ruNumItems, out_aObjectInfos );
				pEvent->Release();
				AKDIAG( L"akdiag: Query::QueryAudioObjectIDs(0x%08x, %u) = %d, %u objects", in_eventID, uAsked, (AkInt32)eResult, io_ruNumItems );
				return eResult;
			}

			/** 2012 0x91b7a0. */
			AKRESULT QueryAudioObjectIDs( const AkOSChar * in_pszEventName, AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos )
			{
				const AkUniqueID eventID = GetIDFromString( in_pszEventName );
				return eventID ? QueryAudioObjectIDs( eventID, io_ruNumItems, out_aObjectInfos ) : AK_IDNotFound;
			}

			/** 2012 0x91b7c0. */
			AKRESULT QueryAudioObjectIDs( const char * in_pszEventName, AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos )
			{
				const AkUniqueID eventID = GetIDFromString( in_pszEventName );
				return eventID ? QueryAudioObjectIDs( eventID, io_ruNumItems, out_aObjectInfos ) : AK_IDNotFound;
			}

			/** 2012 0x91b7e0: buses have no positioning (AK_NotCompatible). */
			AKRESULT GetPositioningInfo( AkUniqueID in_ObjectID, AkPositioningInfo & out_rPositioningInfo )
			{
				if( !g_pIndex )
				{
					return AK_IDNotFound;
				}
				AkAutoLock<CAkLock> gate( g_csMain );
				CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_ObjectID );
				if( !pNode )
				{
					AKDIAG( L"akdiag: Query::GetPositioningInfo(0x%08x) = AK_IDNotFound", in_ObjectID );
					return AK_IDNotFound;
				}
				AKRESULT eResult = AK_NotCompatible;
				if( pNode->NodeCategory() != AkNodeCategory_Bus )
				{
					eResult = static_cast<CAkParameterNode *>( pNode )->GetStatic3DParams( out_rPositioningInfo );
				}
				pNode->Release();
				AKDIAG( L"akdiag: Query::GetPositioningInfo(0x%08x) = %d: type %d, attenuation %d, max distance %.2f, center %.2f",
					in_ObjectID, (AkInt32)eResult, (AkInt32)out_rPositioningInfo.positioningType, (AkInt32)out_rPositioningInfo.bUseAttenuation,
					out_rPositioningInfo.fMaxDistance, out_rPositioningInfo.fCenterPct );
				return eResult;
			}

			/** 2012 0x91b250. */
			AKRESULT GetPosition( AkGameObjectID in_GameObjectID, AkSoundPosition & out_rPosition )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				CAkRegisteredObj * pObj = g_pRegistryMgr ? g_pRegistryMgr->GetObj( in_GameObjectID ) : NULL;
				if( !pObj )
				{
					return AK_IDNotFound;
				}
				out_rPosition = pObj->m_PosKeep.m_aPos ? pObj->m_PosKeep.m_aPos[ 0 ] : g_DefaultSoundPosition;
				return AK_Success;
			}

			/** 2012 0x91b2f0. */
			AKRESULT GetActiveListeners( AkGameObjectID in_GameObjectID, AkUInt32 & out_ruListenerMask )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				CAkRegisteredObj * pObj = g_pRegistryMgr ? g_pRegistryMgr->GetObj( in_GameObjectID ) : NULL;
				if( !pObj )
				{
					return AK_IDNotFound;
				}
				out_ruListenerMask = pObj->m_PosKeep.m_uListenerMask;
				return AK_Success;
			}

			/** 2012 0x91b120. */
			bool GetIsGameObjectActive( AkGameObjectID in_GameObjectID )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				return g_pRegistryMgr && g_pRegistryMgr->IsGameObjectActive( in_GameObjectID );
			}

			/** 2012 0x91b540: AK_PartialSuccess when the caller's array is smaller than the send count. */
			AKRESULT GetGameObjectEnvironmentsValues( AkGameObjectID in_GameObjectID, AkEnvironmentValue * out_paEnvironmentValues, AkUInt32 & io_ruNumEnvValues )
			{
				if( !io_ruNumEnvValues || !out_paEnvironmentValues )
				{
					return AK_InvalidParameter;
				}
				AkAutoLock<CAkLock> gate( g_csMain );
				CAkRegisteredObj * pObj = g_pRegistryMgr ? g_pRegistryMgr->GetObj( in_GameObjectID ) : NULL;
				if( !pObj )
				{
					return AK_IDNotFound;
				}
				AkUInt32 uCount = 0;
				while( uCount < 4 && pObj->m_EnvironmentValues[ uCount ].EnvID )
				{
					++uCount;
				}
				AKRESULT eResult = AK_Success;
				if( io_ruNumEnvValues < uCount )
				{
					uCount = io_ruNumEnvValues;
					eResult = AK_PartialSuccess;
				}
				io_ruNumEnvValues = uCount;
				memcpy( out_paEnvironmentValues, pObj->m_EnvironmentValues, sizeof( AkEnvironmentValue ) * uCount );
				return eResult;
			}

			/** 2012 0x91b610. */
			AKRESULT GetGameObjectDryLevelValue( AkGameObjectID in_GameObjectID, AkReal32 & out_rfControlValue )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				CAkRegisteredObj * pObj = g_pRegistryMgr ? g_pRegistryMgr->GetObj( in_GameObjectID ) : NULL;
				if( !pObj )
				{
					return AK_IDNotFound;
				}
				out_rfControlValue = pObj->m_fDryLevelValue;
				return AK_Success;
			}

			/** 2012 0x91b680. */
			AKRESULT GetObjectObstructionAndOcclusion( AkGameObjectID in_ObjectID, AkUInt32 in_uListener, AkReal32 & out_rfObstructionLevel, AkReal32 & out_rfOcclusionLevel )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				CAkRegisteredObj * pObj = g_pRegistryMgr ? g_pRegistryMgr->GetObj( in_ObjectID ) : NULL;
				if( !pObj || in_uListener >= 8 )
				{
					return AK_IDNotFound;
				}
				out_rfObstructionLevel = pObj->m_fObstructionValue[ in_uListener ] * ( 1.f / 255.f );
				out_rfOcclusionLevel = pObj->m_fOcclusionValue[ in_uListener ] * ( 1.f / 255.f );
				return AK_Success;
			}

			/** 2012 0x91af90. */
			AKRESULT GetState( AkStateGroupID in_stateGroup, AkStateID & out_rState )
			{
				AkAutoLock<CAkLock> gate( g_csMain );
				out_rState = g_pStateMgr ? g_pStateMgr->GetState( in_stateGroup ) : 0;
				return AK_Success;
			}

			AKRESULT GetState( const AkOSChar * in_pstrStateGroupName, AkStateID & out_rState )
			{
				return GetState( GetIDFromString( in_pstrStateGroupName ), out_rState );
			}

			AKRESULT GetState( const char * in_pstrStateGroupName, AkStateID & out_rState )
			{
				return GetState( GetIDFromString( in_pstrStateGroupName ), out_rState );
			}
		}
	}
}

/** The silent engine's PostEvent asks this under -akdiag: retail PostEvent fails for an event not in the index. */
bool AkAudioLibIsEventLoaded( AkUniqueID in_eventID )
{
	if( !g_pIndex )
	{
		return false;
	}
	CAkEvent * pEvent = g_pIndex->m_idxEvents.GetPtrAndAddRef( in_eventID );
	if( !pEvent )
	{
		return false;
	}
	pEvent->Release();
	return true;
}

/** -akdiag / CoreSmoke: how many objects each index holds. */
void AkAudioLibIndexCounts( AkUInt32 out_counts[ 10 ] )
{
	CAkIndexItemBase * aItems[ 10 ] =
	{
		&g_pIndex->m_idxAudioNode, &g_pIndex->m_idxCustomStates, &g_pIndex->m_idxEvents, &g_pIndex->m_idxActions,
		&g_pIndex->m_idxLayers, &g_pIndex->m_idxAttenuations, &g_pIndex->m_idxDynamicSequences,
		&g_pIndex->m_idxDialogueEvents, &g_pIndex->m_idxFxShareSets, &g_pIndex->m_idxFxCustom
	};
	for( AkUInt32 i = 0; i < 10; ++i )
	{
		out_counts[ i ] = g_pIndex ? aItems[ i ]->m_mapIDToPtr.Length() : 0;
	}
}

/** CoreSmoke: the last bank operation's HIRC counts, by HIRC type, and the failures. */
void AkAudioLibLastBankCounts( AkUInt32 out_counts[ 20 ], AkUInt32 out_failures[ 20 ], AkUInt32 out_misparsed[ 20 ], AkUInt32 & out_uBytes )
{
	for( AkUInt32 i = 0; i < AkBank::HIRCType_Count; ++i )
	{
		out_counts[ i ] = g_pBankManager ? g_pBankManager->m_aHircCounts[ i ] : 0;
		out_failures[ i ] = g_pBankManager ? g_pBankManager->m_aHircFailures[ i ] : 0;
		out_misparsed[ i ] = g_pBankManager ? g_pBankManager->m_aHircMisparsed[ i ] : 0;
	}
	out_uBytes = g_pBankManager ? g_pBankManager->m_uLoadBytes : 0;
}

/** CoreSmoke / IH2: the node an id names, its parent, child count and positioning owner. */
bool AkAudioLibDescribeNode( AkUniqueID in_nodeID, AkUInt32 & out_category, AkUniqueID & out_parentID, AkUniqueID & out_busID )
{
	if( !g_pIndex )
	{
		return false;
	}
	CAkParameterNodeBase * pNode = g_pIndex->m_idxAudioNode.GetPtrAndAddRef( in_nodeID );
	if( !pNode )
	{
		return false;
	}
	out_category = pNode->NodeCategory();
	out_parentID = pNode->m_pParentNode ? pNode->m_pParentNode->key : 0;
	out_busID = pNode->m_pBusOutputNode ? pNode->m_pBusOutputNode->key : 0;
	pNode->Release();
	return true;
}
