// DishonoredGame/src/disusestate_base.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (3):
//   0x722240  public: static void __cdecl UDisUseState_Base::InitializePrivateStaticClassUDisUseState_Base(void)
//   0x72abb0  public: static class UClass * __cdecl UDisUseState_Base::GetPrivateStaticClassUDisUseState_Base(wchar_t const *)
//   0x72db00  public: static class UClass * __cdecl UDisUseState_Base::StaticClassNoInline(void)

// DISHONORED(port): agent GD (PHASE19 GD). The base state has no bodies in retail; the parameters that enter the four states are
// Inc/disusestate.h. This unit is the link anchor of the five: every body in the four state units is a virtual, so nothing
// references them and MSVC would drop them out of the static library. Named from DishonoredGameRegistrants.cpp.
#include "DishonoredGame.h"

void DishonoredLinkDisUseStates()
{
}
