// DishonoredGame/src/dismantlefinderstate_checkcanstand.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x727c50  private: virtual void __thiscall FDisMantleFinderState_CheckCanStand_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x72afc0  public: static class UClass * __cdecl UDisMantleFinderState_CheckCanStand::GetPrivateStaticClassUDisMantleFinderState_CheckCanStand(wchar_t const *)
//   0x72b050  private: virtual void __thiscall UDisMantleFinderState_CheckCanStand::TickState(float)
//   0x72dbf0  public: static void __cdecl UDisMantleFinderState_CheckCanStand::InitializePrivateStaticClassUDisMantleFinderState_CheckCanStand(void)
//   0x72ff80  public: static class UClass * __cdecl UDisMantleFinderState_CheckCanStand::StaticClassNoInline(void)
//   0x731cf0  public: __thiscall FDisMantleFinderState_CheckCanStand_Param::FDisMantleFinderState_CheckCanStand_Param(struct FDisMantleFinderOwnerInfo const &, unsigned int, class FVector const &, class FVector const &, float)

/*-----------------------------------------------------------------------------
	agent IG (PHASE23 IG): the edge finder's last state. The standing box swept forward over the edge decides whether the mantle
	ends crouched; the spot is the search start's XY at the stand height, the yaw of the forward sweep, the edge height above the feet.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dismantlefinderparams.h"

void DishonoredLinkIG_DisMantleFinderStateCheckCanStand()
{
}

// 2012 0x731cf0 / 2013 0x6d1cc0 (133 B)
FDisMantleFinderState_CheckCanStand_Param::FDisMantleFinderState_CheckCanStand_Param( const FDisMantleFinderOwnerInfo& OwnerInfo, UBOOL bImpactAnimRequired,
	const FVector& SearchStart, const FVector& ForwardTraceDelta, FLOAT CurrentHeightOffset )
	: FDisMantleFinderState_Base_Param( UDisMantleFinderState_CheckCanStand::StaticClass(), OwnerInfo )
	, m_bImpactAnimRequired( bImpactAnimRequired )
	, m_SearchStart( SearchStart )
	, m_ForwardTraceDelta( ForwardTraceDelta )
	, m_fCurrentHeightOffset( CurrentHeightOffset )
{
}

// 2012 0x727c50 / 2013 0x6c6dd0 (116 B)
void FDisMantleFinderState_CheckCanStand_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	UDisMantleFinderState_CheckCanStand* State = (UDisMantleFinderState_CheckCanStand*)PendingState;
	State->m_pOwningDMEFC = (UDishonoredMantleEdgeFinderComponent*)ManagedObject;
	State->m_OwnerInfo = m_OwnerInfo;
	State->m_bImpactAnimRequired = m_bImpactAnimRequired ? TRUE : FALSE;
	State->m_SearchStart = m_SearchStart;
	State->m_ForwardTraceDelta = m_ForwardTraceDelta;
	State->m_fCurrentHeightOffset = m_fCurrentHeightOffset;
}

// 2012 0x72b050 / 2013 0x6ccad0 (432 B, same bytes)
void UDisMantleFinderState_CheckCanStand::TickState( FLOAT DeltaSeconds )
{
	const FLOAT StandCenterZ = ( ( m_SearchStart.Z + m_fCurrentHeightOffset ) - m_OwnerInfo.m_CrouchExtent.Z ) + m_OwnerInfo.m_StandExtent.Z;
	const FVector StandForwardTraceStart( m_SearchStart.X, m_SearchStart.Y, StandCenterZ );
	const FVector StandForwardTraceStop( m_ForwardTraceDelta.X + m_SearchStart.X, m_ForwardTraceDelta.Y + m_SearchStart.Y, m_ForwardTraceDelta.Z + StandCenterZ );
	UBOOL bForceCrouch = TRUE;
	if( m_OwnerInfo.m_StandExtent.Z + StandCenterZ <= m_OwnerInfo.m_fClearDistanceAboveOwner + m_OwnerInfo.m_FootPos.Z )
	{
		FCheckResult StandingForwardsHit;
		bForceCrouch = !GWorld->SingleLineCheck( StandingForwardsHit, m_pOwningDMEFC->GetOwner(), StandForwardTraceStop, StandForwardTraceStart, 0x800020C6,
			m_OwnerInfo.m_StandExtent );
	}
	const FVector StartLocation( m_SearchStart.X, m_SearchStart.Y, m_OwnerInfo.m_FootPos.Z + m_OwnerInfo.m_StandExtent.Z );
	const INT StartYaw = appRound( appAtan2( m_ForwardTraceDelta.Y, m_ForwardTraceDelta.X ) * 10430.219f );
	SetMantleCheckSucceeded( StartLocation, StartYaw, m_fCurrentHeightOffset - m_OwnerInfo.m_CrouchExtent.Z, m_bImpactAnimRequired, bForceCrouch );
}
