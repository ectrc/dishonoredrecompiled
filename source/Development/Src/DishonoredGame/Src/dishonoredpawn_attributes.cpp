// DishonoredGame/src/dishonoredpawn_attributes.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x78e850  public: struct FDisRelationshipOverrideInfo const & __thiscall ADishonoredPawn::GetPersonalRelationships(void)const
//   0x78e860  public: class UDisTweaks_Faction const * __thiscall ADishonoredPawn::RequestFactionTweak(void)const
//   0x78e880  public: virtual void __thiscall ADishonoredPawn::Tick_Attributes(float, enum ELevelTick)
//   0x78e8b0  private: virtual class UDisAttributes & __thiscall ADishonoredPawn::GetAttributes(void)
//   0x78e8c0  public: void __thiscall ADishonoredPawn::SetSprinting(unsigned int)
//   0x794860  public: virtual void __thiscall ADishonoredPawn::ApplyAttributes(void)
//   0x794b10  public: virtual float __thiscall ADishonoredPawn::MaxSpeedModifier(void)
//   0x794cd0  public: void __thiscall ADishonoredPawn::OnDifficultyChange(class FArkGameEvent const &)
//   0x79b730  public: virtual void __thiscall ADishonoredPawn::PreBeginPlay_Attributes(void)

#include "DishonoredGame.h"

// ---- agent BF ports (PHASE8 BF): the pawn's half of the attributes system ----

// DISHONORED(written): 2013 rva 0x7497b0 (2012 0x78e8b0): m_pAttributes (@1916; the seven-byte retail body reads +724
// from the IDisAttributesInterface subobject at @1192).
// DISHONORED(bringup): retail builds m_pAttributes in PreBeginPlay_Attributes, which ADishonoredPawn::PreBeginPlay
// (2013 rva 0x748f10) calls. That PreBeginPlay is not ported - it also needs PreBeginPlay_NativeComponents (0x755a10),
// UArkComponentContainer::StartAllComponents, PreBeginPlay_Inventory and PreBeginPlay_Actions, none of which exist - so
// the construction is deferred to the first read here. PreBeginPlay_Attributes itself is ported below and is what runs.
UDisAttributes& ADishonoredPawn::GetAttributes()
{
	if( !m_pAttributes )
	{
		PreBeginPlay_Attributes();
	}
	return *m_pAttributes;
}

// DISHONORED(written): 2013 rva 0x762190 (2012 0x79b730): clear the sprint bit, then build the attributes object out of
// the pawn tweaks' m_pAttributeTweaks[1] (the tweak object at UDisTweaks_Pawn @264; retail hard-codes index 1 of the
// four-element array and nothing reads the other three - the per-difficulty values live inside each FDisAttribute).
// DISHONORED(port): retail then registers OnDifficultyChange on FArkGameEventDispatcher event 9. FArkGameEventDispatcher
// is a comment-only skeleton (Engine/Inc/arkgameeventdispatcher.h), so the registration is left out and the attributes
// keep the difficulty they were built with.
void ADishonoredPawn::PreBeginPlay_Attributes()
{
	m_bSprinting = FALSE;
	UDisTweaks_Pawn_Attributes* AttributeTweaks = GetPawnTweaks()->m_pAttributeTweaks[1];
	if( !AttributeTweaks )
	{
		// DISHONORED(bringup): retail passes the pointer on unchecked. A pawn whose tweak object is missing gets an empty
		// attribute set rather than a NULL dereference, and says so once per class.
		AttributeTweaks = UDisTweaks_Pawn_Attributes::StaticClass()->GetDefaultObject<UDisTweaks_Pawn_Attributes>();
		static TArray<UClass*> Named;
		if( Named.FindItemIndex( GetClass() ) == INDEX_NONE )
		{
			Named.AddItem( GetClass() );
			debugf( TEXT("DISHONORED(bringup): %s has no m_pAttributeTweaks[1]: attributes come from the class default"),
				*GetClass()->GetName() );
		}
	}
	m_pAttributes = AttributeTweaks->ConstructAttributes( this );
	if( UDisAttributes::IsCensusEnabled() )
	{
		m_pAttributes->DumpAttributes( *FString::Printf( TEXT("%s from %s"), *GetName(), *AttributeTweaks->GetName() ) );
	}
}

// DISHONORED(written): 2013 rva (2012 0x794cd0): rebuild the attribute values from the same tweak object when the
// difficulty changes. Its caller is the FArkGameEventDispatcher registration above, which is not ported.
void ADishonoredPawn::OnDifficultyChange()
{
	UDisTweaks_Pawn_Attributes* AttributeTweaks = GetPawnTweaks()->m_pAttributeTweaks[1];
	if( AttributeTweaks && m_pAttributes )
	{
		AttributeTweaks->RefreshAttributesFromSource( m_pAttributes, this );
	}
}

// DISHONORED(port): ADishonoredPawn::Tick_Attributes (2013 rva 0x748880 region, 2012 0x78e880) is
// m_pAttributes->TickAttributes( DeltaTime ) followed by ApplyAttributes, and ADishonoredPawn::Tick (2013 rva 0x750720)
// is what calls it, gated on the pawn's bit 0x80000000 @1288. Tick is not ported, so neither is Tick_Attributes: nothing
// adds a timed modifier yet either (UDisTweaks_Upgrade::ApplyAttributes 0x64f460 and the two Kismet actions are the only
// callers of AddAttributeModifier and none of them is ported), so no modifier can need ageing.
// DISHONORED(port): ApplyAttributes (2012 rva 0x794860) writes GroundSpeed / WaterSpeed / AccelRate from
// Attribute_GroundSpeed[CarryingCorpse] / Attribute_WaterSpeed / Attribute_AccelerationRate and the eight ammo capacities
// into m_pInventory->m_AmmoInfo, then CrouchHeight / MaxFallSpeed / WalkableFloorZ from the body tweaks. It needs
// ADishonoredPawn::IsCarryingCorpse, which does not exist, and it would overwrite the movement values the walking path
// depends on the moment Tick is ported - so it is deliberately left out and named here instead.
// DISHONORED(port): MaxSpeedModifier (2012 rva 0x794b10) is an APawn virtual the engine's movement code calls every
// frame. Its body needs MaxSpeedModifier_Derived, IsSneaking and IsCarryingCorpse, none of which are ported, and a
// partial port would multiply the pawn's speed by 0 and stop it walking. Left out on purpose.

// agentDO:setsprinting
/**
 * DISHONORED(port): 2013 rva 0x7497c0 (2012 0x78e8c0), 57 bytes. The flag is written unconditionally and the
 * notification fires only on a change, which is what makes it safe to call every frame.
 */
void ADishonoredPawn::SetSprinting( UBOOL bSprinting )
{
	const UBOOL bWasSprinting = m_bSprinting ? TRUE : FALSE;
	m_bSprinting = bSprinting ? TRUE : FALSE;
	if( bWasSprinting != ( bSprinting ? TRUE : FALSE ) )
	{
		OnSprintChange_Derived();
	}
}
