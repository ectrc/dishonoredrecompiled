/*=============================================================================
	AkMemoryMgr.h - Wwise 2012.1 pool allocator surface.

	DISHONORED(written): every declaration mirrors a demangled 2012 PDB signature
	(resources/docs/symbols/functions.csv), 2012 rvas in the comments. PoolStats and PoolMemInfo layouts
	are the 2012 PDB's (AK::MemoryMgr::PoolStats size=24, AK::MemoryMgr::PoolMemInfo size=8); their
	member names are not in the type dump beyond the sizes, so the fields carry the SDK's documented
	meaning and are marked as such - nothing in Dishonored reads them.
=============================================================================*/
#ifndef _AK_MEMORYMGR_H_
#define _AK_MEMORYMGR_H_

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkModule.h>

namespace AK
{
	namespace MemoryMgr
	{
		/** DISHONORED(layout): 24 bytes (2012 PDB AK::MemoryMgr::PoolStats). Field names: SDK contract. */
		struct PoolStats
		{
			AkUInt32 uReserved;
			AkUInt32 uUsed;
			AkUInt32 uMaxFreeBlock;
			AkUInt32 uAllocs;
			AkUInt32 uFrees;
			AkUInt32 uPeakUsed;
		};

		/** DISHONORED(layout): 8 bytes (2012 PDB AK::MemoryMgr::PoolMemInfo). Field names: SDK contract. */
		struct PoolMemInfo
		{
			AkUInt32 uReserved;
			AkUInt32 uUsed;
		};

		bool IsInitialized();												///< 2012 rva 0x960620
		void Term();														///< 2012 rva 0x9609f0
		AKRESULT InitBase( AkMemPoolId in_poolId );							///< 2012 rva 0x9606f0
		AkInt32 GetNumPools();												///< 2012 rva 0x9606a0
		AkInt32 GetMaxPools();												///< 2012 rva 0x9606b0

		AkMemPoolId CreatePool( void * in_pMemAddress, AkUInt32 in_uMemSize, AkUInt32 in_uBlockSize, AkUInt32 in_eAttributes, AkUInt32 in_uBlockAlign = 0 );	///< 2012 rva 0x960af0
		AKRESULT DestroyPool( AkMemPoolId in_poolId );						///< 2012 rva 0x960cc0
		void DeallocatePool( AkMemPoolId in_poolId );						///< 2012 rva 0x960aa0
		AKRESULT SetPoolName( AkMemPoolId in_poolId, const char * in_pszPoolName );	///< 2012 rva 0x960630
		AKRESULT CheckPoolId( AkMemPoolId in_poolId );						///< 2012 rva 0x9606c0
		AkMemPoolAttributes GetPoolAttributes( AkMemPoolId in_poolId );		///< 2012 rva 0x960660
		AkUInt32 GetBlockSize( AkMemPoolId in_poolId );						///< 2012 rva 0x960680

		void * Malloc( AkMemPoolId in_poolId, AkUInt32 in_uSize );			///< 2012 rva 0x960790
		void * Malign( AkMemPoolId in_poolId, AkUInt32 in_uSize, AkUInt32 in_uAlignment );	///< 2012 rva 0x960800
		AKRESULT Free( AkMemPoolId in_poolId, void * in_pMemAddress );		///< 2012 rva 0x960860
		void * GetBlock( AkMemPoolId in_poolId );							///< 2012 rva 0x960950
		AKRESULT ReleaseBlock( AkMemPoolId in_poolId, void * in_pMemAddress );	///< 2012 rva 0x9609a0

		AKRESULT GetPoolStats( AkMemPoolId in_poolId, PoolStats & out_stats );	///< 2012 rva 0x9608b0
		void GetPoolMemoryUsed( AkMemPoolId in_poolId, PoolMemInfo & out_memInfo );	///< 2012 rva 0x960910
	}
}

#endif // _AK_MEMORYMGR_H_
