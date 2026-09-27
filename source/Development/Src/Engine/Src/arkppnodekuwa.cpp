// Engine/src/arkppnodekuwa.cpp
// Stub created by resources/tools/import_reference.py: this file exists in Dishonored's
// build but not in the reference engine tree. Rewrite it from the decompile (Phase 3).
// PDB functions attributed to this file (24):
//   0x549e80  public: virtual unsigned int __thiscall UArkPpNodeKuwa::LinkInput(unsigned int, class UArkPpNode *)
//   0x549eb0  public: virtual unsigned int __thiscall UArkPpNodeKuwa::UnlinkInput(class UArkPpNode *)
//   0x549ee0  public: virtual unsigned int __thiscall UArkPpNodeKuwa::UnlinkInput(unsigned int)
//   0x54bac0  public: __thiscall FArkPpNodeKuwaProxy::FArkPpNodeKuwaProxy(struct FArkPpCreateProxyConfig &, class UArkPpNodeKuwa *)
//   0x54bb80  public: virtual class TDynamicRHIResourceReference<12> const __thiscall FArkPpNodeBlurProxy::GetSurface(class FViewInfo const &)
//   0x54bbb0  public: virtual __thiscall FArkPpNodeKuwaProxy::~FArkPpNodeKuwaProxy(void)
//   0x54dab0  public: void __thiscall TKuwaPixelShader<3>::SetParameters(struct FArkPpKuwaParameters const &)
//   0x54e890  public: static class FShader * __cdecl FMLAAVertexShader::ConstructSerializedInstance(void)
//   0x54f0f0  public: virtual class TDynamicRHIResourceReference<14> const __thiscall FArkPpNodeBlurProxy::GetTexture(class FViewInfo const &)
//   0x54f120  public: virtual unsigned int __thiscall FArkPpNodeKuwaProxy::GetSurfaceSizeX(void)
//   0x54f130  public: virtual unsigned int __thiscall FArkPpNodeKuwaProxy::GetSurfaceSizeY(void)
//   0x54f8e0  public: virtual class FString __thiscall UArkPpNodeKuwa::InputName(unsigned int)const
//   0x5555e0  public: void __thiscall TKuwaVertexShader<3>::SetParameters(struct FArkPpKuwaParameters const &)
//   0x55d7d0  public: static class UClass * __cdecl UArkPpNodeKuwa::GetPrivateStaticClassUArkPpNodeKuwa(wchar_t const *)
//   0x55f3d0  public: static void __cdecl UArkPpNodeKuwa::InitializePrivateStaticClassUArkPpNodeKuwa(void)
//   0x564120  public: static class UClass * __cdecl UArkPpNodeKuwa::StaticClassNoInline(void)
//   0x564150  public: unsigned int __thiscall FArkPpNodeKuwaProxy::RenderKuwa(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x5646b0  public: virtual unsigned int __thiscall FArkPpNodeKuwaProxy::Render(class FScene const *, class FViewInfo &, struct FArkPpRenderConfig)
//   0x565610  public: virtual unsigned int __thiscall UArkPpNodeKuwa::IsValid(struct FArkPpIsValidData &)
//   0x5656c0  public: virtual class FArkPpNodeProxy * __thiscall UArkPpNodeKuwa::CreateSceneProxy(struct FArkPpCreateProxyConfig &)
//   0xb9f7d0  _dynamic_initializer_for__TKuwaVertexShader_5_::StaticType__
//   0xb9f810  _dynamic_initializer_for__TKuwaPixelShader_5_::StaticType__
//   0xb9f850  _dynamic_initializer_for__TKuwaVertexShader_3_::StaticType__
//   0xb9f890  _dynamic_initializer_for__TKuwaPixelShader_3_::StaticType__

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"
#include "arkpp.h"

/**
 * DISHONORED(port): the shader types of Arkane's Kuwahara painterly filter node (UArkPpNodeKuwa /
 * FArkPpNodeKuwaProxy). The template argument is the kernel radius, 3 or 5 (2013 rva 0xb83050 ff., source ArkKuwa,
 * entry points MainVS / MainPS, 793 / 26).
 */

/** DISHONORED(layout): two parameters (2013 rva 0x5155e0 SetParameters: (1/w, 1/h, strength) and the viewport scale/bias). */
template<UINT KernelRadius>
class TKuwaVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TKuwaVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TKuwaVertexShader() {}

	TKuwaVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		m_OORTSizeParameter.Bind(Initializer.ParameterMap,TEXT("OORTSize"),TRUE);
		m_ViewportScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("ViewportScaleBias"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << m_OORTSizeParameter;
		Ar << m_ViewportScaleBiasParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter m_OORTSizeParameter;
	FShaderParameter m_ViewportScaleBiasParameter;
};

/** DISHONORED(layout): one parameter, the source colour (2013 rva 0x50dab0 SetParameters). */
template<UINT KernelRadius>
class TKuwaPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TKuwaPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform) { return TRUE; }

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TKuwaPixelShader() {}

	TKuwaPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		m_SrcColor.Bind(Initializer.ParameterMap,TEXT("SrcColor"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << m_SrcColor;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter m_SrcColor;
};

// DISHONORED(retail): 2013 rva 0xb83050 .. 0xb83110 ("ArkKuwa", 793 / 26).
typedef TKuwaVertexShader<5> FKuwaVertexShader5Type;
typedef TKuwaPixelShader<5> FKuwaPixelShader5Type;
typedef TKuwaVertexShader<3> FKuwaVertexShader3Type;
typedef TKuwaPixelShader<3> FKuwaPixelShader3Type;
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FKuwaVertexShader5Type,TEXT("FKuwaVertexShader5"),TEXT("ArkKuwa"),TEXT("MainVS"),SF_Vertex,793,26);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FKuwaPixelShader5Type,TEXT("FKuwaPixelShader5"),TEXT("ArkKuwa"),TEXT("MainPS"),SF_Pixel,793,26);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FKuwaVertexShader3Type,TEXT("FKuwaVertexShader3"),TEXT("ArkKuwa"),TEXT("MainVS"),SF_Vertex,793,26);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FKuwaPixelShader3Type,TEXT("FKuwaPixelShader3"),TEXT("ArkKuwa"),TEXT("MainPS"),SF_Pixel,793,26);

/*-----------------------------------------------------------------------------
	UArkPpNodeKuwa / FArkPpNodeKuwaProxy (2012 PDB 28 bytes; ctor 0x54bac0, Render 0x5646b0, RenderKuwa 0x564150)
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): the Kuwahara filter node's proxy: the graph shape and the surface delegation are retail's.
 * DISHONORED(bringup): the filter pass itself is not ported - see Render.
 */
class FArkPpNodeKuwaProxy : public FArkPpNodeProxy
{
public:
	/** DISHONORED(port): 2013 rva 0x50b230 (2012 0x54bac0). */
	FArkPpNodeKuwaProxy(FArkPpCreateProxyConfig& Config,UArkPpNodeKuwa* InNode)
		: m_Type(InNode->m_Type)
		, mStrength(InNode->m_Strength)
	{
		m_TargetProxy = InNode->m_SurfaceTarget ? InNode->m_SurfaceTarget->CreateSceneProxy(Config) : NULL;
		m_InProxy = InNode->m_SrcColor ? InNode->m_SrcColor->CreateSceneProxy(Config) : NULL;
	}

	/**
	 * DISHONORED(port): 2013 rva 0x5238c0 (2012 0x5646b0) - retail renders the source colour and the target with m_bForceToDestination
	 * cleared and then draws the filter (RenderKuwa, 0x564150) with TKuwa{Vertex,Pixel}Shader<3|5>.
	 * DISHONORED(bringup): the filter pass is not ported. The source colour is rendered with the unchanged config, so
	 * the image still reaches the destination - unfiltered. The four shader types load (agentBD.md 1); the pass needs
	 * TKuwaVertexShader::SetParameters (0x5555e0) and TKuwaPixelShader::SetParameters (0x54dab0) with
	 * FArkPpKuwaParameters (2012 PDB 112 bytes).
	 */
	virtual UBOOL Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
	{
		if (m_bDone)
		{
			return FALSE;
		}
		m_bDone = TRUE;
		GDisCensusArkPpSkipped++;
		UBOOL bDirty = m_InProxy ? m_InProxy->Render(Scene,View,Config) : FALSE;
		if (m_TargetProxy)
		{
			bDirty |= m_TargetProxy->Render(Scene,View,FArkPpRenderConfig(FALSE));
		}
		return bDirty;
	}

	/** DISHONORED(port): 2013 rva 0x50e9d0 (2012 0x54f120) / 0x54f130 - everything is the source colour's. */
	virtual const FSurfaceRHIRef GetSurface(const FViewInfo& View) { return m_InProxy ? m_InProxy->GetSurface(View) : FSurfaceRHIRef(); }
	virtual const FTexture2DRHIRef GetTexture(const FViewInfo& View) { return m_InProxy ? m_InProxy->GetTexture(View) : FTexture2DRHIRef(); }
	virtual UINT GetSurfaceSizeX() { return m_InProxy ? m_InProxy->GetSurfaceSizeX() : 0; }
	virtual UINT GetSurfaceSizeY() { return m_InProxy ? m_InProxy->GetSurfaceSizeY() : 0; }

private:
	TRefCountPtr<FArkPpNodeProxy> m_InProxy;
	TRefCountPtr<FArkPpNodeProxy> m_TargetProxy;
	INT m_Type;
	FLOAT mStrength;
};

/** DISHONORED(port): 2013 rva 0x524710 (2012 0x565610). */
UBOOL UArkPpNodeKuwa::IsValid(FArkPpIsValidData& Cache)
{
	UBOOL* Memo = Cache.mIsValidCache.Find(this);
	if (Memo)
	{
		return *Memo;
	}
	const UBOOL bValid = m_SurfaceTarget && m_SurfaceTarget->IsValid(Cache) && m_SrcColor && m_SrcColor->IsValid(Cache);
	Cache.mIsValidCache.Set(this,bValid);
	return bValid;
}

/** DISHONORED(port): 2013 rva 0x5247c0 (2012 0x5656c0) - a hidden node, or one with no kernel, is its surface target. */
FArkPpNodeProxy* UArkPpNodeKuwa::CreateSceneProxy(FArkPpCreateProxyConfig& Config)
{
	if (!IsShownInConfig(Config) || m_Type <= 0)
	{
		return m_SurfaceTarget ? m_SurfaceTarget->CreateSceneProxy(Config) : NULL;
	}
	FArkPpNodeProxy* Cached = NULL;
	if (ArkPpFindCachedProxy(Config,this,Cached))
	{
		return Cached;
	}
	FArkPpNodeProxy* Proxy = new FArkPpNodeKuwaProxy(Config,this);
	Config.mNodeCache.Set(this,Proxy);
	return Proxy;
}

/** DISHONORED(bringup): the link anchor of this unit - see DishonoredLinkArkPartMeshShaderTypes. */
void DishonoredLinkArkPpKuwaShaderTypes()
{
}
