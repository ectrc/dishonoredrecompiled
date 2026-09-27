// Engine/src/arkppnodematerial.cpp
// DISHONORED(port): Arkane's post-process material node - the node that carries Dishonored's colour treatment.
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
//   0x55a640  public: void __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::SetMeshRenderState(...)
//   0x55a760  public: void __thiscall TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 0>>::SetParameters(...)
//   0x55b010  public: __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>(...)
//   0x55b6b0  public: void __thiscall TPpMaterialDrawingPolicy<class FPpMaterialMeshPolicy>::DrawShared(class FSceneView const *, class TDynamicRHIResource<9> *)const
//   0x55b840  public: virtual __thiscall FArkPpNodeMaterialProxy::~FArkPpNodeMaterialProxy(void)
//   0x55d070  public: __thiscall TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 1>>::TPpMaterialPixelShader(void)
//   0x55d0f0  public: virtual unsigned int __thiscall TPpMaterialPixelShader<class ArkPpAddGammaValue<class FPpMaterialMeshPolicy, 0>>::Serialize(class FArchive &)
//   0x55d360  public: static unsigned int __cdecl TPpMaterialDrawingPolicyFactory<class FPpMaterialMeshPolicy>::DrawDynamicMesh(...)
//   0x55e2f0  public: static class FShader * __cdecl TPpMaterialPixelShader<...,0>::ConstructSerializedInstance(void)
//   0x55f260  public: virtual unsigned int __thiscall UArkPpNodeMaterial::NumInputs(void)const
//   0x55f270  public: virtual class UArkPpNode * __thiscall UArkPpNodeMaterial::GetInput(unsigned int)
//   0x55f3f0  public: static void __cdecl UArkPpNodeMaterial::InitializePrivateStaticClassUArkPpNodeMaterial(void)
//   0x55f410  public: virtual unsigned int __thiscall FArkPpNodeMaterialProxy::Render(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x5657a0  public: virtual unsigned int __thiscall UArkPpNodeMaterial::IsValid(struct FArkPpIsValidData &)
//   0x5658c0  public: virtual class FArkPpNodeProxy * __thiscall UArkPpNodeMaterial::CreateSceneProxy(struct FArkPpCreateProxyConfig &)
//   0xb9f8d0  _dynamic_initializer_for__TPpMaterialVertexShader_FPpMaterialMeshPolicy_::StaticType__
//   0xb9f910  _dynamic_initializer_for__TPpMaterialPixelShader_ArkPpAddGammaValue_FPpMaterialMeshPolicy_0___::StaticType__
//   0xb9f950  _dynamic_initializer_for__TPpMaterialPixelShader_ArkPpAddGammaValue_FPpMaterialMeshPolicy_1___::StaticType__

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "DrawingPolicy.h"
#include "TileRendering.h"
#include "arkpp.h"

/**
 * DISHONORED(written): the shader types of Arkane's post-process material node (UArkPpNodeMaterial, 2013 rva 0x519050 (2012 0x5599b0) ff.),
 * which draws a material over a post-process surface with up to eight bound node textures.
 */
class FPpMaterialMeshPolicy {};
class FPpMaterialLinearSpaceMeshPolicy {};
class FPpMaterialGammaSpaceMeshPolicy {};

/** The number of post-process node textures a pp-material pixel shader can sample (2012 PDB: FShaderResourceParameter[8]). */
#define ARK_PP_MATERIAL_NUM_TEXTURES 8

/**
 * DISHONORED(port): 2012 PDB FPpMaterialContextType (524 bytes): the eight node textures the material samples, the size
 * of the surface it draws into and whether the output has to be gamma-corrected.
 */
struct FPpMaterialContextType
{
	UINT m_NeedGammaOutput;
	UINT m_SurfaceX;
	UINT m_SurfaceY;
	FTexture mSamplers[ARK_PP_MATERIAL_NUM_TEXTURES];

	FPpMaterialContextType()
		: m_NeedGammaOutput(FALSE)
		, m_SurfaceX(0)
		, m_SurfaceY(0)
	{}
};

/**
 * DISHONORED(layout): TPpMaterialVertexShader<FPpMaterialMeshPolicy> is 196 bytes (2012 PDB): FShader,
 * FVertexFactoryParameterRef @108, FMaterialVertexShaderParameters @136 (2012 ctor rva 0x55cfd0).
 */
template<typename MeshPolicyType>
class TPpMaterialVertexShader : public FMeshMaterialVertexShader
{
	DECLARE_SHADER_TYPE(TPpMaterialVertexShader,MeshMaterial);
public:
	TPpMaterialVertexShader() {}

	TPpMaterialVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FMeshMaterialVertexShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.ParameterMap);
	}

	// DISHONORED(bringup): retail's gating is not recoverable (no shader compiler in the shipping exes); the type accepts
	// every material and compiles nothing, so only the cooked shaders ever exist.
	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		bShaderHasOutdatedParameters |= Ar << VertexFactoryParameters;
		Ar << MaterialParameters;
		return bShaderHasOutdatedParameters;
	}

	virtual UBOOL IsUniformExpressionSetValid(const FUniformExpressionSet& UniformExpressionSet) const
	{
		return MaterialParameters.IsUniformExpressionSetValid(UniformExpressionSet);
	}

	/** DISHONORED(port): 2013 rva 0x51ad50 (2012 0x55b6b0) (inside DrawShared) - the vertex factory's and the material's parameters. */
	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView* View)
	{
		VertexFactoryParameters.Set(this,VertexFactory,*View);
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy,*MaterialRenderProxy->GetMaterial(),View->Family->CurrentWorldTime,View->Family->CurrentRealTime,View);
		MaterialParameters.Set(this,MaterialRenderContext);
	}

	void SetMesh(const FPrimitiveSceneInfo* PrimitiveSceneInfo,const FMeshBatch& Mesh,INT BatchElementIndex,const FSceneView& View)
	{
		VertexFactoryParameters.SetMesh(this,Mesh,BatchElementIndex,View);
		MaterialParameters.SetMesh(this,PrimitiveSceneInfo,Mesh,BatchElementIndex,View);
	}

private:
	FMaterialVertexShaderParameters MaterialParameters;
};

/**
 * DISHONORED(layout): TPpMaterialPixelShader is 368 bytes (2012 PDB TPpMaterialPixelShader<ArkPpAddGammaValue<Policy,0|1> >):
 * FShader, FMaterialPixelShaderParameters @108, FShaderResourceParameter m_ArkPpTextureSampleParameter[8] @300,
 * m_ScreenPositionScaleBias @348, m_SurfaceResolution @354, m_ScreenResolution @360.
 * DISHONORED(port): 2013 rva 0x51caa0 (2012 0x55d0f0) Serialize: the material parameters, the eight texture samplers, then the three
 * vectors; 0x55a760 SetParameters.
 * The registered names are TPpMaterialPixelShader<FPpMaterialLinearSpaceMeshPolicy> / <FPpMaterialGammaSpaceMeshPolicy>
 * (the cooked type names), which is why the gamma variant is a policy here and not the retail template's integer argument.
 */
template<typename MeshPolicyType>
class TPpMaterialPixelShader : public FShader
{
	DECLARE_SHADER_TYPE(TPpMaterialPixelShader,MeshMaterial);
public:
	TPpMaterialPixelShader() {}

	TPpMaterialPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FShader(Initializer)
	{
		MaterialParameters.Bind(Initializer.ParameterMap);
		for (INT TextureIndex = 0; TextureIndex < ARK_PP_MATERIAL_NUM_TEXTURES; TextureIndex++)
		{
			// DISHONORED(bringup): the retail parameter names are not in the shipping exes (no Bind); loaded from the caches only
			m_ArkPpTextureSampleParameter[TextureIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("ArkPpTexture_%u"),TextureIndex),TRUE);
		}
		m_ScreenPositionScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("ScreenPositionScaleBias"),TRUE);
		m_SurfaceResolutionParameter.Bind(Initializer.ParameterMap,TEXT("SurfaceResolution"),TRUE);
		m_ScreenResolutionParameter.Bind(Initializer.ParameterMap,TEXT("ScreenResolution"),TRUE);
	}

	static UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material,const FVertexFactoryType* VertexFactoryType)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << MaterialParameters;
		for (INT TextureIndex = 0; TextureIndex < ARK_PP_MATERIAL_NUM_TEXTURES; TextureIndex++)
		{
			Ar << m_ArkPpTextureSampleParameter[TextureIndex];
		}
		Ar << m_ScreenPositionScaleBiasParameter;
		Ar << m_SurfaceResolutionParameter;
		Ar << m_ScreenResolutionParameter;
		return bShaderHasOutdatedParameters;
	}

	virtual UBOOL IsUniformExpressionSetValid(const FUniformExpressionSet& UniformExpressionSet) const
	{
		return MaterialParameters.IsUniformExpressionSetValid(UniformExpressionSet);
	}

	/**
	 * DISHONORED(port): 2013 rva 0x519e10 (2012 0x55a760) - the material's parameters, the eight node textures (only where the shader
	 * bound one and the node produced a texture), the view's screen-position scale and bias, and the surface and screen
	 * resolutions with their reciprocals in zw.
	 */
	void SetParameters(const FVertexFactory* VertexFactory,const FMaterialRenderProxy* MaterialRenderProxy,const FSceneView* View,FVector2D Resolution,const FTexture* iSamplers)
	{
		FMaterialRenderContext MaterialRenderContext(MaterialRenderProxy,*MaterialRenderProxy->GetMaterial(),View->Family->CurrentWorldTime,View->Family->CurrentRealTime,View);
		MaterialParameters.Set(this,MaterialRenderContext,SceneDepthUsage_Normal);

		for (INT TextureIndex = 0; TextureIndex < ARK_PP_MATERIAL_NUM_TEXTURES; TextureIndex++)
		{
			if (m_ArkPpTextureSampleParameter[TextureIndex].IsBound() && iSamplers[TextureIndex].TextureRHI)
			{
				SetTextureParameter(GetPixelShader(),m_ArkPpTextureSampleParameter[TextureIndex],&iSamplers[TextureIndex]);
			}
		}

		SetPixelShaderValue(GetPixelShader(),m_ScreenPositionScaleBiasParameter,View->ScreenPositionScaleBias);
		SetPixelShaderValue(GetPixelShader(),m_SurfaceResolutionParameter,
			FVector4(Resolution.X,Resolution.Y,1.0f / Resolution.X,1.0f / Resolution.Y));
		SetPixelShaderValue(GetPixelShader(),m_ScreenResolutionParameter,
			FVector4((FLOAT)View->RenderTargetSizeX,(FLOAT)View->RenderTargetSizeY,1.0f / (FLOAT)View->RenderTargetSizeX,1.0f / (FLOAT)View->RenderTargetSizeY));
	}

	void SetMesh(const FPrimitiveSceneInfo* PrimitiveSceneInfo,const FMeshBatch& Mesh,INT BatchElementIndex,const FSceneView& View,UBOOL bBackFace)
	{
		MaterialParameters.SetMesh(this,PrimitiveSceneInfo,Mesh,BatchElementIndex,View,bBackFace);
	}

private:
	FMaterialPixelShaderParameters MaterialParameters;
	FShaderResourceParameter m_ArkPpTextureSampleParameter[ARK_PP_MATERIAL_NUM_TEXTURES];
	FShaderParameter m_ScreenPositionScaleBiasParameter;
	FShaderParameter m_SurfaceResolutionParameter;
	FShaderParameter m_ScreenResolutionParameter;
};

// DISHONORED(retail): 2013 rva 0xb83150 ("ArkPpMaterialVertexShader", 786 / 23), 0xb83190 / 0xb831d0
// ("ArkPpMaterialPixelShader", 801 / 23 - the highest material gate in the exe).
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TPpMaterialVertexShader<FPpMaterialMeshPolicy>,TEXT("ArkPpMaterialVertexShader"),TEXT("Main"),SF_Vertex,786,23);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TPpMaterialPixelShader<FPpMaterialLinearSpaceMeshPolicy>,TEXT("ArkPpMaterialPixelShader"),TEXT("Main"),SF_Pixel,801,23);
IMPLEMENT_MATERIAL_SHADER_TYPE(template<>,TPpMaterialPixelShader<FPpMaterialGammaSpaceMeshPolicy>,TEXT("ArkPpMaterialPixelShader"),TEXT("Main"),SF_Pixel,801,23);

/*-----------------------------------------------------------------------------
	TPpMaterialDrawingPolicy (2012 rvas 0x55b010 ctor, 0x55b6b0 DrawShared, 0x55a640 SetMeshRenderState,
	0x55a570 CreateBoundShaderState, 0x55d360 the factory)
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): the drawing policy of the post-process material pass. One vertex shader and both pixel shaders are
 * fetched from the material; the context says which of the two pixel shaders draws.
 */
class TPpMaterialDrawingPolicy : public FMeshDrawingPolicy
{
public:
	typedef FMeshDrawingPolicy::ElementDataType ElementDataType;

	/** DISHONORED(port): 2013 rva 0x51a690 (2012 0x55b010). */
	TPpMaterialDrawingPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialRenderProxy* InMaterialRenderProxy,
		const FMaterial& InMaterialResource,
		const FPpMaterialContextType& InDrawContext,
		UBOOL bInOverrideWithShaderComplexity
		)
		: FMeshDrawingPolicy(InVertexFactory,InMaterialRenderProxy,InMaterialResource,bInOverrideWithShaderComplexity)
		, mDrawContext(InDrawContext)
	{
		// DISHONORED(bringup): retail fetches these with FMaterial::GetShader, which appErrorf's when a material's
		// cooked map has no such shader. A post-process material that was cooked without the pp-material types would
		// then take the game down mid-frame, so the three are looked up without the assert and the pass reports and
		// skips instead (HasShaders below).
		FVertexFactoryType* VertexFactoryType = InVertexFactory->GetType();
		m_VertexShader = (TPpMaterialVertexShader<FPpMaterialMeshPolicy>*)FindShader(InMaterialResource,&TPpMaterialVertexShader<FPpMaterialMeshPolicy>::StaticType,VertexFactoryType);
		m_LinearPixelShader = (TPpMaterialPixelShader<FPpMaterialLinearSpaceMeshPolicy>*)FindShader(InMaterialResource,&TPpMaterialPixelShader<FPpMaterialLinearSpaceMeshPolicy>::StaticType,VertexFactoryType);
		m_GammaPixelShader = (TPpMaterialPixelShader<FPpMaterialGammaSpaceMeshPolicy>*)FindShader(InMaterialResource,&TPpMaterialPixelShader<FPpMaterialGammaSpaceMeshPolicy>::StaticType,VertexFactoryType);
	}

	/** The shader the context asks for, or NULL. */
	static FShader* FindShader(const FMaterial& MaterialResource,FShaderType* ShaderType,FVertexFactoryType* VertexFactoryType)
	{
		const FMaterialShaderMap* ShaderMap = MaterialResource.GetShaderMap();
		const FMeshMaterialShaderMap* MeshShaderMap = ShaderMap ? ShaderMap->GetMeshShaderMap(VertexFactoryType) : NULL;
		return MeshShaderMap ? MeshShaderMap->GetShader(ShaderType) : NULL;
	}

	UBOOL HasShaders() const
	{
		return m_VertexShader != NULL && (mDrawContext.m_NeedGammaOutput ? (m_GammaPixelShader != NULL) : (m_LinearPixelShader != NULL));
	}

	UBOOL Matches(const TPpMaterialDrawingPolicy& Other) const
	{
		return FMeshDrawingPolicy::Matches(Other)
			&& m_VertexShader == Other.m_VertexShader
			&& m_LinearPixelShader == Other.m_LinearPixelShader
			&& m_GammaPixelShader == Other.m_GammaPixelShader;
	}

	/** DISHONORED(port): 2013 rva 0x51ad50 (2012 0x55b6b0) - the pass draws with depth off, whatever the material asks for. */
	void DrawShared(const FSceneView* View,FBoundShaderStateRHIParamRef BoundShaderState) const
	{
		m_VertexShader->SetParameters(VertexFactory,MaterialRenderProxy,View);
		const FVector2D Resolution((FLOAT)mDrawContext.m_SurfaceX,(FLOAT)mDrawContext.m_SurfaceY);
		if (mDrawContext.m_NeedGammaOutput)
		{
			m_GammaPixelShader->SetParameters(VertexFactory,MaterialRenderProxy,View,Resolution,mDrawContext.mSamplers);
		}
		else
		{
			m_LinearPixelShader->SetParameters(VertexFactory,MaterialRenderProxy,View,Resolution,mDrawContext.mSamplers);
		}
		FMeshDrawingPolicy::DrawShared(View);
		RHISetBoundShaderState(BoundShaderState);
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	}

	/** DISHONORED(port): 2013 rva 0x519c20 (2012 0x55a570). */
	FBoundShaderStateRHIRef CreateBoundShaderState(DWORD DynamicStride = 0)
	{
		FVertexDeclarationRHIRef VertexDeclaration;
		DWORD StreamStrides[MaxVertexElementCount];
		FMeshDrawingPolicy::GetVertexDeclarationInfo(VertexDeclaration,StreamStrides);
		if (DynamicStride)
		{
			StreamStrides[0] = DynamicStride;
		}
		FShader* PixelShader = mDrawContext.m_NeedGammaOutput ? (FShader*)m_GammaPixelShader : (FShader*)m_LinearPixelShader;
		return RHICreateBoundShaderState(VertexDeclaration,StreamStrides,m_VertexShader->GetVertexShader(),PixelShader->GetPixelShader(),EGST_None);
	}

	/** DISHONORED(port): 2013 rva 0x519cf0 (2012 0x55a640). */
	void SetMeshRenderState(
		const FSceneView& View,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshBatch& Mesh,
		INT BatchElementIndex,
		UBOOL bBackFace,
		const ElementDataType& ElementData
		) const
	{
		EmitMeshDrawEvents(PrimitiveSceneInfo,Mesh);
		m_VertexShader->SetMesh(PrimitiveSceneInfo,Mesh,BatchElementIndex,View);
		if (mDrawContext.m_NeedGammaOutput)
		{
			m_GammaPixelShader->SetMesh(PrimitiveSceneInfo,Mesh,BatchElementIndex,View,bBackFace);
		}
		else
		{
			m_LinearPixelShader->SetMesh(PrimitiveSceneInfo,Mesh,BatchElementIndex,View,bBackFace);
		}
		FMeshDrawingPolicy::SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,BatchElementIndex,bBackFace,ElementData);
	}

private:
	TPpMaterialVertexShader<FPpMaterialMeshPolicy>* m_VertexShader;
	TPpMaterialPixelShader<FPpMaterialLinearSpaceMeshPolicy>* m_LinearPixelShader;
	TPpMaterialPixelShader<FPpMaterialGammaSpaceMeshPolicy>* m_GammaPixelShader;
	FPpMaterialContextType mDrawContext;
};

/** DISHONORED(port): 2013 rva 0x51cd70 (2012 0x55d360) - the factory FTileRenderer::DrawTile is instantiated with. */
class TPpMaterialDrawingPolicyFactory
{
public:
	enum { bAllowSimpleElements = FALSE };
	typedef FPpMaterialContextType ContextType;

	static UBOOL DrawDynamicMesh(
		const FSceneView& View,
		ContextType DrawingContext,
		const FMeshBatch& Mesh,
		UBOOL bBackFace,
		UBOOL bPreFog,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		FHitProxyId HitProxyId
		)
	{
		if (!Mesh.MaterialRenderProxy)
		{
			return FALSE;
		}
		TPpMaterialDrawingPolicy DrawingPolicy(
			Mesh.VertexFactory,
			Mesh.MaterialRenderProxy,
			*Mesh.MaterialRenderProxy->GetMaterial(),
			DrawingContext,
			(View.Family->ShowFlags & SHOW_ShaderComplexity) != 0
			);
		if (!DrawingPolicy.HasShaders())
		{
			// DISHONORED(bringup): the material has no pp-material shader for this vertex factory - see FindShader.
			static UBOOL bReported = FALSE;
			if (!bReported)
			{
				bReported = TRUE;
				warnf(TEXT("DISHONORED(bringup): FArkPp material node: %s has no ArkPpMaterial shader for %s, the node is skipped"),
					*Mesh.MaterialRenderProxy->GetMaterial()->GetFriendlyName(),
					Mesh.VertexFactory->GetType()->GetName());
			}
			GDisCensusArkPpSkipped++;
			return FALSE;
		}
		DrawingPolicy.DrawShared(&View,DrawingPolicy.CreateBoundShaderState(Mesh.GetDynamicVertexStride()));
		for (INT BatchElementIndex = 0; BatchElementIndex < Mesh.Elements.Num(); BatchElementIndex++)
		{
			DrawingPolicy.SetMeshRenderState(View,PrimitiveSceneInfo,Mesh,BatchElementIndex,bBackFace,TPpMaterialDrawingPolicy::ElementDataType());
			DrawingPolicy.DrawMesh(Mesh,BatchElementIndex);
		}
		GDisCensusArkPpMaterialDraws++;
		return TRUE;
	}
};

/*-----------------------------------------------------------------------------
	FArkPpNodeMaterialProxy (2012 PDB 48 bytes)
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): the proxy of a material node. It renders its input nodes first and binds each one's texture with
 * the sampler state its FAnInput asks for, then its target node, then draws the material over the target's surface -
 * or over the back buffer when it is the node that holds m_bForceToDestination.
 */
class FArkPpNodeMaterialProxy : public FArkPpNodeProxy
{
public:
	/** DISHONORED(port): 2013 rva 0x518d00 (2012 0x559670). */
	FArkPpNodeMaterialProxy(FArkPpCreateProxyConfig& Config,UArkPpNodeMaterial* InNode)
		: m_MatProxy(NULL)
		, m_GoToLR(FALSE)
		, m_ToBackBuffer(Config.mbRedirectToBackBuffer)
		, m_TargetMask(InNode->m_bPreserveAlphaChannel ? (EColorWriteMask)CW_RGB : (EColorWriteMask)CW_RGBA)
		, m_pController(InNode->m_Controller)
	{
		// the destination belongs to the first material node that asks for it
		Config.mbRedirectToBackBuffer = 0;

		UMaterialInterface* Material = InNode->m_Material;
		if (m_pController)
		{
			FArkUberPpParameters lCopy = InNode->m_UberParameters;
			UBOOL bOverrideUber = InNode->m_bOverrideUberPp;
			FLOAT UberOverrideWeight = InNode->m_UberParametersWeight;
			UMaterialInterface* ControllerMaterial = m_pController->Update(InNode,&bOverrideUber,&UberOverrideWeight,&lCopy);
			if (bOverrideUber)
			{
				Config.PushUberOverride(lCopy,UberOverrideWeight);
			}
			if (ControllerMaterial)
			{
				Material = ControllerMaterial;
			}
		}
		else if (InNode->m_bOverrideUberPp)
		{
			Config.PushUberOverride(InNode->m_UberParameters,InNode->m_UberParametersWeight);
		}
		m_MatProxy = Material ? Material->GetRenderProxy(FALSE) : NULL;

		// one entry per declared input, in order; the sampler state is the input's tiling pair
		m_InputProxy.Empty(InNode->m_Inputs.Num());
		for (INT InputIndex = 0; InputIndex < InNode->m_Inputs.Num(); InputIndex++)
		{
			const FAnInput& Input = InNode->m_Inputs(InputIndex);
			// AddZeroed, not Add: TArray::Add leaves the element uninitialised and the entry holds a TRefCountPtr
			FAnEntry& Entry = m_InputProxy(m_InputProxy.AddZeroed());
			Entry.m_pNode = Input.m_Node ? Input.m_Node->CreateSceneProxy(Config) : NULL;
			Entry.m_SState = Input.m_TileU + 3 * Input.m_TileV;
		}

		m_TargetProxy = InNode->m_SurfaceTarget ? InNode->m_SurfaceTarget->CreateSceneProxy(Config) : NULL;
	}

	/** DISHONORED(port): 2013 rva 0x51e720 (2012 0x55f410). */
	virtual UBOOL Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config);

	/** DISHONORED(port): 2012 rvas 0x5599b0 / 0x5599e0 / 0x559990 / 0x5599a0 - everything is the target's. */
	virtual const FSurfaceRHIRef GetSurface(const FViewInfo& View) { return m_TargetProxy ? m_TargetProxy->GetSurface(View) : FSurfaceRHIRef(); }
	virtual const FTexture2DRHIRef GetTexture(const FViewInfo& View) { return m_TargetProxy ? m_TargetProxy->GetTexture(View) : FTexture2DRHIRef(); }
	virtual UINT GetSurfaceSizeX() { return m_TargetProxy ? m_TargetProxy->GetSurfaceSizeX() : 0; }
	virtual UINT GetSurfaceSizeY() { return m_TargetProxy ? m_TargetProxy->GetSurfaceSizeY() : 0; }

private:
	/** 2012 PDB FArkPpNodeMaterialProxy::FAnEntry (8 bytes): the input's proxy and the index of its sampler state. */
	struct FAnEntry
	{
		TRefCountPtr<FArkPpNodeProxy> m_pNode;
		UINT m_SState;
	};

	const FMaterialRenderProxy* m_MatProxy;
	UINT m_GoToLR;
	UINT m_ToBackBuffer;
	TRefCountPtr<FArkPpNodeProxy> m_TargetProxy;
	TArray<FAnEntry> m_InputProxy;
	EColorWriteMask m_TargetMask;
	UArkPpNodeController* m_pController;
};

/**
 * DISHONORED(port): 2013 rva 0x51e720 (2012 0x55f410). The nine sampler states are the tiling pairs of a bilinear sampler, indexed
 * m_TileU + 3 * m_TileV (AM_Wrap, AM_Clamp, AM_Mirror); retail binds the filter as SF_Bilinear whatever the input's
 * m_FilterType says.
 */
UBOOL FArkPpNodeMaterialProxy::Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
{
	if (m_bDone)
	{
		return FALSE;
	}
	m_bDone = TRUE;
	if (m_pController)
	{
		m_pController->Render(EPpRs_BeforeAll,Scene,&View);
	}
	if (!m_MatProxy || !m_TargetProxy)
	{
		GDisCensusArkPpSkipped++;
		return FALSE;
	}
	GDisCensusArkPpNodes++;

	UBOOL bDirty = FALSE;
	FTexture LArkPPSamplers[ARK_PP_MATERIAL_NUM_TEXTURES];
	FSamplerStateRHIRef sStates[9];
	sStates[0] = TStaticSamplerState<SF_Bilinear,AM_Wrap,AM_Wrap,AM_Wrap>::GetRHI();
	sStates[1] = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Wrap,AM_Wrap>::GetRHI();
	sStates[2] = TStaticSamplerState<SF_Bilinear,AM_Mirror,AM_Wrap,AM_Wrap>::GetRHI();
	sStates[3] = TStaticSamplerState<SF_Bilinear,AM_Wrap,AM_Clamp,AM_Wrap>::GetRHI();
	sStates[4] = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Clamp,AM_Wrap>::GetRHI();
	sStates[5] = TStaticSamplerState<SF_Bilinear,AM_Mirror,AM_Clamp,AM_Wrap>::GetRHI();
	sStates[6] = TStaticSamplerState<SF_Bilinear,AM_Wrap,AM_Mirror,AM_Wrap>::GetRHI();
	sStates[7] = TStaticSamplerState<SF_Bilinear,AM_Clamp,AM_Mirror,AM_Wrap>::GetRHI();
	sStates[8] = TStaticSamplerState<SF_Bilinear,AM_Mirror,AM_Mirror,AM_Wrap>::GetRHI();

	{
		SCOPED_DRAW_EVENT(EventInputs)(DEC_SCENE_ITEMS,TEXT("ArkPpNodeMAterialInputs"));
		for (INT InputIndex = 0; InputIndex < m_InputProxy.Num() && InputIndex < ARK_PP_MATERIAL_NUM_TEXTURES; InputIndex++)
		{
			FAnEntry& Entry = m_InputProxy(InputIndex);
			if (Entry.m_pNode)
			{
				bDirty |= Entry.m_pNode->Render(Scene,View,FArkPpRenderConfig(FALSE));
				LArkPPSamplers[InputIndex].TextureRHI = Entry.m_pNode->GetTexture(View);
				LArkPPSamplers[InputIndex].SamplerStateRHI = sStates[Min<UINT>(Entry.m_SState,8)];
			}
			else
			{
				LArkPPSamplers[InputIndex] = FTexture();
			}
		}
	}

	{
		SCOPED_DRAW_EVENT(EventTarget)(DEC_SCENE_ITEMS,TEXT("ArkPpNodeMaterialTarget"));
		bDirty |= m_TargetProxy->Render(Scene,View,FArkPpRenderConfig(FALSE));
	}

	const UINT SizeX = m_TargetProxy->GetSurfaceSizeX();
	const UINT SizeY = m_TargetProxy->GetSurfaceSizeY();

	SCOPED_DRAW_EVENT(EventMaterial)(DEC_SCENE_ITEMS,TEXT("ArkPpNodeMaterial"));

	// the last node of the graph draws into the destination, everything else into its own target
	FSurfaceRHIRef iSurface;
	UBOOL bToDestination = FALSE;
	if (!Config.m_bForceToDestination || GSystemSettings.NeedsUpscale())
	{
		iSurface = GetSurface(View);
	}
	else
	{
		iSurface = GSceneRenderTargets.GetBackBuffer();
		bToDestination = TRUE;
	}
	RHISetRenderTarget(iSurface,FSurfaceRHIRef());
	ArkPpSetNodeViewport(View,SizeX,SizeY);

	if (m_pController)
	{
		m_pController->Render(EPpRs_BeforeDraw,Scene,&View);
	}

	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetColorWriteMask(m_TargetMask);
	RHISetBlendState(TStaticBlendState<>::GetRHI());

	FPpMaterialContextType DrawContext;
	for (INT TextureIndex = 0; TextureIndex < ARK_PP_MATERIAL_NUM_TEXTURES; TextureIndex++)
	{
		DrawContext.mSamplers[TextureIndex] = LArkPPSamplers[TextureIndex];
	}
	DrawContext.m_SurfaceX = SizeX;
	DrawContext.m_SurfaceY = SizeY;
	DrawContext.m_NeedGammaOutput = FALSE;

	// DISHONORED(bringup): -arkppdbg reports the first node pass, -arkppclear clears the destination to red just
	// before the tile, which says whether the surface this pass binds is the one the frame is presented from.
	static UBOOL bArkPpDbg = ParseParam(appCmdLine(),TEXT("arkppdbg"));
	static UBOOL bArkPpClear = ParseParam(appCmdLine(),TEXT("arkppclear"));
	if (bArkPpClear)
	{
		RHIClear(TRUE,FLinearColor(1.0f,0.0f,0.0f,1.0f),FALSE,0.0f,FALSE,0);
	}
	if (bArkPpDbg)
	{
		static INT NumReported = 0;
		if (NumReported < 8)
		{
			NumReported++;
			FString Inputs;
			for (INT InputIndex = 0; InputIndex < m_InputProxy.Num(); InputIndex++)
			{
				Inputs += FString::Printf(TEXT(" [%i proxy %s tex %s sampler %i]"),InputIndex,
					m_InputProxy(InputIndex).m_pNode ? TEXT("yes") : TEXT("no"),
					LArkPPSamplers[InputIndex].TextureRHI ? TEXT("yes") : TEXT("no"),
					m_InputProxy(InputIndex).m_SState);
			}
			debugf(TEXT("DISHONORED(bringup): FArkPp material pass: material %s, surface %ix%i, destination %s, back buffer %s, mask %i, %i inputs%s"),
				m_MatProxy->GetMaterial() ? *m_MatProxy->GetMaterial()->GetFriendlyName() : TEXT("none"),
				SizeX, SizeY, bToDestination ? TEXT("yes") : TEXT("no"),
				(iSurface == GSceneRenderTargets.GetBackBuffer()) ? TEXT("same") : TEXT("other"),
				(INT)m_TargetMask, m_InputProxy.Num(), *Inputs);
		}
	}

	FTileRenderer TileRenderer;
	TileRenderer.DrawTile<TPpMaterialDrawingPolicyFactory>(View,m_MatProxy,DrawContext);
	GDisCensusArkPpDraws++;

	// DISHONORED(bringup): retail ORs m_GoToLR in here and its constructor never writes that member: 2013 rva
	// 0x518d00 stores every other one and leaves offset 16 as appMalloc handed it over, so what retail reads is
	// uninitialised heap. The intent is plain in FSceneRenderer::FinishRenderViewTarget (2013 rva 0x45c110): it
	// copies scene colour over the view's render target unless bUseLDRSceneColor is set - and that render target is
	// the very back buffer this node has just drawn into, so with the bit clear the graph's output is overwritten by
	// the un-post-processed scene. The bit is set from the surface this node actually drew into instead, which is
	// what retail gets in practice out of non-zero garbage.
	View.bUseLDRSceneColor |= m_GoToLR || bToDestination;
	if (m_pController)
	{
		m_pController->Render(EPpRs_AfterDraw,Scene,&View);
	}
	RHISetColorWriteMask(m_TargetMask);
	RHICopyToResolveTarget(iSurface,TRUE,FResolveParams());
	if (m_pController)
	{
		m_pController->Render(EPpRs_AfterAll,Scene,&View);
	}
	return bDirty;
}

/** DISHONORED(port): 2013 rva 0x5248a0 (2012 0x5657a0) - a material node needs its surface, a material that is a post-process one, and
    every one of its inputs. */
UBOOL UArkPpNodeMaterial::IsValid(FArkPpIsValidData& Cache)
{
	UBOOL* Memo = Cache.mIsValidCache.Find(this);
	if (Memo)
	{
		return *Memo;
	}
	UBOOL bValid = m_SurfaceTarget && m_SurfaceTarget->IsValid(Cache);
	// the material has to be usable as a post-process material (retail reads the bit at FMaterial @752, mask 0x40000000)
	UMaterial* BaseMaterial = m_Material ? m_Material->GetMaterial() : NULL;
	bValid = bValid && BaseMaterial && BaseMaterial->bUsedWithArkPostProcess;
	for (UINT InputIndex = 1; bValid && InputIndex < NumInputs(); InputIndex++)
	{
		UArkPpNode* Input = GetInput(InputIndex);
		bValid = bValid && Input && Input->IsValid(Cache);
	}
	Cache.mIsValidCache.Set(this,bValid);
	return bValid;
}

/** DISHONORED(port): 2013 rva 0x5249c0 (2012 0x5658c0) - a hidden node is not a node: the graph goes on with its surface. */
FArkPpNodeProxy* UArkPpNodeMaterial::CreateSceneProxy(FArkPpCreateProxyConfig& Config)
{
	FArkPpNodeProxy* Cached = NULL;
	if (ArkPpFindCachedProxy(Config,this,Cached))
	{
		return Cached;
	}
	if (!IsShownInConfig(Config))
	{
		GDisCensusArkPpSkipped++;
		return m_SurfaceTarget ? m_SurfaceTarget->CreateSceneProxy(Config) : NULL;
	}
	FArkPpNodeProxy* Proxy = new FArkPpNodeMaterialProxy(Config,this);
	Config.mNodeCache.Set(this,Proxy);
	return Proxy;
}

/**
 * DISHONORED(bringup): see DishonoredLinkArkPartMeshShaderTypes - the static library drops an unreferenced object file, and
 * with it these shader type registrations. The FArkPp graph references this unit now, so the anchor only has to stay until
 * the link-anchor array in FogRendering.cpp goes.
 */
void DishonoredLinkArkPpMaterialShaderTypes()
{
}
