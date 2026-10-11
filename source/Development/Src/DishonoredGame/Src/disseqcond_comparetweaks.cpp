// DishonoredGame/src/disseqcond_comparetweaks.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x7cb2d0  public: static void __cdecl UDisSeqCond_CompareTweaks::InitializePrivateStaticClassUDisSeqCond_CompareTweaks(void)
//   0x7d19c0  class UDisTweaksBase const * __cdecl ExtractTweak(class UObject * const)
//   0x7d1a50  public: virtual void __thiscall UDisSeqCond_CompareTweaks::Activated(void)
//   0x7e9f00  public: static class UClass * __cdecl UDisSeqCond_CompareTweaks::GetPrivateStaticClassUDisSeqCond_CompareTweaks(wchar_t const *)
//   0x7edbd0  public: static class UClass * __cdecl UDisSeqCond_CompareTweaks::StaticClassNoInline(void)

// ---- agent FK (PHASE15 FK) ----
// The addresses in the banner above are the 2012 ones the importer wrote. The 2013 rvas this body was
// read against: Activated 0x78eaf0, ExtractTweak 0x78ea60.
//
// This is the one unported USequenceCondition that demonstrably fires on the first mission's own path:
// -fkdiag counted DisSeqCond_CompareTweaks x15 in l_tower_p's nine levels with SIX activations in a
// 148-second new-game run (build/agentFK/FK1.log). An unported USequenceCondition does not auto-activate
// its output links, so each of those six activations ended a Kismet chain with no log line at all
// (STATUS, agent EL). What is downstream of those six was NOT traced: see agentFK.md, the number of ops
// ever activated on this route did not change when this body landed.
#include "DishonoredGame.h"
#include "fkmatineediag.h"

/**
 * DISHONORED(port): agent FK, 2013 rva 0x78ea60 (2012 0x7d19c0). File-local in retail too.
 * The object either IS a UDisTweaksBase or carries IDisTweaksInterface; an implementer whose own tweaks
 * are NULL falls back to UDisTweaksBase's class default object, so two such objects compare equal.
 */
static const UDisTweaksBase* ExtractTweak( UObject* const Object )
{
	if( Object == NULL )
	{
		return NULL;
	}
	const UDisTweaksBase* AsTweaks = ConstCast<UDisTweaksBase>( Object );
	if( AsTweaks != NULL )
	{
		return AsTweaks;
	}
	IDisTweaksInterface* Tweaks = (IDisTweaksInterface*)Object->GetInterfaceAddress( UDisTweaksInterface::StaticClass() );
	if( Tweaks == NULL )
	{
		return NULL;
	}
	const UDisTweaksBase* Derived = Tweaks->GetTweaks_Derived();
	if( Derived == NULL )
	{
		return UDisTweaksBase::StaticClass()->GetDefaultObject<UDisTweaksBase>();
	}
	return Derived;
}

/**
 * DISHONORED(port): agent FK, 2013 rva 0x78eaf0 (2012 0x7d1a50). Three output links:
 * 0 the same tweaks, 1 related through the fallback chain, 2 unrelated - and 2 is also where a pair
 * with a missing tweak goes. Retail evaluates Cast<IDisTweaksInterface> on both inputs first and
 * discards the result; that is dead code and is left out.
 */
void UDisSeqCond_CompareTweaks::Activated()
{
	const UDisTweaksBase* TweaksA = ExtractTweak( m_pA );
	const UDisTweaksBase* TweaksB = ExtractTweak( m_pB );

	INT Output = 2;
	if( TweaksA != NULL && TweaksB != NULL )
	{
		if( TweaksA == TweaksB )
		{
			Output = 0;
		}
		else if( TweaksA->IsFallbackRelated( TweaksB ) )
		{
			Output = 1;
		}
	}
	const UBOOL bFollowed = OutputLinks( Output ).ActivateOutputLink();

	if( DisFKDiagEnabled() )
	{
		debugf( TEXT("DISHONORED(bringup): fkdiag CompareTweaks %s: A %s B %s -> output %d, link %s %d connection(s)"),
			*GetPathName(),
			( TweaksA != NULL ) ? *TweaksA->GetName() : TEXT("NONE"),
			( TweaksB != NULL ) ? *TweaksB->GetName() : TEXT("NONE"),
			Output, bFollowed ? TEXT("pulsed") : TEXT("refused"),
			OutputLinks( Output ).Links.Num() );
	}
}

/**
 * DISHONORED(bringup): agent FK. External linkage, referenced from DishonoredGameRegistrants.cpp.
 * IMPLEMENT_CLASS's vtable is a folded COMDAT and not a reference into this object, so without an
 * anchor MSVC drops the whole unit out of the static library and Activated never dispatches - agent FI
 * measured exactly that on Src/disseqact_startspawn.cpp (agentFI.md section 9).
 */
void DishonoredLinkDisSeqCondCompareTweaks()
{
}
