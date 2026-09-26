/*=============================================================================
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineDecalClasses.h"

IMPLEMENT_CLASS(ADecalManager);

/** @return whether dynamic decals are enabled */
UBOOL ADecalManager::AreDynamicDecalsEnabled()
{
	return GSystemSettings.bAllowDynamicDecals;
}

void ADecalManager::TickSpecial(FLOAT DeltaTime)
{
	Super::TickSpecial(DeltaTime);

	for (INT i = 0; i < ActiveDecals.Num(); i++)
	{
		FActiveDecalInfo& DecalInfo = ActiveDecals(i); //@warning: will be invalidated by the various Remove() calls below
		if (DecalInfo.Decal == NULL || DecalInfo.Decal->HasAnyFlags(RF_PendingKill))
		{
			ActiveDecals.Remove(i--);
		}
		else if (DecalInfo.Decal->DecalReceivers.Num() == 0)
		{
			// not projecting on anything, so no point in keeping it around
			eventDecalFinished(DecalInfo.Decal);
			ActiveDecals.Remove(i--);
		}
		else
		{
			// update lifetime and remove if it ran out
			DecalInfo.LifetimeRemaining -= DeltaTime;
			if (DecalInfo.LifetimeRemaining <= 0.f)
			{
				eventDecalFinished(DecalInfo.Decal);
				ActiveDecals.Remove(i--);
			}
		}
	}
}


/*-----------------------------------------------------------------------------
	DISHONORED(port): 2013 DecalManager natives without a reference body (agent AE, PHASE6 AE.2).
	DecalManager.uc runs these in script in the reference; retail 2013 made them native.
-----------------------------------------------------------------------------*/

// DISHONORED(port): 2013 ADecalManager::CanSpawnDecals (exec unnamed, vtable +932 = 0xd9f20 AreDynamicDecalsEnabled)
UBOOL ADecalManager::CanSpawnDecals()
{
	return AreDynamicDecalsEnabled();
}

// DISHONORED(port): 2013 ADecalManager::SetDecalParameters (exec 0x1d40f0, body 0xde1f0): the material goes
// through the decal's pooled MaterialInstanceConstant (SetParent), retail orders the fields as below
void ADecalManager::SetDecalParameters( UDecalComponent* TheDecal, UMaterialInterface* DecalMaterial, FVector DecalLocation, FRotator DecalOrientation, FLOAT Width, FLOAT Height, FLOAT Thickness, UBOOL bNoClip, FLOAT DecalRotation, UPrimitiveComponent* HitComponent, UBOOL bProjectOnTerrain, UBOOL bProjectOnSkeletalMeshes, FName HitBone, INT HitNodeIndex, INT HitLevelIndex, INT InFracturedStaticMeshComponentIndex, FLOAT DepthBias, FVector2D BlendRange )
{
	if( !TheDecal )
	{
		return;
	}
	UMaterialInstanceConstant* DecalMIC = Cast<UMaterialInstanceConstant>( TheDecal->GetDecalMaterial() );
	if( DecalMIC )
	{
		DecalMIC->SetParent( DecalMaterial );
	}
	else
	{
		TheDecal->SetDecalMaterial( DecalMaterial );
	}
	TheDecal->Location = DecalLocation;
	TheDecal->DecalRotation = DecalRotation;
	TheDecal->Orientation = DecalOrientation;
	TheDecal->Width = Width;
	TheDecal->Height = Height;
	TheDecal->FarPlane = Thickness * 0.5f;
	TheDecal->NearPlane = -TheDecal->FarPlane;
	TheDecal->bNoClip = bNoClip;
	TheDecal->bProjectOnTerrain = bProjectOnTerrain;
	TheDecal->bProjectOnSkeletalMeshes = bProjectOnSkeletalMeshes;
	TheDecal->HitComponent = HitComponent;
	TheDecal->HitBone = HitBone;
	TheDecal->HitNodeIndex = HitNodeIndex;
	TheDecal->HitLevelIndex = HitLevelIndex;
	TheDecal->FracturedStaticMeshComponentIndex = InFracturedStaticMeshComponentIndex;
	TheDecal->DepthBias = DepthBias;
	TheDecal->BlendRange = BlendRange;
}

// DISHONORED(port): 2013 ADecalManager::GetPooledComponent (exec 0x1d4410, body 0x104d00): pooled decals carry
// a pooled MaterialInstanceConstant (PoolMICs, retail SDK @600), a new decal gets a fresh MIC of its own
UDecalComponent* ADecalManager::GetPooledComponent()
{
	while( PoolDecals.Num() > 0 )
	{
		const INT PoolIdx = PoolDecals.Num() - 1;
		UDecalComponent* PooledDecal = PoolDecals(PoolIdx);
		UMaterialInstanceConstant* PooledMIC = PoolIdx < PoolMICs.Num() ? PoolMICs(PoolIdx) : NULL;
		PoolDecals.Remove( PoolIdx );
		if( PoolIdx < PoolMICs.Num() )
		{
			PoolMICs.Remove( PoolIdx );
		}
		if( PooledDecal && !PooledDecal->IsPendingKill() && !PooledDecal->IsWaitingForResetToDefaultsToComplete() )
		{
			PooledDecal->SetDecalMaterial( PooledMIC );
			return PooledDecal;
		}
	}
	if( MaxActiveDecals > 0 && ActiveDecals.Num() >= MaxActiveDecals )
	{
		UDecalComponent* OldestDecal = ActiveDecals(0).Decal;
		ActiveDecals.Remove( 0 );
		if( OldestDecal )
		{
			OldestDecal->ResetToDefaults();
			return OldestDecal;
		}
	}
	UDecalComponent* NewDecal = ConstructObject<UDecalComponent>( DecalTemplate ? DecalTemplate->GetClass() : UDecalComponent::StaticClass(), this, NAME_None, RF_Transient, DecalTemplate );
	NewDecal->SetDecalMaterial( ConstructObject<UMaterialInstanceConstant>( UMaterialInstanceConstant::StaticClass(), this, NAME_None, RF_Transient ) );
	return NewDecal;
}

// DISHONORED(port): 2013 ADecalManager::OnDecalFinished (exec 2012 0x1d94f0, body 0x104c20): the decal's MIC
// loses its parent and both go back to the pools
void ADecalManager::OnDecalFinished( UDecalComponent* Decal )
{
	if( !Decal )
	{
		return;
	}
	UMaterialInstanceConstant* DecalMIC = Cast<UMaterialInstanceConstant>( Decal->GetDecalMaterial() );
	if( DecalMIC )
	{
		DecalMIC->SetParent( NULL );
		PoolMICs.AddItem( DecalMIC );
		Decal->ResetToDefaults();
		PoolDecals.AddItem( Decal );
	}
}

// DISHONORED(port): 2013 ADecalManager::SpawnDecal (exec 0x1d4450, body 0xe42e0): retail hands the fractured
// mesh component index and depth bias through unchanged and attaches the decal to the manager
UDecalComponent* ADecalManager::SpawnDecal( UMaterialInterface* DecalMaterial, FVector DecalLocation, FRotator DecalOrientation, FLOAT Width, FLOAT Height, FLOAT Thickness, UBOOL bNoClip, FLOAT DecalRotation, FLOAT InDecalLifeSpan, FLOAT InDepthBias, FVector2D InBlendRange, UBOOL bProjectOnTerrain, UBOOL bProjectOnSkeletalMeshes, UPrimitiveComponent* HitComponent, FName HitBone, INT HitNodeIndex, INT HitLevelIndex, INT InFracturedStaticMeshComponentIndex )
{
	if( !CanSpawnDecals() )
	{
		return NULL;
	}
	UDecalComponent* NewDecal = GetPooledComponent();
	if( !NewDecal )
	{
		return NULL;
	}
	SetDecalParameters( NewDecal, DecalMaterial, DecalLocation, DecalOrientation, Width, Height, Thickness, bNoClip, DecalRotation, HitComponent, bProjectOnTerrain, bProjectOnSkeletalMeshes, HitBone, HitNodeIndex, HitLevelIndex, InFracturedStaticMeshComponentIndex, InDepthBias, InBlendRange );
	AttachComponent( NewDecal );
	FActiveDecalInfo* DecalInfo = new(ActiveDecals) FActiveDecalInfo;
	DecalInfo->Decal = NewDecal;
	DecalInfo->LifetimeRemaining = InDecalLifeSpan;
	return NewDecal;
}

void ADecalManager::execCanSpawnDecals( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UBOOL*)Result = CanSpawnDecals();
}

void ADecalManager::execSetDecalParameters( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDecalComponent,TheDecal);
	P_GET_OBJECT(UMaterialInterface,DecalMaterial);
	P_GET_VECTOR(DecalLocation);
	P_GET_ROTATOR(DecalOrientation);
	P_GET_FLOAT(Width);
	P_GET_FLOAT(Height);
	P_GET_FLOAT(Thickness);
	P_GET_UBOOL(bNoClip);
	P_GET_FLOAT(DecalRotation);
	P_GET_OBJECT(UPrimitiveComponent,HitComponent);
	P_GET_UBOOL(bProjectOnTerrain);
	P_GET_UBOOL(bProjectOnSkeletalMeshes);
	P_GET_NAME(HitBone);
	P_GET_INT(HitNodeIndex);
	P_GET_INT(HitLevelIndex);
	P_GET_INT(InFracturedStaticMeshComponentIndex);
	P_GET_FLOAT(DepthBias);
	P_GET_STRUCT(FVector2D,BlendRange);
	P_FINISH;
	SetDecalParameters( TheDecal, DecalMaterial, DecalLocation, DecalOrientation, Width, Height, Thickness, bNoClip, DecalRotation, HitComponent, bProjectOnTerrain, bProjectOnSkeletalMeshes, HitBone, HitNodeIndex, HitLevelIndex, InFracturedStaticMeshComponentIndex, DepthBias, BlendRange );
}

void ADecalManager::execGetPooledComponent( FFrame& Stack, RESULT_DECL )
{
	P_FINISH;
	*(UDecalComponent**)Result = GetPooledComponent();
}

void ADecalManager::execOnDecalFinished( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDecalComponent,Decal);
	P_FINISH;
	OnDecalFinished( Decal );
}

void ADecalManager::execSpawnDecal( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UMaterialInterface,DecalMaterial);
	P_GET_VECTOR(DecalLocation);
	P_GET_ROTATOR(DecalOrientation);
	P_GET_FLOAT(Width);
	P_GET_FLOAT(Height);
	P_GET_FLOAT(Thickness);
	P_GET_UBOOL(bNoClip);
	P_GET_FLOAT(DecalRotation);
	P_GET_FLOAT(InDecalLifeSpan);
	P_GET_FLOAT(InDepthBias);
	P_GET_STRUCT(FVector2D,InBlendRange);
	P_GET_UBOOL_OPTX(bProjectOnTerrain,TRUE);
	P_GET_UBOOL_OPTX(bProjectOnSkeletalMeshes,FALSE);
	P_GET_OBJECT_OPTX(UPrimitiveComponent,HitComponent,NULL);
	P_GET_NAME_OPTX(HitBone,NAME_None);
	P_GET_INT_OPTX(HitNodeIndex,INDEX_NONE);
	P_GET_INT_OPTX(HitLevelIndex,INDEX_NONE);
	P_GET_INT_OPTX(InFracturedStaticMeshComponentIndex,INDEX_NONE);
	P_FINISH;
	*(UDecalComponent**)Result = SpawnDecal( DecalMaterial, DecalLocation, DecalOrientation, Width, Height, Thickness, bNoClip, DecalRotation, InDecalLifeSpan, InDepthBias, InBlendRange, bProjectOnTerrain, bProjectOnSkeletalMeshes, HitComponent, HitBone, HitNodeIndex, HitLevelIndex, InFracturedStaticMeshComponentIndex );
}
