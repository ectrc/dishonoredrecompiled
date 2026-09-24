// Engine/src/arkppnodedof.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (36):
//   0x549e20  public: virtual unsigned int __thiscall UArkPpNodeDof::LinkInput(unsigned int, class UArkPpNode *)
//   0x549e40  public: virtual unsigned int __thiscall UArkPpNodeDof::UnlinkInput(class UArkPpNode *)
//   0x549e70  public: virtual unsigned int __thiscall UArkPpNodeDof::UnlinkInput(unsigned int)
//   0x54b4d0  public: __thiscall TArkPpDofUberPS<0, 1>::TArkPpDofUberPS<0, 1>(void)
//   0x54b520  public: virtual unsigned int __thiscall TArkPpDofUberPS<1, 0>::Serialize(class FArchive &)
//   0x54ba50  public: virtual unsigned int __thiscall FArkPpDofLutBlenderPS::Serialize(class FArchive &)
//   0x54d9b0  public: virtual __thiscall `anonymous namespace'::FArkDofRamp<16>::~FArkDofRamp<16>(void)
//   0x54da40  public: static class FShader * __cdecl TArkPpDofUberPS<1, 0>::ConstructSerializedInstance(void)
//   0x54ed50  public: virtual class FString __thiscall UArkPpNodeDof::InputName(unsigned int)const
//   0x54ee70  public: static class FShader * __cdecl FArkPpDofLutBlenderPS::ConstructSerializedInstance(void)
//   0x54ef00  public: __thiscall FArkPpNodeDofProxy::FArkPpNodeDofProxy(struct FArkPpCreateProxyConfig &, class UArkPpNodeDof *)
//   0x54f140  public: virtual __thiscall FArkPpNodeDofProxy::~FArkPpNodeDofProxy(void)
//   0x54f1f0  public: void __thiscall FArkPpDofDownsampleVS::SetParameters(struct FArkPpDofDownsampleParameters const &)
//   0x54f290  public: void __thiscall FArkPpDofUberVS::SetParameters(struct FArkPpDofUberParameters const &)
//   0x54f440  public: void __thiscall FArkPpDofLutBlenderPS::SetParameters(struct FArkUberPpParameters const &, struct FLinearColor const &)
//   0x555220  public: virtual void __thiscall `anonymous namespace'::FArkDofRamp<16>::InitDynamicRHI(void)
//   0x555350  public: virtual void __thiscall `anonymous namespace'::FArkDofRamp<16>::ReleaseDynamicRHI(void)
//   0x5553a0  public: void __thiscall TArkPpDofUberPS<0, 1>::SetParameters(struct FArkPpDofUberParameters const &)
//   0x55d740  public: static class UClass * __cdecl UArkPpNodeDof::GetPrivateStaticClassUArkPpNodeDof(wchar_t const *)
//   0x55f3b0  public: static void __cdecl UArkPpNodeDof::InitializePrivateStaticClassUArkPpNodeDof(void)
//   0x563020  public: static class UClass * __cdecl UArkPpNodeDof::StaticClassNoInline(void)
//   0x563050  public: unsigned int __thiscall FArkPpNodeDofProxy::Downsample(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x563560  public: unsigned int __thiscall FArkPpNodeDofProxy::LutCreation(struct FLinearColor const &)
//   0x5637a0  public: unsigned int __thiscall FArkPpNodeDofProxy::Blend(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x563f40  public: virtual unsigned int __thiscall FArkPpNodeDofProxy::Render(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x5654b0  public: virtual unsigned int __thiscall UArkPpNodeDof::IsValid(struct FArkPpIsValidData &)
//   0x565540  public: virtual class FArkPpNodeProxy * __thiscall UArkPpNodeDof::CreateSceneProxy(struct FArkPpCreateProxyConfig &)
//   0xb9f5b0  _anonymous_namespace_::_dynamic_initializer_for__GDofRamp__
//   0xb9f5d0  _dynamic_initializer_for__FArkPpDofDownsamplePS::StaticType__
//   0xb9f610  _dynamic_initializer_for__FArkPpDofDownsampleVS::StaticType__
//   0xb9f650  _dynamic_initializer_for__TArkPpDofUberPS_1_1_::StaticType__
//   0xb9f690  _dynamic_initializer_for__TArkPpDofUberPS_1_0_::StaticType__
//   0xb9f6d0  _dynamic_initializer_for__TArkPpDofUberPS_0_1_::StaticType__
//   0xb9f710  _dynamic_initializer_for__FArkPpDofUberVS::StaticType__
//   0xb9f750  _dynamic_initializer_for__FArkPpDofLutBlenderPS::StaticType__
//   0xb9f790  _dynamic_initializer_for__FArkPpDofLutBlenderVS::StaticType__
