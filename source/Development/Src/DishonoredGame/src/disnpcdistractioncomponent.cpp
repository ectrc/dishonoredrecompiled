// DishonoredGame/src/disnpcdistractioncomponent.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (46):
//   0x7c96e0  public: static void __cdecl UDisNPCDistractionComponent::InitializePrivateStaticClassUDisNPCDistractionComponent(void)
//   0x7c9700  public: unsigned int __thiscall UDisNPCDistractionComponent::IsDistractionEnabled(void)const
//   0x7c9710  FindExistingOrEmptySlot
//   0x7c9760  public: virtual void __thiscall UDisNPCDistractionComponent::Attach(void)
//   0x7c9780  public: class FRotator __thiscall UDisNPCDistractionComponent::GetDistractionRot(void)const
//   0x7c97a0  public: float __thiscall UDisNPCDistractionComponent::GetDistractionRange(void)const
//   0x7c97b0  public: unsigned int __thiscall UDisNPCDistractionComponent::AvailableToDistract(unsigned int)const
//   0x7c9800  private: void __thiscall UDisNPCDistractionComponent::AddBrainToCheck(class UDishonoredAIBrain *)
//   0x7c9850  public: void __thiscall UDisNPCDistractionComponent::AddBrain(class UDishonoredAIBrain *)
//   0x7c9890  public: struct FDistractionChances const * __thiscall UDisNPCDistractionComponent::GetDistractionChances(class ADishonoredNPCPawn *)const
//   0x7c9900  public: virtual class AActor const * __thiscall UDisNPCDistractionComponent::GetOwningActor(void)const
//   0x7c9910  public: static class FName __cdecl UDisNPCDistractionComponent::GetActorSpecificDistractionCategory(void)
//   0x7cc150  public: virtual void __thiscall UDisNPCDistractionComponent::GameSave(class FArchive &, enum ESaveLoadLocation)
//   0x7cc1a0  public: virtual void __thiscall UDisNPCDistractionComponent::BeginPlay(void)
//   0x7cc1e0  public: virtual void __thiscall UDisNPCDistractionComponent::GoTo_Begun(class ADishonoredNPCPawn *)
//   0x7cc230  public: class FVector __thiscall UDisNPCDistractionComponent::GetDistractionPos(void)const
//   0x7cc260  public: __thiscall FDistractionComponentSceneProxy::FDistractionComponentSceneProxy(class UDisNPCDistractionComponent * const)
//   0x7cc310  public: virtual void __thiscall FDistractionComponentSceneProxy::DrawDynamicElements(class FPrimitiveDrawInterface *, class FSceneView const *, unsigned int, unsigned long)
//   0x7cc400  public: virtual class FPrimitiveSceneProxy * __thiscall UDisNPCDistractionComponent::CreateSceneProxy(void)
//   0x7cc470  public: virtual class FVector __thiscall UDisNPCDistractionComponent::GetLookAtPoint(void)const
//   0x7ce630  PawnTypeInList
//   0x7ce6d0  private: unsigned int __thiscall UDisNPCDistractionComponent::PawnTypeAllowed(class ADishonoredNPCPawn *)const
//   0x7ce730  private: unsigned int __thiscall UDisNPCDistractionComponent::BrainIsOnAllowedRoute(class UDishonoredAIBrain *)const
//   0x7da0b0  public: void __thiscall UDisNPCDistractionComponent::LinkGoToPawn(class ADishonoredNPCPawn *)
//   0x7da160  public: void __thiscall UDisNPCDistractionComponent::RemoveBrain(class UDishonoredAIBrain *)
//   0x7da240  private: unsigned int __thiscall UDisNPCDistractionComponent::BrainHasLOS(class UDishonoredAIBrain *)
//   0x7dde90  public: void __thiscall UDisNPCDistractionComponent::SetDistractionEnabled(unsigned int)
//   0x7ddec0  public: virtual void __thiscall UDisNPCDistractionComponent::GoTo_BehaviorStart(class ADishonoredNPCPawn *)
//   0x7dded0  public: virtual void __thiscall UDisNPCDistractionComponent::GoTo_BehaviorStop(class ADishonoredNPCPawn *)
//   0x7ddee0  private: void __thiscall UDisNPCDistractionComponent::CheckDistract(class UDishonoredAIBrain *)
//   0x7df070  public: static class UClass * __cdecl UDisNPCDistractionComponent::GetPrivateStaticClassUDisNPCDistractionComponent(wchar_t const *)
//   0x7df100  private: void __thiscall UDisNPCDistractionComponent::Tick_CheckDistract(float)
//   0x7e0970  public: static class UClass * __cdecl UDisNPCDistractionComponent::StaticClassNoInline(void)
//   0x7efc40  private: void __thiscall UDisNPCDistractionComponent::TriggerDistractedEvent(class ADishonoredNPCPawn *, int)
//   0x7efdc0  private: void __thiscall UDisNPCDistractionComponent::SoireeDistractionSetLoop(void)
//   0x7f0ac0  public: void __thiscall UDisNPCDistractionComponent::OnAnimDistract(class ADishonoredNPCPawn *)
//   0x7f0b80  protected: void __thiscall UDisNPCDistractionComponent::OnPawnSoireeUpdate(class FArkGameEvent const &)
//   0x7f0e50  public: virtual void __thiscall UDisNPCDistractionComponent::BeginDestroy(void)
//   0x7f0ea0  public: void __thiscall UDisNPCDistractionComponent::OnSoireeDistract(class ADishonoredNPCPawn *)
//   0x7f1060  public: virtual void __thiscall UDisNPCDistractionComponent::GoTo_Completed(class ADishonoredNPCPawn *)
//   0x7f1aa0  public: void __thiscall UDisNPCDistractionComponent::OnLookAtDistract(class ADishonoredNPCPawn *)
//   0x7f1b20  public: void __thiscall UDisNPCDistractionComponent::StopAnimDistract(void)
//   0x7f1c50  private: void __thiscall UDisNPCDistractionComponent::PlayDistractionAnim(void)
//   0x7f2270  public: virtual void __thiscall UDisNPCDistractionComponent::Detach(unsigned int)
//   0x7f22a0  private: void __thiscall UDisNPCDistractionComponent::UpdateAnimDistract(void)
//   0x7f23f0  public: virtual void __thiscall UDisNPCDistractionComponent::Tick(float)
