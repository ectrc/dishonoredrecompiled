/*=============================================================================
	AkChunkRing.h - AkSparseChunkRing, the audio manager's message queue storage (akchunkring.cpp).

	DISHONORED(port): agent IH2 (wave 26). Blocks of kSparseChunkSize (2048, 2012 .rdata 0xe369fc) bytes, at least two,
	grown up to m_uMaxChunks (uCommandQueueSize / 2048) when the writer catches up with the reader.
=============================================================================*/
#pragma once

#include <AkPrivateTypes.h>

class AkSparseChunkRing
{
public:
	struct Block
	{
		void *		pData;
		AkUInt32	uUsedSize;
	};

	AkSparseChunkRing() : m_uMaxChunks( 0 ), m_uReadBlock( 0 ), m_uReadOffset( 0 ), m_uWriteBlock( 0 ), m_uWriteOffset( 0 ), m_bBufferFull( false ) {}

	AKRESULT Init( AkUInt32 in_uNumChunks );
	void Term();
	AKRESULT Write( void * in_pData, AkUInt32 in_uSize );
	void * BeginRead();
	void * BeginReadEx( AkUInt32 & out_uSizeAvail );
	void EndRead( AkUInt32 in_uSize );
	static AkUInt32 GetChunkSize();

	bool IsEmpty() const { return m_uReadBlock == m_uWriteBlock && m_uReadOffset == m_uWriteOffset && !m_bBufferFull; }

	AkArray<Block, const Block &, ArrayPoolDefault, 8>	blocks;
	AkUInt32	m_uMaxChunks;
	AkUInt32	m_uReadBlock;
	AkUInt32	m_uReadOffset;
	AkUInt32	m_uWriteBlock;
	AkUInt32	m_uWriteOffset;
	bool		m_bBufferFull;
};
