#pragma once
// Engine/inc/arkgameeventdispatcher.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (13):
//   0x6b5fd0  public: virtual void __thiscall FRealCallableMethod<class UStateNPCMasterMinigame, class UStateNPCMasterMinigame, class FArkGameEvent const &>::operator()(class FArkGameEvent const &)
//   0x6b5fe0  public: virtual unsigned int __thiscall FRealCallableMethod<class UStateNPCMasterDead_Limp, class UStateNPCMasterDead_Limp, class FArkGameEvent const &>::HasObject(void *)const
//   0x6c1970  public: void __thiscall FArkGameEventDispatcher::UnregisterToEvent<struct FDisDesireRequest, struct FDisDesireRequest>(int const &, struct FDisDesireRequest *, void (__thiscall FDisDesireRequest::*)(class FArkGameEvent const &))
//   0x6cc540  public: void __thiscall FArkGameEventDispatcher::UnregisterToObjectEvent<class UDisSeqAct_SetRatSwarmCustomBehavior, class UDisSeqAct_SetRatSwarmCustomBehavior>(int const &, void *, class UDisSeqAct_SetRatSwarmCustomBehavior *, void (__thiscall UDisSeqAct_SetRatSwarmCustomBehavior::*)(class FArkGameEvent const &))
//   0x6cc820  public: void __thiscall FArkGameEventDispatcher::RegisterToObjectEvent<class UStateNPCMasterThrown, class UStateNPCMasterThrown>(int const &, void *, class UStateNPCMasterThrown *, void (__thiscall UStateNPCMasterThrown::*)(class FArkGameEvent const &))
//   0x70ec80  public: void __thiscall FArkGameEventDispatcher::RegisterToEvent<class UDisAIBlackboard, class UDisAIBlackboard>(int const &, class UDisAIBlackboard *, void (__thiscall UDisAIBlackboard::*)(class FArkGameEvent const &))
//   0x72ec40  public: void __thiscall FArkGameEventDispatcher::UnregisterToEvent<class UDisAISubStateFirePistol, class UDisAISubStateFirePistol>(int const &, class UDisAISubStateFirePistol *, void (__thiscall UDisAISubStateFirePistol::*)(class FArkGameEvent const &))
//   0x72ef20  public: void __thiscall FArkGameEventDispatcher::RegisterToEvent<class UDisAISubState, class UDisAISubState>(int const &, class UDisAISubState *, void (__thiscall UDisAISubState::*)(class FArkGameEvent const &))
//   0x76df50  public: virtual unsigned int __thiscall FRealCallableMethod<class ADisWallOfLight, class ADisWallOfLight, class FArkGameEvent const &>::HasMethod(void (__thiscall ADisWallOfLight::*)(class FArkGameEvent const &))const
//   0x784940  public: void __thiscall FArkGameEventDispatcher::UnregisterToObjectEvent<class UDisAISubProcessAmbientBarks, class UDisAISubProcessAmbientBarks>(int const &, void *, class UDisAISubProcessAmbientBarks *, void (__thiscall UDisAISubProcessAmbientBarks::*)(class FArkGameEvent const &))
//   0x791df0  public: virtual void __thiscall FRealCallableMethod<class ADishonoredPawn, class ADishonoredPawn, class FArkGameEvent const &>::operator()(class FArkGameEvent const &)
//   0x8d0200  public: void __thiscall FArkGameEventDispatcher::RegisterToObjectEvent<class FArkComponentFaceTo, class FArkComponentFaceTo>(int const &, void *, class FArkComponentFaceTo *, void (__thiscall FArkComponentFaceTo::*)(class FArkGameEvent const &))
//   0x8fa960  public: virtual unsigned int __thiscall FRealCallableMethod<class UDisTutorialTracker, class UDisTutorialTracker, class FArkGameEvent const &>::HasMethod(void (__thiscall UDisTutorialTracker::*)(class FArkGameEvent const &))const

/*-----------------------------------------------------------------------------
	DISHONORED(port): Arkane's game-event dispatcher - a process-wide singleton with one registration list per event
	type, plus a per-instigator multi-map so a listener can ask for one object's events only. Registering or
	unregistering while an event of that same type is being dispatched is DEFERRED into m_PendingRegistrations /
	m_PendingUnregistrations and applied when the outermost dispatch of that type finishes; that is what
	m_EventTypeBeingProcessed (a stack, so nesting works) is for. Several AI classes register and unregister from
	inside an event handler, so the deferral is not optional.

	2013 layout (FArkGameEventDispatcher, 0x165C = 5724 bytes, from CreateInstance 0x5572d0 and the constructor 0x557230):
		@0    m_PendingUnregistrations      TArray<FArkPendingEvent>
		@12   m_PendingRegistrations        TArray<FArkPendingEvent>
		@24   m_RegistrationTable           TArray<FCallableMethod>[79]
		@972  m_PerObjectRegistrationTable  TMultiMap<INT,FCallableMethod>[79]
		@5712 m_EventTypeBeingProcessed     TArray<INT>
	DISHONORED(layout): the 2012 Shipping build has 39 event types (2844 bytes); retail 2013 has 79. Both constructors
	are otherwise identical. Decoding against the 2012 count would drop every event id above 38, which includes the
	abstract-item (65) and elixir (57) notifications agentAJ.md names.
-----------------------------------------------------------------------------*/

/**
 * Number of event types the retail 2013 registration tables are sized for. The ids are EDisGameEventType
 * (DishonoredGame); the four lowest are the engine-side Ark events, of which the AI uses 1 (pushed by an avoidable),
 * 2 (LOD changed) and 3 (another actor terminated); 9 is DisGameEventType_DifficultyChange.
 */
enum { ARK_GAME_EVENT_TYPE_COUNT = 79 };

// DISHONORED(layout): 2012 PDB FArkGameEvent, 12 bytes. m_pEventParams points at the event's own parameter struct, whose
// type the handler picks by m_Type.
class FArkGameEvent
{
public:
	FArkGameEvent( INT _Type, void* _pEventParams = NULL, UObject* _pInstigator = NULL )
		: m_Type( _Type )
		, m_pEventParams( _pEventParams )
		, m_pInstigator( _pInstigator )
	{}

	INT			m_Type;
	void*		m_pEventParams;
	UObject*	m_pInstigator;
};

/** The behaviour every stored callable exposes; retail's FRealCallableMethod vtable in slot order. */
template< typename ParamType >
class FArkCallableMethodImpl
{
public:
	virtual void Invoke( ParamType _Param ) const = 0;
	virtual UBOOL HasObject( void* _pObject ) const = 0;
	virtual UBOOL HasMethod( const void* _pMethodBits, INT _NumBytes ) const = 0;
};

/**
 * DISHONORED(port): FRealCallableMethod<Object,Object,Param> - an object plus a pointer to one of its methods. Its three
 * virtuals are the vtable the dispatcher calls through: slot 0 invokes (2012 rvas 0x6b5fd0, 0x791df0), slot 1 answers
 * "is this your object" (0x6b5fe0) and slot 2 "is this your method" (0x76df50, 0x8fa960), which is how UnregisterToEvent
 * identifies the entry to drop.
 */
template< class ObjectType, typename ParamType >
class FRealCallableMethod : public FArkCallableMethodImpl< ParamType >
{
public:
	typedef void (ObjectType::*MethodType)( ParamType );

	FRealCallableMethod( ObjectType* _pObject, MethodType _pMethod )
		: m_pObject( _pObject )
		, m_pMethod( _pMethod )
	{}

	virtual void Invoke( ParamType _Param ) const { ( m_pObject->*m_pMethod )( _Param ); }
	virtual UBOOL HasObject( void* _pObject ) const { return m_pObject == _pObject; }
	virtual UBOOL HasMethod( const void* _pMethodBits, INT _NumBytes ) const
	{
		return _NumBytes == sizeof( MethodType ) && appMemcmp( &m_pMethod, _pMethodBits, sizeof( MethodType ) ) == 0;
	}

	ObjectType*	m_pObject;
	MethodType	m_pMethod;
};

/**
 * DISHONORED(port): a 16-byte inline buffer holding one FRealCallableMethod, vtable pointer first - which is why the
 * dispatcher can call, test and bitwise-compare a registration without knowing the listener's class. Retail zeroes the
 * buffer before writing so that the appMemcmp its pending lists use is well defined; so does this.
 */
template< typename ParamType >
class FArkCallableMethod
{
public:
	FArkCallableMethod() { appMemzero( m_pBuffer, sizeof( m_pBuffer ) ); }

	template< class ObjectType > void Set( ObjectType* _pObject, void (ObjectType::*_pMethod)( ParamType ) )
	{
		typedef FRealCallableMethod< ObjectType, ParamType > RealType;
		checkAtCompileTime( sizeof( RealType ) <= 16, FArkCallableMethod_buffer_too_small );
		appMemzero( m_pBuffer, sizeof( m_pBuffer ) );
		RealType Real( _pObject, _pMethod );
		appMemcpy( m_pBuffer, &Real, sizeof( RealType ) );
	}

	UBOOL IsValid() const { return *(void* const*)m_pBuffer != NULL; }
	void operator()( ParamType _Param ) const { Impl().Invoke( _Param ); }
	UBOOL HasObject( void* _pObject ) const { return Impl().HasObject( _pObject ); }

	template< class ObjectType > UBOOL HasMethod( void (ObjectType::*_pMethod)( ParamType ) ) const
	{
		return Impl().HasMethod( &_pMethod, sizeof( _pMethod ) );
	}

	UBOOL operator==( const FArkCallableMethod& _rOther ) const
	{
		return appMemcmp( m_pBuffer, _rOther.m_pBuffer, sizeof( m_pBuffer ) ) == 0;
	}

	BYTE m_pBuffer[16];

private:
	const FArkCallableMethodImpl< ParamType >& Impl() const
	{
		return *(const FArkCallableMethodImpl< ParamType >*)m_pBuffer;
	}
};

typedef FArkCallableMethod< const FArkGameEvent& > FArkGameEventCallback;

// DISHONORED(layout): 2012 PDB FArkPendingEvent, 24 bytes: the event type, the per-object sender key (0 for a global
// registration) and the 16-byte callable.
class FArkPendingEvent
{
public:
	FArkPendingEvent()
		: m_iEventType( 0 )
		, m_pSender( 0 )
	{}

	FArkPendingEvent( INT _EventType, void* _pSender, const FArkGameEventCallback& _rCallback )
		: m_iEventType( _EventType )
		, m_pSender( (PTRINT)_pSender )
		, m_Callback( _rCallback )
	{}

	INT						m_iEventType;
	PTRINT					m_pSender;
	FArkGameEventCallback	m_Callback;
};

class FArkGameEventDispatcher
{
public:
	// DISHONORED(port): 2013 rvas 0x54df00 / 0x5572d0 / 0x557230 (2012 0x58ead0 / 0x597a90 / 0x5979f0)
	static FArkGameEventDispatcher* GetInstance() { return s_pInstance; }
	static void CreateInstance();
	FArkGameEventDispatcher();

	// DISHONORED(port): 2012 rva 0x597510
	void ProcessEvent( const FArkGameEvent& _rEvent );
	// DISHONORED(port): 2012 rva 0x5974c0
	void CheckLeaks();

	/**
	 * DISHONORED(port): 2013 rva 0x736ca0 (2012 0x72ef20), one instantiation per listener class. Registers a global
	 * listener for one event type, or defers the registration while that type is being dispatched.
	 */
	template< class ObjectType >
	void RegisterToEvent( const INT& _EventType, ObjectType* _pObject, void (ObjectType::*_pMethod)( const FArkGameEvent& ) )
	{
		FArkGameEventCallback Callback;
		Callback.Set( _pObject, _pMethod );
		if( IsBeingProcessed( _EventType ) )
		{
			m_PendingRegistrations.AddItem( FArkPendingEvent( _EventType, NULL, Callback ) );
		}
		else
		{
			m_RegistrationTable[ _EventType ].AddItem( Callback );
		}
	}

	/**
	 * DISHONORED(port): 2013 rva 0x6edf80 (2012 0x72ec40). A registration that has not been applied yet is simply
	 * cancelled; a live one is dropped at once, or queued for removal while its event type is being dispatched.
	 */
	template< class ObjectType >
	void UnregisterToEvent( const INT& _EventType, ObjectType* _pObject, void (ObjectType::*_pMethod)( const FArkGameEvent& ) )
	{
		for( INT i = 0; i < m_PendingRegistrations.Num(); ++i )
		{
			const FArkPendingEvent& Pending = m_PendingRegistrations(i);
			if( Pending.m_iEventType == _EventType && Pending.m_pSender == 0
				&& Pending.m_Callback.HasObject( _pObject ) && Pending.m_Callback.HasMethod( _pMethod ) )
			{
				m_PendingRegistrations.Remove( i, 1 );
				break;
			}
		}

		TArray< FArkGameEventCallback >& Registrations = m_RegistrationTable[ _EventType ];
		for( INT i = 0; i < Registrations.Num(); ++i )
		{
			if( !Registrations(i).HasObject( _pObject ) || !Registrations(i).HasMethod( _pMethod ) )
			{
				continue;
			}
			if( !IsBeingProcessed( _EventType ) )
			{
				Registrations.Remove( i, 1 );
			}
			else if( !IsUnregistrationPending( _EventType, NULL, Registrations(i) ) )
			{
				m_PendingUnregistrations.AddItem( FArkPendingEvent( _EventType, NULL, Registrations(i) ) );
			}
			return;
		}
	}

	/**
	 * DISHONORED(port): 2013 rva 0x7250b0 (2012 0x6cc820). Registers a listener for one event type raised by one
	 * instigator only; the sender pointer is the multi-map key.
	 */
	template< class ObjectType >
	void RegisterToObjectEvent( const INT& _EventType, void* _pSender, ObjectType* _pObject, void (ObjectType::*_pMethod)( const FArkGameEvent& ) )
	{
		FArkGameEventCallback Callback;
		Callback.Set( _pObject, _pMethod );
		if( IsBeingProcessed( _EventType ) )
		{
			m_PendingRegistrations.AddItem( FArkPendingEvent( _EventType, _pSender, Callback ) );
		}
		else
		{
			m_PerObjectRegistrationTable[ _EventType ].Add( (PTRINT)_pSender, Callback );
		}
	}

	/** DISHONORED(port): 2013 rva 0x67a190 (2012 0x784940 / 0x6cc540). */
	template< class ObjectType >
	void UnregisterToObjectEvent( const INT& _EventType, void* _pSender, ObjectType* _pObject, void (ObjectType::*_pMethod)( const FArkGameEvent& ) )
	{
		TArray< FArkGameEventCallback > Registered;
		m_PerObjectRegistrationTable[ _EventType ].MultiFind( (PTRINT)_pSender, Registered, FALSE );
		for( INT i = 0; i < Registered.Num(); ++i )
		{
			if( !Registered(i).HasObject( _pObject ) || !Registered(i).HasMethod( _pMethod ) )
			{
				continue;
			}
			if( !IsBeingProcessed( _EventType ) )
			{
				m_PerObjectRegistrationTable[ _EventType ].RemoveSinglePair( (PTRINT)_pSender, Registered(i) );
				return;
			}
			if( !IsUnregistrationPending( _EventType, _pSender, Registered(i) ) )
			{
				m_PendingUnregistrations.AddItem( FArkPendingEvent( _EventType, _pSender, Registered(i) ) );
			}
			break;
		}

		for( INT i = 0; i < m_PendingRegistrations.Num(); ++i )
		{
			const FArkPendingEvent& Pending = m_PendingRegistrations(i);
			if( Pending.m_iEventType == _EventType && Pending.m_pSender == (PTRINT)_pSender
				&& Pending.m_Callback.HasObject( _pObject ) && Pending.m_Callback.HasMethod( _pMethod ) )
			{
				m_PendingRegistrations.Remove( i, 1 );
				break;
			}
		}
	}

private:
	/** TRUE while an event of that type is on the dispatch stack, which is what makes a change deferred. */
	UBOOL IsBeingProcessed( INT _EventType ) const
	{
		return m_EventTypeBeingProcessed.FindItemIndex( _EventType ) != INDEX_NONE;
	}

	UBOOL IsUnregistrationPending( INT _EventType, void* _pSender, const FArkGameEventCallback& _rCallback ) const;
	void FlushPendingChanges( INT _EventType );

	static FArkGameEventDispatcher* s_pInstance;

	TArray< FArkPendingEvent >					m_PendingUnregistrations;
	TArray< FArkPendingEvent >					m_PendingRegistrations;
	TArray< FArkGameEventCallback >				m_RegistrationTable[ ARK_GAME_EVENT_TYPE_COUNT ];
	TMultiMap< PTRINT, FArkGameEventCallback >	m_PerObjectRegistrationTable[ ARK_GAME_EVENT_TYPE_COUNT ];
	TArray< INT >								m_EventTypeBeingProcessed;
};
