// Engine/src/arkppnodeblur.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x549d00  public: virtual unsigned int __thiscall UArkPpNodeBlur::LinkInput(unsigned int, class UArkPpNode *)
//   0x549d40  public: virtual unsigned int __thiscall UArkPpNodeBlur::UnlinkInput(class UArkPpNode *)
//   0x549d80  public: virtual unsigned int __thiscall UArkPpNodeBlur::UnlinkInput(unsigned int)
//   0x549dd0  public: virtual unsigned int __thiscall UArkPpNodeBlur::NumInputs(void)const
//   0x549de0  public: virtual class UArkPpNode * __thiscall UArkPpNodeBlur::GetInput(unsigned int)
//   0x54b490  public: virtual unsigned int __thiscall TArkPpBlurPixelShader<struct MotionBlur2NoOffsPolicy>::Serialize(class FArchive &)
//   0x54ba10  public: virtual unsigned int __thiscall TArkPpBlurVertexShader<struct BoxBlurPolicy>::Serialize(class FArchive &)
//   0x54ea30  public: virtual class FString __thiscall UArkPpNodeBlur::InputName(unsigned int)const
//   0x54ea90  public: __thiscall FArkPpNodeBlurProxy::FArkPpNodeBlurProxy(struct FArkPpCreateProxyConfig &, class UArkPpNodeBlur *)
//   0x54ebf0  public: virtual __thiscall FArkPpNodeBlurProxy::~FArkPpNodeBlurProxy(void)
//   0x54ec70  public: virtual unsigned int __thiscall FArkPpNodeBlurProxy::Render(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x54edf0  public: static class FShader * __cdecl FArkPpDofUberVS::ConstructSerializedInstance(void)
//   0x554e90  public: void __thiscall TArkPpBlurVertexShader<struct MotionBlurPolicy>::SetParameters(struct FArkPpBlurParameters const &)
//   0x554f70  public: void __thiscall TArkPpBlurPixelShader<struct MotionBlur2NoOffsPolicy>::SetParameters(struct FArkPpBlurParameters const &)
//   0x55d6b0  public: static class UClass * __cdecl UArkPpNodeBlur::GetPrivateStaticClassUArkPpNodeBlur(wchar_t const *)
//   0x55f390  public: static void __cdecl UArkPpNodeBlur::InitializePrivateStaticClassUArkPpNodeBlur(void)
//   0x562690  public: static class UClass * __cdecl UArkPpNodeBlur::StaticClassNoInline(void)
//   0x5626c0  public: virtual unsigned int __thiscall FArkPpNodeBlurProxy::RenderMotionBlur(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x565290  public: virtual unsigned int __thiscall UArkPpNodeBlur::IsValid(struct FArkPpIsValidData &)
//   0x565390  public: virtual class FArkPpNodeProxy * __thiscall UArkPpNodeBlur::CreateSceneProxy(struct FArkPpCreateProxyConfig &)
//   0xb9f3b0  _dynamic_initializer_for__TArkPpBlurVertexShader_BoxBlurPolicy_::StaticType__
//   0xb9f3f0  _dynamic_initializer_for__TArkPpBlurVertexShader_MotionBlurPolicy_::StaticType__
//   0xb9f430  _dynamic_initializer_for__TArkPpBlurVertexShader_RadialBlurPolicy_::StaticType__
//   0xb9f470  _dynamic_initializer_for__TArkPpBlurPixelShader_BoxBlurPolicy_::StaticType__
//   0xb9f4b0  _dynamic_initializer_for__TArkPpBlurPixelShader_MotionBlurPolicy_::StaticType__
//   0xb9f4f0  _dynamic_initializer_for__TArkPpBlurPixelShader_MotionBlur2Policy_::StaticType__
//   0xb9f530  _dynamic_initializer_for__TArkPpBlurPixelShader_MotionBlur2NoOffsPolicy_::StaticType__
//   0xb9f570  _dynamic_initializer_for__TArkPpBlurPixelShader_RadialBlurPolicy_::StaticType__
