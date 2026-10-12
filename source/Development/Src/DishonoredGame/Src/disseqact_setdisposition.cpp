// DishonoredGame/src/disseqact_setdisposition.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4), 2012 rvas:
//   0x7cb8e0  public: static void __cdecl UDisSeqAct_SetDisposition::InitializePrivateStaticClassUDisSeqAct_SetDisposition(void)
//   0x7d36b0  public: virtual void __thiscall UDisSeqAct_SetDisposition::Activated(void)
//   0x7ebbb0  public: static class UClass * __cdecl UDisSeqAct_SetDisposition::GetPrivateStaticClassUDisSeqAct_SetDisposition(wchar_t const *)
//   0x7ee600  public: static class UClass * __cdecl UDisSeqAct_SetDisposition::StaticClassNoInline(void)

#include "DishonoredGame.h"

// DISHONORED(port): 2013 rva 0x790a50 (2012 0x7d36b0, 105 B): the action has three inputs - Set, Clear, Clear All - and
// remembers which one fired in m_SetDispositionKismetInput before the handler (OnSetDisposition on each target) reads it.
// With none of the three impulsed the byte keeps its last value, as retail's does.
void UDisSeqAct_SetDisposition::Activated()
{
	for( INT Idx = 0; Idx < 3 && Idx < InputLinks.Num(); Idx++ )
	{
		if( InputLinks(Idx).bHasImpulse )
		{
			m_SetDispositionKismetInput = Idx;
			break;
		}
	}
	Super::Activated();
}

// DISHONORED(bringup, agent FV): the unit's only function is a virtual, so nothing references this object and MSVC
// would drop it out of the static library (the pitfall agents EN and FI recorded). Named from DishonoredGameRegistrants.cpp.
void DishonoredLinkDisSeqActSetDisposition()
{
}
