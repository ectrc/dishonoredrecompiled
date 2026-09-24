// Engine/src/arkbloompartsrendering.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (32):
//   0x54b650  public: virtual unsigned int __thiscall FBloomComposePixelShader::Serialize(class FArchive &)
//   0x54b690  public: static class FShader * __cdecl FShadowProjectionVertexShader::ConstructSerializedInstance(void)
//   0x54b700  public: virtual unsigned int __thiscall VisualizeTexturePixelShader::Serialize(class FArchive &)
//   0x54b740  public: virtual __thiscall bloom::FArkGaussianVertexDeclaration::~FArkGaussianVertexDeclaration(void)
//   0x54b9e0  public: virtual unsigned int __thiscall TBloomBlurPixelShader<5>::Serialize(class FArchive &)
//   0x54dca0  public: __thiscall TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>::TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>(class FVertexFactory const *, class FMaterialRenderProxy const *, unsigned int, unsigned int)
//   0x54dd20  public: void __thiscall TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>::SetMeshRenderState(class FSceneView const &, class FPrimitiveSceneInfo const *, struct FMeshElement const &, unsigned int, struct FMeshDrawingPolicy::ElementDataType const &)const
//   0x54e510  public: static class FShader * __cdecl FBloomComposePixelShader::ConstructSerializedInstance(void)
//   0x54e5a0  public: void __thiscall FBloomDownSamplePixelShader::SetParameters(class FViewInfo const &, class FTexture const *)
//   0x54e6c0  public: void __thiscall FArkBloomPartPrimSet::AddScenePrimitive(class FPrimitiveSceneInfo *, class FViewInfo const &)
//   0x54e6d0  public: virtual void __thiscall bloom::FArkGaussianVertexDeclaration::InitRHI(void)
//   0x54ed80  public: static class FShader * __cdecl FSplashPixelShader::ConstructSerializedInstance(void)
//   0x55cfd0  public: __thiscall TPpMaterialVertexShader<class FPpMaterialMeshPolicy>::TPpMaterialVertexShader<class FPpMaterialMeshPolicy>(void)
//   0x55d050  public: virtual unsigned int __thiscall TBasePassVertexShader<class FNoLightMapPolicy>::IsUniformExpressionSetValid(class FUniformExpressionSet const &)const
//   0x55d1b0  public: virtual unsigned int __thiscall FShadowDepthPixelShader::IsUniformExpressionSetValid(class FUniformExpressionSet const &)const
//   0x55e280  public: static class FShader * __cdecl TDistortionMeshPixelShader<class FDistortMeshAccumulatePolicy>::ConstructSerializedInstance(void)
//   0x560890  public: void __thiscall TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>::DrawShared(class FSceneView const *, class TDynamicRHIResource<9> *)const
//   0x5609f0  public: class TDynamicRHIResourceReference<9> __thiscall TBloomPartMeshDrawingPolicy<class FBloomPartMeshPolicy>::CreateBoundShaderState(unsigned long)
//   0x560b00  void __cdecl bloom::GaussianBlur(class TDynamicRHIResourceReference<12> const &, class TDynamicRHIResourceReference<14> const &, class TDynamicRHIResourceReference<12> const &, class TDynamicRHIResourceReference<14> const &, unsigned int, unsigned int, struct bloom::URECT<unsigned int>, unsigned int, float)
//   0x564980  public: static unsigned int __cdecl TBloomPartMeshDrawingPolicyFactory<class FBloomPartMeshPolicy>::DrawStaticMesh(class FSceneView const *, unsigned int, class FStaticMesh const &, unsigned int, class FPrimitiveSceneInfo const *, class FHitProxyId)
//   0x564ad0  public: static unsigned int __cdecl TBloomPartMeshDrawingPolicyFactory<class FBloomPartMeshPolicy>::DrawDynamicMesh(class FSceneView const &, unsigned int, struct FMeshElement const &, unsigned int, unsigned int, class FPrimitiveSceneInfo const *, class FHitProxyId)
//   0x565e80  public: unsigned int __thiscall FArkBloomPartPrimSet::DrawBloomPrims(class FViewInfo const *, unsigned int, unsigned int)
//   0x566120  private: unsigned int __thiscall FSceneRenderer::RenderBloomParts(unsigned int)
//   0xb9ee50  _dynamic_initializer_for__FBloomComposePixelShader::StaticType__
//   0xb9ee90  _dynamic_initializer_for__FBloomComposeVertexShader::StaticType__
//   0xb9eed0  _dynamic_initializer_for__FBloomDownSampleVertexShader::StaticType__
//   0xb9ef10  _dynamic_initializer_for__FBloomDownSamplePixelShader::StaticType__
//   0xb9ef50  _dynamic_initializer_for__TBloomBlurVertexShader_5_::StaticType__
//   0xb9ef90  _dynamic_initializer_for__TBloomBlurPixelShader_5_::StaticType__
//   0xb9efd0  _dynamic_initializer_for__TBloomPartMeshVertexShader_FBloomPartMeshPolicy_::StaticType__
//   0xb9f010  _dynamic_initializer_for__TBloomPartMeshPixelShader_FBloomPartMeshPolicy_::StaticType__
//   0xb9f050  bloom::_dynamic_initializer_for__GArkGaussianVertexDeclaration__
