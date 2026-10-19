// DishonoredGame/src/dishonoredmantleedgefindercomponent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (17):
//   0x722260  public: static void __cdecl UDishonoredMantleEdgeFinderComponent::InitializePrivateStaticClassUDishonoredMantleEdgeFinderComponent(void)
//   0x722280  public: virtual void __thiscall UDishonoredMantleEdgeFinderComponent::Detach(unsigned int)
//   0x7222a0  public: void __thiscall UDishonoredMantleEdgeFinderComponent::OnMantleCheckEnd(void)
//   0x7222f0  public: unsigned int __thiscall UDishonoredMantleEdgeFinderComponent::HasValidMantleSpot(void)const
//   0x722300  public: void __thiscall UDishonoredMantleEdgeFinderComponent::SetMantleCheckDisabled(unsigned int)
//   0x722340  public: void __thiscall UDishonoredMantleEdgeFinderComponent::SetMantleStartPos(class FVector const &, class FVector const &)
//   0x722390  public: void __thiscall UDishonoredMantleEdgeFinderComponent::DebugSingleStep(void)
//   0x7223a0  public: void __thiscall UDishonoredMantleEdgeFinderComponent::DebugSingleStepOff(void)
//   0x7223b0  public: void __thiscall UDishonoredMantleEdgeFinderComponent::DebugOverrideMantleCheckTime(float)
//   0x7223d0  public: void __thiscall UDishonoredMantleEdgeFinderComponent::DebugOverrideMantleStepSize(float)
//   0x72ae60  public: struct FDisMantleSpot __thiscall UDishonoredMantleEdgeFinderComponent::GetMantleSpot(void)const
//   0x731c60  public: static class UClass * __cdecl UDishonoredMantleEdgeFinderComponent::GetPrivateStaticClassUDishonoredMantleEdgeFinderComponent(wchar_t const *)
//   0x735490  public: static class UClass * __cdecl UDishonoredMantleEdgeFinderComponent::StaticClassNoInline(void)
//   0x7354c0  public: virtual void __thiscall UDishonoredMantleEdgeFinderComponent::Attach(void)
//   0x7354f0  public: void __thiscall UDishonoredMantleEdgeFinderComponent::ResetMantleFinder(void)
//   0x743ba0  private: float __thiscall UDishonoredMantleEdgeFinderComponent::GetCurrentMaxTickTime(void)const
//   0x744a30  public: virtual void __thiscall UDishonoredMantleEdgeFinderComponent::Tick(float)

/*-----------------------------------------------------------------------------
	agent IG (PHASE23 IG): the player's mantle edge finder. A component of the player pawn (MantleEdgeFinder, and
	MantleEdgeFinder_Blink for the blink's target) that runs its own FSM (FindClearDistanceAboveOwner -> ForwardSearches ->
	CheckDestinationValid -> CheckCanStand, Src/dismantlefinderstate_*.cpp) every tick, within m_fMaxMantleCheckTimePerTick_ms:
	one complete check per frame at most, resumed where it stopped when the budget runs out. ADishonoredPlayerPawn::PerformMantle
	reads the last result. 2012 / 2013 rvas in Inc/CppText/UDishonoredMantleEdgeFinderComponent.h.
	DISHONORED(bringup): nothing in this tree hands the finder its tweaks (m_pTweaks stays None, as the retail code paths found so
	far leave it - see agentIG.md); every reader falls back to the class default object exactly as retail's GetTweaks_Derived
	callers do, through GetMantleTweaks.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dismantlefinderparams.h"
#include "igdiag.h"

void DishonoredLinkIG_MantleEdgeFinderComponent()
{
}

// the GetTweaks_Derived-or-class-default read every retail caller inlines
UDisTweaks_PlayerPawn_Mantle* UDishonoredMantleEdgeFinderComponent::GetMantleTweaks()
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = (UDisTweaks_PlayerPawn_Mantle*)GetTweaks_Derived();
	return Tweaks ? Tweaks : UDisTweaks_PlayerPawn_Mantle::StaticClass()->GetDefaultObject<UDisTweaks_PlayerPawn_Mantle>();
}

UDisTweaksBase* UDishonoredMantleEdgeFinderComponent::GetTweaks_Derived()
{
	return m_pTweaks;
}

void UDishonoredMantleEdgeFinderComponent::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pTweaks = (UDisTweaks_PlayerPawn_Mantle*)Tweaks;
}

// 2012 0x7354c0 / 2013 0x6d5e30 (42 B)
void UDishonoredMantleEdgeFinderComponent::Attach()
{
	Super::Attach();
	FDisMantleFinderState_FindClearDistanceAboveOwner_Param Param;
	m_pMantleFinderFSM->InitFSM( this, Param, NULL );
	DisIGNoteFinderAttach( this );
}

// 2012 0x722280 / 2013 0x6c1c40 (30 B)
void UDishonoredMantleEdgeFinderComponent::Detach( UBOOL bWillReattach )
{
	m_pMantleFinderFSM->DestroyFSM();
	Super::Detach( bWillReattach );
}

// 2012 0x7222a0 (78 B; inlined in 2013): the check's wall time joins the average, the step loop may stop
void UDishonoredMantleEdgeFinderComponent::OnMantleCheckEnd()
{
	m_Debug_AverageCheckWait.AddValue( (FLOAT)( appSeconds() - m_Debug_dCheckStartTime ) );
	m_bMantleCheckSteppingCompleted = TRUE;
}

// 2012 0x722300 / 2013 0x6c1cf0 (57 B): re-enabling a disabled finder forgets the old spot
void UDishonoredMantleEdgeFinderComponent::SetMantleCheckDisabled( UBOOL bDisabled )
{
	const UBOOL bReEnabled = m_bMantleCheckDisabled && !bDisabled;
	m_bMantleCheckDisabled = bDisabled ? TRUE : FALSE;
	if( bReEnabled )
	{
		m_bHasValidMantleSpot = FALSE;
	}
}

// 2012 0x722340 / 2013 0x6c1d30 (66 B)
void UDishonoredMantleEdgeFinderComponent::SetMantleStartPos( const FVector& FootPos, const FVector& Facing )
{
	m_bUseAlternateStartPos = TRUE;
	m_AlternateStartPos_FootPos = FootPos;
	m_AlternateStartPos_Facing = Facing;
}

// 2012 0x7354f0 / 2013 0x6d5ed0 (47 B)
void UDishonoredMantleEdgeFinderComponent::ResetMantleFinder()
{
	m_bHasValidMantleSpot = FALSE;
	OnMantleCheckEnd();
	FDisMantleFinderState_FindClearDistanceAboveOwner_Param Param;
	m_pMantleFinderFSM->RequestStateChange( Param, NULL, FALSE );
}

// 2012 0x743ba0 / 2013 0x6d5e60 (98 B)
FLOAT UDishonoredMantleEdgeFinderComponent::GetCurrentMaxTickTime()
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = GetMantleTweaks();
	return m_Debug_fOverrideMantleCheckTime_ms < 0.f ? Tweaks->m_fMaxMantleCheckTimePerTick_ms : m_Debug_fOverrideMantleCheckTime_ms;
}

/**
 * 2012 0x744a30 / 2013 0x6da380 (319 / 313 B; 2013 drops the 2^24 offset from the clock). In single-step mode only a requested step
 * runs (and clears the debug rays). The machine is ticked with no time until the check of this frame completes or the per-tick
 * budget is spent; a disabled finder is put back in its first state instead.
 */
void UDishonoredMantleEdgeFinderComponent::Tick( FLOAT DeltaTime )
{
	Super::Tick( DeltaTime );
	const FLOAT MaxTimePerTick_ms = GetCurrentMaxTickTime();
	INT NumSteps = 0;
	if( m_Debug_bSingleStepMode )
	{
		const UBOOL bDoStep = m_Debug_bSingleStepMode_DoStep;
		m_Debug_bSingleStepMode_DoStep = FALSE;
		if( !bDoStep )
		{
			return;
		}
		reinterpret_cast<TArray<FMantleCollisionCheckDebugInfo>*>( m_DebugRays )->Empty();
	}
	m_bMantleCheckSteppingStarted = FALSE;
	m_bMantleCheckSteppingCompleted = FALSE;
	const DOUBLE StartTime = appSeconds();
	const DOUBLE TimeAllotment = MaxTimePerTick_ms * 0.001;
	if( !m_bMantleCheckDisabled )
	{
		do
		{
			m_pMantleFinderFSM->TickStateMachine( 0.f );
			NumSteps++;
		}
		while( !m_Debug_bSingleStepMode
			&& !( m_bMantleCheckSteppingStarted && m_bMantleCheckSteppingCompleted )
			&& TimeAllotment >= appSeconds() - StartTime );
	}
	else
	{
		FDisMantleFinderState_FindClearDistanceAboveOwner_Param Param;
		m_pMantleFinderFSM->RequestStateChange( Param, NULL, FALSE );
	}
	m_Debug_AverageNumStepsPerTick.AddValue( (FLOAT)NumSteps );
	DisIGNoteFinderTick( this, NumSteps, (FLOAT)( ( appSeconds() - StartTime ) * 1000.0 ) );
}
