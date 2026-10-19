// DishonoredGame/src/dismantlefinderstate_checkdestinationvalid.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (7):
//   0x727cd0  public: virtual void __thiscall FDisMantleFinderState_CheckDestinationValid_Param::OnPending(class UDishonoredNativeState *, class UObject *)
//   0x72b200  public: static class UClass * __cdecl UDisMantleFinderState_CheckDestinationValid::GetPrivateStaticClassUDisMantleFinderState_CheckDestinationValid(wchar_t const *)
//   0x72dc10  public: static void __cdecl UDisMantleFinderState_CheckDestinationValid::InitializePrivateStaticClassUDisMantleFinderState_CheckDestinationValid(void)
//   0x72ffb0  public: static class UClass * __cdecl UDisMantleFinderState_CheckDestinationValid::StaticClassNoInline(void)
//   0x731d80  public: __thiscall FDisMantleFinderState_CheckDestinationValid_Param::FDisMantleFinderState_CheckDestinationValid_Param(struct FDisMantleFinderOwnerInfo const &, class FVector const &, unsigned int, class FVector const &, class FVector const &, float, class FVector const &)
//   0x743c10  private: unsigned int __thiscall UDisMantleFinderState_CheckDestinationValid::VerifyDestinationIsValid_Grid(class UStaticMeshComponent *)const
//   0x744b70  private: virtual void __thiscall UDisMantleFinderState_CheckDestinationValid::TickState(float)

/*-----------------------------------------------------------------------------
	agent IG (PHASE23 IG): the edge finder's floor check. Below the free forward sweep's end (one step and a unit down) the
	crouched box must find either nothing or a floor no steeper than m_MaxSlopeAngleForEdgeTop; a steeper static mesh is
	accepted when one cell of a grid over the crouched footprint, on the far side of the edge, finds such a floor.
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dismantlefinderparams.h"
#include "dishonoredutilities.h"

void DishonoredLinkIG_DisMantleFinderStateCheckDestinationValid()
{
}

// 2012 0x731d80 / 2013 0x6d1d50 (173 B)
FDisMantleFinderState_CheckDestinationValid_Param::FDisMantleFinderState_CheckDestinationValid_Param( const FDisMantleFinderOwnerInfo& OwnerInfo,
	const FVector& SearchStart, UBOOL bImpactAnimRequired, const FVector& DestLocation, const FVector& SearchPos, FLOAT CurrentSearchHeight,
	const FVector& ForwardTraceDelta )
	: FDisMantleFinderState_Base_Param( UDisMantleFinderState_CheckDestinationValid::StaticClass(), OwnerInfo )
	, m_SearchStart( SearchStart )
	, m_bImpactAnimRequired( bImpactAnimRequired )
	, m_DestLocation( DestLocation )
	, m_SearchPos( SearchPos )
	, m_fCurrentSearchHeight( CurrentSearchHeight )
	, m_ForwardTraceDelta( ForwardTraceDelta )
{
}

// 2012 0x727cd0 / 2013 0x6c6e70 (164 B)
void FDisMantleFinderState_CheckDestinationValid_Param::OnPending( UDishonoredNativeState* PendingState, UObject* ManagedObject )
{
	UDisMantleFinderState_CheckDestinationValid* State = (UDisMantleFinderState_CheckDestinationValid*)PendingState;
	State->m_pOwningDMEFC = (UDishonoredMantleEdgeFinderComponent*)ManagedObject;
	State->m_OwnerInfo = m_OwnerInfo;
	State->m_SearchStart = m_SearchStart;
	State->m_bImpactAnimRequired = m_bImpactAnimRequired ? TRUE : FALSE;
	State->m_DestLocation = m_DestLocation;
	State->m_SearchPos = m_SearchPos;
	State->m_fCurrentHeightOffset = m_fCurrentSearchHeight;
	State->m_ForwardTraceDelta = m_ForwardTraceDelta;
}

/**
 * 2012 0x743c10 / 2013 0x6d5f40 (1227 B). The crouched footprint around the destination is cut into m_FinalVerifyLocationGridSize
 * squared cells; the cells beyond the edge (on the far side of the plane through the edge point, facing back to the search) each
 * sweep a 5-unit-high box down from 5 above the box's bottom by the depth the slope allows across the footprint plus one step; the
 * first cell that finds a floor no steeper than the tweak accepts.
 */
UBOOL UDisMantleFinderState_CheckDestinationValid::VerifyDestinationIsValid_Grid( UStaticMeshComponent* StaticMeshComp ) const
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = m_pOwningDMEFC->GetMantleTweaks();
	const FLOAT MaxSlopeRad = Tweaks->m_MaxSlopeAngleForEdgeTop * 0.017453292f;
	const FLOAT Width = m_OwnerInfo.m_CrouchExtent.X * 2.f;
	const FLOAT Depth = m_OwnerInfo.m_CrouchExtent.Y * 2.f;
	const INT GridSize = Tweaks->m_FinalVerifyLocationGridSize;
	const FLOAT InvGrid = 1.f / (FLOAT)GridSize;
	const FLOAT CellSizeX = InvGrid * Width;
	const FLOAT CellSizeY = InvGrid * Depth;
	const FVector CellExtent( CellSizeX * 0.5f, CellSizeY * 0.5f, 5.f );
	const FLOAT FirstCellX = ( m_DestLocation.X - m_OwnerInfo.m_CrouchExtent.X ) + CellSizeX * 0.5f;
	const FLOAT FirstCellY = ( m_DestLocation.Y - m_OwnerInfo.m_CrouchExtent.Y ) + CellSizeY * 0.5f;
	const FLOAT CellZ = m_DestLocation.Z - m_OwnerInfo.m_CrouchExtent.Z;
	const FLOAT CheckDepth = appSqrt( Depth * Depth + Width * Width ) * appTan( MaxSlopeRad ) + Tweaks->m_LineCheckStepSize;
	const FVector OffsetToEdge = m_DestLocation - m_SearchPos;
	const FVector PointOnEdge = m_SearchPos + DisFindExtentInDirection( m_OwnerInfo.m_CrouchExtent, OffsetToEdge );
	const FVector EdgeNormal = OffsetToEdge * -appInvSqrt( OffsetToEdge.SizeSquared() );
	UBOOL bFound = FALSE;
	for( INT X = 0; X < GridSize && !bFound; X++ )
	{
		const FLOAT CellX = (FLOAT)X * CellSizeX + FirstCellX;
		for( INT Y = 0; Y < GridSize && !bFound; Y++ )
		{
			const FLOAT CellY = (FLOAT)Y * CellSizeY + FirstCellY;
			if( ( CellX - PointOnEdge.X ) * EdgeNormal.X + ( CellZ - PointOnEdge.Z ) * EdgeNormal.Z + ( CellY - PointOnEdge.Y ) * EdgeNormal.Y < 0.f )
			{
				const FVector CellTop( CellX, CellY, CellZ + 5.f );
				const FVector CellBottom( CellX, CellY, -( CheckDepth + 5.f ) + CellZ );
				FCheckResult CheckResult;
				if( !StaticMeshComp->LineCheck( CheckResult, CellBottom, CellTop, CellExtent, 0x800020C6 ) )
				{
					const FLOAT NormalZ = Clamp( ( CheckResult.Normal.Y + CheckResult.Normal.X ) * 0.f + CheckResult.Normal.Z, -1.f, 1.f );
					if( appAcos( NormalZ ) <= MaxSlopeRad )
					{
						bFound = TRUE;
					}
				}
			}
		}
	}
	return bFound;
}

// 2012 0x744b70 / 2013 0x6da4c0 (558 / 546 B; 2013 calls the shared failure tail)
void UDisMantleFinderState_CheckDestinationValid::TickState( FLOAT DeltaSeconds )
{
	UDisTweaks_PlayerPawn_Mantle* Tweaks = m_pOwningDMEFC->GetMantleTweaks();
	const FLOAT DropDistance = Tweaks->m_LineCheckStepSize + 1.f;
	const FLOAT MaxSlopeRad = Tweaks->m_MaxSlopeAngleForEdgeTop * 0.017453292f;
	const FVector CheckStart = m_DestLocation;
	const FVector CheckStop( m_DestLocation.X - DropDistance * 0.f, m_DestLocation.Y - DropDistance * 0.f, m_DestLocation.Z - DropDistance );
	FCheckResult Hit;
	UBOOL bValid = GWorld->SingleLineCheck( Hit, m_pOwningDMEFC->GetOwner(), CheckStop, CheckStart, 0x800024C6, m_OwnerInfo.m_CrouchExtent );
	if( !bValid )
	{
		const FLOAT NormalZ = Clamp( Hit.Normal.Z, -1.f, 1.f );
		bValid = appAcos( NormalZ ) <= MaxSlopeRad
			|| ( Hit.Component && Hit.Component->IsA( UStaticMeshComponent::StaticClass() ) && VerifyDestinationIsValid_Grid( (UStaticMeshComponent*)Hit.Component ) );
	}
	if( bValid )
	{
		FDisMantleFinderState_CheckCanStand_Param Param( m_OwnerInfo, m_bImpactAnimRequired, m_SearchStart, m_ForwardTraceDelta, m_fCurrentHeightOffset );
		DemandStateChange( Param );
	}
	else
	{
		MantleCheckFailed( TEXT("the edge top is steeper than m_MaxSlopeAngleForEdgeTop") );
	}
}
