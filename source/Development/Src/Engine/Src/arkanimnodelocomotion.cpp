// Engine/src/arkanimnodelocomotion.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (52):
//   0x54a2b0  public: static void __cdecl UArkAnimNodeLocomotion::InitializePrivateStaticClassUArkAnimNodeLocomotion(void)
//   0x54a2d0  public: void __thiscall UArkAnimNodeLocomotion::SetAngle(int)
//   0x54a2f0  public: void __thiscall UArkAnimNodeLocomotion::SetSpeed(float, unsigned int)
//   0x54a320  public: float __thiscall UArkAnimNodeLocomotion::GetSpeed(void)const
//   0x54a330  public: void __thiscall UArkAnimNodeLocomotion::SetSpeedMultiplier(float)
//   0x54a350  public: void __thiscall UArkAnimNodeLocomotion::SetModifier(int)
//   0x54a360  public: void __thiscall UArkAnimNodeLocomotion::GetMovementAnimPrevAndCurProgress(float &, float &)const
//   0x54a380  public: unsigned int __thiscall UArkAnimNodeLocomotion::IsSpecificAnimRequested(void)const
//   0x54a390  public: unsigned int __thiscall UArkAnimNodeLocomotion::IsSpecificAnimPlaying(void)const
//   0x54a3a0  public: unsigned int __thiscall UArkAnimNodeLocomotion::IsSpecificAnimRequestedOrPlaying(void)const
//   0x54a3c0  public: enum EAnimNodeLocoSpecificAnimType __thiscall UArkAnimNodeLocomotion::GetTypeOfCurSpecificAnim(void)const
//   0x54a3d0  public: unsigned int __thiscall UArkAnimNodeLocomotion::IsPlayingOnlySpecificAnim(void)const
//   0x54a400  public: void __thiscall UArkAnimNodeLocomotion::AbortSpecificAnimRequest(unsigned int)
//   0x54a450  public: struct FAnimNodeLocoModifierSlot const * __thiscall UArkAnimNodeLocomotion::GetMostRelevantModifierSlot(void)const
//   0x54a480  public: float __thiscall UArkAnimNodeLocomotion::GetOutOfShapeRatio(void)const
//   0x54a490  public: virtual void __thiscall UArkAnimNodeLocomotion::OnCeaseRelevant(void)
//   0x54a4c0  public: float __thiscall UArkAnimNodeLocomotion::GetDeltaTime(void)const
//   0x54a4d0  private: virtual void __thiscall UArkAnimNodeLocomotion::Render(class FSceneView const *, class FPrimitiveDrawInterface *)
//   0x54c370  public: static class UArkAnimNodeLocomotion * __cdecl UArkAnimNodeLocomotion::GetAnimNodeLocomotion(class APawn const * const)
//   0x54c3b0  private: void __thiscall UArkAnimNodeLocomotion::BlendSpeedAndAngleProperties(float)
//   0x54c560  private: void __thiscall UArkAnimNodeLocomotion::UpdateSpecificAnim(float)
//   0x550750  public: float __thiscall UArkAnimNodeLocomotion::GetDurationBeforePlayingTurnAnim(int, int)const
//   0x5508c0  public: void __thiscall UArkAnimNodeLocomotion::RequestTurnAnim(int, int, unsigned int, unsigned int, unsigned int)
//   0x550a30  public: void __thiscall UArkAnimNodeLocomotion::RequestStartAnim(int)
//   0x550b60  public: void __thiscall UArkAnimNodeLocomotion::RequestStopAnim(int, int)
//   0x550ca0  private: void __thiscall UArkAnimNodeLocomotion::UpdateModifierSlot(int, float)
//   0x551060  private: float __thiscall UArkAnimNodeLocomotion::UpdateMovementChild(struct FAnimNodeLocoModifierSlot const &, struct FAnimNodeLocoSpeedSlot const &, struct FAnimNodeLocoAngleSlot const &, int, unsigned int)
//   0x551380  private: void __thiscall UArkAnimNodeLocomotion::ResetModifierSlot(int, unsigned int)
//   0x5513e0  private: float __thiscall UArkAnimNodeLocomotion::FindFastestSpeed(void)const
//   0x551520  private: float __thiscall UArkAnimNodeLocomotion::ComputeBlendingOutOfShapeRatio(void)const
//   0x551920  private: void __thiscall UArkAnimNodeLocomotion::HandleTransition(void)
//   0x551a90  private: void __thiscall UArkAnimNodeLocomotion::UpdateProgressVariables(float)
//   0x551b90  private: void __thiscall UArkAnimNodeLocomotion::UpdateFeetPropeties(void)
//   0x551eb0  private: int __thiscall UArkAnimNodeLocomotion::GetFootIndexFromBoneIndex(int)const
//   0x551fa0  private: float __thiscall UArkAnimNodeLocomotion::GetFinalSpeedMultiplier(void)const
//   0x552000  private: void __thiscall UArkAnimNodeLocomotion::UpdateGridMove(float)
//   0x555e50  public: int __thiscall UArkAnimNodeLocomotion::GetFootProperties(int, struct FArkCpntLocoFootProp &)const
//   0x555ed0  public: unsigned int __thiscall UArkAnimNodeLocomotion::GetNextFootOnGroundProperties(int, class FVector const &, struct FAnimNodeLocoFootOnGroundProps &)const
//   0x556250  public: unsigned int __thiscall UArkAnimNodeLocomotion::GetDefaultLocalRootFootDelta(int, class FVector &)const
//   0x556390  public: virtual void __thiscall UArkAnimNodeLocomotion::OnBecomeRelevant(void)
//   0x556440  public: virtual float __thiscall UArkAnimNodeLocomotion::GetSliderPosition(int, int)
//   0x556640  public: virtual void __thiscall UArkAnimNodeLocomotion::HandleSliderMove(int, int, float)
//   0x5568c0  public: virtual class FString __thiscall UArkAnimNodeLocomotion::GetSliderDrawValue(int)
//   0x556c40  private: void __thiscall UArkAnimNodeLocomotion::UpdateMovementAnims(float)
//   0x556e20  private: void __thiscall UArkAnimNodeLocomotion::UpdateMovementChildren(void)
//   0x557910  private: void __thiscall UArkAnimNodeLocomotion::UpdateSpecificChild(void)
//   0x55b910  public: virtual void __thiscall UArkAnimNodeLocomotion::TickAnim(float)
//   0x55e050  public: void __thiscall UArkAnimNodeLocomotion::Initialize(class UArkComponentLocomotionConfig const * const)
//   0x55eac0  public: static class UClass * __cdecl UArkAnimNodeLocomotion::GetPrivateStaticClassUArkAnimNodeLocomotion(wchar_t const *)
//   0x55fd30  public: static class UClass * __cdecl UArkAnimNodeLocomotion::StaticClassNoInline(void)
//   0xb9f990  _dynamic_initializer_for__UArkAnimNodeLocomotion::s_AnimNodeLocoName__
//   0xb9f9b0  _dynamic_initializer_for__UArkAnimNodeLocomotion::s_AnimNodeLocoGroupName__
