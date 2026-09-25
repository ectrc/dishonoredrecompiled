// GameFramework/src/gamecrowdpopulationmanager.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (17):
//   0x59abe0  public: static void __cdecl UGameCrowdPopulationManager::InitializePrivateStaticClassUGameCrowdPopulationManager(void)
//   0x5a2a50  public: class AGameCrowdAgent * __thiscall UGameCrowdPopulationManager::PickFromAgentPool(class AGameCrowdAgent *, class UGameCrowdSpawner *)
//   0x5a2c60  public: int __thiscall UGameCrowdPopulationManager::CreateAttractor(void)
//   0x5a2ca0  public: struct FGameCrowdAttractor & __thiscall UGameCrowdPopulationManager::GetAttractor(int)
//   0x5a2d40  public: class FVector __thiscall UGameCrowdPopulationManager::GetAttractionForceForAgent(class AGameCrowdAgent *)
//   0x5a2f40  public: virtual void __thiscall UGameCrowdPopulationManager::ApplyGameSettings(class ArkSettingsParameters const &, enum IArkSettingsListenerInterface::EChangeReason)
//   0x5a72c0  public: void __thiscall UGameCrowdPopulationManager::AnalyzeDestinationPoints(int, int, class FVector const &, class FVector const &)
//   0x5a7590  public: void __thiscall UGameCrowdPopulationManager::DestroyAttractor(int)
//   0x5a9180  public: void __thiscall UGameCrowdPopulationManager::UpdateDestinationPoints(float)
//   0x5a93c0  public: void __thiscall UGameCrowdPopulationManager::FlushAgents(void)
//   0x5aa760  public: static class UClass * __cdecl UGameCrowdPopulationManager::GetPrivateStaticClassUGameCrowdPopulationManager(wchar_t const *)
//   0x5aa7f0  public: virtual void __thiscall UGameCrowdPopulationManager::Terminate(void)
//   0x5aa820  public: virtual void __thiscall UGameCrowdPopulationManager::PreCommitMapChange(void)
//   0x5abf00  public: static class UClass * __cdecl UGameCrowdPopulationManager::StaticClassNoInline(void)
//   0x5b0100  public: virtual void __thiscall UGameCrowdPopulationManager::Initialize(void)
//   0x5b0240  public: class AGameCrowdAgent * __thiscall UGameCrowdPopulationManager::CreateNewAgent(class UGameCrowdSpawner *, class AActor *, class FVector const *, class AGameCrowdAgent *, class AGameCrowdAgent *)
//   0x5b1890  public: virtual void __thiscall UGameCrowdPopulationManager::Tick(float)

#include "GameFramework.h"

// DISHONORED(retail): GameFramework.GameCrowdPopulationManager, the retail native UObject (GameFrameworkClasses.h); 2013 StaticClassNoInline
// rva 0x55e7c0, InternalConstructor rva 0x56bb60.
IMPLEMENT_CLASS(UGameCrowdPopulationManager);
static_assert(sizeof(UGameCrowdPopulationManager) == 144, "UGameCrowdPopulationManager: retail 2013 size is 144");

// DISHONORED(bringup): not ported yet (2013 rva 0x5620b0, 238 bytes; 2012 rva 0x5a2f40): applies the crowd shadow/quality settings.
// Listed as needed in function_status.csv; nothing reaches it before a crowd exists.
void UGameCrowdPopulationManager::ApplyGameSettings(const ArkSettingsParameters* Parameters, EChangeReason Reason)
{
}
