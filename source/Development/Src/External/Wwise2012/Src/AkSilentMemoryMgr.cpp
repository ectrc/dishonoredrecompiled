/*=============================================================================
	AkSilentMemoryMgr.cpp - AK::MemoryMgr on the game's own allocator.

	DISHONORED(bringup): Wwise's real pool manager carves fixed-size block pools out of one reservation
	and hands them to the lower engine. The backend has no lower engine, so every pool is a bookkeeping
	record and every allocation goes straight to AK::AllocHook / AK::FreeHook, which AkAudio routes to
	appMalloc / appFree (retail does the same for the pool reservations: the 2012 PDB attributes
	AK::FreeHook / AK::VirtualAllocHook / AK::VirtualFreeHook to AkAudio/src/akaudiodevice.cpp).
	Pool ids are handed out from 0 upwards, and AK_DEFAULT_POOL_ID (-1) is accepted everywhere as
	"the default pool" like the SDK does.
=============================================================================*/
#include "AkSilentInternal.h"

#include <AK/SoundEngine/Common/AkMemoryMgr.h>

namespace
{
	struct SilentPool
	{
		bool		bInUse;
		AkUInt32	uBlockSize;
		AkUInt32	uAttributes;
		AkUInt32	uAllocs;
		AkUInt32	uUsed;
		char		szName[64];
	};

	AkUInt32 g_uMaxPools = 0;
	SilentPool * g_pPools = NULL;
	bool g_bInitialised = false;

	SilentPool * PoolFor( AkMemPoolId id )
	{
		if( id < 0 || (AkUInt32)id >= g_uMaxPools || !g_pPools )
		{
			return NULL;
		}
		return &g_pPools[ id ];
	}
}

namespace AK
{
	namespace MemoryMgr
	{
		AKRESULT Init( AkMemSettings * in_pSettings )
		{
			if( g_bInitialised )
			{
				return AK_Success;
			}
			g_uMaxPools = ( in_pSettings && in_pSettings->uMaxNumPools ) ? in_pSettings->uMaxNumPools : 32;
			g_pPools = (SilentPool *)AK::AllocHook( sizeof( SilentPool ) * g_uMaxPools );
			if( !g_pPools )
			{
				return AK_InsufficientMemory;
			}
			memset( g_pPools, 0, sizeof( SilentPool ) * g_uMaxPools );
			g_bInitialised = true;
			AkSilent::Get().bMemoryMgrInit = true;
			AkSilent::Logf( L"MemoryMgr::Init: %u pools (silent backend, allocations go to appMalloc)", g_uMaxPools );
			return AK_Success;
		}

		void GetDefaultSettings( AkMemSettings & out_settings )
		{
			out_settings.uMaxNumPools = 32;
		}

		bool IsInitialized()
		{
			return g_bInitialised;
		}

		void Term()
		{
			if( !g_bInitialised )
			{
				return;
			}
			AK::FreeHook( g_pPools );
			g_pPools = NULL;
			g_uMaxPools = 0;
			g_bInitialised = false;
			AkSilent::Get().bMemoryMgrInit = false;
			AkSilent::Logf( L"MemoryMgr::Term" );
		}

		AKRESULT InitBase( AkMemPoolId /*in_poolId*/ )
		{
			return g_bInitialised ? AK_Success : AK_MemManagerNotInitialized;
		}

		AkInt32 GetNumPools()
		{
			AkInt32 count = 0;
			for( AkUInt32 i = 0; i < g_uMaxPools; ++i )
			{
				count += g_pPools[ i ].bInUse ? 1 : 0;
			}
			return count;
		}

		AkInt32 GetMaxPools()
		{
			return (AkInt32)g_uMaxPools;
		}

		AkMemPoolId CreatePool( void * /*in_pMemAddress*/, AkUInt32 /*in_uMemSize*/, AkUInt32 in_uBlockSize, AkUInt32 in_eAttributes, AkUInt32 /*in_uBlockAlign*/ )
		{
			for( AkUInt32 i = 0; i < g_uMaxPools; ++i )
			{
				if( !g_pPools[ i ].bInUse )
				{
					g_pPools[ i ].bInUse = true;
					g_pPools[ i ].uBlockSize = in_uBlockSize;
					g_pPools[ i ].uAttributes = in_eAttributes;
					g_pPools[ i ].uAllocs = 0;
					g_pPools[ i ].uUsed = 0;
					g_pPools[ i ].szName[ 0 ] = '\0';
					return (AkMemPoolId)i;
				}
			}
			return AK_INVALID_POOL_ID;
		}

		AKRESULT DestroyPool( AkMemPoolId in_poolId )
		{
			SilentPool * pool = PoolFor( in_poolId );
			if( !pool || !pool->bInUse )
			{
				return AK_InvalidParameter;
			}
			const AKRESULT result = pool->uAllocs ? AK_MemoryLeak : AK_Success;
			pool->bInUse = false;
			return result;
		}

		void DeallocatePool( AkMemPoolId in_poolId )
		{
			DestroyPool( in_poolId );
		}

		AKRESULT SetPoolName( AkMemPoolId in_poolId, const char * in_pszPoolName )
		{
			SilentPool * pool = PoolFor( in_poolId );
			if( !pool )
			{
				return AK_InvalidParameter;
			}
			if( in_pszPoolName )
			{
				strncpy_s( pool->szName, in_pszPoolName, 63 );
			}
			return AK_Success;
		}

		AKRESULT CheckPoolId( AkMemPoolId in_poolId )
		{
			if( in_poolId == AK_DEFAULT_POOL_ID )
			{
				return AK_Success;
			}
			SilentPool * pool = PoolFor( in_poolId );
			return ( pool && pool->bInUse ) ? AK_Success : AK_InvalidParameter;
		}

		AkMemPoolAttributes GetPoolAttributes( AkMemPoolId in_poolId )
		{
			SilentPool * pool = PoolFor( in_poolId );
			return pool ? (AkMemPoolAttributes)pool->uAttributes : AkNoAlloc;
		}

		AkUInt32 GetBlockSize( AkMemPoolId in_poolId )
		{
			SilentPool * pool = PoolFor( in_poolId );
			return pool ? pool->uBlockSize : 0;
		}

		void * Malloc( AkMemPoolId in_poolId, AkUInt32 in_uSize )
		{
			SilentPool * pool = PoolFor( in_poolId );
			if( pool )
			{
				++pool->uAllocs;
				pool->uUsed += in_uSize;
			}
			return AK::AllocHook( in_uSize );
		}

		void * Malign( AkMemPoolId in_poolId, AkUInt32 in_uSize, AkUInt32 /*in_uAlignment*/ )
		{
			// The hook is appMalloc, whose alignment already covers every Wwise request on Win32.
			return Malloc( in_poolId, in_uSize );
		}

		AKRESULT Free( AkMemPoolId in_poolId, void * in_pMemAddress )
		{
			SilentPool * pool = PoolFor( in_poolId );
			if( pool && pool->uAllocs )
			{
				--pool->uAllocs;
			}
			AK::FreeHook( in_pMemAddress );
			return AK_Success;
		}

		void * GetBlock( AkMemPoolId in_poolId )
		{
			SilentPool * pool = PoolFor( in_poolId );
			return pool ? Malloc( in_poolId, pool->uBlockSize ) : NULL;
		}

		AKRESULT ReleaseBlock( AkMemPoolId in_poolId, void * in_pMemAddress )
		{
			return Free( in_poolId, in_pMemAddress );
		}

		AKRESULT GetPoolStats( AkMemPoolId in_poolId, PoolStats & out_stats )
		{
			memset( &out_stats, 0, sizeof( out_stats ) );
			SilentPool * pool = PoolFor( in_poolId );
			if( !pool )
			{
				return AK_InvalidParameter;
			}
			out_stats.uUsed = pool->uUsed;
			out_stats.uPeakUsed = pool->uUsed;
			out_stats.uAllocs = pool->uAllocs;
			return AK_Success;
		}

		void GetPoolMemoryUsed( AkMemPoolId in_poolId, PoolMemInfo & out_memInfo )
		{
			memset( &out_memInfo, 0, sizeof( out_memInfo ) );
			SilentPool * pool = PoolFor( in_poolId );
			if( pool )
			{
				out_memInfo.uUsed = pool->uUsed;
			}
		}
	}
}
