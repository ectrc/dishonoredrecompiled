// DishonoredGame/src/dismantlefinderstate_findcleardistanceaboveowner.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x727d80  private: virtual void __thiscall UDisMantleFinderState_FindClearDistanceAboveOwner::OnEnterState(class UDishonoredNativeState *)
//   0x72b290  public: static class UClass * __cdecl UDisMantleFinderState_FindClearDistanceAboveOwner::GetPrivateStaticClassUDisMantleFinderState_FindClearDistanceAboveOwner(wchar_t const *)
//   0x72dc30  public: static void __cdecl UDisMantleFinderState_FindClearDistanceAboveOwner::InitializePrivateStaticClassUDisMantleFinderState_FindClearDistanceAboveOwner(void)
//   0x72ffe0  public: static class UClass * __cdecl UDisMantleFinderState_FindClearDistanceAboveOwner::StaticClassNoInline(void)
//   0x731e30  public: __thiscall FDisMantleFinderState_FindClearDistanceAboveOwner_Param::FDisMantleFinderState_FindClearDistanceAboveOwner_Param(void)
//   0x7440e0  private: virtual void __thiscall UDisMantleFinderState_FindClearDistanceAboveOwner::TickState(float)

/*-----------------------------------------------------------------------------
	agent IG (PHASE23 IG): the edge finder's first state. Entering snapshots the pawn (InitMantlerInfo); the one tick sweeps the
	crouched box straight up from the feet for the highest edge the tweaks allow (plus the stand-crouch difference) and hands the
	clear height above the pawn to ForwardSearches. Also the default state of the machine (Attach, ResetMantleFinder).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dismantlefinderparams.h"

void DishonoredLinkIG_DisMantleFinderStateFindClear()
{
}

// 2012 0x731e30 / 2013 0x6d1e00 (96 B): a zeroed owner snapshot
FDisMantleFinderState_FindClearDistanceAboveOwner_Param::FDisMantleFinderState_FindClearDistanceAboveOwner_Param()
	: FDisMantleFinderState_Base_Param( UDisMantleFinderState_FindClearDistanceAboveOwner::StaticClass(), FDisMantleFinderOwnerInfo( EC_EventParm ) )
{
}

// built inline by 2012 0x7440e0 / 2013 0x6d6410 with the FindClear parameters' vtable (/OPT:ICF)
FDisMantleFinderState_ForwardSearches_Param::FDisMantleFinderState_ForwardSearches_Param( const FDisMantleFinderOwnerInfo& OwnerInfo )
	: FDisMantleFinderState_Base_Param( UDisMantleFinderState_ForwardSearches::StaticClass(), OwnerInfo )
{
}

// 2012 0x727d80 / 2013 0x6c6f40 (8 B thunk)
void UDisMantleFinderState_FindClearDistanceAboveOwner::OnEnterState( UDishonoredNativeState* LastState )
{
	InitMantlerInfo();
}

// 2012 0x7440e0 / 2013 0x6d6410 (498 / 492 B; 2013 drops the 2^24 offset from the check-start time)
void UDisMantleFinderState_FindClearDistanceAboveOwner::TickState( FLOAT DeltaSeconds )
{
	UDishonoredMantleEdgeFinderComponent* Finder = m_pOwningDMEFC;
	Finder->m_bMantleCheckSteppingStarted = TRUE;
	Finder->m_Debug_dCheckStartTime = appSeconds();
	UDisTweaks_PlayerPawn_Mantle* Tweaks = m_pOwningDMEFC->GetMantleTweaks();
	const FLOAT SearchHeight = ( Tweaks->m_MantleMaxEdgeHeight - m_OwnerInfo.m_CrouchExtent.Z * 2.f ) + m_OwnerInfo.m_StandExtent.Z * 2.f;
	const FVector UpwardTraceStart( m_OwnerInfo.m_FootPos.X, m_OwnerInfo.m_FootPos.Y, m_OwnerInfo.m_FootPos.Z + m_OwnerInfo.m_CrouchExtent.Z );
	const FVector UpwardTraceStop( UpwardTraceStart.X, UpwardTraceStart.Y, UpwardTraceStart.Z + SearchHeight );
	FCheckResult UpwardsHit;
	const FLOAT Time = GWorld->SingleLineCheck( UpwardsHit, m_pOwningDMEFC->GetOwner(), UpwardTraceStop, UpwardTraceStart, 0x800020C6, m_OwnerInfo.m_CrouchExtent )
		? 1.f : UpwardsHit.Time;
	FDisMantleFinderOwnerInfo NewInfo( m_OwnerInfo );
	NewInfo.m_fClearDistanceAboveOwner = Time * SearchHeight + m_OwnerInfo.m_CrouchExtent.Z * 2.f;
	FDisMantleFinderState_ForwardSearches_Param Param( NewInfo );
	DemandStateChange( Param );
}
