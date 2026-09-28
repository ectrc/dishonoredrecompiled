// DishonoredGame/src/disaisubstatestareatunreachable.cpp
// ---- agent DF ports (PHASE10 DF): UDisAISubStateStareAtUnreachable and its parameter ----

#include "DishonoredGame.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"
#include "aistimstruct.h"
#include "dishonoredutilities_ai.h"

/*-----------------------------------------------------------------------------
	FDisAISubStateStareAtUnreachable_Param
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x782140
FDisAISubStateStareAtUnreachable_Param::FDisAISubStateStareAtUnreachable_Param( const FDisAttentionProxy& _rUnreachableProxy )
	: FDisAISubState_Param( UDisAISubStateStareAtUnreachable::StaticClass() )
	, m_UnreachableProxy( _rUnreachableProxy )
{
}

// DISHONORED(port): 2012 rva 0x769490 (folded with FDisAISubStateMenace_Param::OnPending: both write one proxy at
// sub-state offset 208)
void FDisAISubStateStareAtUnreachable_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FDisAISubState_Param::OnPending( PendingState, ManagedObject );
	( (UDisAISubStateStareAtUnreachable*)PendingState )->m_UnreachableProxy = m_UnreachableProxy;
}

/*-----------------------------------------------------------------------------
	UDisAISubStateStareAtUnreachable
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2012 rva 0x7658e0. The whole class: look at it with head and torso for as long as it takes, and face
// it. There is no tick and no exit condition of its own - the behaviour's RefreshCallback decides when to give up.
void UDisAISubStateStareAtUnreachable::BeginSubState_Derived()
{
	SetLookAtProxyDesire( m_UnreachableProxy, FDisLookAtInfluence::Torso, -1.f );
	SetFaceToProxyDesire( m_UnreachableProxy, -100.f, FALSE );
}
