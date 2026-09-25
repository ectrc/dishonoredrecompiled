// DishonoredGame/src/disskelcontrol_footplacement.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x6d01a0  public: static void __cdecl UDisSkelControl_FootPlacement::InitializePrivateStaticClassUDisSkelControl_FootPlacement(void)
//   0x6d01c0  public: void __thiscall UDisSkelControl_FootPlacement::EnableFootPlacement(unsigned int, unsigned int)
//   0x6d0230  public: void __thiscall UDisSkelControl_FootPlacement::LockFootPlacement(unsigned int)
//   0x6d4b90  protected: void __thiscall UDisSkelControl_FootPlacement::CalcFootPredictVector(class FVector &, class FMatrix const &, class FVector const &, float, float)const
//   0x6d4d00  protected: void __thiscall UDisSkelControl_FootPlacement::DoEffectorBlending(float, class FVector const &, unsigned int)
//   0x6d4fc0  protected: void __thiscall UDisSkelControl_FootPlacement::DoFootBlending(float, class FQuat const &, unsigned int, class FBoneAtom &)
//   0x6d50c0  private: void __thiscall UDisSkelControl_FootPlacement::ResetGroundTestArray(void)
//   0x6d53e0  private: void __thiscall UDisSkelControl_FootPlacement::UpdateGroundMove(class APawn const * const, unsigned int)
//   0x6d54b0  private: unsigned int __thiscall UDisSkelControl_FootPlacement::IsFootOnGround(class FVector const &, class FVector const &, float, unsigned int &)const
//   0x6d5650  private: class FVector __thiscall UDisSkelControl_FootPlacement::TransformRootFootDeltaInFootLocInWorld(class FVector const &, unsigned int)const
//   0x6d5790  private: enum EGroundShape __thiscall UDisSkelControl_FootPlacement::ComputeGroundShape(void)const
//   0x6d5840  private: float __thiscall UDisSkelControl_FootPlacement::EstimateGroundHeight(struct FGroundTestResults const * const, class FVector const &)const
//   0x6d58c0  private: unsigned int __thiscall UDisSkelControl_FootPlacement::UpdateLockProperties(class FVector const &, unsigned int &)
//   0x6d5ba0  private: void __thiscall UDisSkelControl_FootPlacement::HandleFootRotationLock(unsigned int, class FMatrix const &, class FBoneAtom &)
//   0x6d9980  public: void __thiscall UDisSkelControl_FootPlacement::GetFootInfo(class FVector &, class FVector &, class FVector *)
//   0x6d9bc0  protected: unsigned int __thiscall UDisSkelControl_FootPlacement::UpdateFootOrientationTarget(class FBoneAtom const &, class FVector const &, class FQuat &)const
//   0x6da640  private: unsigned int __thiscall UDisSkelControl_FootPlacement::InitializeFootProperties(void)
//   0x6da960  private: class FVector __thiscall UDisSkelControl_FootPlacement::ComputePredictedFootOnGroundLocFromProperties(struct FAnimNodeLocoFootOnGroundProps &)const
//   0x6daa50  private: void __thiscall UDisSkelControl_FootPlacement::DoLineCheck(class FVector const &, struct FGroundTestResults &)const
//   0x6dad90  private: unsigned int __thiscall UDisSkelControl_FootPlacement::UpdateNavMeshStateOfGroundTest(struct FGroundTestResults &)const
//   0x6e4d70  protected: virtual void __thiscall UDisSkelControl_FootPlacement::CalculateNewBoneTransformsOriginal(class TArray<class FBoneAtom, class FDefaultAllocator> &)
//   0x6e6a00  private: struct FGroundTestResults const * __thiscall UDisSkelControl_FootPlacement::PutLocationOnGround(class FVector const &, unsigned int)
//   0x6e6c10  private: void __thiscall UDisSkelControl_FootPlacement::UpdatePredictionProperties(class FVector const &, unsigned int)
//   0x6e7520  private: void __thiscall UDisSkelControl_FootPlacement::UpdateFootOrientation(unsigned int, class FBoneAtom &)
//   0x6e8ed0  private: void __thiscall UDisSkelControl_FootPlacement::CalculateNewBoneTransformsNewComer(class TArray<class FBoneAtom, class FDefaultAllocator> &)
//   0x6ed860  public: virtual void __thiscall UDisSkelControl_FootPlacement::CalculateNewBoneTransforms(int, class USkeletalMeshComponent *, class TArray<class FBoneAtom, class FDefaultAllocator> &)
//   0x6f38f0  public: static class UClass * __cdecl UDisSkelControl_FootPlacement::GetPrivateStaticClassUDisSkelControl_FootPlacement(wchar_t const *)
//   0x6f4d90  public: static class UClass * __cdecl UDisSkelControl_FootPlacement::StaticClassNoInline(void)
