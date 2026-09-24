// Engine/src/arkcomponentlookat.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (51):
//   0x58df30  public: virtual unsigned long __thiscall FArkComponentLookat::GetMemoryFootprint(void)const
//   0x58df40  public: void __thiscall FArkCpntFaceToProp::OverwriteExactRotation(unsigned int)
//   0x58df50  public: class FVector __thiscall FArkComponentLookat::GetLookEyesPosition(class FVector *, class FVector *, class FVector *)
//   0x58e320  public: class FVector const & __thiscall FArkComponentLookat::GetHeadAt(void)const
//   0x58e330  public: class FVector const __thiscall FArkComponentLookat::GetCurrentLookAtDirection(void)const
//   0x58e480  public: class FVector __thiscall FArkComponentLookat::GetLookAtNeutralDirection(void)
//   0x58ec40  public: void __thiscall FArkComponentLookat::DynamicInterpolator<float>::Update(float, float const *, float const *)
//   0x58ee90  public: void __thiscall FArkComponentLookat::DynamicInterpolator<struct FVector2D>::Update(float, float const *, struct FVector2D const *)
//   0x58f230  private: void __thiscall FArkComponentLookat::LookAt(class AActor const *, class FLookAtInfluence const &, float, class FName const &)
//   0x58f2e0  private: void __thiscall FArkComponentLookat::LookAt(class FVector const &, class FLookAtInfluence const &, float, unsigned int)
//   0x58f390  private: void __thiscall FArkComponentLookat::AimAt(class AActor const *)
//   0x58f440  private: void __thiscall FArkComponentLookat::AimAt(class FVector const &)
//   0x58f4f0  private: void __thiscall FArkComponentLookat::LookForward(void)
//   0x58f570  public: void __thiscall FArkComponentLookat::SetLookAtMode(int)
//   0x58f5b0  public: void __thiscall FArkComponentLookat::SetDebugLookAtMode(int)
//   0x58f600  private: void __thiscall FArkComponentLookat::UpdateEyes(float)
//   0x58fab0  public: void __thiscall FArkComponentLookat::OnLODChanged(class FArkGameEvent const &)
//   0x593200  public: virtual unsigned long __thiscall FArkComponentLookat::GetAllocatedSize(void)const
//   0x593220  public: void __thiscall FArkComponentLookat::OnComposeSkeleton(void)
//   0x593550  private: void __thiscall FArkComponentLookat::ProceduralLookAt(float, struct FArkComponentLookat::FLookatRequestData const &)
//   0x5937b0  public: static class FVector __cdecl FArkComponentLookat::GetLookAtPosition(class AActor const *, class FName const &)
//   0x593ac0  private: void __thiscall FArkComponentLookat::UpdateBlink(float)
//   0x593c50  private: void __thiscall FArkComponentLookat::UpdateHeadAndTorso(float)
//   0x594a70  private: void __thiscall FArkComponentLookat::UpdateVariablesFromRequest(float)
//   0x595520  public: static class FArkComponentLookat * __cdecl FArkComponentLookat::GetLookatComponent(class APawn const *)
//   0x595540  public: int __thiscall FArkComponentLookat::StartLookForward(void const * const, class FName const &, int, float, float)
//   0x5955f0  public: int __thiscall FArkComponentLookat::StartLookAtActor(void const * const, class FName const &, int, class AActor const * const, class FLookAtInfluence const &, class FName const &, float, float)
//   0x5956a0  public: int __thiscall FArkComponentLookat::StartLookAtLocation(void const * const, class FName const &, int, class FVector, class FLookAtInfluence const &, float, unsigned int, float)
//   0x595740  public: int __thiscall FArkComponentLookat::StartAimAtActor(void const * const, class FName const &, int, class AActor const * const, class FLookAtInfluence const &, float)
//   0x5957d0  public: int __thiscall FArkComponentLookat::StartAimAtLocation(void const * const, class FName const &, int, class FVector, class FLookAtInfluence const &, float)
//   0x595860  public: int __thiscall FArkComponentLookat::StartProceduralLookAt(void const * const, class FName const &, int, int, class FLookAtInfluence const &, float)
//   0x595900  public: int __thiscall FArkComponentLookat::StartProceduralLookAtOnActor(void const * const, class FName const &, int, int, class AActor const * const, class FLookAtInfluence const &, class FName const &, float)
//   0x5959b0  public: int __thiscall FArkComponentLookat::StartProceduralLookAtOnLocation(void const * const, class FName const &, int, int, class FVector const &, class FLookAtInfluence const &, float)
//   0x595a50  public: struct FArkComponentLookat::FLookatRequestData const * __thiscall FArkComponentLookat::GetActiveRequest(void)const
//   0x595a90  public: int __thiscall FArkComponentLookat::GetActiveRequestPriority(void)const
//   0x595ad0  public: float __thiscall FArkComponentLookat::GetTimeLeft(int)const
//   0x5962d0  public: unsigned int __thiscall FArkComponentLookat::StopLookAt(int)
//   0x596310  public: unsigned int __thiscall FArkComponentLookat::StopAllLookAtFromAsker(void const * const)
//   0x596320  public: virtual void __thiscall FArkComponentLookat::ManageReferences(class FArkComponentBase::FGCHelper &)
//   0x596750  public: virtual void __thiscall FArkComponentLookat::PreAsyncWorkTick(float)
//   0x5968f0  public: __thiscall FArkComponentLookat::FArkComponentLookat(void)
//   0x596ee0  public: virtual void __thiscall FArkComponentLookat::Starting(void)
//   0x597260  public: virtual void __thiscall FArkComponentLookat::Stopping(void)
//   0xb9fea0  _dynamic_initializer_for__FArkComponentLookat::s_CameraBoneName__
//   0xb9fec0  _dynamic_initializer_for__FArkComponentLookat::s_HeadBoneName__
//   0xb9fee0  _dynamic_initializer_for__FArkComponentLookat::s_LeftEyeBoneName__
//   0xb9ff00  _dynamic_initializer_for__FArkComponentLookat::s_RightEyeBoneName__
//   0xb9ff20  _dynamic_initializer_for__FArkComponentLookat::s_LeftEyeControlName__
//   0xb9ff40  _dynamic_initializer_for__FArkComponentLookat::s_RightEyeControlName__
//   0xb9ff60  _dynamic_initializer_for__FArkComponentLookat::s_BlinkControlName__
//   0xb9ff80  _dynamic_initializer_for__FArkComponentLookat::s_BlinkAnimName__
