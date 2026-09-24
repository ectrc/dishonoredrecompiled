// Engine/src/arkanimnodelookat.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (43):
//   0x54a850  public: static void __cdecl UArkAnimNodeLookAt::InitializePrivateStaticClassUArkAnimNodeLookAt(void)
//   0x54a870  public: void __thiscall FDynInterpFVector2D::Init(struct FVector2D const &, struct FVector2D const &, struct FVector2D const &, float, float, float)
//   0x54a920  public: void __thiscall UArkAnimNodeLookAt::SetConfig(class UArkComponentLookatConfig const *)
//   0x54a940  public: void __thiscall UArkAnimNodeLookAt::SetAim(struct FVector2D const &)
//   0x54a980  public: float __thiscall UArkAnimNodeLookAt::GetUseHeadRange(void)const
//   0x54a990  public: float __thiscall UArkAnimNodeLookAt::GetUseTorsoRange(void)const
//   0x54a9a0  public: void __thiscall UArkAnimNodeLookAt::SetUseTorsoDeadZone(float)
//   0x54a9d0  public: void __thiscall BlendInfos::UpdateRanges(struct FVector2D const * const)
//   0x54ab40  private: void __thiscall BlendInfos::staticInit(void)
//   0x54ac00  private: static unsigned int __cdecl BlendInfos::GetP1P2(struct FVector2D const * const, int &, int &, struct FVector2D const &)
//   0x54acc0  private: static unsigned int __cdecl BlendInfos::IsOutside(struct FVector2D const &, struct FVector2D const * const, int, int)
//   0x54adb0  private: virtual void __thiscall UArkAnimNodeLookAt::Render(class FSceneView const *, class FPrimitiveDrawInterface *)
//   0x54afa0  public: struct FVector2D const & __thiscall UArkAnimNodeLookAt::GetCurrentBlendedAim(void)const
//   0x54afb0  private: void __thiscall UArkAnimNodeLookAt::AutoInitConfigInAnimTreeEd(void)
//   0x54b080  public: virtual enum ESliderType __thiscall UArkAnimNodeLookAt::GetSliderType(int)const
//   0x54b0b0  public: virtual void __thiscall UArkAnimNodeLookAt::BeginDestroy(void)
//   0x54c800  public: void __thiscall FDynInterpFVector2D::Update(float)
//   0x54cb90  public: static class UArkAnimNodeLookAt * __cdecl UArkAnimNodeLookAt::GetAnimNodeLookAt(class APawn const * const)
//   0x54cbc0  public: void __thiscall UArkAnimNodeLookAt::SetLookAtMode(int)
//   0x54cbf0  public: void __thiscall UArkAnimNodeLookAt::SetUseHeadRange(float)
//   0x54cc30  public: void __thiscall UArkAnimNodeLookAt::SetUseTorsoRange(float)
//   0x54cc70  public: void __thiscall BlendInfos::UpdateDirection(struct FVector2D const &)
//   0x54cd60  public: struct FVector2D __thiscall UArkAnimNodeLookAt::GetMaxAim(struct FVector2D const &)
//   0x54ce00  private: unsigned int __thiscall UArkAnimNodeLookAt::HandleBlendingBetweenLookatNodes(float)
//   0x54d190  public: virtual float __thiscall UArkAnimNodeLookAt::GetSliderPosition(int, int)
//   0x54d2c0  public: virtual void __thiscall UArkAnimNodeLookAt::HandleSliderMove(int, int, float)
//   0x54d430  public: virtual unsigned short __thiscall UArkAnimNodeLookAt::BuildEdgeAnimTree(struct UAnimNode::FEdgeAnimTreeContext &)
//   0x5522b0  private: static struct FVector2D __cdecl BlendInfos::MapIntoNormalizedSpace(struct FVector2D const &, struct FVector2D const * const, struct FVector2D const * const)
//   0x552c60  public: virtual void __thiscall UArkAnimNodeLookAt::CheckAnimsUpToDate(void)
//   0x5532e0  private: void __thiscall UArkAnimNodeLookAt::UpdateNodeWeights(struct FVector2D const * const)
//   0x553fc0  public: float __thiscall UArkAnimNodeLookAt::GetAnimInfoTotalWeight(int)
//   0x557a50  public: struct FVector2D __thiscall BlendInfos::Map(struct FVector2D &)const
//   0x557b80  public: void __thiscall UArkAnimNodeLookAt::ComputeHeadAndTorsoAim(struct FVector2D * const)
//   0x557fe0  public: virtual class FString __thiscall UArkAnimNodeLookAt::GetSliderDrawValue(int)
//   0x55bab0  public: virtual void __thiscall UArkAnimNodeLookAt::GetBoneAtoms(int, class TArray<class FBoneAtom, class TMemStackAllocator<class FMemStack GMainThreadMemStack, 8>> &, class TArray<unsigned char, class FDefaultAllocator> const &, class FBoneAtom &, int &)
//   0x55c310  public: virtual void __thiscall UArkAnimNodeLookAt::GetBoneAtoms(class TArray<class FBoneAtom, class TMemStackAllocator<class FMemStack GMainThreadMemStack, 8>> &, class TArray<unsigned char, class FDefaultAllocator> const &, class FBoneAtom &, int &)
//   0x564c20  public: __thiscall UArkAnimNodeLookAt::UArkAnimNodeLookAt(void)
//   0x565c60  public: static class UClass * __cdecl UArkAnimNodeLookAt::GetPrivateStaticClassUArkAnimNodeLookAt(wchar_t const *)
//   0x566f80  public: static class UClass * __cdecl UArkAnimNodeLookAt::StaticClassNoInline(void)
//   0x566fb0  private: void __thiscall UArkAnimNodeLookAt::HandleProceduralLookatInAnimTreeEd(float)
//   0x5672c0  public: virtual void __thiscall UArkAnimNodeLookAt::TickAnim(float)
//   0x567330  public: virtual void __thiscall UArkAnimNodeLookAt::InitAnim(class USkeletalMeshComponent *, class UAnimNodeBlendBase *)
//   0xb9f9d0  _dynamic_initializer_for__UArkAnimNodeLookAt::s_AnimNodeLookAtName__
