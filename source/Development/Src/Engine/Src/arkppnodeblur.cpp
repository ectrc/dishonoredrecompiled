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

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "SceneFilterRendering.h"
#include "arkpp.h"

/** DISHONORED(bringup, agent EE): the motion-blur node's share of the post-process census. */
INT GDisCensusArkPpBlurDraws = 0;

/**
 * DISHONORED(port): the shader types of Arkane's post-process blur node (UArkPpNodeBlur / FArkPpNodeBlurProxy).
 * One vertex and one pixel shader templated on the kind of blur; three cooked vertex shaders and five cooked pixel
 * shaders of the FArkPp family come from these two templates (2013 rva 0xb82c30 ff., source ArkPpBlur, entry points
 * MainVS / MainPS, 786 / 1). The node graph that drives them is not ported yet.
 */
struct BoxBlurPolicy {};
struct MotionBlurPolicy {};
struct MotionBlur2Policy {};
struct MotionBlur2NoOffsPolicy {};
struct RadialBlurPolicy {};

/**
 * DISHONORED(layout): three parameters (2013 rva 0x50bb50 Serialize; SetParameters 0x514860 writes the first two:
 * (1/width, 1/height) and the source rectangle's scale and bias). The cooked records carry 9 history words.
 */
template<typename BlurPolicyType>
class TArkPpBlurVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TArkPpBlurVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TArkPpBlurVertexShader() {}

	TArkPpBlurVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		// DISHONORED(bringup): retail binds nothing (the shipping exes have no shader compiler); the cooked record
		// carries the register indices, so these names only have to be stable.
		m_OORTSizeParameter.Bind(Initializer.ParameterMap,TEXT("OORTSize"),TRUE);
		m_ViewportScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("ViewportScaleBias"),TRUE);
		m_NoiseParameter.Bind(Initializer.ParameterMap,TEXT("NoiseParams"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << m_OORTSizeParameter;
		Ar << m_ViewportScaleBiasParameter;
		Ar << m_NoiseParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter m_OORTSizeParameter;
	FShaderParameter m_ViewportScaleBiasParameter;
	FShaderParameter m_NoiseParameter;
};

/**
 * DISHONORED(layout): three parameters (2013 rva 0x50b490 Serialize, 0x514940 SetParameters): the source colour, the
 * vector field the blur follows, and the scales applied to it.
 */
template<typename BlurPolicyType>
class TArkPpBlurPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TArkPpBlurPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TArkPpBlurPixelShader() {}

	TArkPpBlurPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		m_SrcTextureParameter.Bind(Initializer.ParameterMap,TEXT("SrcTexture"),TRUE);
		m_VectorFieldTextureParameter.Bind(Initializer.ParameterMap,TEXT("VectorFieldTexture"),TRUE);
		m_VectorFieldScales.Bind(Initializer.ParameterMap,TEXT("VectorFieldScales"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << m_SrcTextureParameter;
		Ar << m_VectorFieldTextureParameter;
		Ar << m_VectorFieldScales;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderResourceParameter m_SrcTextureParameter;
	FShaderResourceParameter m_VectorFieldTextureParameter;
	FShaderParameter m_VectorFieldScales;
};

// DISHONORED(retail): the cooked names of the eight blur types (2013 rva 0xb82c30 .. 0xb82df0).
typedef TArkPpBlurVertexShader<BoxBlurPolicy> FArkPpBlurBoxBlurVertexShaderType;
typedef TArkPpBlurVertexShader<MotionBlurPolicy> FArkPpMotionBlurVertexShaderType;
typedef TArkPpBlurVertexShader<RadialBlurPolicy> FArkPpRadialBlurVertexShaderType;
typedef TArkPpBlurPixelShader<BoxBlurPolicy> FArkPpBlurBoxBlurPixelShaderType;
typedef TArkPpBlurPixelShader<MotionBlurPolicy> FArkPpMotionBlurPixelShaderType;
typedef TArkPpBlurPixelShader<MotionBlur2Policy> FArkPpMotionBlur2PixelShaderType;
typedef TArkPpBlurPixelShader<MotionBlur2NoOffsPolicy> FArkPpMotionBlur2NoOffsPixelShaderType;
typedef TArkPpBlurPixelShader<RadialBlurPolicy> FArkPpRadialBlurPixelShaderType;

IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpBlurBoxBlurVertexShaderType,TEXT("FArkPpBlurBoxBlurVertexShader"),TEXT("ArkPpBlur"),TEXT("MainVS"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpMotionBlurVertexShaderType,TEXT("FArkPpMotionBlurVertexShader"),TEXT("ArkPpBlur"),TEXT("MainVS"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpRadialBlurVertexShaderType,TEXT("FArkPpRadialBlurVertexShader"),TEXT("ArkPpBlur"),TEXT("MainVS"),SF_Vertex,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpBlurBoxBlurPixelShaderType,TEXT("FArkPpBlurBoxBlurPixelShader"),TEXT("ArkPpBlur"),TEXT("MainPS"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpMotionBlurPixelShaderType,TEXT("FArkPpMotionBlurPixelShader"),TEXT("ArkPpBlur"),TEXT("MainPS"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpMotionBlur2PixelShaderType,TEXT("FArkPpMotionBlur2PixelShader"),TEXT("ArkPpBlur"),TEXT("MainPS"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpMotionBlur2NoOffsPixelShaderType,TEXT("FArkPpMotionBlur2NoOffsPixelShader"),TEXT("ArkPpBlur"),TEXT("MainPS"),SF_Pixel,786,1);
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FArkPpRadialBlurPixelShaderType,TEXT("FArkPpRadialBlurPixelShader"),TEXT("ArkPpBlur"),TEXT("MainPS"),SF_Pixel,786,1);

/*-----------------------------------------------------------------------------
	UArkPpNodeBlur / FArkPpNodeBlurProxy (2012 PDB 60 bytes; ctor 0x54ea90, Render 0x54ec70, RenderMotionBlur 0x5626c0)
-----------------------------------------------------------------------------*/

/**
 * DISHONORED(port): the blur node's proxy: the graph shape, the configuration and the surface delegation are retail's.
 * DISHONORED(bringup): the blur pass itself is not ported - see Render.
 */
class FArkPpNodeBlurProxy : public FArkPpNodeProxy
{
public:
	/** DISHONORED(port): 2013 rva 0x50e350 (2012 0x54ea90) - an output node is optional and then the input's proxy is the output too. */
	FArkPpNodeBlurProxy(FArkPpCreateProxyConfig& Config,UArkPpNodeBlur* InNode)
		: mType(InNode->m_Type)
		, mBoxBlur(InNode->m_BoxBlurConfig)
		, mMotionBlur(InNode->m_MotionBlurConfig)
		, mRadialBlur(InNode->m_RadialConfig)
	{
		if (InNode->m_bOverrideUberPp)
		{
			Config.PushUberOverride(InNode->m_UberParameters,InNode->m_UberParametersWeight);
		}
		if (InNode->m_VectorField)
		{
			m_VectorFieldProxy = InNode->m_VectorField->CreateSceneProxy(Config);
		}
		m_InProxy = InNode->m_Input ? InNode->m_Input->CreateSceneProxy(Config) : NULL;
		m_OutProxy = InNode->m_Output ? InNode->m_Output->CreateSceneProxy(Config) : m_InProxy.GetReference();
	}

	/**
	 * DISHONORED(port): 2013 rva 0x50e560 (2012 0x54ec70) - retail renders the input with m_bForceToDestination cleared and then draws
	 * the blur (RenderMotionBlur, 0x5626c0) into the destination with TArkPpBlur{Vertex,Pixel}Shader<Policy>.
	 * DISHONORED(bringup): the blur pass is not ported. The input is rendered with the *unchanged* config instead, so a
	 * blur node that ends the graph still puts its input on the destination surface rather than nothing: the image is
	 * the unblurred one. The shader types load (agentBD.md 1) and the pass needs TArkPpBlurVertexShader::SetParameters
	 * (0x554e90) and TArkPpBlurPixelShader::SetParameters (0x554f70) with FArkPpBlurParameters (2012 PDB 176 bytes).
	 */
	virtual UBOOL Render(const FScene* Scene,FViewInfo& View,FArkPpRenderConfig Config)
	{
		if (m_bDone)
		{
			return FALSE;
		}
		m_bDone = TRUE;
		GDisCensusArkPpSkipped++;
		return m_InProxy ? m_InProxy->Render(Scene,View,Config) : FALSE;
	}

	/** DISHONORED(port): 2013 rva 0x50b2f0 (2012 0x54bb80) / 0x54f0f0 - everything is the input's. */
	virtual const FSurfaceRHIRef GetSurface(const FViewInfo& View) { return m_InProxy ? m_InProxy->GetSurface(View) : FSurfaceRHIRef(); }
	virtual const FTexture2DRHIRef GetTexture(const FViewInfo& View) { return m_InProxy ? m_InProxy->GetTexture(View) : FTexture2DRHIRef(); }
	virtual UINT GetSurfaceSizeX() { return m_InProxy ? m_InProxy->GetSurfaceSizeX() : 0; }
	virtual UINT GetSurfaceSizeY() { return m_InProxy ? m_InProxy->GetSurfaceSizeY() : 0; }

private:
	TRefCountPtr<FArkPpNodeProxy> m_InProxy;
	TRefCountPtr<FArkPpNodeProxy> m_OutProxy;
	TRefCountPtr<FArkPpNodeProxy> m_VectorFieldProxy;
	BYTE mType;
	FBoxBlurConfig mBoxBlur;
	FMotionBlurConfig mMotionBlur;
	FRadialBlurConfig mRadialBlur;
};

/** DISHONORED(port): 2013 rva 0x524390 (2012 0x565290) - the input, and the vector field when this is a motion blur. */
UBOOL UArkPpNodeBlur::IsValid(FArkPpIsValidData& Cache)
{
	UBOOL* Memo = Cache.mIsValidCache.Find(this);
	if (Memo)
	{
		return *Memo;
	}
	const UBOOL bVectorFieldOk = (m_Type != EPpBt_Motion) || (m_VectorField && m_VectorField->IsValid(Cache));
	const UBOOL bValid = m_Input && m_Input->IsValid(Cache)
		&& (!m_Output || m_Output->IsValid(Cache))
		&& (!m_VectorField || m_VectorField->IsValid(Cache))
		&& bVectorFieldOk;
	Cache.mIsValidCache.Set(this,bValid);
	return bValid;
}

/** DISHONORED(port): 2013 rva 0x524490 (2012 0x565390) - a hidden blur node is its input. */
FArkPpNodeProxy* UArkPpNodeBlur::CreateSceneProxy(FArkPpCreateProxyConfig& Config)
{
	FArkPpNodeProxy* Cached = NULL;
	if (ArkPpFindCachedProxy(Config,this,Cached))
	{
		return Cached;
	}
	if (!IsShownInConfig(Config))
	{
		return m_Input ? m_Input->CreateSceneProxy(Config) : NULL;
	}
	FArkPpNodeProxy* Proxy = new FArkPpNodeBlurProxy(Config,this);
	Config.mNodeCache.Set(this,Proxy);
	return Proxy;
}

/** DISHONORED(bringup): the link anchor of this unit - see DishonoredLinkArkPartMeshShaderTypes. */
void DishonoredLinkArkPpBlurShaderTypes()
{
}
