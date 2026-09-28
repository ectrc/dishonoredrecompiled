// DishonoredGame/src/disbehavioridle.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).

// ---- agent CG ports (PHASE9 CG): the idle behaviour's stim table ----
//
// This is the smallest complete example of how a behaviour is activated, and it is the one that matters most: it is the
// behaviour every NPC falls back to, and the only one that answers the BrainInit stim UDishonoredAIBrain::InitBrain
// raises. Without it a brain finishes initialising with 21 constructed behaviours and an empty active stack, which is
// exactly what the -disai census reported before this file: "26 initialized, 0 activations".
//
// ---- agent DF ports (PHASE10 DF): the rest of it ----
//
// And this is where an NPC stops standing in DisAISubStateInit. OnBehaviorResume is called the moment the idle behaviour
// takes slot 0 (UDishonoredAIBehavior::CallOnBehaviorResume, 2013 rva 0x6f7670) and it asks its machine for slot 0's
// sub-state - UDisAISubStateStand - with the pawn's own position and rotation. Everything the -disai census now reports
// beyond "substate DisAISubStateInit" comes through these six functions.

#include "DishonoredGame.h"
#include "disdelegate.h"
#include "aistimstruct.h"
#include "disaisubstate.h"
#include "disdesirestructs.h"

/** DISHONORED(port): the shared "yes" predicate retail calls s_DelegateReturnTrue. A behaviour that wants a stim
    unconditionally hands this out instead of a bound method, which is why the idle behaviour needs no Evaluate* member
    of its own. */
static UBOOL DisStimAlwaysTrue( void* /*_pObject*/, const FAIStimStruct& /*_rStim*/ )
{
	return TRUE;
}

static const FDisStimPredicateDelegate GDisDelegateReturnTrue( NULL, &DisStimAlwaysTrue );

// DISHONORED(port): 2013 rva 0x6e3900 (2012 0x723330): the mask is a static built once per class and shared, which is
// why retail guards it with its own initialised flag rather than rebuilding it per behaviour.
const BYTE* UDisBehaviorIdle::BuildEvaluateStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_BrainInit, EAIStimID_IdleRequest };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2013 rva 0x6e6650 (2012 0x7284f0): the idle behaviour takes the brain-init and the idle-request stim
// unconditionally and nothing else. Every other stim gets the null delegate, which the brain reads as "not interested".
FDisStimPredicateDelegate UDisBehaviorIdle::GetEvaluateStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_BrainInit || _StimID == EAIStimID_IdleRequest )
	{
		return GDisDelegateReturnTrue;
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2012 rva 0x723380 (agent DF): the one stim the idle behaviour itself filters while it is running.
const BYTE* UDisBehaviorIdle::BuildFilterStimMask()
{
	static const BYTE StimIDs[] = { EAIStimID_Teleported };
	static FDisStimFilterMask s_Mask;
	return s_Mask.Build( StimIDs, ARRAY_COUNT(StimIDs) );
}

// DISHONORED(port): 2012 rva 0x7436b0
FDisStimPredicateDelegate UDisBehaviorIdle::GetFilterStimDelegate( BYTE _StimID )
{
	if( _StimID == EAIStimID_Teleported )
	{
		return DIS_BIND_STIM_PREDICATE( UDisBehaviorIdle, FAIStimStruct, FilterTeleported );
	}
	return FDisStimPredicateDelegate();
}

// DISHONORED(port): 2012 rva 0x7410c0: a teleported NPC is standing somewhere else, so the Stand sub-state's whole
// position is re-stated from where the pawn now is.
UBOOL UDisBehaviorIdle::FilterTeleported( const FAIStimStruct& _rStim )
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	if( !Pawn )
	{
		return FALSE;
	}
	FDisAISubStateStand_Param Params( Pawn->Location, Pawn->Rotation, FALSE, FALSE, FALSE, eDisNPCBodyStance_None );
	RequestSubStateChange< UDisTweaks_AIBehavior_Idle, UDisTweaks_AISubState_Stand >( 0, Params );
	return FALSE;
}

// DISHONORED(port): 2012 rva 0x728530: the behaviour's own mask, with the idle-request bit added so the behaviour can be
// re-asked for while it is already running, then the internal roll-up over its sub-processes and sub-states.
const BYTE* UDisBehaviorIdle::BuildBehaviorFilterStimMasks( const BYTE*& _rOutSubProcessesMask, const BYTE*& _rOutCompleteMask )
{
	static BYTE s_Mask[EAIStimID_MAX];
	static BYTE s_SubProcessesMask[EAIStimID_MAX];
	static BYTE s_CompleteMask[EAIStimID_MAX];
	static UBOOL s_bMaskInitialized = FALSE;
	static UBOOL s_bInternalInitialized = FALSE;

	if( !s_bMaskInitialized )
	{
		s_bMaskInitialized = TRUE;
		const BYTE* Filter = BuildFilterStimMask();
		appMemcpy( s_Mask, Filter, sizeof(s_Mask) );
		s_Mask[EAIStimID_IdleRequest] |= 1;
	}
	if( !s_bInternalInitialized )
	{
		s_bInternalInitialized = TRUE;
		appMemcpy( s_CompleteMask, s_Mask, sizeof(s_CompleteMask) );
		BuildInternalFilterStimMasks( s_SubProcessesMask, s_CompleteMask );
	}
	_rOutSubProcessesMask = s_SubProcessesMask;
	_rOutCompleteMask = s_CompleteMask;
	return s_Mask;
}

// DISHONORED(port): 2012 rva 0x741070. THE transition: the idle behaviour has just taken slot 0, so it asks its machine
// for that slot's sub-state - UDisAISubStateStand - with the pawn's current position and rotation as the post to hold.
// Retail asks for no accurate stop, no exact rotation, no strafing and no particular stance, i.e. "stand where you are".
void UDisBehaviorIdle::OnBehaviorResume()
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	if( !Pawn )
	{
		return;
	}
	FDisAISubStateStand_Param Params( Pawn->Location, Pawn->Rotation, FALSE, FALSE, FALSE, eDisNPCBodyStance_None );
	RequestSubStateChange< UDisTweaks_AIBehavior_Idle, UDisTweaks_AISubState_Stand >( 0, Params );
}

// DISHONORED(port): 2012 rva 0x72b920. The idle behaviour's per-frame job is one thing: keep the standing sub-state's
// facing in step with the conversation system. A speaker the NPC is meant to turn to becomes a face-to desire at the
// BEHAVIOUR's priority (below the sub-state's), and the sub-state's own rotation focus is switched off while that lasts -
// which is what DisableRotationFocus is for and its only caller.
void UDisBehaviorIdle::TickBehavior( FLOAT _fDeltaSeconds )
{
	ADishonoredNPCPawn* Pawn = m_pOwningBrain ? m_pOwningBrain->m_pOwningPawn : NULL;
	if( !Pawn )
	{
		return;
	}

	// DISHONORED(bringup): retail asks the pawn's IDisConvSpeakerInterface::GetConvLookTarget (2012 rva 0x8f9750), which
	// reads UDisConversationComponent::GetLookTarget. The conversation system is not ported - IDisConvSpeakerInterface is a
	// 30-slot interface in retail and declaring one of its slots here would put a false vtable in the tree - so there is
	// never a conversation look target and this takes retail's "not speaking to anyone" branch every time: the behaviour's
	// face-to desire is cleared and the Stand sub-state keeps its own rotation focus. That is what an NPC standing alone
	// does in retail too, which is why the idle behaviour is still observable without it.
	const AActor* LookTarget = NULL;
	{
		static UBOOL bNoted = FALSE;
		if( !bNoted )
		{
			bNoted = TRUE;
			debugf( NAME_Warning, TEXT("DISHONORED(bringup): IDisConvSpeakerInterface::GetConvLookTarget is not ported; the idle behaviour never turns to a speaker") );
		}
	}
	ClearFaceToDesire();

	UDisAISubStateStand* Stand = (UDisAISubStateStand*)m_pBehaviorFSM->GetSubState( UDisAISubStateStand::StaticClass() );
	if( Stand )
	{
		const UBOOL bDisable = ( LookTarget != NULL );
		if( Stand->IsRotationFocusDisabled() != bDisable )
		{
			Stand->DisableRotationFocus( bDisable );
		}
	}
}
