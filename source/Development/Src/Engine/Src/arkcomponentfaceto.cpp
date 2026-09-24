// Engine/src/arkcomponentfaceto.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (47):
//   0x574820  public: virtual unsigned long __thiscall FArkComponentFaceTo::GetMemoryFootprint(void)const
//   0x574830  public: unsigned int __thiscall FArkComponentFaceTo::IsInitialized(void)const
//   0x574840  public: int __thiscall FArkComponentFaceTo::GetAngle(void)const
//   0x574850  public: void __thiscall FArkComponentFaceTo::DisableRootMotion(void)
//   0x574860  public: unsigned int __thiscall FArkComponentFaceTo::IsRootMotion(void)const
//   0x574880  public: void __thiscall FArkComponentFaceTo::AllowTurnControl(void)
//   0x574890  public: void __thiscall FArkComponentFaceTo::ForceBodySnapOnMovement(unsigned int)
//   0x5748b0  public: void __thiscall FArkComponentFaceTo::SetRealVelocity(class FVector const &)
//   0x5748d0  public: static void __cdecl FArkComponentFaceTo::SetDebugDontRotate(unsigned int)
//   0x5748e0  public: static unsigned int __cdecl FArkComponentFaceTo::GetDebugDontRotate(void)
//   0x5748f0  public: int __thiscall FFluidVertexBuffer::GetNumQuadsX(void)const
//   0x574900  public: void __thiscall FArkComponentFaceTo::HandleTeleport(void)
//   0x574930  private: unsigned int __thiscall FArkComponentFaceTo::IsRequestHasInfiniteRotationSpeed(struct FArkComponentFaceTo::FFaceToRequestData const &)const
//   0x574960  private: unsigned int __thiscall FArkComponentFaceTo::IsRequestHasDefaultAcceleration(struct FArkComponentFaceTo::FFaceToRequestData const &)const
//   0x574990  private: unsigned int __thiscall FArkComponentFaceTo::CanKeepSnapped(struct FArkComponentFaceTo::FFaceToRequestData const &, float, float)const
//   0x5749f0  public: __thiscall FArkCpntFaceToProp::FArkCpntFaceToProp(void)
//   0x574a00  public: void __thiscall FArkCpntFaceToActorProp::InitAllByDefaultExcept(class FArkComponentFaceTo const * const, class AActor const *, float, class UObject * const, void (__thiscall UObject::*)(enum EArkCpntFaceToEvent))
//   0x574a50  public: void __thiscall FArkCpntFaceToLocationProp::InitAllByDefaultExcept(class FVector const &, float, class UObject * const, void (__thiscall UObject::*)(enum EArkCpntFaceToEvent))
//   0x574aa0  public: void __thiscall FArkCpntFaceToYawProp::InitAllByDefaultExcept(int, float, class UObject * const, void (__thiscall UObject::*)(enum EArkCpntFaceToEvent))
//   0x575400  public: void __thiscall FArkComponentFaceTo::Initialize(class UArkComponentLocomotionConfig const * const)
//   0x575430  public: void __thiscall FArkComponentFaceTo::OnLODChanged(class FArkGameEvent const &)
//   0x575b10  protected: virtual unsigned long __thiscall FArkComponentFaceTo::GetAllocatedSize(void)const
//   0x575b20  public: int __thiscall FArkComponentFaceTo::GetRequestsCount(void)const
//   0x577860  public: static class FArkComponentFaceTo * __cdecl FArkComponentFaceTo::GetFaceToComponent(class APawn const * const)
//   0x577880  public: int __thiscall FArkComponentFaceTo::StartFaceToActor(void const * const, class FName, int, struct FArkCpntFaceToActorProp const &)
//   0x5778f0  public: int __thiscall FArkComponentFaceTo::StartFaceToLocation(void const * const, class FName, int, struct FArkCpntFaceToLocationProp const &)
//   0x577960  public: int __thiscall FArkComponentFaceTo::StartFaceToYaw(void const * const, class FName, int, struct FArkCpntFaceToYawProp const &)
//   0x5779e0  public: int __thiscall FArkComponentFaceTo::GetActiveRequestID(void)const
//   0x577a20  public: int __thiscall FArkComponentFaceTo::GetActiveRequestPriority(void)const
//   0x577a60  private: unsigned int __thiscall FArkComponentFaceTo::UpdateCurYawAndRotationSpeed(float, struct FArkComponentFaceTo::FFaceToRequestData const &, float &, float &)const
//   0x577cf0  private: void __thiscall FArkComponentFaceTo::ResetCurRequestWorkData(unsigned int)
//   0x577d70  private: int __thiscall FArkComponentFaceTo::FindTargetYaw(void)const
//   0x577ef0  private: void __thiscall FArkComponentFaceTo::UpdateFaceToRequest(int, struct FArkComponentFaceTo::FFaceToRequestData const &)
//   0x577f40  private: unsigned int __thiscall FArkComponentFaceTo::SendFaceToEvent(int, enum EArkCpntFaceToEvent)
//   0x5795d0  public: __thiscall FArkComponentFaceTo::FArkComponentFaceTo(void)
//   0x5797b0  public: unsigned int __thiscall FArkComponentFaceTo::UpdateFaceToActor(int, struct FArkCpntFaceToActorProp const &)
//   0x579820  public: unsigned int __thiscall FArkComponentFaceTo::UpdateFaceToLocation(int, struct FArkCpntFaceToLocationProp const &)
//   0x5798a0  public: unsigned int __thiscall FArkComponentFaceTo::UpdateFaceToYaw(int, struct FArkCpntFaceToYawProp const &)
//   0x579920  public: unsigned int __thiscall FArkComponentFaceTo::StopFaceTo(int)
//   0x579960  public: unsigned int __thiscall FArkComponentFaceTo::StopAllFaceToFromAsker(void const * const)
//   0x579970  public: void __thiscall FArkComponentFaceTo::AllowRootMotion(unsigned int)
//   0x5799a0  public: void __thiscall FArkComponentFaceTo::DisableTurnControl(void)
//   0x5799c0  private: void __thiscall FArkComponentFaceTo::HandleOrientationReachedEvent(void)
//   0x579b60  private: void __thiscall FArkComponentFaceTo::OnRequestManagerEvent(enum FArkRequestManager<struct FArkComponentFaceTo::FFaceToRequestData>::EArkReqMgrEvent, int)
//   0x57ab20  public: virtual void __thiscall FArkComponentFaceTo::PreAsyncWorkTick(float)
//   0x57dc00  public: virtual void __thiscall FArkComponentFaceTo::Starting(void)
//   0x57dcc0  public: virtual void __thiscall FArkComponentFaceTo::Stopping(void)
