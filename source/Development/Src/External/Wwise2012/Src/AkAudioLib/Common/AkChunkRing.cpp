/*=============================================================================
	AkChunkRing.cpp - AkSparseChunkRing (akchunkring.cpp, 2012 0x91bcf0..0x91bf00; 2013 matched by bytes).

	DISHONORED(port): agent IH2 (wave 26). No deviation.
=============================================================================*/
#include "AkChunkRing.h"

static const AkUInt32 kSparseChunkSize = 2048;

/** 2012 0x91bd00. */
AkUInt32 AkSparseChunkRing::GetChunkSize()
{
	return kSparseChunkSize;
}

/** 2012 0x91be10: two blocks up front, at most in_uNumChunks (never fewer than two). */
AKRESULT AkSparseChunkRing::Init( AkUInt32 in_uNumChunks )
{
	m_uMaxChunks = in_uNumChunks < 2 ? 2 : in_uNumChunks;
	for( AkUInt32 i = 0; i < 2; ++i )
	{
		void * pData = AK::MemoryMgr::Malloc( g_DefaultPoolId, kSparseChunkSize );
		if( !pData )
		{
			return AK_InsufficientMemory;
		}
		Block * pBlock = blocks.AddLast();
		if( !pBlock )
		{
			AK::MemoryMgr::Free( g_DefaultPoolId, pData );
			return AK_InsufficientMemory;
		}
		pBlock->pData = pData;
		pBlock->uUsedSize = 0;
	}
	return AK_Success;
}

/** 2012 0x91beb0. */
void AkSparseChunkRing::Term()
{
	for( Block * p = blocks.m_pItems; p != blocks.m_pItemsEnd; ++p )
	{
		AK::MemoryMgr::Free( g_DefaultPoolId, p->pData );
	}
	blocks.Term();
}

/** 2012 0x91bf00: a message never straddles two blocks; a new block is inserted before the reader when the ring is full. */
AKRESULT AkSparseChunkRing::Write( void * in_pData, AkUInt32 in_uSize )
{
	if( kSparseChunkSize - m_uWriteOffset >= in_uSize )
	{
		memcpy( (AkUInt8 *)blocks[ m_uWriteBlock ].pData + m_uWriteOffset, in_pData, in_uSize );
		m_uWriteOffset += in_uSize;
		blocks[ m_uWriteBlock ].uUsedSize = m_uWriteOffset;
		return AK_Success;
	}
	if( m_bBufferFull )
	{
		return AK_InsufficientMemory;
	}
	AkUInt32 uNextBlock = m_uWriteBlock + 1;
	if( uNextBlock >= blocks.Length() )
	{
		uNextBlock = 0;
	}
	void * pData;
	if( m_uReadBlock != uNextBlock )
	{
		pData = blocks[ uNextBlock ].pData;
	}
	else
	{
		if( blocks.Length() >= m_uMaxChunks )
		{
			m_bBufferFull = true;
			return AK_InsufficientMemory;
		}
		pData = AK::MemoryMgr::Malloc( g_DefaultPoolId, kSparseChunkSize );
		if( !pData )
		{
			m_bBufferFull = true;
			return AK_InsufficientMemory;
		}
		Block * pBlock = blocks.Insert( uNextBlock );
		if( !pBlock )
		{
			m_bBufferFull = true;
			AK::MemoryMgr::Free( g_DefaultPoolId, pData );
			return AK_InsufficientMemory;
		}
		pBlock->pData = pData;
		++m_uReadBlock;
	}
	m_uWriteBlock = uNextBlock;
	m_uWriteOffset = in_uSize;
	memcpy( pData, in_pData, in_uSize );
	blocks[ m_uWriteBlock ].uUsedSize = m_uWriteOffset;
	return AK_Success;
}

/** 2012 0x91bd10. */
void * AkSparseChunkRing::BeginRead()
{
	if( m_uReadOffset < blocks[ m_uReadBlock ].uUsedSize )
	{
		return (AkUInt8 *)blocks[ m_uReadBlock ].pData + m_uReadOffset;
	}
	AkUInt32 uNextBlock = m_uReadBlock + 1;
	m_uReadOffset = 0;
	if( uNextBlock >= blocks.Length() )
	{
		uNextBlock = 0;
	}
	m_uReadBlock = uNextBlock;
	return blocks[ uNextBlock ].pData;
}

/** 2012 0x91bd50. */
void * AkSparseChunkRing::BeginReadEx( AkUInt32 & out_uSizeAvail )
{
	const AkUInt32 uUsedSize = blocks[ m_uReadBlock ].uUsedSize;
	if( m_uReadOffset < uUsedSize )
	{
		out_uSizeAvail = uUsedSize - m_uReadOffset;
		return (AkUInt8 *)blocks[ m_uReadBlock ].pData + m_uReadOffset;
	}
	AkUInt32 uNextBlock = m_uReadBlock + 1;
	m_uReadOffset = 0;
	if( uNextBlock >= blocks.Length() )
	{
		uNextBlock = 0;
	}
	m_uReadBlock = uNextBlock;
	out_uSizeAvail = blocks[ uNextBlock ].uUsedSize;
	return blocks[ uNextBlock ].pData;
}

/** 2012 0x91bcf0. */
void AkSparseChunkRing::EndRead( AkUInt32 in_uSize )
{
	m_uReadOffset += in_uSize;
	m_bBufferFull = false;
}
