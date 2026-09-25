/*=============================================================================
	Pool.h: fixed-size element pools (Arkane addition to Core).

	Reconstructed from the 2012 Shipping PDB/decompile (core\inc\pool.h):
	TPoolRaw<Policy> (56 bytes) keeps a singly linked list of chunks, each
	chunk holding N elements of m_iElementAlignedSize bytes with an intrusive
	free list threaded through the free elements. TPool<T, Policy> adds the
	element type.

	Policies:
	  TGrowablePoolPolicy            chunks are kept in a doubly linked list of
	                                 chunks with free elements sorted by their
	                                 free count; a chunk that becomes empty is
	                                 released; no locking
	  TGrowablePoolThreadSafePolicy  same, every request under
	                                 m_RequestCriticalSection
	  TGrowOnlyPoolPolicy            one global free list (m_pFirstFreeElement),
	                                 chunks are never released; no locking

	Known users (2012 exe): AllocateObjectFromPool/DestroyObjectFromPool in
	UnObj.cpp (one TPoolRaw<TGrowablePoolPolicy> per UObject size, keyed by
	Align(PropertiesSize,8), sized from s_ObjectPoolParams), Engine's
	g_PrimitiveSceneInfoPool (4000/100), s_LightPrimitiveInteractionPool
	(5000/500), TStaticMeshDrawList<>::s_ElementHandlePool,
	s_StaticMeshPool (thread safe, 2000/500) and g_DisJobQueueAsyncJobPool
	(grow only, 100/20).
=============================================================================*/

#pragma once

class TGrowablePoolPolicy
{
public:
	enum { bThreadSafe = FALSE, bGrowOnly = FALSE };
};

class TGrowablePoolThreadSafePolicy
{
public:
	enum { bThreadSafe = TRUE, bGrowOnly = FALSE };
};

class TGrowOnlyPoolPolicy
{
public:
	enum { bThreadSafe = FALSE, bGrowOnly = TRUE };
};

template<class Policy>
class TPoolRaw
{
public:
	/**
	 * @param iSizeOfElement         size of one element; must hold a pointer (the free list link) and should be a multiple of 8
	 * @param iInitialNbrOfElements  elements of the first chunk, allocated right away when > 0
	 * @param iGrowNbrOfElements     elements of every further chunk
	 */
	TPoolRaw( INT iSizeOfElement, INT iInitialNbrOfElements, INT iGrowNbrOfElements );
	~TPoolRaw();

	void* AllocateRawBuffer();
	void DestroyRawBuffer( void* pObject );

	INT GetElementSize() const
	{
		return m_iElementAlignedSize;
	}

protected:
	struct Chunk
	{
		Chunk* m_pNext;
		Chunk* m_pNextChunkWithFreeElements;
		Chunk* m_pPrevChunkWithFreeElements;
		BYTE* m_pRawElements;
		BYTE* m_pRawEndElements;
		INT m_iNbrFreeElements;
		INT m_iNbrElements;
		void* m_pFirstFreeElementInChunk;

		UBOOL Contains( const void* pObject ) const
		{
			return (const BYTE*)pObject >= m_pRawElements && (const BYTE*)pObject < m_pRawEndElements;
		}
	};

	void AllocateNewChunk( INT iNbrElements );
	void AllocateNewChunkOnDemand();
	Chunk* FindChunk( const void* pObject ) const;
	void ReleaseChunk( Chunk* pChunk );
	void MoveChunkTowardsLargerFreeCounts( Chunk* pChunk );

	void Lock()
	{
		if( Policy::bThreadSafe )
		{
			m_RequestCriticalSection.Lock();
		}
	}

	void Unlock()
	{
		if( Policy::bThreadSafe )
		{
			m_RequestCriticalSection.Unlock();
		}
	}

	static void*& NextFreeElement( void* pElement )
	{
		return *(void**)pElement;
	}

	void* m_pFirstFreeElement;
	Chunk* m_pFirstChunkWithFreeElement;
	Chunk* m_pLastChunkWithFreeElement;
	INT m_iElementAlignedSize;
	INT m_iGrowNbrOfElements;
	INT m_iInitialNbrOfElements;
	Chunk* m_pFirstChunk;
	FCriticalSection m_RequestCriticalSection;
};

template<class T, class Policy = TGrowablePoolPolicy>
class TPool : public TPoolRaw<Policy>
{
public:
	TPool( INT iInitialNbrOfElements = 0, INT iGrowNbrOfElements = 0 )
	:	TPoolRaw<Policy>( (INT)Align( sizeof(T), 8 ), iInitialNbrOfElements, iGrowNbrOfElements )
	{}

	T* Allocate()
	{
		return new( this->AllocateRawBuffer() ) T();
	}

	void Destroy( T* pObject )
	{
		pObject->~T();
		this->DestroyRawBuffer( pObject );
	}
};

template<class Policy>
TPoolRaw<Policy>::TPoolRaw( INT iSizeOfElement, INT iInitialNbrOfElements, INT iGrowNbrOfElements )
:	m_pFirstFreeElement( NULL )
,	m_pFirstChunkWithFreeElement( NULL )
,	m_pLastChunkWithFreeElement( NULL )
,	m_iGrowNbrOfElements( iGrowNbrOfElements )
,	m_iInitialNbrOfElements( iInitialNbrOfElements )
,	m_pFirstChunk( NULL )
{
	checkSlow( iSizeOfElement >= (INT)sizeof(void*) );
	m_iElementAlignedSize = iSizeOfElement;
	if( iInitialNbrOfElements > 0 )
	{
		AllocateNewChunk( iInitialNbrOfElements );
	}
}

template<class Policy>
TPoolRaw<Policy>::~TPoolRaw()
{
	while( m_pFirstChunk )
	{
		Chunk* pNext = m_pFirstChunk->m_pNext;
		appFree( m_pFirstChunk );
		m_pFirstChunk = pNext;
	}
}

template<class Policy>
void TPoolRaw<Policy>::AllocateNewChunk( INT iNbrElements )
{
	check( iNbrElements > 0 );
	Chunk* pChunk = (Chunk*)appMalloc( sizeof(Chunk) + iNbrElements * m_iElementAlignedSize, 8 );
	pChunk->m_pNext = m_pFirstChunk;
	pChunk->m_pNextChunkWithFreeElements = NULL;
	pChunk->m_pPrevChunkWithFreeElements = NULL;
	pChunk->m_pRawElements = Align( (BYTE*)(pChunk + 1), 8 );
	pChunk->m_pRawEndElements = pChunk->m_pRawElements + iNbrElements * m_iElementAlignedSize;
	pChunk->m_iNbrFreeElements = iNbrElements;
	pChunk->m_iNbrElements = iNbrElements;
	pChunk->m_pFirstFreeElementInChunk = pChunk->m_pRawElements;
	m_pFirstChunk = pChunk;

	for( INT i = 0; i < iNbrElements - 1; i++ )
	{
		NextFreeElement( pChunk->m_pRawElements + i * m_iElementAlignedSize ) = pChunk->m_pRawElements + (i + 1) * m_iElementAlignedSize;
	}
	NextFreeElement( pChunk->m_pRawElements + (iNbrElements - 1) * m_iElementAlignedSize ) = NULL;

	// only ever called when no free element is left, so the free lists restart at this chunk
	m_pFirstFreeElement = pChunk->m_pRawElements;
	m_pFirstChunkWithFreeElement = pChunk;
	m_pLastChunkWithFreeElement = pChunk;
}

template<class Policy>
void TPoolRaw<Policy>::AllocateNewChunkOnDemand()
{
	if( m_pFirstChunk == NULL && m_iInitialNbrOfElements != 0 )
	{
		AllocateNewChunk( m_iInitialNbrOfElements );
	}
	else
	{
		AllocateNewChunk( m_iGrowNbrOfElements );
	}
}

template<class Policy>
typename TPoolRaw<Policy>::Chunk* TPoolRaw<Policy>::FindChunk( const void* pObject ) const
{
	Chunk* pChunk = m_pFirstChunk;
	while( pChunk && !pChunk->Contains( pObject ) )
	{
		pChunk = pChunk->m_pNext;
	}
	return pChunk;
}

template<class Policy>
void* TPoolRaw<Policy>::AllocateRawBuffer()
{
	if( Policy::bGrowOnly )
	{
		while( m_pFirstFreeElement == NULL )
		{
			AllocateNewChunkOnDemand();
		}
		void* pElement = m_pFirstFreeElement;
		m_pFirstFreeElement = NextFreeElement( pElement );
		return pElement;
	}

	while( 1 )
	{
		Lock();
		if( m_pFirstChunkWithFreeElement )
		{
			break;
		}
		AllocateNewChunkOnDemand();
		Unlock();
	}

	Chunk* pChunk = m_pFirstChunkWithFreeElement;
	void* pElement = pChunk->m_pFirstFreeElementInChunk;
	pChunk->m_pFirstFreeElementInChunk = NextFreeElement( pElement );
	pChunk->m_iNbrFreeElements--;

	if( pChunk->m_pFirstFreeElementInChunk == NULL )
	{
		// the chunk is full: leave the list of chunks with free elements
		m_pFirstChunkWithFreeElement = pChunk->m_pNextChunkWithFreeElements;
		if( m_pLastChunkWithFreeElement == pChunk )
		{
			m_pLastChunkWithFreeElement = NULL;
		}
		pChunk->m_pNextChunkWithFreeElements = NULL;
		pChunk->m_pPrevChunkWithFreeElements = NULL;
		if( m_pFirstChunkWithFreeElement )
		{
			m_pFirstChunkWithFreeElement->m_pPrevChunkWithFreeElements = NULL;
		}
	}
	else
	{
		// keep the list sorted by ascending free count so the emptiest chunks fill up first
		Chunk* pNext = pChunk->m_pNextChunkWithFreeElements;
		if( pNext && pChunk->m_iNbrFreeElements > pNext->m_iNbrFreeElements )
		{
			m_pFirstChunkWithFreeElement = pNext;
			pChunk->m_pNextChunkWithFreeElements = pNext->m_pNextChunkWithFreeElements;
			pChunk->m_pPrevChunkWithFreeElements = pNext;
			pNext->m_pNextChunkWithFreeElements = pChunk;
			pNext->m_pPrevChunkWithFreeElements = NULL;
			// DISHONORED: the original left the back link of the chunk now following pChunk pointing at pNext
			if( pChunk->m_pNextChunkWithFreeElements )
			{
				pChunk->m_pNextChunkWithFreeElements->m_pPrevChunkWithFreeElements = pChunk;
			}
			if( m_pLastChunkWithFreeElement == pNext )
			{
				m_pLastChunkWithFreeElement = pChunk;
			}
		}
	}

	Unlock();
	return pElement;
}

template<class Policy>
void TPoolRaw<Policy>::ReleaseChunk( Chunk* pChunk )
{
	Chunk* pNext = pChunk->m_pNext;
	Chunk* pPrevFree = pChunk->m_pPrevChunkWithFreeElements;
	Chunk* pNextFree = pChunk->m_pNextChunkWithFreeElements;
	appFree( pChunk );

	if( m_pFirstChunk == pChunk )
	{
		m_pFirstChunk = pNext;
	}
	else
	{
		Chunk* pPrev = m_pFirstChunk;
		while( pPrev->m_pNext != pChunk )
		{
			pPrev = pPrev->m_pNext;
		}
		pPrev->m_pNext = pNext;
	}

	if( m_pFirstChunkWithFreeElement == pChunk )
	{
		m_pFirstChunkWithFreeElement = pNextFree;
	}
	if( m_pLastChunkWithFreeElement == pChunk )
	{
		m_pLastChunkWithFreeElement = pPrevFree;
	}
	if( pNextFree )
	{
		pNextFree->m_pPrevChunkWithFreeElements = pPrevFree;
	}
	if( pPrevFree )
	{
		pPrevFree->m_pNextChunkWithFreeElements = pNextFree;
	}
}

template<class Policy>
void TPoolRaw<Policy>::MoveChunkTowardsLargerFreeCounts( Chunk* pChunk )
{
	while( pChunk->m_pNextChunkWithFreeElements && pChunk->m_iNbrFreeElements > pChunk->m_pNextChunkWithFreeElements->m_iNbrFreeElements )
	{
		Chunk* pNext = pChunk->m_pNextChunkWithFreeElements;
		Chunk* pPrev = pChunk->m_pPrevChunkWithFreeElements;
		pChunk->m_pNextChunkWithFreeElements = pNext->m_pNextChunkWithFreeElements;
		pNext->m_pPrevChunkWithFreeElements = pPrev;
		pChunk->m_pPrevChunkWithFreeElements = pNext;
		pNext->m_pNextChunkWithFreeElements = pChunk;
		if( pPrev )
		{
			pPrev->m_pNextChunkWithFreeElements = pNext;
		}
		if( pChunk->m_pNextChunkWithFreeElements )
		{
			pChunk->m_pNextChunkWithFreeElements->m_pPrevChunkWithFreeElements = pChunk;
		}
		if( m_pFirstChunkWithFreeElement == pChunk )
		{
			m_pFirstChunkWithFreeElement = pNext;
		}
		if( m_pLastChunkWithFreeElement == pNext )
		{
			m_pLastChunkWithFreeElement = pChunk;
		}
	}
}

template<class Policy>
void TPoolRaw<Policy>::DestroyRawBuffer( void* pObject )
{
	if( Policy::bGrowOnly )
	{
		NextFreeElement( pObject ) = m_pFirstFreeElement;
		m_pFirstFreeElement = pObject;
		return;
	}

	Lock();

	Chunk* pChunk = FindChunk( pObject );
	check( pChunk );
	NextFreeElement( pObject ) = pChunk->m_pFirstFreeElementInChunk;
	pChunk->m_pFirstFreeElementInChunk = pObject;
	pChunk->m_iNbrFreeElements++;

	if( pChunk->m_iNbrFreeElements == pChunk->m_iNbrElements )
	{
		ReleaseChunk( pChunk );
	}
	else if( pChunk->m_iNbrFreeElements == 1 )
	{
		// the chunk was full: it becomes the head of the list of chunks with free elements
		pChunk->m_pNextChunkWithFreeElements = m_pFirstChunkWithFreeElement;
		if( m_pFirstChunkWithFreeElement )
		{
			m_pFirstChunkWithFreeElement->m_pPrevChunkWithFreeElements = pChunk;
		}
		else
		{
			m_pLastChunkWithFreeElement = pChunk;
		}
		m_pFirstChunkWithFreeElement = pChunk;
	}
	else
	{
		MoveChunkTowardsLargerFreeCounts( pChunk );
	}

	Unlock();
}
