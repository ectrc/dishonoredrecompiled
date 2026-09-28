// Engine/src/arkgameeventdispatcher.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x58ead0  public: static class FArkGameEventDispatcher * __cdecl FArkGameEventDispatcher::GetInstance(void)
//   0x5974c0  public: void __thiscall FArkGameEventDispatcher::CheckLeaks(void)
//   0x597510  public: void __thiscall FArkGameEventDispatcher::ProcessEvent(class FArkGameEvent const &)
//   0x5979f0  public: __thiscall FArkGameEventDispatcher::FArkGameEventDispatcher(void)
//   0x597a90  public: static void __cdecl FArkGameEventDispatcher::CreateInstance(void)

#include "EnginePrivate.h"

FArkGameEventDispatcher* FArkGameEventDispatcher::s_pInstance = NULL;

// DISHONORED(bringup): agent DF. See the note beside the declarations in arkgameeventdispatcher.h.
INT GArkGameEventRegistrations = 0;
INT GArkGameEventUnregistrations = 0;
INT GArkGameEventPerObjectRegistrations = 0;
INT GArkGameEventDispatches = 0;
INT GArkGameEventCallbacksInvoked = 0;
INT GArkGameEventDeferred = 0;


// DISHONORED(port): 2013 rva 0x557230 (2012 0x5979f0): every table starts empty; the two fixed-size arrays are
// default-constructed in place (retail's eh vector constructor iterator over 79 entries of 12 and 60 bytes).
FArkGameEventDispatcher::FArkGameEventDispatcher()
{
}

// DISHONORED(port): 2013 rva 0x5572d0 (2012 0x597a90): one allocation of the whole dispatcher (0x165C bytes in retail),
// and s_pInstance stays NULL when it fails.
void FArkGameEventDispatcher::CreateInstance()
{
	if( !s_pInstance )
	{
		s_pInstance = new FArkGameEventDispatcher;
	}
}

// DISHONORED(port): 2012 rva 0x597510. The event type is pushed on the dispatch stack, then every global registration for
// that type is called, then every registration made for this event's instigator. The type is popped, and when it is no
// longer anywhere on the stack (i.e. this was the outermost dispatch of it) the changes that were deferred while it ran
// are applied. Both loops re-read Num() every iteration, exactly as retail's do, so a handler that registers another
// listener of the same type from outside a dispatch is still reached.
void FArkGameEventDispatcher::ProcessEvent( const FArkGameEvent& _rEvent )
{
	const INT EventType = _rEvent.m_Type;
	if( EventType < 0 || EventType >= ARK_GAME_EVENT_TYPE_COUNT )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): FArkGameEventDispatcher::ProcessEvent: event type %d is outside the %d retail slots"), EventType, (INT)ARK_GAME_EVENT_TYPE_COUNT );
		return;
	}

	GArkGameEventDispatches++;
	m_EventTypeBeingProcessed.AddItem( EventType );

	TArray< FArkGameEventCallback >& Registrations = m_RegistrationTable[ EventType ];
	for( INT i = 0; i < Registrations.Num(); ++i )
	{
		GArkGameEventCallbacksInvoked++;
		Registrations(i)( _rEvent );
	}

	TArray< FArkGameEventCallback > PerObjectRegistrations;
	m_PerObjectRegistrationTable[ EventType ].MultiFind( (PTRINT)_rEvent.m_pInstigator, PerObjectRegistrations, FALSE );
	for( INT i = 0; i < PerObjectRegistrations.Num(); ++i )
	{
		GArkGameEventCallbacksInvoked++;
		PerObjectRegistrations(i)( _rEvent );
	}

	m_EventTypeBeingProcessed.Pop();
	if( !IsBeingProcessed( EventType ) )
	{
		FlushPendingChanges( EventType );
	}
}

// DISHONORED(written): the tail of retail's ProcessEvent, walked backwards over both pending lists. An unregistration is
// tried against the per-object map first and falls back to the global list when the map held no such pair; a registration
// with a sender goes into the per-object map, one without into the global list.
void FArkGameEventDispatcher::FlushPendingChanges( INT _EventType )
{
	for( INT i = m_PendingUnregistrations.Num() - 1; i >= 0; --i )
	{
		const FArkPendingEvent& Pending = m_PendingUnregistrations(i);
		if( Pending.m_iEventType != _EventType )
		{
			continue;
		}
		if( m_PerObjectRegistrationTable[ _EventType ].RemoveSinglePair( Pending.m_pSender, Pending.m_Callback ) <= 0 )
		{
			TArray< FArkGameEventCallback >& Registrations = m_RegistrationTable[ _EventType ];
			for( INT j = 0; j < Registrations.Num(); ++j )
			{
				if( Registrations(j) == Pending.m_Callback )
				{
					Registrations.Remove( j, 1 );
					break;
				}
			}
		}
		m_PendingUnregistrations.Remove( i, 1 );
	}

	for( INT i = m_PendingRegistrations.Num() - 1; i >= 0; --i )
	{
		const FArkPendingEvent Pending = m_PendingRegistrations(i);
		if( Pending.m_iEventType != _EventType )
		{
			continue;
		}
		if( Pending.m_pSender != 0 )
		{
			m_PerObjectRegistrationTable[ _EventType ].Add( Pending.m_pSender, Pending.m_Callback );
		}
		else
		{
			m_RegistrationTable[ _EventType ].AddItem( Pending.m_Callback );
		}
		m_PendingRegistrations.Remove( i, 1 );
	}
}

// DISHONORED(written): retail compares the 16-byte callable and the sender key, so registering the same method twice and
// unregistering it once queues only one removal.
UBOOL FArkGameEventDispatcher::IsUnregistrationPending( INT _EventType, void* _pSender, const FArkGameEventCallback& _rCallback ) const
{
	for( INT i = 0; i < m_PendingUnregistrations.Num(); ++i )
	{
		const FArkPendingEvent& Pending = m_PendingUnregistrations(i);
		if( Pending.m_iEventType == _EventType && Pending.m_pSender == (PTRINT)_pSender && Pending.m_Callback == _rCallback )
		{
			return TRUE;
		}
	}
	return FALSE;
}

// DISHONORED(written): 2012 rva 0x5974c0. Retail walks all 79 per-object maps and does nothing observable with them: the
// leak report itself is compiled out of the Shipping build, so its text is not recoverable. Ours keeps the walk and names
// the registrations that outlived the dispatcher, which is what the function is for.
void FArkGameEventDispatcher::CheckLeaks()
{
	INT NumLeaked = 0;
	for( INT EventType = 0; EventType < ARK_GAME_EVENT_TYPE_COUNT; ++EventType )
	{
		NumLeaked += m_PerObjectRegistrationTable[ EventType ].Num();
	}
	if( NumLeaked > 0 )
	{
		debugf( NAME_Warning, TEXT("DISHONORED(bringup): FArkGameEventDispatcher::CheckLeaks: %d per-object registrations were never removed"), NumLeaked );
	}
}
