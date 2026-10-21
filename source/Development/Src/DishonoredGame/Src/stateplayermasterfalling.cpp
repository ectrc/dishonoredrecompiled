// DishonoredGame/src/stateplayermasterfalling.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (8):
//   0x692ec0  public: unsigned int __thiscall UStatePlayerMasterFalling::ShouldAnimateFalling(void)const
//   0x696260  public: virtual void __thiscall UStatePlayerMasterFalling::OnExitState(class UDishonoredNativeState *)
//   0x696340  public: virtual void __thiscall UStatePlayerMasterFalling::LoadPartialState(class UDishonoredNativeStateMachine *, class UObject *, class FArchive &, enum ESaveLoadLocation)
//   0x69b5c0  public: static class UClass * __cdecl UStatePlayerMasterFalling::GetPrivateStaticClassUStatePlayerMasterFalling(wchar_t const *)
//   0x69b650  public: virtual void __thiscall UStatePlayerMasterFalling::TickState(float)
//   0x6a0530  public: static void __cdecl UStatePlayerMasterFalling::InitializePrivateStaticClassUStatePlayerMasterFalling(void)
//   0x6a1ec0  public: static class UClass * __cdecl UStatePlayerMasterFalling::StaticClassNoInline(void)
//   0x6a76d0  public: virtual void __thiscall UStatePlayerMasterFalling::OnEnterState(class UDishonoredNativeState *)

/*-----------------------------------------------------------------------------
	agent IF (PHASE23 IF): UStatePlayerMasterFalling. ADishonoredPlayerPawn::setPhysics requests it for PHYS_Falling (a step
	off a ledge) and the jump demands it at the top of its rise; landing (setPhysics PHYS_Walking) requests the walk state. The 2013
	bodies are ported (rvas in Inc/CppText/UStatePlayerMasterFalling.h).
	DISHONORED(bringup), named and not called inside: the projectiles' cancel, the longest-fall
	statistic (ADishonoredPlayerPawn::SetStatValueIfGreater, 2013 0x6b9980, ePlayerStat_LongestDistanceFallen), the 2013 game events
	40 / 41 (start and end of a fall, through FArkGameEventDispatcher, a skeleton). UpdateWalkNavAnimState is agent IA's port (the crouch FOV target follows a sneak).
	agent JG (PHASE24 JG): the camera bump smoother faded out over 0.1 s for the fall and back to its default weight when it ends
	(2013 0x6759f0 / 0x663110).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "stateplayermasterparams.h"
#include "ifdiag.h"
#include "jgdiag.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkIF_UStatePlayerMasterFalling()
{
}

FStatePlayerMasterFalling_Param::FStatePlayerMasterFalling_Param()
	: FStatePlayerMasterBase_Param( UStatePlayerMasterFalling::StaticClass() )
{
}

/**
 * 2013 0x6759f0 (398 B). The action blend, the projectiles, every master input but the slide (0x200) enabled, the fall's start
 * kept, whether it came from a jump and whether the pawn was sneaking, and the zoom input (0x8000) dropped for a jump or a long
 * fall.
 */
void UStatePlayerMasterFalling::OnEnterState( UDishonoredNativeState* LastState )
{
	UStatePlayerMasterAction* Action = LastState ? Cast<UStatePlayerMasterAction>( LastState ) : NULL;
	if( Action )
	{
		m_bBlendingFromLastState = TRUE;
		m_fBlendTimeFromLastState = Action->m_fTimeToGo;
	}
	if( !AreIncomingAttacksAllowed() )
	{
		IFDIAG_ONCE( TEXT("UStatePlayerMasterFalling::OnEnterState: UDisGlobalProjectileManager::CancelActiveProjectiles is not ported") );
	}
	ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera();
	if( Camera )
	{
		Camera->GetCamera_BumpSmoother()->SetWeight( 0.f, 0.1f, FALSE );
		DisJGNoteSmootherWeight( m_pPlayerPawn, TEXT("falling enter"), 0.f, 0.1f );
	}
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, TRUE, DisInputMaskLevel_MasterAction );
	m_pPlayerPawn->EnableControllerInput( 0x200, FALSE, DisInputMaskLevel_MasterAction );
	m_fFallTime = 0.f;
	m_vStartingFallLocation = m_pPlayerPawn->Location;
	m_bCameFromJump = FALSE;
	m_bIsStealthFall = FALSE;
	if( LastState )
	{
		m_bCameFromJump = LastState->IsA( UStatePlayerMasterJump::StaticClass() );
		m_bIsStealthFall = m_pPlayerPawn->IsSneaking();
	}
	UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
	if( ShouldAnimateFalling() )
	{
		m_pPlayerPawn->EnableControllerInput( 0x8000, FALSE, DisInputMaskLevel_MasterAction );
	}
	IFDIAG_ONCE( TEXT("UStatePlayerMasterFalling: the 2013 fall events (FArkGameEventDispatcher 40 / 41) are not ported") );
}

// 2013 0x663110 (324 B): the blend flag, the inputs back, the player camera influence restored, the climbable let go, the fall's length
void UStatePlayerMasterFalling::OnExitState( UDishonoredNativeState* NextState )
{
	m_bBlendingFromLastState = FALSE;
	ADishonoredPlayerCamera* Camera = m_pPlayerPawn->GetDishonoredCamera();
	if( Camera )
	{
		UDishonoredCamera_BumpSmoother* Smoother = Camera->GetCamera_BumpSmoother();
		Smoother->SetWeight( Smoother->m_fDefaultWeight, 0.1f, FALSE );
		DisJGNoteSmootherWeight( m_pPlayerPawn, TEXT("falling exit"), Smoother->m_fDefaultWeight, 0.1f );
	}
	m_pPlayerPawn->EnableControllerInput( 0x7FFFF, TRUE, DisInputMaskLevel_MasterAction );
	m_pPlayerPawn->EnableCoreCamInfluence( UDishonoredCamera_PlayerControl::StaticClass(), -1.f );
	m_pPlayerPawn->m_bLettingGoOfClimbable = FALSE;
	IFDIAG( TEXT("falling exit after %.2f s, fell %.1f"), m_fFallTime, ( m_pPlayerPawn->Location - m_vStartingFallLocation ).Size() );
	IFDIAG_ONCE( TEXT("UStatePlayerMasterFalling::OnExitState: ADishonoredPlayerPawn::SetStatValueIfGreater (the longest fall) is not ported") );
	m_vStartingFallLocation = FVector( 0.f, 0.f, 0.f );
}

// 2013 0x675b80 (66 B)
void UStatePlayerMasterFalling::TickState( FLOAT DeltaSeconds )
{
	UpdateWalkNavAnimState( m_pPlayerPawn, TRUE );
	m_fFallTime += DeltaSeconds;
	if( ShouldAnimateFalling() )
	{
		m_pPlayerPawn->EnableControllerInput( 0x8000, FALSE, DisInputMaskLevel_MasterAction );
	}
}

// 2013 0x663260 (74 B): a pawn loaded in the middle of a fall falls
void UStatePlayerMasterFalling::LoadPartialState( UDishonoredNativeStateMachine* StateMachine, UObject* ManagedObject, FArchive& Ar, ESaveLoadLocation Location )
{
	((AActor*)ManagedObject)->setPhysics( PHYS_Falling, NULL, FVector( 0.f, 0.f, 1.f ) );
}
