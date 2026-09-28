// DishonoredGame/src/dishonorednpcpawn_body.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (44):
//   0x7abb70  public: static void __cdecl UDisTweaks_ComponentMeshOffset::InitializePrivateStaticClassUDisTweaks_ComponentMeshOffset(void)
//   0x7abb90  public: enum eDisNPCBodyStance __thiscall ADishonoredNPCPawn::GetCurrentManifestedBodyStance(void)const
//   0x7abba0  public: void __thiscall ADishonoredNPCPawn::SetSpineBendingTarget(class FVector const &)
//   0x7abbd0  public: void __thiscall ADishonoredNPCPawn::ClearSpineBendingTarget(void)
//   0x7abbe0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsCorpseDetectable(void)const
//   0x7abc10  public: virtual class IDisRelationshipInterface * __thiscall ADishonoredNPCPawn::GetRelationshipInterface(void)
//   0x7abc30  public: void __thiscall ADishonoredNPCPawn::SetCurrentMurderer(class ADishonoredPawn *)
//   0x7abc40  public: class UStaticMeshComponent * __thiscall ADishonoredNPCPawn::GetAccessoryStaticMeshComponent(enum eDisAccessoryType)const
//   0x7adcc0  class FArchive & __cdecl operator<<(class FArchive &, struct FDisMaterialReplacement &)
//   0x7adde0  protected: class FVector __thiscall ADishonoredNPCPawn::ComputeTurnAngle(class UDisTweaks_NPCPawn_Body const * const, float &)const
//   0x7ae310  public: void __thiscall ADishonoredNPCPawn::SetBodyIntention(enum EDisBodyIntentionPriority, enum eDisNPCBodyStance, class UClass * const, class UClass * const)
//   0x7ae420  public: enum eDisNPCBodyStance __thiscall ADishonoredNPCPawn::GetBodyStance(void)const
//   0x7ae440  public: class UClass * __thiscall ADishonoredNPCPawn::GetDesiredPrimaryItem(void)const
//   0x7ae460  public: class UClass * __thiscall ADishonoredNPCPawn::GetDesiredSecondaryItem(void)const
//   0x7ae480  public: void __thiscall ADishonoredNPCPawn::ClearBodyIntention(enum EDisBodyIntentionPriority)
//   0x7ae4a0  public: void __thiscall ADishonoredNPCPawn::GoToLimpState(class ADishonoredPawn *)
//   0x7ae500  public: virtual void __thiscall ADishonoredNPCPawn::ModifyMovementExtents(class FVector &, class FVector &, class FVector &, class FVector &, unsigned int &)const
//   0x7ae530  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsCorpseKnockedOut(void)const
//   0x7ae560  public: virtual unsigned int __thiscall ADishonoredNPCPawn::CanDeathBeWitnessed(void)const
//   0x7ae590  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IsAttnTargetBeingMurdered(class IDisAttentionTargetInterface * *)const
//   0x7ae5c0  private: virtual void __thiscall ADishonoredNPCPawn::OnExitStealthVolume(class ADisStealthVolume *)
//   0x7ae5f0  private: class UStaticMeshComponent * __thiscall ADishonoredNPCPawn::CreateAccessoryStaticMeshComponent(enum eDisAccessoryType)
//   0x7b29b0  public: void __thiscall ADishonoredNPCPawn::RestoreAccessories(int const *, int const *)
//   0x7b2a70  protected: unsigned int __thiscall ADishonoredNPCPawn::IsFootPlacementEnabled(void)const
//   0x7b2b40  protected: unsigned int __thiscall ADishonoredNPCPawn::IsActionSpineBendingEnabled(void)const
//   0x7b2bf0  protected: void __thiscall ADishonoredNPCPawn::Tick_NPCFootPlacement(float)
//   0x7b2cc0  public: virtual void __thiscall ADishonoredNPCPawn::NPCTouchedDeepWater(void)
//   0x7b2e20  public: unsigned int __thiscall ADishonoredNPCPawn::AllowSurfacesDisableFootPlacement(void)const
//   0x7b2ec0  public: virtual class UClass * __thiscall ADishonoredNPCPawn::GetImpactContactType(struct FImpactInfo const &, class UClass * const, enum eDisPawnHitReactionType)const
//   0x7b6ec0  public: void __thiscall FDisMaterialsOverride::Set(class UMaterialInterface *, class UMaterialInterface *)
//   0x7b7050  protected: unsigned int __thiscall ADishonoredNPCPawn::IsSlopeOffsettingEnabled(void)const
//   0x7b7120  protected: void __thiscall ADishonoredNPCPawn::Tick_SpineBending_Apply(float)
//   0x7bbf50  public: void __thiscall FDisMeshMaterialVariationList::ApplyMaterialVariationToMesh(class USkeletalMeshComponent *, struct FDisMaterialsOverride &)const
//   0x7bc110  public: static void __cdecl FDisMeshMaterialVariationList::RestoreMaterialVariationToMesh(class USkeletalMeshComponent *, struct FDisMaterialsOverride const &)
//   0x7bc180  public: virtual void __thiscall ADishonoredNPCPawn::PostBeginPlay_Body(void)
//   0x7bc480  protected: void __thiscall ADishonoredNPCPawn::Tick_SpineBending(float)
//   0x7be180  public: virtual void __thiscall ADishonoredNPCPawn::Tick_Body(float, enum ELevelTick)
//   0x7be670  public: void __thiscall ADishonoredNPCPawn::RestoreAppearance(int, struct FDisMaterialsOverride const &, struct FDisMaterialsOverride const &, int const *, int const *)
//   0x7bf2c0  public: static class UClass * __cdecl UDisTweaks_ComponentMeshOffset::GetPrivateStaticClassUDisTweaks_ComponentMeshOffset(wchar_t const *)
//   0x7c11d0  public: static class UClass * __cdecl UDisTweaks_ComponentMeshOffset::StaticClassNoInline(void)
//   0x7c1200  public: virtual unsigned int __thiscall ADishonoredNPCPawn::OnTeleport_Native(class USeqAct_Teleport *)
//   0x7c42c0  protected: virtual class UClass * __thiscall ADishonoredNPCPawn::ChooseFootfallContactType(void)const
//   0x7c4320  protected: virtual unsigned int __thiscall ADishonoredNPCPawn::IsAutoFootfallEnabled(void)const
//   0x7c94a0  public: virtual unsigned int __thiscall ADishonoredNPCPawn::IgnoreBlockingBy(class AActor const *)const

#include "DishonoredGame.h"

// ---- agent BF ports (PHASE8 BF) ----

// DISHONORED(written): the generated exec wrapper of ADishonoredNPCPawn's own script declaration of
// TakeFallingDamage_Native. The class has no C++ override - the 2012 PDB has no
// ADishonoredNPCPawn::TakeFallingDamage_Native symbol - so the exec dispatches through the virtual to
// ADishonoredPawn::TakeFallingDamage_Native (2013 rva 0x74dcc0), which is why ICF folded this wrapper onto
// ADishonoredPawn::execTakeFallingDamage_Native (2013 rva 0x5ec5f0).
void ADishonoredNPCPawn::execTakeFallingDamage_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_STRUCT(FVector, HitNormal);
	P_GET_ACTOR(FloorActor);
	P_FINISH;
	*(INT*)Result = TakeFallingDamage_Native( HitNormal, FloorActor );
}

// ---- agent DI ports (PHASE10 DI): the modular character ----

#include "dishonoredutilities.h"
#include "disheadcensus.h"

// DISHONORED(port): 2013 rva 0x776f80 (2012 0x7b6ec0), retail's FDisMaterialsOverride::Set. The override list is a
// (default, custom) pair per replaced material, so the pawn can put the mesh's own materials back when the variation is
// dropped: setting a custom material either updates the pair whose default matches or appends a new one, and clearing it
// removes the pair.
// DISHONORED(bringup): retail spells this as a member of FDisMaterialsOverride. The struct is generated
// (DishonoredGameGlobalStructsClasses.h) and only grows methods through a new Inc/CppText/<Struct>.h, which needs the
// DishonoredGame generator re-run; it is a file-local function here so this package does not regenerate the module. The
// body is retail's.
static void DisMaterialsOverrideSet( FDisMaterialsOverride& _rOverride, UMaterialInterface* _pDefault, UMaterialInterface* _pCustom )
{
	for( INT Idx = 0; Idx < _rOverride.m_MaterialReplacements.Num(); Idx++ )
	{
		if( _rOverride.m_MaterialReplacements(Idx).m_pDefaultMaterial == _pDefault )
		{
			if( _pCustom )
			{
				_rOverride.m_MaterialReplacements(Idx).m_pCustomMaterial = _pCustom;
			}
			else
			{
				_rOverride.m_MaterialReplacements.RemoveSwap( Idx, 1 );
			}
			return;
		}
	}
	if( _pCustom )
	{
		const INT Added = _rOverride.m_MaterialReplacements.Add( 1 );
		_rOverride.m_MaterialReplacements(Added).m_pDefaultMaterial = _pDefault;
		_rOverride.m_MaterialReplacements(Added).m_pCustomMaterial = _pCustom;
	}
}

// DISHONORED(port): 2013 rva 0x77b0d0 (2012 0x7bbf50), retail's
// FDisMeshMaterialVariationList::ApplyMaterialVariationToMesh. This is what makes two guards wearing the same mesh look
// different: the variation list is indexed by the mesh's own material index, and each entry is a weighted draw over
// replacement materials. Every replacement made is recorded in the pawn's override list so it can be undone.
// DISHONORED(bringup): a file-local function for the same reason as DisMaterialsOverrideSet above; the body is retail's.
static void DisApplyMaterialVariationToMesh( const FDisMeshMaterialVariationList& _rList, USkeletalMeshComponent* _pMeshComponent, FDisMaterialsOverride& _rOverride )
{
	const INT NumVariationMaterials = _rList.m_VariationMaterialIndex.Num();
	_rOverride.m_MaterialReplacements.Empty( NumVariationMaterials );
	if( !_pMeshComponent )
	{
		return;
	}
	for( INT Idx = 0; Idx < NumVariationMaterials; Idx++ )
	{
		USkeletalMesh* SkeletalMesh = _pMeshComponent->SkeletalMesh;
		if( !SkeletalMesh || !SkeletalMesh->Materials.IsValidIndex( Idx ) )
		{
			continue;
		}
		UMaterialInterface* ExistingMaterial = SkeletalMesh->Materials(Idx);
		if( !ExistingMaterial )
		{
			continue;
		}
		const FDisBodyMaterialVariation& VariationList = _rList.m_VariationMaterialIndex(Idx);
		const FLOAT fRandChance = appFrand();
		FLOAT fRunningChance = 0.f;
		for( INT Variation = 0; Variation < VariationList.m_MaterialVariation.Num(); Variation++ )
		{
			fRunningChance += VariationList.m_MaterialVariation(Variation).m_fRandomChance;
			if( fRunningChance > fRandChance )
			{
				UMaterialInterface* NewMaterial = VariationList.m_MaterialVariation(Variation).m_pMaterial;
				if( NewMaterial )
				{
					DisReplaceMatchingMaterialsInSkelMesh( _pMeshComponent, Idx, NewMaterial );
					DisMaterialsOverrideSet( _rOverride, ExistingMaterial, NewMaterial );
				}
				break;
			}
		}
	}
}

// DISHONORED(port): 2013 rva 0x76e0f0 (2012 0x7abc40) and 0x7706b0 (0x7ae5f0): the hat and the mask are static meshes
// hung off the body mesh's socket, one component per accessory type, created on first use and kept for the pawn's life.
// The component shares the pawn's light environment and takes the body mesh as its shadow parent, casts no shadow of its
// own, has no rigid-body channel and does not collide.
UStaticMeshComponent* ADishonoredNPCPawn::GetAccessoryStaticMeshComponent( BYTE _eAccessoryType ) const
{
	return ( _eAccessoryType < eDisAccessoryType_MAX ) ? m_AccessoryComponents[_eAccessoryType] : NULL;
}

UStaticMeshComponent* ADishonoredNPCPawn::CreateAccessoryStaticMeshComponent( BYTE _eAccessoryType )
{
	if( _eAccessoryType >= eDisAccessoryType_MAX )
	{
		return NULL;
	}
	if( m_AccessoryComponents[_eAccessoryType] )
	{
		return m_AccessoryComponents[_eAccessoryType];
	}
	UStaticMeshComponent* Component = ConstructObject<UStaticMeshComponent>( UStaticMeshComponent::StaticClass(), this );
	m_AccessoryComponents[_eAccessoryType] = Component;
	Component->SetLightEnvironment( m_pLightEnvironment );
	Component->SetShadowParent( Mesh );
	// retail's three bitfield writes, spelled out: (@280 & 0xFFFFF3AF) | 0x800, (@276 & 0xFFFE1FFF),
	// (m_CollisionTraceTypes @320 & 0xFFFFFF80)
	Component->CollideActors = FALSE;
	Component->BlockActors = FALSE;
	Component->BlockRigidBody = FALSE;
	Component->bDisableAllRigidBody = TRUE;
	Component->bAcceptsDecals = FALSE;
	Component->bAcceptsDecalsDuringGameplay = FALSE;
	Component->bAcceptsStaticDecals = FALSE;
	Component->bAcceptsDynamicDecals = FALSE;
	Component->SetRBChannel( RBCC_Nothing );
	Component->m_CollisionTraceTypes.SetAllMovementTrace( FALSE );
	Component->m_CollisionTraceTypes.SetAllGameplayTrace( FALSE );
	return Component;
}

// DISHONORED(port): 2013 rva 0x77f8a0 (2012 0x7bc180). **This is the function whose absence left every NPC headless.**
// A Dishonored character is not one skeletal mesh: the body is on APawn::Mesh (set from the tweaks by
// ApplyTweakChanges_Derived) and the head is a second skeletal mesh component, m_pHeadMesh, which the pawn's archetype
// creates empty. This pass draws one head out of the tweaks' m_RandomHeadMeshes and sets it on that component; a
// USkeletalMeshComponent with no USkeletalMesh fails USkeletalMeshComponent::IsValidComponent, so until it is set the
// component is never even attached. Retail then applies the body's own material variation, hangs the hat and mask
// accessories off the body mesh's sockets, finds the spine bender, resets the rotation intents and the body intentions,
// and records how far above the pawn's origin the possession camera sits.
// DISHONORED(bringup): two calls of retail's body are left out and named:
//   USkeletalMeshComponent::SetFaceFXAsset( Mesh, HeadMesh->FaceFXAsset ) (2013 rva 0x312b90) - this build has
//     WITH_FACEFX off, so the Engine tree has no SetFaceFXAsset and faces do not lip-sync;
//   retail dereferences Tweaks->m_pVisionTweak unchecked for the possession-camera bone; it is guarded here, so an NPC
//     whose tweaks have no vision object keeps m_fPossessCamZOffset at its default instead of crashing.
void ADishonoredNPCPawn::PostBeginPlay_Body()
{
	Super::PostBeginPlay_Body();

	UDisTweaks_NPCPawn* Tweaks = Cast<UDisTweaks_NPCPawn>( GetTweaks_Derived() );
	if( !Tweaks )
	{
		Tweaks = UDisTweaks_NPCPawn::StaticClass()->GetDefaultObject<UDisTweaks_NPCPawn>();
	}
	UDisTweaks_NPCPawn_Body* BodyTweaks = Cast<UDisTweaks_NPCPawn_Body>( Tweaks->m_pBodyTweaks );

	// retail inlines ADishonoredNPCPawn::Init_Rotation here: no rotation intent is active and every request id is free
	m_CurrentNPCRotationPriority = 0;
	for( INT Idx = 0; Idx < ARRAY_COUNT(m_NPCRotationIntent); Idx++ )
	{
		m_NPCRotationIntent[Idx].m_bIsActive = FALSE;
		m_NPCRotationIntent[Idx].m_RequestID = INDEX_NONE;
	}

	const FDisBodyMesh* HeadMesh = DisChooseRandomMesh<FDisBodyMesh>( Tweaks->m_RandomHeadMeshes, m_iRandomHeadMeshSel );
	if( HeadMesh && HeadMesh->m_pMesh && m_pHeadMesh && !DisHeadSuppressed() )
	{
		m_pHeadMesh->SetSkeletalMesh( HeadMesh->m_pMesh, FALSE );
		DisHideGoreSections( *m_pHeadMesh );
		DisApplyMaterialVariationToMesh( HeadMesh->m_MaterialVariation, m_pHeadMesh, m_HeadMeshMaterialsOverride );
		GDisHeadMeshesSet++;
	}
	else
	{
		// retail: an NPC whose tweaks name no head mesh loses the component altogether
		DetachComponent( m_pHeadMesh );
		m_pHeadMesh = NULL;
		GDisHeadMeshesDetached++;
	}

	if( Mesh )
	{
		DisHideGoreSections( *Mesh );
		DisApplyMaterialVariationToMesh( Tweaks->m_BodyVariationMaterials, Mesh, m_BodyMeshMaterialsOverride );
	}

	for( INT Idx = 0; Idx < eDisAccessoryType_MAX; Idx++ )
	{
		m_RandomAccessoriesSel[Idx] = INDEX_NONE;
	}
	for( INT AccessoryType = 0; AccessoryType < eDisAccessoryType_MAX; AccessoryType++ )
	{
		if( Tweaks->m_RandomAccessoryInfo[AccessoryType].m_fRandomChanceOfUse <= appFrand() )
		{
			continue;
		}
		// retail 2013 inlines UDisTweaks_NPCPawn::GetAccessoryMeshes (2012 rva 0x7c9ae0) as this two-way choice
		const TArray<FDisPawnAccessoryMesh>* AccessoryMeshes = NULL;
		if( AccessoryType == eDisAccessoryType_Hat )
		{
			AccessoryMeshes = &Tweaks->m_HatMeshes;
		}
		else if( AccessoryType == eDisAccessoryType_Mask )
		{
			AccessoryMeshes = &Tweaks->m_MaskMeshes;
		}
		if( !AccessoryMeshes || AccessoryMeshes->Num() == 0 )
		{
			continue;
		}
		const FDisPawnAccessoryMesh* Accessory = DisChooseRandomMesh<FDisPawnAccessoryMesh>( *AccessoryMeshes, m_RandomAccessoriesSel[AccessoryType] );
		UStaticMeshComponent* Component = CreateAccessoryStaticMeshComponent( AccessoryType );
		if( !Accessory || !Component || !Mesh )
		{
			continue;
		}
		Component->SetStaticMesh( Accessory->m_pMesh, FALSE );
		Mesh->AttachComponentToSocket( Component, Accessory->m_SocketName );
	}

	if( Mesh && BodyTweaks )
	{
		m_pSpineBender = Cast<UDisSkelControl_SpineBender>( Mesh->FindSkelControl( BodyTweaks->m_SpineBenderName ) );
	}

	m_bLocoModifierNeedsApplication = FALSE;
	m_CurrentManifestedBodyStance = 0;
	m_CurrentSecondaryItemPriority = 0;
	m_CurrentPrimaryItemPriority = 0;
	m_CurrentBodyStancePriority = 0;
	for( INT Idx = 0; Idx < ARRAY_COUNT(m_BodyIntention); Idx++ )
	{
		m_BodyIntention[Idx].m_IntendedBodyStance = 0;
		m_BodyIntention[Idx].m_pDesiredPrimaryItemClass = NULL;
		m_BodyIntention[Idx].m_pDesiredSecondaryItemClass = NULL;
	}

	if( Tweaks->m_pVisionTweak )
	{
		FVector BonePos( 0.f, 0.f, 0.f );
		GetBone_ByName( Tweaks->m_pVisionTweak->m_Vision_BoneName, &BonePos, NULL );
		m_fPossessCamZOffset = BonePos.Z - Location.Z;
	}

	GDisHeadBodyPasses++;
}

// ---- agent DI: the -dishead census (PHASE10 DI) ----
//
// DISHONORED(written): not a retail unit. Dishonored composes an NPC from two skeletal meshes - the body on APawn::Mesh
// and the head on ADishonoredNPCPawn::m_pHeadMesh (retail offset 3312) - so "the NPCs have no heads" is a question about
// one component per pawn. This census answers it in one line per run second: how many spawned NPC pawns hold a head
// component, how many of those are attached, how many have a USkeletalMesh set, how many resolve at least one material,
// and how many the renderer actually drew (LastRenderTime), out of the population the spawners produced. Free when
// -dishead is absent; hung off the same per-frame script call as agent CG's -disai, so no engine file is touched for it.

#include "disheadcensus.h"

INT GDisHeadBodyPasses = 0;
INT GDisHeadMeshesSet = 0;
INT GDisHeadMeshesDetached = 0;

static INT GDisHeadCensus = -1;

UBOOL DisHeadCensusEnabled()
{
	// DISHONORED(bringup): read on first use, never as a file-scope static initialiser - in a static library those run
	// before WinMain sets GCmdLine and the switch is then always FALSE (agent CA, PHASE9 "Rules for agents").
	if( GDisHeadCensus < 0 )
	{
		GDisHeadCensus = ( appStrfind( appCmdLine(), TEXT("-dishead") ) != NULL ) ? 1 : 0;
	}
	return GDisHeadCensus != 0;
}

struct FDisHeadCensusState
{
	UWorld* World;
	FLOAT NextReportTime;
	UBOOL bDumpedOne;

	FDisHeadCensusState() : World(NULL), NextReportTime(0.f), bDumpedOne(FALSE) {}
};

static FDisHeadCensusState GDisHeadState;

/** Materials that actually resolve on a mesh component, which is what decides whether an attached mesh draws anything. */
static INT DisHeadCountMaterials( USkeletalMeshComponent* Component )
{
	INT Resolved = 0;
	if( Component )
	{
		const INT Num = Component->GetNumElements();
		for( INT Idx = 0; Idx < Num; Idx++ )
		{
			if( Component->GetMaterial( Idx ) != NULL )
			{
				Resolved++;
			}
		}
	}
	return Resolved;
}

/** One pawn, spelled out: the state of both meshes and of the tweak entry the head is supposed to come from. */
static FString DisHeadDescribe( ADishonoredNPCPawn* Pawn )
{
	UDisTweaks_NPCPawn* Tweaks = Cast<UDisTweaks_NPCPawn>( Pawn->GetTweaks_Derived() );
	const INT HeadChoices = Tweaks ? Tweaks->m_RandomHeadMeshes.Num() : -1;
	USkeletalMeshComponent* Body = Pawn->Mesh;
	USkeletalMeshComponent* Head = Pawn->m_pHeadMesh;
	return FString::Printf(
		TEXT("%s tweaks=%s headChoices=%i body[%s mesh=%s attached=%i mats=%i lastRender=%.2f] head[%s mesh=%s attached=%i parentAnim=%s mats=%i hidden=%i lastRender=%.2f] accessories=%i/%i sel=%i"),
		*Pawn->GetName(),
		Tweaks ? *Tweaks->GetName() : TEXT("NULL"),
		HeadChoices,
		Body ? *Body->GetName() : TEXT("NULL"),
		( Body && Body->SkeletalMesh ) ? *Body->SkeletalMesh->GetName() : TEXT("NULL"),
		Body ? (INT)Body->IsAttached() : 0,
		DisHeadCountMaterials( Body ),
		Body ? Body->LastRenderTime : -1.f,
		Head ? *Head->GetName() : TEXT("NULL"),
		( Head && Head->SkeletalMesh ) ? *Head->SkeletalMesh->GetName() : TEXT("NULL"),
		Head ? (INT)Head->IsAttached() : 0,
		( Head && Head->ParentAnimComponent ) ? *Head->ParentAnimComponent->GetName() : TEXT("NULL"),
		DisHeadCountMaterials( Head ),
		Head ? (INT)Head->HiddenGame : 0,
		Head ? Head->LastRenderTime : -1.f,
		Pawn->m_AccessoryComponents[0] ? 1 : 0,
		Pawn->m_AccessoryComponents[1] ? 1 : 0,
		Pawn->m_iRandomHeadMeshSel );
}

/** Every component the pawn owns, once, so a missing head component can be told apart from an unset one. */
static void DisHeadDumpComponents( ADishonoredNPCPawn* Pawn )
{
	INT Skeletal = 0;
	FString Line;
	for( INT Idx = 0; Idx < Pawn->Components.Num(); Idx++ )
	{
		UActorComponent* Component = Pawn->Components(Idx);
		if( !Component )
		{
			continue;
		}
		USkeletalMeshComponent* Skel = Cast<USkeletalMeshComponent>( Component );
		if( Skel )
		{
			Skeletal++;
		}
		Line += FString::Printf( TEXT("%s(%s%s) "),
			*Component->GetClass()->GetName(),
			Component->IsAttached() ? TEXT("a") : TEXT("-"),
			Skel ? ( Skel->SkeletalMesh ? TEXT("+m") : TEXT("-m") ) : TEXT("") );
	}
	debugf( TEXT("dishead components of %s: %i components, %i skeletal: %s"),
		*Pawn->GetName(), Pawn->Components.Num(), Skeletal, *Line );
	debugf( TEXT("dishead attachments of %s: %s"), *Pawn->GetName(), *DisHeadDescribe( Pawn ) );
}

void DisHeadReport( UWorld* World, FLOAT DeltaSeconds )
{
	if( !World || !World->GetWorldInfo() )
	{
		return;
	}
	const FLOAT Now = World->GetTimeSeconds();
	if( GDisHeadState.World != World )
	{
		GDisHeadState = FDisHeadCensusState();
		GDisHeadState.World = World;
	}
	if( Now < GDisHeadState.NextReportTime )
	{
		return;
	}
	GDisHeadState.NextReportTime = Now + 1.f;

	INT Pawns = 0;
	INT WithComponent = 0;
	INT Attached = 0;
	INT WithMesh = 0;
	INT WithMaterial = 0;
	INT Drawn = 0;
	INT BodyDrawn = 0;
	INT WithParentAnim = 0;
	INT HeadChoicesTotal = 0;
	ADishonoredNPCPawn* First = NULL;
	for( FActorIterator It; It; ++It )
	{
		ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( *It );
		if( !Pawn || Pawn->IsPendingKill() )
		{
			continue;
		}
		Pawns++;
		if( !First )
		{
			First = Pawn;
		}
		UDisTweaks_NPCPawn* Tweaks = Cast<UDisTweaks_NPCPawn>( Pawn->GetTweaks_Derived() );
		if( Tweaks )
		{
			HeadChoicesTotal += Tweaks->m_RandomHeadMeshes.Num();
		}
		if( Pawn->Mesh && Pawn->Mesh->LastRenderTime > 0.f && Now - Pawn->Mesh->LastRenderTime < 1.f )
		{
			BodyDrawn++;
		}
		USkeletalMeshComponent* Head = Pawn->m_pHeadMesh;
		if( !Head )
		{
			continue;
		}
		WithComponent++;
		if( Head->IsAttached() )
		{
			Attached++;
		}
		if( Head->SkeletalMesh )
		{
			WithMesh++;
		}
		if( DisHeadCountMaterials( Head ) > 0 )
		{
			WithMaterial++;
		}
		if( Head->ParentAnimComponent )
		{
			WithParentAnim++;
		}
		if( Head->LastRenderTime > 0.f && Now - Head->LastRenderTime < 1.f )
		{
			Drawn++;
		}
	}

	debugf( TEXT("dishead census: %i NPC pawns; head component %i, attached %i, mesh set %i, with material %i, parentAnim %i, drawn this second %i (bodies drawn %i); head choices in tweaks %i; PostBeginPlay_Body %i passes, %i heads set, %i detached"),
		Pawns, WithComponent, Attached, WithMesh, WithMaterial, WithParentAnim, Drawn, BodyDrawn,
		HeadChoicesTotal, GDisHeadBodyPasses, GDisHeadMeshesSet, GDisHeadMeshesDetached );
	if( First )
	{
		debugf( TEXT("dishead first: %s"), *DisHeadDescribe( First ) );
		if( !GDisHeadState.bDumpedOne )
		{
			GDisHeadState.bDumpedOne = TRUE;
			DisHeadDumpComponents( First );
		}
	}
}

// DISHONORED(bringup): -disheadcam[=<seconds>] frames one spawned NPC for a screenshot. The NPCs of L_Tower_P are put
// down by 41 spawners spread over the mission, so a run that starts at the player's spawn anchor never has one in shot;
// rather than drive a camera (UDishonoredCamera is stubbed, STATUS.md), this takes the player's own view point and holds
// one NPC in front of it, facing the player, with its physics off so gravity does not walk it out of frame. Nothing here
// runs unless the switch is present, and it moves nothing but that one NPC.
static INT GDisHeadCam = -1;
static FLOAT GDisHeadCamTime = 0.f;
static ADishonoredNPCPawn* GDisHeadCamTarget = NULL;

UBOOL DisHeadCamEnabled()
{
	if( GDisHeadCam < 0 )
	{
		GDisHeadCam = ( appStrfind( appCmdLine(), TEXT("-disheadcam") ) != NULL ) ? 1 : 0;
		GDisHeadCamTime = 30.f;
		Parse( appCmdLine(), TEXT("disheadcam="), GDisHeadCamTime );
	}
	return GDisHeadCam != 0;
}

void DisHeadFrameOneNPC( ADishonoredPlayerController* Controller, UWorld* World )
{
	if( !Controller || !World || World->GetTimeSeconds() < GDisHeadCamTime )
	{
		return;
	}
	FVector CamLocation( 0.f, 0.f, 0.f );
	FRotator CamRotation( 0, 0, 0 );
	Controller->GetPlayerViewPoint( CamLocation, CamRotation );

	if( !GDisHeadCamTarget || GDisHeadCamTarget->IsPendingKill() )
	{
		ADishonoredNPCPawn* AnyPawn = NULL;
		for( FActorIterator It; It; ++It )
		{
			ADishonoredNPCPawn* Pawn = Cast<ADishonoredNPCPawn>( *It );
			if( !Pawn || Pawn->IsPendingKill() )
			{
				continue;
			}
			if( !AnyPawn )
			{
				AnyPawn = Pawn;
			}
			if( Pawn->m_pHeadMesh && Pawn->m_pHeadMesh->SkeletalMesh )
			{
				GDisHeadCamTarget = Pawn;
				break;
			}
		}
		// -disnohead leaves no NPC with a head component; the pair must still frame the same character
		if( !GDisHeadCamTarget )
		{
			GDisHeadCamTarget = AnyPawn;
		}
		if( !GDisHeadCamTarget )
		{
			return;
		}
		debugf( TEXT("dishead cam: framing %s (head %s) at world time %.2f"),
			*GDisHeadCamTarget->GetName(),
			( GDisHeadCamTarget->m_pHeadMesh && GDisHeadCamTarget->m_pHeadMesh->SkeletalMesh )
				? *GDisHeadCamTarget->m_pHeadMesh->SkeletalMesh->GetName() : TEXT("NONE"),
			World->GetTimeSeconds() );
	}

	// 110 uu in front of the eye, dropped just enough that a standing NPC's head lands on the middle of the frame
	const FVector Forward = CamRotation.Vector();
	const FVector Destination = CamLocation + Forward * 110.f - FVector( 0.f, 0.f, 62.f );
	GDisHeadCamTarget->Physics = PHYS_None;
	GDisHeadCamTarget->Velocity = FVector( 0.f, 0.f, 0.f );
	World->FarMoveActor( GDisHeadCamTarget, Destination, FALSE, TRUE );
	FRotator Facing = ( CamLocation - Destination ).Rotation();
	Facing.Pitch = 0;
	Facing.Roll = 0;
	GDisHeadCamTarget->Rotation = Facing;
}

// DISHONORED(bringup): -disnohead reproduces the defect on this same binary. Before this package
// ADishonoredPawn::PostBeginPlay was not ported at all, so PostBeginPlay_Body never ran and no NPC ever got a head; with
// the switch on, the one call that sets the head mesh is skipped and everything else runs, which is what makes the
// before/after screenshots a controlled pair rather than two different builds.
static INT GDisNoHead = -1;

UBOOL DisHeadSuppressed()
{
	if( GDisNoHead < 0 )
	{
		GDisNoHead = ( appStrfind( appCmdLine(), TEXT("-disnohead") ) != NULL ) ? 1 : 0;
	}
	return GDisNoHead != 0;
}
