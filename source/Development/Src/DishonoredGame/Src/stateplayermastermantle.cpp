// DishonoredGame/src/stateplayermastermantle.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (18):
//   0x6af320  public: static void __cdecl UStatePlayerMasterMantle::InitializePrivateStaticClassUStatePlayerMasterMantle(void)
//   0x6af340  private: void __thiscall UStatePlayerMasterMantle::OnEnterState_IntroTransition(void)
//   0x6af3b0  private: void __thiscall UStatePlayerMasterMantle::OnExitState_Crouching(void)
//   0x6af420  public: float __thiscall UStatePlayerMasterMantle::GetCurrentMantleHeight(void)const
//   0x6af430  public: unsigned int __thiscall UStatePlayerMasterMantle::GetCrouchRequired(void)const
//   0x6af440  public: void __thiscall UStatePlayerMasterMantle::SetAllowPlayerEnd(void)
//   0x6af450  public: void __thiscall UStatePlayerMasterMantle::SetAllowPlayerCancel(unsigned int)
//   0x6af470  public: void __thiscall UStatePlayerMasterMantle::StartCrouchTransition(float)
//   0x6b3980  public: virtual void __thiscall FStatePlayerMasterMantle_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x6b3a00  private: void __thiscall UStatePlayerMasterMantle::Tick_IntroTransition(float)
//   0x6b6d10  private: void __thiscall UStatePlayerMasterMantle::OnEnterState_ImpactAnim(void)
//   0x6b6d70  private: void __thiscall UStatePlayerMasterMantle::Tick_ImpactAnim(void)
//   0x6be580  public: static class UClass * __cdecl UStatePlayerMasterMantle::GetPrivateStaticClassUStatePlayerMasterMantle(wchar_t const *)
//   0x6be610  private: virtual void __thiscall UStatePlayerMasterMantle::OnEnterState(class UDishonoredNativeState *)
//   0x6be7b0  private: virtual void __thiscall UStatePlayerMasterMantle::OnExitState(class UDishonoredNativeState *)
//   0x6c1df0  public: static class UClass * __cdecl UStatePlayerMasterMantle::StaticClassNoInline(void)
//   0x6c1e20  private: virtual void __thiscall UStatePlayerMasterMantle::TickState(float)
//   0x6c3b20  public: __thiscall FStatePlayerMasterMantle_Param::FStatePlayerMasterMantle_Param(struct FPawnAction const &, float, class FVector const &, int, unsigned int, unsigned int, unsigned int)

/*-----------------------------------------------------------------------------
	agent IG (PHASE23 IG): UStatePlayerMasterMantle, the master FSM's animated mantle (edges above m_LowMantleMaxEdgeHeight).
	Entered through ADishonoredPlayerPawn::DoMantle_Anim's full-body action: the pawn's world collision and encroach check are
	saved and switched off, crouch and sneak are left, the camera stops colliding, the impact animation is chosen, the intro
	transition (or, facing the edge, a snap) puts the pawn at the spot's start; the animation carries the pawn up and its
	AnimStateExit notify ends the state, else the timeout does. Leaving restores collision at the nearest free spot and crouches
	when the spot forced it. 2012 / 2013 rvas in Inc/CppText/UStatePlayerMasterMantle.h.
	DISHONORED(bringup), named and not called inside: the master action base (UStateSharedActionBase::OnEnterState's DoAction,
	UStatePlayerMasterAction::OnExitState 2012 0x695ec0 - every UStatePlayerMasterAction body is a skeleton) and
	ADishonoredPlayerPawn::FindNodeInCurrentAnimStateByName (UDisAnimStateComponent, not ported: no impact blend node is found).
	PerformMantle does not request this state while the action base cannot play its animation (dishonoredplayerpawn_mantle.cpp).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"
#include "dishonoredutilities.h"
#include "igdiag.h"

void DishonoredLinkIG_UStatePlayerMasterMantle()
{
}

// 2012 0x695c10 (44 B)
void FStatePlayerMasterAction_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	UStatePlayerMasterAction* State = (UStatePlayerMasterAction*)PendingState;
	State->m_pPawn = (ADishonoredPawn*)ManagedObject;
	if( State->m_pCurAction != (FPointer)m_pCurAction )
	{
		State->m_NonRepeatINTRandomHelper.m_iLastReturnedValue = -1;
	}
	State->m_pCurAction = (FPointer)m_pCurAction;
	State->m_pPlayerPawn = (ADishonoredPlayerPawn*)ManagedObject;
}

// 2012 0x6c3b20 / 2013 0x673020 (115 / 137 B)
FStatePlayerMasterMantle_Param::FStatePlayerMasterMantle_Param( const FPawnAction& Action, FLOAT CurrentMantleHeight, const FVector& MantleStartLocation,
	INT MantleStartYaw, UBOOL bImpactAnimRequired, UBOOL bCrouchRequired, UBOOL bCancelAllowed )
	: FStatePlayerMasterAction_Param( Action )
	, m_fCurrentMantleHeight( CurrentMantleHeight )
	, m_MantleStartLocation( MantleStartLocation )
	, m_MantleStartYaw( MantleStartYaw )
	, m_bImpactAnimRequired( bImpactAnimRequired )
	, m_bCrouchRequired( bCrouchRequired )
	, m_bCancelAllowed( bCancelAllowed )
{
	m_pStateClass = UStatePlayerMasterMantle::StaticClass();
}

// 2012 0x6b3980 / 2013 0x663620 (123 / 149 B: 2013 inlines the action OnPending)
void FStatePlayerMasterMantle_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	FStatePlayerMasterAction_Param::OnPending( PendingState, ManagedObject );
	UStatePlayerMasterMantle* State = (UStatePlayerMasterMantle*)PendingState;
	State->m_fCurrentMantleHeight = m_fCurrentMantleHeight;
	State->m_MantleIntroTargetLocation = m_MantleStartLocation;
	State->m_MantleIntroTargetYaw = m_MantleStartYaw;
	State->m_bImpactAnimRequired = m_bImpactAnimRequired ? TRUE : FALSE;
	State->m_bCrouchRequired = m_bCrouchRequired ? TRUE : FALSE;
	State->m_bCancelable = m_bCancelAllowed ? TRUE : FALSE;
}

// 2013 0x660270: the intro transition runs over the action's blend time when the state faces the edge
void UStatePlayerMasterMantle::OnEnterState_IntroTransition()
{
	m_fIntroTransitionTime = m_fTimeToGo;
	if( m_fIntroTransitionTime > 0.f && m_bFaceEdge )
	{
		m_bIsDoingIntroTransition = TRUE;
		m_fTransitionTime = 0.f;
		m_MantleIntroStartLocation = m_pPlayerPawn->Location;
		m_MantleIntroStartYaw = m_pPlayerPawn->Rotation.Yaw;
	}
	else
	{
		m_bIsDoingIntroTransition = FALSE;
	}
}

// 2013 0x6636c0 (572 B): the pawn and the view are moved by this frame's share of the start-to-spot lerp
void UStatePlayerMasterMantle::Tick_IntroTransition( FLOAT DeltaSeconds )
{
	if( !m_bIsDoingIntroTransition )
	{
		return;
	}
	const FLOAT LastTime = m_fTransitionTime;
	m_fTransitionTime = LastTime + DeltaSeconds;
	if( m_fTransitionTime >= m_fIntroTransitionTime )
	{
		m_bIsDoingIntroTransition = FALSE;
		m_fTransitionTime = m_fIntroTransitionTime;
	}
	const FLOAT InvTime = 1.f / m_fIntroTransitionTime;
	const FLOAT LastInterval = InvTime * LastTime;
	const FLOAT Interval = m_fTransitionTime * InvTime;
	const FVector Span = m_MantleIntroTargetLocation - m_MantleIntroStartLocation;
	const FVector TranslationDelta = ( m_MantleIntroStartLocation + Span * Interval ) - ( m_MantleIntroStartLocation + Span * LastInterval );
	const INT LastYaw = DisLerpAngle( m_MantleIntroStartYaw, m_MantleIntroTargetYaw, LastInterval );
	const INT YawDelta = DisAngleDiff( DisLerpAngle( m_MantleIntroStartYaw, m_MantleIntroTargetYaw, Interval ), LastYaw );
	ADishonoredPlayerPawn* Pawn = m_pPlayerPawn;
	const FRotator ControllerRotation = Pawn->Controller->Rotation;
	Pawn->Location += TranslationDelta;
	Pawn->Rotation.Yaw += YawDelta;
	Pawn->Controller->Rotation = FRotator( ControllerRotation.Pitch, ControllerRotation.Yaw + YawDelta, ControllerRotation.Roll );
}

// 2013 0x667c50 (81 B)
void UStatePlayerMasterMantle::OnEnterState_ImpactAnim()
{
	m_pImpactBlendNode = NULL;
	IGDIAG_ONCE( TEXT("UStatePlayerMasterMantle::OnEnterState_ImpactAnim: ADishonoredPlayerPawn::FindNodeInCurrentAnimStateByName (UDisAnimStateComponent) is not ported; no impact blend node") );
	if( m_pImpactBlendNode )
	{
		m_pImpactBlendNode->SetActiveChild( m_bImpactAnimRequired ? 0 : 1, 0.f );
	}
}

// 2013 0x667cb0 (160 B): once the impact sequence has stopped, blend to the climb
void UStatePlayerMasterMantle::Tick_ImpactAnim()
{
	if( m_pImpactBlendNode && m_pImpactBlendNode->ActiveChildIndex == 0 )
	{
		UAnimNodeSequence* ImpactSeq = Cast<UAnimNodeSequence>( m_pImpactBlendNode->Children( m_pImpactBlendNode->ActiveChildIndex ).Anim );
		if( ImpactSeq && !ImpactSeq->bPlaying )
		{
			m_pImpactBlendNode->SetActiveChild( 1, 0.2f );
		}
	}
}

// 2013 0x66ec20 (466 B; 2013 adds m_bSneakRequired)
void UStatePlayerMasterMantle::OnEnterState( UDishonoredNativeState* LastState )
{
	UStateSharedActionBase* LastAction = Cast<UStateSharedActionBase>( LastState );
	m_fTimeToGo = LastAction ? LastAction->m_fTimeToGo : -1.f;
	IGDIAG_ONCE( TEXT("UStatePlayerMasterMantle::OnEnterState: UStateSharedActionBase's DoAction (vtable +372, UStatePlayerMasterAction::DoAction 2012 0x695c40) is not ported; the mantle animation does not play") );
	ADishonoredPlayerPawn* Pawn = m_pPlayerPawn;
	m_bSavedCollideWorld = Pawn->bCollideWorld;
	Pawn->bCollideWorld = FALSE;
	m_bSavedNoEncroachCheck = Pawn->bNoEncroachCheck;
	Pawn->bNoEncroachCheck = TRUE;
	m_bSneakRequired = FALSE;
	m_bAllowPlayerEnd = FALSE;
	m_bAllowPlayerCancelActive = FALSE;
	m_bPlayerCancelled = FALSE;
	m_bSneakRequired = Pawn->IsSneaking();
	Pawn->EnableControllerInput( 0x300, FALSE, DisInputMaskLevel_MasterAction );
	Pawn->UnCrouch( 0 );
	Pawn->StopSneaking( FALSE );
	ADishonoredPlayerCamera* Camera = Pawn->GetDishonoredCamera();
	if( Camera )
	{
		Camera->EnableCollision( FALSE );
	}
	OnEnterState_ImpactAnim();
	OnEnterState_IntroTransition();
	if( !m_bIsDoingIntroTransition && m_bFaceEdge )
	{
		const FRotator PawnRotation = Pawn->Rotation;
		const FRotator ControllerRotation = Pawn->Controller->Rotation;
		Pawn->SetLocation( m_MantleIntroTargetLocation );
		Pawn->SetRotation( FRotator( PawnRotation.Pitch, m_MantleIntroTargetYaw, PawnRotation.Roll ) );
		Pawn->Controller->SetRotation( FRotator( ControllerRotation.Pitch, m_MantleIntroTargetYaw, ControllerRotation.Roll ) );
	}
	Pawn->EnableControllerInput( 0x8800, FALSE, DisInputMaskLevel_MasterAction );
	m_fTimeoutTimer = m_fMaxTimeInMantleState;
	DisIGNoteMantleState( Pawn, TRUE, m_fCurrentMantleHeight );
}

// 2013 0x6602e0 (105 B)
void UStatePlayerMasterMantle::OnExitState_Crouching()
{
	ADishonoredPlayerPawn* Pawn = m_pPlayerPawn;
	Pawn->EnableControllerInput( 0x300, TRUE, DisInputMaskLevel_MasterAction );
	if( !m_bPlayerCancelled && m_bCrouchRequired )
	{
		Pawn->Crouch( 0 );
		Pawn->StartSneaking( FALSE );
	}
	ADishonoredPlayerCamera* Camera = Pawn->GetDishonoredCamera();
	UDishonoredCamera_CrouchMantleOffset* CrouchOffset = Camera ? Camera->GetCamera_CrouchMantleOffset() : NULL;
	if( CrouchOffset )
	{
		CrouchOffset->SetWeight( 0.f, 0.f );
	}
}

// 2012 0x6af470 (109 B)
void UStatePlayerMasterMantle::StartCrouchTransition( FLOAT TransitionTime )
{
	if( !m_bCrouchRequired )
	{
		return;
	}
	ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera();
	UDishonoredCamera_CrouchMantleOffset* CrouchOffset = Camera ? Camera->GetCamera_CrouchMantleOffset() : NULL;
	if( CrouchOffset )
	{
		CrouchOffset->SetOffset();
		m_pPlayerPawn->Crouch( 0 );
		CrouchOffset->SetWeight( 1.f, 0.f );
		CrouchOffset->SetWeight( 0.f, TransitionTime );
	}
}

// 2012 0x6be7b0 / 2013 0x66ee00 (283 B)
void UStatePlayerMasterMantle::OnExitState( UDishonoredNativeState* NextState )
{
	if( NextState )
	{
		OnExitState_Crouching();
	}
	ADishonoredPlayerPawn* Pawn = m_pPlayerPawn;
	ADishonoredPlayerCamera* Camera = Pawn->GetDishonoredCamera();
	if( Camera )
	{
		Camera->EnableCollision( TRUE );
	}
	FVector PlayerLoc = Pawn->Location;
	if( GWorld->FindSpot( Pawn->GetCylinderExtent(), PlayerLoc, FALSE ) )
	{
		Pawn->Location = PlayerLoc;
	}
	Pawn->bCollideWorld = m_bSavedCollideWorld;
	Pawn->bNoEncroachCheck = m_bSavedNoEncroachCheck;
	Pawn->FindTouchingActors();
	IGDIAG_ONCE( TEXT("UStatePlayerMasterMantle::OnExitState: UStatePlayerMasterAction::OnExitState (2012 0x695ec0) is not ported") );
	m_DebugCurrentAnimName = NAME_None;
	Pawn->EnableControllerInput( 0x8800, TRUE, DisInputMaskLevel_MasterAction );
	DisIGNoteMantleState( Pawn, FALSE, m_fCurrentMantleHeight );
}

// 2013 0x66c590 (345 B; 2013 inlines RequestStateExit on the timeout and ends with the base's bWantsToCrouch clear)
void UStatePlayerMasterMantle::TickState( FLOAT DeltaSeconds )
{
	Tick_ImpactAnim();
	Tick_IntroTransition( DeltaSeconds );
	ADishonoredPlayerPawn* Pawn = m_pPlayerPawn;
	if( m_bCancelable && m_bAllowPlayerCancelActive )
	{
		ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Pawn->Controller );
		if( PC && !PC->m_bJumpButton )
		{
			RequestStateExit();
			m_bPlayerCancelled = TRUE;
		}
	}
	if( m_bAllowPlayerEnd && Pawn->Controller )
	{
		APlayerController* PlayerController = Cast<APlayerController>( Pawn->Controller );
		UPlayerInput* Input = PlayerController ? PlayerController->PlayerInput : NULL;
		if( Input && ( Input->RawJoyUp != 0.f || Input->RawJoyRight != 0.f || Input->RawJoyLookRight != 0.f || Input->RawJoyLookUp != 0.f ) )
		{
			RequestStateExit();
		}
	}
	m_fTimeoutTimer -= DeltaSeconds;
	if( m_fTimeoutTimer < 0.f )
	{
		debugf( NAME_Warning, TEXT("Mantle state timed out!  Is the animation missing an AnimStateExit signal?") );
		RequestStateExit();
	}
	Pawn->bWantsToCrouch = FALSE;
}
