/*=============================================================================
	AkMemoryMgr.cpp - AK::MemoryMgr, the Wwise 2012.1 pool manager.

	DISHONORED(port): agent IH1 (wave 26), replacing AkSilentMemoryMgr.cpp. Two retail units:
	  wwise\sdk\source\soundengine\akmemorymgr\common\akmemorymgrbase.cpp (17 functions)
	    AkMemPool::Init 0x9605f0, IsInitialized 0x960620, SetPoolName 0x960630, GetPoolAttributes 0x960660,
	    GetBlockSize 0x960680, GetNumPools 0x9606a0, GetMaxPools 0x9606b0, CheckPoolId 0x9606c0,
	    InitBase 0x9606f0, Malloc 0x960790, Malign 0x960800, Free 0x960860, GetPoolStats 0x9608b0,
	    GetPoolMemoryUsed 0x960910, GetBlock 0x960950, ReleaseBlock 0x9609a0, Term 0x9609f0
	  wwise\sdk\source\soundengine\akmemorymgr\win32\akmemorymgr.cpp (4 functions)
	    Init 0x960a70, DeallocatePool 0x960aa0, CreatePool 0x960af0, DestroyPool 0x960cc0
	Every pool is one reservation: AkFixedSizeBlocksMode (attribute 8) pools hand out equal blocks from a
	free list (the IO pool, the bank data pools), the others are TLSF heaps (tlsf.cpp). Each pool has its own
	CAkLock; Malloc/Malign/Free/stats take it, GetBlock/ReleaseBlock do not (retail's callers lock).
	The reservation goes through AK::AllocHook (AkMalloc) or AK::VirtualAllocHook (AkVirtualAlloc), which
	AkSilentHooks.cpp routes to appMalloc once AkAudio installs its allocator.

	As retail: no pool id is range-checked outside CheckPoolId / GetPoolStats; SetPoolName only converts the
	name (the profiler that would display it is not built in); GetPoolStats leaves uAllocs/uFrees/uMaxFreeBlock
	at 0 and does not write uPeakUsed.
=============================================================================*/
#include <AkPrivateTypes.h>
#include <AK/SoundEngine/Common/AkModule.h>

#include "tlsf.h"

/** AkMemPool: 76 bytes in the 2012 PDB (ulNumBlocks .. ulTotalAvailable, CAkLock at 36). */
struct AkLinkedBuffer
{
	AkLinkedBuffer * pNextItem;
};

struct AkMemPool
{
	void Init()
	{
		ulNumBlocks = 0;
		ulBlockSize = 0;
		pcAllocAddress = NULL;
		pcMemAddress = NULL;
		eAttributes = AkMalloc;
		pTlsfPool = NULL;
		ulAlign = 0;
		ulUsed = 0;
		ulTotalAvailable = 0;
	}

	AkUInt32							ulNumBlocks;
	AkUInt32							ulBlockSize;
	char *								pcAllocAddress;
	char *								pcMemAddress;
	AkListBare<AkLinkedBuffer>			listBuffers;
	AkUInt32							eAttributes;
	AkUInt32							bAllocated : 1;
	CAkLock								lock;
	tlsf_pool							pTlsfPool;
	AkUInt32							ulAlign;
	AkUInt32							ulUsed;
	AkUInt32							ulTotalAvailable;
};

static_assert( sizeof( AkMemPool ) == 76, "AkMemPool: 2012 PDB sizeof 76" );

namespace AK
{
	namespace MemoryMgr
	{
		static AkMemPool *	s_pMemPools = NULL;
		static AkInt32		s_iMaxNumPools = 0;
		static AkInt32		s_iNumPools = 0;
		static bool			s_bInitialized = false;

		bool IsInitialized()
		{
			return s_bInitialized;
		}

		AKRESULT SetPoolName( AkMemPoolId /*in_poolId*/, const char * in_pszPoolName )
		{
			wchar_t wideString[ AK_DEFAULT_PATH_LENGTH ];
			MultiByteToWideChar( CP_ACP, 0, in_pszPoolName, -1, wideString, AK_DEFAULT_PATH_LENGTH );
			return AK_Success;
		}

		AkMemPoolAttributes GetPoolAttributes( AkMemPoolId in_poolId )
		{
			return (AkMemPoolAttributes)s_pMemPools[ in_poolId ].eAttributes;
		}

		AkUInt32 GetBlockSize( AkMemPoolId in_poolId )
		{
			return s_pMemPools[ in_poolId ].ulBlockSize;
		}

		AkInt32 GetNumPools()
		{
			return s_iNumPools;
		}

		AkInt32 GetMaxPools()
		{
			return s_iMaxNumPools;
		}

		AKRESULT CheckPoolId( AkMemPoolId in_poolId )
		{
			if( in_poolId >= s_iMaxNumPools || !s_pMemPools[ in_poolId ].ulNumBlocks )
			{
				return AK_InvalidID;
			}
			return AK_Success;
		}

		AKRESULT InitBase( AkMemPoolId in_iNumPools )
		{
			s_pMemPools = (AkMemPool *)AK::AllocHook( sizeof( AkMemPool ) * in_iNumPools );
			if( !s_pMemPools )
			{
				return AK_Fail;
			}
			for( AkInt32 i = 0; i < in_iNumPools; ++i )
			{
				AkPlacementNew( &s_pMemPools[ i ] ) AkMemPool;
				s_pMemPools[ i ].Init();
			}
			s_iNumPools = 0;
			s_iMaxNumPools = in_iNumPools;
			s_bInitialized = true;
			return AK_Success;
		}

		void * Malloc( AkMemPoolId in_poolId, AkUInt32 in_uSize )
		{
			if( !in_uSize )
			{
				return NULL;
			}
			AkMemPool & pool = s_pMemPools[ in_poolId ];
			AkAutoLock<CAkLock> gate( pool.lock );
			void * pAlloc = ( pool.ulAlign <= 4 ) ? tlsf_malloc( pool.pTlsfPool, in_uSize ) : tlsf_memalign( pool.pTlsfPool, pool.ulAlign, in_uSize );
			if( pAlloc )
			{
				pool.ulUsed += (AkUInt32)tlsf_block_size( pAlloc ) + 4;
			}
			return pAlloc;
		}

		void * Malign( AkMemPoolId in_poolId, AkUInt32 in_uSize, AkUInt32 in_uAlignment )
		{
			if( !in_uSize )
			{
				return NULL;
			}
			AkMemPool & pool = s_pMemPools[ in_poolId ];
			AkAutoLock<CAkLock> gate( pool.lock );
			void * pAlloc = tlsf_memalign( pool.pTlsfPool, in_uAlignment, in_uSize );
			if( pAlloc )
			{
				pool.ulUsed += (AkUInt32)tlsf_block_size( pAlloc ) + 4;
			}
			return pAlloc;
		}

		AKRESULT Free( AkMemPoolId in_poolId, void * in_pMemAddress )
		{
			if( in_pMemAddress )
			{
				AkMemPool & pool = s_pMemPools[ in_poolId ];
				AkAutoLock<CAkLock> gate( pool.lock );
				pool.ulUsed -= (AkUInt32)tlsf_block_size( in_pMemAddress ) + 4;
				tlsf_free( pool.pTlsfPool, in_pMemAddress );
			}
			return AK_Success;
		}

		AKRESULT GetPoolStats( AkMemPoolId in_poolId, PoolStats & out_stats )
		{
			if( in_poolId >= s_iMaxNumPools || in_poolId < 0 )
			{
				return AK_Fail;
			}
			AkMemPool & pool = s_pMemPools[ in_poolId ];
			AkAutoLock<CAkLock> gate( pool.lock );
			out_stats.uReserved = pool.ulTotalAvailable;
			out_stats.uUsed = pool.ulUsed;
			out_stats.uAllocs = 0;
			out_stats.uFrees = 0;
			out_stats.uMaxFreeBlock = 0;
			return AK_Success;
		}

		void GetPoolMemoryUsed( AkMemPoolId in_poolId, PoolMemInfo & out_memInfo )
		{
			AkMemPool & pool = s_pMemPools[ in_poolId ];
			AkAutoLock<CAkLock> gate( pool.lock );
			out_memInfo.uUsed = pool.ulUsed;
			out_memInfo.uReserved = pool.ulTotalAvailable;
		}

		void * GetBlock( AkMemPoolId in_poolId )
		{
			AkMemPool & pool = s_pMemPools[ in_poolId ];
			AkLinkedBuffer * pBuffer = pool.listBuffers.RemoveFirst();
			if( pBuffer )
			{
				pool.ulUsed += pool.ulBlockSize;
			}
			return pBuffer;
		}

		AKRESULT ReleaseBlock( AkMemPoolId in_poolId, void * in_pMemAddress )
		{
			AkMemPool & pool = s_pMemPools[ in_poolId ];
			pool.ulUsed -= pool.ulBlockSize;
			pool.listBuffers.AddLast( (AkLinkedBuffer *)in_pMemAddress );
			return AK_Success;
		}

		void Term()
		{
			if( !s_bInitialized )
			{
				return;
			}
			for( AkInt32 i = 0; i < s_iMaxNumPools; ++i )
			{
				if( s_pMemPools[ i ].ulNumBlocks )
				{
					DestroyPool( i );
				}
				s_pMemPools[ i ].~AkMemPool();
			}
			AK::FreeHook( s_pMemPools );
			s_iMaxNumPools = 0;
			s_iNumPools = 0;
			s_pMemPools = NULL;
			s_bInitialized = false;
		}

		// -- akmemorymgr\win32\akmemorymgr.cpp ------------------------------------------------------

		AKRESULT Init( AkMemSettings * in_pSettings )
		{
			if( IsInitialized() || !in_pSettings )
			{
				return AK_Fail;
			}
			return InitBase( (AkMemPoolId)in_pSettings->uMaxNumPools );
		}

		void GetDefaultSettings( AkMemSettings & out_settings )
		{
			out_settings.uMaxNumPools = 32;
		}

		/** 2012 0x960aa0: the reservation goes back to the hook it came from. */
		static void DeallocatePoolMemory( AkMemPool & io_pool )
		{
			if( ( io_pool.eAttributes & AkAllocMask ) == AkVirtualAlloc )
			{
				AK::VirtualFreeHook( io_pool.pcAllocAddress, 0, MEM_RELEASE );
			}
			else
			{
				AK::FreeHook( io_pool.pcAllocAddress );
			}
			io_pool.pcMemAddress = NULL;
			io_pool.pcAllocAddress = NULL;
		}

		void DeallocatePool( AkMemPoolId in_poolId )
		{
			DeallocatePoolMemory( s_pMemPools[ in_poolId ] );
		}

		AkMemPoolId CreatePool( void * in_pMemAddress, AkUInt32 in_uMemSize, AkUInt32 in_uBlockSize, AkUInt32 in_eAttributes, AkUInt32 in_uBlockAlign )
		{
			const AkUInt32 eAlloc = in_eAttributes & AkAllocMask;
			if( eAlloc != AkNoAlloc )
			{
				if( eAlloc != AkMalloc && eAlloc != AkVirtualAlloc )
				{
					return AK_INVALID_POOL_ID;
				}
			}
			else if( !in_pMemAddress )
			{
				return AK_INVALID_POOL_ID;
			}
			if( s_iNumPools >= s_iMaxNumPools )
			{
				return AK_INVALID_POOL_ID;
			}

			AkMemPoolId poolId = 0;
			AkMemPool * pPool = s_pMemPools;
			for( ;; ++pPool )
			{
				pPool->lock.Lock();
				if( !pPool->ulNumBlocks )
				{
					break;
				}
				pPool->lock.Unlock();
				if( ++poolId >= s_iMaxNumPools )
				{
					return AK_INVALID_POOL_ID;
				}
			}

			const AkUInt32 ulMemSize = in_uBlockSize * ( in_uMemSize / in_uBlockSize );
			if( in_pMemAddress )
			{
				pPool->bAllocated = 0;
				pPool->pcMemAddress = (char *)in_pMemAddress;
			}
			else
			{
				if( ulMemSize + in_uBlockAlign )
				{
					pPool->pcAllocAddress = ( eAlloc == AkVirtualAlloc )
						? (char *)AK::VirtualAllocHook( NULL, ulMemSize + in_uBlockAlign, MEM_COMMIT, PAGE_EXECUTE_READWRITE )
						: (char *)AK::AllocHook( ulMemSize + in_uBlockAlign );
				}
				pPool->pcMemAddress = pPool->pcAllocAddress;
				if( !pPool->pcAllocAddress )
				{
					pPool->lock.Unlock();
					return AK_INVALID_POOL_ID;
				}
				if( in_uBlockAlign && (AkUIntPtr)pPool->pcAllocAddress % in_uBlockAlign )
				{
					pPool->pcMemAddress = pPool->pcAllocAddress + in_uBlockAlign - (AkUIntPtr)pPool->pcAllocAddress % in_uBlockAlign;
				}
				pPool->bAllocated = 1;
			}

			pPool->eAttributes = in_eAttributes;
			pPool->ulTotalAvailable = ulMemSize;
			if( in_eAttributes & AkFixedSizeBlocksMode )
			{
				for( char * pBlock = pPool->pcMemAddress; pBlock != pPool->pcMemAddress + ulMemSize; pBlock += in_uBlockSize )
				{
					pPool->listBuffers.AddLast( (AkLinkedBuffer *)pBlock );
				}
			}
			else
			{
				pPool->pTlsfPool = tlsf_create( pPool->pcMemAddress, ulMemSize );
				if( !pPool->pTlsfPool )
				{
					DeallocatePoolMemory( *pPool );
					pPool->lock.Unlock();
					return AK_INVALID_POOL_ID;
				}
				pPool->ulTotalAvailable -= (AkUInt32)tlsf_overhead();
			}

			pPool->ulNumBlocks = in_uMemSize / in_uBlockSize;
			pPool->ulBlockSize = in_uBlockSize;
			pPool->ulAlign = in_uBlockAlign;
			++s_iNumPools;
			pPool->lock.Unlock();
			return poolId;
		}

		AKRESULT DestroyPool( AkMemPoolId in_poolId )
		{
			const AKRESULT eResult = CheckPoolId( in_poolId );
			if( eResult != AK_Success )
			{
				return eResult;
			}
			AkMemPool & pool = s_pMemPools[ in_poolId ];
			AkAutoLock<CAkLock> gate( pool.lock );
			if( pool.eAttributes & AkFixedSizeBlocksMode )
			{
				pool.listBuffers.RemoveAll();
			}
			if( pool.bAllocated )
			{
				DeallocatePoolMemory( pool );
			}
			pool.Init();
			--s_iNumPools;
			return AK_Success;
		}
	}
}
