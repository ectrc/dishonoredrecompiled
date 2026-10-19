/*=============================================================================
	AkStreamMgr.cpp - AK::StreamMgr, CAkStreamMgr and AkDeferredOpenData.

	DISHONORED(port): agent IH1 (wave 26), replacing AkSilentStreamMgr.cpp; wwise\sdk\source\streammanager\
	common\akstreammgr.cpp, 2012 rvas:
	  GetDefaultSettings 0x96e3e0, GetDefaultDeviceSettings 0x96e3f0, Get/SetFileLocationResolver 0x96e450/
	  0x96e460, GetPoolID 0x96e470, GetCurrentLanguage 0x96e480, AkDeferredOpenData::Init x2 0x96e490/0x96e530,
	  Create x2 0x96e6c0/0x96ef70, Destroy 0x96e6f0, CAkStreamMgr::GetDevice 0x96e690, ForceCleanup 0x96e750,
	  CreateStd x2 0x96e7a0/0x96e900, CreateAuto x2 0x96ea50/0x96ebc0, SetCurrentLanguage 0x96ed60,
	  RemoveLanguageChangeObserver 0x96ee00, Create 0x96ee60, DestroyDevice 0x96eee0, Term 0x96efe0,
	  AddLanguageChangeObserver 0x96f080, Destroy 0x96f110, CreateDevice 0x96f160 (and the AK::StreamMgr
	  forwarders 0x96ef30 / 0x96ef50 / 0x96f0f0 / 0x96f290).
	Retail's GetDefaultDeviceSettings: 2 MB IO memory, 4-byte alignment, AkVirtualAlloc, 16 KB granularity,
	blocking scheduler, IO thread priority 1 with an 8 KB stack, 380 ms auto-stream target, 8 concurrent IO,
	cache ratio 1.0 (= no IO cache). The silent backend's guesses differed in five of those fields.

	Deviation (named): CreateDevice builds only CAkDeviceBlocking. The deferred lined-up device (flag 2) is
	never requested by Dishonored (AkAudio asks AK_SCHEDULER_BLOCKING, 2013 0x5b1430), so the flag is refused
	(AK_INVALID_DEVICE_ID) rather than ported unexercised.
	-akdiag: every stream creation is logged with the resolver's answer (DiagFileOpen).
=============================================================================*/
#include "AkStreamMgrPrivate.h"
#include "../AkSilentInternal.h"

AK::IAkStreamMgr * AK::IAkStreamMgr::m_pStreamMgr = NULL;

namespace AK
{
	namespace StreamMgr
	{
		AkMemPoolId CAkStreamMgr::m_streamMgrPoolId = AK_INVALID_POOL_ID;
		IAkFileLocationResolver * CAkStreamMgr::m_pFileLocationResolver = NULL;
		AkArray<CAkDeviceBase *, CAkDeviceBase *, CAkStreamMgr::ArrayPoolLocal, 1> CAkStreamMgr::m_arDevices;
		AkArray<CAkStreamMgr::LangChgObserver, CAkStreamMgr::LangChgObserver &, CAkStreamMgr::ArrayPoolLocal, 1> CAkStreamMgr::m_arLangChgObserver;

		static AkOSChar m_szCurrentLanguage[ 32 ];

		void DiagFileOpen( const AkOSChar * in_pszFileName, AkFileID in_fileID, const AkFileSystemFlags * in_pFlags, AKRESULT in_eResult, const AkFileDesc * in_pDesc )
		{
			if( !DishonoredWwise::IsDiagnostics() )
			{
				return;
			}
			wchar_t szName[ 32 ];
			if( !in_pszFileName )
			{
				swprintf_s( szName, L"id %u", in_fileID );
			}
			AkSilent::Logf( L"[akdiag] stream open %s (company %u, codec %u, language-specific %d) -> %d, size %I64d, sector %u, block %u",
				in_pszFileName ? in_pszFileName : szName,
				in_pFlags ? in_pFlags->uCompanyID : 0, in_pFlags ? in_pFlags->uCodecID : 0, in_pFlags ? (int)in_pFlags->bIsLanguageSpecific : 0,
				(int)in_eResult, in_eResult == AK_Success ? in_pDesc->iFileSize : 0, in_eResult == AK_Success ? in_pDesc->uSector : 0,
				in_eResult == AK_Success ? in_pDesc->uCustomParamSize : 0 );
		}

		// -- AkDeferredOpenData ---------------------------------------------------------------------

		AKRESULT AkDeferredOpenData::Init( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode )
		{
			bByString = 1;
			eOpenMode = in_eOpenMode;
			if( in_pFSFlags )
			{
				bUseFlags = 1;
				flags = *in_pFSFlags;
			}
			else
			{
				bUseFlags = 0;
			}
			const size_t uStrLen = wcslen( in_pszFileName );
			pszFileName = (AkOSChar *)AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, (AkUInt32)( sizeof( AkOSChar ) * ( uStrLen + 1 ) ) );
			if( !pszFileName )
			{
				return AK_Fail;
			}
			AKPLATFORM::SafeStrCpy( pszFileName, in_pszFileName, uStrLen + 1 );
			return AK_Success;
		}

		void AkDeferredOpenData::Init( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode )
		{
			bByString = 0;
			if( in_pFSFlags )
			{
				bUseFlags = 1;
				flags = *in_pFSFlags;
			}
			else
			{
				bUseFlags = 0;
			}
			eOpenMode = in_eOpenMode;
			fileID = in_fileID;
		}

		AkDeferredOpenData * AkDeferredOpenData::Create( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode )
		{
			AkDeferredOpenData * pData = (AkDeferredOpenData *)AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, sizeof( AkDeferredOpenData ) );
			if( pData )
			{
				pData->Init( in_fileID, in_pFSFlags, in_eOpenMode );
			}
			return pData;
		}

		AkDeferredOpenData * AkDeferredOpenData::Create( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode )
		{
			AkDeferredOpenData * pData = (AkDeferredOpenData *)AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, sizeof( AkDeferredOpenData ) );
			if( pData )
			{
				if( pData->Init( in_pszFileName, in_pFSFlags, in_eOpenMode ) != AK_Success )
				{
					pData->Destroy();
					return NULL;
				}
			}
			return pData;
		}

		void AkDeferredOpenData::Destroy()
		{
			if( bByString && pszFileName )
			{
				AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, pszFileName );
			}
			AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, this );
		}

		// -- CAkStreamMgr ----------------------------------------------------------------------------

		IAkStreamMgr * CAkStreamMgr::Create( const AkStreamMgrSettings & in_settings )
		{
			if( !AK::MemoryMgr::IsInitialized() )
			{
				return NULL;
			}
			if( !m_pStreamMgr )
			{
				m_szCurrentLanguage[ 0 ] = 0;
				if( m_streamMgrPoolId == AK_INVALID_POOL_ID )
				{
					m_streamMgrPoolId = AK::MemoryMgr::CreatePool( NULL, in_settings.uMemorySize, 32, AkMalloc, 0 );
					if( m_streamMgrPoolId == AK_INVALID_POOL_ID )
					{
						return NULL;
					}
				}
				void * pMem = AK::MemoryMgr::Malloc( m_streamMgrPoolId, sizeof( CAkStreamMgr ) );
				if( pMem )
				{
					m_pStreamMgr = AkPlacementNew( pMem ) CAkStreamMgr();
					return m_pStreamMgr;
				}
				AK::MemoryMgr::DestroyPool( m_streamMgrPoolId );
			}
			return m_pStreamMgr;
		}

		void CAkStreamMgr::Destroy()
		{
			Term();
			if( AK::MemoryMgr::IsInitialized() )
			{
				const AkMemPoolId poolId = m_streamMgrPoolId;
				if( poolId != AK_INVALID_POOL_ID )
				{
					this->~CAkStreamMgr();
					AK::MemoryMgr::Free( poolId, this );
				}
			}
			m_pStreamMgr = NULL;
			AK::MemoryMgr::DestroyPool( m_streamMgrPoolId );
			m_streamMgrPoolId = AK_INVALID_POOL_ID;
			AkSilent::Logf( L"StreamMgr::Destroy" );
		}

		void CAkStreamMgr::Term()
		{
			m_pFileLocationResolver = NULL;
			for( CAkDeviceBase ** it = m_arDevices.m_pItems; it != m_arDevices.m_pItemsEnd; ++it )
			{
				if( *it )
				{
					( *it )->Destroy();
				}
			}
			m_arDevices.Term();
			m_arLangChgObserver.Term();
		}

		CAkDeviceBase * CAkStreamMgr::GetDevice( AkDeviceID in_deviceID )
		{
			return in_deviceID < m_arDevices.Length() ? m_arDevices[ in_deviceID ] : NULL;
		}

		void CAkStreamMgr::ForceCleanup( CAkDeviceBase * in_pCallingDevice, AkPriority in_priority )
		{
			for( AkUInt32 i = 0; i < m_arDevices.Length(); ++i )
			{
				m_arDevices[ i ]->ForceCleanup( in_pCallingDevice == m_arDevices[ i ], in_priority );
			}
		}

		AkDeviceID CAkStreamMgr::CreateDevice( const AkDeviceSettings & in_settings, IAkLowLevelIOHook * in_pLowLevelHook )
		{
			AkDeviceID newDeviceID = AK_INVALID_DEVICE_ID;
			for( AkUInt32 i = 0; i < m_arDevices.Length(); ++i )
			{
				if( !m_arDevices[ i ] )
				{
					newDeviceID = i;
					break;
				}
			}
			if( newDeviceID == AK_INVALID_DEVICE_ID )
			{
				CAkDeviceBase ** ppSlot = m_arDevices.AddLast();
				if( !ppSlot )
				{
					return AK_INVALID_DEVICE_ID;
				}
				*ppSlot = NULL;
				newDeviceID = m_arDevices.Length() - 1;
			}

			CAkDeviceBase * pNewDevice = NULL;
			if( in_settings.uSchedulerTypeFlags & AK_SCHEDULER_BLOCKING )
			{
				void * pMem = AK::MemoryMgr::Malloc( m_streamMgrPoolId, sizeof( CAkDeviceBlocking ) );
				if( !pMem )
				{
					return AK_INVALID_DEVICE_ID;
				}
				pNewDevice = AkPlacementNew( pMem ) CAkDeviceBlocking( in_pLowLevelHook );
			}
			else
			{
				AkSilent::Logf( L"StreamMgr::CreateDevice: scheduler flags 0x%x name the deferred lined-up device, which Dishonored never uses and IH1 did not port", in_settings.uSchedulerTypeFlags );
				return AK_INVALID_DEVICE_ID;
			}

			if( pNewDevice->Init( in_settings, newDeviceID ) == AK_Success )
			{
				m_arDevices[ newDeviceID ] = pNewDevice;
				AkSilent::Logf( L"StreamMgr::CreateDevice %u: blocking, granularity %u, %u bytes of IO memory, IO thread priority %d",
					newDeviceID, in_settings.uGranularity, in_settings.uIOMemorySize, in_settings.threadProperties.nPriority );
				return newDeviceID;
			}
			pNewDevice->Destroy();
			return AK_INVALID_DEVICE_ID;
		}

		AKRESULT CAkStreamMgr::DestroyDevice( AkDeviceID in_deviceID )
		{
			if( in_deviceID >= m_arDevices.Length() || !m_arDevices[ in_deviceID ] )
			{
				return AK_InvalidParameter;
			}
			m_arDevices[ in_deviceID ]->Destroy();
			m_arDevices[ in_deviceID ] = NULL;
			return AK_Success;
		}

		/** 2012 0x96e7a0. The resolver opens the file in the caller's thread; a deferred open is done by the IO thread. */
		AKRESULT CAkStreamMgr::CreateStd( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream, bool in_bSyncOpen )
		{
			if( !in_pszFileName )
			{
				return AK_InvalidParameter;
			}
			if( in_pFSFlags )
			{
				in_pFSFlags->bIsAutomaticStream = false;
			}
			AkFileDesc * pFileDesc = (AkFileDesc *)AK::MemoryMgr::Malloc( m_streamMgrPoolId, sizeof( AkFileDesc ) );
			if( !pFileDesc )
			{
				return AK_Fail;
			}
			bool bSyncOpen = in_bSyncOpen;
			const AKRESULT eResult = m_pFileLocationResolver->Open( in_pszFileName, in_eOpenMode, in_pFSFlags, bSyncOpen, *pFileDesc );
			DiagFileOpen( in_pszFileName, AK_INVALID_FILE_ID, in_pFSFlags, eResult, pFileDesc );
			if( eResult != AK_Success )
			{
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return eResult;
			}
			CAkDeviceBase * pDevice = GetDevice( pFileDesc->deviceID );
			if( !pDevice )
			{
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return AK_Fail;
			}
			IAkStdStream * pStream = NULL;
			CAkStmTask * pTask = pDevice->CreateStd( pFileDesc, in_eOpenMode, pStream );
			if( !pTask )
			{
				if( bSyncOpen )
				{
					pDevice->GetLowLevelHook()->Close( *pFileDesc );
				}
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return AK_Fail;
			}
			if( bSyncOpen )
			{
				pTask->SetFileOpen( pFileDesc );
			}
			else if( pTask->SetDeferredFileOpen( pFileDesc, in_pszFileName, in_pFSFlags, in_eOpenMode ) != AK_Success )
			{
				pTask->SetToBeDestroyed();
				pTask->Kill();
				return AK_Fail;
			}
			out_pStream = pStream;
			return AK_Success;
		}

		AKRESULT CAkStreamMgr::CreateStd( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream, bool in_bSyncOpen )
		{
			if( in_pFSFlags )
			{
				in_pFSFlags->bIsAutomaticStream = false;
			}
			AkFileDesc * pFileDesc = (AkFileDesc *)AK::MemoryMgr::Malloc( m_streamMgrPoolId, sizeof( AkFileDesc ) );
			if( !pFileDesc )
			{
				return AK_Fail;
			}
			bool bSyncOpen = in_bSyncOpen;
			const AKRESULT eResult = m_pFileLocationResolver->Open( in_fileID, in_eOpenMode, in_pFSFlags, bSyncOpen, *pFileDesc );
			DiagFileOpen( NULL, in_fileID, in_pFSFlags, eResult, pFileDesc );
			if( eResult != AK_Success )
			{
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return eResult;
			}
			CAkDeviceBase * pDevice = GetDevice( pFileDesc->deviceID );
			if( !pDevice )
			{
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return AK_Fail;
			}
			IAkStdStream * pStream = NULL;
			CAkStmTask * pTask = pDevice->CreateStd( pFileDesc, in_eOpenMode, pStream );
			if( !pTask )
			{
				if( bSyncOpen )
				{
					pDevice->GetLowLevelHook()->Close( *pFileDesc );
				}
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return AK_Fail;
			}
			if( bSyncOpen )
			{
				pTask->SetFileOpen( pFileDesc );
			}
			else if( pTask->SetDeferredFileOpen( pFileDesc, in_fileID, in_pFSFlags, in_eOpenMode ) != AK_Success )
			{
				pTask->SetToBeDestroyed();
				pTask->Kill();
				return AK_Fail;
			}
			out_pStream = pStream;
			return AK_Success;
		}

		AKRESULT CAkStreamMgr::CreateAuto( const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream, bool in_bSyncOpen )
		{
			if( !in_pszFileName || in_heuristics.fThroughput < 0.f || (AkUInt8)in_heuristics.priority > AK_MAX_PRIORITY )
			{
				return AK_InvalidParameter;
			}
			if( in_pFSFlags )
			{
				in_pFSFlags->bIsAutomaticStream = true;
			}
			AkFileDesc * pFileDesc = (AkFileDesc *)AK::MemoryMgr::Malloc( m_streamMgrPoolId, sizeof( AkFileDesc ) );
			if( !pFileDesc )
			{
				return AK_Fail;
			}
			bool bSyncOpen = in_bSyncOpen;
			const AKRESULT eResult = m_pFileLocationResolver->Open( in_pszFileName, AK_OpenModeRead, in_pFSFlags, bSyncOpen, *pFileDesc );
			DiagFileOpen( in_pszFileName, AK_INVALID_FILE_ID, in_pFSFlags, eResult, pFileDesc );
			if( eResult != AK_Success )
			{
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return eResult;
			}
			CAkDeviceBase * pDevice = GetDevice( pFileDesc->deviceID );
			if( !pDevice )
			{
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return AK_Fail;
			}
			IAkAutoStream * pStream = NULL;
			CAkStmTask * pTask = pDevice->CreateAuto( pFileDesc, AK_INVALID_FILE_ID, in_heuristics, in_pBufferSettings, pStream );
			if( !pTask )
			{
				if( bSyncOpen )
				{
					pDevice->GetLowLevelHook()->Close( *pFileDesc );
				}
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return AK_Fail;
			}
			if( bSyncOpen )
			{
				pTask->SetFileOpen( pFileDesc );
			}
			else if( pTask->SetDeferredFileOpen( pFileDesc, in_pszFileName, in_pFSFlags, AK_OpenModeRead ) != AK_Success )
			{
				pTask->SetToBeDestroyed();
				pTask->Kill();
				return AK_Fail;
			}
			out_pStream = pStream;
			return AK_Success;
		}

		/** 2012 0x96ebc0: a file id is the cache key only for Audiokinetic files (uCompanyID 0). */
		AKRESULT CAkStreamMgr::CreateAuto( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream, bool in_bSyncOpen )
		{
			if( in_heuristics.fThroughput < 0.f || (AkUInt8)in_heuristics.priority > AK_MAX_PRIORITY )
			{
				return AK_InvalidParameter;
			}
			AkUInt32 uCompanyID = 0;
			if( in_pFSFlags )
			{
				uCompanyID = in_pFSFlags->uCompanyID;
				in_pFSFlags->bIsAutomaticStream = true;
			}
			AkFileDesc * pFileDesc = (AkFileDesc *)AK::MemoryMgr::Malloc( m_streamMgrPoolId, sizeof( AkFileDesc ) );
			if( !pFileDesc )
			{
				return AK_Fail;
			}
			bool bSyncOpen = in_bSyncOpen;
			const AKRESULT eResult = m_pFileLocationResolver->Open( in_fileID, AK_OpenModeRead, in_pFSFlags, bSyncOpen, *pFileDesc );
			DiagFileOpen( NULL, in_fileID, in_pFSFlags, eResult, pFileDesc );
			if( eResult != AK_Success )
			{
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return eResult;
			}
			CAkDeviceBase * pDevice = GetDevice( pFileDesc->deviceID );
			if( !pDevice )
			{
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return AK_Fail;
			}
			IAkAutoStream * pStream = NULL;
			CAkStmTask * pTask = pDevice->CreateAuto( pFileDesc, uCompanyID ? AK_INVALID_FILE_ID : in_fileID, in_heuristics, in_pBufferSettings, pStream );
			if( !pTask )
			{
				if( bSyncOpen )
				{
					pDevice->GetLowLevelHook()->Close( *pFileDesc );
				}
				AK::MemoryMgr::Free( m_streamMgrPoolId, pFileDesc );
				return AK_Fail;
			}
			if( bSyncOpen )
			{
				pTask->SetFileOpen( pFileDesc );
			}
			else if( pTask->SetDeferredFileOpen( pFileDesc, in_fileID, in_pFSFlags, AK_OpenModeRead ) != AK_Success )
			{
				pTask->SetToBeDestroyed();
				pTask->Kill();
				return AK_Fail;
			}
			out_pStream = pStream;
			return AK_Success;
		}

		/** 2012 0x96ed60: at most 31 characters, no trailing separator; observers are called last-registered first. */
		AKRESULT CAkStreamMgr::SetCurrentLanguage( const AkOSChar * in_pszLanguageName )
		{
			if( !in_pszLanguageName )
			{
				return AK_Fail;
			}
			const size_t uLen = wcslen( in_pszLanguageName );
			if( uLen >= 32 )
			{
				return AK_Fail;
			}
			if( uLen && ( in_pszLanguageName[ uLen - 1 ] == L'/' || in_pszLanguageName[ uLen - 1 ] == L'\\' ) )
			{
				return AK_Fail;
			}
			AKPLATFORM::SafeStrCpy( m_szCurrentLanguage, in_pszLanguageName, 32 );
			for( AkUInt32 i = m_arLangChgObserver.Length(); i > 0; --i )
			{
				m_arLangChgObserver[ i - 1 ].handler( m_szCurrentLanguage, m_arLangChgObserver[ i - 1 ].pCookie );
			}
			AkSilent::Logf( L"StreamMgr::SetCurrentLanguage: %s", m_szCurrentLanguage );
			return AK_Success;
		}

		AKRESULT CAkStreamMgr::AddLanguageChangeObserver( AkLanguageChangeHandler in_handler, void * in_pCookie )
		{
			LangChgObserver * pObserver = m_arLangChgObserver.AddLast();
			if( !pObserver )
			{
				return AK_Fail;
			}
			pObserver->handler = in_handler;
			pObserver->pCookie = in_pCookie;
			return AK_Success;
		}

		void CAkStreamMgr::RemoveLanguageChangeObserver( void * in_pCookie )
		{
			LangChgObserver * it = m_arLangChgObserver.m_pItems;
			while( it != m_arLangChgObserver.m_pItemsEnd )
			{
				if( it->pCookie == in_pCookie )
				{
					m_arLangChgObserver.Erase( (AkUInt32)( it - m_arLangChgObserver.m_pItems ) );
				}
				else
				{
					++it;
				}
			}
		}

		// -- AK::StreamMgr ---------------------------------------------------------------------------

		void GetDefaultSettings( AkStreamMgrSettings & out_settings )
		{
			out_settings.uMemorySize = 0x10000;
		}

		void GetDefaultDeviceSettings( AkDeviceSettings & out_settings )
		{
			out_settings.pIOMemory = NULL;
			out_settings.uIOMemorySize = 0x200000;
			out_settings.uIOMemoryAlignment = 4;
			out_settings.ePoolAttributes = AkVirtualAlloc;
			out_settings.uGranularity = 0x4000;
			out_settings.uSchedulerTypeFlags = AK_SCHEDULER_BLOCKING;
			out_settings.threadProperties.uStackSize = 0x2000;
			out_settings.threadProperties.dwAffinityMask = 0;
			out_settings.fTargetAutoStmBufferLength = 380.f;
			out_settings.threadProperties.nPriority = THREAD_PRIORITY_ABOVE_NORMAL;
			out_settings.uMaxConcurrentIO = 8;
			out_settings.fMaxCacheRatio = 1.f;
		}

		IAkFileLocationResolver * GetFileLocationResolver()
		{
			return CAkStreamMgr::m_pFileLocationResolver;
		}

		void SetFileLocationResolver( IAkFileLocationResolver * in_pFileLocationResolver )
		{
			CAkStreamMgr::m_pFileLocationResolver = in_pFileLocationResolver;
		}

		AkMemPoolId GetPoolID()
		{
			return CAkStreamMgr::m_streamMgrPoolId;
		}

		const AkOSChar * GetCurrentLanguage()
		{
			return m_szCurrentLanguage;
		}

		IAkStreamMgr * Create( const AkStreamMgrSettings & in_settings )
		{
			IAkStreamMgr * pStreamMgr = CAkStreamMgr::Create( in_settings );
			AkSilent::Logf( L"StreamMgr::Create: %u bytes of stream manager memory -> %s", in_settings.uMemorySize, pStreamMgr ? L"ok" : L"failed" );
			return pStreamMgr;
		}

		AkDeviceID CreateDevice( const AkDeviceSettings & in_settings, IAkLowLevelIOHook * in_pLowLevelHook )
		{
			return static_cast<CAkStreamMgr *>( IAkStreamMgr::Get() )->CreateDevice( in_settings, in_pLowLevelHook );
		}

		AKRESULT DestroyDevice( AkDeviceID in_deviceID )
		{
			return static_cast<CAkStreamMgr *>( IAkStreamMgr::Get() )->DestroyDevice( in_deviceID );
		}

		AKRESULT SetCurrentLanguage( const AkOSChar * in_pszLanguageName )
		{
			return static_cast<CAkStreamMgr *>( IAkStreamMgr::Get() )->SetCurrentLanguage( in_pszLanguageName );
		}

		AKRESULT AddLanguageChangeObserver( AkLanguageChangeHandler in_handler, void * in_pCookie )
		{
			return static_cast<CAkStreamMgr *>( IAkStreamMgr::Get() )->AddLanguageChangeObserver( in_handler, in_pCookie );
		}

		void RemoveLanguageChangeObserver( void * in_pCookie )
		{
			static_cast<CAkStreamMgr *>( IAkStreamMgr::Get() )->RemoveLanguageChangeObserver( in_pCookie );
		}
	}
}
