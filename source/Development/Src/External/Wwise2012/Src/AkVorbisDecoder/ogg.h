/*=============================================================================
	ogg.h - the bit-packer records and the bump allocator of Wwise's Vorbis decoder (Tremor lowmem, packed variant).

	DISHONORED(port): agent IH2 (wave 26). Layouts are the 2012 PDB's (oggpack_buffer 12, ogg_buffer 8, ogg_packet 12,
	CAkVorbisAllocator 16 bytes). CAkVorbisAllocator::Alloc is inlined at every call site in retail (size rounded up to 4,
	NULL when zero or when the block is full); Calloc is the call sites that memset the block afterwards.
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <string.h>

extern AkMemPoolId g_LEngineDefaultPoolId;

struct oggpack_buffer
{
	int headbit;
	unsigned char * headptr;
	int headend;
};

struct ogg_buffer
{
	unsigned char * data;
	int size;
};

struct ogg_packet
{
	ogg_buffer buffer;
	unsigned char e_o_s;
};

class CAkVorbisAllocator
{
public:
	void * Alloc( AkUInt32 in_uSize )
	{
		if( !in_uSize )
		{
			return NULL;
		}
		const AkUInt32 uAligned = ( in_uSize + 3 ) & ~3;
		if( CurrentSize + uAligned > MaxSize )
		{
			return NULL;
		}
		void * pBlock = pCurrentAddress;
		pCurrentAddress += uAligned;
		CurrentSize += uAligned;
		return pBlock;
	}

	void * Calloc( AkUInt32 in_uSize )
	{
		void * pBlock = Alloc( in_uSize );
		if( pBlock )
		{
			memset( pBlock, 0, in_uSize );
		}
		return pBlock;
	}

	/** 2012 0x976b60; 2013 0x973340. */
	void Term()
	{
		if( pStartAddress )
		{
			AK::MemoryMgr::Free( g_LEngineDefaultPoolId, pStartAddress );
			pCurrentAddress = NULL;
			CurrentSize = 0;
			MaxSize = 0;
			pStartAddress = NULL;
		}
	}

	unsigned char * pStartAddress;
	unsigned char * pCurrentAddress;
	AkUInt32 CurrentSize;
	AkUInt32 MaxSize;
};

static_assert( sizeof( oggpack_buffer ) == 12, "oggpack_buffer: 2012 PDB sizeof 12" );
static_assert( sizeof( ogg_packet ) == 12, "ogg_packet: 2012 PDB sizeof 12" );
static_assert( sizeof( CAkVorbisAllocator ) == 16, "CAkVorbisAllocator: 2012 PDB sizeof 16" );

long oggpack_look( oggpack_buffer * b, int bits );
void oggpack_adv( oggpack_buffer * b, int bits );
long oggpack_read( oggpack_buffer * b, int bits );
