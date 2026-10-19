// DishonoredGame/src/dismantlefinderstate_forwardsearches.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (6):
//   0x72b320  public: static class UClass * __cdecl UDisMantleFinderState_ForwardSearches::GetPrivateStaticClassUDisMantleFinderState_ForwardSearches(wchar_t const *)
//   0x72dc50  public: static void __cdecl UDisMantleFinderState_ForwardSearches::InitializePrivateStaticClassUDisMantleFinderState_ForwardSearches(void)
//   0x730010  public: static class UClass * __cdecl UDisMantleFinderState_ForwardSearches::StaticClassNoInline(void)
//   0x7442e0  private: virtual void __thiscall UDisMantleFinderState_ForwardSearches::TickState(float)
//   0x7446a0  private: float __thiscall UDisMantleFinderState_ForwardSearches::CalcSearchStartHeight(unsigned int &)const
//   0x744da0  private: virtual void __thiscall UDisMantleFinderState_ForwardSearches::OnEnterState(class UDishonoredNativeState *)

/*-----------------------------------------------------------------------------
	agent IG (PHASE23 IG): the edge finder's forward search. From the lowest edge height the pawn's fall speed allows up to the
	clear height above it, one step (m_LineCheckStepSize) per tick, the crouched box is swept forward along the facing. A blocking
	face steep enough and facing the pawn turns the search towards it (start at the hit, direction against its normal, distance the
	forward move); the first height above a found face where the forward sweep is free is the edge: CheckDestinationValid.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dismantlefinderparams.h"

void DishonoredLinkIG_DisMantleFinderStateForwardSearches()
{
}

/**
 * 2012 0x7446a0 / 2013 0x6d69b0 (227 B, same bytes). Falling faster than m_fMaxFallSpeedForMantle: no search (FLT_MAX). Falling
 * faster than m_fFallSpeedForLedgeGrab: a ledge grab, from m_fLedgeGrabMantleMinEdgeHeight, with the impact animation. Otherwise
 * from m_MantleMinEdgeHeight. One step below either (the current step size: the value of the last search, as retail reads it
 * before OnEnterState sets it), never below 0.
 */
FLOAT UDisMantleFinderState_ForwardSearches::CalcSearchStartHeight( UBOOL& bOutImpactAnimRequired ) const
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = m_pOwningDMEFC->GetMantleTweaks();
	const FLOAT VelocityZ = m_OwnerInfo.m_Velocity.Z;
	if( -Tweaks->m_fMaxFallSpeedForMantle > VelocityZ )
	{
		return 3.4028235e38f;
	}
	if( -Tweaks->m_fFallSpeedForLedgeGrab <= VelocityZ )
	{
		return Max( Tweaks->m_MantleMinEdgeHeight - m_fSearchStepSize, 0.f );
	}
	bOutImpactAnimRequired = TRUE;
	return Max( Tweaks->m_fLedgeGrabMantleMinEdgeHeight - m_fSearchStepSize, 0.f );
}

// 2012 0x744da0 / 2013 0x6da6f0 (530 / 518 B; 2013 calls the shared failure tail)
void UDisMantleFinderState_ForwardSearches::OnEnterState( UDishonoredNativeState* LastState )
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = m_pOwningDMEFC->GetMantleTweaks();
	const FLOAT CrouchZ = m_OwnerInfo.m_CrouchExtent.Z;
	m_bFoundBlockerInFront = FALSE;
	m_bFirstBlockerInFront = TRUE;
	UBOOL bImpactAnimRequired = FALSE;
	const FLOAT SearchStartHeight = CalcSearchStartHeight( bImpactAnimRequired ) + CrouchZ;
	m_bImpactAnimRequired = bImpactAnimRequired;
	if( SearchStartHeight == 3.4028235e38f )
	{
		MantleCheckFailed( TEXT("falling faster than m_fMaxFallSpeedForMantle") );
		return;
	}
	m_OwnerCrouchPos = FVector( m_OwnerInfo.m_FootPos.X + CrouchZ * 0.f, m_OwnerInfo.m_FootPos.Y + CrouchZ * 0.f, m_OwnerInfo.m_FootPos.Z + CrouchZ );
	FLOAT SearchEndHeight = m_OwnerInfo.m_fClearDistanceAboveOwner - CrouchZ;
	if( Tweaks->m_MantleMaxEdgeHeight + CrouchZ < SearchEndHeight )
	{
		SearchEndHeight = Tweaks->m_MantleMaxEdgeHeight + CrouchZ;
	}
	m_SearchStart = m_OwnerInfo.m_FootPos;
	m_SearchDir = m_OwnerInfo.m_Facing;
	m_fSearchEndHeight = SearchEndHeight;
	m_fSearchDistance = SearchEndHeight - SearchStartHeight;
	m_fSearchDist = Tweaks->m_MantleEdgeSearchDist;
	m_fSearchStepSize = m_pOwningDMEFC->m_Debug_fOverrideMantleStepSize < 0.f ? Tweaks->m_LineCheckStepSize : m_pOwningDMEFC->m_Debug_fOverrideMantleStepSize;
	m_fCurrentHeightOffset = ( ( m_fSearchDistance - appFloor( m_fSearchDistance / m_fSearchStepSize ) * m_fSearchStepSize ) - 0.1f ) + SearchStartHeight;
}

// 2012 0x7442e0 / 2013 0x6d6600 (945 / 933 B; 2013 calls the shared failure tail)
void UDisMantleFinderState_ForwardSearches::TickState( FLOAT DeltaSeconds )
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = m_pOwningDMEFC->GetMantleTweaks();
	const FVector ForwardTraceDelta = m_SearchDir * m_fSearchDist;
	const FLOAT HeightOffset = m_fCurrentHeightOffset;
	const FVector CrouchForwardTraceStart( m_SearchStart.X + HeightOffset * 0.f, m_SearchStart.Y + HeightOffset * 0.f, m_SearchStart.Z + HeightOffset );
	const FVector CrouchForwardTraceStop = CrouchForwardTraceStart + ForwardTraceDelta;
	FCheckResult CrouchingForwardsHit;
	if( GWorld->SingleLineCheck( CrouchingForwardsHit, m_pOwningDMEFC->GetOwner(), CrouchForwardTraceStop, CrouchForwardTraceStart, 0x800020C6, m_OwnerInfo.m_CrouchExtent ) )
	{
		if( m_bFoundBlockerInFront )
		{
			FDisMantleFinderState_CheckDestinationValid_Param Param( m_OwnerInfo, m_SearchStart, m_bImpactAnimRequired, CrouchForwardTraceStop,
				CrouchForwardTraceStart, m_fCurrentHeightOffset, ForwardTraceDelta );
			DemandStateChange( Param );
			return;
		}
	}
	else
	{
		AActor* HitActor = CrouchingForwardsHit.Actor;
		IDisMantleableInterface* Mantleable = HitActor ? (IDisMantleableInterface*)HitActor->GetInterfaceAddress( UDisMantleableInterface::StaticClass() ) : NULL;
		if( Mantleable && Mantleable->ShouldRejectPlayerMantle() )
		{
			MantleCheckFailed( *FString::Printf( TEXT("%s rejects the player's mantle"), *HitActor->GetName() ) );
			return;
		}
		const FLOAT NormalZ = Clamp( ( CrouchingForwardsHit.Normal.Y + CrouchingForwardsHit.Normal.X ) * 0.f + CrouchingForwardsHit.Normal.Z, -1.f, 1.f );
		if( Tweaks->m_MaxVerticalAngleForEdgeFace * 0.017453292f >= 1.57079632679 - appAcos( NormalZ ) )
		{
			const FLOAT FacingDot = Clamp( -m_OwnerInfo.m_Facing.X * CrouchingForwardsHit.Normal.X + -m_OwnerInfo.m_Facing.Y * CrouchingForwardsHit.Normal.Y, -1.f, 1.f );
			if( Tweaks->m_MaxHorizontalAngleForEdgeFace * 0.017453292f >= appAcos( FacingDot ) )
			{
				m_bFoundBlockerInFront = TRUE;
				m_SearchDir = FVector( -CrouchingForwardsHit.Normal.X, -CrouchingForwardsHit.Normal.Y, 0.f );
				m_SearchStart = FVector( CrouchingForwardsHit.Location.X, CrouchingForwardsHit.Location.Y, m_SearchStart.Z );
				if( m_bFirstBlockerInFront )
				{
					m_fSearchDist = Tweaks->m_fMantleForwardMoveAmount;
					m_bFirstBlockerInFront = FALSE;
				}
				else
				{
					m_fSearchDist = ( 1.f - CrouchingForwardsHit.Time ) * m_fSearchDist;
				}
			}
		}
	}
	m_fCurrentHeightOffset = m_fSearchStepSize + m_fCurrentHeightOffset;
	if( m_fCurrentHeightOffset > m_fSearchEndHeight )
	{
		MantleCheckFailed( m_bFoundBlockerInFront ? TEXT("no free height above the face up to the search end") : TEXT("no face to mantle in reach") );
	}
}
