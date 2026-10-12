// DishonoredGame/src/dishonoredpawn_camera.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (2):
//   0x7aa7a0  public: void __thiscall ADishonoredPawn::OnCameraUpdate(struct FDishonoredViewTarget const &, int)
//   0x7afa20  public: class ADishonoredPlayerCamera * __thiscall ADishonoredPawn::GetDishonoredCamera(void)

// Agent FZ (PHASE18 FZ): GetDishonoredCamera 2012 0x7afa20 / 2013 0x7515c0 (named in both, 43 B). OnCameraUpdate
// (0x7aa7a0: OnCameraUpdate_Derived then the inventory's OnCameraUpdate on pass 1) is not ported - neither callee exists.

#include "DishonoredGame.h"

// the external-linkage anchor DishonoredGameRegistrants.cpp names (a static-library member nothing refers to is dropped whole)
void DishonoredLinkFZ_PawnCamera()
{
}

ADishonoredPlayerCamera* ADishonoredPawn::GetDishonoredCamera() const
{
	ADishonoredPlayerController* PC = Cast<ADishonoredPlayerController>( Controller );
	return PC ? Cast<ADishonoredPlayerCamera>( PC->PlayerCamera ) : NULL;
}
