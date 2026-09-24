// Engine/src/arkcomponentlocomotiondynamic.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (42):
//   0x57f090  public: float __thiscall FArkComponentLocomotion::GetRealSpeed(void)const
//   0x57f0a0  public: class FVector const & __thiscall FArkComponentLocomotion::GetPathMove(void)const
//   0x57f0b0  public: class FVector __thiscall FArkComponentLocomotion::GetDirectionToNextPathPoint(void)const
//   0x57f120  public: int __thiscall FArkComponentLocomotion::GetMostRelevantSpeedIdx(void)const
//   0x57f130  public: void __thiscall FArkComponentLocomotion::AddSteeringForce(class FVector const &)
//   0x57f170  public: class FVector const & __thiscall FArkComponentLocomotion::GetSteeringForce(void)const
//   0x57f1b0  public: void __thiscall FArkComponentLocomotion::DisableInertia(void)
//   0x57f1c0  public: void __thiscall FArkComponentLocomotion::EnableInertia(void)
//   0x57f1d0  public: void __thiscall FArkComponentLocomotion::DisableSteering(void)
//   0x57f1f0  public: void __thiscall FArkComponentLocomotion::EnableSteering(void)
//   0x57f200  public: void __thiscall FArkComponentLocomotion::AllowToFall(unsigned int)
//   0x57f220  private: void __thiscall FArkComponentLocomotion::SendTouchAndBumpEvents(struct FCheckResult const * const, unsigned int)
//   0x57f340  private: int __thiscall FArkComponentLocomotion::ComputeMoveYaw(void)const
//   0x57f370  private: float __thiscall FArkComponentLocomotion::UpdateMoveSpeed(float)const
//   0x57f440  private: unsigned int __thiscall FArkComponentLocomotion::HaveAvoidanceAffector(void)const
//   0x57f460  private: unsigned int __thiscall FArkComponentLocomotion::CanBeTeleportedToLocationWithoutBeingSeen(class FVector const &)const
//   0x57ff30  public: void __thiscall FArkComponentLocomotion::OnLODChanged(class FArkGameEvent const &)
//   0x57ff70  public: unsigned int __thiscall FArkComponentLocomotion::IsRootMotion(void)const
//   0x57ffb0  public: struct FCheckResult * __thiscall FArkComponentLocomotion::FindValidLocation(class FMemStack &, class FVector &, unsigned long, class FVector const &, unsigned int &, unsigned int &)const
//   0x5802a0  private: int __thiscall FArkComponentLocomotion::ComputeMostRelevantSpeedIdx(void)const
//   0x580a40  private: void __thiscall FArkComponentLocomotion::UnTouchActors(struct FCheckResult const * const, unsigned int)
//   0x580ae0  private: void __thiscall FArkComponentLocomotion::FullMovePawn(float, unsigned int, unsigned int)
//   0x581d50  private: unsigned int __thiscall FArkComponentLocomotion::ModerateMovePawn(float)
//   0x582300  private: int __thiscall FArkComponentLocomotion::UpdateMoveYaw(float, float &, float &, float &)const
//   0x5829d0  private: unsigned int __thiscall FArkComponentLocomotion::IsThereAnotherForceThanPath(void)const
//   0x582ad0  private: float __thiscall FArkComponentLocomotion::GetFinalSpeedMultiplier(void)const
//   0x584ca0  public: void __thiscall FArkComponentLocomotion::MovePawn(float, unsigned int)
//   0x584d40  private: float __thiscall FArkComponentLocomotion::ComputeLookatSpeedMultiplier(void)const
//   0x584f10  private: unsigned int __thiscall FArkComponentLocomotion::DetectPreAvoidanceCollision(class FVector, class FVector const &, class FVector &)const
//   0x585330  private: unsigned int __thiscall FArkComponentLocomotion::ComputeIsArrivedFlagAndSq2DDistToPathEnd(float &)const
//   0x585430  private: class FVector __thiscall FArkComponentLocomotion::ClampMove(float, class FVector const &, float &)const
//   0x586f10  private: unsigned int __thiscall FArkComponentLocomotion::IsArrived(void)const
//   0x586f40  private: float __thiscall FArkComponentLocomotion::GetSq2DDistToPathEnd(void)const
//   0x586f60  private: void __thiscall FArkComponentLocomotion::UpdateSharedProperties(void)
//   0x588700  private: int __thiscall FArkComponentLocomotion::ComputeTargetMoveYaw(void)const
//   0x588e00  private: float __thiscall FArkComponentLocomotion::ComputeTargetMoveSpeed(float, float &)const
//   0x5891c0  private: class FVector __thiscall FArkComponentLocomotion::ComputePathMove(float)
//   0x589560  private: void __thiscall FArkComponentLocomotion::HandleArrival(float)
//   0x589940  private: float __thiscall FArkComponentLocomotion::HandlePushPeriod(void)
//   0x589e00  private: void __thiscall FArkComponentLocomotion::ComputePriorityOfNPCsFromSharedProps(void)
//   0x58b110  private: class FVector __thiscall FArkComponentLocomotion::ApplySteering(float, class FVector const &)
//   0x58ca10  private: void __thiscall FArkComponentLocomotion::ComputeDynamic(float)
