/*=============================================================================
	AkVorbisCodebookMgr.cpp - one decoded setup header per uHashCodebook, shared by refcount (akvorbiscodebookmgr.cpp).

	DISHONORED(port): agent IH2 (wave 26), from the 2012 decompile. Retail inlines the hash lookups: Decodebook walks the
	bucket itself, ReleaseCodebook calls the AkHashListBare instantiations FindEx (2012 0x976ba0; 2013 0x973380) and Erase
	(2012 0x976c40; 2013 0x973420); Set (2012 0x976c00; 2013 0x9733e0) is AkHashListBare::Set. The PBI argument of
	Decodebook is unused in retail (no call reads it). g_VorbisCodebookMgr is a zero-initialised global as retail's
	(2012 .data 0x10123b8; its only dynamic initialiser 0xbab220 registers the atexit destructor, which has nothing to do).
=============================================================================*/
#include "AkVorbisCodebookMgr.h"

AkVorbisCodebookMgr g_VorbisCodebookMgr;

/** 2012 0x976d30; 2013 0x973510. */
CAkVorbisAllocator * AkVorbisCodebookMgr::Decodebook( AkVorbisSourceState & in_VorbisState, CAkPBI * in_pPBI, ogg_packet * op )
{
	const AkUInt32 uHash = in_VorbisState.VorbisInfo.uHashCodebook;
	for( Codebook * pBook = m_codebooks.m_table[ uHash % 31 ]; pBook; pBook = pBook->pNextItem )
	{
		if( pBook->key == uHash )
		{
			++pBook->cRef;
			return &pBook->allocator;
		}
	}

	Codebook * pBook = (Codebook *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, sizeof( Codebook ) );
	if( !pBook )
	{
		return NULL;
	}
	pBook->allocator.pStartAddress = NULL;
	pBook->allocator.pCurrentAddress = NULL;
	pBook->allocator.CurrentSize = 0;
	pBook->allocator.MaxSize = 0;
	pBook->cRef = 0;
	int channels = 0;
	for( AkUInt32 uMask = in_VorbisState.TremorInfo.uChannelMask; uMask; uMask &= uMask - 1 )
	{
		++channels;
	}
	const AkUInt32 uAllocSize = in_VorbisState.VorbisInfo.dwDecodeAllocSize;
	pBook->allocator.pStartAddress = (unsigned char *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, uAllocSize );
	pBook->allocator.pCurrentAddress = pBook->allocator.pStartAddress;
	pBook->allocator.MaxSize = uAllocSize;
	if( pBook->allocator.pStartAddress )
	{
		codec_setup_info * csi = (codec_setup_info *)pBook->allocator.Alloc( sizeof( codec_setup_info ) );
		if( !vorbis_info_init( csi, in_VorbisState.VorbisInfo.uBlockSizes[ 0 ], in_VorbisState.VorbisInfo.uBlockSizes[ 1 ] ) )
		{
			oggpack_buffer opb;
			opb.headbit = 0;
			opb.headptr = op->buffer.data;
			opb.headend = op->buffer.size;
			if( !vorbis_unpack_books( csi, channels, &opb, pBook->allocator ) )
			{
				++pBook->cRef;
				pBook->key = uHash;
				m_codebooks.Set( pBook );
				return &pBook->allocator;
			}
		}
	}
	pBook->allocator.Term();
	AK::MemoryMgr::Free( g_LEngineDefaultPoolId, pBook );
	return NULL;
}

/** 2012 0x976cb0; 2013 0x973490. */
void AkVorbisCodebookMgr::ReleaseCodebook( AkVorbisSourceState & in_VorbisState )
{
	const AkUInt32 uHash = in_VorbisState.VorbisInfo.uHashCodebook;
	AkHashListBare<AkUInt32, Codebook, 31>::IteratorEx it;
	it.pTable = m_codebooks.m_table;
	it.uiTable = uHash % 31;
	it.pItem = m_codebooks.m_table[ it.uiTable ];
	it.pPrevItem = NULL;
	while( it.pItem && it.pItem->key != uHash )
	{
		it.pPrevItem = it.pItem;
		it.pItem = it.pItem->pNextItem;
	}
	Codebook * pBook = it.pItem;
	if( !pBook )
	{
		return;
	}
	if( --pBook->cRef <= 0 )
	{
		m_codebooks.Erase( it );
		pBook->allocator.Term();
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, pBook );
	}
}
