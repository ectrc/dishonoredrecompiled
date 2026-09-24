// Engine/src/arkppnodematerial.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (28):
//   0x54f9d0  public: virtual class FString __thiscall UArkPpNodeMaterial::InputName(unsigned int)const
//   0x54fb40  public: virtual unsigned int __thiscall UArkPpNodeMaterial::LinkInput(unsigned int, class UArkPpNode *)
//   0x54fbc0  public: virtual unsigned int __thiscall UArkPpNodeMaterial::UnlinkInput(class UArkPpNode *)
//   0x54fc60  public: virtual unsigned int __thiscall UArkPpNodeMaterial::UnlinkInput(unsigned int)
//   0x559670  public: __thiscall FArkPpNodeMaterialProxy::FArkPpNodeMaterialProxy(struct FArkPpCreateProxyConfig &, class UArkPpNodeMaterial *)
//   0x559990  public: virtual unsigned int __thiscall FArkPpNodeMaterialProxy::GetSurfaceSizeX(void)
//   0x5599a0  public: virtual unsigned int __thiscall FArkPpNodeMaterialProxy::GetSurfaceSizeY(void)
//   0x5599b0  public: virtual class TDynamicRHIResourceReference<12> const __thiscall FArkPpNodeMaterialProxy::GetSurface(class FViewInfo const &)
//   0x5599e0  public: virtual class TDynamicRHIResourceReference<14> const __thiscall FArkPpNodeMaterialProxy::GetTexture(class FViewInfo const &)
//   0x55a570  public: class TDynamicRHIResourceReference<9> __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::CreateBoundShaderState(unsigned long)
//   0x55a640  public: void __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::SetMeshRenderState(class FSceneView const &, class FPrimitiveSceneInfo const *, struct FMeshElement const &, unsigned int, struct FMeshDrawingPolicy::ElementDataType const &)const
//   0x55a760  public: void __thiscall TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 0>>::SetParameters(class FVertexFactory const *, class FMaterialRenderProxy const *, class FSceneView const *, struct FVector2D, class FTexture const * const)
//   0x55b010  public: __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>(class FVertexFactory const *, class FMaterialRenderProxy const *, struct FPpMaterialContextType, unsigned int)
//   0x55b6b0  public: void __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::DrawShared(class FSceneView const *, class TDynamicRHIResource<9> *)const
//   0x55b840  public: virtual __thiscall FArkPpNodeMaterialProxy::~FArkPpNodeMaterialProxy(void)
//   0x55d070  public: __thiscall TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 1>>::TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 1>>(void)
//   0x55d0f0  public: virtual unsigned int __thiscall TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 0>>::Serialize(class FArchive &)
//   0x55d360  public: static unsigned int __cdecl TPpMaterialDrawingPolicyFactory<class FPpMaterialMeshPolicy>::DrawDynamicMesh(class FSceneView const &, struct FPpMaterialContextType, struct FMeshElement const &, unsigned int, unsigned int, class FPrimitiveSceneInfo const *, class FHitProxyId)
//   0x55e2f0  public: static class FShader * __cdecl TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 0>>::ConstructSerializedInstance(void)
//   0x55f3f0  public: static void __cdecl UArkPpNodeMaterial::InitializePrivateStaticClassUArkPpNodeMaterial(void)
//   0x55f410  public: virtual unsigned int __thiscall FArkPpNodeMaterialProxy::Render(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x564720  public: static class UClass * __cdecl UArkPpNodeMaterial::GetPrivateStaticClassUArkPpNodeMaterial(wchar_t const *)
//   0x564bf0  public: static class UClass * __cdecl UArkPpNodeMaterial::StaticClassNoInline(void)
//   0x5657a0  public: virtual unsigned int __thiscall UArkPpNodeMaterial::IsValid(struct FArkPpIsValidData &)
//   0x5658c0  public: virtual class FArkPpNodeProxy * __thiscall UArkPpNodeMaterial::CreateSceneProxy(struct FArkPpCreateProxyConfig &)
//   0xb9f8d0  _dynamic_initializer_for__TPpMaterialVertexShader_FPpMaterialMeshPolicy_::StaticType__
//   0xb9f910  _dynamic_initializer_for__TPpMaterialPixelShader_ArkPpAddGammaValue_FPpMaterialMeshPolicy_0___::StaticType__
//   0xb9f950  _dynamic_initializer_for__TPpMaterialPixelShader_ArkPpAddGammaValue_FPpMaterialMeshPolicy_1___::StaticType__
