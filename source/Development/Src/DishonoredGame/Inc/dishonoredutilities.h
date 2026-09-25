#pragma once
// DishonoredGame/inc/dishonoredutilities.h
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (9):
//   0x75e010  unsigned int __cdecl DisRefreshArrayClasses<class UDisTweaks_AISubState>(class TArray<class UDisTweaks_AISubState *, class FDefaultAllocator> const &, class TArray<class UDisTweaks_AISubState *, class FDefaultAllocator> &, class UObject *, unsigned int)
//   0x7b5300  struct FDisBodyMesh const * __cdecl DisChooseRandomMesh<struct FDisBodyMesh>(class TArray<struct FDisBodyMesh, class FDefaultAllocator> const &, int &)
//   0x7b5420  struct FDisPawnAccessoryMesh const * __cdecl DisChooseRandomMesh<struct FDisPawnAccessoryMesh>(class TArray<struct FDisPawnAccessoryMesh, class FDefaultAllocator> const &, int &)
//   0x7b8bb0  class UDisNPCDistractionComponent * __cdecl DisGetActorComponent<class UDisNPCDistractionComponent>(class AActor const *)
//   0x8355d0  class UDisGrenadeComponent * __cdecl DisGetActorComponent<class UDisGrenadeComponent>(class AActor const *)
//   0x8647e0  void __cdecl DisShuffle<struct FCheckResult const *, class TMemStackAllocator<class FMemStack GMainThreadMemStack, 8>>(class TArray<struct FCheckResult const *, class TMemStackAllocator<class FMemStack GMainThreadMemStack, 8>> &)
//   0x884f70  unsigned int __cdecl DisRefreshObjectClass<class UDisTweaks_AISubProcess>(class UDisTweaks_AISubProcess const *, class UDisTweaks_AISubProcess * &, class UObject *)
//   0x8bff30  class USkeletalMeshComponent * __cdecl DisGetActorComponent<class USkeletalMeshComponent>(class AActor const *)
//   0x915780  class UArkComponentContainer * __cdecl DisGetActorComponent<class UArkComponentContainer>(class AActor const *)

// DISHONORED(written): accessors of dishonoredutilities_accessors.cpp (2013 rvas in that unit)
class UDishonoredMapInfo* DishonoredGetMapInfo();
class ADishonoredGameInfo* DisGetGameInfo();
class UDisGlobalUIManager* DisGetGlobalUIManager();
class UDisLocalPlayer* DisGetLocalPlayer();
class UDishonoredAudioSystem* DisGetAudioSystem();
class UArkPpNode* DisGetArkPpNode( const FName& EffectName );
class UArkPpNodeMaterial* DisGetArkPpNodeMaterial( const FName& EffectName, UBOOL bMakeUnique );
