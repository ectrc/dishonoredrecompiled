/*=============================================================================
	AkIOMemMgr.cpp - CAkIOMemMgr, the IO memory of one streaming device.

	DISHONORED(port): agent IH1 (wave 26); wwise\sdk\source\streammanager\common\akiomemmgr.cpp
	(2012 0x974510..0x974d10): AkMemBlocksDictionnary::Move 0x974510, CloneTempBlock 0x974580,
	DestroyTempBlock 0x9745f0, ctor 0x974650, ReleaseBlock 0x974690, GetOldestFreeBlock 0x974710,
	UntagBlock 0x974760, TagBlock 0x9748c0, Init 0x974ae0, Term 0x974c90, GetCachedBlock 0x974d10.
	The IO memory is one AkFixedSizeBlocksMode pool holding a single block of uIOMemorySize rounded down to the
	granularity, cut into granularity-sized AkMemBlocks on a free list. The data cache (blocks kept tagged with
	their file id and position for reuse) is on only when fMaxCacheRatio > 1; retail Dishonored keeps the default
	1.0, so TagBlock only records position/size and the dictionary is never searched there.

	Deviation (named): the cache-only paths (TagBlock with the cache on, UntagBlock, GetCachedBlock) are written
	to the SDK's sorted-dictionary contract - (fileID, position, data) order, Move() into the insertion slot,
	the containing block of the same file - rather than transcribed branch for branch; they are dead in
	Dishonored's configuration (cache ratio 1.0, so no block is ever tagged with a file id).
=============================================================================*/
#include "AkStreamMgrPrivate.h"

namespace AK
{
	namespace StreamMgr
	{
		/** 2012 0x974510: rotate one dictionary entry from source to destination. */
		void AkMemBlocksDictionnary::Move( AkUInt32 in_uIndexSource, AkUInt32 in_uIndexDestination )
		{
			const AkUInt16 uMoved = m_pItems[ in_uIndexSource ];
			if( in_uIndexSource < in_uIndexDestination )
			{
				const AkUInt32 uLast = in_uIndexDestination - 1;
				for( AkUInt32 i = in_uIndexSource + 1; i <= uLast; ++i )
				{
					m_pItems[ i - 1 ] = m_pItems[ i ];
				}
				m_pItems[ uLast ] = uMoved;
				return;
			}
			if( in_uIndexSource > in_uIndexDestination )
			{
				for( AkUInt32 i = in_uIndexSource; i > in_uIndexDestination; --i )
				{
					m_pItems[ i ] = m_pItems[ i - 1 ];
				}
			}
			m_pItems[ in_uIndexDestination ] = uMoved;
		}

		CAkIOMemMgr::CAkIOMemMgr()
			: m_pIOMemory( NULL )
			, m_pMemBlocks( NULL )
			, m_streamIOPoolId( AK_INVALID_POOL_ID )
			, m_uNumViewsAvail( 0 )
			, m_bUseCache( false )
		{
		}

		CAkIOMemMgr::~CAkIOMemMgr()
		{
		}

		/** 2012 0x974ae0. */
		AKRESULT CAkIOMemMgr::Init( const AkDeviceSettings & in_settings )
		{
			const AkUInt32 uNumBuffers = in_settings.uIOMemorySize / in_settings.uGranularity;
			const AkUInt32 uMemorySize = uNumBuffers * in_settings.uGranularity;
			if( uMemorySize )
			{
				m_streamIOPoolId = AK::MemoryMgr::CreatePool( in_settings.pIOMemory, uMemorySize, uMemorySize, in_settings.ePoolAttributes | AkFixedSizeBlocksMode, in_settings.uIOMemoryAlignment );
			}
			if( m_streamIOPoolId == AK_INVALID_POOL_ID )
			{
				return in_settings.uIOMemorySize ? AK_Fail : AK_Success;
			}
			m_pIOMemory = AK::MemoryMgr::GetBlock( m_streamIOPoolId );
			m_pMemBlocks = (AkMemBlock *)AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, sizeof( AkMemBlock ) * uNumBuffers );
			if( !m_pMemBlocks )
			{
				return AK_Fail;
			}
			AkUInt8 * pIOMemory = (AkUInt8 *)m_pIOMemory;
			for( AkMemBlock * pBlock = m_pMemBlocks; pBlock < m_pMemBlocks + uNumBuffers; ++pBlock )
			{
				AkPlacementNew( pBlock ) AkMemBlock( pIOMemory );
				pIOMemory += in_settings.uGranularity;
				m_listFreeBuffers.AddLast( pBlock );
			}
			if( m_arMemBlocks.Reserve( uNumBuffers ) != AK_Success )
			{
				return AK_Fail;
			}
			for( AkUInt16 i = 0; i < uNumBuffers; ++i )
			{
				m_arMemBlocks.AddLast( i );
			}
			m_uNumViewsAvail = (AkUInt32)(AkInt64)( (AkReal64)uNumBuffers * in_settings.fMaxCacheRatio + 0.5 );
			if( m_uNumViewsAvail < uNumBuffers )
			{
				m_uNumViewsAvail = uNumBuffers;
			}
			m_bUseCache = in_settings.fMaxCacheRatio > 1.f;
			return AK_Success;
		}

		void CAkIOMemMgr::Term()
		{
			m_arMemBlocks.Term();
			if( m_listFreeBuffers.m_pFirst )
			{
				m_listFreeBuffers.RemoveAll();
				AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, m_pMemBlocks );
			}
			m_listFreeBuffers.RemoveAll();
			if( m_streamIOPoolId != AK_INVALID_POOL_ID )
			{
				AK::MemoryMgr::ReleaseBlock( m_streamIOPoolId, m_pIOMemory );
				AK::MemoryMgr::DestroyPool( m_streamIOPoolId );
				m_streamIOPoolId = AK_INVALID_POOL_ID;
			}
		}

		/** 2012 0x974690: untagged blocks go to the front (reused first), tagged ones to the back (kept longest). */
		AkUInt32 CAkIOMemMgr::ReleaseBlock( AkMemBlock * in_pMemBlock )
		{
			const AkUInt16 uRefCount = --in_pMemBlock->uRefCount;
			if( !uRefCount )
			{
				if( in_pMemBlock->fileID == AK_INVALID_FILE_ID )
				{
					m_listFreeBuffers.AddFirst( in_pMemBlock );
				}
				else
				{
					m_listFreeBuffers.AddLast( in_pMemBlock );
				}
			}
			++m_uNumViewsAvail;
			return uRefCount;
		}

		void CAkIOMemMgr::GetOldestFreeBlock( AkMemBlock *& out_pMemBlock )
		{
			out_pMemBlock = m_listFreeBuffers.m_pFirst;
			if( out_pMemBlock )
			{
				++out_pMemBlock->uRefCount;
				m_listFreeBuffers.RemoveFirst();
				--m_uNumViewsAvail;
				out_pMemBlock->pNextBlock = NULL;
			}
		}

		static bool MemBlockLess( AkFileID in_fileID, AkUInt64 in_uPosition, void * in_pData, const AkMemBlock & in_rBlock )
		{
			if( in_fileID != in_rBlock.fileID )
			{
				return in_fileID < in_rBlock.fileID;
			}
			if( in_uPosition != in_rBlock.uPosition )
			{
				return in_uPosition < in_rBlock.uPosition;
			}
			return in_pData < in_rBlock.pData;
		}

		AkInt32 CAkIOMemMgr::FindBlockIndex( const AkMemBlock & in_block ) const
		{
			AkInt32 iTop = 0;
			AkInt32 iBottom = (AkInt32)m_arMemBlocks.Length() - 1;
			AkInt32 iThis = 0;
			while( iTop <= iBottom )
			{
				iThis = ( iBottom - iTop ) / 2 + iTop;
				const AkMemBlock & rBlock = m_pMemBlocks[ m_arMemBlocks[ iThis ] ];
				if( MemBlockLess( in_block.fileID, in_block.uPosition, in_block.pData, rBlock ) )
				{
					iBottom = iThis - 1;
				}
				else if( MemBlockLess( rBlock.fileID, rBlock.uPosition, rBlock.pData, in_block ) )
				{
					iTop = iThis + 1;
				}
				else
				{
					break;
				}
			}
			return iThis;
		}

		/** First dictionary slot whose block sorts after (fileID, position, data): where a re-tagged block belongs. */
		AkInt32 CAkIOMemMgr::FindInsertionIndex( AkFileID in_fileID, AkUInt64 in_uPosition, void * in_pData ) const
		{
			AkInt32 iTop = 0;
			AkInt32 iBottom = (AkInt32)m_arMemBlocks.Length() - 1;
			while( iTop <= iBottom )
			{
				const AkInt32 iThis = ( iBottom - iTop ) / 2 + iTop;
				const AkMemBlock & rBlock = m_pMemBlocks[ m_arMemBlocks[ iThis ] ];
				if( MemBlockLess( in_fileID, in_uPosition, in_pData, rBlock ) )
				{
					iBottom = iThis - 1;
				}
				else
				{
					iTop = iThis + 1;
				}
			}
			return iTop;
		}

		/** 2012 0x9748c0: with the cache on, the block moves to its sorted slot under its new (file, position). */
		void CAkIOMemMgr::TagBlock( AkMemBlock * in_pMemBlock, CAkLowLevelTransfer * in_pTransfer, AkFileID in_fileID, AkUInt64 in_uPosition, AkUInt32 in_uDataSize )
		{
			if( !m_bUseCache )
			{
				in_pMemBlock->uPosition = in_uPosition;
				in_pMemBlock->uAvailableSize = in_uDataSize;
				in_pMemBlock->pTransfer = in_pTransfer;
				return;
			}
			const AkInt32 iOriginalLocation = FindBlockIndex( *in_pMemBlock );
			const AkInt32 iNewLocation = FindInsertionIndex( in_fileID, in_uPosition, in_pMemBlock->pData );
			m_arMemBlocks.Move( (AkUInt32)iOriginalLocation, (AkUInt32)iNewLocation );
			in_pMemBlock->fileID = in_fileID;
			in_pMemBlock->uPosition = in_uPosition;
			in_pMemBlock->uAvailableSize = in_uDataSize;
			in_pMemBlock->pTransfer = in_pTransfer;
		}

		/** 2012 0x974760: a failed transfer's block becomes anonymous again (file id invalid). */
		void CAkIOMemMgr::UntagBlock( AkMemBlock * in_pMemBlock )
		{
			const AkInt32 iOriginalLocation = FindBlockIndex( *in_pMemBlock );
			const AkInt32 iNewLocation = FindInsertionIndex( AK_INVALID_FILE_ID, in_pMemBlock->uPosition, in_pMemBlock->pData );
			m_arMemBlocks.Move( (AkUInt32)iOriginalLocation, (AkUInt32)iNewLocation );
			in_pMemBlock->fileID = AK_INVALID_FILE_ID;
		}

		/**
		 * 2012 0x974d10: a cached block of the same file that contains in_uPosition with at least in_uMinSize
		 * bytes, aligned as required. Only reached with the cache on (fMaxCacheRatio > 1).
		 */
		AkUInt32 CAkIOMemMgr::GetCachedBlock( AkFileID in_fileID, AkUInt64 in_uPosition, AkUInt32 in_uMinSize, AkUInt32 in_uRequiredAlign, AkUInt32 & io_uRequestedSize, AkMemBlock *& out_pMemBlock )
		{
			out_pMemBlock = NULL;
			if( m_uNumViewsAvail == m_listFreeBuffers.m_ulNumListItems )
			{
				return 0;
			}
			AkMemBlock * pCandidate = NULL;
			for( AkUInt32 i = 0; i < m_arMemBlocks.Length(); ++i )
			{
				AkMemBlock * pBlock = &m_pMemBlocks[ m_arMemBlocks[ i ] ];
				if( pBlock->fileID == in_fileID && pBlock->uPosition <= in_uPosition && in_uPosition + in_uMinSize <= pBlock->uPosition + pBlock->uAvailableSize )
				{
					if( !pCandidate || pBlock->uPosition > pCandidate->uPosition )
					{
						pCandidate = pBlock;
					}
				}
			}
			if( !pCandidate )
			{
				return 0;
			}
			const AkUInt32 uOffset = (AkUInt32)( in_uPosition - pCandidate->uPosition );
			const AkUInt32 uAvailable = pCandidate->uAvailableSize - uOffset;
			if( uAvailable > io_uRequestedSize || uAvailable % in_uRequiredAlign || ( (AkUIntPtr)pCandidate->pData + uOffset ) % in_uRequiredAlign || uAvailable < in_uMinSize )
			{
				return 0;
			}
			io_uRequestedSize = uAvailable;
			if( !pCandidate->uRefCount )
			{
				m_listFreeBuffers.Remove( pCandidate );
				pCandidate->pNextBlock = NULL;
			}
			++pCandidate->uRefCount;
			--m_uNumViewsAvail;
			out_pMemBlock = pCandidate;
			return uOffset;
		}

		void CAkIOMemMgr::CloneTempBlock( AkMemBlock * in_pMemBlockBase, AkMemBlock *& out_pMemBlock )
		{
			out_pMemBlock = (AkMemBlock *)AK::MemoryMgr::Malloc( CAkStreamMgr::m_streamMgrPoolId, sizeof( AkMemBlock ) );
			if( out_pMemBlock )
			{
				AkPlacementNew( out_pMemBlock ) AkMemBlock( in_pMemBlockBase->pData );
				out_pMemBlock->uAvailableSize = in_pMemBlockBase->uAvailableSize;
				out_pMemBlock->uPosition = in_pMemBlockBase->uPosition;
			}
		}

		void CAkIOMemMgr::DestroyTempBlock( AkMemBlock * in_pMemBlockBase, AkMemBlock * in_pMemBlock )
		{
			if( in_pMemBlock != in_pMemBlockBase )
			{
				AK::MemoryMgr::Free( CAkStreamMgr::m_streamMgrPoolId, in_pMemBlock );
			}
		}
	}
}
