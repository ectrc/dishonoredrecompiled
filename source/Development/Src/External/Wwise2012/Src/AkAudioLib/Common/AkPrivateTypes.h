/*=============================================================================
	AkPrivateTypes.h - the Wwise 2012.1 runtime's private building blocks: the lock, the platform helpers and
	the intrusive containers every manager is made of (AkArray, AkListBare, AkListBareLight, AkHashList,
	AkHashListBare, CAkKeyArray, AkSortedKeyArray, CAkList2/CAkKeyList, AkPropBundle, CAkConversionTable).

	DISHONORED(port): agent IH1 (wave 26). These are header templates in the SDK (wwise\sdk\include\ak\tools\
	common\akarray.h, aklistbare.h, ...; wwise\sdk\source\soundengine\akaudiolib\common\akhashlist.h,
	akkeyarray.h, aklist2.h, akpropbundle.h, akconversiontable.h), so the 2012 PDB carries their instantiations
	by file. Each method below follows those instantiations' bytes, e.g.
	  AkArray<...,8>::GrowArray                 2012 0x937dc0 (allocate reserve+grow from the pool, copy, free)
	  AkHashListBare<...,193>::Erase            2012 0x94dd60
	  AkHashList<unsigned long,AkMediaEntry,193>::CreateEntry / Set / Unset / RemoveAll  0x92af90..0x92c600
	  AkSortedKeyArray<...>::BinarySearch / AddNoSetKey  0x92a300 / 0x92aff0
	  CAkKeyArray<CAkUsageSlot*,AkMediaInfo,1>::Set / UnsetSwap  0x92b3d0 / 0x92ae60
	  CAkList2<CAkBankMgr::AkBankQueueItem,...>::AddLast / RemoveAll / Term  0x92aec0 / 0x92af40 / 0x92b470
	  AkPropBundle<float>::SetInitialParams / AkPropBundle<AkPropValue>::AddAkProp / SetAkProp  0x952d40 / 0x9384a0 / 0x938f40
	  CAkConversionTable<AkRTPCGraphPointBase<float>,float>::Set  0x931370
	The memory layout of each container is the PDB's (AkArray = {m_pItems, m_pItemsEnd, m_ulReserved}, ...),
	which is what the class layouts in the other headers rely on.

	Deviation: the pool of every container is ArrayPoolDefault (g_DefaultPoolId) or the one the call site names,
	as retail; there is no profiling build, so the SDK's AKASSERTs are not reproduced.
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include "AkInterpolation.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <string.h>
#include <wchar.h>
#include <new>

extern AkMemPoolId g_DefaultPoolId;

#define AkPlacementNew( _memory ) ::new( _memory )

/** The SDK's CAkLock: one CRITICAL_SECTION (24 bytes, the PDB's CAkLock). */
class CAkLock
{
public:
	CAkLock() { InitializeCriticalSection( &m_csLock ); }
	~CAkLock() { DeleteCriticalSection( &m_csLock ); }
	void Lock() { EnterCriticalSection( &m_csLock ); }
	void Unlock() { LeaveCriticalSection( &m_csLock ); }

	CRITICAL_SECTION m_csLock;
};

template <class LOCK>
class AkAutoLock
{
public:
	explicit AkAutoLock( LOCK & in_rLock ) : m_rLock( in_rLock ) { m_rLock.Lock(); }
	~AkAutoLock() { m_rLock.Unlock(); }
	LOCK & m_rLock;
private:
	AkAutoLock & operator=( const AkAutoLock & );
};

/** AkManualEvent (CAkBankCallbackMgr::m_CallbackActiveEvent): a manual-reset Win32 event. */
class AkManualEvent
{
public:
	AkManualEvent() : m_Event( NULL ) {}
	~AkManualEvent() { if( m_Event ) CloseHandle( m_Event ); }
	AKRESULT Init() { m_Event = CreateEventW( NULL, TRUE, TRUE, NULL ); return m_Event ? AK_Success : AK_Fail; }
	void Wait() { WaitForSingleObject( m_Event, INFINITE ); }
	void Signal() { SetEvent( m_Event ); }
	void Reset() { ResetEvent( m_Event ); }
	HANDLE m_Event;
};


namespace AKPLATFORM
{
	/** 2012 0x916d40 (char) / 0x5f4c00 (wchar_t): copy at most in_uDestMaxNumChars - 1 and terminate. */
	inline void SafeStrCpy( char * in_pDest, const char * in_pSrc, size_t in_uDestMaxNumChars )
	{
		size_t uSize = in_uDestMaxNumChars - 1;
		if( uSize >= strlen( in_pSrc ) + 1 )
		{
			uSize = strlen( in_pSrc ) + 1;
		}
		strncpy_s( in_pDest, in_uDestMaxNumChars, in_pSrc, uSize );
		in_pDest[ uSize ] = 0;
	}

	inline void SafeStrCpy( wchar_t * in_pDest, const wchar_t * in_pSrc, size_t in_uDestMaxNumChars )
	{
		size_t uSize = in_uDestMaxNumChars - 1;
		if( uSize >= wcslen( in_pSrc ) + 1 )
		{
			uSize = wcslen( in_pSrc ) + 1;
		}
		wcsncpy_s( in_pDest, in_uDestMaxNumChars, in_pSrc, uSize );
		in_pDest[ uSize ] = 0;
	}

	/** AkWideCharToChar: WideCharToMultiByte(CP_ACP) into a caller buffer of in_uiOutBufferSize + 1 chars. */
	inline AkInt32 AkWideCharToChar( const wchar_t * in_pszUnicodeString, AkUInt32 in_uiOutBufferSize, char * io_pszAnsiString )
	{
		const int iWritten = WideCharToMultiByte( CP_ACP, 0, in_pszUnicodeString, -1, io_pszAnsiString, (int)in_uiOutBufferSize, NULL, NULL );
		io_pszAnsiString[ in_uiOutBufferSize ] = 0;
		return iWritten;
	}

	inline AkInt32 AkCharToWideChar( const char * in_pszAnsiString, AkUInt32 in_uiOutBufferSize, wchar_t * io_pvUnicodeStringBuffer )
	{
		return MultiByteToWideChar( CP_ACP, 0, in_pszAnsiString, -1, io_pvUnicodeStringBuffer, (int)in_uiOutBufferSize );
	}

	inline void PerformanceCounter( AkInt64 * out_piLastTime ) { QueryPerformanceCounter( (LARGE_INTEGER *)out_piLastTime ); }
	inline void PerformanceFrequency( AkInt64 * out_piFreq ) { QueryPerformanceFrequency( (LARGE_INTEGER *)out_piFreq ); }

	void AkSetThreadName( DWORD in_dwThreadID, const char * in_szThreadName );
	void AkCreateThread( DWORD ( WINAPI * in_pStartRoutine )( LPVOID ), void * in_pParams, const AkThreadProperties & in_threadProperties, AkThread * out_pThread, const char * in_szThreadName );
}

namespace AK
{
	/** QueryPerformanceFrequency / 1000, set by AK::SoundEngine::PreInit (2012 0x919cc0). */
	extern AkReal32 g_fFreqRatio;
}

template <class T_KEY, class T_ITEM>
struct MapStruct
{
	T_KEY	key;
	T_ITEM	item;
};

struct ArrayPoolDefault
{
	static AkMemPoolId Get() { return g_DefaultPoolId; }
};

/** AkArray: m_pItems, m_pItemsEnd, m_ulReserved; TGrowBy 0 means "never grows past Reserve". */
template <class T, class ARG_T, class TAlloc = ArrayPoolDefault, unsigned long TGrowBy = 1>
class AkArray
{
public:
	AkArray() : m_pItems( NULL ), m_pItemsEnd( NULL ), m_ulReserved( 0 ) {}
	~AkArray() {}

	class Iterator
	{
	public:
		T * pItem;
		Iterator & operator++() { ++pItem; return *this; }
		T & operator*() { return *pItem; }
		bool operator==( const Iterator & in_rOp ) const { return pItem == in_rOp.pItem; }
		bool operator!=( const Iterator & in_rOp ) const { return pItem != in_rOp.pItem; }
	};

	Iterator Begin() const { Iterator it; it.pItem = m_pItems; return it; }
	Iterator End() const { Iterator it; it.pItem = m_pItemsEnd; return it; }

	AkUInt32 Length() const { return (AkUInt32)( m_pItemsEnd - m_pItems ); }
	AkUInt32 Reserved() const { return m_ulReserved; }
	bool IsEmpty() const { return m_pItems == m_pItemsEnd; }
	T & operator[]( AkUInt32 uiIndex ) const { return m_pItems[ uiIndex ]; }
	T * Data() const { return m_pItems; }

	AKRESULT Reserve( AkUInt32 in_ulReserve )
	{
		if( in_ulReserve )
		{
			m_pItems = (T *)AK::MemoryMgr::Malloc( TAlloc::Get(), sizeof( T ) * in_ulReserve );
			m_pItemsEnd = m_pItems;
			if( !m_pItems )
			{
				return AK_InsufficientMemory;
			}
			m_ulReserved = in_ulReserve;
		}
		return AK_Success;
	}

	bool GrowArray( AkUInt32 in_uGrowBy = TGrowBy )
	{
		const AkUInt32 ulNewReserve = m_ulReserved + in_uGrowBy;
		T * pNewItems = (T *)AK::MemoryMgr::Malloc( TAlloc::Get(), sizeof( T ) * ulNewReserve );
		if( !pNewItems )
		{
			return false;
		}
		const AkUInt32 cItems = Length();
		if( m_pItems )
		{
			for( AkUInt32 i = 0; i < cItems; ++i )
			{
				AkPlacementNew( pNewItems + i ) T( m_pItems[ i ] );
				m_pItems[ i ].~T();
			}
			AK::MemoryMgr::Free( TAlloc::Get(), m_pItems );
		}
		m_pItems = pNewItems;
		m_pItemsEnd = pNewItems + cItems;
		m_ulReserved = ulNewReserve;
		return true;
	}

	T * AddLast()
	{
		const AkUInt32 cItems = Length();
		if( cItems >= m_ulReserved && ( TGrowBy == 0 || !GrowArray() || cItems >= m_ulReserved ) )
		{
			return NULL;
		}
		T * pEnd = m_pItemsEnd++;
		AkPlacementNew( pEnd ) T;
		return pEnd;
	}

	T * AddLast( ARG_T in_rItem )
	{
		T * pItem = AddLast();
		if( pItem )
		{
			*pItem = in_rItem;
		}
		return pItem;
	}

	T * Insert( AkUInt32 in_uIndex )
	{
		const AkUInt32 cItems = Length();
		if( cItems >= m_ulReserved && ( TGrowBy == 0 || !GrowArray() || cItems >= m_ulReserved ) )
		{
			return NULL;
		}
		T * pEnd = m_pItemsEnd++;
		AkPlacementNew( pEnd ) T;
		for( T * p = pEnd; p > m_pItems + in_uIndex; --p )
		{
			*p = *( p - 1 );
		}
		return m_pItems + in_uIndex;
	}

	T * Exists( ARG_T in_Item ) const
	{
		for( T * p = m_pItems; p != m_pItemsEnd; ++p )
		{
			if( *p == in_Item )
			{
				return p;
			}
		}
		return NULL;
	}

	AKRESULT Remove( ARG_T in_rItem )
	{
		for( T * p = m_pItems; p != m_pItemsEnd; ++p )
		{
			if( *p == in_rItem )
			{
				Erase( (AkUInt32)( p - m_pItems ) );
				return AK_Success;
			}
		}
		return AK_Fail;
	}

	AKRESULT RemoveSwap( ARG_T in_rItem )
	{
		for( T * p = m_pItems; p != m_pItemsEnd; ++p )
		{
			if( *p == in_rItem )
			{
				if( Length() > 1 )
				{
					*p = *( m_pItemsEnd - 1 );
				}
				( --m_pItemsEnd )->~T();
				return AK_Success;
			}
		}
		return AK_Fail;
	}

	void Erase( AkUInt32 in_uIndex )
	{
		for( T * p = m_pItems + in_uIndex; p < m_pItemsEnd - 1; ++p )
		{
			*p = *( p + 1 );
		}
		( --m_pItemsEnd )->~T();
	}

	void RemoveAll()
	{
		for( T * p = m_pItems; p != m_pItemsEnd; ++p )
		{
			p->~T();
		}
		m_pItemsEnd = m_pItems;
	}

	void Term()
	{
		if( m_pItems )
		{
			RemoveAll();
			AK::MemoryMgr::Free( TAlloc::Get(), m_pItems );
			m_pItems = NULL;
			m_pItemsEnd = NULL;
			m_ulReserved = 0;
		}
	}

	T *			m_pItems;
	T *			m_pItemsEnd;
	AkUInt32	m_ulReserved;
};

/** CAkKeyArray: an AkArray of MapStruct, linear search by key (akkeyarray.h). */
template <class T_KEY, class T_ITEM, unsigned long TGrowBy = 1>
class CAkKeyArray : public AkArray< MapStruct<T_KEY, T_ITEM>, const MapStruct<T_KEY, T_ITEM> &, ArrayPoolDefault, TGrowBy >
{
	typedef AkArray< MapStruct<T_KEY, T_ITEM>, const MapStruct<T_KEY, T_ITEM> &, ArrayPoolDefault, TGrowBy > Base;
public:
	MapStruct<T_KEY, T_ITEM> * FindEx( T_KEY in_Key ) const
	{
		for( MapStruct<T_KEY, T_ITEM> * p = this->m_pItems; p != this->m_pItemsEnd; ++p )
		{
			if( p->key == in_Key )
			{
				return p;
			}
		}
		return NULL;
	}

	T_ITEM * Exists( T_KEY in_Key ) const
	{
		MapStruct<T_KEY, T_ITEM> * p = FindEx( in_Key );
		return p ? &p->item : NULL;
	}

	T_ITEM * Set( T_KEY in_Key )
	{
		T_ITEM * pItem = Exists( in_Key );
		if( pItem )
		{
			return pItem;
		}
		MapStruct<T_KEY, T_ITEM> * pStruct = Base::AddLast();
		if( !pStruct )
		{
			return NULL;
		}
		pStruct->key = in_Key;
		return &pStruct->item;
	}

	T_ITEM * Set( T_KEY in_Key, const T_ITEM & in_Item )
	{
		T_ITEM * pItem = Exists( in_Key );
		if( pItem )
		{
			*pItem = in_Item;
			return pItem;
		}
		MapStruct<T_KEY, T_ITEM> * pStruct = Base::AddLast();
		if( !pStruct )
		{
			return NULL;
		}
		pStruct->key = in_Key;
		pStruct->item = in_Item;
		return &pStruct->item;
	}

	void Unset( T_KEY in_Key )
	{
		MapStruct<T_KEY, T_ITEM> * p = FindEx( in_Key );
		if( p )
		{
			Base::Erase( (AkUInt32)( p - this->m_pItems ) );
		}
	}

	void UnsetSwap( T_KEY in_Key )
	{
		MapStruct<T_KEY, T_ITEM> * p = FindEx( in_Key );
		if( p )
		{
			if( Base::Length() > 1 )
			{
				*p = *( this->m_pItemsEnd - 1 );
			}
			--this->m_pItemsEnd;
		}
	}
};

/** AkSortedKeyArray: an AkArray kept sorted by KEYGETTER::Get(item), binary searched (akkeyarray.h). */
template <class T_KEY, class T_ITEM, class U_POOL, class KEYGETTER, unsigned long TGrowBy = 1>
class AkSortedKeyArray : public AkArray<T_ITEM, const T_ITEM &, U_POOL, TGrowBy>
{
	typedef AkArray<T_ITEM, const T_ITEM &, U_POOL, TGrowBy> Base;
public:
	T_ITEM * BinarySearch( T_KEY in_key, bool & out_bFound ) const
	{
		AkInt32 iTop = 0;
		AkInt32 iBottom = (AkInt32)Base::Length() - 1;
		while( iTop <= iBottom )
		{
			const AkInt32 iThis = ( iBottom - iTop ) / 2 + iTop;
			const T_KEY thisKey = KEYGETTER::Get( this->m_pItems[ iThis ] );
			if( in_key < thisKey )
			{
				iBottom = iThis - 1;
			}
			else if( thisKey < in_key )
			{
				iTop = iThis + 1;
			}
			else
			{
				out_bFound = true;
				return this->m_pItems + iThis;
			}
		}
		out_bFound = false;
		return this->m_pItems ? this->m_pItems + iTop : NULL;
	}

	T_ITEM * Exists( T_KEY in_key ) const
	{
		bool bFound;
		T_ITEM * pItem = BinarySearch( in_key, bFound );
		return bFound ? pItem : NULL;
	}

	T_ITEM * AddNoSetKey( T_KEY in_key )
	{
		bool bFound;
		T_ITEM * pItem = BinarySearch( in_key, bFound );
		if( pItem )
		{
			return Base::Insert( (AkUInt32)( pItem - this->m_pItems ) );
		}
		return Base::AddLast();
	}

	void Unset( T_KEY in_key )
	{
		bool bFound;
		T_ITEM * pItem = BinarySearch( in_key, bFound );
		if( bFound )
		{
			Base::Erase( (AkUInt32)( pItem - this->m_pItems ) );
		}
	}
};

/** AkListBareNextItem: the default "pNextItem" member policy of AkListBare. */
template <class T>
struct AkListBareNextItem
{
	static T *& Get( T * in_pItem ) { return in_pItem->pNextItem; }
};

/** AkListBare: m_pFirst, m_pLast, m_ulNumListItems over an intrusive next pointer (aklistbare.h). */
template <class T, class U_NEXTITEM = AkListBareNextItem<T> >
class AkListBare
{
public:
	AkListBare() : m_pFirst( NULL ), m_pLast( NULL ), m_ulNumListItems( 0 ) {}

	struct IteratorEx
	{
		T * pItem;
		T * pPrevItem;
		IteratorEx & operator++() { pPrevItem = pItem; pItem = U_NEXTITEM::Get( pItem ); return *this; }
		T * operator*() const { return pItem; }
		bool operator!=( const IteratorEx & in_rOp ) const { return pItem != in_rOp.pItem; }
	};

	IteratorEx BeginEx() const { IteratorEx it; it.pItem = m_pFirst; it.pPrevItem = NULL; return it; }
	IteratorEx EndEx() const { IteratorEx it; it.pItem = NULL; it.pPrevItem = NULL; return it; }

	void AddFirst( T * in_pItem )
	{
		if( m_pFirst )
		{
			U_NEXTITEM::Get( in_pItem ) = m_pFirst;
			m_pFirst = in_pItem;
		}
		else
		{
			m_pLast = in_pItem;
			m_pFirst = in_pItem;
			U_NEXTITEM::Get( in_pItem ) = NULL;
		}
		++m_ulNumListItems;
	}

	void AddLast( T * in_pItem )
	{
		U_NEXTITEM::Get( in_pItem ) = NULL;
		if( m_pLast )
		{
			U_NEXTITEM::Get( m_pLast ) = in_pItem;
		}
		else
		{
			m_pFirst = in_pItem;
		}
		m_pLast = in_pItem;
		++m_ulNumListItems;
	}

	AKRESULT Remove( T * in_pItem )
	{
		T * pPrev = NULL;
		for( T * p = m_pFirst; p; p = U_NEXTITEM::Get( p ) )
		{
			if( p == in_pItem )
			{
				T * pNext = U_NEXTITEM::Get( p );
				if( p == m_pFirst )
				{
					m_pFirst = pNext;
				}
				else
				{
					U_NEXTITEM::Get( pPrev ) = pNext;
				}
				if( p == m_pLast )
				{
					m_pLast = pPrev;
				}
				--m_ulNumListItems;
				return AK_Success;
			}
			pPrev = p;
		}
		return AK_Fail;
	}

	T * RemoveFirst()
	{
		T * pItem = m_pFirst;
		if( pItem )
		{
			if( U_NEXTITEM::Get( pItem ) )
			{
				m_pFirst = U_NEXTITEM::Get( pItem );
			}
			else
			{
				m_pFirst = NULL;
				m_pLast = NULL;
			}
			--m_ulNumListItems;
		}
		return pItem;
	}

	IteratorEx Erase( const IteratorEx & in_rIter )
	{
		IteratorEx nextIter;
		nextIter.pItem = U_NEXTITEM::Get( in_rIter.pItem );
		nextIter.pPrevItem = in_rIter.pPrevItem;
		if( in_rIter.pItem == m_pFirst )
		{
			m_pFirst = nextIter.pItem;
		}
		else
		{
			U_NEXTITEM::Get( in_rIter.pPrevItem ) = nextIter.pItem;
		}
		if( in_rIter.pItem == m_pLast )
		{
			m_pLast = in_rIter.pPrevItem;
		}
		--m_ulNumListItems;
		return nextIter;
	}

	void RemoveAll() { m_pFirst = NULL; m_pLast = NULL; m_ulNumListItems = 0; }
	AkUInt32 Length() const { return m_ulNumListItems; }
	bool IsEmpty() const { return m_pFirst == NULL; }
	T * First() const { return m_pFirst; }
	T * Last() const { return m_pLast; }

	T *			m_pFirst;
	T *			m_pLast;
	AkUInt32	m_ulNumListItems;
};

template <class T>
struct AkListBareLightNextItem
{
	static T *& Get( T * in_pItem ) { return in_pItem->pNextLightItem; }
};

/** AkListBareLight: first pointer only (aklistbarelight.h). */
template <class T, class U_NEXTITEM = AkListBareLightNextItem<T> >
class AkListBareLight
{
public:
	AkListBareLight() : m_pFirst( NULL ) {}

	void AddFirst( T * in_pItem )
	{
		U_NEXTITEM::Get( in_pItem ) = m_pFirst;
		m_pFirst = in_pItem;
	}

	AKRESULT Remove( T * in_pItem )
	{
		T * pPrev = NULL;
		for( T * p = m_pFirst; p; p = U_NEXTITEM::Get( p ) )
		{
			if( p == in_pItem )
			{
				if( pPrev )
				{
					U_NEXTITEM::Get( pPrev ) = U_NEXTITEM::Get( p );
				}
				else
				{
					m_pFirst = U_NEXTITEM::Get( p );
				}
				return AK_Success;
			}
			pPrev = p;
		}
		return AK_Fail;
	}

	T * First() const { return m_pFirst; }

	T * m_pFirst;
};

/** AkDefaultHashListBarePolicy: the key and the next pointer are members of the item. */
template <class T_KEY, class T_ITEM>
struct AkDefaultHashListBarePolicy
{
	static T_KEY Key( const T_ITEM * in_pItem ) { return in_pItem->key; }
};

inline AkUIntPtr AkHashListHash( AkUInt32 in_key ) { return in_key; }
inline AkUIntPtr AkHashListHash( const void * in_key ) { return (AkUIntPtr)in_key; }

/** AkHashListBare: intrusive hash list, items carry `key` and `pNextItem` (akhashlist.h). */
template <class T_KEY, class T_ITEM, unsigned int T_HASHSIZE, class T_POLICY = AkDefaultHashListBarePolicy<T_KEY, T_ITEM> >
class AkHashListBare
{
public:
	struct Iterator
	{
		T_ITEM **	pTable;
		AkUInt32	uiTable;
		T_ITEM *	pItem;

		Iterator & operator++()
		{
			pItem = pItem->pNextItem;
			while( !pItem && ++uiTable < T_HASHSIZE )
			{
				pItem = pTable[ uiTable ];
			}
			return *this;
		}
		T_ITEM * operator*() const { return pItem; }
		bool operator!=( const Iterator & in_rOp ) const { return pItem != in_rOp.pItem; }
	};

	struct IteratorEx : public Iterator
	{
		T_ITEM * pPrevItem;

		IteratorEx & operator++()
		{
			pPrevItem = this->pItem;
			this->pItem = this->pItem->pNextItem;
			while( !this->pItem && ++this->uiTable < T_HASHSIZE )
			{
				pPrevItem = NULL;
				this->pItem = this->pTable[ this->uiTable ];
			}
			return *this;
		}
	};

	AkHashListBare() : m_uiSize( 0 ) { memset( m_table, 0, sizeof( m_table ) ); }

	void Init() { m_uiSize = 0; memset( m_table, 0, sizeof( m_table ) ); }
	void Term() { RemoveAll(); }
	void RemoveAll() { memset( m_table, 0, sizeof( m_table ) ); m_uiSize = 0; }

	Iterator Begin() const
	{
		Iterator it;
		it.pTable = (T_ITEM **)m_table;
		it.uiTable = 0;
		it.pItem = m_table[ 0 ];
		while( !it.pItem && ++it.uiTable < T_HASHSIZE )
		{
			it.pItem = m_table[ it.uiTable ];
		}
		return it;
	}

	IteratorEx BeginEx() const
	{
		IteratorEx it;
		it.pTable = (T_ITEM **)m_table;
		it.uiTable = 0;
		it.pItem = m_table[ 0 ];
		it.pPrevItem = NULL;
		while( !it.pItem && ++it.uiTable < T_HASHSIZE )
		{
			it.pItem = m_table[ it.uiTable ];
		}
		return it;
	}

	Iterator End() const { Iterator it; it.pItem = NULL; return it; }

	T_ITEM * Exists( T_KEY in_Key ) const
	{
		for( T_ITEM * pItem = m_table[ AkHashListHash( in_Key ) % T_HASHSIZE ]; pItem; pItem = pItem->pNextItem )
		{
			if( T_POLICY::Key( pItem ) == in_Key )
			{
				return pItem;
			}
		}
		return NULL;
	}

	void Set( T_ITEM * in_pItem )
	{
		const AkUInt32 uiTable = AkHashListHash( T_POLICY::Key( in_pItem ) ) % T_HASHSIZE;
		in_pItem->pNextItem = m_table[ uiTable ];
		m_table[ uiTable ] = in_pItem;
		++m_uiSize;
	}

	T_ITEM * Unset( T_KEY in_Key )
	{
		const AkUInt32 uiTable = AkHashListHash( in_Key ) % T_HASHSIZE;
		T_ITEM * pPrev = NULL;
		for( T_ITEM * pItem = m_table[ uiTable ]; pItem; pItem = pItem->pNextItem )
		{
			if( T_POLICY::Key( pItem ) == in_Key )
			{
				if( pPrev )
				{
					pPrev->pNextItem = pItem->pNextItem;
				}
				else
				{
					m_table[ uiTable ] = pItem->pNextItem;
				}
				--m_uiSize;
				return pItem;
			}
			pPrev = pItem;
		}
		return NULL;
	}

	/** 2012 0x94dd60: advance first, then unlink the erased item. */
	IteratorEx Erase( const IteratorEx & in_rIter )
	{
		IteratorEx returnedIt;
		returnedIt.pTable = in_rIter.pTable;
		returnedIt.uiTable = in_rIter.uiTable;
		returnedIt.pItem = in_rIter.pItem->pNextItem;
		returnedIt.pPrevItem = in_rIter.pPrevItem;
		while( !returnedIt.pItem && ++returnedIt.uiTable < T_HASHSIZE )
		{
			returnedIt.pPrevItem = NULL;
			returnedIt.pItem = returnedIt.pTable[ returnedIt.uiTable ];
		}
		if( in_rIter.pPrevItem )
		{
			in_rIter.pPrevItem->pNextItem = in_rIter.pItem->pNextItem;
		}
		else
		{
			m_table[ in_rIter.uiTable ] = in_rIter.pItem->pNextItem;
		}
		--m_uiSize;
		return returnedIt;
	}

	AkUInt32 Length() const { return m_uiSize; }

	T_ITEM *	m_table[ T_HASHSIZE ];
	AkUInt32	m_uiSize;
};

/** AkHashList: owns its items (pNextItem + MapStruct), allocated from m_MemPoolId (akhashlist.h). */
template <class T_KEY, class T_ITEM, unsigned int T_HASHSIZE>
class AkHashList
{
public:
	struct Item
	{
		Item *						pNextItem;
		MapStruct<T_KEY, T_ITEM>	Assoc;
	};

	struct Iterator
	{
		Item **		pTable;
		AkUInt32	uiTable;
		Item *		pItem;

		Iterator & operator++()
		{
			pItem = pItem->pNextItem;
			while( !pItem && ++uiTable < T_HASHSIZE )
			{
				pItem = pTable[ uiTable ];
			}
			return *this;
		}
		MapStruct<T_KEY, T_ITEM> & operator*() const { return pItem->Assoc; }
		bool operator!=( const Iterator & in_rOp ) const { return pItem != in_rOp.pItem; }
	};

	struct IteratorEx : public Iterator
	{
		Item * pPrevItem;
	};

	AkHashList() : m_MemPoolId( AK_INVALID_POOL_ID ), m_uiSize( 0 ) { memset( m_table, 0, sizeof( m_table ) ); }

	void Init( AkMemPoolId in_MemPoolId ) { m_MemPoolId = in_MemPoolId; m_uiSize = 0; memset( m_table, 0, sizeof( m_table ) ); }

	void Term()
	{
		if( m_MemPoolId != AK_INVALID_POOL_ID )
		{
			RemoveAll();
			m_MemPoolId = AK_INVALID_POOL_ID;
		}
	}

	void RemoveAll()
	{
		for( AkUInt32 i = 0; i < T_HASHSIZE; ++i )
		{
			Item * pItem = m_table[ i ];
			while( pItem )
			{
				Item * pNext = pItem->pNextItem;
				pItem->Assoc.item.~T_ITEM();
				AK::MemoryMgr::Free( m_MemPoolId, pItem );
				pItem = pNext;
			}
			m_table[ i ] = NULL;
		}
		m_uiSize = 0;
	}

	Iterator Begin() const
	{
		Iterator it;
		it.pTable = (Item **)m_table;
		it.uiTable = 0;
		it.pItem = m_table[ 0 ];
		while( !it.pItem && ++it.uiTable < T_HASHSIZE )
		{
			it.pItem = m_table[ it.uiTable ];
		}
		return it;
	}

	Iterator End() const { Iterator it; it.pItem = NULL; return it; }

	IteratorEx FindEx( T_KEY in_Key ) const
	{
		IteratorEx it;
		it.pTable = (Item **)m_table;
		it.uiTable = AkHashListHash( in_Key ) % T_HASHSIZE;
		it.pItem = m_table[ it.uiTable ];
		it.pPrevItem = NULL;
		while( it.pItem && it.pItem->Assoc.key != in_Key )
		{
			it.pPrevItem = it.pItem;
			it.pItem = it.pItem->pNextItem;
		}
		return it;
	}

	T_ITEM * Exists( T_KEY in_Key ) const
	{
		for( Item * pItem = m_table[ AkHashListHash( in_Key ) % T_HASHSIZE ]; pItem; pItem = pItem->pNextItem )
		{
			if( pItem->Assoc.key == in_Key )
			{
				return &pItem->Assoc.item;
			}
		}
		return NULL;
	}

	T_ITEM * Set( T_KEY in_Key )
	{
		T_ITEM * pItem = Exists( in_Key );
		return pItem ? pItem : CreateEntry( in_Key, AkHashListHash( in_Key ) % T_HASHSIZE );
	}

	T_ITEM * Set( T_KEY in_Key, bool & out_bWasAlreadyThere )
	{
		T_ITEM * pItem = Exists( in_Key );
		out_bWasAlreadyThere = pItem != NULL;
		return pItem ? pItem : CreateEntry( in_Key, AkHashListHash( in_Key ) % T_HASHSIZE );
	}

	void Unset( T_KEY in_Key )
	{
		IteratorEx it = FindEx( in_Key );
		if( it.pItem )
		{
			RemoveItem( it.uiTable, it.pItem, it.pPrevItem );
		}
	}

	IteratorEx Erase( const IteratorEx & in_rIter )
	{
		IteratorEx returnedIt;
		returnedIt.pTable = in_rIter.pTable;
		returnedIt.uiTable = in_rIter.uiTable;
		returnedIt.pItem = in_rIter.pItem->pNextItem;
		returnedIt.pPrevItem = in_rIter.pPrevItem;
		while( !returnedIt.pItem && ++returnedIt.uiTable < T_HASHSIZE )
		{
			returnedIt.pPrevItem = NULL;
			returnedIt.pItem = returnedIt.pTable[ returnedIt.uiTable ];
		}
		RemoveItem( in_rIter.uiTable, in_rIter.pItem, in_rIter.pPrevItem );
		return returnedIt;
	}

	AkUInt32 Length() const { return m_uiSize; }

	Item *		m_table[ T_HASHSIZE ];
	AkMemPoolId	m_MemPoolId;
	AkUInt32	m_uiSize;

protected:
	T_ITEM * CreateEntry( T_KEY in_Key, AkUInt32 in_uiTable )
	{
		Item * pNewItem = (Item *)AK::MemoryMgr::Malloc( m_MemPoolId, sizeof( Item ) );
		if( !pNewItem )
		{
			return NULL;
		}
		pNewItem->pNextItem = m_table[ in_uiTable ];
		pNewItem->Assoc.key = in_Key;
		AkPlacementNew( &pNewItem->Assoc.item ) T_ITEM;
		m_table[ in_uiTable ] = pNewItem;
		++m_uiSize;
		return &pNewItem->Assoc.item;
	}

	void RemoveItem( AkUInt32 in_uiTable, Item * in_pItem, Item * in_pPrevItem )
	{
		if( in_pPrevItem )
		{
			in_pPrevItem->pNextItem = in_pItem->pNextItem;
		}
		else
		{
			m_table[ in_uiTable ] = in_pItem->pNextItem;
		}
		in_pItem->Assoc.item.~T_ITEM();
		AK::MemoryMgr::Free( m_MemPoolId, in_pItem );
		--m_uiSize;
	}
};

/** CAkList2: a pooled singly linked list (aklist2.h): m_ulMinNumListItems pre-allocated in m_pvMemStart. */
template <class T, class ARG_T, int TAllocStrategy = 1, class TAlloc = ArrayPoolDefault>
class CAkList2
{
public:
	struct ListItem
	{
		ListItem *	pNextListItem;
		T			Item;
	};

	struct Iterator
	{
		ListItem * pItem;
		Iterator & operator++() { pItem = pItem->pNextListItem; return *this; }
		T & operator*() const { return pItem->Item; }
		bool operator!=( const Iterator & in_rOp ) const { return pItem != in_rOp.pItem; }
	};

	CAkList2() : m_pFree( NULL ), m_pvMemStart( NULL ), m_ulMinNumListItems( 0 ), m_ulMaxNumListItems( 0 ), m_ulNumListItems( 0 ), m_pFirst( NULL ), m_pLast( NULL ) {}

	/** CAkBankMgr::Init's shape: in_ulNumListItems pre-allocated, at most in_ulMaxNumListItems. */
	AKRESULT Init( AkUInt32 in_ulNumListItems, AkUInt32 in_ulMaxNumListItems )
	{
		m_ulNumListItems = 0;
		m_ulMaxNumListItems = in_ulMaxNumListItems;
		m_pFree = NULL;
		m_pvMemStart = NULL;
		m_pFirst = NULL;
		m_pLast = NULL;
		m_ulMinNumListItems = 0;
		if( in_ulNumListItems )
		{
			m_pvMemStart = (ListItem *)AK::MemoryMgr::Malloc( TAlloc::Get(), sizeof( ListItem ) * in_ulNumListItems );
			if( !m_pvMemStart )
			{
				return AK_InsufficientMemory;
			}
			m_ulMinNumListItems = in_ulNumListItems;
			m_pFree = m_pvMemStart;
			for( AkUInt32 i = 0; i < in_ulNumListItems - 1; ++i )
			{
				m_pvMemStart[ i ].pNextListItem = &m_pvMemStart[ i + 1 ];
			}
			m_pvMemStart[ in_ulNumListItems - 1 ].pNextListItem = NULL;
		}
		return AK_Success;
	}

	/** CAkSwitchCntr::Init's shape: nothing pre-allocated, no maximum. */
	void InitUnbounded()
	{
		m_ulNumListItems = 0;
		m_pFree = NULL;
		m_pvMemStart = NULL;
		m_pFirst = NULL;
		m_pLast = NULL;
		m_ulMaxNumListItems = (AkUInt32)-1;
	}

	AKRESULT Reserve( AkUInt32 in_ulNumItems )
	{
		if( !in_ulNumItems || m_pvMemStart )
		{
			return AK_Success;
		}
		m_pvMemStart = (ListItem *)AK::MemoryMgr::Malloc( TAlloc::Get(), sizeof( ListItem ) * in_ulNumItems );
		if( !m_pvMemStart )
		{
			return AK_InsufficientMemory;
		}
		m_ulMinNumListItems = in_ulNumItems;
		for( AkUInt32 i = 0; i < in_ulNumItems; ++i )
		{
			m_pvMemStart[ i ].pNextListItem = m_pFree;
			m_pFree = &m_pvMemStart[ i ];
		}
		return AK_Success;
	}

	T * AddLast()
	{
		if( !m_pFree )
		{
			if( m_ulNumListItems >= m_ulMaxNumListItems )
			{
				return NULL;
			}
			ListItem * pNew = (ListItem *)AK::MemoryMgr::Malloc( TAlloc::Get(), sizeof( ListItem ) );
			if( !pNew )
			{
				return NULL;
			}
			pNew->pNextListItem = m_pFree;
			m_pFree = pNew;
		}
		if( m_pLast )
		{
			m_pLast->pNextListItem = m_pFree;
		}
		else
		{
			m_pFirst = m_pFree;
		}
		m_pLast = m_pFree;
		m_pFree = m_pFree->pNextListItem;
		m_pLast->pNextListItem = NULL;
		++m_ulNumListItems;
		AkPlacementNew( &m_pLast->Item ) T;
		return &m_pLast->Item;
	}

	T * AddLast( ARG_T in_rItem )
	{
		T * pItem = AddLast();
		if( pItem )
		{
			*pItem = in_rItem;
		}
		return pItem;
	}

	void RemoveFirst()
	{
		ListItem * pItem = m_pFirst;
		if( !pItem )
		{
			return;
		}
		m_pFirst = pItem->pNextListItem;
		if( pItem == m_pLast )
		{
			m_pLast = NULL;
		}
		pItem->Item.~T();
		FreeListItem( pItem );
		--m_ulNumListItems;
	}

	void RemoveAll()
	{
		while( m_pFirst )
		{
			RemoveFirst();
		}
	}

	AKRESULT Term()
	{
		if( m_ulMaxNumListItems )
		{
			RemoveAll();
			ListItem * pItem = m_pFree;
			while( pItem )
			{
				ListItem * pNext = pItem->pNextListItem;
				if( pItem < m_pvMemStart || pItem >= m_pvMemStart + m_ulMinNumListItems )
				{
					AK::MemoryMgr::Free( TAlloc::Get(), pItem );
				}
				pItem = pNext;
			}
			if( m_pvMemStart )
			{
				AK::MemoryMgr::Free( TAlloc::Get(), m_pvMemStart );
			}
			m_pvMemStart = NULL;
			m_pFree = NULL;
			m_ulMinNumListItems = 0;
			m_ulMaxNumListItems = 0;
			m_pFirst = NULL;
			m_pLast = NULL;
		}
		return AK_Success;
	}

	/** aklist2.h RemoveItem: unlink, destroy, and return the slot (strategy 2 keeps every slot on the free list). */
	void RemoveItem( ListItem * in_pItem, ListItem * in_pPrevItem )
	{
		if( in_pItem == m_pFirst )
		{
			m_pFirst = in_pItem->pNextListItem;
		}
		else
		{
			in_pPrevItem->pNextListItem = in_pItem->pNextListItem;
		}
		if( in_pItem == m_pLast )
		{
			m_pLast = in_pPrevItem;
		}
		in_pItem->Item.~T();
		if( TAllocStrategy == 2 )
		{
			in_pItem->pNextListItem = m_pFree;
			m_pFree = in_pItem;
		}
		else
		{
			FreeListItem( in_pItem );
		}
		--m_ulNumListItems;
	}

	Iterator Begin() const { Iterator it; it.pItem = m_pFirst; return it; }
	Iterator End() const { Iterator it; it.pItem = NULL; return it; }
	AkUInt32 Length() const { return m_ulNumListItems; }
	T & First() { return m_pFirst->Item; }

	ListItem *	m_pFree;
	ListItem *	m_pvMemStart;
	AkUInt32	m_ulMinNumListItems;
	AkUInt32	m_ulMaxNumListItems;
	AkUInt32	m_ulNumListItems;
	ListItem *	m_pFirst;
	ListItem *	m_pLast;

protected:
	void FreeListItem( ListItem * in_pItem )
	{
		if( in_pItem >= m_pvMemStart && in_pItem < m_pvMemStart + m_ulMinNumListItems )
		{
			in_pItem->pNextListItem = m_pFree;
			m_pFree = in_pItem;
		}
		else
		{
			AK::MemoryMgr::Free( TAlloc::Get(), in_pItem );
		}
	}
};

/** CAkKeyList: CAkList2 of MapStruct with lookup by key (akkeylist.h). */
template <class T_KEY, class T_ITEM, int TAllocStrategy = 1, class TAlloc = ArrayPoolDefault>
class CAkKeyList : public CAkList2< MapStruct<T_KEY, T_ITEM>, const MapStruct<T_KEY, T_ITEM> &, TAllocStrategy, TAlloc >
{
	typedef CAkList2< MapStruct<T_KEY, T_ITEM>, const MapStruct<T_KEY, T_ITEM> &, TAllocStrategy, TAlloc > Base;
public:
	T_ITEM * Exists( T_KEY in_Key ) const
	{
		for( typename Base::ListItem * p = this->m_pFirst; p; p = p->pNextListItem )
		{
			if( p->Item.key == in_Key )
			{
				return &p->Item.item;
			}
		}
		return NULL;
	}

	T_ITEM * Set( T_KEY in_Key )
	{
		T_ITEM * pItem = Exists( in_Key );
		if( pItem )
		{
			return pItem;
		}
		MapStruct<T_KEY, T_ITEM> * pStruct = Base::AddLast();
		if( !pStruct )
		{
			return NULL;
		}
		pStruct->key = in_Key;
		return &pStruct->item;
	}

	T_ITEM * Set( T_KEY in_Key, const T_ITEM & in_Item )
	{
		T_ITEM * pItem = Set( in_Key );
		if( pItem )
		{
			*pItem = in_Item;
		}
		return pItem;
	}

	/** akkeylist.h Unset (agent IH3 fork C): the first item of the key, if any. */
	void Unset( T_KEY in_Key )
	{
		typename Base::ListItem * pPrev = NULL;
		for( typename Base::ListItem * p = this->m_pFirst; p; pPrev = p, p = p->pNextListItem )
		{
			if( p->Item.key == in_Key )
			{
				Base::RemoveItem( p, pPrev );
				return;
			}
		}
	}
};

/** CAkMultiKeyList: CAkList2 of MapStruct kept sorted by key, equal keys in insertion order (akmultikeylist.h). */
template <class T_KEY, class T_ITEM, int TAllocStrategy = 2, class TAlloc = ArrayPoolDefault>
class CAkMultiKeyList : public CAkList2< MapStruct<T_KEY, T_ITEM>, const MapStruct<T_KEY, T_ITEM> &, TAllocStrategy, TAlloc >
{
	typedef CAkList2< MapStruct<T_KEY, T_ITEM>, const MapStruct<T_KEY, T_ITEM> &, TAllocStrategy, TAlloc > Base;
public:
	/** 2012 0x922160 (the <AkUInt32, AkPendingAction *> instantiation). */
	AKRESULT Insert( T_KEY in_Key, T_ITEM & in_rItem )
	{
		if( !this->m_pFree )
		{
			if( this->m_ulNumListItems >= this->m_ulMaxNumListItems )
			{
				return AK_Fail;
			}
			typename Base::ListItem * pNew = (typename Base::ListItem *)AK::MemoryMgr::Malloc( TAlloc::Get(), sizeof( typename Base::ListItem ) );
			if( !pNew )
			{
				return AK_Fail;
			}
			pNew->pNextListItem = this->m_pFree;
			this->m_pFree = pNew;
		}
		typename Base::ListItem * pNew = this->m_pFree;
		pNew->Item.key = in_Key;
		pNew->Item.item = in_rItem;
		typename Base::ListItem * pPrev = NULL;
		typename Base::ListItem * pNext = this->m_pFirst;
		while( pNext && pNext->Item.key <= in_Key )
		{
			pPrev = pNext;
			pNext = pNext->pNextListItem;
		}
		if( !pNext )
		{
			this->m_pLast = pNew;
		}
		if( pPrev )
		{
			pPrev->pNextListItem = pNew;
		}
		else
		{
			this->m_pFirst = pNew;
		}
		this->m_pFree = pNew->pNextListItem;
		pNew->pNextListItem = pNext;
		++this->m_ulNumListItems;
		return AK_Success;
	}
};

/** AkPropValue: one property value, float or int view (the PDB's AkPropValue union). */
union AkPropValue
{
	AkReal32	fValue;
	AkInt32		iValue;
	AkPropValue() : iValue( 0 ) {}
	AkPropValue( AkReal32 in_fValue ) : fValue( in_fValue ) {}
};

template <class T>
struct RANGED_MODIFIERS
{
	T m_min;
	T m_max;
};

template <class T>
struct RANGED_PARAMETER
{
	T					m_base;
	RANGED_MODIFIERS<T>	m_mod;
};

/**
 * AkPropBundle<T>: one allocation [count][ids x count][pad to 4][T x count] (akpropbundle.h).
 * SetInitialParams 2012 0x952d40 (float) / 0x9383d0 (RANGED_MODIFIERS<AkPropValue>) copies the bank bytes
 * verbatim into that layout.
 */
template <class T>
class AkPropBundle
{
public:
	AkPropBundle() : m_pProps( NULL ) {}

	AKRESULT SetInitialParams( AkUInt8 *& io_rpData, AkUInt32 & /*io_rulDataSize*/ )
	{
		const AkUInt8 cProps = *io_rpData++;
		if( cProps )
		{
			const AkUInt32 uIdsSize = ( cProps + 4 ) & ~3u;
			const AkUInt32 uValuesSize = sizeof( T ) * cProps;
			AkUInt8 * pProps = (AkUInt8 *)AK::MemoryMgr::Malloc( g_DefaultPoolId, uIdsSize + uValuesSize );
			if( !pProps )
			{
				return AK_InsufficientMemory;
			}
			pProps[ 0 ] = cProps;
			memcpy( pProps + 1, io_rpData, cProps );
			io_rpData += cProps;
			memcpy( pProps + uIdsSize, io_rpData, uValuesSize );
			io_rpData += uValuesSize;
			m_pProps = pProps;
		}
		return AK_Success;
	}

	AkUInt32 Count() const { return m_pProps ? m_pProps[ 0 ] : 0; }
	AkUInt8 IDAt( AkUInt32 in_uIndex ) const { return m_pProps[ in_uIndex + 1 ]; }
	T * ValueAt( AkUInt32 in_uIndex ) const { return (T *)( m_pProps + ( ( m_pProps[ 0 ] + 4 ) & ~3u ) ) + in_uIndex; }

	T * FindProp( AkUInt8 in_ePropID ) const
	{
		if( m_pProps )
		{
			const AkUInt32 cProps = m_pProps[ 0 ];
			for( AkUInt32 i = 0; i < cProps; ++i )
			{
				if( m_pProps[ i + 1 ] == in_ePropID )
				{
					return ValueAt( i );
				}
			}
		}
		return NULL;
	}

	/** AkPropBundle<AkPropValue>::AddAkProp 2012 0x9384a0: grow the allocation by one id and one value. */
	T * AddAkProp( AkUInt8 in_ePropID )
	{
		const AkUInt32 cProps = Count();
		const AkUInt32 uNewIds = ( cProps + 5 ) & ~3u;
		AkUInt8 * pProps = (AkUInt8 *)AK::MemoryMgr::Malloc( g_DefaultPoolId, uNewIds + sizeof( T ) * ( cProps + 1 ) );
		if( !pProps )
		{
			return NULL;
		}
		if( m_pProps )
		{
			memcpy( pProps + 1, m_pProps + 1, cProps );
			memcpy( pProps + uNewIds, m_pProps + ( ( cProps + 4 ) & ~3u ), sizeof( T ) * cProps );
			AK::MemoryMgr::Free( g_DefaultPoolId, m_pProps );
		}
		pProps[ cProps + 1 ] = in_ePropID;
		pProps[ 0 ] = (AkUInt8)( cProps + 1 );
		m_pProps = pProps;
		return (T *)( pProps + uNewIds ) + cProps;
	}

	AKRESULT SetAkProp( AkUInt8 in_ePropID, const T & in_Value )
	{
		T * pValue = FindProp( in_ePropID );
		if( !pValue )
		{
			pValue = AddAkProp( in_ePropID );
		}
		if( !pValue )
		{
			return AK_Fail;
		}
		*pValue = in_Value;
		return AK_Success;
	}

	void RemoveAll()
	{
		if( m_pProps )
		{
			AK::MemoryMgr::Free( g_DefaultPoolId, m_pProps );
		}
		m_pProps = NULL;
	}

	AkUInt8 * m_pProps;
};

enum AkCurveScaling
{
	AkCurveScaling_None					= 0,
	AkCurveScaling_dB_96_3				= 2,
	AkCurveScaling_Frequency_20_20000	= 3,
	AkCurveScaling_dB_96_3_NoCheck		= 4
};

template <class VALUE_TYPE>
struct AkRTPCGraphPointBase
{
	AkReal32				From;
	VALUE_TYPE				To;
	AkCurveInterpolation	Interp;
};

typedef AkRTPCGraphPointBase<AkReal32> AkRTPCGraphPoint;

/** CAkConversionTable: a copied curve (Set 2012 0x931370) and its evaluation: ConvertInternal
    <float> 2012 0x930ec0 / <unsigned long> 0x931040 (IH3, fork R). io_index is the first point to test; the SDK passes 0. */
template <class T_GRAPH_POINT, class T_TARGET>
class CAkConversionTable
{
public:
	CAkConversionTable() : m_pArrayGraphPoints( NULL ), m_ulArraySize( 0 ), m_eScaling( AkCurveScaling_None ) {}

	T_TARGET Convert( AkReal32 in_valueToConvert )
	{
		AkUInt32 uIndex = 0;
		return ConvertInternal( in_valueToConvert, uIndex );
	}

	T_TARGET ConvertInternal( AkReal32 in_valueToConvert, AkUInt32 & io_index )
	{
		AkReal32 fResult = in_valueToConvert;
		if( m_pArrayGraphPoints && m_ulArraySize )
		{
			if( m_ulArraySize == 1 )
			{
				fResult = (AkReal32)m_pArrayGraphPoints[ 0 ].To;
			}
			else
			{
				while( io_index < m_ulArraySize )
				{
					const T_GRAPH_POINT & rPoint = m_pArrayGraphPoints[ io_index ];
					if( rPoint.From >= in_valueToConvert )
					{
						fResult = (AkReal32)rPoint.To;
						break;
					}
					if( io_index == m_ulArraySize - 1 )
					{
						fResult = (AkReal32)rPoint.To;
						break;
					}
					const T_GRAPH_POINT & rNext = m_pArrayGraphPoints[ io_index + 1 ];
					if( rNext.From > in_valueToConvert )
					{
						const AkReal32 fRatio = ( in_valueToConvert - rPoint.From ) / ( rNext.From - rPoint.From );
						if( rPoint.Interp == AkCurveInterpolation_Linear )
						{
							fResult = fRatio * ( (AkReal32)rNext.To - (AkReal32)rPoint.To ) + (AkReal32)rPoint.To;
						}
						else if( rPoint.Interp == AkCurveInterpolation_Constant )
						{
							fResult = (AkReal32)rPoint.To;
						}
						else
						{
							fResult = g_Interpol.InterpolateNoCheck( fRatio, (AkReal32)rPoint.To, (AkReal32)rNext.To, rPoint.Interp );
						}
						break;
					}
					++io_index;
				}
			}
		}
		switch( m_eScaling )
		{
		case AkCurveScaling_dB_96_3:
			return (T_TARGET)g_Interpol.m_tableLinearMutingTodBMuting96.Get( fResult );
		case AkCurveScaling_Frequency_20_20000:
			return (T_TARGET)g_Interpol.m_tableLinearToFrequency_20_20000.Get( fResult );
		case AkCurveScaling_dB_96_3_NoCheck:
			return (T_TARGET)g_Interpol.m_tableLinearMutingTodBMuting96.GetNoCheck( fResult );
		default:
			return (T_TARGET)fResult;
		}
	}

	AKRESULT Set( T_GRAPH_POINT * in_pArrayConversion, AkUInt32 in_ulConversionArraySize, AkCurveScaling in_eScaling )
	{
		Unset();
		if( !in_pArrayConversion || !in_ulConversionArraySize )
		{
			return AK_InvalidParameter;
		}
		m_pArrayGraphPoints = (T_GRAPH_POINT *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( T_GRAPH_POINT ) * in_ulConversionArraySize );
		if( !m_pArrayGraphPoints )
		{
			m_ulArraySize = 0;
			return AK_InsufficientMemory;
		}
		memcpy( m_pArrayGraphPoints, in_pArrayConversion, sizeof( T_GRAPH_POINT ) * in_ulConversionArraySize );
		m_ulArraySize = in_ulConversionArraySize;
		m_eScaling = in_eScaling;
		return AK_Success;
	}

	void Unset()
	{
		if( m_pArrayGraphPoints )
		{
			AK::MemoryMgr::Free( g_DefaultPoolId, m_pArrayGraphPoints );
			m_pArrayGraphPoints = NULL;
		}
		m_ulArraySize = 0;
		m_eScaling = AkCurveScaling_None;
	}

	T_GRAPH_POINT *	m_pArrayGraphPoints;
	AkUInt32		m_ulArraySize;
	AkCurveScaling	m_eScaling;
};

/** CAkBitArrayMax32. */
struct CAkBitArrayMax32
{
	CAkBitArrayMax32() : m_i32BitArray( 0 ) {}
	void SetBit( AkUInt32 in_uBit ) { m_i32BitArray |= ( 1u << in_uBit ); }
	void UnsetBit( AkUInt32 in_uBit ) { m_i32BitArray &= ~( 1u << in_uBit ); }
	bool IsSet( AkUInt32 in_uBit ) const { return ( m_i32BitArray & ( 1u << in_uBit ) ) != 0; }
	AkUInt32 m_i32BitArray;
};
