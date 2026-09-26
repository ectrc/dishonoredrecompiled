// DishonoredGame/src/dishonoredutilities_saveload.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (12):
//   0x8245c0  void __cdecl DisSaveLoadObject(class FArchive &, class UObject *)
//   0x8245e0  class FName __cdecl DisGetLevelName(class ULevel *)
//   0x82f1c0  class ULevel * __cdecl DisGetCurrentLevel(void)
//   0x82f2e0  class ULevel * __cdecl DisFindLevelFromName(class FName const &)
//   0x82f3d0  unsigned int __cdecl DisIsObjectStateGoingToBeRestored(class UObject *)
//   0x82f420  void __cdecl DisSavePhysicsAssetInstanceBodies(class USkeletalMeshComponent *, class FArchive &, enum ESaveLoadLocation)
//   0x82f4c0  void __cdecl DisLoadPhysicsAssetInstanceBodies(class USkeletalMeshComponent *, class FArchive &, enum ESaveLoadLocation)
//   0x82f550  void __cdecl DisPushDisableSave(enum EDisDisableSaveType)
//   0x82f580  void __cdecl DisPopDisableSave(enum EDisDisableSaveType, float)
//   0x82f5b0  void __cdecl DisPushIgnoreAutosave(enum EDisIgnoreAutosaveType)
//   0x82f5e0  void __cdecl DisPopIgnoreAutosave(enum EDisIgnoreAutosaveType)
//   0x82f610  void __cdecl DisPeriodicAutosave(float)

// ---- agent AU ports (PHASE7 AU) ----

#include "DishonoredGame.h"
#include "dishonoredutilities.h"

// DISHONORED(written): 2013 rva 0x7ec6a0 (2012 0x82f3d0): while a game is loading, and for every object the saved level
// state covers, the object's own begin-play must not build state the save is about to overwrite.
UBOOL DisIsObjectStateGoingToBeRestored( UObject* Object )
{
	UDishonoredEngine* Engine = Cast<UDishonoredEngine>( GEngine );
	if( !Engine )
	{
		return FALSE;
	}
	if( Engine->IsLoadingGame() )
	{
		return TRUE;
	}
	// DISHONORED(bringup): UDishonoredEngine::IsObjectPartOfSavedLevelState is not ported (DisSaveLoad, agentAJ.md's
	// hand-over 6), so only the loading-game half of the test answers.
	return FALSE;
}
