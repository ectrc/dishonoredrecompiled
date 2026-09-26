// DishonoredGame/src/disinteractableinterface.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (30):
//   0x669a40  public: static void __cdecl UDisInteractableInterface::InitializePrivateStaticClassUDisInteractableInterface(void)
//   0x669a60  public: static void __cdecl UDisTweaks_InteractableInterface::InitializePrivateStaticClassUDisTweaks_InteractableInterface(void)
//   0x669b70  public: virtual class FString const & __thiscall IDisInteractableInterface::GetCrosshairFocusText(void)const
//   0x669b80  private: virtual unsigned int __thiscall IDisInteractableInterface::AttemptCannotUseInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x669ba0  public: virtual unsigned int __thiscall IDisInteractableInterface::ShouldBlockInteractProbe(struct FCanInteractParams const &)const
//   0x66bdd0  public: virtual void __thiscall IDisInteractableInterface::FormatText(class FString &)const
//   0x66fb80  public: static class UClass * __cdecl UDisInteractableInterface::GetPrivateStaticClassUDisInteractableInterface(wchar_t const *)
//   0x672d50  public: static class UClass * __cdecl UDisInteractableInterface::StaticClassNoInline(void)
//   0x674030  public: void __thiscall IDisInteractableInterface::EndInteract(struct FEndInteractParams const &)
//   0x6774e0  private: void __thiscall IDisInteractableInterface::WitnessInteraction(class AActor * const, class ADishonoredPawn * const)const
//   0x678010  public: unsigned int __thiscall IDisInteractableInterface::AttemptInteract(class ADishonoredPawn * const)
//   0x678150  public: unsigned int __thiscall IDisInteractableInterface::AttemptAltInteract(class ADishonoredPawn *)
//   0x6782a0  private: virtual unsigned int __thiscall IDisInteractableInterface::AttemptAltInteract_Derived(class ADishonoredPawn *, unsigned int &)
//   0x678320  public: unsigned int __thiscall IDisInteractableInterface::AttemptCannotUseInteract(class ADishonoredPawn *)
//   0x67a900  public: static class UClass * __cdecl UDisTweaks_InteractableInterface::GetPrivateStaticClassUDisTweaks_InteractableInterface(wchar_t const *)
//   0x67bb60  public: static class UClass * __cdecl UDisTweaks_InteractableInterface::StaticClassNoInline(void)
//   0x67c3f0  public: class UDisTweaks_InteractableInterface const & __thiscall IDisInteractableInterface::GetInteractableTweaks(void)const
//   0x67d0b0  public: void __thiscall IDisInteractableInterface::DoHighlight(void)
//   0x67d120  public: void __thiscall IDisInteractableInterface::UnDoHighlight(void)
//   0x67d160  public: virtual class FString const & __thiscall IDisInteractableInterface::GetInteractableName(void)const
//   0x67d190  public: virtual void __thiscall IDisInteractableInterface::FillUIInteraction(struct FDisUIInteractionContext &)const
//   0x67d1f0  public: virtual void __thiscall IDisInteractableInterface::FillUIAltInteraction(struct FDisUIInteractionContext &)const
//   0x67d250  public: virtual class FString const & __thiscall IDisInteractableInterface::GetUseMessage(void)const
//   0x67d280  public: virtual class FString const & __thiscall IDisInteractableInterface::GetAltUseMessage(void)const
//   0x67d2b0  public: virtual class FString const & __thiscall IDisInteractableInterface::GetCannotUseMessage(void)const
//   0x681680  public: void __thiscall IDisInteractableInterface::SetHighlightBit(int)
//   0x6816b0  public: void __thiscall IDisInteractableInterface::ClearHighlightBit(int)
//   0x681720  public: virtual void __thiscall IDisInteractableInterface::GainCrosshairFocus(void)
//   0x681750  public: virtual void __thiscall IDisInteractableInterface::LoseCrosshairFocus(void)
//   0xba9890  _dynamic_initializer_for__EmptyText__

// ---- agent AU ports (PHASE7 AU): the interactable interface the pickups are collected through ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x621550 (2012 0x67c3f0): the implementing class's interactable tweaks, or the class
// default of UDisTweaks_InteractableInterface. Every text and distance accessor below goes through it.
const UDisTweaks_InteractableInterface& IDisInteractableInterface::GetInteractableTweaks() const
{
	const UDisTweaks_InteractableInterface* Tweaks = GetInteractableTweaks_Derived();
	if( !Tweaks )
	{
		Tweaks = (const UDisTweaks_InteractableInterface*)UDisTweaks_InteractableInterface::StaticClass()->GetDefaultObject();
	}
	return *Tweaks;
}

// DISHONORED(written): 2013 rva 0x6280a0 (2012 0x67d160, same bytes)
const FString& IDisInteractableInterface::GetInteractableName() const
{
	return GetInteractableTweaks().m_Name;
}

// DISHONORED(written): 2013 rva 0x628190 (2012 0x67d250, same bytes)
const FString& IDisInteractableInterface::GetUseMessage() const
{
	return GetInteractableTweaks().m_UseMessage;
}

// DISHONORED(written): 2013 rva 0x6281c0 (2012 0x67d280, same bytes)
const FString& IDisInteractableInterface::GetAltUseMessage() const
{
	return GetInteractableTweaks().m_AltUseMessage;
}

// DISHONORED(written): 2013 rva 0x6281f0 (2012 0x67d2b0, same bytes)
const FString& IDisInteractableInterface::GetCannotUseMessage() const
{
	return GetInteractableTweaks().m_CannotUseMessage;
}

// DISHONORED(written): 2013 rva 0x61c8c0 (2012 0x66bdd0): the interactable's name replaces the "`~" token of any of its
// messages (an empty name removes the token).
void IDisInteractableInterface::FormatText( FString& Text ) const
{
	const FString& Name = GetInteractableName();
	Text = Text.Replace( TEXT("`~"), Name.Len() > 0 ? *Name : TEXT("") );
}

// DISHONORED(written): 2013 rva 0x627ff0 (2012 0x67d0b0, same bytes): highlighting needs a HUD, the tweaks' permission
// and a highlight material.
void IDisInteractableInterface::DoHighlight()
{
	const UDisTweaks_InteractableInterface& Tweaks = GetInteractableTweaks();
	if( !DisGetGFxHUD() )
	{
		return;
	}
	// DISHONORED(bringup): retail also asks the HUD whether highlighting is switched on (2013 rva 0x7bd0d0's caller);
	// that accessor is GFx and not ported.
	if( Tweaks.m_bAllowedToHighlight && Tweaks.m_HighLightMaterial )
	{
		ShowHighlight( Tweaks.m_HighLightMaterial );
	}
}

// DISHONORED(written): 2013 rva 0x628060 (2012 0x67d120, same bytes)
void IDisInteractableInterface::UnDoHighlight()
{
	if( GetInteractableTweaks().m_bAllowedToHighlight )
	{
		HideHighlight();
	}
}

// DISHONORED(written): 2013 rva 0x630710 (2012 0x681680, same bytes): the first bit set turns the highlight on
void IDisInteractableInterface::SetHighlightBit( INT Bit )
{
	INT* Flags = GetHighlightFlags();
	if( !Flags )
	{
		DoHighlight();
		return;
	}
	const UBOOL bWasHighlighted = ( *Flags != 0 );
	*Flags |= Bit;
	if( !bWasHighlighted )
	{
		DoHighlight();
	}
}

// DISHONORED(written): 2013 rva 0x630740 (2012 0x6816b0, same bytes): the last bit cleared turns it off
void IDisInteractableInterface::ClearHighlightBit( INT Bit )
{
	INT* Flags = GetHighlightFlags();
	if( Flags )
	{
		*Flags &= ~Bit;
		if( *Flags != 0 )
		{
			return;
		}
	}
	UnDoHighlight();
}

// DISHONORED(written): 2013 rva 0x6307e0 (2012 0x681720, same bytes): crosshair focus is highlight bit 1
void IDisInteractableInterface::GainCrosshairFocus()
{
	SetHighlightBit( 1 );
}

// DISHONORED(written): 2013 rva 0x63b560 (2012 0x6774e0, same bytes).
// DISHONORED(bringup): the body walks every observable NPC of the interactor's component container and calls
// UDishonoredAIBrain::HandleWitnessedInteraction. FDisComponentObservable, UArkComponentContainer's typed accessor and
// UDishonoredAIBrain are all unported (agentAJ.md's dependency roots), so nothing witnesses an interaction yet.
void IDisInteractableInterface::WitnessInteraction( AActor* const InteractedWith, ADishonoredPawn* const Pawn )
{
}

// DISHONORED(written): 2013 rva 0x63df60 (2012 0x678010): the interact. The derived interact decides; on success the
// Kismet UDisSeqEvent_Interact events of the pawn fire, witnesses are told, and the use message is shown.
UBOOL IDisInteractableInterface::AttemptInteract( ADishonoredPawn* const Pawn )
{
	UBOOL bCanBeWitnessed = FALSE;
	const UBOOL bSuccess = AttemptInteract_Derived( Pawn, bCanBeWitnessed );
	if( !bSuccess )
	{
		return FALSE;
	}

	AActor* InteractedWith = (AActor*)GetUObjectInterfaceDisInteractableInterface();
	TArray<INT> IndicesToActivate;
	IndicesToActivate.AddItem( 0 );
	DisFireKismetEvent( Pawn, UDisSeqEvent_Interact::StaticClass(), Pawn, InteractedWith, TRUE, &IndicesToActivate );

	if( bCanBeWitnessed )
	{
		WitnessInteraction( InteractedWith, Pawn );
	}

	FString UseMessage = GetUseMessage();
	if( UseMessage.Len() > 0 )
	{
		FormatText( UseMessage );
		DisAddUseMessage( UseMessage );
	}
	return TRUE;
}

// DISHONORED(written): 2013 rva 0x63e200 (2012 0x678320, same bytes): the "cannot use" interact, which still counts as
// something a witness sees and still prints its own message.
UBOOL IDisInteractableInterface::AttemptCannotUseInteract( ADishonoredPawn* Pawn )
{
	UBOOL bCanBeWitnessed = TRUE;
	const UBOOL bSuccess = AttemptCannotUseInteract_Derived( Pawn, bCanBeWitnessed );
	if( !bSuccess )
	{
		return FALSE;
	}
	if( bCanBeWitnessed )
	{
		WitnessInteraction( (AActor*)GetUObjectInterfaceDisInteractableInterface(), Pawn );
	}
	FString CannotUseMessage = GetCannotUseMessage();
	if( CannotUseMessage.Len() > 0 )
	{
		FormatText( CannotUseMessage );
		DisAddUseMessage( CannotUseMessage );
	}
	return TRUE;
}
