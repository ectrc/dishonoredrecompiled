/*=============================================================================
	AkDeviceBlocking.cpp - CAkDeviceBlocking, CAkStdStmBlocking, CAkAutoStmBlocking.

	DISHONORED(port): agent IH1 (wave 26); wwise\sdk\source\streammanager\common\akdeviceblocking.cpp
	(22 functions, 2012 0x973200..0x974000):
	  CAkDeviceBlocking: ctor 0x973200, ExecuteTask 0x973220, Init 0x973410, CreateStd 0x9734b0 (0x88 bytes),
	    CreateAuto 0x973580 (0x90 bytes), PerformIO 0x973680, CreateMemViewStd 0x9736b0, CreateMemViewAuto 0x973760
	  CAkStdStmBlocking: Cancel 0x973310, Destroy 0x973910, CanBeDestroyed 0x973970, PrepareTransfer 0x9739b0,
	    Update 0x973a80
	  CAkAutoStmBlocking: CancelCurrentTransfer 0x973370, CanBeDestroyed 0x973b20, PrepareTransfer 0x973b60,
	    Update 0x973c80, GetVirtualFilePosition 0x973d10, CancelPendingTransfers 0x973d50,
	    ComputeVirtualBuffering 0x973d80, FlushSmallBuffersAndPendingTransfers 0x973e40,
	    RecomputeVirtualBuffering 0x974000
	One transfer at a time, executed synchronously by the IO thread through IAkIOHookBlocking::Read/Write.
	The file position handed to the hook is the stream position plus uSector * low-level block size, so a
	file inside an AKPK package (uSector = its start block) is read in place.
=============================================================================*/
#include "AkStreamMgrPrivate.h"

namespace AK
{
	namespace StreamMgr
	{
		CAkDeviceBlocking::CAkDeviceBlocking( IAkLowLevelIOHook * in_pLowLevelHook )
			: CAkDeviceBase( in_pLowLevelHook )
		{
			m_currentTransfer.info.uFilePosition = 0;
			m_currentTransfer.info.uBufferSize = 0;
			m_currentTransfer.info.uRequestedSize = 0;
			m_currentTransfer.pAddress = NULL;
		}

		/** 2012 0x973410: one view per IO memory view the cache allows, plus one for the current transfer. */
		AKRESULT CAkDeviceBlocking::Init( const AkDeviceSettings & in_settings, AkDeviceID in_deviceID )
		{
			const AKRESULT eResult = CAkDeviceBase::Init( in_settings, in_deviceID );
			if( eResult != AK_Success )
			{
				return eResult;
			}
			const AkUInt32 uNumViews = m_mgrMemIO.NumViewsAvailable() + 1;
			m_pStmMemViewMem = (CAkStmMemView *)AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, sizeof( CAkStmMemView ) * uNumViews );
			if( !m_pStmMemViewMem )
			{
				return AK_Fail;
			}
			for( CAkStmMemView * pView = m_pStmMemViewMem; pView < m_pStmMemViewMem + uNumViews; ++pView )
			{
				AkPlacementNew( pView ) CAkStmMemView();
				m_poolStmMemView.AddFirst( pView );
			}
			return AK_Success;
		}

		CAkStmTask * CAkDeviceBlocking::CreateStd( AkFileDesc * in_pFileDesc, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream )
		{
			out_pStream = NULL;
			void * pMem = AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, sizeof( CAkStdStmBlocking ) );
			if( !pMem )
			{
				CAkStreamMgr::ForceCleanup( this, AK_MAX_PRIORITY );
				pMem = AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, sizeof( CAkStdStmBlocking ) );
				if( !pMem )
				{
					return NULL;
				}
			}
			CAkStdStmBlocking * pStm = AkPlacementNew( pMem ) CAkStdStmBlocking();
			if( pStm->Init( this, in_pFileDesc, in_eOpenMode ) == AK_Success )
			{
				AddTask( pStm );
				out_pStream = pStm;
				return pStm;
			}
			const AkMemPoolId poolId = CAkStreamMgr::m_streamMgrPoolId;
			pStm->~CAkStdStmBlocking();
			AK::MemoryMgr::Free( poolId, pStm );
			return NULL;
		}

		CAkStmTask * CAkDeviceBlocking::CreateAuto( AkFileDesc * in_pFileDesc, AkFileID in_fileID, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream )
		{
			out_pStream = NULL;
			void * pMem = AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, sizeof( CAkAutoStmBlocking ) );
			if( !pMem )
			{
				CAkStreamMgr::ForceCleanup( this, in_heuristics.priority );
				pMem = AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, sizeof( CAkAutoStmBlocking ) );
				if( !pMem )
				{
					return NULL;
				}
			}
			CAkAutoStmBlocking * pStm = AkPlacementNew( pMem ) CAkAutoStmBlocking();
			if( pStm->Init( this, in_pFileDesc, in_fileID, in_heuristics, in_pBufferSettings, m_uGranularity ) == AK_Success )
			{
				AddTask( pStm );
				out_pStream = pStm;
				return pStm;
			}
			const AkMemPoolId poolId = CAkStreamMgr::m_streamMgrPoolId;
			pStm->~CAkAutoStmBlocking();
			AK::MemoryMgr::Free( poolId, pStm );
			out_pStream = NULL;
			return NULL;
		}

		void CAkDeviceBlocking::PerformIO()
		{
			AkReal32 fOpDeadline;
			CAkStmTask * pTask = SchedulerFindNextTask( fOpDeadline );
			if( pTask )
			{
				ExecuteTask( pTask, fOpDeadline );
			}
		}

		/** 2012 0x973220. */
		void CAkDeviceBlocking::ExecuteTask( CAkStmTask * in_pTask, AkReal32 in_fOpDeadline )
		{
			++m_uNumConcurrentIO;
			if( in_pTask->EnsureFileIsOpen() != AK_Success )
			{
				in_pTask->Update( NULL, AK_Fail, false );
				return;
			}
			AkFileDesc * pFileDesc = NULL;
			CAkLowLevelTransfer * pLowLevelXfer = NULL;
			CAkStmMemView * pMemView = in_pTask->PrepareTransfer( pFileDesc, pLowLevelXfer );
			if( !pMemView )
			{
				in_pTask->Update( NULL, AK_Cancelled, false );
				return;
			}
			AKRESULT eResult = AK_Success;
			if( pLowLevelXfer )
			{
				AkIoHeuristics heuristics;
				heuristics.fDeadline = in_fOpDeadline;
				heuristics.priority = in_pTask->Priority();
				CAkLowLevelTransferBlocking * pXfer = static_cast<CAkLowLevelTransferBlocking *>( pLowLevelXfer );
				IAkIOHookBlocking * pHook = static_cast<IAkIOHookBlocking *>( m_pLowLevelHook );
				if( in_pTask->IsWriteOp() )
				{
					eResult = pHook->Write( *pFileDesc, heuristics, pXfer->pAddress, pXfer->info );
				}
				else
				{
					eResult = pHook->Read( *pFileDesc, heuristics, pXfer->pAddress, pXfer->info );
				}
				AkMemBlock * pBlock = pMemView->Block();
				pBlock->pTransfer = NULL;
				if( eResult != AK_Success && pBlock->fileID != AK_INVALID_FILE_ID )
				{
					m_mgrMemIO.UntagBlock( pBlock );
				}
			}
			in_pTask->Update( pMemView, eResult, pLowLevelXfer != NULL );
		}

		/** 2012 0x9736b0: a standard stream transfers straight into the user buffer (its own m_memBlock). */
		CAkStmMemView * CAkDeviceBlocking::CreateMemViewStd( CAkStmTask * in_pOwner, AkMemBlock * in_pMemBlock, AkUInt32 in_uDataOffset, AkUInt64 in_uPosition, AkUInt32 in_uBufferSize, AkUInt32 in_uRequestedSize, CAkLowLevelTransfer *& out_pLowLevelXfer )
		{
			out_pLowLevelXfer = NULL;
			CAkStmMemView * pMemView;
			{
				AkAutoLock<CAkLock> gate( m_lockSems );
				pMemView = m_poolStmMemView.m_pFirst;
				if( pMemView )
				{
					m_poolStmMemView.m_pFirst = pMemView->pNextView;
				}
			}
			m_currentTransfer.info.uFilePosition = in_uPosition + (AkUInt64)in_pOwner->m_uLLBlockSize * in_pOwner->m_pFileDesc->uSector;
			m_currentTransfer.pAddress = (AkUInt8 *)in_pMemBlock->pData + in_uDataOffset;
			m_currentTransfer.info.uBufferSize = in_uBufferSize;
			m_currentTransfer.info.uRequestedSize = in_uRequestedSize;
			out_pLowLevelXfer = &m_currentTransfer;
			in_pMemBlock->pTransfer = &m_currentTransfer;
			pMemView->Attach( in_pMemBlock, in_uDataOffset );
			return pMemView;
		}

		/** 2012 0x973760: an automatic stream reads into an IO memory block (or reuses a cached one). */
		CAkStmMemView * CAkDeviceBlocking::CreateMemViewAuto( CAkStmTask * in_pOwner, AkFileID in_fileID, AkUInt64 in_uPosition, AkUInt32 in_uMinSize, AkUInt32 in_uRequiredAlign, AkUInt32 & io_uRequestedSize, CAkLowLevelTransfer *& out_pLowLevelXfer )
		{
			out_pLowLevelXfer = NULL;
			AkAutoLock<CAkLock> gate( m_lockSems );
			CAkStmMemView * pMemView = m_poolStmMemView.m_pFirst;
			if( pMemView )
			{
				m_poolStmMemView.m_pFirst = pMemView->pNextView;
			}
			AkMemBlock * pMemBlock = NULL;
			AkUInt32 uOffset = 0;
			if( m_mgrMemIO.UseCache() && in_fileID != AK_INVALID_FILE_ID )
			{
				uOffset = m_mgrMemIO.GetCachedBlock( in_fileID, in_uPosition, in_uMinSize, in_uRequiredAlign, io_uRequestedSize, pMemBlock );
				if( pMemBlock )
				{
					pMemView->Attach( pMemBlock, uOffset );
					return pMemView;
				}
			}
			m_mgrMemIO.GetOldestFreeBlock( pMemBlock );
			if( pMemBlock )
			{
				m_currentTransfer.pAddress = pMemBlock->pData;
				m_currentTransfer.info.uFilePosition = in_uPosition + (AkUInt64)in_pOwner->m_uLLBlockSize * in_pOwner->m_pFileDesc->uSector;
				m_currentTransfer.info.uBufferSize = m_uGranularity;
				m_currentTransfer.info.uRequestedSize = io_uRequestedSize;
				out_pLowLevelXfer = &m_currentTransfer;
				m_mgrMemIO.TagBlock( pMemBlock, &m_currentTransfer, in_fileID, in_uPosition, io_uRequestedSize );
				pMemView->Attach( pMemBlock, uOffset );
				return pMemView;
			}
			DestroyMemView( pMemView );
			NotifyMemIdle();
			return NULL;
		}

		// -- CAkStdStmBlocking -----------------------------------------------------------------------

		/** 2012 0x973910: mark for destruction and wait for the transfer in flight, if any. */
		void CAkStdStmBlocking::Destroy()
		{
			m_lockStatus.Lock();
			SetToBeDestroyed();
			SetStatus( AK_StmStatusCancelled );
			if( m_pCurrentTransfer )
			{
				m_bTransferCancelling = true;
				SetBlockedStatus();
				m_lockStatus.Unlock();
				m_pDevice->WaitForIOCompletion( this );
				m_bTransferCancelling = false;
				m_lockStatus.Lock();
			}
			m_lockStatus.Unlock();
		}

		void CAkStdStmBlocking::Cancel()
		{
			m_lockStatus.Lock();
			SetStatus( AK_StmStatusCancelled );
			if( m_pCurrentTransfer )
			{
				m_bTransferCancelling = true;
				SetBlockedStatus();
				m_lockStatus.Unlock();
				m_pDevice->WaitForIOCompletion( this );
				m_lockStatus.Lock();
				m_bTransferCancelling = false;
			}
			m_lockStatus.Unlock();
		}

		bool CAkStdStmBlocking::CanBeDestroyed()
		{
			AkAutoLock<CAkLock> status( m_lockStatus );
			return !m_pCurrentTransfer && !m_bTransferCancelling;
		}

		/** 2012 0x9739b0: the next granularity-sized chunk of the user op. */
		CAkStmMemView * CAkStdStmBlocking::PrepareTransfer( AkFileDesc *& out_pFileDesc, CAkLowLevelTransfer *& out_pLowLevelXfer )
		{
			AkAutoLock<CAkLock> atomicPosition( m_lockStatus );
			if( m_bIsToBeDestroyed || !m_bIsReadyForIO )
			{
				return NULL;
			}
			out_pFileDesc = m_pFileDesc;
			const AkUInt64 uPosition = m_memBlock.uPosition + m_uTotalScheduledSize;
			AkUInt32 uBufferSize = m_memBlock.uAvailableSize - m_uTotalScheduledSize;
			if( uBufferSize > m_pDevice->GetGranularity() )
			{
				uBufferSize = m_pDevice->GetGranularity();
			}
			const AkUInt32 uRequestedSize = m_bIsWriteOp ? uBufferSize : ClampRequestSizeToEof( uPosition, uBufferSize );
			CAkStmMemView * pMemView = static_cast<CAkDeviceBlocking *>( m_pDevice )->CreateMemViewStd( this, &m_memBlock, m_uTotalScheduledSize, uPosition, uBufferSize, uRequestedSize, out_pLowLevelXfer );
			if( pMemView )
			{
				m_uTotalScheduledSize += uRequestedSize;
				m_pCurrentTransfer = pMemView;
				m_iIOStartTime = m_pDevice->GetTime();
			}
			return pMemView;
		}

		void CAkStdStmBlocking::Update( CAkStmMemView * in_pTransfer, AKRESULT in_eIOResult, bool /*in_bRequiredLowLevelXfer*/ )
		{
			AkAutoLock<CAkLock> status( m_lockStatus );
			--m_pDevice->m_uNumConcurrentIO;
			if( in_pTransfer )
			{
				const bool bStoreData = in_eIOResult == AK_Success && in_pTransfer->Status() != CAkStmMemView::TransferStatus_Cancelled;
				AddMemView( in_pTransfer, bStoreData );
			}
			m_pCurrentTransfer = NULL;
			UpdateTaskStatus( in_eIOResult );
		}

		// -- CAkAutoStmBlocking ----------------------------------------------------------------------

		void CAkAutoStmBlocking::CancelCurrentTransfer()
		{
			m_pCurrentTransfer->TagAsCancelled();
			CAkStmMemView * pView = m_pCurrentTransfer;
			m_uVirtualBufferingSize -= GetEffectiveViewSize( pView );
			pView->m_uOffsetInBlock = pView->m_pBlock->uAvailableSize;
			m_pCurrentTransfer = NULL;
			m_bTransferCancelled = true;
		}

		bool CAkAutoStmBlocking::CanBeDestroyed()
		{
			AkAutoLock<CAkLock> status( m_lockStatus );
			return !m_pCurrentTransfer && !m_bTransferCancelled;
		}

		/** 2012 0x973b60: the next buffer after the virtual position, wrapping at the loop end. */
		CAkStmMemView * CAkAutoStmBlocking::PrepareTransfer( AkFileDesc *& out_pFileDesc, CAkLowLevelTransfer *& out_pLowLevelXfer )
		{
			out_pFileDesc = m_pFileDesc;
			AkAutoLock<CAkLock> atomicPosition( m_lockStatus );
			if( m_bIsToBeDestroyed || !m_bIsReadyForIO )
			{
				return NULL;
			}
			AkUInt64 uFilePosition = GetVirtualFilePosition();
			if( m_uLoopEnd && uFilePosition >= m_uLoopEnd )
			{
				uFilePosition = m_uLoopStart;
			}
			AkUInt32 uRequestedSize = ClampRequestSizeToEof( uFilePosition, m_uBufferSize );
			const AkUInt32 uMinSize = m_uMinBufferSize < uRequestedSize ? m_uMinBufferSize : uRequestedSize;
			CAkStmMemView * pMemView = static_cast<CAkDeviceBlocking *>( m_pDevice )->CreateMemViewAuto( this, m_fileID, uFilePosition, uMinSize, m_uBufferAlignment, uRequestedSize, out_pLowLevelXfer );
			if( !pMemView )
			{
				return NULL;
			}
			m_pCurrentTransfer = pMemView;
			if( uFilePosition < m_uLoopEnd && uFilePosition + uRequestedSize > m_uLoopEnd )
			{
				uRequestedSize = m_uLoopEnd - (AkUInt32)uFilePosition;
			}
			m_uVirtualBufferingSize += uRequestedSize;
			UpdateSchedulingStatus();
			m_iIOStartTime = m_pDevice->GetTime();
			return pMemView;
		}

		void CAkAutoStmBlocking::Update( CAkStmMemView * in_pTransfer, AKRESULT in_eIOResult, bool in_bRequiredLowLevelXfer )
		{
			AkAutoLock<CAkLock> status( m_lockStatus );
			--m_pDevice->m_uNumConcurrentIO;
			if( in_pTransfer )
			{
				const bool bStoreData = in_eIOResult == AK_Success && m_pCurrentTransfer && in_pTransfer->Status() != CAkStmMemView::TransferStatus_Cancelled;
				if( !in_bRequiredLowLevelXfer && bStoreData )
				{
					in_pTransfer->TagAsReady();
				}
				AddMemView( in_pTransfer, bStoreData );
			}
			m_pCurrentTransfer = NULL;
			m_bTransferCancelled = false;
			UpdateTaskStatus( in_eIOResult );
		}

		AkUInt64 CAkAutoStmBlocking::GetVirtualFilePosition()
		{
			if( m_pCurrentTransfer )
			{
				return m_pCurrentTransfer->EndPosition();
			}
			if( m_listBuffers.m_ulNumListItems <= m_uNextToGrant )
			{
				return m_uNextExpectedUserPosition;
			}
			return m_listBuffers.m_pLast->EndPosition();
		}

		void CAkAutoStmBlocking::CancelPendingTransfers( AkUInt64 in_uMaxKeepPosition )
		{
			if( m_pCurrentTransfer && m_pCurrentTransfer->StartPosition() >= in_uMaxKeepPosition )
			{
				CancelCurrentTransfer();
			}
		}

		AkUInt32 CAkAutoStmBlocking::ComputeVirtualBuffering()
		{
			AkUInt32 uVirtualBuffering = 0;
			CAkStmMemView * pView = m_listBuffers.m_pFirst;
			for( AkUInt32 uSkip = m_uNextToGrant; pView && uSkip; --uSkip )
			{
				pView = pView->pNextView;
			}
			for( ; pView; pView = pView->pNextView )
			{
				uVirtualBuffering += GetEffectiveViewSize( pView );
			}
			if( m_pCurrentTransfer )
			{
				uVirtualBuffering += GetEffectiveViewSize( m_pCurrentTransfer );
			}
			return uVirtualBuffering;
		}

		/** 2012 0x973e40: buffers smaller than the new minimum (and everything after the first one) go. */
		void CAkAutoStmBlocking::FlushSmallBuffersAndPendingTransfers( AkUInt32 in_uMinBufferSize )
		{
			bool bFlushAll = false;
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
				if( pView )
				{
					while( pView )
					{
						if( bFlushAll || pView->Size() < in_uMinBufferSize )
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
							bFlushAll = true;
						}
						else
						{
							pPrev = pView;
							pView = pView->pNextView;
						}
					}
					if( bFlushAll )
					{
						m_pDevice->NotifyMemChange();
					}
				}
			}
			if( m_pCurrentTransfer && ( bFlushAll || m_pCurrentTransfer->Size() < in_uMinBufferSize ) )
			{
				CancelCurrentTransfer();
			}
		}

		void CAkAutoStmBlocking::RecomputeVirtualBuffering()
		{
			m_uVirtualBufferingSize = ComputeVirtualBuffering();
		}
	}
}
