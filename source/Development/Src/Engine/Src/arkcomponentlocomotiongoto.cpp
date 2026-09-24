// Engine/src/arkcomponentlocomotiongoto.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (31):
//   0x57f770  public: void __thiscall FArkComponentLocomotion::ForceActiveRequestRepath(void)
//   0x57f780  public: void __thiscall FArkComponentLocomotion::Enable(void)
//   0x57f7a0  public: unsigned int __thiscall FArkComponentLocomotion::IsDisabled(void)const
//   0x57f7c0  public: void __thiscall FArkComponentLocomotion::EnableTeleportWhenOutsideOfNavmesh(unsigned int)
//   0x57f800  public: int __thiscall FArkComponentLocomotion::GetModifier(void)const
//   0x57f810  public: __thiscall FArkCpntLocoGoToProp::FArkCpntLocoGoToProp(void)
//   0x57f820  public: void __thiscall FArkCpntLocoGoToProp::OverwriteMaxFunnelRadiusMultiplier(float)
//   0x57f840  public: void __thiscall FArkCpntLocoGoToProp::OverwriteStopBeforeEndDist(float)
//   0x57f860  public: void __thiscall FArkCpntLocoGoToProp::OverwriteSpeedIsLookAtDependent(unsigned int)
//   0x57f870  public: void __thiscall FArkCpntLocoGoToActorProp::OverwriteFollowProperties(unsigned int, float, float)
//   0x580370  public: void __thiscall FArkCpntLocoGoToProp::OverwriteEndLocationThreshold(float)
//   0x5803a0  public: void __thiscall FArkCpntLocoGoToProp::OverwriteSpeedMultiplier(float)
//   0x5803d0  public: void __thiscall FArkCpntLocoGoToActorProp::InitAllByDefaultExcept(class FArkComponentLocomotion const * const, class AActor const *, class UObject * const, void (__thiscall UObject::*)(int, enum EArkCpntLocoEvent))
//   0x582b30  public: int __thiscall FArkComponentLocomotion::GetRequestsCount(void)const
//   0x582b40  public: void __thiscall FArkComponentLocomotion::SetModifier(int)
//   0x582e00  public: void __thiscall FArkCpntLocoGoToLocationProp::InitAllByDefaultExcept(class FArkComponentLocomotion const * const, class FVector const &, class UObject * const, void (__thiscall UObject::*)(int, enum EArkCpntLocoEvent))
//   0x585f20  public: int __thiscall FArkComponentLocomotion::StartLocoToActor(void const * const, class FName, int, struct FArkCpntLocoGoToActorProp const &)
//   0x585fd0  public: int __thiscall FArkComponentLocomotion::StartLocoToLocation(void const * const, class FName, int, struct FArkCpntLocoGoToLocationProp const &)
//   0x586030  public: int __thiscall FArkComponentLocomotion::GetActiveRequestMaxSpeedIdx(void)const
//   0x586070  private: void __thiscall FArkComponentLocomotion::UpdateLocoRequest(int, struct FArkComponentLocomotion::FLocoRequestData const &)
//   0x5860c0  private: void __thiscall FArkComponentLocomotion::ResetCurRequestWorkData(void)
//   0x5860f0  private: class FVector __thiscall FArkComponentLocomotion::GetAskedTargetLocOfActiveRequest(void)const
//   0x586180  private: class FVector __thiscall FArkComponentLocomotion::ComputeTargetedLocationForPath(float, unsigned int)
//   0x587050  public: unsigned int __thiscall FArkComponentLocomotion::UpdateLocoToActor(int, struct FArkCpntLocoGoToActorProp const &)
//   0x587110  public: unsigned int __thiscall FArkComponentLocomotion::UpdateLocoToLocation(int, struct FArkCpntLocoGoToLocationProp const &)
//   0x587170  public: unsigned int __thiscall FArkComponentLocomotion::StopLoco(int)
//   0x5871b0  public: unsigned int __thiscall FArkComponentLocomotion::StopAllLocoFromAsker(class UObject const * const)
//   0x5871c0  public: class FVector __thiscall FArkComponentLocomotion::GetActiveRequestTargetedLocation(void)const
//   0x587210  public: void __thiscall FArkComponentLocomotion::Disable(void)
//   0x587240  public: void __thiscall FArkComponentLocomotion::HandleTeleport(void)
//   0x587350  private: void __thiscall FArkComponentLocomotion::OnRequestManagerEvent(enum FArkRequestManager<struct FArkComponentLocomotion::FLocoRequestData>::EArkReqMgrEvent, int)
