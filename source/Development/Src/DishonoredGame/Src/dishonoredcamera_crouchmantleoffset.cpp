// DishonoredGame/src/dishonoredcamera_crouchmantleoffset.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (5):
//   0x6fc550  protected: virtual unsigned int __thiscall UDishonoredCamera_CrouchMantleOffset::GatherInfluence_Derived(struct FDishonoredViewTarget &, struct FDishonoredViewTarget const &, class APawn *, float, float)
//   0x700830  public: void __thiscall UDishonoredCamera_CrouchMantleOffset::SetOffset(void)
//   0x70a3f0  public: static class UClass * __cdecl UDishonoredCamera_CrouchMantleOffset::GetPrivateStaticClassUDishonoredCamera_CrouchMantleOffset(wchar_t const *)
//   0x70e830  public: static void __cdecl UDishonoredCamera_CrouchMantleOffset::InitializePrivateStaticClassUDishonoredCamera_CrouchMantleOffset(void)
//   0x710bb0  public: static class UClass * __cdecl UDishonoredCamera_CrouchMantleOffset::StaticClassNoInline(void)

/*-----------------------------------------------------------------------------
	agent IG (PHASE23 IG): the crouch-mantle camera offset's start - the view location relative to the player's feet, taken when a
	crouch-required mantle starts its crouch transition (UStatePlayerMasterMantle::StartCrouchTransition).
-----------------------------------------------------------------------------*/

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

void DishonoredLinkIG_DishonoredCameraCrouchMantleOffset()
{
}

// 2012 0x700830 / 2013 0x6c5340 (114 B, same bytes)
void UDishonoredCamera_CrouchMantleOffset::SetOffset()
{
	ADishonoredPlayerController* PC = ADishonoredPlayerController::s_pInstance;
	m_StartingOffset = PC->PlayerCamera->ViewTarget.POV.Location - DisGetPawnFeet( PC->Pawn );
}
