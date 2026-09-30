// DishonoredGame/src/dishonoredutilities.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (54):
//   0x823330  enum ECardinalDirection __cdecl DisGetCardinalDirection(class AActor const * const, class FVector const &)
//   0x823510  class FVector __cdecl DisGetCardinalVector(class AActor const * const, enum ECardinalDirection)
//   0x823830  class FVector __cdecl DisGetPredictedDelta(class AActor const * const, float)
//   0x823880  void __cdecl DisHardAttachToActor(class AActor *, class AActor *)
//   0x823920  unsigned int __cdecl DisIsPawnRendered(class APawn const *)
//   0x823960  void __cdecl DisDestroyActorNextTick(class AActor &)
//   0x8266c0  void __cdecl DisSendObjectiveChangedNotify(enum EDisObjectiveChangedNotifyType, class UObject *)
//   0x8266f0  enum ECardinalDirection __cdecl DisGetCardinalDirectionFromImpact(class AActor const * const, class FVector const &, class FVector const &)
//   0x826920  enum eDis8WayDirection __cdecl DisGet8WayDirFromDamageType(class AActor const * const, class FVector const &, class FVector const &, class UClass const *)
//   0x826ac0  void __cdecl DisRadialImpulse(class FVector const &, float, float, class AActor *, unsigned int, unsigned int)
//   0x826c20  void __cdecl DisPullFromBendTime(class AActor *, class AActor *, unsigned int, unsigned int)
//   0x826cb0  class FVector __cdecl DisRandomizeAimTarget(class FVector const &, class FVector const &, class AActor const *, float)
//   0x826f30  unsigned int __cdecl DisIsPawnValid(class APawn const *)
//   0x826f60  class ADishonoredPawn * __cdecl DisGetValidInstigator(class AActor const &)
//   0x826fb0  class ADishonoredPawn * __cdecl DisGetPawnInstigator(class AController const *)
//   0x826fe0  class ADishonoredPawn * __cdecl DisGetDamageCulprit(class AController const *, class UClass *)
//   0x82bd40  void __cdecl DisDeactivateAllAttachedEmitters(class USkeletalMeshComponent const *)
//   0x82be00  float __cdecl DisFindNotifyTime(class UAnimNodeSequence const *, class UClass const *)
//   0x82beb0  struct FAnimNotifyEvent const * __cdecl DisFindNotify(class UAnimNodeSequence const *, class UClass const *)
//   0x82bf40  enum ECardinalDirection __cdecl DisGetCardinalDirectionConstrained(class AActor const * const, class FVector const &, class TArray<struct FDisDirectionConstraint, class FDefaultAllocator> const &)
//   0x82c170  enum eDisDeathDirection __cdecl DisGetDeathDirFromDamageType(class AActor const * const, class FVector const &, class FVector const &, class UClass *)
//   0x82c240  unsigned int __cdecl DisIsUnsuppressed(class FName &)
//   0x82c260  class FVector __cdecl DisGetPredictedLocation(class AActor const * const, float)
//   0x82c340  void __cdecl DisReturnToBendTime(class AActor *, unsigned int, unsigned int)
//   0x82c390  DisGetAimTargetHelper
//   0x82c4a0  class FVector __cdecl DisGetMedianAimTarget(class AActor const *, unsigned int)
//   0x82c4d0  class FVector __cdecl DisGetAimDirection(class FVector const &, class AActor const *, unsigned int, class FVector const &, float)
//   0x82c5e0  void __cdecl DisBuildComponentSpaceTransform(class FBoneAtom &, int, class TArray<class FBoneAtom, class FDefaultAllocator> const &, class TArray<struct FMeshBone, class FDefaultAllocator> const &)
//   0x82c850  unsigned int __cdecl DisIsActorVisibleToPlayer(class AActor const *)
//   0x82c890  void __cdecl DisGetActorsVisibleToPlayer(struct TMemStackArray<class AActor const *> &)
//   0x82c8c0  unsigned int __cdecl DisIsActorVisibleToAnyPlayer(class AActor const *)
//   0x82c9e0  unsigned int __cdecl DisAttemptAttachToSkeletal(struct FDisLineProbeResult &, class AActor *, class AActor *)
//   0x82cab0  void __cdecl DisListComponentsForActor(class AActor *)
//   0x82cc90  void __cdecl DisFireKismetEvent(class AActor * const, class UClass * const, class AActor * const, class AActor * const, unsigned int, class TArray<int, class FDefaultAllocator> *)
//   0x82cd60  class FString __cdecl DisEnumTypeToString(unsigned char, wchar_t const *, unsigned int)
//   0x82ce70  void __cdecl DisHideGoreSections(class USkeletalMeshComponent &)
//   0x82cf70  void __cdecl DisCreateMICs(class UMeshComponent *, class TArray<class UMaterialInterface *, class FDefaultAllocator> &)
//   0x82d030  void __cdecl DisCreateMICsForNPC(class ADishonoredNPCPawn *, unsigned int)
//   0x82d270  void __cdecl DisSetScalarParameterToAllMats(class FName, float, class UMeshComponent *)
//   0x82d2f0  void __cdecl DisSetScalarParameterToAllMatsOnNPC(class FName const &, float, class ADishonoredNPCPawn *, unsigned int)
//   0x82d440  void __cdecl DisSetParentToAllMats(class UMaterialInterface *, class UMeshComponent *)
//   0x82d4c0  void __cdecl DisSetParentToAllMatsOnNPC(class UMaterialInterface *, class ADishonoredNPCPawn *, unsigned int)
//   0x82d5d0  void __cdecl DisGameOver(class FString const &, float)
//   0x82d600  void __cdecl DisReplaceMatchingMaterialsInSkelMesh(class USkeletalMeshComponent *, class UMaterialInterface const *, class UMaterialInterface *)
//   0x82d680  class FConfigSection const * __cdecl FindLocalizedConfigSection(wchar_t const *, wchar_t const *, wchar_t *, int)
//   0x832010  class UDisParticleSystemComponent * __cdecl DishonoredSpawnEmitter(class UParticleSystem *, class FVector const &, class FRotator const &, class AActor *, unsigned int)
//   0x832090  class UDisParticleSystemComponent * __cdecl DishonoredSpawnAttachedEmitter(class UParticleSystem *, class USkeletalMeshComponent *, class FName, unsigned int, class FVector const &, class FRotator const &)
//   0x832100  void __cdecl DisNotifyOnDialogSelChange(class UObject *, class FDisDialogSelNotify *, unsigned int)
//   0x832160  void __cdecl DisFireSingleKismetEvent(class AActor * const, class UClass * const, class AActor * const, class AActor * const, unsigned int, int)
//   0x8321e0  void __cdecl DisReplaceMatchingMaterialsInSkelMesh(class USkeletalMeshComponent *, int, class UMaterialInterface *)
//   0x839de0  void __cdecl DisGetTaskVars(class USequenceOp const *, wchar_t const *, struct TMemStackArray<class UDishonoredTask_Base const *> &)
//   0x83efb0  void __cdecl DisMatchSeqOutputLinksToDefault(class USequenceOp *)
//   0x840cd0  unsigned int __cdecl DisIsPackageSeekFree(wchar_t const *)
//   0xbaab50  _dynamic_initializer_for__g_DialogSelNotifyActors__

// ---- agent AU ports (PHASE7 AU): the utilities the pickup path calls ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x7cf970 (2012 0x832010, same bytes): the world's emitter pool spawns the effect and
// remembers which actor's lifetime bounds it.
UDisParticleSystemComponent* DishonoredSpawnEmitter( UParticleSystem* EmitterTemplate, const FVector& SpawnLocation, const FRotator& SpawnRotation, AActor* TimeBoundActor, UBOOL bAttachToActor )
{
	AWorldInfo* WorldInfo = GWorld ? GWorld->GetWorldInfo() : NULL;
	if( !WorldInfo || !EmitterTemplate )
	{
		return NULL;
	}
	ADishonoredEmitterPool* Pool = Cast<ADishonoredEmitterPool>( WorldInfo->MyEmitterPool );
	if( !Pool )
	{
		return NULL;
	}
	// DISHONORED(bringup): ADishonoredEmitterPool::SpawnPooledEmitter is a comment-only unit, so the idle particle
	// system of a pickup is not spawned yet; nothing else on the collection path depends on it.
	return NULL;
}

// DISHONORED(written): 2013 rva 0x7bafa0 (2012 0x823960, same bytes): a negative LifeSpan is the engine's
// "destroy on the next tick" marker.
void DisDestroyActorNextTick( AActor& Actor )
{
	Actor.LifeSpan = -1.f;
}

// DISHONORED(written): 2013 rva 0x7bf060 (2012 0x826fb0, same bytes)
ADishonoredPawn* DisGetPawnInstigator( const AController* Controller )
{
	if( !Controller )
	{
		return NULL;
	}
	if( Controller == ADishonoredPlayerController::s_pInstance )
	{
		return ADishonoredPlayerPawn::s_pInstance;
	}
	return Cast<ADishonoredPawn>( Controller->Pawn );
}

// DISHONORED(written): 2013 rva 0x7c7b30 (2012 0x82cc90): every enabled sequence event the actor generated whose class
// matches (exactly, or by inheritance) is offered the activation; the originator falls back to the instigator and then
// to the actor itself.
void DisFireKismetEvent( AActor* const Actor, UClass* const EventClass, AActor* const Instigator, AActor* const Originator, UBOOL bExactClass, TArray<INT>* ActivateIndices )
{
	if( !Actor )
	{
		return;
	}
	for( INT Idx = 0; Idx < Actor->GeneratedEvents.Num(); Idx++ )
	{
		USequenceEvent* Event = Actor->GeneratedEvents(Idx);
		if( !Event || !Event->bEnabled )
		{
			continue;
		}
		const UBOOL bMatches = bExactClass ? ( Event->GetClass() == EventClass ) : Event->IsA( EventClass );
		if( !bMatches )
		{
			continue;
		}
		AActor* InOriginator = Originator ? Originator : ( Instigator ? Instigator : Actor );
		Event->CheckActivate( InOriginator, Instigator, FALSE, ActivateIndices, FALSE );
	}
}

// DISHONORED(written): 2013 rva 0x7bec00 (2012 0x826c20).
// DISHONORED(bringup): the body is four ADishonoredGameInfo virtuals (vtable +968 AddActorToBendTime,
// +972 GetBendTimeLeftFor, +1072 RemoveActorFromBendTime) that are not ported, and AActor::AdjustBendTime does not
// exist in this build's Engine; with DisIsBendTimeOn() always FALSE every caller of this is already dead code.
void DisPullFromBendTime( AActor* Actor, AActor* Cause, UBOOL bRecursive, UBOOL bOnlyIfStaticOrTickDisabled )
{
}

// DISHONORED(written): 2013 rva 0x7bf010 (2012 0x826f60, same bytes): the actor's Instigator, unless it is being
// destroyed or is pending kill.
ADishonoredPawn* DisGetValidInstigator( const AActor& Actor )
{
	APawn* Instigator = Actor.Instigator;
	if( !Instigator || Instigator->bDeleteMe || Instigator->IsPendingKill() )
	{
		return NULL;
	}
	return Cast<ADishonoredPawn>( Instigator );
}

// DISHONORED(written): 2013 rva 0x7b2420 (2012 0x814ef0), UDisGFxMoviePlayerHUD::AddUseMessage, reduced to the part
// this build can run: a non-empty message reaches the HUD's game-message slot.
// DISHONORED(bringup): UDisGFxMoviePlayerHUD::SetGameMessage is GFx and not ported (see agentAW.md's decision), so the
// message is logged under -dispickup instead of being displayed.
void DisAddUseMessage( const FString& Message )
{
	if( Message.Len() <= 0 )
	{
		return;
	}
	if( DisPickupCensusEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): dispickup use message: %s"), *Message );
	}
}

// ---- agent DI ports (PHASE10 DI): the material and gore-section helpers of the appearance path ----

// DISHONORED(port): 2013 rva 0x7c7dd0 (2012 0x82ce70). A Dishonored skeletal mesh carries one FBodyPart per material
// index (USkeletalMesh::m_MaterialsToBodyParts): a part with a cut bone and m_bShowIfCut set is the stump that only
// appears once that limb is severed, so at spawn it is hidden and every other section is shown. Material indices past
// the end of the body-part array are always shown.
// DISHONORED(bringup): retail batches the whole visibility array into one render command through
// USkeletalMeshComponent::ShowMaterialSections (2013 rva 0x34ddf0), which this Engine tree does not declare; the
// per-index USkeletalMeshComponent::ShowMaterialSection it does have has the same effect one section at a time.
// Retail passes LODModels.Num() - 1 as the LOD index, so only the coarsest LOD is touched - kept as retail has it.
void DisHideGoreSections( USkeletalMeshComponent& _rMeshComponent )
{
	USkeletalMesh* SkeletalMesh = _rMeshComponent.SkeletalMesh;
	if( !SkeletalMesh )
	{
		return;
	}
	const INT NumMaterials = SkeletalMesh->Materials.Num();
	const INT NumBodyParts = SkeletalMesh->m_MaterialsToBodyParts.Num();
	const INT NumBoth = Min( NumBodyParts, NumMaterials );
	const INT LODIndex = SkeletalMesh->LODModels.Num() - 1;
	for( INT Idx = 0; Idx < NumMaterials; Idx++ )
	{
		UBOOL bShow = TRUE;
		if( Idx < NumBoth )
		{
			const FBodyPart& BodyPart = SkeletalMesh->m_MaterialsToBodyParts(Idx);
			const UBOOL bHasCutBone = ( BodyPart.m_CutBone != NAME_None );
			bShow = !bHasCutBone || !BodyPart.m_bShowIfCut;
		}
		_rMeshComponent.ShowMaterialSection( Idx, bShow, LODIndex );
	}
}

// DISHONORED(port): 2013 rva 0x7c8640 (2012 0x82d600): the component's element is overridden wherever the mesh's own
// material is the reference one, so one variation material can replace the same base material on several sections.
void DisReplaceMatchingMaterialsInSkelMesh( USkeletalMeshComponent* _pMeshComponent, const UMaterialInterface* _pReferenceMaterial, UMaterialInterface* _pMaterial )
{
	if( !_pMeshComponent || !_pMeshComponent->SkeletalMesh )
	{
		return;
	}
	USkeletalMesh* SkeletalMesh = _pMeshComponent->SkeletalMesh;
	for( INT Idx = 0; Idx < SkeletalMesh->Materials.Num(); Idx++ )
	{
		if( SkeletalMesh->Materials(Idx) == _pReferenceMaterial )
		{
			_pMeshComponent->SetMaterial( Idx, _pMaterial );
		}
	}
}

// DISHONORED(port): 2013 rva 0x7cfb50 (2012 0x8321e0): the index form reads the mesh's own material at that index and
// hands it to the reference form, which is what makes a variation apply to every section sharing that material.
void DisReplaceMatchingMaterialsInSkelMesh( USkeletalMeshComponent* _pMeshComponent, INT _MaterialIndex, UMaterialInterface* _pMaterial )
{
	if( !_pMeshComponent || !_pMeshComponent->SkeletalMesh )
	{
		return;
	}
	USkeletalMesh* SkeletalMesh = _pMeshComponent->SkeletalMesh;
	if( !SkeletalMesh->Materials.IsValidIndex( _MaterialIndex ) )
	{
		return;
	}
	DisReplaceMatchingMaterialsInSkelMesh( _pMeshComponent, SkeletalMesh->Materials(_MaterialIndex), _pMaterial );
}

// DISHONORED(port): agent EO. 2012 rva 0x82cd60
FString DisEnumTypeToString( INT _Value, const TCHAR* _EnumPath )
{
	UEnum* Enum = FindObject<UEnum>( NULL, _EnumPath );
	if( Enum == NULL || _Value < 0 || _Value >= Enum->NumEnums() )
	{
		return FString();
	}
	return Enum->GetEnum( _Value ).ToString();
}
