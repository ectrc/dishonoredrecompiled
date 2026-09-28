#pragma once
// DishonoredGame/inc/disaisubstate.h
// DISHONORED(written): the state-change parameters of the AI sub-state machine. Retail declares each of them beside its own
// sub-state (disaisubstatestand.cpp and friends); they live in one header here because every behaviour that requests a
// sub-state builds one and there is no per-sub-state header in this tree.
//
// A _Param is the REQUEST half of a sub-state change, and the pair is the whole protocol: the constructor names the target
// sub-state class (m_pStateClass) and records the arguments, UDishonoredNativeStateMachine::RequestStateChange copies
// SizeOf() bytes of it, and OnPending then writes those arguments into the sub-state object itself just before
// OnEnterState runs. So the constructor is "what I am asking for" and OnPending is "putting it on the state". That is why
// agent AU's rule forbids porting one without the other: a param whose sub-state has no body makes a behaviour callback
// request a state that does nothing, which is worse than the stub it replaced.
//
// DISHONORED(layout): every OnPending body was read by translating the decompiler's `_pState[N].Field` spelling into a
// member offset with build/agentDF_work/off.py (stride 64 = sizeof(UDishonoredNativeState) in the 2012 build, UObject
// field offsets inside it) and looking the member up in the 2012 PDB; each one was then checked against the retail
// generated declaration, and all 25 agree member for member.
#ifndef _INC_DISAISUBSTATE
#define _INC_DISAISUBSTATE

/**
 * DISHONORED(bringup): where a melee NPC should stand and look. Retail asks UDisBehaviorCombatMelee::ComputeNewMeleePosition
 * (2012 rva 0x771290), which folds in the attack pattern, the formation slot and the other NPCs already engaged; that
 * behaviour is not ported, so this is the position it degenerates to with no formation - the proxy's own feet, looking at
 * its best target point. MaintainDistance, MeleeChase and MeleeEngage all go through this one function, so the whole melee
 * family becomes exact the moment UDisBehaviorCombatMelee lands. Defined in Src/disaisubstatemaintaindistance.cpp.
 */
FVector DisComputeMeleePosition( const struct FDisAttentionProxy& _rProxy, FVector& _rOutLookPosition );

/** DISHONORED(written): the FArkGameEvent type for "an actor was terminated", the literal 3 at every retail call site.
    Agents CG and DF each had a file static of this name; it lives here now because the sub-state parameters register and
    unregister with it too, and an event id with two definitions is an event id that can disagree with itself. */
static const INT GDisAIEvent_OtherActorTerminated = 3;

/**
 * DISHONORED(port): every sub-state, sub-process and behaviour answers "which stims do I even look at" with a byte per
 * EAIStimID, bit 0 set for the ones it filters. Retail builds it once per class into a file static guarded by its own
 * initialised flag and hands the same array to every instance (UDisAISubStateStand::BuildFilterStimMask, 2012 rva
 * 0x765840, is the shape of all of them), which is why UDisAISubState::m_pFilterStimMask is a raw pointer that a save
 * file cannot carry.
 *
 * DISHONORED(retail): the ids MUST be written as enumerators, never as the numbers the 2012 decompile's mask addresses
 * imply. Retail 2013's EAIStimID has 122 entries against 2012's 112: it inserts AttentionBehaviorBegin at 7 and
 * InhibitBegin / InhibitEnd at 53 and 54, which renumbers 105 of the 112 ids 2012 already had. Measured on Stand's own
 * mask: its four bytes sit 0, 9, 28 and 76 apart in the 2012 image, and its GetFilterStimDelegate_SubState names
 * DestinationReached, EndPossession, IncomingDamage and Teleported - 2013 ids 24, 33, 52, 102, i.e. 0, 9, 28 and 78
 * apart. The first three agree because nothing was inserted below 52; the fourth is two out, which is InhibitBegin and
 * InhibitEnd. Each mask below is therefore read from its class's delegate switch (which the PDB names symbolically) and
 * cross-checked against the bit count of the 2012 mask body.
 */
struct FDisStimFilterMask
{
	BYTE m_Mask[EAIStimID_MAX];
	UBOOL m_bInitialized;

	const BYTE* Build( const BYTE* _pStimIDs, INT _Count )
	{
		if( !m_bInitialized )
		{
			m_bInitialized = TRUE;
			appMemzero( m_Mask, sizeof(m_Mask) );
			for( INT i = 0; i < _Count; ++i )
			{
				m_Mask[ _pStimIDs[i] ] |= 1;
			}
		}
		return m_Mask;
	}
};

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

/** DISHONORED(port): ctor 2012 rva 0x77bc60. The idle sub-state takes no parameters of its own and shares
    FDisAISubState_Param's vtable (the 2012 ctor stores that vtable, not one of its own), so all it adds is naming
    UDisAISubStateInit as the target class - which is the whole reason it exists as a type.
    DISHONORED(retail): agent CG had this as a typedef of FDisAISubState_Param, which made every caller responsible for
    passing the class; retail's own callers (UDisBehaviorNotice::RequestStateExitCallback_GenericAction, 2013 rva
    0x6f2ba0, and UDisAISubStateMachine::OnOwningBehaviorStop) construct it with no arguments. */
struct FDisAISubStateInit_Param : public FDisAISubState_Param
{
	FDisAISubStateInit_Param();
	virtual INT SizeOf() const { return sizeof(FDisAISubStateInit_Param); }
};

/** DISHONORED(port): ctors 2012 rvas 0x781f90 (location + rotation), 0x782030 (location + focus actor), 0x7820b0
    (location + focus proxy); OnPending 0x7694e0 (209 bytes, the largest of the 25); SizeOf 0x782020 = 80.
    "Stand here, facing that." The three constructors are the three ways of saying what to face: a fixed yaw, an actor, or
    an attention proxy. m_bAlwaysStrafe means the NPC may face its focus while still walking to the spot; without it the
    facing only starts once it has arrived (UDisAISubStateStand::EnsureProperLocation). */
struct FDisAISubStateStand_Param : public FDisAISubState_Param
{
	FDisLocRot m_ActorPosition;
	AActor* m_pFocusActor;
	FDisAttentionProxy m_FocusProxy;
	BYTE m_eDesiredBodyStance;
	UBOOL m_bAccurateStop;
	UBOOL m_bExactRotation;
	UBOOL m_bAlwaysStrafe;

	FDisAISubStateStand_Param( const FVector& _rLocation, const FRotator& _rRotation, UBOOL _bAccurateStop, UBOOL _bExactRotation, UBOOL _bAlwaysStrafe, BYTE _eDesiredBodyStance );
	FDisAISubStateStand_Param( const FVector& _rLocation, AActor* _pFocusActor, UBOOL _bAccurateStop, UBOOL _bExactRotation, UBOOL _bAlwaysStrafe, BYTE _eDesiredBodyStance );
	FDisAISubStateStand_Param( const FVector& _rLocation, const FDisAttentionProxy& _rFocusProxy, UBOOL _bAccurateStop, UBOOL _bExactRotation, UBOOL _bAlwaysStrafe, BYTE _eDesiredBodyStance );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateStand_Param); }
};

/** DISHONORED(port): ctors 2012 rvas 0x7821e0 (destination only), 0x782260 (+ focus actor), 0x7822f0 (+ rotation angle);
    OnPending 0x777980. "Walk to there and stop like this." The stop type is the arrival contract - Anim stops on the
    animation, Accurate stops on the spot, Continuous does not stop at all - and m_eTakePosRotationTarget records which of
    the three rotation forms the caller used, which is the one field OnPending derives rather than copies. */
struct FDisAISubStateTakePosition_Param : public FDisAISubState_Param
{
	FVector m_vDestination;
	AActor* m_pFocusTarget;
	INT m_iRotationTargetAngle;
	BYTE m_StopType;
	UBOOL m_bRotationSetByAngle;
	UBOOL m_bFullSpeed;
	UBOOL m_bExactRotation;

	FDisAISubStateTakePosition_Param( FVector _vDestination, BYTE _StopType, UBOOL _bFullSpeed );
	FDisAISubStateTakePosition_Param( FVector _vDestination, BYTE _StopType, UBOOL _bFullSpeed, AActor* const _pFocusTarget, UBOOL _bExactRotation );
	FDisAISubStateTakePosition_Param( FVector _vDestination, BYTE _StopType, UBOOL _bFullSpeed, FRotator _RotationTargetAngle, UBOOL _bExactRotation );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateTakePosition_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x7846e0, OnPending 0x77bee0. "Walk to where that actor is, and keep at least this far
    from it." It takes the actor's position NOW as its destination (so the base param's copy is a snapshot) and then keeps
    the actor, which is what lets the sub-state re-path as the actor moves. */
struct FDisAISubStateTakeActorPosition_Param : public FDisAISubStateTakePosition_Param
{
	AActor* m_pDestinationActor;
	FLOAT m_fMinAllowedDistanceFromActor;

	FDisAISubStateTakeActorPosition_Param( AActor* _pDestinationActor, BYTE _StopType, UBOOL _bFullSpeed, FLOAT _fMinAllowedDistanceFromActor );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateTakeActorPosition_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x77bbb0, OnPending 0x768d50. "Play this action." The action id overrides the one the
    slot's tweaks name; m_GroupIdx is set to INDEX_NONE so the sub-state picks its own group. */
struct FDisAISubStateGenericAction_Param : public FDisAISubState_Param
{
	BYTE m_OverriddenActionID;
	INT m_StartingAnimStep;
	UBOOL m_bUseDualPlayAnim;

	FDisAISubStateGenericAction_Param( BYTE _ActionID, UBOOL _bUseDualPlayAnim, INT _StartingAnimStep );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateGenericAction_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x77bc10, OnPending 0x768db0. The same sub-state class asked for differently: "do
    nothing, but be in the generic-action state of this group". It sets only the group, leaving the action id as the
    tweaks left it, which is how a behaviour parks an NPC in an idle animation group. */
struct FDisAISubStateGenericAction_Nothing_Param : public FDisAISubState_Param
{
	INT m_iCustomGroup;

	FDisAISubStateGenericAction_Nothing_Param( INT _iCustomGroup );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateGenericAction_Nothing_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x782140, OnPending 0x769490. "Stand and stare at something you cannot reach." */
struct FDisAISubStateStareAtUnreachable_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_UnreachableProxy;

	FDisAISubStateStareAtUnreachable_Param( const FDisAttentionProxy& _rUnreachableProxy );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateStareAtUnreachable_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781f10. "Threaten that." Shares FDisAISubStateMenace_Param's vtable with
    StareAtUnreachable's in the 2012 build (both OnPending bodies write one proxy at the same offset and were folded). */
struct FDisAISubStateMenace_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_EnemyProxy;

	FDisAISubStateMenace_Param( const FDisAttentionProxy& _rEnemyProxy );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateMenace_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781e80, OnPending 0x7693b0, SizeOf 0x781f00 = 40. "Keep this far from that." */
struct FDisAISubStateMaintainDistance_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_Focus;
	FLOAT m_fIdealDistance;

	FDisAISubStateMaintainDistance_Param( FLOAT _fIdealDistance, const FDisAttentionProxy& _rFocus );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateMaintainDistance_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x782380, OnPending 0x769650. "Follow that to where it went." */
struct FDisAISubStateTrackTarget_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_TrackTargetProxy;
	FLOAT m_fMaxFunnelRadiusMultiplier;

	FDisAISubStateTrackTarget_Param( const FDisAttentionProxy& _rTrackTargetProxy, FLOAT _fMaxFunnelRadiusMultiplier );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateTrackTarget_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781d60, OnPending 0x768e70, SizeOf 0x781e10 = 68. "Go and look at what you noticed,
    and tell me afterwards which hook to fire." m_InvestigateStartReason is the attention change that caused it, which the
    sub-state hands back to the behaviour so the bark matches the cause. */
struct FDisAISubStateInvestigate_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_InvestigateTargetProxy;
	FDisAttentionChangeReason m_InvestigateStartReason;
	BYTE m_InvestigateHookOutput;
	UBOOL m_bRunningInvestigate;
	FLOAT m_fReactionDelayTime;
	FLOAT m_fMaxFunnelRadiusMultiplier;

	FDisAISubStateInvestigate_Param( const FDisAttentionProxy& _rAttentionProxy, UBOOL _bRunningInvestigate, FDisAttentionChangeReason _InvestigateStartReason, BYTE _InvestigateHookOutput, FLOAT _fReactionDelayTime, FLOAT _fMaxFunnelRadiusMultiplier );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateInvestigate_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781c80, OnPending 0x78c810. "Run away from that." OnPending is the only one of the 25
    that calls a method of its sub-state (UDisAISubStateFlee::SetThreat) rather than writing members, because setting the
    threat also subscribes to its termination. */
struct FDisAISubStateFlee_Param : public FDisAISubState_Param
{
	AActor* m_pThreat;
	UBOOL m_bForReal;

	FDisAISubStateFlee_Param( AActor* _pThreat, UBOOL _bForReal );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateFlee_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x7819c0, OnPending 0x768690. "Cower from that, behind this flee component." */
struct FDisAISubStateCower_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_EnemyProxy;
	class UDisFleeComponent* m_pCurrentFleeComponent;

	FDisAISubStateCower_Param( const FDisAttentionProxy& _rEnemyProxy, class UDisFleeComponent* const _pCurrentFleeComponent );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateCower_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781e20, OnPending 0x769200. "Lie in wait at that ambush point." */
struct FDisAISubStateLieInWait_Param : public FDisAISubState_Param
{
	class ADisAmbushPoint* m_pAmbushPoint;
	UBOOL m_bTeleported;
	UBOOL m_bLieInWaitIndefinitely;

	FDisAISubStateLieInWait_Param( class ADisAmbushPoint* _pAmbushPoint, UBOOL _bTeleported, UBOOL _bLieInWaitIndefinitely );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateLieInWait_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781a30. "Cast the attract spell at that." Its OnPending is folded with
    FDisAISubStateMenace_Param's, and the two write the same single proxy at the same offset. */
struct FDisAISubStateDoAttractSpell_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_TargetProxy;

	FDisAISubStateDoAttractSpell_Param( const FDisAttentionProxy& _rTargetProxy );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateDoAttractSpell_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781ab0, OnPending 0x7686e0. "Use a weapon manoeuvre." The only argument is whether
    the precondition should honour the cooldown, and the 2012 constructor does not set it - it is the caller's job
    (m_bCheckCooldownOnPrecondition is written after construction at every retail call site). */
struct FDisAISubStateDoWeaponManoeuver_Param : public FDisAISubState_Param
{
	UBOOL m_bCheckCooldownOnPrecondition;

	FDisAISubStateDoWeaponManoeuver_Param();

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateDoWeaponManoeuver_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x7744e0, OnPending 0x7696d0. The shared half of the melee and wolfhound combat
    params: the enemy and the stance to hold while fighting it. */
struct FDisAISubStateCombatBase_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_EnemyProxy;
	BYTE m_eOverriddenBodyStance;

	FDisAISubStateCombatBase_Param( const FDisAttentionProxy& _rEnemyProxy, BYTE _eBodyStance );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateCombatBase_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x782400. "Chase that to melee range." */
struct FDisAISubStateMeleeChase_Param : public FDisAISubStateCombatBase_Param
{
	FDisAISubStateMeleeChase_Param( const FDisAttentionProxy& _rEnemyProxy, BYTE _eOverriddenBodyStance );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateMeleeChase_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x782450. "Fight that at melee range." */
struct FDisAISubStateMeleeEngage_Param : public FDisAISubStateCombatBase_Param
{
	FDisAISubStateMeleeEngage_Param( const FDisAttentionProxy& _rEnemyProxy, BYTE _eBodyStance );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateMeleeEngage_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x782920. A wolfhound closing in. It forces the Equipped stance rather than taking one,
    because a wolfhound has no other. */
struct FDisAISubStateWHCombatShortDistance_Param : public FDisAISubStateCombatBase_Param
{
	FDisAISubStateWHCombatShortDistance_Param( const FDisAttentionProxy& _rEnemyProxy );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateWHCombatShortDistance_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x7824a0, OnPending 0x769770. A wolfhound circling at range: the eight distance and
    frequency values are zeroed by the constructor and filled in by the caller from its own tweaks, which is why the
    constructor takes only the enemy. */
struct FDisAISubState_WH_CombatLongDistance_Param : public FDisAISubStateCombatBase_Param
{
	FLOAT m_fAllowedDistanceFromEnemyMin;
	FLOAT m_fAllowedDistanceFromEnemyMax;
	FLOAT m_fPreferredDistanceFromEnemyMin;
	FLOAT m_fPreferredDistanceFromEnemyMax;
	FLOAT m_fRepositionDistanceMin;
	FLOAT m_fRepositionFrequencyMin;
	FLOAT m_fRepositionFrequencyMax;
	FLOAT m_fLOSOriginOffsetZ;

	FDisAISubState_WH_CombatLongDistance_Param( const FDisAttentionProxy& _rEnemyProxy );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubState_WH_CombatLongDistance_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781af0, OnPending 0x7687e0, SizeOf 0x781ba0 = 64. "Find somewhere you can shoot that
    from." The line-check extent and shooting height are the trace shape the candidate search uses. */
struct FDisAISubStateFindShootingPosition_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_TargetProxy;
	FVector m_LineCheckExtent;
	FLOAT m_fShootingHeight;
	UBOOL m_bForceMovement;
	UBOOL m_bManageRotation;
	UBOOL m_bIgnoreMinRangeWhenUnreachable;

	FDisAISubStateFindShootingPosition_Param( const FDisAttentionProxy& _rTargetProxy, FVector _LineCheckExtent, FLOAT _fShootingHeight, UBOOL _bForceMovement, UBOOL _bManageRotation, UBOOL _bIgnoreMinRangeWhenUnreachable );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateFindShootingPosition_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781c10, OnPending 0x777580. "Shoot at that until it is dealt with." The combat form
    takes an attention proxy and resolves it to an actor in OnPending, which is what makes the shot follow the NPC's own
    belief about where the target is rather than the truth. */
struct FDisAISubStateFirePistolCombat_Param : public FDisAISubState_Param
{
	FDisAttentionProxy m_TargetProxy;
	UBOOL m_bChainingShots;

	FDisAISubStateFirePistolCombat_Param( const FDisAttentionProxy& _rTargetProxy, UBOOL _bChainingShots );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateFirePistolCombat_Param); }
};

/** DISHONORED(port): ctor 2012 rva 0x781bb0, OnPending 0x7774b0. The same sub-state driven from Kismet instead of combat:
    a plain actor, a shot count, and the controlled-firing flag that stops the sub-state deciding for itself. */
struct FDisAISubStateFirePistolControlled_Param : public FDisAISubState_Param
{
	AActor* m_pTarget;
	UBOOL m_bShootForever;
	INT m_iMaxNumShots;

	FDisAISubStateFirePistolControlled_Param( AActor* _pTarget, UBOOL _bShootForever, INT _iMaxNumShots );

	virtual void OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject );
	virtual INT SizeOf() const { return sizeof(FDisAISubStateFirePistolControlled_Param); }
};

/*-----------------------------------------------------------------------------
	UDishonoredAIBehavior's two sub-state request templates.

	DISHONORED(port): agent DF. Declared in Inc/CppText/UDishonoredAIBehavior.h; defined here because they need
	UDisAISubStateMachine and UDisAISubState complete. Read off the 2012 instantiations at rvas 0x740000
	(<Idle, Stand>) and 0x73fcb0 (<EnemyUnreachable, DoWeaponManoeuver>).
-----------------------------------------------------------------------------*/

template< class BehaviorTweaksType, class SubStateTweaksType >
void UDishonoredAIBehavior::RequestSubStateChange( BYTE _SubStateArrayIndex, FDisNativeStateParam& _rAISubStateParam )
{
	const BehaviorTweaksType* Tweaks = Cast<BehaviorTweaksType>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const BehaviorTweaksType*)BehaviorTweaksType::StaticClass()->GetDefaultObject();
	}
	if( _SubStateArrayIndex >= Tweaks->m_SubStateTweaks.Num() )
	{
		return;
	}
	m_pBehaviorFSM->RequestSafeAIStateChange( _rAISubStateParam, _SubStateArrayIndex, Tweaks->m_SubStateTweaks( _SubStateArrayIndex ) );
}

/** The "would this even work" test: the slot's tweaks are applied to the target sub-state, it is asked, and the previous
    tweaks are put back - so a precondition can read the settings it would run with without the sub-state keeping them
    when the answer is no. */
template< class BehaviorTweaksType, class SubStateTweaksType >
UBOOL UDishonoredAIBehavior::AreSubstatePreconditionsMet( const BYTE _SubStateArrayIndex, FDisNativeStateParam& _rAISubStateParam ) const
{
	UDishonoredAIBehavior* Self = const_cast<UDishonoredAIBehavior*>( this );
	const BehaviorTweaksType* Tweaks = Cast<BehaviorTweaksType>( Self->GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = (const BehaviorTweaksType*)BehaviorTweaksType::StaticClass()->GetDefaultObject();
	}
	if( _SubStateArrayIndex >= Tweaks->m_SubStateTweaks.Num() )
	{
		return FALSE;
	}
	UDisTweaks_AISubState* SlotTweaks = Tweaks->m_SubStateTweaks( _SubStateArrayIndex );

	UDisAISubState* SubState = Self->m_pBehaviorFSM->GetSubState( _rAISubStateParam.m_pStateClass );
	if( !SubState )
	{
		return FALSE;
	}
	UDisTweaksBase* PreviousTweaks = SubState->GetTweaks_Derived();
	if( !PreviousTweaks )
	{
		PreviousTweaks = (UDisTweaksBase*)SubStateTweaksType::StaticClass()->GetDefaultObject();
	}
	if( SubState->GetTweaks_Derived() != SlotTweaks )
	{
		SubState->SetTweaks_Derived( SlotTweaks );
		SubState->ApplyTweakChanges();
	}
	const UBOOL bResult = SubState->ArePreconditionsMet( _rAISubStateParam );
	if( SubState->GetTweaks_Derived() != PreviousTweaks )
	{
		SubState->SetTweaks_Derived( PreviousTweaks );
		SubState->ApplyTweakChanges();
	}
	return bResult;
}

#endif
