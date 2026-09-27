#pragma once
// DishonoredGame/inc/disaisubstate.h
// DISHONORED(written): the state-change parameter of the AI sub-state machine. Retail declares it beside UDisAISubState
// (2012 PDB FDisAISubState_Param, 20 bytes: FDisNativeStateParam + FDisBodyIntention @8); it is the only native type of
// disaisubstate.cpp that call sites outside the unit need, because UDisAISubStateMachine::OnOwningBehaviorStop and every
// behaviour that requests a sub-state build one. Written by agent CG (sub-task CG-substate).
#ifndef _INC_DISAISUBSTATE
#define _INC_DISAISUBSTATE

// DISHONORED(port): 2013 rva of OnPending 0x705820 (2012 0x768440), ctor 2012 0x768420. OnPending runs while the state
// is pending, before OnEnterState: it plants the owning behaviour and its brain on the state and carries the body
// intention the pawn had when the change was requested, so OnEnterState can resume the desires against it.
struct FDisAISubState_Param : public FDisNativeStateParam
{
	FDisBodyIntention m_BodyIntentionBeforeEnterState;

	FDisAISubState_Param( UClass* StateClass = NULL ) : FDisNativeStateParam( StateClass )
	{
		m_BodyIntentionBeforeEnterState.m_IntendedBodyStance = 0;
		m_BodyIntentionBeforeEnterState.m_pDesiredPrimaryItemClass = NULL;
		m_BodyIntentionBeforeEnterState.m_pDesiredSecondaryItemClass = NULL;
	}

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubState_Param); }
};

/** DISHONORED(layout): 2012 PDB FDisAISubStateInit_Param, 20 bytes, derives from FDisAISubState_Param and adds
    nothing - the idle sub-state takes no parameters of its own. Retail's name is kept because UDishonoredAIBehavior::
    CallInitBehavior (2013 rva 0x6f7030) constructs it by that name. */
typedef FDisAISubState_Param FDisAISubStateInit_Param;

#endif
