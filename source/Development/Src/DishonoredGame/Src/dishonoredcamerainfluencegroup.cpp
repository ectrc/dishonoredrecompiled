// DishonoredGame/src/dishonoredcamerainfluencegroup.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x6fbdd0  public: static void __cdecl UDishonoredCameraInfluenceGroup::InitializePrivateStaticClassUDishonoredCameraInfluenceGroup(void)
//   0x6fbdf0  protected: void __thiscall UDishonoredCameraInfluenceGroup::WeightRotator(class FRotator &, class FRotator const &, float)
//   0x709710  public: class UDishonoredCameraInfluence * __thiscall UDishonoredCameraInfluenceGroup::GetInfluence(class UClass const *)
//   0x709770  public: void __thiscall UDishonoredCameraInfluenceGroup::SetWeight(float, class UClass const *, float)
//   0x7099c0  public: void __thiscall UDishonoredCameraInfluenceGroup::OnTeleport(class USeqAct_Teleport *)
//   0x70e110  public: unsigned int __thiscall UDishonoredCameraInfluenceGroup::GatherInfluences(struct FDishonoredViewTarget &, class APawn *, float, float)
//   0x718700  public: static class UClass * __cdecl UDishonoredCameraInfluenceGroup::GetPrivateStaticClassUDishonoredCameraInfluenceGroup(wchar_t const *)
//   0x719010  public: static class UClass * __cdecl UDishonoredCameraInfluenceGroup::StaticClassNoInline(void)
//   0x71bac0  public: void __thiscall UDishonoredCameraInfluenceGroup::PostBeginPlay(class ADishonoredPlayerCamera *)

/*-----------------------------------------------------------------------------
	Agent FZ (PHASE18 FZ): a group of camera influences. The core group (INF_GROUP_NON_ADDITIVE) replaces the view: its
	influences' weights sum to one and the view is their weighted blend, and SetWeight on one of them is a hand-over - the
	named influence gets the weight and the others share the remainder at the same speed. An additive group
	(INF_GROUP_ADDITIVE) adds each influence's delta on top. Retail 2012 rvas: PostBeginPlay 0x71bac0 / 2013 0x6cfbb0 (named
	in both, same 243 B), GatherInfluences 0x70e110 / 2013 0x6cfcb0 (named in both, 1473 -> 1495 B), SetWeight 0x709770 / 2013
	0x6c9c70 (neighbours 0.670; the 2013 body is ported: the bWaitToBlendOneFrame argument), GetInfluence 0x709710 / 2013
	0x6c9c10, WeightRotator 0x6fbdf0 / 2013 0x6c00a0, OnTeleport 0x7099c0 (unnamed in 2013).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_CameraInfluenceGroup()
{
}

/**
 * The dominant-influence index from the class default, then every influence: not active, not sleeping, owned by the
 * camera, weight and target weight from its m_fDefaultWeight, and its PostBeginPlay_Derived.
 */
void UDishonoredCameraInfluenceGroup::PostBeginPlay( ADishonoredPlayerCamera* OwningCam )
{
	m_iDominantInfluence = ((UDishonoredCameraInfluenceGroup*)GetClass()->GetDefaultObject())->m_iDominantInfluence;
	for( INT Index = 0; Index < m_Influences.Num(); Index++ )
	{
		UDishonoredCameraInfluence* Influence = m_Influences(Index);
		if( !Influence )
		{
			continue;
		}
		Influence->m_bActive = FALSE;
		Influence->m_bIsSleeping = FALSE;
		Influence->m_pOwningCam = OwningCam;
		Influence->m_Weight = Influence->m_fDefaultWeight;
		Influence->m_TargetWeight = Influence->m_fDefaultWeight;
		Influence->PostBeginPlay_Derived();
	}
}

// the one influence of the given class, or NULL
UDishonoredCameraInfluence* UDishonoredCameraInfluenceGroup::GetInfluence( const UClass* InfluenceType )
{
	for( INT Index = 0; Index < m_Influences.Num(); Index++ )
	{
		UDishonoredCameraInfluence* Influence = m_Influences(Index);
		if( Influence && Influence->GetClass() == InfluenceType )
		{
			return Influence;
		}
	}
	return NULL;
}

static void DisSetInfluenceTarget( UDishonoredCameraInfluence* Influence, FLOAT Weight, FLOAT BlendTime, UBOOL bWaitToBlendOneFrame )
{
	FLOAT Speed;
	if( BlendTime == 0.f )
	{
		Speed = 1000000.f;
		Influence->m_Weight = Weight;
	}
	else if( BlendTime < 0.f )
	{
		Speed = ((UDishonoredCameraInfluence*)Influence->GetClass()->GetDefaultObject())->m_TransitionSpeed;
	}
	else
	{
		Speed = 1.f / BlendTime;
	}
	if( Abs( Influence->m_TargetWeight - Weight ) <= 0.0001f )
	{
		Speed = Max( Speed, Influence->m_TransitionSpeed );
	}
	else
	{
		Influence->m_TargetWeight = Weight;
	}
	Influence->m_TransitionSpeed = Speed;
	Influence->m_bWaitToBlendOneFrame = bWaitToBlendOneFrame ? TRUE : FALSE;
}

/**
 * The named influence is given the weight (the same speed rule as UDishonoredCameraInfluence::SetWeight, open-coded in
 * retail); in the core group it becomes the dominant influence and every other influence is given its share of the
 * remainder, (1 - Weight) / (N - 1), at the dominant's transition time.
 */
void UDishonoredCameraInfluenceGroup::SetWeight( FLOAT Weight, const UClass* InfluenceType, FLOAT BlendTime, UBOOL bWaitToBlendOneFrame )
{
	INT Found = INDEX_NONE;
	for( INT Index = 0; Index < m_Influences.Num(); Index++ )
	{
		if( m_Influences(Index) && m_Influences(Index)->GetClass() == InfluenceType )
		{
			Found = Index;
			break;
		}
	}
	if( Found == INDEX_NONE )
	{
		return;
	}
	DisSetInfluenceTarget( m_Influences(Found), Weight, BlendTime, bWaitToBlendOneFrame );
	if( m_GroupType != INF_GROUP_NON_ADDITIVE )
	{
		return;
	}
	m_iDominantInfluence = Found;
	const INT NumInfluences = m_Influences.Num();
	if( NumInfluences <= 1 )
	{
		return;
	}
	const FLOAT TransitionTime = 1.f / m_Influences(Found)->m_TransitionSpeed;
	const FLOAT RemainWeight = ( 1.f - Weight ) / (FLOAT)( NumInfluences - 1 );
	for( INT Index = 0; Index < NumInfluences; Index++ )
	{
		if( Index != m_iDominantInfluence && m_Influences(Index) )
		{
			DisSetInfluenceTarget( m_Influences(Index), RemainWeight, TransitionTime, bWaitToBlendOneFrame );
		}
	}
}

// additive: the weighted rotation added; core: the weighted shortest route from the blend so far to the new rotation
void UDishonoredCameraInfluenceGroup::WeightRotator( FRotator& InOut, const FRotator& CurRot, FLOAT Weight )
{
	if( m_GroupType == INF_GROUP_ADDITIVE )
	{
		InOut += CurRot * Weight;
		InOut.MakeShortestRoute();
	}
	else
	{
		FRotator Dif = CurRot - InOut;
		Dif.MakeShortestRoute();
		InOut += Dif * Weight;
		InOut.MakeShortestRoute();
	}
}

void UDishonoredCameraInfluenceGroup::OnTeleport( USeqAct_Teleport* Action )
{
	for( INT Index = 0; Index < m_Influences.Num(); Index++ )
	{
		if( m_Influences(Index) )
		{
			m_Influences(Index)->OnTeleport( Action );
		}
	}
}

/**
 * Every influence's weight moves toward its target at its transition speed; an influence that reaches nil weight is
 * deactivated, one that leaves it is activated. Each active, awake influence gathers into a copy of the incoming view
 * (the core group) or into a zeroed one (an additive group); the first contribution of a core group replaces the view by
 * its weight (group weight * influence weight), every later one and every additive one accumulates: FOV and location as
 * weighted sums, the rotations through WeightRotator, the arm-follow weights clamped to 0..1. The core group finally
 * records the blended view as the settings' non-additive position and rotation, which the arm placement reads.
 */
UBOOL UDishonoredCameraInfluenceGroup::GatherInfluences( FDishonoredViewTarget& OutVT, APawn* Pawn, FLOAT DeltaTime, FLOAT UncappedDeltaTime )
{
	const FDishonoredViewTarget OrigVT = OutVT;
	UBOOL bFirstInfluence = m_GroupType == INF_GROUP_NON_ADDITIVE;
	UBOOL bReturn = FALSE;
	for( INT Index = 0; Index < m_Influences.Num(); Index++ )
	{
		UDishonoredCameraInfluence* Influence = m_Influences(Index);
		if( !Influence )
		{
			continue;
		}
		const FLOAT Step = Influence->m_TransitionSpeed * DeltaTime;
		if( Influence->m_TargetWeight <= Influence->m_Weight )
		{
			Influence->m_Weight = Max( Influence->m_Weight - Step, Influence->m_TargetWeight );
		}
		else
		{
			Influence->m_Weight = Min( Influence->m_Weight + Step, Influence->m_TargetWeight );
		}
		if( Influence->m_Weight < 1e-8f )
		{
			if( Influence->m_bActive )
			{
				Influence->OnDeactivate_Derived( Pawn );
			}
			Influence->m_bActive = FALSE;
			continue;
		}
		if( !Influence->m_bActive )
		{
			Influence->m_bActive = TRUE;
			Influence->m_fAccumulatedDelta = 0.f;
			Influence->OnActivate_Derived( Pawn );
		}
		FDishonoredViewTarget CurVT;
		if( m_GroupType == INF_GROUP_ADDITIVE )
		{
			appMemzero( &CurVT, sizeof(CurVT) );
		}
		else
		{
			CurVT = OrigVT;
		}
		if( Influence->m_bIsSleeping || !Influence->GatherInfluence( CurVT, OutVT, Pawn, DeltaTime, UncappedDeltaTime ) )
		{
			continue;
		}
		bReturn = TRUE;
		const FLOAT CurWeight = m_Weight * Influence->m_Weight;
		FDishonoredVTSettings& Out = OutVT.m_DishonoredSettings;
		const FDishonoredVTSettings& Cur = CurVT.m_DishonoredSettings;
		if( bFirstInfluence )
		{
			OutVT.m_VT.POV.FOV = CurWeight * CurVT.m_VT.POV.FOV;
			OutVT.m_VT.POV.Location = CurVT.m_VT.POV.Location * CurWeight;
			if( m_GroupType == INF_GROUP_ADDITIVE )
			{
				OutVT.m_VT.POV.Rotation = CurVT.m_VT.POV.Rotation * CurWeight;
				Out.m_ArmFollowOffset_Rot_Primary = Cur.m_ArmFollowOffset_Rot_Primary * CurWeight;
				Out.m_ArmFollowOffset_Rot_Secondary = Cur.m_ArmFollowOffset_Rot_Secondary * CurWeight;
				Out.m_ControllerOffset_Rot = Cur.m_ControllerOffset_Rot * CurWeight;
			}
			else
			{
				OutVT.m_VT.POV.Rotation = CurVT.m_VT.POV.Rotation;
				Out.m_ArmFollowOffset_Rot_Primary = Cur.m_ArmFollowOffset_Rot_Primary;
				Out.m_ArmFollowOffset_Rot_Secondary = Cur.m_ArmFollowOffset_Rot_Secondary;
				Out.m_ControllerOffset_Rot = Cur.m_ControllerOffset_Rot;
			}
			Out.m_ArmFollowWeight = CurWeight * Cur.m_ArmFollowWeight;
			Out.m_ArmFollowWeight_Rot_Primary = CurWeight * Cur.m_ArmFollowWeight_Rot_Primary;
			Out.m_ArmFollowOffset_Weight_Primary = CurWeight * Cur.m_ArmFollowOffset_Weight_Primary;
			Out.m_ArmFollowWeight_Rot_Secondary = CurWeight * Cur.m_ArmFollowWeight_Rot_Secondary;
			Out.m_ArmFollowOffset_Weight_Secondary = CurWeight * Cur.m_ArmFollowOffset_Weight_Secondary;
			Out.m_MeshSpecificFOVWeight = CurWeight * Cur.m_MeshSpecificFOVWeight;
			bFirstInfluence = FALSE;
		}
		else
		{
			OutVT.m_VT.POV.FOV += CurWeight * CurVT.m_VT.POV.FOV;
			OutVT.m_VT.POV.Location += CurVT.m_VT.POV.Location * CurWeight;
			Out.m_ArmFollowWeight = Clamp( Out.m_ArmFollowWeight + CurWeight * Cur.m_ArmFollowWeight, 0.f, 1.f );
			Out.m_ArmFollowWeight_Rot_Primary = Clamp( Out.m_ArmFollowWeight_Rot_Primary + CurWeight * Cur.m_ArmFollowWeight_Rot_Primary, 0.f, 1.f );
			Out.m_ArmFollowOffset_Weight_Primary = Clamp( Out.m_ArmFollowOffset_Weight_Primary + CurWeight * Cur.m_ArmFollowOffset_Weight_Primary, 0.f, 1.f );
			Out.m_ArmFollowWeight_Rot_Secondary = Clamp( Out.m_ArmFollowWeight_Rot_Secondary + CurWeight * Cur.m_ArmFollowWeight_Rot_Secondary, 0.f, 1.f );
			Out.m_ArmFollowOffset_Weight_Secondary = Clamp( Out.m_ArmFollowOffset_Weight_Secondary + CurWeight * Cur.m_ArmFollowOffset_Weight_Secondary, 0.f, 1.f );
			Out.m_MeshSpecificFOVWeight += CurWeight * Cur.m_MeshSpecificFOVWeight;
			WeightRotator( OutVT.m_VT.POV.Rotation, CurVT.m_VT.POV.Rotation, CurWeight );
			WeightRotator( Out.m_ArmFollowOffset_Rot_Primary, Cur.m_ArmFollowOffset_Rot_Primary, CurWeight );
			WeightRotator( Out.m_ArmFollowOffset_Rot_Secondary, Cur.m_ArmFollowOffset_Rot_Secondary, CurWeight );
			WeightRotator( Out.m_ControllerOffset_Rot, Cur.m_ControllerOffset_Rot, CurWeight );
		}
	}
	if( m_GroupType == INF_GROUP_NON_ADDITIVE )
	{
		OutVT.m_DishonoredSettings.m_NonAdditive_Pos = OutVT.m_VT.POV.Location;
		OutVT.m_DishonoredSettings.m_NonAdditive_Rot = OutVT.m_VT.POV.Rotation;
	}
	return bReturn;
}
