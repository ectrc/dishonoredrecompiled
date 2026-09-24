// Engine/src/arkppnodeaa.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (58):
//   0x549c90  public: virtual unsigned int __thiscall UArkPpNodeAA::LinkInput(unsigned int, class UArkPpNode *)
//   0x549cb0  public: virtual unsigned int __thiscall UArkPpNodeAA::UnlinkInput(class UArkPpNode *)
//   0x549ce0  public: virtual unsigned int __thiscall UArkPpNodeAA::UnlinkInput(unsigned int)
//   0x54b270  public: __thiscall TFXAAPixelShader<0, 1, 1>::TFXAAPixelShader<0, 1, 1>(void)
//   0x54b2e0  public: virtual unsigned int __thiscall TFXAAPixelShader<2, 1, 0>::Serialize(class FArchive &)
//   0x54b390  public: virtual unsigned int __thiscall TMLAAEdgeDetectionPixelShader<0>::Serialize(class FArchive &)
//   0x54b3e0  public: virtual unsigned int __thiscall TMLAABlendPixelShader<0>::Serialize(class FArchive &)
//   0x54b440  public: virtual unsigned int __thiscall TMLAABlendPixelShader<1>::Serialize(class FArchive &)
//   0x54b800  public: __thiscall FArkPpNodeAAProxy::FArkPpNodeAAProxy(struct FArkPpCreateProxyConfig &, class UArkPpNodeAA *)
//   0x54b8d0  public: virtual class TDynamicRHIResourceReference<12> const __thiscall FArkPpNodeAAProxy::GetSurface(class FViewInfo const &)
//   0x54b900  public: virtual class TDynamicRHIResourceReference<14> const __thiscall FArkPpNodeAAProxy::GetTexture(class FViewInfo const &)
//   0x54b930  public: virtual unsigned int __thiscall FArkPpNodeAAProxy::GetSurfaceSizeX(void)
//   0x54b940  public: virtual unsigned int __thiscall FArkPpNodeAAProxy::GetSurfaceSizeY(void)
//   0x54b950  public: virtual __thiscall FArkPpNodeAAProxy::~FArkPpNodeAAProxy(void)
//   0x54d720  public: static class FShader * __cdecl TFXAAPixelShader<1, 1, 0>::ConstructSerializedInstance(void)
//   0x54d790  public: static class FShader * __cdecl TMLAAEdgeDetectionPixelShader<0>::ConstructSerializedInstance(void)
//   0x54d810  public: static class FShader * __cdecl TMLAAEdgeDetectionPixelShader<1>::ConstructSerializedInstance(void)
//   0x54d890  public: static class FShader * __cdecl TMLAABlendPixelShader<0>::ConstructSerializedInstance(void)
//   0x54d920  public: static class FShader * __cdecl TMLAABlendPixelShader<1>::ConstructSerializedInstance(void)
//   0x54e7b0  public: void __thiscall FFXAAVertexShader::SetParameters(struct FArkPpFxAaParameters const &)
//   0x54e900  public: void __thiscall FMLAAVertexShader::SetParameters(struct FArkPpMlaaParameters const &)
//   0x54e9e0  public: virtual class FString __thiscall UArkPpNodeAA::InputName(unsigned int)const
//   0x555760  public: void __thiscall TMLAAEdgeDetectionPixelShader<0>::SetParameters(struct FArkPpMlaaParameters &)
//   0x5558b0  public: void __thiscall TMLAAEdgeDetectionPixelShader<1>::SetParameters(struct FArkPpMlaaParameters &)
//   0x5587a0  public: void __thiscall TFXAAPixelShader<0, 1, 0>::SetParameters(struct FArkPpFxAaParameters const &)
//   0x558ba0  public: void __thiscall TMLAABlendPixelShader<0>::SetParameters(struct FArkPpMlaaParameters const &)
//   0x558dc0  public: void __thiscall TMLAABlendPixelShader<1>::SetParameters(struct FArkPpMlaaParameters const &)
//   0x55d620  public: static class UClass * __cdecl UArkPpNodeAA::GetPrivateStaticClassUArkPpNodeAA(wchar_t const *)
//   0x55f370  public: static void __cdecl UArkPpNodeAA::InitializePrivateStaticClassUArkPpNodeAA(void)
//   0x560170  public: void __thiscall FArkPpNodeAAProxy::FlushShader<class TFXAAPixelShader<0, 1, 0>>(class TShaderMapRef<class FFXAAVertexShader> &, struct FArkPpFxAaParameters &)
//   0x560250  public: void __thiscall FArkPpNodeAAProxy::FlushShader<class TFXAAPixelShader<2, 1, 0>>(class TShaderMapRef<class FFXAAVertexShader> &, struct FArkPpFxAaParameters &)
//   0x560330  public: void __thiscall FArkPpNodeAAProxy::FlushShader<class TFXAAPixelShader<1, 1, 0>>(class TShaderMapRef<class FFXAAVertexShader> &, struct FArkPpFxAaParameters &)
//   0x560410  public: void __thiscall FArkPpNodeAAProxy::FlushMlaaEdgeShader<class TMLAAEdgeDetectionPixelShader<1>>(class TShaderMapRef<class FMLAAVertexShader> &, struct FArkPpMlaaParameters &)
//   0x5604f0  public: void __thiscall FArkPpNodeAAProxy::FlushMlaaEdgeShader<class TMLAAEdgeDetectionPixelShader<0>>(class TShaderMapRef<class FMLAAVertexShader> &, struct FArkPpMlaaParameters &)
//   0x5605d0  public: void __thiscall FArkPpNodeAAProxy::FlushMlaaEdgeShader<class TMLAABlendPixelShader<1>>(class TShaderMapRef<class FMLAAVertexShader> &, struct FArkPpMlaaParameters &)
//   0x5606b0  public: void __thiscall FArkPpNodeAAProxy::FlushMlaaEdgeShader<class TMLAABlendPixelShader<0>>(class TShaderMapRef<class FMLAAVertexShader> &, struct FArkPpMlaaParameters &)
//   0x561080  public: static class UClass * __cdecl UArkPpNodeAA::StaticClassNoInline(void)
//   0x5610b0  public: unsigned int __thiscall FArkPpNodeAAProxy::RenderFxaa(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x561570  public: void __thiscall FArkPpNodeAAProxy::RenderMlaaEdgeDetectingPass(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig, unsigned int)
//   0x561990  public: void __thiscall FArkPpNodeAAProxy::RenderMlaaComputeEdgeLengthPass(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig, unsigned int)
//   0x561ee0  public: void __thiscall FArkPpNodeAAProxy::RenderMlaaBlendColorPass(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig, unsigned int)
//   0x5623f0  public: unsigned int __thiscall FArkPpNodeAAProxy::RenderMlaa(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x5625d0  public: virtual unsigned int __thiscall FArkPpNodeAAProxy::Render(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x565120  public: virtual unsigned int __thiscall UArkPpNodeAA::IsValid(struct FArkPpIsValidData &)
//   0x5651c0  public: virtual class FArkPpNodeProxy * __thiscall UArkPpNodeAA::CreateSceneProxy(struct FArkPpCreateProxyConfig &)
//   0xb9f070  _dynamic_initializer_for__FFXAAVertexShader::StaticType__
//   0xb9f0b0  _dynamic_initializer_for__TFXAAPixelShader_0_1_0_::StaticType__
//   0xb9f0f0  _dynamic_initializer_for__TFXAAPixelShader_2_1_0_::StaticType__
//   0xb9f130  _dynamic_initializer_for__TFXAAPixelShader_1_1_0_::StaticType__
//   0xb9f170  _dynamic_initializer_for__TFXAAPixelShader_0_1_1_::StaticType__
//   0xb9f1b0  _dynamic_initializer_for__TFXAAPixelShader_2_1_1_::StaticType__
//   0xb9f1f0  _dynamic_initializer_for__TFXAAPixelShader_1_1_1_::StaticType__
//   0xb9f230  _dynamic_initializer_for__TMLAAEdgeDetectionPixelShader_0_::StaticType__
//   0xb9f270  _dynamic_initializer_for__TMLAAEdgeDetectionPixelShader_1_::StaticType__
//   0xb9f2b0  _dynamic_initializer_for__TMLAABlendPixelShader_0_::StaticType__
//   0xb9f2f0  _dynamic_initializer_for__TMLAABlendPixelShader_1_::StaticType__
//   0xb9f330  _dynamic_initializer_for__FMLAAVertexShader::StaticType__
//   0xb9f370  _dynamic_initializer_for__FMLAAComputeLineLengthPixelShader::StaticType__
