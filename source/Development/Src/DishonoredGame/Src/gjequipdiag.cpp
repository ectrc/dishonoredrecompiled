// Agent GJ (PHASE20 GJ): the equip-path instrument (-gjdiag). Once a second, for the player: every inventory slot (required type and
// usage, the item, its socket, its mesh - skeletal mesh, hidden, attached, parent socket, depth priority group, LOD), the two hands'
// equipped and re-equip classes and the pending keep-trying equips. Nothing here changes behaviour.

#include "DishonoredGame.h"
#include "dishonoredutilities.h"
#include "gjequipdiag.h"

UBOOL DisGJDiagEnabled()
{
	static INT State = -1;
	if( State < 0 )
	{
		State = ParseParam( appCmdLine(), TEXT("gjdiag") ) ? 1 : 0;
	}
	return State == 1;
}

static FString DisGJMesh( const UDishonoredItemSkeletalComponent* Mesh )
{
	if( !Mesh )
	{
		return FString( TEXT("mesh none") );
	}
	const USkeletalMeshComponent* Parent = Mesh->AttachedToSkelComponent;
	FName SocketName = NAME_None;
	for( INT Idx = 0; Parent && Idx < Parent->Attachments.Num(); Idx++ )
	{
		if( Parent->Attachments(Idx).Component == Mesh )
		{
			SocketName = Parent->Attachments(Idx).BoneName;
		}
	}
	return FString::Printf( TEXT("mesh %s skel %s hidden %d attached %d parent %s at %s DPG %d LOD %d tree %s animSets %d origin %.0f/%.0f/%.0f radius %.0f"), *Mesh->GetName(),
		Mesh->SkeletalMesh ? *Mesh->SkeletalMesh->GetName() : TEXT("none"), (INT)Mesh->HiddenGame, (INT)Mesh->IsAttached(),
		Parent ? *Parent->GetName() : TEXT("none"), *SocketName.ToString(), (INT)Mesh->DepthPriorityGroup, Mesh->PredictedLODLevel,
		Mesh->AnimTreeTemplate ? *Mesh->AnimTreeTemplate->GetName() : TEXT("none"), Mesh->AnimSets.Num(), Mesh->Bounds.Origin.X, Mesh->Bounds.Origin.Y, Mesh->Bounds.Origin.Z, Mesh->Bounds.SphereRadius );
}

// -gjgive=<item class>: the harness route gives the player nothing (the loadouts are logged by PostBeginPlay_Inventory), so the
// measurement spawns one item once, the way SpawnInventoryLoadout_Items would: the first loaded UDisTweaks_InventoryItem (map loadout,
// default loadout, then every loaded tweak object) whose spawned class is the named one, into the inventory, not shown - the draw is
// the exec Dis_EquipItemByType the run script sends afterwards
static void DisGJGiveOnce( ADishonoredPawn* Pawn )
{
	static UBOOL bDone = FALSE;
	FString ClassName;
	if( bDone || !Parse( appCmdLine(), TEXT("gjgive="), ClassName ) )
	{
		return;
	}
	bDone = TRUE;
	UClass* ItemClass = FindObject<UClass>( ANY_PACKAGE, *ClassName );
	if( !ItemClass )
	{
		debugf( TEXT("DISHONORED(bringup): gjgive: no class '%s'"), *ClassName );
		return;
	}
	TArray<UDisTweaks_InventoryItem*> Candidates;
	UDishonoredMapInfo* MapInfo = DishonoredGetMapInfo();
	const UDisTweaks_PlayerLoadout* MapLoadout = MapInfo ? MapInfo->GetMapSpecificPlayerLoadout() : NULL;
	UDisTweaks_PlayerPawn* Tweaks = Cast<UDisTweaks_PlayerPawn>( Pawn->GetTweaks_Derived() );
	const UDisTweaks_PlayerLoadout* DefaultLoadout = Tweaks ? Tweaks->m_pDefaultPlayerLoadout : NULL;
	if( MapLoadout )
	{
		Candidates += MapLoadout->m_InventoryLoadout.m_Items;
	}
	if( DefaultLoadout )
	{
		Candidates += DefaultLoadout->m_InventoryLoadout.m_Items;
	}
	for( TObjectIterator<UDisTweaks_InventoryItem> It; It; ++It )
	{
		if( !It->HasAnyFlags( RF_ClassDefaultObject ) )
		{
			Candidates.AddUniqueItem( *It );
		}
	}
	for( INT Idx = 0; Idx < Candidates.Num(); Idx++ )
	{
		UDisTweaks_InventoryItem* ItemTweaks = Candidates(Idx);
		UClass* SpawnedClass = ItemTweaks ? ItemTweaks->GetSpawnedObjectClass( 0 ) : NULL;
		if( SpawnedClass && SpawnedClass->IsChildOf( ItemClass ) )
		{
			UDishonoredInventoryItem* Item = Pawn->m_pInventory->SpawnAndAddInventoryItem( ItemTweaks, FALSE, FALSE, ItemTweaks->m_RequiredEquipUsage );
			debugf( TEXT("DISHONORED(bringup): gjgive: %s from tweaks %s (candidate %d of %d) -> item %s, slots %d"), *ClassName, *ItemTweaks->GetPathName(), Idx, Candidates.Num(),
				Item ? *Item->GetFullName() : TEXT("REFUSED"), Pawn->m_pInventory->m_Slots.Num() );
			// -gjequip: the item equipped at once the way a loadout's m_pDefaultEquipType_Primary is (SpawnInventoryLoadout 2012 0x7c8bb0 +0x5b)
			if( Item && ParseParam( appCmdLine(), TEXT("gjequip") ) )
			{
				const UBOOL bEquipped = Pawn->EquipItemByType( ItemClass, ItemTweaks->m_RequiredEquipUsage ? ItemTweaks->m_RequiredEquipUsage : EDisEquipUsage_Primary, ADishonoredPawn::EEquipItemFlags_InstantEquip );
				debugf( TEXT("DISHONORED(bringup): gjequip: %s instant equip -> %d"), *ClassName, (INT)bEquipped );
			}
			return;
		}
	}
	debugf( TEXT("DISHONORED(bringup): gjgive: no loaded UDisTweaks_InventoryItem spawns a %s (%d candidates)"), *ClassName, Candidates.Num() );
}

void DisGJReportInventory( ADishonoredPawn* Pawn )
{
	if( !DisGJDiagEnabled() || !Pawn || !Pawn->m_pInventory || !Pawn->IsA( ADishonoredPlayerPawn::StaticClass() ) || !GWorld )
	{
		return;
	}
	DisGJGiveOnce( Pawn );
	static FLOAT LastTime = -1.f;
	const FLOAT Now = GWorld->GetTimeSeconds();
	if( LastTime >= 0.f && Now - LastTime < 1.f )
	{
		return;
	}
	LastTime = Now;
	UDishonoredInventory* Inventory = Pawn->m_pInventory;
	ADishonoredPlayerCamera* Camera = Pawn->GetDishonoredCamera();
	FVector HandPos( 0.f, 0.f, 0.f );
	FRotator HandRot( 0, 0, 0 );
	Pawn->GetBone_ByName( FName( TEXT("handAttachment_R_jnt") ), &HandPos, &HandRot );
	debugf( TEXT("DISHONORED(bringup): gjdiag view t=%.2f camera %.0f/%.0f/%.0f rot %d/%d/%d fov %.1f arms mesh origin %.0f/%.0f/%.0f radius %.0f hand_R %.0f/%.0f/%.0f"), Now,
		Camera ? Camera->Location.X : 0.f, Camera ? Camera->Location.Y : 0.f, Camera ? Camera->Location.Z : 0.f, Camera ? Camera->Rotation.Pitch : 0, Camera ? Camera->Rotation.Yaw : 0, Camera ? Camera->Rotation.Roll : 0,
		Camera ? Camera->m_fCurFOV : 0.f, Pawn->Mesh ? Pawn->Mesh->Bounds.Origin.X : 0.f, Pawn->Mesh ? Pawn->Mesh->Bounds.Origin.Y : 0.f, Pawn->Mesh ? Pawn->Mesh->Bounds.Origin.Z : 0.f,
		Pawn->Mesh ? Pawn->Mesh->Bounds.SphereRadius : 0.f, HandPos.X, HandPos.Y, HandPos.Z );
	for( INT Idx = 0; Idx < Inventory->m_Slots.Num(); Idx++ )
	{
		const FPawnInventorySlot& Slot = Inventory->m_Slots(Idx);
		const UDishonoredInventoryItem* Item = Slot.m_pItem;
		debugf( TEXT("DISHONORED(bringup): gjdiag slot %d t=%.2f requires %s usage %d item %s equipUsage %d socket %d tick %d %s"), Idx, Now,
			Slot.m_pRequiredType ? *Slot.m_pRequiredType->GetName() : TEXT("none"), (INT)Slot.m_RequiredUsage,
			Item ? *Item->GetClass()->GetName() : TEXT("none"), Item ? (INT)Item->m_EquipUsage : -1, Item ? (INT)Item->m_CurSocket : -1,
			Item ? (INT)Item->m_bNeedToTick : -1, Item ? *DisGJMesh( Item->m_pPlayerMesh ) : TEXT("") );
	}
	for( INT Usage = EDisEquipUsage_Primary; Usage < EDisEquipUsage_MAX; Usage++ )
	{
		const UDishonoredInventoryItem* Item = Inventory->GetEquippedItem( Usage );
		UClass* Reequip = Inventory->GetReequipItem( Usage, FALSE );
		UClass* ReequipNonEmpty = Inventory->GetReequipItem( Usage, TRUE );
		UClass* Desired = Usage < Pawn->m_DesiredEquippedItems.Num() ? Pawn->m_DesiredEquippedItems(Usage).m_pItemClass : NULL;
		debugf( TEXT("DISHONORED(bringup): gjdiag hand %d t=%.2f equipped %s slot %d reequip %s nonEmpty %s desired %s pending %d"), Usage, Now,
			Item ? *Item->GetClass()->GetName() : TEXT("none"), Inventory->m_EquipUsageInfo[Usage].m_iEquippedSlot,
			Reequip ? *Reequip->GetName() : TEXT("none"), ReequipNonEmpty ? *ReequipNonEmpty->GetName() : TEXT("none"),
			Desired ? *Desired->GetName() : TEXT("none"), Pawn->m_nPendingEquip );
	}
}
