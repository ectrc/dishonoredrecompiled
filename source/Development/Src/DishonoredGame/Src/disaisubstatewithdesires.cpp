// DishonoredGame/src/disaisubstatewithdesires.cpp
// ---- agent DF ports (PHASE10 DF): the three "with desires" mix-ins ----
//
// Retail gives each of them its own unit (disaisubstatewithdesires.cpp, disaibehaviorwithdesires.cpp and the sub-process
// equivalent) because each is an unrelated class; they are two real accessors and four folded constants each, so they
// share one unit here. What they add is the answer to three questions the desire layer asks of whoever owns a desire:
// which UObject am I, whose pawn do I move, and at which priority band do I speak.
//
// DISHONORED(retail): the priority bands are why all three exist. A sub-process outranks a sub-state, which outranks its
// behaviour, so a head-track sub-process can keep an NPC glancing at a distraction while the sub-state below it is still
// walking to a position the behaviour below that picked. The twelve constants are identical-code-folded in both builds
// and were read off the vtables' folded targets, then cross-checked against EDisFaceToPriority / EDisLocoPriority /
// EDisLookAtPriority / EDisBodyIntentionPriority: all twelve land on their own class's enumerator, which is also what
// settles the declaration order of the four slots (see Inc/CppText/UDisAISubStateWithDesires.h).

#include "DishonoredGame.h"
#include "disdesirestructs.h"

/*-----------------------------------------------------------------------------
	UDisAISubStateWithDesires
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 rva 0x728150 (2012 0x765ac0)
IDisDesiresInterface* UDisAISubStateWithDesires::GetDesires()
{
	return this;
}

// DISHONORED(port): 2013 rva 0x702800 (2012 0x766b80): retail's body is `return this - 204`, i.e. the reverse of the
// interface sub-object offset; the compiler generates the same adjustment for `return this` in the class's own scope.
UObject* UDisAISubStateWithDesires::GetUObjectInterfaceDisDesiresInterface()
{
	return this;
}

// DISHONORED(port): 2013 rva 0x728160 (2012 0x765ad0)
ADishonoredNPCPawn* UDisAISubStateWithDesires::GetDesiresOwningPawn()
{
	return m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
}

// DISHONORED(port): 2013 rva 0x728170 (2012 0x765ae0)
void UDisAISubStateWithDesires::DesiresFaceToEventCallback( BYTE _FaceToEvent )
{
	HandleFaceToEvent( _FaceToEvent );
}

// DISHONORED(port): 2013 rva 0x728180 (2012 0x765af0): the request id retail is handed is deliberately ignored - a
// sub-state has at most one loco request at a time, so the event can only be about that one.
void UDisAISubStateWithDesires::DesiresLocoEventCallback( INT _iRequestID, BYTE _LocoEvent )
{
	HandleLocoEvent( _LocoEvent );
}

/*-----------------------------------------------------------------------------
	UDisAIBehaviorWithDesires
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x722460
IDisDesiresInterface* UDisAIBehaviorWithDesires::GetDesires()
{
	return this;
}

UObject* UDisAIBehaviorWithDesires::GetUObjectInterfaceDisDesiresInterface()
{
	return this;
}

// DISHONORED(port): 2012 rva 0x722470
ADishonoredNPCPawn* UDisAIBehaviorWithDesires::GetDesiresOwningPawn()
{
	return m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
}

// DISHONORED(port): 2012 rvas 0x722480 / 0x722490
void UDisAIBehaviorWithDesires::DesiresFaceToEventCallback( BYTE _FaceToEvent )
{
	HandleFaceToEvent( _FaceToEvent );
}

void UDisAIBehaviorWithDesires::DesiresLocoEventCallback( INT _iRequestID, BYTE _LocoEvent )
{
	HandleLocoEvent( _LocoEvent );
}

/*-----------------------------------------------------------------------------
	UDisAISubProcessWithDesires
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x78d280
IDisDesiresInterface* UDisAISubProcessWithDesires::GetDesires()
{
	return this;
}

// DISHONORED(port): 2012 rva 0x766990 (`return this - 104`)
UObject* UDisAISubProcessWithDesires::GetUObjectInterfaceDisDesiresInterface()
{
	return this;
}

// DISHONORED(port): 2012 rva 0x78d290
ADishonoredNPCPawn* UDisAISubProcessWithDesires::GetDesiresOwningPawn()
{
	return m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
}

// DISHONORED(port): 2012 rvas 0x78d2a0 / 0x78d2b0
void UDisAISubProcessWithDesires::DesiresFaceToEventCallback( BYTE _FaceToEvent )
{
	HandleFaceToEvent( _FaceToEvent );
}

void UDisAISubProcessWithDesires::DesiresLocoEventCallback( INT _iRequestID, BYTE _LocoEvent )
{
	HandleLocoEvent( _LocoEvent );
}
