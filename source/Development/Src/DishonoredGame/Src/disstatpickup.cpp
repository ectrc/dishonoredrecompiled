// DishonoredGame/src/disstatpickup.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (21):
//   0x66aa40  protected: virtual class UDisTweaksBase * __thiscall ADisStatPickup::GetTweaks_Derived(void)
//   0x66aa50  protected: virtual unsigned int __thiscall ADisStatPickup::WantsTick_Derived(void)const
//   0x66cc90  protected: virtual void __thiscall ADisStatPickup::SetTweaks_Derived(class UDisTweaksBase *)
//   0x66cca0  private: virtual void __thiscall ADisStatPickup::FormatText(class FString &)const
//   0x66cd00  public: virtual void __thiscall ADisStatPickup::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x670780  public: virtual void __thiscall UDisTweaks_StatPickup::PostLoad(void)
//   0x678c50  public: static class UClass * __cdecl ADisStatPickup::GetPrivateStaticClassADisStatPickup(wchar_t const *)
//   0x67b0d0  public: static void __cdecl ADisStatPickup::InitializePrivateStaticClassADisStatPickup(void)
//   0x67b0f0  public: static class UClass * __cdecl UDisTweaks_StatPickup::GetPrivateStaticClassUDisTweaks_StatPickup(wchar_t const *)
//   0x67be10  public: static class UClass * __cdecl ADisStatPickup::StaticClassNoInline(void)
//   0x67c780  public: static void __cdecl UDisTweaks_StatPickup::InitializePrivateStaticClassUDisTweaks_StatPickup(void)
//   0x680ac0  public: static class UClass * __cdecl UDisTweaks_StatPickup::StaticClassNoInline(void)
//   0x680af0  public: virtual void __thiscall ADisStatPickup::GameLoad(class FArchive &, enum ESaveLoadLocation)
//   0x6872c0  public: virtual void __thiscall ADisStatPickup::PostBeginPlay(void)
//   0x687330  public: virtual unsigned int __thiscall ADisStatPickup::DoInteract_Impl(class ADishonoredPlayerPawn *)
//   0x687460  public: unsigned int __thiscall ADisStatPickup::HasStatsSet(void)const
//   0x687520  public: virtual unsigned int __thiscall ADisStatPickup::CanBePickedUp(class ADishonoredPlayerPawn *)const
//   0x687610  private: virtual class FString const & __thiscall ADisStatPickup::GetUseMessage(void)const
//   0x687690  private: void __thiscall ADisStatPickup::Explode(void)
//   0x68aed0  private: virtual void __thiscall ADisStatPickup::TakeDamage_Impl(int, class AController * const, class FVector const &, class FVector const &, class UClass * const, struct FTraceHitInfo const &, class AActor * const)
//   0x68afd0  public: virtual unsigned int __thiscall ADisStatPickup::Tick(float, enum ELevelTick)

// DISHONORED(written): ADisStatPickup's IDisTweaksInterface slots. GetTweaks_Derived 2013 rva 0x3710 (2012 0x66aa40), SetTweaks_Derived 0x61dd90 (0x66cc90).
// Without these every pickup reads the class default of its tweaks class, so its item, ammo and elixir amounts are all
// zero - which is exactly what the first -dispickupprobe run measured before they were added.

// ---- agent AU ports (PHASE7 AU): the coin / ammo / potion pickup ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): the GetTweaks_Derived-or-class-default shape of every ADisStatPickup body
UDisTweaks_StatPickup* ADisStatPickup::GetStatTweaks() const
{
	UDisTweaksBase* Tweaks = const_cast<ADisStatPickup*>( this )->GetTweaks_Derived();
	if( !Tweaks )
	{
		Tweaks = (UDisTweaksBase*)UDisTweaks_StatPickup::StaticClass()->GetDefaultObject();
	}
	return (UDisTweaks_StatPickup*)Tweaks;
}

// DISHONORED(written): 2013 rva 0x636730 (2012 0x6872c0): each of the twelve ammo types rolls its own amount out of the
// tweaks' range, so two instances of the same coin pile hold different amounts.
void ADisStatPickup::PostBeginPlay()
{
	ADisPickup_Base::PostBeginPlay();
	UDisTweaks_StatPickup* Tweaks = GetStatTweaks();
	for( INT Type = 0; Type < eDisAmmoType_MAX; Type++ )
	{
		// DISHONORED(written): FDisRangedInt::GetRandomIntValue (2013 rva 0x7d0a30) is the inclusive random pick the
		// generated struct has no method for.
		const FDisRangedInt& Range = Tweaks->m_AmmoRanges[Type];
		m_CurAmmo[Type] = Range.m_MaxValue > Range.m_MinValue
			? Range.m_MinValue + ( appRand() % ( Range.m_MaxValue - Range.m_MinValue + 1 ) )
			: Range.m_MinValue;
	}
}

// DISHONORED(written): 2013 rva 0x6368d0 (2012 0x687460): anything to give at all
UBOOL ADisStatPickup::HasStatsSet() const
{
	for( INT Type = 0; Type < eDisAmmoType_MAX; Type++ )
	{
		if( m_CurAmmo[Type] )
		{
			return TRUE;
		}
	}
	UDisTweaks_StatPickup* Tweaks = GetStatTweaks();
	return Tweaks->m_HealthChange != 0 || Tweaks->m_ManaChange != 0;
}

// DISHONORED(written): 2013 rva 0x636980 (2012 0x687520): an exploding pickup cannot be taken; otherwise it can when it
// carries health or mana, or when any of its ammo types still fits in the player's capacity.
UBOOL ADisStatPickup::CanBePickedUp( ADishonoredPlayerPawn* PlayerPawn ) const
{
	if( m_fExplosionChainTimer != 0.f )
	{
		return FALSE;
	}
	UBOOL bRoomForAmmo = FALSE;
	UDishonoredInventory* Inventory = PlayerPawn ? PlayerPawn->m_pInventory : NULL;
	if( Inventory )
	{
		for( INT Type = 0; Type < eDisAmmoType_MAX && Type < Inventory->m_AmmoInfo.Num(); Type++ )
		{
			if( m_CurAmmo[Type] && Inventory->m_AmmoInfo(Type).m_AmmoCount < Inventory->m_AmmoInfo(Type).m_AmmoCapacity )
			{
				bRoomForAmmo = TRUE;
			}
		}
	}
	UDisTweaks_StatPickup* Tweaks = GetStatTweaks();
	return bRoomForAmmo || Tweaks->m_HealthChange != 0 || Tweaks->m_ManaChange != 0;
}

// DISHONORED(written): 2013 rva 0x6367a0 (2012 0x687330): health (raised by the player's potion-potency attribute and,
// for a food pickup, by the food-heal attribute), mana, and then the ammo through the inventory.
// DISHONORED(bringup): IDisAttributesInterface::GetAttributeValue needs UDisAttributes, which is not ported, so every
// bonus attribute reads as 0 - i.e. exactly an un-upgraded Corvo. ADishonoredPlayerPawn::AddMana (2013 rva 0x6a1ac0)
// and its SetMana (vtable +1824) are not ported either, so the clamped add is written here instead.
UBOOL ADisStatPickup::DoInteract_Impl( ADishonoredPlayerPawn* PlayerPawn )
{
	if( !PlayerPawn )
	{
		return FALSE;
	}
	UDisTweaks_StatPickup* Tweaks = GetStatTweaks();
	UBOOL bTookSomething = FALSE;

	const INT HealthIncrease = Tweaks->m_HealthChange;
	if( HealthIncrease > 0 )
	{
		PlayerPawn->Health = Min<INT>( PlayerPawn->Health + HealthIncrease, PlayerPawn->HealthMax );
		bTookSomething = TRUE;
	}

	const INT ManaIncrease = Tweaks->m_ManaChange;
	if( ManaIncrease > 0 )
	{
		PlayerPawn->m_Mana = Min<INT>( PlayerPawn->m_Mana + ManaIncrease, PlayerPawn->m_ManaMax );
		bTookSomething = TRUE;
	}

	const UBOOL bTookAmmo = PlayerPawn->m_pInventory
		? PlayerPawn->m_pInventory->ConsumeStatPickup( this, &m_LastConsumedAmmoCount )
		: FALSE;
	return bTookSomething || bTookAmmo;
}

// DISHONORED(written): 2013 rva 0x636a70 (2012 0x687610): one unit of ammo, none, or the generic interactable message
const FString& ADisStatPickup::GetUseMessage() const
{
	if( m_LastConsumedAmmoCount <= 1 )
	{
		UDisTweaks_StatPickup* Tweaks = GetStatTweaks();
		return m_LastConsumedAmmoCount ? Tweaks->m_SingleAmmoPickupMessage : Tweaks->m_EmptyPickupMessage;
	}
	return IDisInteractableInterface::GetUseMessage();
}

// DISHONORED(written): 2013 rva 0x61dda0 (2012 0x66cca0): the amount taken replaces the "`n" token
void ADisStatPickup::FormatText( FString& Text ) const
{
	IDisInteractableInterface::FormatText( Text );
	Text = Text.Replace( TEXT("`n"), *FString::Printf( TEXT("%i"), m_LastConsumedAmmoCount ) );
}

// DISHONORED(written): 2013 rva 0x619ce0 (2012 0x66aa50, same bytes)
UBOOL ADisStatPickup::WantsTick_Derived() const
{
	return m_fExplosionChainTimer != 0.f || ADisPickup_Base::WantsTick_Derived();
}

// DISHONORED(written): 2013 rva 0x63cdc0 (2012 0x68afd0): the chain-explosion countdown, then the base tick
UBOOL ADisStatPickup::Tick( FLOAT DeltaTime, ELevelTick TickType )
{
	if( m_fExplosionChainTimer > 0.f )
	{
		m_fExplosionChainTimer -= DeltaTime;
		if( m_fExplosionChainTimer <= 0.f )
		{
			m_fExplosionChainTimer = -1.f;
			Explode();
		}
	}
	return ADisPickup_Base::Tick( DeltaTime, TickType );
}

// DISHONORED(written): 2013 rva 0x636af0 (2012 0x687690): the tweaks' explosion actor is spawned in place of the pickup.
// DISHONORED(bringup): the spawned explosion's own kick (retail also calls into the explosion actor after spawning) is
// not ported; UDisTweaksBase::SpawnActor is agent AJ's and is used as it stands.
void ADisStatPickup::Explode()
{
	UDisTweaks_StatPickup* Tweaks = GetStatTweaks();
	if( !Tweaks->m_pExplosion )
	{
		return;
	}
	SetCollisionType( COLLIDE_NoCollision );
	AActor* Explosion = Tweaks->m_pExplosion->SpawnActor( 0, NAME_None, Location, Rotation, NULL, TRUE, FALSE, NULL, NULL, TRUE );
	if( Explosion )
	{
		Explosion->Instigator = DisGetValidInstigator( *this );
	}
	DisDestroyActorNextTick( *this );
	if( LightEnvironment )
	{
		LightEnvironment->bDynamic = TRUE;
	}
	SetTickIsDisabled( FALSE );
}

// DISHONORED(written): ADisStatPickup's IDisTweaksInterface slots. GetTweaks_Derived 2013 rva 0x3710 (2012 0x66aa40), SetTweaks_Derived 0x61dd90 (0x66cc90).
UDisTweaksBase* ADisStatPickup::GetTweaks_Derived()
{
	return m_pStatPickupTweaks;
}

void ADisStatPickup::SetTweaks_Derived( UDisTweaksBase* Tweaks )
{
	m_pStatPickupTweaks = (UDisTweaks_StatPickup*)Tweaks;
}
