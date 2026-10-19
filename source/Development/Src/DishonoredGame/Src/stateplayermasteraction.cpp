// DishonoredGame/src/stateplayermasteraction.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (16):
//   0x692ba0  public: virtual void __thiscall UStatePlayerMasterAction::TickState(float)
//   0x692bc0  public: unsigned int __thiscall UStatePlayerMasterAction::ShouldIgnorePitch(void)const
//   0x692bd0  private: void __thiscall UStatePlayerMasterAction::EnableCamInfluences(unsigned int)const
//   0x692ce0  public: virtual void __thiscall UStatePlayerMasterAction::OnOtherActorTerminated(class AActor const &)
//   0x692d00  public: unsigned int __thiscall UStatePlayerMasterAction::AreIncomingAttacksAllowed(void)const
//   0x695c10  public: virtual void __thiscall FStatePlayerMasterAction_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x695c40  public: virtual unsigned int __thiscall UStatePlayerMasterAction::DoAction(void)
//   0x695ec0  public: virtual void __thiscall UStatePlayerMasterAction::OnExitState(class UDishonoredNativeState *)
//   0x695f90  protected: virtual unsigned int __thiscall UStatePlayerMasterAction::AnimState_AnimNotFound_Derived(class FString const &)
//   0x695fd0  public: virtual void __thiscall UStatePlayerMasterAction::OnLinkedActionExit(void)
//   0x69a6c0  public: void __thiscall UStatePlayerMasterAction::SetLinkedNPC(class ADishonoredNPCPawn *)
//   0x69df60  public: static class UClass * __cdecl UStatePlayerMasterAction::GetPrivateStaticClassUStatePlayerMasterAction(wchar_t const *)
//   0x6a1e70  public: static void __cdecl UStatePlayerMasterAction::InitializePrivateStaticClassUStatePlayerMasterAction(void)
//   0x6a3d70  public: static class UClass * __cdecl UStatePlayerMasterAction::StaticClassNoInline(void)
//   0x6a5d60  public: __thiscall FStatePlayerMasterAction_Param::FStatePlayerMasterAction_Param(struct FPawnAction const &)
//   0x6a7160  public: virtual unsigned int __thiscall UStatePlayerMasterAction::CanTransitionExternal(class UClass const *)const

/*-----------------------------------------------------------------------------
	agent IM (PHASE23 IM): the master FSM's full-body action (Inc/CppText/UStatePlayerMasterAction.h), 2013 bodies.
	DISHONORED(bringup), named deviations:
	  * the action's saved crouch: retail keeps the pawn's bIsCrouched at DoAction in a bit of +112 the SDK layout does not declare
	    (its first declared flag, m_bIgnorePitch, is read as bit 1 by ShouldIgnorePitch) and TickState copies it into bWantsToCrouch;
	    with no member for it TickState clears bWantsToCrouch (the player's crouch is the sneak, which leaves bIsCrouched FALSE, so
	    the value is the same on every route here)
	  * OnLinkedActionExit's game event 13 (FArkGameEventDispatcher) is not sent and IsInputEnabled is asked of the controller (the
	    pawn's 2012 0x702580 is not ported); no action links an NPC on this route
	  * SetLinkedNPC (2012 0x69a6c0: the active Blink power's ForceCancel) has no caller and no power component bodies: not written
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"
#include "imdiag.h"

void DishonoredLinkIM_StatePlayerMasterAction()
{
}

// 2012 0x692ba0 (28 B)
void UStatePlayerMasterAction::TickState( FLOAT DeltaSeconds )
{
	m_pPlayerPawn->bWantsToCrouch = FALSE;
}

// 2012 0x692bd0 / 2013 0x65fc90 (269 B): the bump smoother, physical and hit reactions back to their default weight, or off over m_fTimeToGo
void UStatePlayerMasterAction::EnableCamInfluences( UBOOL bEnabled ) const
{
	ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera();
	if( Camera == NULL )
	{
		return;
	}
	const FLOAT TargetBlend = m_fTimeToGo;
	UDishonoredCamera_BumpSmoother* BumpSmoother = Camera->GetCamera_BumpSmoother();
	if( BumpSmoother )
	{
		if( bEnabled )
		{
			BumpSmoother->SetWeight( BumpSmoother->m_fDefaultWeight, BumpSmoother->m_Weight >= 0.00000001f ? -1.f : 0.f );
		}
		else
		{
			BumpSmoother->SetWeight( 0.f, TargetBlend );
		}
	}
	UDishonoredCamera_PhysicalReact* PhysicalReact = Camera->GetCamera_PhysicalReact();
	if( PhysicalReact )
	{
		PhysicalReact->SetWeight( bEnabled ? PhysicalReact->m_fDefaultWeight : 0.f, bEnabled ? -1.f : TargetBlend );
	}
	UDishonoredCamera_HitReact* HitReact = Camera->m_pHitReact_Influence;
	if( HitReact )
	{
		HitReact->SetWeight( bEnabled ? HitReact->m_fDefaultWeight : 0.f, bEnabled ? -1.f : TargetBlend );
	}
}

// 2012 0x692ce0 (22 B)
void UStatePlayerMasterAction::OnOtherActorTerminated( const AActor& Actor )
{
	if( &Actor == m_pLinkedNPC )
	{
		m_pLinkedNPC = NULL;
	}
}

/**
 * 2013 0x662ae0 (641 B). The action's input mask is taken (movement unless m_bCanMove, look unless m_bCanLook, everything of a linked
 * action); the chosen full-body anim state is set attached to this state: on success its blend time becomes the state's when none was
 * handed over, the camera's bump smoother / physical / hit reactions blend out over it and the pawn stops dead; on failure the state
 * leaves at once; an action without an anim state clears the full-body picker (0.2 s by default). The camera is anim driven with both
 * arms' follow off for an action that cannot look, player controlled with both on otherwise; 2013 stops sneaking when the action asks.
 */
UBOOL UStatePlayerMasterAction::DoAction()
{
	m_bOverrideInterrupt_ByUse = FALSE;
	m_bOverrideInterrupt_ByParry = FALSE;
	const FPawnAction* CurAction = GetCurAction();
	INT InputMask = CurAction->m_bCanMove ? 0 : 1;
	if( !CurAction->m_bCanLook )
	{
		InputMask |= 2;
	}
	if( m_pLinkedNPC )
	{
		InputMask |= 0x8000;
	}
	if( InputMask )
	{
		m_pPlayerPawn->EnableControllerInput( InputMask, FALSE, DisInputMaskLevel_MasterActionBase );
	}
	const FName AnimState = ChooseAnimState();
	UBOOL bSucceeded = TRUE;
	if( AnimState != NAME_None )
	{
		FDisPawnAnimState* State = m_pPlayerPawn->SetAnimState( AnimState, ANIMSTATE_FULL_BODY, this, m_fTimeToGo, (UObject*)m_pFiringObj );
		if( State )
		{
			if( m_fTimeToGo < 0.f )
			{
				m_fTimeToGo = State->m_fBlendTime;
			}
			ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera();
			if( Camera )
			{
				const FLOAT TargetBlend = m_fTimeToGo;
				if( Camera->GetCamera_BumpSmoother() )
				{
					Camera->GetCamera_BumpSmoother()->SetWeight( 0.f, TargetBlend );
				}
				if( Camera->GetCamera_PhysicalReact() )
				{
					Camera->GetCamera_PhysicalReact()->SetWeight( 0.f, TargetBlend );
				}
				if( Camera->m_pHitReact_Influence )
				{
					Camera->m_pHitReact_Influence->SetWeight( 0.f, TargetBlend );
				}
			}
			m_pPlayerPawn->Velocity = FVector( 0.f, 0.f, 0.f );
			m_pPlayerPawn->Acceleration = FVector( 0.f, 0.f, 0.f );
		}
		else
		{
			m_fTimeToGo = 0.f;
			bSucceeded = FALSE;
		}
	}
	else
	{
		if( m_fTimeToGo < 0.f )
		{
			m_fTimeToGo = 0.2f;
		}
		m_pPlayerPawn->ClearAnimState( ANIMSTATE_FULL_BODY, m_fTimeToGo );
	}
	const FLOAT BlendTime = m_fTimeToGo;
	if( !CurAction->m_bCanLook )
	{
		m_pPlayerPawn->EnableCoreCamInfluence( UDishonoredCamera_AnimDriven::StaticClass(), BlendTime );
		m_pPlayerPawn->DisableArmFollow( EDisEquipUsage_Primary, TRUE, BlendTime );
		m_pPlayerPawn->DisableArmFollow( EDisEquipUsage_Secondary, TRUE, BlendTime );
	}
	else
	{
		m_pPlayerPawn->EnableCoreCamInfluence( UDishonoredCamera_PlayerControl::StaticClass(), BlendTime );
		m_pPlayerPawn->DisableArmFollow( EDisEquipUsage_Primary, FALSE, BlendTime );
		m_pPlayerPawn->DisableArmFollow( EDisEquipUsage_Secondary, FALSE, BlendTime );
	}
	if( CurAction->m_bAutomaticallyStopSneaking )
	{
		m_pPlayerPawn->StopSneaking( FALSE );
	}
	if( !bSucceeded )
	{
		RequestStateExit_WithTime( 0.f );
	}
	return TRUE;
}

/**
 * 2013 0x66e0a0 (300 B). A linked action hears its end; 2013 also asks the linked NPC's master action to leave over this state's
 * blend time when this action was not already leaving. The camera goes back to player control, a hand that is not NotReady gets
 * its arm follow back, the camera influences come back, a matinee body mode drops to arms only, all input is given back.
 */
void UStatePlayerMasterAction::OnExitState( UDishonoredNativeState* NextState )
{
	ADishonoredNPCPawn* LinkedNPC = m_pLinkedNPC;
	const UBOOL bWasExiting = m_bActionIsExiting;
	if( LinkedNPC && NextState )
	{
		OnLinkedActionExit();
	}
	m_bActionIsExiting = FALSE;
	const FLOAT BlendTime = m_fTimeToGo;
	if( LinkedNPC && NextState && !bWasExiting && LinkedNPC->m_pNPCMasterFSM )
	{
		UStateSharedActionBase* NPCAction = Cast<UStateSharedActionBase>( LinkedNPC->m_pNPCMasterFSM->GetLogicalState() );
		if( NPCAction )
		{
			NPCAction->RequestStateExit_WithTime( BlendTime );
		}
	}
	m_pPlayerPawn->EnableCoreCamInfluence( UDishonoredCamera_PlayerControl::StaticClass(), BlendTime );
	if( m_pPlayerPawn->GetActualPlayerStance( EDisEquipUsage_Primary ) != eDisPlayerStance_NotReady )
	{
		m_pPlayerPawn->DisableArmFollow( EDisEquipUsage_Primary, FALSE, BlendTime );
	}
	if( m_pPlayerPawn->GetActualPlayerStance( EDisEquipUsage_Secondary ) != eDisPlayerStance_NotReady )
	{
		m_pPlayerPawn->DisableArmFollow( EDisEquipUsage_Secondary, FALSE, BlendTime );
	}
	EnableCamInfluences( TRUE );
	const BYTE BodyMode = m_pPlayerPawn->m_BodyMode;
	if( BodyMode != 2 && BodyMode != 0 )
	{
		m_pPlayerPawn->SetBodyMode( BODY_MODE_ARMS_ONLY );
	}
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, TRUE, DisInputMaskLevel_MasterActionBase );
}

// 2012 0x695f90 / 2013 0x662d70 (64 B): a missing sequence ends the action (RequestStateExit inlined); answered
UBOOL UStatePlayerMasterAction::AnimState_AnimNotFound_Derived( const FString& SeqName )
{
	IMDIAG( TEXT("anim not found '%s' in %s: the action leaves"), *SeqName, *GetClass()->GetName() );
	RequestStateExit();
	return TRUE;
}

// 2013 0x662db0 (178 B): the movement and look the link took are given back, the camera influences restored, the link dropped
void UStatePlayerMasterAction::OnLinkedActionExit()
{
	IMDIAG_ONCE( TEXT("UStatePlayerMasterAction::OnLinkedActionExit: the game event 13 (FArkGameEventDispatcher) is not sent") );
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( m_pPlayerPawn->Controller );
	if( PC && !PC->IsInputEnabled( 2 ) )
	{
		m_pPlayerPawn->EnableControllerInput( 2, TRUE, DisInputMaskLevel_MasterActionBase );
		m_pPlayerPawn->EnableCoreCamInfluence( UDishonoredCamera_PlayerControl::StaticClass(), -1.f );
	}
	if( PC && !PC->IsInputEnabled( 1 ) )
	{
		m_pPlayerPawn->EnableControllerInput( 1, TRUE, DisInputMaskLevel_MasterActionBase );
		m_pPlayerPawn->ChangeAnimStateMotionModes( RMM_Ignore, RMRM_Ignore );
	}
	EnableCamInfluences( TRUE );
	m_pLinkedNPC = NULL;
}

// 2012 0x6a7160 / 2013 0x6727a0 (123 B): no walk or fall interrupts a master action; death always does
UBOOL UStatePlayerMasterAction::CanTransitionExternal( const UClass* StateID ) const
{
	UBOOL bResult = UStateSharedActionBase_PlayerOnly::CanTransitionExternal( StateID );
	if( bResult && ( StateID == UStatePlayerMasterWalk::StaticClass() || StateID == UStatePlayerMasterFalling::StaticClass() ) )
	{
		bResult = FALSE;
	}
	if( StateID && StateID->IsChildOf( UStatePlayerMasterDead::StaticClass() ) )
	{
		return TRUE;
	}
	return bResult;
}
