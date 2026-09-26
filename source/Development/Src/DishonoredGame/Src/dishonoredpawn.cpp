// DishonoredGame/src/dishonoredpawn.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (67):
//   0x78e1b0  public: static void __cdecl ADishonoredPawn::InitializePrivateStaticClassADishonoredPawn(void)
//   0x78e1d0  public: virtual void __thiscall ADishonoredPawn::AddReferencedObjects(class TArray<class UObject *, class FDefaultAllocator> &)
//   0x78e1f0  public: virtual void __thiscall ADishonoredPawn::Serialize(class FArchive &)
//   0x78e220  public: virtual void __thiscall ADishonoredPawn::PreBeginPlay(void)
//   0x78e270  public: virtual void __thiscall ADishonoredPawn::PossessedBy(class AController *)
//   0x78e2b0  public: virtual class USkeletalMeshComponent * __thiscall ADishonoredPawn::GetMeshByIndex(int)const
//   0x78e2d0  public: virtual void __thiscall ADishonoredPawn::PlayDying_Native(class AController *, class UClass *, class FVector)
//   0x78e2e0  public: void __thiscall ADishonoredPawn::BecomeOfficiallyDead(void)
//   0x78e300  public: unsigned int __thiscall ADishonoredPawn::IsDead(void)const
//   0x78e310  public: virtual unsigned int __thiscall ADishonoredPawn::ArkIsDeadOrDestroyed(void)const
//   0x78e320  public: unsigned int __thiscall ADishonoredPawn::ArePowersInhibited(void)const
//   0x78e350  public: virtual class UObject * __thiscall ADishonoredPawn::GetUObjectInterfaceDisLookAtInterface(void)
//   0x78e360  public: virtual class ADishonoredPawn * __thiscall ADishonoredPawn::GetNoiseMakerPawn_Derived(void)const
//   0x78e3a0  public: virtual class ADishonoredAudioVolume * __thiscall ADishonoredPawn::GetNoiseMakerAudioCellAtPoint_Derived(class FVector const &)const
//   0x78e3c0  public: virtual class FVector __thiscall ADishonoredPawn::GetEarLocation(void)const
//   0x78e3f0  public: virtual class ADishonoredAudioVolume * __thiscall ADishonoredPawn::GetListenerAudioCell(void)const
//   0x78e410  public: virtual void __thiscall ADishonoredPawn::OnSetDisposition(class UDisSeqAct_SetDisposition *)
//   0x78e420  protected: virtual class UDisTweaksBase * __thiscall ADishonoredPawn::GetTweaks_Derived(void)
//   0x78e430  public: class FDisComponentRetargeting * __thiscall ADishonoredPawn::GetComponentRetargeting(void)const
//   0x78e440  public: void __thiscall ADishonoredPawn::ConsumeRat(void)
//   0x78e490  public: void __thiscall ADishonoredPawn::DeferredDestroy(float)
//   0x78e4b0  public: virtual void __thiscall ADishonoredPawn::StartRootMotionMode(enum ERootMotionMode, enum ERootMotionRotationMode)
//   0x78e4c0  public: virtual void __thiscall ADishonoredPawn::StopRootMotionMode(void)
//   0x78e4d0  protected: virtual unsigned int __thiscall ADishonoredPawn::MAT_RootMotionEnabled(void)const
//   0x78e500  protected: void __thiscall ADishonoredPawn::MAT_StopRootMotion(void)
//   0x790c10  public: virtual unsigned int __thiscall ADishonoredPawn::Tick(float, enum ELevelTick)
//   0x790d40  public: void __thiscall ADishonoredPawn::SetIsVisible(unsigned int)
//   0x790dc0  public: virtual void __thiscall ADishonoredPawn::Landed_Native(class FVector, class AActor *)
//   0x790fc0  public: virtual void __thiscall ADishonoredPawn::SelectPower(int, enum ADishonoredPawn::EEquipItemFlags)
//   0x790ff0  protected: virtual void __thiscall ADishonoredPawn::SetTweaks_Derived(class UDisTweaksBase *)
//   0x791000  public: void __thiscall ADishonoredPawn::DrinkWater(void)
//   0x791070  public: virtual void __thiscall ADishonoredPawn::MAT_SetAnimPosition(class FName, int, class FName, float, unsigned int, unsigned int, unsigned int)
//   0x7910e0  protected: virtual void __thiscall ADishonoredPawn::MAT_SetRootMotionMode(enum EMatRootMotionMode)
//   0x791130  protected: virtual void __thiscall ADishonoredPawn::MAT_DisableMeshTranslation(unsigned int)
//   0x791170  protected: void __thiscall ADishonoredPawn::MAT_EnableMeshTranslation(void)
//   0x791190  protected: virtual void __thiscall ADishonoredPawn::MAT_SetMeshTranslationMode(enum EMatMeshTranslationMode)
//   0x793f50  protected: virtual void __thiscall ADishonoredPawn::PreBeginPlay_NativeComponents(void)
//   0x793ff0  protected: void __thiscall ADishonoredPawn::PostBeginPlay_Attachments(void)
//   0x794090  public: virtual unsigned int __thiscall ADishonoredPawn::ShouldTrace(class UPrimitiveComponent *, class AActor *, unsigned long)
//   0x794110  public: virtual void __thiscall ADishonoredPawn::NotifyBump(class AActor *, class UPrimitiveComponent *, class FVector const &)
//   0x7941c0  public: virtual void __thiscall ADishonoredPawn::Touch(class AActor *, class UPrimitiveComponent *, class FVector const &, class FVector const &)
//   0x794280  public: virtual unsigned int __thiscall ADishonoredPawn::AllowBump(class ADishonoredNPCPawn *)const
//   0x794370  public: virtual int __thiscall ADishonoredPawn::GetCurrentPower(void)const
//   0x7943a0  protected: virtual void __thiscall ADishonoredPawn::SwitchToPower(int, enum ADishonoredPawn::EEquipItemFlags)
//   0x794440  public: virtual class FVector __thiscall ADishonoredPawn::GetLookAtPoint(void)const
//   0x794540  public: virtual unsigned int __thiscall ADishonoredPawn::IsVulnerableToInstaDeath_Derived(enum EDisInstaDeathType)const
//   0x7945b0  public: virtual enum eDisPawnHitReactionType __thiscall ADishonoredPawn::IsVulnerableToHitReact_Derived(enum eDisPawnHitReactionType)const
//   0x794600  public: class FName __thiscall ADishonoredPawn::GetAnchorBoneName(void)const
//   0x794690  public: virtual unsigned short __thiscall ADishonoredPawn::GetAnchorBoneIndex(class USkeletalMeshComponent *)const
//   0x7946c0  public: virtual void __thiscall ADishonoredPawn::setPhysics(unsigned char, class AActor *, class FVector, class USkeletalMeshComponent *, class FName)
//   0x794780  public: unsigned int __thiscall ADishonoredPawn::IsBlinking(void)const
//   0x7995f0  public: virtual void __thiscall ADishonoredPawn::PostBeginPlay(void)
//   0x799760  public: virtual void __thiscall ADishonoredPawn::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7999d0  public: virtual void __thiscall ADishonoredPawn::AddAssociatedActor(class AActor *)
//   0x7999f0  public: virtual unsigned int __thiscall ADishonoredPawn::CanSplash(void)
//   0x799aa0  public: void __thiscall ADishonoredPawn::ForceDestroyAssociatedActors(void)
//   0x79b360  public: virtual void __thiscall ADishonoredPawn::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x79d300  public: virtual void __thiscall ADishonoredPawn::ClearComponents(void)
//   0x79d360  private: void __thiscall ADishonoredPawn::ShutPawnDown(void)
//   0x79ed70  private: virtual void __thiscall ADishonoredPawn::MAT_OnDestroyPreviewPawn(void)
//   0x79ed80  private: virtual void __thiscall ADishonoredPawn::OnActorTerminated(void)
//   0x79eda0  public: void __thiscall ADishonoredPawn::PreGameSave(void)
//   0x7a2d60  protected: virtual void __thiscall ADishonoredPawn::ApplyTweakChanges_Derived(void)
//   0x7a5b10  public: static class UClass * __cdecl ADishonoredPawn::GetPrivateStaticClassADishonoredPawn(wchar_t const *)
//   0x7a5d10  public: static class UClass * __cdecl ADishonoredPawn::StaticClassNoInline(void)
//   0x7a5fd0  protected: virtual void __thiscall ADishonoredPawn::HandleBendtimeTouch(class AActor *)
//   0x7a6190  protected: virtual void __thiscall ADishonoredPawn::OnOtherActorTerminated(class AActor const &)

#include "DishonoredGame.h"

// DISHONORED(written): exec 2013 rva 0x5ecc30 -> vtable +1380 = 2013 rva 0x74d580 (2013 only; not overridden in retail): the
// action is taken as a UDisSeqAct_DLC05_SetStoryGroup without a test, as retail does
void ADishonoredPawn::execOnDLC05SetStoryGroup( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(USequenceAction, _pAction);
	P_FINISH;
	m_pCurStoryGroupTweak = ((UDisSeqAct_DLC05_SetStoryGroup*)_pAction)->m_pStoryGroup;
}

// ---- natives whose retail body is trivial (generated by build/agentAC_work/gen_trivial.py from the 2013 vtables) ----

// DISHONORED(written): 2013 rva 0x5ec2c0; the retail ADishonoredPawn vtable slot +1312 it dispatches to is an empty body (or a jump to one; DisGetGameInfo() calls have no side effect)
// (2013 rva 0x8e7f30) and no retail subclass overrides it
void ADishonoredPawn::execDisplayDebug_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(AHUD, _pHUD);
	P_GET_FLOAT_REF(_rfOut_YL);
	P_GET_FLOAT_REF(_rfOut_YPos);
	P_FINISH;
}

// ---- end of trivial natives ----

// ---- agent AJ ports ----

// DISHONORED(written): 2013 rva 0x748ee0 (2012 0x78e1f0, dishonoredpawn.cpp:59)
void ADishonoredPawn::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	DisSerializeRelationshipOverrideInfo( m_PersonalRelationships, Ar );
}

// DISHONORED(written): 2013 rva 0x748f60 (2012 0x78e270, same bytes): the controller inherits the pawn's m_bAlwaysOutOfBendTime
// (Actor bitfield at 296, mask 0x40000000)
void ADishonoredPawn::PossessedBy( AController* C )
{
	Super::PossessedBy( C );
	if( C )
	{
		C->m_bAlwaysOutOfBendTime = m_bAlwaysOutOfBendTime;
	}
}

// DISHONORED(written): 2013 rva 0x749110 / 0x74d350 (2012 0x78e420 / 0x790ff0): the IDisTweaksInterface slots read and write
// m_pPawnTweaks (UDisTweaks_Pawn is declared after the pawn, so the bodies live here)
UDisTweaksBase* ADishonoredPawn::GetTweaks_Derived()
{
	return m_pPawnTweaks;
}

void ADishonoredPawn::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pPawnTweaks = Cast<UDisTweaks_Pawn>( Tweaks );
}

// DISHONORED(written): the retail pawn functions fall back to the UDisTweaks_Pawn class default when no tweaks are set
// (ApplyTweakChanges_Body 0x757060, ApplyTweakChanges_Derived 0x769720, CrushedBy_Native 0x757af0)
UDisTweaks_Pawn* ADishonoredPawn::GetPawnTweaks() const
{
	return m_pPawnTweaks ? m_pPawnTweaks : UDisTweaks_Pawn::StaticClass()->GetDefaultObject<UDisTweaks_Pawn>();
}

// DISHONORED(written): 2013 rva 0x757060 (2012 0x794fd0): the body tweaks size the collision cylinder and the step height;
// without pawn tweaks the UDisTweaks_Pawn class default answers
void ADishonoredPawn::ApplyTweakChanges_Body()
{
	UDisTweaks_Pawn_Body* Body = GetPawnTweaks()->m_pBodyTweaks;
	if( !Body || !CylinderComponent )
	{
		return;
	}
	if( Body->m_fCylinderRadius > 0.f )
	{
		CylinderComponent->CollisionRadius = Body->m_fCylinderRadius;
	}
	if( Body->m_fCylinderHalfHeight > 0.f )
	{
		CylinderComponent->CollisionHeight = Body->m_fCylinderHalfHeight;
	}
	CylinderComponent->Translation = Body->m_CylinderOffset;
	if( Body->m_fMaxStepHeight > 0.f )
	{
		MaxStepHeight = Body->m_fMaxStepHeight;
	}
}

// DISHONORED(written): 2013 rva 0x769720 (2012 0x7a2d60): after the body tweaks, the pawn tweaks (or the UDisTweaks_Pawn class
// default) drive the mesh: anim sets, anim tree template, physics asset and skeletal mesh (through the setters while attached in
// game), m_bHasJiggleBones from any full-anim-weight body whose bone the mesh has, then the faction and story-group tweaks
void ADishonoredPawn::ApplyTweakChanges_Derived()
{
	ApplyTweakChanges_Body();
	UDisTweaks_Pawn* Tweaks = GetPawnTweaks();
	UBOOL bHasJiggleBones = FALSE;
	if( Mesh )
	{
		Mesh->AnimSets = Tweaks->m_AnimSets;
		// DISHONORED(port): the tweaks' anim tree is applied unconditionally, as retail does. The -distweakanimtree gate
		// agent AJ added is gone: the Arkane nodes the tree needs (UDishonoredAnimNodeStatePicker, the two tree-ref nodes,
		// UDishonoredAnimTree, UDisAnimStatePool) are ported, so UAnimNodeBlendBase::GetBoneAtoms is no longer reached with
		// every child at weight 0 (agent AV).
		Mesh->AnimTreeTemplate = Tweaks->m_pAnimTreeTemplate;
		if( GIsGame && Mesh->IsAttached() )
		{
			Mesh->SetPhysicsAsset( Tweaks->m_pPhysicsAsset, FALSE );
			Mesh->SetSkeletalMesh( Tweaks->m_pSkeletalMesh, FALSE );
		}
		else
		{
			Mesh->PhysicsAsset = Tweaks->m_pPhysicsAsset;
			Mesh->SkeletalMesh = Tweaks->m_pSkeletalMesh;
		}
		if( Mesh->PhysicsAsset && Mesh->SkeletalMesh )
		{
			for( INT Index = 0; Index < Mesh->PhysicsAsset->BodySetup.Num(); Index++ )
			{
				URB_BodySetup* BodySetup = Mesh->PhysicsAsset->BodySetup(Index);
				if( BodySetup->bAlwaysFullAnimWeight && Mesh->SkeletalMesh->MatchRefBone( BodySetup->BoneName ) != INDEX_NONE )
				{
					bHasJiggleBones = TRUE;
					break;
				}
			}
		}
		Mesh->bHasPhysicsAssetInstance = TRUE;
		Mesh->bUpdateKinematicBonesFromAnimation = bHasJiggleBones;
	}
	m_bHasJiggleBones = bHasJiggleBones;
	m_pCurFactionTweak = Tweaks->m_pFactionTweak;
	m_pCurStoryGroupTweak = Tweaks->m_pStoryGroupTweak;
}

// ---- agent AJ ports (inventory natives) ----

// DISHONORED(written): 2013 rva 0x74a560 (2012 0x7aac20), ADishonoredPawn vtable +1356: the item is added at the back.
// DISHONORED(bringup): retail then fires FArkGameEventDispatcher event 65 (abstract item added) with (item, quantity); the
// event structs of the dispatcher are not ported.
void ADishonoredPawn::OnAddAbstractItem( UDisSeqAct_AddAbstractItem* Action )
{
	if( m_pInventory )
	{
		m_pInventory->AddAbstractItem( Action->m_pItemToAdd, Action->m_Quantity, FALSE );
	}
}

// DISHONORED(written): 2013 rva 0x74a5d0 (2012 0x7aaca0), vtable +1360: retail asks for the quantity first (the result is
// unused, the call keeps the retail order) and then removes
void ADishonoredPawn::OnRemoveAbstractItem( UDisSeqAct_RemoveAbstractItem* Action )
{
	UDisAbstractItem* Item = Action->m_pItemToRemove;
	if( !Item || !m_pInventory )
	{
		return;
	}
	m_pInventory->GetAbstractItemQuantity( Item );
	m_pInventory->RemoveAbstractItem( Item, Action->m_Quantity );
}

// DISHONORED(written): 2013 rva 0x74a610 (2012 0x7aace0), vtable +1364: the action's output is zero for an unknown item
void ADishonoredPawn::OnGetAbstractItemQuantity( UDisSeqAct_GetAbstractItemQuantity* Action )
{
	Action->m_Quantity = 0;
	if( Action->m_pItem && m_pInventory )
	{
		Action->m_Quantity = m_pInventory->GetAbstractItemQuantity( Action->m_pItem );
	}
}

// DISHONORED(written): 2013 rva 0x74a650 (2012 0x7aad20), vtable +1368: add / remove go through AddElixir with a signed
// count, set through SetElixirCount
void ADishonoredPawn::OnModifyElixirCount( UDisSeqAct_ModifyElixirCount* Action )
{
	if( !m_pInventory )
	{
		return;
	}
	switch( Action->m_ElixirOp )
	{
	case eDisElixirOp_AddElixirs:
		m_pInventory->AddElixir( Action->m_ElixirType, Action->m_Value );
		break;
	case eDisElixirOp_RemoveElixirs:
		m_pInventory->AddElixir( Action->m_ElixirType, -Action->m_Value );
		break;
	case eDisElixirOp_SetElixirCount:
		m_pInventory->SetElixirCount( Action->m_ElixirType, Action->m_Value );
		break;
	}
}

// DISHONORED(written): 2013 rva 0x751cc0 (2012 0x7b0100), vtable +1352: every entry of the action is applied with the
// action's operation (set never ignores the capacity in retail)
void ADishonoredPawn::OnModifyAmmo( UDisSeqAct_ModifyAmmo* Action )
{
	if( !m_pInventory )
	{
		return;
	}
	for( INT Index = 0; Index < Action->m_Ammo.Num(); Index++ )
	{
		const FDisInventoryAmmoEntry& Entry = Action->m_Ammo(Index);
		switch( Action->m_AmmoOp )
		{
		case eDisAmmoOp_AddAmmo:
			m_pInventory->AddAmmo( Entry.m_AmmoType, Entry.m_AmmoAmount );
			break;
		case eDisAmmoOp_SetAmmo:
			m_pInventory->SetAmmo( Entry.m_AmmoType, Entry.m_AmmoAmount, FALSE );
			break;
		case eDisAmmoOp_RemoveAmmo:
			m_pInventory->ConsumeAmmo( Entry.m_AmmoType, Entry.m_AmmoAmount );
			break;
		}
	}
}

// DISHONORED(written): 2013 rva 0x5eca70: the exec reads the action object and dispatches through the vtable, as retail does
// (the action is taken as its type without a test)
void ADishonoredPawn::execOnAddAbstractItem( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(USequenceAction, Action);
	P_FINISH;
	OnAddAbstractItem( (UDisSeqAct_AddAbstractItem*)Action );
}

// DISHONORED(written): 2013 rva 0x5ecad0
void ADishonoredPawn::execOnRemoveAbstractItem( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(USequenceAction, Action);
	P_FINISH;
	OnRemoveAbstractItem( (UDisSeqAct_RemoveAbstractItem*)Action );
}

// DISHONORED(written): 2013 rva 0x5ecb30
void ADishonoredPawn::execOnGetAbstractItemQuantity( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(USequenceAction, Action);
	P_FINISH;
	OnGetAbstractItemQuantity( (UDisSeqAct_GetAbstractItemQuantity*)Action );
}

// DISHONORED(written): 2013 rva 0x5ee590
void ADishonoredPawn::execOnModifyElixirCount( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(USequenceAction, Action);
	P_FINISH;
	OnModifyElixirCount( (UDisSeqAct_ModifyElixirCount*)Action );
}

// DISHONORED(written): 2013 rva 0x5eca10
void ADishonoredPawn::execOnModifyAmmo( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(USequenceAction, Action);
	P_FINISH;
	OnModifyAmmo( (UDisSeqAct_ModifyAmmo*)Action );
}

// DISHONORED(written): 2013 vtable slot +1328 of ADishonoredPawn is the shared 5-byte `return 0` body (2013 rva 0x233610):
// the base pawn triggers no death event. ADishonoredNPCPawn overrides the slot with the real chooser (2013 rva 0x77bf50,
// 1350 bytes) and keeps its own warn-once exec stub until that is ported, so NPC deaths still announce themselves.
UBOOL ADishonoredPawn::ChooseAndTriggerDeathEvent_Native( UClass* DamageType )
{
	return FALSE;
}

// DISHONORED(written): 2013 rva 0x5ec770 (the exec is shared with ADishonoredNPCPawn's native table entry): one object
// parameter, the result is the virtual's
void ADishonoredPawn::execChooseAndTriggerDeathEvent_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UClass, DamageType);
	P_FINISH;
	*(UBOOL*)Result = ChooseAndTriggerDeathEvent_Native( DamageType );
}

// DISHONORED(written): 2013 vtable slot +1324 of ADishonoredPawn is 13 bytes (2013 rva 0x748fd0): the base pawn only records
// that it played its death. ADishonoredNPCPawn (0x778040) and ADishonoredPlayerPawn override the slot and keep their own
// warn-once exec stubs until those are ported (ADishonoredPlayerPawn is agent AF's file).
void ADishonoredPawn::PlayDying_Native( AController* Killer, UClass* DamageType, FVector HitLocation )
{
	m_bPlayedDying = TRUE;
}

// DISHONORED(written): 2013 rva 0x5ec6a0 (the exec is shared with the NPC/player pawn native table entries): controller,
// damage type and hit location
void ADishonoredPawn::execPlayDying_Native( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(AController, Killer);
	P_GET_OBJECT(UClass, DamageType);
	P_GET_STRUCT(FVector, HitLocation);
	P_FINISH;
	PlayDying_Native( Killer, DamageType, HitLocation );
}
