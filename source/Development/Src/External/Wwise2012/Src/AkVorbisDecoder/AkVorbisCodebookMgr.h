/*=============================================================================
	AkVorbisCodebookMgr.h - the decoded setup headers shared by every voice of the same media (akvorbiscodebookmgr.cpp).

	DISHONORED(port): agent IH2 (wave 26). A setup header is decoded once per uHashCodebook (the 'vorb' chunk's hash of
	the setup packet) into its own CAkVorbisAllocator block of dwDecodeAllocSize bytes; voices AddRef it through
	Decodebook and drop it through ReleaseCodebook. Layout: AkHashListBare<AkUInt32, Codebook, 31> (2012 PDB).
=============================================================================*/
#pragma once

#include <AkPrivateTypes.h>
#include "AkVorbisCodec.h"

class AkVorbisCodebookMgr
{
public:
	struct Codebook
	{
		AkUInt32 key;
		Codebook * pNextItem;
		CAkVorbisAllocator allocator;
		AkInt32 cRef;
	};

	CAkVorbisAllocator * Decodebook( AkVorbisSourceState & in_VorbisState, CAkPBI * in_pPBI, ogg_packet * op );
	void ReleaseCodebook( AkVorbisSourceState & in_VorbisState );

	AkHashListBare<AkUInt32, Codebook, 31> m_codebooks;
};

static_assert( sizeof( AkVorbisCodebookMgr::Codebook ) == 28, "AkVorbisCodebookMgr::Codebook: Decodebook mallocs 0x1C" );

extern AkVorbisCodebookMgr g_VorbisCodebookMgr;
