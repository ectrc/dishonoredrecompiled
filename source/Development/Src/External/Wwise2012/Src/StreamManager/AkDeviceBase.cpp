/*=============================================================================
	AkDeviceBase.cpp - CAkDeviceBase, CAkStmTask, CAkStdStmBase, CAkAutoStmBase.

	DISHONORED(port): agent IH1 (wave 26); wwise\sdk\source\streammanager\common\akdevicebase.cpp (58 functions,
	2012 0x96f2b0..0x971850). The interesting ones:
	  CAkStmTask: OnFileDeferredOpen 0x96f2f0, TimeSinceLastTransfer 0x96f300, ClampRequestSizeToEof 0x96f330,
	    SetDeferredFileOpen x2 0x96f480/0x96f4c0, EnsureFileIsOpen 0x96f500, ~CAkStmTask 0x96fbb0
	  CAkDeviceBase: Init 0x96f3e0, OnThreadStart 0x96f470, DestroyMemView 0x96fae0, Destroy 0x970600,
	    AddTask 0x9706e0, ClearStreams 0x970e50, ScheduleStdStmOnly 0x970ee0, ForceCleanup 0x9710c0,
	    SchedulerFindNextTask 0x9714c0
	  CAkStdStmBase: SetPosition 0x96f680, EffectiveDeadline 0x96f7c0, SetStatus 0x96f840, Init 0x96fce0,
	    ExecuteOp 0x96fd60, AddMemView 0x96fec0, UpdateTaskStatus 0x96ffd0, Kill 0x970070
	  CAkAutoStmBase: GetEffectiveViewSize 0x96f380, SetStreamName 0x96f5d0, SetBufferingSettings 0x96f930 (+Gcd
	    0x96f8b0), GetNominalBuffering 0x96fa30, OnFileDeferredOpen 0x96fa70, Init 0x9701c0, GetPosition 0x9702f0,
	    QueryBufferingStatus 0x970370, AddMemView 0x970400, UpdateSchedulingStatus 0x9704c0, SetHeuristics 0x970850,
	    Start 0x970b30, ReleaseBuffer 0x970bb0, UpdateTaskStatus 0x970c70, Kill 0x970cb0, Flush 0x970cf0,
	    Destroy 0x9711a0, SetMinimalBufferSize 0x971260, Stop 0x971320, ForceFilePosition 0x971350,
	    GetReadBuffer 0x9713f0, SetPosition 0x971730, GetBuffer 0x971850
	The auto streams are ported for completeness of the device (IH2's streamed media are their client); the
	bank reader of this package uses standard streams only.
=============================================================================*/
#include "AkStreamMgrPrivate.h"

namespace AK
{
	namespace StreamMgr
	{
		// -- CAkStmTask ------------------------------------------------------------------------------

		CAkStmTask::CAkStmTask()
			: pNextLightItem( NULL )
			, m_pDeferredOpenData( NULL )
			, m_pFileDesc( NULL )
			, m_iIOStartTime( 0 )
			, m_pDevice( NULL )
			, m_pszStreamName( NULL )
			, m_uLLBlockSize( 0 )
			, m_priority( AK_DEFAULT_PRIORITY )
			, m_bIsAutoStm( 0 )
			, m_bIsWriteOp( 0 )
			, m_bHasReachedEof( 0 )
			, m_bIsToBeDestroyed( 0 )
			, m_bIsFileOpen( 0 )
			, m_bRequiresScheduling( 0 )
			, m_bIsReadyForIO( 0 )
		{
		}

		CAkStmTask::~CAkStmTask()
		{
			if( m_bIsFileOpen )
			{
				m_pDevice->GetLowLevelHook()->Close( *m_pFileDesc );
			}
			if( m_pDeferredOpenData )
			{
				m_pDeferredOpenData->Destroy();
				m_pDeferredOpenData = NULL;
			}
			if( m_pszStreamName )
			{
				AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, m_pszStreamName );
			}
			if( m_pFileDesc )
			{
				AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, m_pFileDesc );
			}
		}

		void CAkStmTask::OnFileDeferredOpen()
		{
			m_bIsFileOpen = true;
		}

		AkReal32 CAkStmTask::TimeSinceLastTransfer( const AkInt64 & in_liNow ) const
		{
			return (AkReal32)( (AkReal64)( in_liNow - m_iIOStartTime ) / AK::g_fFreqRatio );
		}

		AkUInt32 CAkStmTask::ClampRequestSizeToEof( AkUInt64 in_uPosition, AkUInt32 in_uDesiredSize )
		{
			if( in_uPosition + in_uDesiredSize <= (AkUInt64)m_pFileDesc->iFileSize )
			{
				return in_uDesiredSize;
			}
			if( in_uPosition >= (AkUInt64)m_pFileDesc->iFileSize )
			{
				return 0;
			}
			return (AkUInt32)( m_pFileDesc->iFileSize - in_uPosition );
		}

		AKRESULT CAkStmTask::SetDeferredFileOpen( AkFileDesc * in_pFileDesc, const AkOSChar * in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode )
		{
			m_bIsFileOpen = false;
			m_pFileDesc = in_pFileDesc;
			m_pDeferredOpenData = AkDeferredOpenData::Create( in_pszFileName, in_pFSFlags, in_eOpenMode );
			return m_pDeferredOpenData ? AK_Success : AK_Fail;
		}

		AKRESULT CAkStmTask::SetDeferredFileOpen( AkFileDesc * in_pFileDesc, AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode )
		{
			m_bIsFileOpen = false;
			m_pFileDesc = in_pFileDesc;
			m_pDeferredOpenData = AkDeferredOpenData::Create( in_fileID, in_pFSFlags, in_eOpenMode );
			return m_pDeferredOpenData ? AK_Success : AK_Fail;
		}

		/** 2012 0x96f500: the IO thread opens a deferred file before its first transfer. */
		AKRESULT CAkStmTask::EnsureFileIsOpen()
		{
			if( !m_pDeferredOpenData || m_bIsToBeDestroyed )
			{
				return AK_Success;
			}
			bool bSyncOpen = true;
			AkFileSystemFlags * pFlags = m_pDeferredOpenData->bUseFlags ? &m_pDeferredOpenData->flags : NULL;
			AKRESULT eResult;
			if( m_pDeferredOpenData->bByString )
			{
				eResult = CAkStreamMgr::m_pFileLocationResolver->Open( m_pDeferredOpenData->pszFileName, m_pDeferredOpenData->eOpenMode, pFlags, bSyncOpen, *m_pFileDesc );
			}
			else
			{
				eResult = CAkStreamMgr::m_pFileLocationResolver->Open( m_pDeferredOpenData->fileID, m_pDeferredOpenData->eOpenMode, pFlags, bSyncOpen, *m_pFileDesc );
			}
			if( eResult == AK_Success )
			{
				OnFileDeferredOpen();
			}
			if( m_pDeferredOpenData )
			{
				m_pDeferredOpenData->Destroy();
				m_pDeferredOpenData = NULL;
			}
			return eResult;
		}

		// -- CAkDeviceBase ---------------------------------------------------------------------------

		CAkDeviceBase::CAkDeviceBase( IAkLowLevelIOHook * in_pLowLevelHook )
			: m_time( 0 )
			, m_pLowLevelHook( in_pLowLevelHook )
			, m_uGranularity( 0 )
			, m_fTargetAutoStmBufferLength( 0.f )
			, m_deviceID( AK_INVALID_DEVICE_ID )
			, m_pStmMemViewMem( NULL )
		{
		}

		CAkDeviceBase::~CAkDeviceBase()
		{
		}

		AKRESULT CAkDeviceBase::Init( const AkDeviceSettings & in_settings, AkDeviceID in_deviceID )
		{
			if( !in_settings.uGranularity || ( in_settings.uIOMemorySize && in_settings.fTargetAutoStmBufferLength < 0.f ) )
			{
				return AK_InvalidParameter;
			}
			if( ( in_settings.uSchedulerTypeFlags & AK_SCHEDULER_DEFERRED_LINED_UP ) && ( !in_settings.uMaxConcurrentIO || in_settings.uMaxConcurrentIO > 1024 ) )
			{
				return AK_InvalidParameter;
			}
			m_uGranularity = in_settings.uGranularity;
			m_fTargetAutoStmBufferLength = in_settings.fTargetAutoStmBufferLength;
			m_deviceID = in_deviceID;
			m_uMaxConcurrentIO = in_settings.uMaxConcurrentIO;
			if( m_mgrMemIO.Init( in_settings ) != AK_Success )
			{
				return AK_Fail;
			}
			return CAkIOThread::Init( in_settings.threadProperties );
		}

		void CAkDeviceBase::OnThreadStart()
		{
			AKPLATFORM::PerformanceCounter( &m_time );
		}

		/** 2012 0x970600: stop the thread, free the view pool, the IO memory, then ourselves. */
		void CAkDeviceBase::Destroy()
		{
			CAkIOThread::Term();
			if( m_pStmMemViewMem )
			{
				if( m_mgrMemIO.UseCache() )
				{
					CAkStmMemView * pView = m_poolStmMemView.m_pFirst;
					CAkStmMemView * pPrev = NULL;
					while( pView )
					{
						CAkStmMemView * pNext = pView->pNextView;
						if( pPrev )
						{
							pPrev->pNextView = pNext;
						}
						else
						{
							m_poolStmMemView.m_pFirst = pNext;
						}
						if( !pView->m_bCachedAlloc )
						{
							AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, pView );
						}
						pView = pNext;
					}
				}
				else
				{
					m_poolStmMemView.m_pFirst = NULL;
				}
				AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, m_pStmMemViewMem );
			}
			m_poolStmMemView.m_pFirst = NULL;
			m_mgrMemIO.Term();
			const AkMemPoolId poolId = CAkStreamMgr::m_streamMgrPoolId;
			this->~CAkDeviceBase();
			AK::MemoryMgr::Free( poolId, this );
		}

		void CAkDeviceBase::AddTask( CAkStmTask * in_pStmTask )
		{
			AkAutoLock<CAkLock> scheduling( m_lockTasksList );
			m_listTasks.AddFirst( in_pStmTask );
		}

		void CAkDeviceBase::DestroyTask( CAkStmTask * in_pTask )
		{
			const AkMemPoolId poolId = CAkStreamMgr::m_streamMgrPoolId;
			in_pTask->~CAkStmTask();
			AK::MemoryMgr::Free( poolId, in_pTask );
		}

		AkUInt32 CAkDeviceBase::DestroyMemView( CAkStmMemView * in_pMemView )
		{
			AkUInt32 uRefCount = 0;
			AkMemBlock * pBlock = in_pMemView->Detach();
			if( pBlock )
			{
				uRefCount = m_mgrMemIO.ReleaseBlock( pBlock );
			}
			m_poolStmMemView.AddFirst( in_pMemView );
			return uRefCount;
		}

		/** 2012 0x970e50: called by the IO thread when it is told to stop; true once every task is gone. */
		bool CAkDeviceBase::ClearStreams()
		{
			CAkStmTask * pPrev = NULL;
			CAkStmTask * pTask = m_listTasks.m_pFirst;
			while( pTask )
			{
				if( pTask->IsToBeDestroyed() )
				{
					if( pTask->CanBeDestroyed() )
					{
						CAkStmTask * pNext = pTask->pNextLightItem;
						if( pTask == m_listTasks.m_pFirst )
						{
							m_listTasks.m_pFirst = pNext;
						}
						else
						{
							pPrev->pNextLightItem = pNext;
						}
						DestroyTask( pTask );
						pTask = pNext;
						continue;
					}
				}
				else
				{
					pTask->Kill();
				}
				pPrev = pTask;
				pTask = pTask->pNextLightItem;
			}
			if( m_listTasks.m_pFirst )
			{
				return false;
			}
			m_listTasks.m_pFirst = NULL;
			return true;
		}

		/** 2012 0x970ee0: when the IO memory is exhausted only standard streams are served. */
		CAkStmTask * CAkDeviceBase::ScheduleStdStmOnly( AkReal32 & out_fOpDeadline )
		{
			CAkStmTask * pPrev = NULL;
			CAkStmTask * pTask = m_listTasks.m_pFirst;
			while( pTask )
			{
				if( pTask->IsToBeDestroyed() )
				{
					if( pTask->CanBeDestroyed() )
					{
						CAkStmTask * pNext = pTask->pNextLightItem;
						if( pTask == m_listTasks.m_pFirst )
						{
							m_listTasks.m_pFirst = pNext;
						}
						else
						{
							pPrev->pNextLightItem = pNext;
						}
						DestroyTask( pTask );
						pTask = pNext;
						continue;
					}
				}
				else if( !pTask->m_bIsAutoStm && pTask->ReadyForIO() )
				{
					break;
				}
				pPrev = pTask;
				pTask = pTask->pNextLightItem;
			}
			if( !pTask )
			{
				return NULL;
			}

			CAkStmTask * pTaskToSchedule = pTask;
			AkReal32 fSmallestDeadline = pTask->EffectiveDeadline();
			pPrev = pTask;
			pTask = pTask->pNextLightItem;
			while( pTask )
			{
				if( pTask->IsToBeDestroyed() )
				{
					if( pTask->CanBeDestroyed() )
					{
						CAkStmTask * pNext = pTask->pNextLightItem;
						if( pTask == m_listTasks.m_pFirst )
						{
							m_listTasks.m_pFirst = pNext;
						}
						else
						{
							pPrev->pNextLightItem = pNext;
						}
						DestroyTask( pTask );
						pTask = pNext;
						continue;
					}
				}
				else if( !pTask->m_bIsAutoStm && pTask->ReadyForIO() )
				{
					const AkReal32 fDeadline = pTask->EffectiveDeadline();
					if( fDeadline == 0.f )
					{
						if( pTask->Priority() > pTaskToSchedule->Priority() || fSmallestDeadline > 0.f
							|| ( pTask->Priority() == pTaskToSchedule->Priority() && pTask->TimeSinceLastTransfer( m_time ) > pTaskToSchedule->TimeSinceLastTransfer( m_time ) ) )
						{
							pTaskToSchedule = pTask;
							fSmallestDeadline = fDeadline;
						}
					}
					else if( fSmallestDeadline > fDeadline )
					{
						pTaskToSchedule = pTask;
						fSmallestDeadline = fDeadline;
					}
				}
				pPrev = pTask;
				pTask = pTask->pNextLightItem;
			}
			out_fOpDeadline = fSmallestDeadline;
			return pTaskToSchedule;
		}

		/**
		 * 2012 0x9714c0: the task with the smallest effective deadline; a deadline of 0 (already late) is
		 * decided by priority then by the time since its last transfer. A task that "requires scheduling"
		 * (an auto stream below its target buffering, or any pending std op) beats one that does not.
		 */
		CAkStmTask * CAkDeviceBase::SchedulerFindNextTask( AkReal32 & out_fOpDeadline )
		{
			AkAutoLock<CAkLock> scheduling( m_lockTasksList );
			AKPLATFORM::PerformanceCounter( &m_time );
			if( m_bDoWaitMemoryChange )
			{
				return ScheduleStdStmOnly( out_fOpDeadline );
			}

			CAkStmTask * pPrev = NULL;
			CAkStmTask * pTask = m_listTasks.m_pFirst;
			while( pTask )
			{
				if( pTask->IsToBeDestroyed() )
				{
					if( pTask->CanBeDestroyed() )
					{
						CAkStmTask * pNext = pTask->pNextLightItem;
						if( pTask == m_listTasks.m_pFirst )
						{
							m_listTasks.m_pFirst = pNext;
						}
						else
						{
							pPrev->pNextLightItem = pNext;
						}
						DestroyTask( pTask );
						pTask = pNext;
						continue;
					}
				}
				else if( pTask->ReadyForIO() )
				{
					break;
				}
				pPrev = pTask;
				pTask = pTask->pNextLightItem;
			}
			if( !pTask )
			{
				return NULL;
			}

			CAkStmTask * pTaskToSchedule = pTask;
			AkReal32 fSmallestDeadline = pTask->EffectiveDeadline();
			bool bLeastBufferedTaskRequiresScheduling = pTask->RequiresScheduling();
			pPrev = pTask;
			pTask = pTask->pNextLightItem;
			while( pTask )
			{
				if( pTask->IsToBeDestroyed() )
				{
					if( pTask->CanBeDestroyed() )
					{
						CAkStmTask * pNext = pTask->pNextLightItem;
						if( pTask == m_listTasks.m_pFirst )
						{
							m_listTasks.m_pFirst = pNext;
						}
						else
						{
							pPrev->pNextLightItem = pNext;
						}
						DestroyTask( pTask );
						pTask = pNext;
						continue;
					}
				}
				else if( pTask->ReadyForIO() )
				{
					const AkReal32 fDeadline = pTask->EffectiveDeadline();
					const bool bRequiresScheduling = pTask->RequiresScheduling();
					if( !bLeastBufferedTaskRequiresScheduling && bRequiresScheduling )
					{
						pTaskToSchedule = pTask;
						bLeastBufferedTaskRequiresScheduling = true;
						fSmallestDeadline = fDeadline;
					}
					else if( bRequiresScheduling || !bLeastBufferedTaskRequiresScheduling )
					{
						if( fDeadline == 0.f )
						{
							if( pTask->Priority() > pTaskToSchedule->Priority() || fSmallestDeadline > 0.f
								|| ( pTask->Priority() == pTaskToSchedule->Priority() && pTask->TimeSinceLastTransfer( m_time ) > pTaskToSchedule->TimeSinceLastTransfer( m_time ) ) )
							{
								pTaskToSchedule = pTask;
								bLeastBufferedTaskRequiresScheduling = bRequiresScheduling;
								fSmallestDeadline = fDeadline;
							}
						}
						else if( fSmallestDeadline > fDeadline )
						{
							pTaskToSchedule = pTask;
							bLeastBufferedTaskRequiresScheduling = bRequiresScheduling;
							fSmallestDeadline = fDeadline;
						}
					}
				}
				pPrev = pTask;
				pTask = pTask->pNextLightItem;
			}
			out_fOpDeadline = fSmallestDeadline;
			return bLeastBufferedTaskRequiresScheduling ? pTaskToSchedule : NULL;
		}

		/** 2012 0x9710c0: out of stream-manager memory; free dead tasks, optionally kill the lowest priority one. */
		void CAkDeviceBase::ForceCleanup( bool in_bKillLowestPriorityTask, AkPriority in_priority )
		{
			AkAutoLock<CAkLock> scheduling( m_lockTasksList );
			CAkStmTask * pTaskToKill = NULL;
			CAkStmTask * pPrev = NULL;
			CAkStmTask * pTask = m_listTasks.m_pFirst;
			while( pTask )
			{
				if( pTask->IsToBeDestroyed() )
				{
					if( pTask->CanBeDestroyed() )
					{
						CAkStmTask * pNext = pTask->pNextLightItem;
						if( pTask == m_listTasks.m_pFirst )
						{
							m_listTasks.m_pFirst = pNext;
						}
						else
						{
							pPrev->pNextLightItem = pNext;
						}
						DestroyTask( pTask );
						pTask = pNext;
						continue;
					}
				}
				else if( in_bKillLowestPriorityTask
					&& ( !pTaskToKill || pTask->Priority() < pTaskToKill->Priority() )
					&& pTask->Priority() < in_priority
					&& pTask->ReadyForIO() )
				{
					pTaskToKill = pTask;
				}
				pPrev = pTask;
				pTask = pTask->pNextLightItem;
			}
			if( pTaskToKill )
			{
				pTaskToKill->Kill();
			}
		}

		// -- CAkStdStmBase ---------------------------------------------------------------------------

		CAkStdStmBase::CAkStdStmBase()
			: m_uTotalScheduledSize( 0 )
			, m_fDeadline( 0.f )
			, m_eStmStatus( AK_StmStatusIdle )
			, m_bIsOpComplete( 1 )
		{
			m_bIsAutoStm = 0;
			m_bIsWriteOp = 0;
		}

		CAkStdStmBase::~CAkStdStmBase()
		{
			if( m_bRequiresScheduling )
			{
				m_pDevice->StdSemDecr();
			}
		}

		/** 2012 0x96fce0: the low-level block size must divide the device granularity. */
		AKRESULT CAkStdStmBase::Init( CAkDeviceBase * in_pDevice, AkFileDesc * in_pFileDesc, AkOpenMode /*in_eOpenMode*/ )
		{
			m_pDevice = in_pDevice;
			if( in_pFileDesc->iFileSize < 0 )
			{
				SetToBeDestroyed();
				return AK_InvalidParameter;
			}
			const AkUInt32 uLLBlockSize = in_pDevice->GetLowLevelHook()->GetBlockSize( *in_pFileDesc );
			if( !uLLBlockSize || uLLBlockSize > in_pDevice->GetGranularity() || in_pDevice->GetGranularity() % uLLBlockSize )
			{
				SetToBeDestroyed();
				return AK_Fail;
			}
			m_uLLBlockSize = uLLBlockSize;
			return AK_Success;
		}

		void CAkStdStmBase::GetInfo( AkStreamInfo & out_info )
		{
			out_info.deviceID = m_pDevice->GetDeviceID();
			out_info.pszName = m_pszStreamName;
			out_info.uSize = m_pFileDesc->iFileSize;
			out_info.bIsOpen = m_bIsFileOpen;
		}

		void * CAkStdStmBase::GetFileDescriptor()
		{
			return m_pFileDesc;
		}

		AKRESULT CAkStdStmBase::SetStreamName( const AkOSChar * in_pszStreamName )
		{
			if( m_pszStreamName )
			{
				AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, m_pszStreamName );
			}
			if( in_pszStreamName )
			{
				const size_t uStrLen = wcslen( in_pszStreamName );
				m_pszStreamName = (AkOSChar *)AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, (AkUInt32)( sizeof( AkOSChar ) * ( uStrLen + 1 ) ) );
				if( !m_pszStreamName )
				{
					return AK_InsufficientMemory;
				}
				AKPLATFORM::SafeStrCpy( m_pszStreamName, in_pszStreamName, uStrLen + 1 );
			}
			return AK_Success;
		}

		AkUInt32 CAkStdStmBase::GetBlockSize()
		{
			return m_uLLBlockSize;
		}

		AKRESULT CAkStdStmBase::Read( void * in_pBuffer, AkUInt32 in_uReqSize, bool in_bWait, AkPriority in_priority, AkReal32 in_fDeadline, AkUInt32 & out_uSize )
		{
			return ExecuteOp( false, in_pBuffer, in_uReqSize, in_bWait, in_priority, in_fDeadline, out_uSize );
		}

		AKRESULT CAkStdStmBase::Write( void * in_pBuffer, AkUInt32 in_uReqSize, bool in_bWait, AkPriority in_priority, AkReal32 in_fDeadline, AkUInt32 & out_uSize )
		{
			return ExecuteOp( true, in_pBuffer, in_uReqSize, in_bWait, in_priority, in_fDeadline, out_uSize );
		}

		AkUInt64 CAkStdStmBase::GetPosition( bool * out_pbEndOfStream )
		{
			if( out_pbEndOfStream )
			{
				*out_pbEndOfStream = m_bHasReachedEof;
			}
			return m_memBlock.uPosition;
		}

		/** 2012 0x96f680: the new position is rounded down to the low-level block size. */
		AKRESULT CAkStdStmBase::SetPosition( AkInt64 in_iMoveOffset, AkMoveMethod in_eMoveMethod, AkInt64 * out_piRealOffset )
		{
			if( out_piRealOffset )
			{
				*out_piRealOffset = 0;
			}
			if( m_eStmStatus == AK_StmStatusPending )
			{
				return AK_Fail;
			}
			AkInt64 iPosition;
			if( in_eMoveMethod == AK_MoveBegin )
			{
				iPosition = in_iMoveOffset;
			}
			else if( in_eMoveMethod == AK_MoveCurrent )
			{
				iPosition = in_iMoveOffset + (AkInt64)m_memBlock.uPosition;
			}
			else if( in_eMoveMethod == AK_MoveEnd )
			{
				iPosition = in_iMoveOffset + m_pFileDesc->iFileSize;
			}
			else
			{
				return AK_InvalidParameter;
			}
			if( iPosition < 0 )
			{
				return AK_InvalidParameter;
			}
			const AkInt64 iRemainder = iPosition % m_uLLBlockSize;
			if( iRemainder )
			{
				iPosition -= iRemainder;
			}
			if( out_piRealOffset )
			{
				if( in_eMoveMethod == AK_MoveBegin )
				{
					*out_piRealOffset = iPosition;
				}
				else if( in_eMoveMethod == AK_MoveCurrent )
				{
					*out_piRealOffset = iPosition - (AkInt64)m_memBlock.uPosition;
				}
				else
				{
					*out_piRealOffset = iPosition - m_pFileDesc->iFileSize;
				}
			}
			m_memBlock.uPosition = (AkUInt64)iPosition;
			return AK_Success;
		}

		void * CAkStdStmBase::GetData( AkUInt32 & out_uSize )
		{
			out_uSize = m_memBlock.uAvailableSize;
			return m_memBlock.pData;
		}

		AkStmStatus CAkStdStmBase::GetStatus()
		{
			return (AkStmStatus)m_eStmStatus;
		}

		/** 2012 0x96f7c0: the user deadline spread over the remaining granularity-sized transfers, minus time spent. */
		AkReal32 CAkStdStmBase::EffectiveDeadline()
		{
			const AkUInt32 uGranularity = m_pDevice->GetGranularity();
			const AkUInt32 uNumTransfersRemaining = ( m_memBlock.uAvailableSize - m_uTotalScheduledSize + uGranularity - 1 ) / uGranularity;
			const AkReal64 fDeadline = m_fDeadline / (AkReal64)uNumTransfersRemaining - (AkReal64)( m_pDevice->GetTime() - m_iIOStartTime ) / AK::g_fFreqRatio;
			return fDeadline > 0.0 ? (AkReal32)fDeadline : 0.f;
		}

		/** 2012 0x96f840: status change drives the device's std semaphore through m_bRequiresScheduling. */
		void CAkStdStmBase::SetStatus( AkStmStatus in_eStatus )
		{
			m_eStmStatus = in_eStatus;
			if( ( m_bIsToBeDestroyed && CanBeDestroyed() ) || in_eStatus == AK_StmStatusPending )
			{
				if( in_eStatus == AK_StmStatusPending && !( m_bIsToBeDestroyed && CanBeDestroyed() ) )
				{
					m_bIsReadyForIO = true;
				}
				if( !m_bRequiresScheduling )
				{
					m_bRequiresScheduling = true;
					m_pDevice->StdSemIncr();
				}
				return;
			}
			m_bIsReadyForIO = false;
			if( m_bRequiresScheduling )
			{
				m_bRequiresScheduling = false;
				m_pDevice->StdSemDecr();
			}
		}

		/**
		 * 2012 0x96fd60. The op is scheduled as Pending and the IO thread executes it in granularity-sized
		 * transfers; a blocking caller waits on its own event until the op is complete.
		 */
		AKRESULT CAkStdStmBase::ExecuteOp( bool in_bWrite, void * in_pBuffer, AkUInt32 in_uReqSize, bool in_bWait, AkPriority in_priority, AkReal32 in_fDeadline, AkUInt32 & out_uSize )
		{
			out_uSize = 0;
			m_bIsOpComplete = false;
			m_bIsWriteOp = in_bWrite;
			m_uTotalScheduledSize = 0;
			m_priority = in_priority;
			m_fDeadline = in_fDeadline;
			m_memBlock.pData = in_pBuffer;
			m_memBlock.uAvailableSize = in_uReqSize;
			if( !in_pBuffer || (AkUInt8)in_priority > AK_MAX_PRIORITY || in_fDeadline < 0.f )
			{
				return AK_InvalidParameter;
			}
			if( m_eStmStatus == AK_StmStatusPending || m_eStmStatus == AK_StmStatusError )
			{
				return AK_Fail;
			}
			AkUInt32 uReqSize = in_uReqSize;
			if( !in_bWrite )
			{
				if( in_uReqSize % m_uLLBlockSize )
				{
					return AK_Fail;
				}
				if( m_bIsFileOpen )
				{
					uReqSize = ClampRequestSizeToEof( m_memBlock.uPosition, in_uReqSize );
				}
			}
			if( !uReqSize )
			{
				out_uSize = 0;
				return AK_Success;
			}
			AKPLATFORM::PerformanceCounter( &m_iIOStartTime );
			m_lockStatus.Lock();
			if( in_bWait )
			{
				SetBlockedStatus();
				SetStatus( AK_StmStatusPending );
				m_lockStatus.Unlock();
				m_pDevice->WaitForIOCompletion( this );
				out_uSize = uReqSize;
				return m_eStmStatus == AK_StmStatusCompleted ? AK_Success : AK_Fail;
			}
			SetStatus( AK_StmStatusPending );
			m_lockStatus.Unlock();
			out_uSize = uReqSize;
			return AK_Success;
		}

		/** 2012 0x96fec0: account a finished transfer; the op completes when it reached its size or the file end. */
		void CAkStdStmBase::AddMemView( CAkStmMemView * in_pMemView, bool in_bStoreData )
		{
			if( in_bStoreData && !m_bIsToBeDestroyed && m_eStmStatus != AK_StmStatusError )
			{
				m_bHasReachedEof = false;
				m_bIsOpComplete = true;
				AkUInt32 uTransferSize = in_pMemView->m_pBlock->uAvailableSize - in_pMemView->m_uOffsetInBlock;
				if( uTransferSize > m_pDevice->GetGranularity() )
				{
					uTransferSize = m_pDevice->GetGranularity();
					m_bIsOpComplete = false;
				}
				if( !m_bIsWriteOp && (AkInt64)uTransferSize >= m_pFileDesc->iFileSize - (AkInt64)in_pMemView->m_uOffsetInBlock - (AkInt64)in_pMemView->m_pBlock->uPosition )
				{
					m_bIsOpComplete = true;
					m_bHasReachedEof = true;
					m_memBlock.uAvailableSize += (AkUInt32)( m_pFileDesc->iFileSize - in_pMemView->m_pBlock->uAvailableSize - in_pMemView->m_pBlock->uPosition );
				}
			}
			AkAutoLock<CAkLock> gate( m_pDevice->LockMem() );
			AkMemBlock * pBlock = in_pMemView->Detach();
			if( pBlock )
			{
				m_pDevice->GetIOMemMgr().DestroyTempBlock( &m_memBlock, pBlock );
			}
			m_pDevice->m_poolStmMemView.AddFirst( in_pMemView );
		}

		/** 2012 0x96ffd0. */
		void CAkStdStmBase::UpdateTaskStatus( AKRESULT in_eIOResult )
		{
			bool bChanged = true;
			if( in_eIOResult == AK_Fail )
			{
				m_eStmStatus = AK_StmStatusError;
			}
			else if( m_bIsOpComplete )
			{
				m_memBlock.uPosition += m_memBlock.uAvailableSize;
				m_eStmStatus = AK_StmStatusCompleted;
			}
			else
			{
				bChanged = false;
			}
			if( bChanged )
			{
				if( m_bIsToBeDestroyed && CanBeDestroyed() )
				{
					if( !m_bRequiresScheduling )
					{
						m_bRequiresScheduling = true;
						m_pDevice->StdSemIncr();
					}
				}
				else
				{
					m_bIsReadyForIO = false;
					if( m_bRequiresScheduling )
					{
						m_bRequiresScheduling = false;
						m_pDevice->StdSemDecr();
					}
				}
			}
			if( m_bIsBlocked && m_eStmStatus != AK_StmStatusPending && m_eStmStatus != AK_StmStatusIdle )
			{
				m_pDevice->SignalIOCompleted( this );
			}
		}

		/** 2012 0x970070. */
		void CAkStdStmBase::Kill()
		{
			AkAutoLock<CAkLock> status( m_lockStatus );
			m_eStmStatus = AK_StmStatusError;
			if( m_bIsToBeDestroyed && CanBeDestroyed() )
			{
				if( !m_bRequiresScheduling )
				{
					m_bRequiresScheduling = true;
					m_pDevice->StdSemIncr();
				}
			}
			else
			{
				m_bIsReadyForIO = false;
				if( m_bRequiresScheduling )
				{
					m_bRequiresScheduling = false;
					m_pDevice->StdSemDecr();
				}
			}
			if( m_bIsBlocked && m_eStmStatus != AK_StmStatusPending && m_eStmStatus != AK_StmStatusIdle )
			{
				m_pDevice->SignalIOCompleted( this );
			}
		}

		// -- CAkAutoStmBase --------------------------------------------------------------------------

		CAkAutoStmBase::CAkAutoStmBase()
			: m_uNextExpectedUserPosition( 0 )
			, m_fileID( AK_INVALID_FILE_ID )
			, m_fThroughput( 0.f )
			, m_uLoopStart( 0 )
			, m_uLoopEnd( 0 )
			, m_uBufferSize( 0 )
			, m_uVirtualBufferingSize( 0 )
			, m_uMinBufferSize( 0 )
			, m_uBufferAlignment( 0 )
			, m_uNextToGrant( 0 )
			, m_uMinNumBuffers( 1 )
			, m_bIsRunning( 0 )
			, m_bIOError( 0 )
		{
			m_bIsAutoStm = 1;
			m_bIsWriteOp = 0;
		}

		CAkAutoStmBase::~CAkAutoStmBase()
		{
			if( m_bRequiresScheduling )
			{
				m_pDevice->AutoSemDecr();
			}
		}

		/** 2012 0x9701c0. */
		AKRESULT CAkAutoStmBase::Init( CAkDeviceBase * in_pDevice, AkFileDesc * in_pFileDesc, AkFileID in_fileID, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, AkUInt32 in_uGranularity )
		{
			m_pDevice = in_pDevice;
			if( in_pFileDesc->iFileSize < 0 )
			{
				SetToBeDestroyed();
				return AK_InvalidParameter;
			}
			m_fileID = in_fileID;
			const AkUInt32 uLLBlockSize = in_pDevice->GetLowLevelHook()->GetBlockSize( *in_pFileDesc );
			if( !uLLBlockSize || uLLBlockSize > in_uGranularity || in_uGranularity % uLLBlockSize )
			{
				SetToBeDestroyed();
				return AK_Fail;
			}
			m_uLLBlockSize = uLLBlockSize;
			m_uBufferAlignment = uLLBlockSize;
			m_fThroughput = in_heuristics.fThroughput > 1.f ? in_heuristics.fThroughput : 1.f;
			m_uLoopStart = in_heuristics.uLoopStart - in_heuristics.uLoopStart % uLLBlockSize;
			m_uLoopEnd = ( (AkInt64)in_heuristics.uLoopEnd > in_pFileDesc->iFileSize ) ? (AkUInt32)in_pFileDesc->iFileSize : in_heuristics.uLoopEnd;
			m_uMinNumBuffers = in_heuristics.uMinNumBuffers ? in_heuristics.uMinNumBuffers : 1;
			m_priority = in_heuristics.priority;
			m_uNextExpectedUserPosition = 0;
			if( !in_pFileDesc->iFileSize )
			{
				m_bIsReadyForIO = false;
				m_bHasReachedEof = true;
			}
			const AKRESULT eResult = SetBufferingSettings( in_pBufferSettings, in_uGranularity );
			if( eResult != AK_Success )
			{
				SetToBeDestroyed();
			}
			return eResult;
		}

		static AkUInt32 Gcd( AkUInt32 a, AkUInt32 b )
		{
			if( a < b )
			{
				const AkUInt32 t = a;
				a = b;
				b = t;
			}
			AkUInt32 r = a % b;
			AkUInt32 result = b;
			if( r )
			{
				int shift = 0;
				while( !( ( result | r ) & 1 ) )
				{
					result >>= 1;
					r >>= 1;
					++shift;
				}
				while( !( result & 1 ) )
				{
					result >>= 1;
				}
				while( !( r & 1 ) )
				{
					r >>= 1;
				}
				while( result != r )
				{
					if( result < r )
					{
						const AkUInt32 t = result;
						result = r;
						r = t;
					}
					for( result = ( result - r ) >> 1; !( result & 1 ); result >>= 1 )
					{
					}
				}
				result <<= shift;
			}
			return result;
		}

		/** 2012 0x96f930. */
		AKRESULT CAkAutoStmBase::SetBufferingSettings( const AkAutoStmBufSettings * in_pBufferSettings, AkUInt32 in_uGranularity )
		{
			if( !in_pBufferSettings )
			{
				m_uMinBufferSize = m_uLLBlockSize;
				m_uBufferSize = in_uGranularity - in_uGranularity % m_uLLBlockSize;
				return AK_Success;
			}
			if( in_pBufferSettings->uBlockSize )
			{
				m_uBufferAlignment = m_uLLBlockSize * ( in_pBufferSettings->uBlockSize / Gcd( m_uLLBlockSize, in_pBufferSettings->uBlockSize ) );
			}
			m_uMinBufferSize = m_uBufferAlignment;
			m_uBufferSize = in_uGranularity - in_uGranularity % m_uBufferAlignment;
			if( in_pBufferSettings->uBufferSize )
			{
				if( in_pBufferSettings->uBufferSize > in_uGranularity || in_pBufferSettings->uBufferSize % m_uBufferAlignment )
				{
					return AK_Fail;
				}
				m_uBufferSize = in_pBufferSettings->uBufferSize;
				m_uMinBufferSize = in_pBufferSettings->uBufferSize;
				return AK_Success;
			}
			if( in_pBufferSettings->uMinBufferSize && m_uBufferAlignment < in_pBufferSettings->uMinBufferSize )
			{
				m_uMinBufferSize = m_uBufferAlignment * ( ( in_pBufferSettings->uMinBufferSize + m_uBufferAlignment - 1 ) / m_uBufferAlignment );
				if( m_uMinBufferSize > m_uBufferSize )
				{
					return AK_Fail;
				}
			}
			return AK_Success;
		}

		AkUInt32 CAkAutoStmBase::GetEffectiveViewSize( CAkStmMemView * in_pStmMemView ) const
		{
			const AkUInt64 uStart = in_pStmMemView->m_pBlock->uPosition + in_pStmMemView->m_uOffsetInBlock;
			const AkUInt64 uEnd = in_pStmMemView->m_pBlock->uPosition + in_pStmMemView->m_pBlock->uAvailableSize;
			if( uStart >= m_uLoopEnd || uEnd <= m_uLoopEnd )
			{
				return in_pStmMemView->m_pBlock->uAvailableSize - in_pStmMemView->m_uOffsetInBlock;
			}
			return m_uLoopEnd - (AkUInt32)in_pStmMemView->m_pBlock->uPosition - in_pStmMemView->m_uOffsetInBlock;
		}

		void CAkAutoStmBase::Destroy()
		{
			AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
			SetToBeDestroyed();
			CAkStmMemView * pView = m_listBuffers.m_pFirst;
			while( m_uNextToGrant )
			{
				m_uVirtualBufferingSize += GetEffectiveViewSize( pView );
				--m_uNextToGrant;
				pView = pView->pNextView;
			}
			Flush();
			m_listBuffers.m_pFirst = NULL;
			m_listBuffers.m_pLast = NULL;
			m_listBuffers.m_ulNumListItems = 0;
		}

		void CAkAutoStmBase::GetInfo( AkStreamInfo & out_info )
		{
			out_info.deviceID = m_pDevice->GetDeviceID();
			out_info.pszName = m_pszStreamName;
			out_info.uSize = m_pFileDesc->iFileSize;
			out_info.bIsOpen = m_bIsFileOpen;
		}

		void * CAkAutoStmBase::GetFileDescriptor()
		{
			return m_pFileDesc;
		}

		void CAkAutoStmBase::GetHeuristics( AkAutoStmHeuristics & out_heuristics )
		{
			out_heuristics.fThroughput = m_fThroughput;
			out_heuristics.uLoopStart = m_uLoopStart;
			out_heuristics.uLoopEnd = m_uLoopEnd;
			out_heuristics.uMinNumBuffers = m_uMinNumBuffers;
			out_heuristics.priority = m_priority;
		}

		/** 2012 0x970850: a new loop region flushes every buffer past the smaller of the old and new loop ends. */
		AKRESULT CAkAutoStmBase::SetHeuristics( const AkAutoStmHeuristics & in_heuristics )
		{
			if( (AkUInt8)in_heuristics.priority > AK_MAX_PRIORITY )
			{
				return AK_InvalidParameter;
			}
			m_priority = in_heuristics.priority;
			const AkReal32 fThroughput = in_heuristics.fThroughput > 1.f ? in_heuristics.fThroughput : 1.f;
			const AkUInt32 uLoopEnd = ( (AkInt64)in_heuristics.uLoopEnd > m_pFileDesc->iFileSize && m_bIsFileOpen ) ? (AkUInt32)m_pFileDesc->iFileSize : in_heuristics.uLoopEnd;

			if( m_uLoopEnd == uLoopEnd && m_uLoopStart == in_heuristics.uLoopStart )
			{
				const AkUInt8 uMinNumBuffers = in_heuristics.uMinNumBuffers ? in_heuristics.uMinNumBuffers : 1;
				if( m_fThroughput != fThroughput || m_uMinNumBuffers != uMinNumBuffers )
				{
					AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
					m_uMinNumBuffers = uMinNumBuffers;
					m_fThroughput = fThroughput;
					UpdateSchedulingStatus();
				}
				return AK_Success;
			}

			AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
			m_fThroughput = fThroughput;
			m_uMinNumBuffers = in_heuristics.uMinNumBuffers ? in_heuristics.uMinNumBuffers : 1;

			AkUInt32 uMaxPosition;
			if( !uLoopEnd )
			{
				uMaxPosition = m_uLoopEnd;
			}
			else if( !m_uLoopEnd )
			{
				uMaxPosition = uLoopEnd;
			}
			else
			{
				uMaxPosition = m_uLoopEnd < uLoopEnd ? m_uLoopEnd : uLoopEnd;
			}

			CAkStmMemView * pPrev = NULL;
			CAkStmMemView * pView = m_listBuffers.m_pFirst;
			for( AkUInt32 uSkip = m_uNextToGrant; uSkip; --uSkip )
			{
				pPrev = pView;
				pView = pView->pNextView;
			}
			bool bReachedLastBufferToKeep = false;
			while( pView )
			{
				if( pView->StartPosition() >= uMaxPosition )
				{
					break;
				}
				pPrev = pView;
				const AkUInt64 uEnd = pView->EndPosition();
				pView = pView->pNextView;
				if( uEnd >= uMaxPosition )
				{
					bReachedLastBufferToKeep = true;
					break;
				}
			}

			{
				AkAutoLock<CAkLock> gate( m_pDevice->LockMem() );
				while( pView )
				{
					m_uVirtualBufferingSize -= GetEffectiveViewSize( pView );
					CAkStmMemView * pNext = pView->pNextView;
					if( pView == m_listBuffers.m_pFirst )
					{
						m_listBuffers.m_pFirst = pNext;
					}
					else
					{
						pPrev->pNextView = pNext;
					}
					if( pView == m_listBuffers.m_pLast )
					{
						m_listBuffers.m_pLast = pPrev;
					}
					--m_listBuffers.m_ulNumListItems;
					m_pDevice->DestroyMemView( pView );
					pView = pNext;
				}
				CancelPendingTransfers( bReachedLastBufferToKeep ? 0 : uMaxPosition );
				m_pDevice->NotifyMemChange();
			}

			m_uLoopStart = in_heuristics.uLoopStart - in_heuristics.uLoopStart % m_uLLBlockSize;
			m_uLoopEnd = uLoopEnd;
			RecomputeVirtualBuffering();
			UpdateSchedulingStatus();
			return AK_Success;
		}

		/** 2012 0x971260. */
		AKRESULT CAkAutoStmBase::SetMinimalBufferSize( AkUInt32 in_uMinBufferSize )
		{
			const AkUInt32 uOldMinBufferSize = m_uMinBufferSize;
			AkAutoStmBufSettings bufferSettings;
			bufferSettings.uBufferSize = 0;
			bufferSettings.uMinBufferSize = in_uMinBufferSize;
			bufferSettings.uBlockSize = 0;
			AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
			const AKRESULT eResult = SetBufferingSettings( &bufferSettings, m_pDevice->GetGranularity() );
			if( eResult == AK_Success )
			{
				if( m_uMinBufferSize > uOldMinBufferSize )
				{
					FlushSmallBuffersAndPendingTransfers( in_uMinBufferSize );
				}
				return AK_Success;
			}
			m_bIOError = true;
			Stop();
			UpdateSchedulingStatus();
			if( m_bIsBlocked )
			{
				m_pDevice->SignalIOCompleted( this );
			}
			return eResult;
		}

		AKRESULT CAkAutoStmBase::SetStreamName( const AkOSChar * in_pszStreamName )
		{
			if( m_pszStreamName )
			{
				AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, m_pszStreamName );
			}
			if( in_pszStreamName )
			{
				const size_t uStrLen = wcslen( in_pszStreamName );
				m_pszStreamName = (AkOSChar *)AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, (AkUInt32)( sizeof( AkOSChar ) * ( uStrLen + 1 ) ) );
				if( !m_pszStreamName )
				{
					return AK_InsufficientMemory;
				}
				AKPLATFORM::SafeStrCpy( m_pszStreamName, in_pszStreamName, uStrLen + 1 );
			}
			return AK_Success;
		}

		AkUInt32 CAkAutoStmBase::GetBlockSize()
		{
			return m_uLLBlockSize;
		}

		/** 2012 0x970370. */
		AKRESULT CAkAutoStmBase::QueryBufferingStatus( AkUInt32 & out_uNumBytesAvailable )
		{
			if( m_bIOError )
			{
				return AK_Fail;
			}
			AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
			out_uNumBytesAvailable = 0;
			AKRESULT eRetCode;
			if( m_uNextToGrant >= m_listBuffers.m_ulNumListItems )
			{
				eRetCode = AK_NoDataReady;
			}
			else
			{
				eRetCode = AK_DataReady;
				CAkStmMemView * pView = m_listBuffers.m_pFirst;
				for( AkUInt32 uSkip = m_uNextToGrant; uSkip; --uSkip )
				{
					pView = pView->pNextView;
				}
				for( ; pView; pView = pView->pNextView )
				{
					out_uNumBytesAvailable += pView->m_pBlock->uAvailableSize - pView->m_uOffsetInBlock;
				}
			}
			if( ( !m_bRequiresScheduling && m_uVirtualBufferingSize <= out_uNumBytesAvailable ) || m_pDevice->m_bDoWaitMemoryChange )
			{
				eRetCode = AK_NoMoreData;
			}
			return eRetCode;
		}

		AkUInt32 CAkAutoStmBase::GetNominalBuffering()
		{
			return (AkUInt32)(AkInt64)( m_pDevice->GetTargetAutoStmBufferLength() * m_fThroughput );
		}

		/** 2012 0x970b30. */
		AKRESULT CAkAutoStmBase::Start()
		{
			if( !m_bIsRunning )
			{
				{
					AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
					m_bIsRunning = true;
					m_bIsReadyForIO = !m_bHasReachedEof && !m_bIsToBeDestroyed;
					UpdateSchedulingStatus();
				}
				{
					AkAutoLock<CAkLock> gate( m_pDevice->LockMem() );
					m_pDevice->NotifyMemChange();
				}
				AKPLATFORM::PerformanceCounter( &m_iIOStartTime );
			}
			return m_bIOError ? AK_Fail : AK_Success;
		}

		/** 2012 0x971320. */
		AKRESULT CAkAutoStmBase::Stop()
		{
			AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
			m_bIsRunning = false;
			m_bIsReadyForIO = false;
			Flush();
			return AK_Success;
		}

		/** 2012 0x9702f0: the first ungranted buffer's start, or the next expected position. */
		AkUInt64 CAkAutoStmBase::GetPosition( bool * out_pbEndOfStream )
		{
			AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
			const AkUInt64 uCurPosition = m_listBuffers.m_pFirst ? m_listBuffers.m_pFirst->StartPosition() : m_uNextExpectedUserPosition;
			if( out_pbEndOfStream )
			{
				*out_pbEndOfStream = uCurPosition >= (AkUInt64)m_pFileDesc->iFileSize;
			}
			return uCurPosition;
		}

		/** 2012 0x971730. */
		AKRESULT CAkAutoStmBase::SetPosition( AkInt64 in_iMoveOffset, AkMoveMethod in_eMoveMethod, AkInt64 * out_piRealOffset )
		{
			if( out_piRealOffset )
			{
				*out_piRealOffset = 0;
			}
			AkInt64 iPosition;
			if( in_eMoveMethod == AK_MoveBegin )
			{
				iPosition = in_iMoveOffset;
			}
			else if( in_eMoveMethod == AK_MoveCurrent )
			{
				iPosition = in_iMoveOffset + (AkInt64)GetPosition( NULL );
			}
			else if( in_eMoveMethod == AK_MoveEnd )
			{
				iPosition = in_iMoveOffset + m_pFileDesc->iFileSize;
			}
			else
			{
				return AK_InvalidParameter;
			}
			if( iPosition < 0 )
			{
				return AK_InvalidParameter;
			}
			const AkInt64 iRemainder = iPosition % m_uLLBlockSize;
			if( iRemainder )
			{
				iPosition -= iRemainder;
			}
			if( out_piRealOffset )
			{
				if( in_eMoveMethod == AK_MoveBegin )
				{
					*out_piRealOffset = iPosition;
				}
				else if( in_eMoveMethod == AK_MoveCurrent )
				{
					*out_piRealOffset = iPosition - (AkInt64)GetPosition( NULL );
				}
				else
				{
					*out_piRealOffset = iPosition - m_pFileDesc->iFileSize;
				}
			}
			ForceFilePosition( (AkUInt64)iPosition );
			return AK_Success;
		}

		/** 2012 0x971850. */
		AKRESULT CAkAutoStmBase::GetBuffer( void *& out_pBuffer, AkUInt32 & out_uSize, bool in_bWait )
		{
			out_pBuffer = NULL;
			out_uSize = 0;
			{
				AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
				out_pBuffer = GetReadBuffer( out_uSize );
			}
			if( in_bWait && !out_pBuffer && !m_bIOError )
			{
				if( !m_bIsRunning )
				{
					return AK_Fail;
				}
				AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
				out_pBuffer = GetReadBuffer( out_uSize );
				while( !out_pBuffer )
				{
					if( m_bIOError || ( !m_bRequiresScheduling && !m_uVirtualBufferingSize ) )
					{
						break;
					}
					SetBlockedStatus();
					m_lockStatus.Unlock();
					m_pDevice->WaitForIOCompletion( this );
					m_lockStatus.Lock();
					out_pBuffer = GetReadBuffer( out_uSize );
				}
			}
			if( m_bIOError )
			{
				return AK_Fail;
			}
			if( !out_pBuffer )
			{
				return ( m_bHasReachedEof && m_uNextExpectedUserPosition >= (AkUInt64)m_pFileDesc->iFileSize ) ? AK_NoMoreData : AK_NoDataReady;
			}
			return ( m_bHasReachedEof && m_uNextExpectedUserPosition >= (AkUInt64)m_pFileDesc->iFileSize ) ? AK_NoMoreData : AK_DataReady;
		}

		/** 2012 0x970bb0. */
		AKRESULT CAkAutoStmBase::ReleaseBuffer()
		{
			AkAutoLock<CAkLock> stmBufferGate( m_lockStatus );
			if( !m_uNextToGrant )
			{
				return AK_Fail;
			}
			CAkStmMemView * pFirst = m_listBuffers.m_pFirst;
			{
				AkAutoLock<CAkLock> gate( m_pDevice->LockMem() );
				m_listBuffers.RemoveFirst();
				m_pDevice->DestroyMemView( pFirst );
				m_pDevice->NotifyMemChange();
			}
			--m_uNextToGrant;
			UpdateSchedulingStatus();
			return AK_Success;
		}

		void CAkAutoStmBase::Kill()
		{
			AkAutoLock<CAkLock> status( m_lockStatus );
			m_bIOError = true;
			Stop();
			UpdateSchedulingStatus();
			if( m_bIsBlocked )
			{
				m_pDevice->SignalIOCompleted( this );
			}
		}

		/** 2012 0x96fa70: once the deferred open succeeded, the loop end is clamped to the real file size. */
		void CAkAutoStmBase::OnFileDeferredOpen()
		{
			m_bIsFileOpen = true;
			AkAutoStmHeuristics heuristics;
			GetHeuristics( heuristics );
			if( (AkInt64)heuristics.uLoopEnd > m_pFileDesc->iFileSize )
			{
				heuristics.uLoopEnd = (AkUInt32)m_pFileDesc->iFileSize;
				SetHeuristics( heuristics );
			}
		}

		AkReal32 CAkAutoStmBase::EffectiveDeadline()
		{
			return (AkReal32)( (AkReal64)m_uVirtualBufferingSize / m_fThroughput );
		}

		/** 2012 0x970400: a ready buffer is appended; a cancelled or failed one goes back to the device. */
		void CAkAutoStmBase::AddMemView( CAkStmMemView * in_pMemView, bool in_bStoreData )
		{
			if( !in_bStoreData || m_bIsToBeDestroyed || m_bIOError )
			{
				m_uVirtualBufferingSize -= GetEffectiveViewSize( in_pMemView );
				AkAutoLock<CAkLock> gate( m_pDevice->LockMem() );
				m_pDevice->DestroyMemView( in_pMemView );
				m_pDevice->NotifyMemChange();
				return;
			}
			in_pMemView->TagAsReady();
			m_listBuffers.AddLast( in_pMemView );
		}

		/** 2012 0x9704c0. */
		void CAkAutoStmBase::UpdateSchedulingStatus()
		{
			if( !m_uLoopEnd && GetVirtualFilePosition() >= (AkUInt64)m_pFileDesc->iFileSize && m_bIsFileOpen )
			{
				m_bIsReadyForIO = false;
				m_bHasReachedEof = true;
			}
			else
			{
				m_bHasReachedEof = false;
				m_bIsReadyForIO = m_bIsRunning && !m_bIsToBeDestroyed;
			}
			if( ( m_bIsReadyForIO && m_pDevice->GetTargetAutoStmBufferLength() * m_fThroughput > (AkReal64)m_uVirtualBufferingSize )
				|| ( m_bIsToBeDestroyed && CanBeDestroyed() ) )
			{
				if( !m_bRequiresScheduling )
				{
					m_bRequiresScheduling = true;
					m_pDevice->AutoSemIncr();
				}
			}
			else if( m_bRequiresScheduling )
			{
				m_bRequiresScheduling = false;
				m_pDevice->AutoSemDecr();
			}
		}

		void CAkAutoStmBase::UpdateTaskStatus( AKRESULT in_eIOResult )
		{
			if( in_eIOResult == AK_Fail )
			{
				m_bIOError = true;
				Stop();
			}
			UpdateSchedulingStatus();
			if( m_bIsBlocked )
			{
				m_pDevice->SignalIOCompleted( this );
			}
		}

		/** 2012 0x970cf0: cancel what is in flight and drop every ungranted buffer. */
		void CAkAutoStmBase::Flush()
		{
			CancelPendingTransfers( 0 );
			if( m_listBuffers.m_ulNumListItems > m_uNextToGrant )
			{
				CAkStmMemView * pPrev = NULL;
				CAkStmMemView * pView = m_listBuffers.m_pFirst;
				for( AkUInt32 uSkip = m_uNextToGrant; uSkip; --uSkip )
				{
					pPrev = pView;
					pView = pView->pNextView;
				}
				AkAutoLock<CAkLock> gate( m_pDevice->LockMem() );
				while( pView )
				{
					m_uVirtualBufferingSize -= GetEffectiveViewSize( pView );
					CAkStmMemView * pNext = pView->pNextView;
					if( pView == m_listBuffers.m_pFirst )
					{
						m_listBuffers.m_pFirst = pNext;
					}
					else
					{
						pPrev->pNextView = pNext;
					}
					if( pView == m_listBuffers.m_pLast )
					{
						m_listBuffers.m_pLast = pPrev;
					}
					--m_listBuffers.m_ulNumListItems;
					m_pDevice->DestroyMemView( pView );
					pView = pNext;
				}
				m_pDevice->NotifyMemChange();
			}
			UpdateSchedulingStatus();
		}

		/** 2012 0x971350. */
		void CAkAutoStmBase::ForceFilePosition( AkUInt64 in_uNewPosition )
		{
			AkAutoLock<CAkLock> status( m_lockStatus );
			m_uNextExpectedUserPosition = in_uNewPosition;
			if( m_uNextToGrant >= m_listBuffers.m_ulNumListItems )
			{
				CancelPendingTransfers( 0 );
			}
			else
			{
				CAkStmMemView * pView = m_listBuffers.m_pFirst;
				for( AkUInt32 uSkip = m_uNextToGrant; uSkip; --uSkip )
				{
					pView = pView->pNextView;
				}
				if( pView->StartPosition() != in_uNewPosition )
				{
					Flush();
					RecomputeVirtualBuffering();
					return;
				}
			}
			UpdateSchedulingStatus();
		}

		/** 2012 0x9713f0: grant the next buffer if it continues the user position, else flush (a seek happened). */
		void * CAkAutoStmBase::GetReadBuffer( AkUInt32 & out_uSize )
		{
			if( m_uNextToGrant >= m_listBuffers.m_ulNumListItems )
			{
				out_uSize = 0;
				return NULL;
			}
			CAkStmMemView * pView = m_listBuffers.m_pFirst;
			for( AkUInt32 uSkip = m_uNextToGrant; uSkip; --uSkip )
			{
				pView = pView->pNextView;
			}
			if( pView->StartPosition() == m_uNextExpectedUserPosition )
			{
				++m_uNextToGrant;
				m_uNextExpectedUserPosition = pView->EndPosition();
				out_uSize = pView->Size();
				m_uVirtualBufferingSize -= GetEffectiveViewSize( pView );
				UpdateSchedulingStatus();
				return pView->Buffer();
			}
			m_uLoopEnd = 0;
			RecomputeVirtualBuffering();
			Flush();
			out_uSize = 0;
			return NULL;
		}
	}
}
