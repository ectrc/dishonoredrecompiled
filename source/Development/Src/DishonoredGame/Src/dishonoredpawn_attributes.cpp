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
#include "ifdiag.h"
#include "jgdiag.h"

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

// agent JG (PHASE24 JG): 2012 0x78e880 / 2013 0x749780 (41 B), the fourth tick phase: the timed modifiers age, then the attributes are
// applied.
void ADishonoredPawn::Tick_Attributes( FLOAT DeltaTime, ELevelTick TickType )
{
	GetAttributes().TickAttributes( DeltaTime );
	ApplyAttributes();
}

/**
 * agent JG (PHASE24 JG): 2012 0x794860 / 2013 0x756880 (678 / 950 B; 2013 adds four ammo kinds). Every tick: the ground speed (the
 * corpse-carrying one while carrying), water speed and acceleration from the attributes, the ammo capacities (2013's eleven, the
 * spring razor is the player's) into the inventory, then the crouch height, maximum fall speed and walkable floor from the pawn
 * tweaks. The attribute names are retail's DNAME_Attribute_* globals, read back from their 2013 FName initialisers (0x9e5d96..0x9e6dee).
 * DISHONORED(written): an ammo slot the inventory does not have is skipped (retail checks the index and stops; GetAttributes builds
 * the attribute set on first use, see above).
 */
void ADishonoredPawn::ApplyAttributes()
{
	static const FName NAME_DisGroundSpeed( TEXT("Attribute_GroundSpeed") );
	static const FName NAME_DisGroundSpeedCarryingCorpse( TEXT("Attribute_GroundSpeedCarryingCorpse") );
	static const FName NAME_DisWaterSpeed( TEXT("Attribute_WaterSpeed") );
	static const FName NAME_DisAccelerationRate( TEXT("Attribute_AccelerationRate") );
	static const FName NAME_DisAmmoCapacity[12] =
	{
		FName( TEXT("Attribute_BulletCapacity") ), FName( TEXT("Attribute_ExplosiveBulletCapacity") ), FName( TEXT("Attribute_ArrowCapacity") ),
		FName( TEXT("Attribute_SleepArrowCapacity") ), FName( TEXT("Attribute_FlareArrowCapacity") ), FName( NAME_None ),
		FName( TEXT("Attribute_GrenadeCapacity") ), FName( TEXT("Attribute_StickyGrenadeCapacity") ), FName( TEXT("Attribute_ExplosiveArrowCapacity") ),
		FName( TEXT("Attribute_ArcMineCapacity") ), FName( TEXT("Attribute_FlashSmokeCapacity") ), FName( TEXT("Attribute_ArcMineStunCapacity") ),
	};
	UDisTweaks_Pawn* Tweaks = GetPawnTweaks();
	GroundSpeed = GetAttributeValue( IsCarryingCorpse() ? NAME_DisGroundSpeedCarryingCorpse : NAME_DisGroundSpeed );
	WaterSpeed = GetAttributeValue( NAME_DisWaterSpeed );
	AccelRate = GetAttributeValue( NAME_DisAccelerationRate );
	if( m_pInventory )
	{
		for( INT Slot = 0; Slot < ARRAY_COUNT(NAME_DisAmmoCapacity); Slot++ )
		{
			if( NAME_DisAmmoCapacity[Slot] != NAME_None && m_pInventory->m_AmmoInfo.IsValidIndex( Slot ) )
			{
				m_pInventory->m_AmmoInfo(Slot).m_AmmoCapacity = (INT)GetAttributeValue( NAME_DisAmmoCapacity[Slot] );
			}
		}
	}
	CrouchHeight = Tweaks->m_fCrouchHeight;
	MaxFallSpeed = Tweaks->m_fMaxFallSpeed;
	WalkableFloorZ = appCos( Tweaks->m_fMaxWalkableFloorInDegrees * 0.017453292f );
	DisJGNoteAttributes( this );
}

/**
 * DISHONORED(port): agent IF. 2012 rva 0x794b10 / 2013 0x756c40 (441 B both), the APawn virtual CalcVelocity scales the maximum
 * speed and the acceleration by every frame. Uncrouched: a sprint replaces the engine's modifier with the sprint speed attribute
 * (times the strafe or backward sprint multiplier) over GroundSpeed; a sneak multiplies it by CrouchedPct, or replaces it with the
 * sneak strafe/backward multiplier while strafing; a run while strafing or backing replaces it with the run multiplier. The
 * class's own factor (MaxSpeedModifier_Derived: 1.0 here, the sword factor for the player) multiplies the result, and a drop is
 * blended down at the pawn tweaks' m_fSpeedBlendDownSpeed per second (a rise is immediate).
 * A crouched pawn (the engine's crouch, which the swim state uses) keeps APawn's modifier (CrouchedPct). NPCs walk through
 * their own physWalking (dishonorednpcpawn_locomotion.cpp), which does not call CalcVelocity. The attribute names are the
 * UDisTweaks_Pawn_Attributes members with m_ replaced by Attribute_ (retail's DNAME_Attribute_* globals, the 2013 ones read back
 * from their FName initialisers at 0x9e62e8..0x9e6480).
 */
FLOAT ADishonoredPawn::MaxSpeedModifier()
{
	static const FName NAME_DisGroundSpeedSprint( TEXT("Attribute_GroundSpeedSprint") );
	static const FName NAME_DisGroundStrafeMultiplierSprint( TEXT("Attribute_GroundStrafeMultiplierSprint") );
	static const FName NAME_DisGroundBackwardMultiplierSprint( TEXT("Attribute_GroundBackwardMultiplierSprint") );
	static const FName NAME_DisGroundStrafeMultiplierSneak( TEXT("Attribute_GroundStrafeMultiplierSneak") );
	static const FName NAME_DisGroundBackwardMultiplierSneak( TEXT("Attribute_GroundBackwardMultiplierSneak") );
	static const FName NAME_DisGroundStrafeMultiplierRun( TEXT("Attribute_GroundStrafeMultiplierRun") );
	static const FName NAME_DisGroundBackwardMultiplierRun( TEXT("Attribute_GroundBackwardMultiplierRun") );
	FLOAT Modifier = APawn::MaxSpeedModifier();
	UDisTweaks_Pawn* Tweaks = GetPawnTweaks();
	if( !bIsCrouched )
	{
		if( m_bSprinting )
		{
			FLOAT SprintSpeed = GetAttributeValue( NAME_DisGroundSpeedSprint );
			if( m_bStrafing )
			{
				SprintSpeed *= GetAttributeValue( NAME_DisGroundStrafeMultiplierSprint );
			}
			else if( m_bMovingBackwards )
			{
				SprintSpeed *= GetAttributeValue( NAME_DisGroundBackwardMultiplierSprint );
			}
			Modifier = SprintSpeed / GroundSpeed;
		}
		else if( IsSneaking() )
		{
			if( !m_bStrafing )
			{
				Modifier *= CrouchedPct;
			}
			else
			{
				Modifier = GetAttributeValue( m_bMovingBackwards ? NAME_DisGroundBackwardMultiplierSneak : NAME_DisGroundStrafeMultiplierSneak );
			}
		}
		else if( m_bStrafing )
		{
			Modifier = GetAttributeValue( NAME_DisGroundStrafeMultiplierRun );
		}
		else if( m_bMovingBackwards )
		{
			Modifier = GetAttributeValue( NAME_DisGroundBackwardMultiplierRun );
		}
	}
	const FLOAT Derived = MaxSpeedModifier_Derived();
	FLOAT Result = Derived * Modifier;
	FLOAT Smoothed = Result;
	if( m_fLastSpeedModifier > Result && m_fLastSpeedModifier - Result > 0.0001f )
	{
		const FLOAT Alpha = Clamp( GWorld->GetDeltaSeconds() * Tweaks->m_fSpeedBlendDownSpeed, 0.f, 1.f );
		Smoothed = ( Result - m_fLastSpeedModifier ) * Alpha + m_fLastSpeedModifier;
		Result = Smoothed;
	}
	m_fLastSpeedModifier = Smoothed;
	DisIFNoteSpeedModifier( this, Modifier, Derived, Result );
	return Result;
}

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
