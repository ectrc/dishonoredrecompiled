#pragma once
// Engine/inc/arkrequestmanager.h
// DISHONORED(port): agent DN. Arkane's priority request queue, shared by FArkComponentLocomotion,
// FArkComponentFaceTo, FArkComponentLookat and FDisComponentLODManager. Retail's unit is this header (the 2012 PDB
// attributes all 25 instantiated bodies to `engine\inc\arkrequestmanager.h`); the line numbers in the comments below are
// that file's, and the rvas are the FLocoRequestData instantiation - the FaceTo, Lookat and LOD instantiations are the
// same code at 0x576b60.., 0x594600.. and 0x8bb700.. (2012).
//
// The shape matters to every caller: a request is *not* a slot the caller owns. AddRequest inserts by priority (highest
// first, ties keeping insertion order), hands back a monotonically increasing id, and tells the owner whenever the
// *first* request changes - which is what makes the component act on the highest-priority order without the askers
// knowing about each other. Everything a component does with a request goes through the id, never the index, because an
// insert moves every index below it.

#ifndef _INC_ARKREQUESTMANAGER
#define _INC_ARKREQUESTMANAGER

class FArkComponentBase;

/**
 * DISHONORED(port): the queue. 2012 PDB sizeof 36 for every instantiation; the FRequest element size is
 * sizeof(RequestData) + 28 (72 + 28 = 100 for locomotion, 36 + 28 = 64 for face-to, 52 + 28 = 80 for look-at).
 */
template< class RequestDataType >
class FArkRequestManager
{
public:
	/** What the owner is told. Values are retail's (2012 PDB enum, same in all four instantiations). */
	enum EArkReqMgrEvent
	{
		ARK_REQMGR_REQUEST_ADDED				= 0,
		ARK_REQMGR_REQUEST_UPDATED				= 1,
		ARK_REQMGR_REQUEST_WILL_BE_REMOVED		= 2,
		ARK_REQMGR_FIRST_REQUEST_HAS_CHANGED	= 3,
	};

	// DISHONORED(layout): the 2012 PDB types this member as `void (__thiscall *)(FArkComponentBase *this,
	// const EArkReqMgrEvent, const int)` - a four-byte *plain* function pointer whose first argument is the owner, not a
	// pointer-to-member. It matters: FArkComponentLocomotion has two bases, so an MSVC pointer-to-member function on it
	// would be eight bytes and the queue would be 40 rather than 36. The component registers a static thunk.
	typedef void (*OwnerEventCallbackType)( FArkComponentBase* _pOwner, const EArkReqMgrEvent _Event, const INT _RequestIdx );

	/** DISHONORED(layout): 2012 PDB FArkRequestManager<FLocoRequestData>::FRequest, sizeof 100, members in this order. */
	struct FRequest
	{
		RequestDataType	m_Data;				// @0
		FName			m_AskerName;		// @sizeof(RequestDataType)
		const void*		m_pAsker;
		INT				m_Priority;
		INT				m_ID;
		FLOAT			m_fTimeLeft;
		UBOOL			m_bOneFrameRequest;
	};

	FArkRequestManager()
		: m_pOwner( NULL )
		, m_pOwnerEventCallback( NULL )
		, m_CurUniqueID( 0 )
		, m_TimedRequestsCount( 0 )
		, m_PrevFirstRequestID( INDEX_NONE )
		, m_bHasRequest( FALSE )
		, m_bInitialized( FALSE )
	{}

	// DISHONORED(port): arkrequestmanager.h:37, 2013 rva 0x5461d0 (2012 0x588690). Note it does NOT clear
	// m_bHasRequest and m_bInitialized independently: retail writes the whole flags dword as
	// (flags & ~3) | 2, i.e. "initialized, no request".
	void Initialize( FArkComponentBase* const _pOwner, OwnerEventCallbackType _pOwnerEventCallback )
	{
		m_Requests.Empty( m_Requests.Num() );
		m_CurUniqueID = 0;
		m_TimedRequestsCount = 0;
		m_PrevFirstRequestID = INDEX_NONE;
		m_pOwner = _pOwner;
		m_pOwnerEventCallback = _pOwnerEventCallback;
		m_bHasRequest = FALSE;
		m_bInitialized = TRUE;
	}

	/** Retail's Stopping path: the queue is emptied and the owner forgotten without any callback. */
	void Reset()
	{
		m_Requests.Empty( m_Requests.Num() );
		m_bHasRequest = FALSE;
		m_bInitialized = FALSE;
		m_pOwner = NULL;
		m_pOwnerEventCallback = NULL;
		m_CurUniqueID = 0;
		m_TimedRequestsCount = 0;
		m_PrevFirstRequestID = INDEX_NONE;
	}

	UBOOL HasRequest() const		{ return m_bInitialized && m_bHasRequest; }
	UBOOL IsInitialized() const		{ return m_bInitialized; }
	INT GetRequestsCount() const	{ return m_Requests.Num(); }

	const FRequest& GetRequest( INT _Idx ) const	{ return m_Requests( _Idx ); }
	FRequest& GetRequest( INT _Idx )				{ return m_Requests( _Idx ); }
	const FRequest& GetFirstRequest() const			{ return m_Requests( 0 ); }
	FRequest& GetFirstRequest()						{ return m_Requests( 0 ); }

	// DISHONORED(port): arkrequestmanager.h:331, 2013 rva 0x542c20 (2012 0x583bf0). The scan stops at the first entry
	// whose priority the new one is >= to, so equal priorities keep insertion order; an empty queue appends.
	// _fDuration < 0 means "until withdrawn"; == 0 means "this frame only".
	INT AddRequest( const void* const _pAsker, FName _AskerName, INT _Priority, const RequestDataType& _RequestData, FLOAT _fDuration )
	{
		INT NewRequestIdx = 0;
		if( m_Requests.Num() )
		{
			NewRequestIdx = m_Requests.Num();
			for( INT i = 0; i < m_Requests.Num(); ++i )
			{
				if( _Priority >= m_Requests( i ).m_Priority )
				{
					NewRequestIdx = i;
					break;
				}
			}
			if( NewRequestIdx == m_Requests.Num() )
			{
				m_Requests.Add( 1 );
			}
			else
			{
				m_Requests.Insert( NewRequestIdx, 1 );
			}
		}
		else
		{
			m_Requests.Add( 1 );
		}

		FRequest& rNewRequest = m_Requests( NewRequestIdx );
		appMemcpy( &rNewRequest.m_Data, &_RequestData, sizeof( RequestDataType ) );
		rNewRequest.m_AskerName			= _AskerName;
		rNewRequest.m_pAsker			= _pAsker;
		rNewRequest.m_Priority			= _Priority;
		rNewRequest.m_ID				= m_CurUniqueID;
		rNewRequest.m_fTimeLeft			= _fDuration;
		rNewRequest.m_bOneFrameRequest	= ( _fDuration == 0.f );
		if( _fDuration >= 0.f )
		{
			++m_TimedRequestsCount;
		}
		m_bHasRequest = TRUE;
		const INT NewID = m_CurUniqueID++;

		if( m_pOwnerEventCallback )
		{
			m_pOwnerEventCallback( m_pOwner, ARK_REQMGR_REQUEST_ADDED, NewRequestIdx );
			if( NewRequestIdx == 0 )
			{
				m_pOwnerEventCallback( m_pOwner, ARK_REQMGR_FIRST_REQUEST_HAS_CHANGED, 0 );
			}
		}
		return NewID;
	}

	// DISHONORED(port): arkrequestmanager.h:405, 2013 rva 0x542dd0 (2012 0x583da0): the data is overwritten in place, so
	// the id, the priority and the position are untouched and only REQUEST_UPDATED is raised.
	void UpdateRequestByIdx( INT _RequestIdx, const RequestDataType& _RequestData )
	{
		appMemcpy( &m_Requests( _RequestIdx ).m_Data, &_RequestData, sizeof( RequestDataType ) );
		if( m_pOwnerEventCallback )
		{
			m_pOwnerEventCallback( m_pOwner, ARK_REQMGR_REQUEST_UPDATED, _RequestIdx );
		}
	}

	// DISHONORED(port): arkrequestmanager.h:470, 2013 rva 0x545c00 (2012 0x586ac0). WILL_BE_REMOVED is raised *before*
	// the entry goes, which is how the owner can still read it to tell the asker's callback that its order was aborted.
	void RemoveRequestByIdx( INT _RequestIdx )
	{
		if( m_Requests( _RequestIdx ).m_fTimeLeft >= 0.f )
		{
			--m_TimedRequestsCount;
		}
		if( m_pOwnerEventCallback )
		{
			m_pOwnerEventCallback( m_pOwner, ARK_REQMGR_REQUEST_WILL_BE_REMOVED, _RequestIdx );
		}
		m_Requests.Remove( _RequestIdx, 1 );
		m_bHasRequest = ( m_Requests.Num() > 0 );
		if( m_pOwnerEventCallback && _RequestIdx == 0 )
		{
			m_pOwnerEventCallback( m_pOwner, ARK_REQMGR_FIRST_REQUEST_HAS_CHANGED, 0 );
		}
	}

	// DISHONORED(port): 2013 rva 0x545ca0 (2012 0x586b60): walks backwards so the indices below the one being removed
	// stay valid.
	UBOOL RemoveAllRequestsFromAsker( const void* const _pAsker )
	{
		UBOOL bRemovedOne = FALSE;
		for( INT i = m_Requests.Num() - 1; i >= 0; --i )
		{
			if( m_Requests( i ).m_pAsker == _pAsker )
			{
				RemoveRequestByIdx( i );
				bRemovedOne = TRUE;
			}
		}
		return bRemovedOne;
	}

	// DISHONORED(port): 2013 rva 0x542bb0 (2012 0x583b80)
	INT GetRequestIdxByID( INT _RequestID ) const
	{
		for( INT i = 0; i < m_Requests.Num(); ++i )
		{
			if( m_Requests( i ).m_ID == _RequestID )
			{
				return i;
			}
		}
		return INDEX_NONE;
	}

	// DISHONORED(port): 2013 rva of the Lookat instantiation 0x553b70 (2012 0x5963e0): one-frame requests die at the end
	// of the frame they were added in, timed ones when their clock runs out. Returns TRUE when the first request changed.
	UBOOL Update( FLOAT _fTimeStep )
	{
		if( m_TimedRequestsCount > 0 )
		{
			for( INT i = m_Requests.Num() - 1; i >= 0; --i )
			{
				FRequest& rRequest = m_Requests( i );
				if( rRequest.m_fTimeLeft < 0.f )
				{
					continue;
				}
				if( rRequest.m_bOneFrameRequest )
				{
					RemoveRequestByIdx( i );
					continue;
				}
				rRequest.m_fTimeLeft -= _fTimeStep;
				if( rRequest.m_fTimeLeft <= 0.f )
				{
					RemoveRequestByIdx( i );
				}
			}
		}
		const INT FirstID = m_Requests.Num() > 0 ? m_Requests( 0 ).m_ID : INDEX_NONE;
		const UBOOL bChanged = ( FirstID != m_PrevFirstRequestID );
		m_PrevFirstRequestID = FirstID;
		return bChanged;
	}

	TArray<FRequest>		m_Requests;					// @0
	FArkComponentBase*		m_pOwner;					// @12
	OwnerEventCallbackType	m_pOwnerEventCallback;		// @16
	INT						m_CurUniqueID;				// @20
	INT						m_TimedRequestsCount;		// @24
	INT						m_PrevFirstRequestID;		// @28
	BITFIELD				m_bHasRequest:1;			// @32 mask 0x1
	BITFIELD				m_bInitialized:1;			// @32 mask 0x2
};

#endif
