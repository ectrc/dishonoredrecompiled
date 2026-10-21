// Engine/src/interptrackinstfaceto.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (4):
//   0x53bb10  public: static void __cdecl UInterpTrackInstFaceTo::InitializePrivateStaticClassUInterpTrackInstFaceTo(void)
//   0x53bb30  public: virtual void __thiscall UInterpTrackInstFaceTo::TermTrackInst(class UInterpTrack *)
//   0x53cd30  public: static class UClass * __cdecl UInterpTrackInstFaceTo::GetPrivateStaticClassUInterpTrackInstFaceTo(wchar_t const *)
//   0x53ef90  public: static class UClass * __cdecl UInterpTrackInstFaceTo::StaticClassNoInline(void)

#include "EnginePrivate.h"
#include "EngineSequenceClasses.h"
#include "EngineInterpolationClasses.h"

// DISHONORED(port): agent JC. 2013 0x8fc9a0 / 2012 0x53bb30 (52 B): every request this instance asked of the group pawn's
// FArkComponentFaceTo is dropped (StopAllFaceToFromAsker); the component half is DishonoredGame's (GDisFaceToTrackTermHook).
void UInterpTrackInstFaceTo::TermTrackInst( UInterpTrack* Track )
{
	if( GDisFaceToTrackTermHook != NULL )
	{
		GDisFaceToTrackTermHook( this );
	}
}
