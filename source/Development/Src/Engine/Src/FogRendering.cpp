/*=============================================================================
	FogRendering.cpp: Fog rendering implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "ScenePrivate.h"
#include "AmbientOcclusionRendering.h"
#include "SceneFilterRendering.h"
#include "arkcommonvertexdeclaration.h"

/** Binds the parameters. */
void FExponentialHeightFogShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	ExponentialFogParameters.Bind(ParameterMap,TEXT("SharedFogParameter0"), TRUE);
	ExponentialFogColorParameter.Bind(ParameterMap,TEXT("SharedFogParameter1"), TRUE);
	LightInscatteringColorParameter.Bind(ParameterMap,TEXT("SharedFogParameter2"), TRUE);
	LightVectorParameter.Bind(ParameterMap,TEXT("SharedFogParameter3"), TRUE);
}

/** Serializer. */
FArchive& operator<<(FArchive& Ar,FExponentialHeightFogShaderParameters& Parameters)
{
	Ar << Parameters.ExponentialFogParameters;
	Ar << Parameters.ExponentialFogColorParameter;
	Ar << Parameters.LightInscatteringColorParameter;
	Ar << Parameters.LightVectorParameter;
	return Ar;
}

/** Binds the parameters. */
void FHeightFogShaderParameters::Bind(const FShaderParameterMap& ParameterMap)
{
	bUseExponentialHeightFogParameter.Bind(ParameterMap,TEXT("bUseExponentialHeightFog"), TRUE);
	FogMinHeightParameter.Bind(ParameterMap,TEXT("SharedFogParameter3"), TRUE);
	FogMaxHeightParameter.Bind(ParameterMap,TEXT("FogMaxHeight"), TRUE);
	FogDistanceScaleParameter.Bind(ParameterMap,TEXT("SharedFogParameter0"), TRUE);
	FogExtinctionDistanceParameter.Bind(ParameterMap,TEXT("SharedFogParameter1"), TRUE);
	FogInScatteringParameter.Bind(ParameterMap,TEXT("FogInScattering"), TRUE);
	FogStartDistanceParameter.Bind(ParameterMap,TEXT("SharedFogParameter2"), TRUE);
	ExponentialParameters.Bind(ParameterMap);
}

static const FLOAT DefaultFogParameters[4] = { 0.f, 0.f, 0.f, 0.f };
static const FLOAT DefaultFogExtinctionDistance[4] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
static const FLinearColor DefaultFogInScattering[4] = { FLinearColor::Black, FLinearColor::Black, FLinearColor::Black, FLinearColor::Black };

/** 
* Sets the parameter values, this must be called before rendering the primitive with the shader applied. 
* @param Shader - the shader to set the parameters on
*/
template<typename ShaderRHIParamRef>
void FHeightFogShaderParameters::Set(
	const FVertexFactory* VertexFactory, 
	const FMaterialRenderProxy* MaterialRenderProxy, 
	const FMaterial& Material,
	const FSceneView* View, 
	const UBOOL bAllowGlobalFog, 
	ShaderRHIParamRef Shader) const
{
	const FViewInfo* ViewInfo = static_cast<const FViewInfo*>(View);

	// Set the fog constants.
	//@todo - translucent decals on translucent receivers currently don't handle fog
	if ( bAllowGlobalFog && ( Material.AllowsFog() && !(VertexFactory->IsDecalFactory() && VertexFactory->IsGPUSkinned()) ))
	{
		SetShaderValue(Shader, bUseExponentialHeightFogParameter, ViewInfo->bRenderExponentialFog ? 1.0f : 0.0f);
		
		if (ViewInfo->bRenderExponentialFog)
		{
			SetShaderValue(Shader, ExponentialParameters.ExponentialFogParameters, ViewInfo->ExponentialFogParameters);
			SetShaderValue(Shader, ExponentialParameters.ExponentialFogColorParameter, FVector4(ViewInfo->ExponentialFogColor, 1.0f - ViewInfo->FogMaxOpacity));
			SetShaderValue(Shader, ExponentialParameters.LightInscatteringColorParameter, ViewInfo->LightInscatteringColor);
			SetShaderValue(Shader, ExponentialParameters.LightVectorParameter, ViewInfo->DominantDirectionalLightDirection);
		}
		else
		{
			TStaticArray<FLOAT,4> TranslatedMinHeight;
			TStaticArray<FLOAT,4> TranslatedMaxHeight;
			for(UINT LayerIndex = 0;LayerIndex < 4;++LayerIndex)
			{
				TranslatedMinHeight[LayerIndex] = ViewInfo->HeightFogParams.FogMinHeight[LayerIndex] + View->PreViewTranslation.Z;
				TranslatedMaxHeight[LayerIndex] = ViewInfo->HeightFogParams.FogMaxHeight[LayerIndex] + View->PreViewTranslation.Z;
			}

			SetShaderValue(Shader,FogMinHeightParameter,TranslatedMinHeight);
			SetShaderValue(Shader,FogMaxHeightParameter,TranslatedMaxHeight);
			SetShaderValue(Shader,FogInScatteringParameter,ViewInfo->HeightFogParams.FogInScattering);
			SetShaderValue(Shader,FogDistanceScaleParameter,ViewInfo->HeightFogParams.FogDistanceScale);
			SetShaderValue(Shader,FogExtinctionDistanceParameter,ViewInfo->HeightFogParams.FogExtinctionDistance);
			SetShaderValue(Shader,FogStartDistanceParameter,ViewInfo->HeightFogParams.FogStartDistance);
		}
	}
	else
	{
		// Set the default values which effectively disable vertex fog
		SetShaderValue(Shader,bUseExponentialHeightFogParameter, 0.0f);
		SetShaderValue(Shader,FogMinHeightParameter,DefaultFogParameters);
		SetShaderValue(Shader,FogMaxHeightParameter,DefaultFogParameters);
		SetShaderValue(Shader,FogInScatteringParameter,DefaultFogInScattering);
		SetShaderValue(Shader,FogDistanceScaleParameter,DefaultFogParameters);
		SetShaderValue(Shader,FogExtinctionDistanceParameter,DefaultFogExtinctionDistance);
		SetShaderValue(Shader,FogStartDistanceParameter,DefaultFogParameters);
	}
}

/** 
* Sets the parameter values, this must be called before rendering the primitive with the shader applied. 
* @param VertexShader - the vertex shader to set the parameters on
*/
void FHeightFogShaderParameters::SetVertexShader(
	const FVertexFactory* VertexFactory, 
	const FMaterialRenderProxy* MaterialRenderProxy,
	const FMaterial& Material,
	const FSceneView* View,
	const UBOOL bAllowGlobalFog,
	FShader* VertexShader) const
{
	Set(VertexFactory, MaterialRenderProxy,Material,View,bAllowGlobalFog,VertexShader->GetVertexShader());
}

#if WITH_D3D11_TESSELLATION
/** 
* Sets the parameter values, this must be called before rendering the primitive with the shader applied. 
* @param DomainShader - the vertex shader to set the parameters on
*/
void FHeightFogShaderParameters::SetDomainShader(
	const FVertexFactory* VertexFactory, 
	const FMaterialRenderProxy* MaterialRenderProxy,
	const FMaterial& Material,
	const FSceneView* View,
	FShader* DomainShader) const
{
	const UBOOL bAllowGlobalFog = FALSE;
	Set(VertexFactory, MaterialRenderProxy,Material,View,bAllowGlobalFog,DomainShader->GetDomainShader());
}
#endif

/** Serializer. */
FArchive& operator<<(FArchive& Ar,FHeightFogShaderParameters& Parameters)
{
	Ar << Parameters.bUseExponentialHeightFogParameter;
	Ar << Parameters.FogDistanceScaleParameter;
	Ar << Parameters.FogExtinctionDistanceParameter;
	Ar << Parameters.FogMinHeightParameter;
	Ar << Parameters.FogMaxHeightParameter;
	Ar << Parameters.FogInScatteringParameter;
	Ar << Parameters.FogStartDistanceParameter;
	Ar << Parameters.ExponentialParameters;
	return Ar;
}


/** A vertex shader for rendering height fog. */
template<UINT NumLayers>
class THeightFogVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(THeightFogVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	THeightFogVertexShader( )	{ }
	THeightFogVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		ScreenPositionScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("ScreenPositionScaleBias"));
		FogMinHeightParameter.Bind(Initializer.ParameterMap,TEXT("FogMinHeight"));
		FogMaxHeightParameter.Bind(Initializer.ParameterMap,TEXT("FogMaxHeight"));
		ScreenToWorldParameter.Bind(Initializer.ParameterMap,TEXT("ScreenToWorld"));
		FogStartZParameter.Bind(Initializer.ParameterMap,TEXT("FogStartZ"), TRUE);
	}

	void SetParameters(const FViewInfo& View)
	{
		// Set the transform from screen coordinates to scene texture coordinates.
		// NOTE: Need to set explicitly, since this is a vertex shader!
		SetVertexShaderValue(GetVertexShader(),ScreenPositionScaleBiasParameter,View.ScreenPositionScaleBias);

		// Set the fog constants.
		SetVertexShaderValue(GetVertexShader(),FogMinHeightParameter,View.HeightFogParams.FogMinHeight);
		SetVertexShaderValue(GetVertexShader(),FogMaxHeightParameter,View.HeightFogParams.FogMaxHeight);

		FMatrix ScreenToWorld = FMatrix(
			FPlane(1,0,0,0),
			FPlane(0,1,0,0),
			FPlane(0,0,(1.0f - Z_PRECISION),1),
			FPlane(0,0,-View.NearClippingDistance * (1.0f - Z_PRECISION),0)
			) *
			View.InvViewProjectionMatrix;

		// Set the view constants, as many as were bound to the parameter.
		SetVertexShaderValue(GetVertexShader(),ScreenToWorldParameter,ScreenToWorld);

		{
			// The fog can be set to start at a certain euclidean distance.
			// We clamp the value to be behind the near plane z.
			// (not the exact value as a distance that near would not allow any optimization anyway)
			FLOAT FogStartDistance = Max(30.0f, View.ExponentialFogParameters.W);

			// Here we compute the nearest z value the fog can start
			// to render the quad at this z value with depth test enabled.
			// This means with a bigger distance specified more pixels are
			// are culled and don't need to be rendered. This is faster if
			// there is opaque content nearer than the computed z.

			FVector ViewSpaceCorner = View.InvProjectionMatrix.TransformFVector4(FVector4(1, 1, 1, 1));

			FLOAT Ratio = ViewSpaceCorner.Z / ViewSpaceCorner.Size();

			FVector ViewSpaceStartFogPoint(0.0f, 0.0f, FogStartDistance * Ratio);
			FVector4 ClipSpaceMaxDistance = View.ProjectionMatrix.TransformFVector(ViewSpaceStartFogPoint);

			FLOAT FogClipSpaceZ = Max(0.0f, ClipSpaceMaxDistance.Z / ClipSpaceMaxDistance.W);

			SetVertexShaderValue(GetVertexShader(),FogStartZParameter, FogClipSpaceZ);
		}
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << ScreenPositionScaleBiasParameter;
		Ar << FogMinHeightParameter;
		Ar << FogMaxHeightParameter;
		Ar << ScreenToWorldParameter;
		Ar << FogStartZParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FShaderParameter ScreenPositionScaleBiasParameter;
	FShaderParameter FogMinHeightParameter;
	FShaderParameter FogMaxHeightParameter;
	FShaderParameter ScreenToWorldParameter;
	FShaderParameter FogStartZParameter;
};

IMPLEMENT_SHADER_TYPE(template<>,THeightFogVertexShader<1>,TEXT("HeightFogVertexShader"),TEXT("OneLayerMain"),SF_Vertex,0,0);
IMPLEMENT_SHADER_TYPE(template<>,THeightFogVertexShader<4>,TEXT("HeightFogVertexShader"),TEXT("FourLayerMain"),SF_Vertex,0,0);

enum EMSAAShaderFrequency
{
	MSAASF_NoMSAA = 0,
	MSAASF_PerFragment,
	MSAASF_PerPixel,
	MSAASF_Num
};

/** A pixel shader for rendering exponential height fog. */
template<EMSAAShaderFrequency MSAAShaderFrequency>
class TExponentialHeightFogPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(TExponentialHeightFogPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return MSAAShaderFrequency == MSAASF_NoMSAA || Platform == SP_PCD3D_SM5;
	}

	/**
	* Add any compiler flags/defines required by the shader
	* @param OutEnvironment - shader environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.Definitions.Set(TEXT("MSAA_ENABLED"),MSAAShaderFrequency != MSAASF_NoMSAA ? TEXT("1") : TEXT("0"));
		OutEnvironment.Definitions.Set(TEXT("PER_FRAGMENT"),MSAAShaderFrequency == MSAASF_PerFragment ? TEXT("1") : TEXT("0"));
	}

	TExponentialHeightFogPixelShader( )	{ }
	TExponentialHeightFogPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		SceneTextureParameters.Bind(Initializer.ParameterMap);
		CameraWorldPositionParameter.Bind(Initializer.ParameterMap,TEXT("CameraWorldPosition"), TRUE);
		ExponentialParameters.Bind(Initializer.ParameterMap);
	}

	void SetParameters(const FViewInfo& View)
	{
		SceneTextureParameters.Set(&View, this);

		SetPixelShaderValue(GetPixelShader(), CameraWorldPositionParameter, (FVector)View.ViewOrigin);

		SetPixelShaderValue(GetPixelShader(), ExponentialParameters.ExponentialFogParameters, View.ExponentialFogParameters);
		SetPixelShaderValue(GetPixelShader(), ExponentialParameters.ExponentialFogColorParameter, FVector4(View.ExponentialFogColor, 1.0f - View.FogMaxOpacity));
		SetPixelShaderValue(GetPixelShader(), ExponentialParameters.LightInscatteringColorParameter, View.LightInscatteringColor);
		SetPixelShaderValue(GetPixelShader(), ExponentialParameters.LightVectorParameter, View.DominantDirectionalLightDirection);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SceneTextureParameters;
		Ar << CameraWorldPositionParameter;
		Ar << ExponentialParameters;
		return bShaderHasOutdatedParameters;
	}

private:
	FSceneTextureShaderParameters SceneTextureParameters;
	FShaderParameter CameraWorldPositionParameter;
	FExponentialHeightFogShaderParameters ExponentialParameters;
};

IMPLEMENT_SHADER_TYPE(template<>,TExponentialHeightFogPixelShader<MSAASF_NoMSAA>,TEXT("HeightFogPixelShader"), TEXT("ExponentialPixelMain"),SF_Pixel,0,0)
IMPLEMENT_SHADER_TYPE(template<>,TExponentialHeightFogPixelShader<MSAASF_PerPixel>,TEXT("HeightFogPixelShader"),TEXT("ExponentialPixelMain"),SF_Pixel,0,0)
IMPLEMENT_SHADER_TYPE(template<>,TExponentialHeightFogPixelShader<MSAASF_PerFragment>,TEXT("HeightFogPixelShader"),TEXT("ExponentialPixelMain"),SF_Pixel,0,0)

/** A pixel shader for rendering height fog. */
template<UINT NumLayers,EMSAAShaderFrequency MSAAShaderFrequency>
class THeightFogPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(THeightFogPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		// Only compile the downsampled version (NumLayers == 0) for xbox
		return (NumLayers != 0 || Platform == SP_XBOXD3D) && (MSAAShaderFrequency == MSAASF_NoMSAA || Platform == SP_PCD3D_SM5);
	}

	/**
	* Add any compiler flags/defines required by the shader
	* @param OutEnvironment - shader environment to modify
	*/
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		//The HLSL compiler for xenon will not always use predicated instructions without this flag.  
		//On PC the compiler consistently makes the right decision.
		new(OutEnvironment.CompilerFlags) ECompilerFlags(CFLAG_PreferFlowControl);
		if( Platform == SP_XBOXD3D )
		{
			//The xenon compiler complains about the [ifAny] attribute
			new(OutEnvironment.CompilerFlags) ECompilerFlags(CFLAG_SkipValidation);
		}

		OutEnvironment.Definitions.Set(TEXT("MSAA_ENABLED"),MSAAShaderFrequency != MSAASF_NoMSAA ? TEXT("1") : TEXT("0"));
		OutEnvironment.Definitions.Set(TEXT("PER_FRAGMENT"),MSAAShaderFrequency == MSAASF_PerFragment ? TEXT("1") : TEXT("0"));
	}

	THeightFogPixelShader( )	{ }
	THeightFogPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		SceneTextureParameters.Bind(Initializer.ParameterMap);
		FogDistanceScaleParameter.Bind(Initializer.ParameterMap,TEXT("SharedFogParameter0"));
		FogExtinctionDistanceParameter.Bind(Initializer.ParameterMap,TEXT("SharedFogParameter1"));
		FogInScatteringParameter.Bind(Initializer.ParameterMap,TEXT("FogInScattering"), TRUE);
		FogStartDistanceParameter.Bind(Initializer.ParameterMap,TEXT("SharedFogParameter2"));
		FogMinStartDistanceParameter.Bind(Initializer.ParameterMap,TEXT("FogMinStartDistance"), TRUE);
		EncodePowerParameter.Bind(Initializer.ParameterMap,TEXT("EncodePower"), TRUE);
	}

	void SetParameters(const FViewInfo& View, INT NumSceneFogLayers)
	{
		check(NumSceneFogLayers > 0);
		SceneTextureParameters.Set( &View, this);

		// Set the fog constants.
		SetPixelShaderValue(GetPixelShader(),FogInScatteringParameter,View.HeightFogParams.FogInScattering);
		SetPixelShaderValue(GetPixelShader(),FogDistanceScaleParameter,View.HeightFogParams.FogDistanceScale);
		SetPixelShaderValue(GetPixelShader(),FogExtinctionDistanceParameter,View.HeightFogParams.FogExtinctionDistance);
		SetPixelShaderValue(GetPixelShader(),FogStartDistanceParameter,View.HeightFogParams.FogStartDistance);
		SetPixelShaderValue(GetPixelShader(),FogMinStartDistanceParameter,*MinElement(&View.HeightFogParams.FogStartDistance[0], (&View.HeightFogParams.FogStartDistance[0]) + NumSceneFogLayers));
		SetPixelShaderValue(GetPixelShader(),EncodePowerParameter, 1.0f);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << SceneTextureParameters;
		Ar << FogDistanceScaleParameter;
		Ar << FogExtinctionDistanceParameter;
		Ar << FogInScatteringParameter;
		Ar << FogStartDistanceParameter;
		Ar << FogMinStartDistanceParameter;
		Ar << EncodePowerParameter;
		return bShaderHasOutdatedParameters;
	}

private:
	FSceneTextureShaderParameters SceneTextureParameters;
	FShaderParameter FogDistanceScaleParameter;
	FShaderParameter FogExtinctionDistanceParameter;
	FShaderParameter FogInScatteringParameter;
	FShaderParameter FogStartDistanceParameter;
	FShaderParameter FogMinStartDistanceParameter;
	FShaderParameter EncodePowerParameter;
};

typedef THeightFogPixelShader<0,MSAASF_NoMSAA> FDownsampleDepthAndFogPixelShader;
IMPLEMENT_SHADER_TYPE(template<>,FDownsampleDepthAndFogPixelShader,TEXT("HeightFogPixelShader"),TEXT("DownsampleDepthAndFogMain"),SF_Pixel,0,0);

typedef THeightFogPixelShader<1,MSAASF_NoMSAA> FOneLayerFogPixelShader;
IMPLEMENT_SHADER_TYPE(template<>,FOneLayerFogPixelShader,TEXT("HeightFogPixelShader"),TEXT("OneLayerMain"),SF_Pixel,VER_HEIGHTFOG_PIXELSHADER_START_DIST_FIX,0);

typedef THeightFogPixelShader<4,MSAASF_NoMSAA> FFourLayerFogPixelShader;
IMPLEMENT_SHADER_TYPE(template<>,FFourLayerFogPixelShader,TEXT("HeightFogPixelShader"),TEXT("FourLayerMain"),SF_Pixel,VER_HEIGHTFOG_PIXELSHADER_START_DIST_FIX,0);

typedef THeightFogPixelShader<1,MSAASF_PerPixel> FPerPixelOneLayerFogPixelShader;
IMPLEMENT_SHADER_TYPE(template<>,FPerPixelOneLayerFogPixelShader,TEXT("HeightFogPixelShader"),TEXT("OneLayerMain"),SF_Pixel,VER_HEIGHTFOG_PIXELSHADER_START_DIST_FIX,0);

typedef THeightFogPixelShader<4,MSAASF_PerPixel> FPerPixelFourLayerFogPixelShader;
IMPLEMENT_SHADER_TYPE(template<>,FPerPixelFourLayerFogPixelShader,TEXT("HeightFogPixelShader"),TEXT("FourLayerMain"),SF_Pixel,VER_HEIGHTFOG_PIXELSHADER_START_DIST_FIX,0);

typedef THeightFogPixelShader<1,MSAASF_PerFragment>  FPerFragmentOneLayerFogPixelShader;
IMPLEMENT_SHADER_TYPE(template<>,FPerFragmentOneLayerFogPixelShader,TEXT("HeightFogPixelShader"),TEXT("OneLayerMain"),SF_Pixel,VER_HEIGHTFOG_PIXELSHADER_START_DIST_FIX,0);

typedef THeightFogPixelShader<4,MSAASF_PerFragment>  FPerFragmentFourLayerFogPixelShader;
IMPLEMENT_SHADER_TYPE(template<>,FPerFragmentFourLayerFogPixelShader,TEXT("HeightFogPixelShader"),TEXT("FourLayerMain"),SF_Pixel,VER_HEIGHTFOG_PIXELSHADER_START_DIST_FIX,0);

/*-----------------------------------------------------------------------------
	DISHONORED(port): DisFog, Arkane's layered fog.

	Retail has no height fog and no exponential height fog: every fog in the game is a UDisFogComponent, and the 30
	cooked FDisFog* global shaders above are the only fog shaders in the cache (renderer.md 2). A fog shader is
	picked by a pair - the number of layers (0..4) and how many of them carry a colour lookup texture - which is
	Arkane's FDisFogPolicy<Layers,Luts>; FlushDisFogShader (2013 rva 0x4365f0) indexes the 15 combinations with
	LayerBaseOffset[Layers] + Luts.
-----------------------------------------------------------------------------*/

/** DISHONORED(layout): the number of layers one fog pass can draw (FSceneRenderer::RenderFog, 2013 rva 0x4370a0, clamps to 4). */
#define MAX_DISFOG_LAYERS 4

/** DISHONORED(port): the policy both DisFog shaders are templated on (2012 PDB FDisFogPolicy<N,M>, an empty base). */
template<UINT InNumLayers,UINT InNumLuts>
class FDisFogPolicy
{
public:
	enum { NumLayers = InNumLayers };
	enum { NumLuts = InNumLuts };
};

/**
 * DISHONORED(layout): 2012 PDB FDisFogVertexShaderInterface (108 bytes, = FGlobalShader): the fog pass keeps one
 * TShaderMapRef per policy in an array of the interface type and calls SetParameters through it (0x4365f0).
 */
class FDisFogVertexShaderInterface : public FGlobalShader
{
public:
	FDisFogVertexShaderInterface() {}
	FDisFogVertexShaderInterface(const ShaderMetaType::CompiledShaderInitializerType& Initializer): FGlobalShader(Initializer) {}

	virtual void SetParameters(const FViewInfo& View,const FDisFogSceneInfo* const* DisFogs,UINT DisFogCount) = 0;
};

/** DISHONORED(layout): 2012 PDB FDisFogPixelShaderInterface (108 bytes). */
class FDisFogPixelShaderInterface : public FGlobalShader
{
public:
	FDisFogPixelShaderInterface() {}
	FDisFogPixelShaderInterface(const ShaderMetaType::CompiledShaderInitializerType& Initializer): FGlobalShader(Initializer) {}

	virtual void SetParameters(const FViewInfo& View,FDisPrecomputedFogSceneInfo* PrecomputedFog,const FDisFogSceneInfo* const* DisFogs,UINT DisFogCount,UBOOL bHasBloom) = 0;
};

/**
 * DISHONORED(layout): TDisFogVertexShader<FDisFogPolicy<N,M>> is 120 bytes (2012 PDB): the interface plus
 * ScreenPositionScaleBias @108 and ScreenToWorld @114. Serialize 2013 rva 0x411fe0, SetParameters 0x416590.
 */
template<typename PolicyType>
class TDisFogVertexShader : public FDisFogVertexShaderInterface, public PolicyType
{
	DECLARE_SHADER_TYPE(TDisFogVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TDisFogVertexShader() {}

	TDisFogVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FDisFogVertexShaderInterface(Initializer)
	{
		// DISHONORED(bringup): the retail parameter names are not in the shipping exes (there is no shader compiler,
		// so nothing calls Bind); these are the reference names of the same two constants.
		ScreenPositionScaleBiasParameter.Bind(Initializer.ParameterMap,TEXT("ScreenPositionScaleBias"),TRUE);
		ScreenToWorldParameter.Bind(Initializer.ParameterMap,TEXT("ScreenToWorld"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << ScreenPositionScaleBiasParameter;
		Ar << ScreenToWorldParameter;
		return bShaderHasOutdatedParameters;
	}

	/** DISHONORED(port): 2013 rva 0x416590 - the screen position scale/bias and the screen-to-world matrix of the view. */
	virtual void SetParameters(const FViewInfo& View,const FDisFogSceneInfo* const* DisFogs,UINT DisFogCount)
	{
		SetVertexShaderValue(GetVertexShader(),ScreenPositionScaleBiasParameter,View.ScreenPositionScaleBias);

		const FLOAT InvDeviceZ = 1.0f - Z_PRECISION;
		const FMatrix ScreenToWorld = FMatrix(
			FPlane(1,0,0,0),
			FPlane(0,1,0,0),
			FPlane(0,0,InvDeviceZ,1),
			FPlane(0,0,-View.NearClippingDistance * InvDeviceZ,0)
			) * View.InvTranslatedViewProjectionMatrix;
		SetVertexShaderValue(GetVertexShader(),ScreenToWorldParameter,ScreenToWorld);
	}

private:
	FShaderParameter ScreenPositionScaleBiasParameter;
	FShaderParameter ScreenToWorldParameter;
};

/**
 * DISHONORED(layout): TDisFogPixelShader<FDisFogPolicy<N,M>> is 228 bytes (2012 PDB): the interface, the five scene
 * texture parameters @108, the eight per-layer constants, the mask texture, the bloom parts texture, four fog LUT
 * textures and the sun direction - 20 parameters, which is the 60 history words of every cooked FDisFogPixelShader
 * record. Constructor 2013 rva 0x416a20, Serialize 0x416ad0, SetParameters 0x41c720 / 0x41c940.
 */
template<typename PolicyType>
class TDisFogPixelShader : public FDisFogPixelShaderInterface, public PolicyType
{
	DECLARE_SHADER_TYPE(TDisFogPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	TDisFogPixelShader() {}

	TDisFogPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FDisFogPixelShaderInterface(Initializer)
	{
		// DISHONORED(bringup): parameter names as above - retail binds nothing, the cooked record carries the indices.
		mSceneTextureParameters.Bind(Initializer.ParameterMap);
		mLayerMinHeights.Bind(Initializer.ParameterMap,TEXT("LayerMinHeights"),TRUE);
		mLayerMaxHeights.Bind(Initializer.ParameterMap,TEXT("LayerMaxHeights"),TRUE);
		mLayerNearPlanes.Bind(Initializer.ParameterMap,TEXT("LayerNearPlanes"),TRUE);
		mLayerNoFogPlanes.Bind(Initializer.ParameterMap,TEXT("LayerNoFogPlanes"),TRUE);
		mLayerHeightDensityFactor.Bind(Initializer.ParameterMap,TEXT("LayerHeightDensityFactor"),TRUE);
		mLayerInvFarMinusNearPlanes.Bind(Initializer.ParameterMap,TEXT("LayerInvFarMinusNearPlanes"),TRUE);
		mLayerColors.Bind(Initializer.ParameterMap,TEXT("LayerColors"),TRUE);
		mLayerOpacities.Bind(Initializer.ParameterMap,TEXT("LayerOpacities"),TRUE);
		mMaskTextureParameter.Bind(Initializer.ParameterMap,TEXT("MaskTexture"),TRUE);
		mBloomParts.Bind(Initializer.ParameterMap,TEXT("BloomParts"),TRUE);
		for (INT LutIndex = 0; LutIndex < MAX_DISFOG_LAYERS; LutIndex++)
		{
			mFogLUT[LutIndex].Bind(Initializer.ParameterMap,*FString::Printf(TEXT("FogLUT%u"),LutIndex),TRUE);
		}
		mSunDirection.Bind(Initializer.ParameterMap,TEXT("SunDirection"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << mSceneTextureParameters;
		Ar << mLayerMinHeights;
		Ar << mLayerMaxHeights;
		Ar << mLayerNearPlanes;
		Ar << mLayerNoFogPlanes;
		Ar << mLayerHeightDensityFactor;
		Ar << mLayerInvFarMinusNearPlanes;
		Ar << mLayerColors;
		Ar << mLayerOpacities;
		Ar << mMaskTextureParameter;
		Ar << mBloomParts;
		for (INT LutIndex = 0; LutIndex < MAX_DISFOG_LAYERS; LutIndex++)
		{
			Ar << mFogLUT[LutIndex];
		}
		Ar << mSunDirection;
		return bShaderHasOutdatedParameters;
	}

	/**
	 * DISHONORED(port): 2013 rva 0x41c940. Every layer contributes one lane of each four-float constant, relative to
	 * the view height; a layer with a custom transition fades its opacity out once the camera is above it; the unused
	 * lanes repeat layer 0 with opacity 0. PrecomputedFog is filled for the exterior pass only, for the base pass.
	 */
	virtual void SetParameters(const FViewInfo& View,FDisPrecomputedFogSceneInfo* PrecomputedFog,const FDisFogSceneInfo* const* DisFogs,UINT DisFogCount,UBOOL bHasBloom)
	{
		mSceneTextureParameters.Set(&View,this,SF_Point);

		const FLOAT ViewHeight = View.ViewOrigin.Z;
		const UINT NumLayers = Min<UINT>(DisFogCount,MAX_DISFOG_LAYERS);
		if (PrecomputedFog)
		{
			PrecomputedFog->iNbFogs = NumLayers;
		}

		FVector4 MinHeights(0,0,0,0);
		FVector4 MaxHeights(0,0,0,0);
		FVector4 NearPlanes(0,0,0,0);
		FVector4 InvFarMinusNearPlanes(0,0,0,0);
		FVector4 NoFogPlanes(0,0,0,0);
		FVector4 HeightDensityFactors(0,0,0,0);
		FVector4 Opacities(0,0,0,0);
		FLinearColor Colors[MAX_DISFOG_LAYERS];

		for (UINT LayerIndex = 0; LayerIndex < MAX_DISFOG_LAYERS; LayerIndex++)
		{
			// DISHONORED(port): 0x41c940 - the lanes past the layer count repeat layer 0 and are switched off by opacity.
			const FDisFogSceneInfo& Fog = *DisFogs[LayerIndex < NumLayers ? LayerIndex : 0];
			const UBOOL bUsedLayer = LayerIndex < NumLayers;

			MinHeights[LayerIndex] = Fog.mOrigin - ViewHeight;
			MaxHeights[LayerIndex] = (Fog.mHeight + Fog.mOrigin) - ViewHeight;
			NearPlanes[LayerIndex] = Fog.mNearPlane;
			InvFarMinusNearPlanes[LayerIndex] = 1.0f / (Fog.mFarPlane - Fog.mNearPlane);
			NoFogPlanes[LayerIndex] = Fog.mNoFogPlane;
			HeightDensityFactors[LayerIndex] = Fog.mHeightDensityFactor;
			Colors[LayerIndex] = Fog.mLightColor;

			FLOAT Opacity = bUsedLayer ? Fog.mOpacity : 0.0f;
			if (bUsedLayer && Fog.m_bCustomTransition)
			{
				const FLOAT TopOfLayer = Max(MaxHeights[LayerIndex],MinHeights[LayerIndex]);
				const FLOAT TransitionHeight = Fog.m_fCustomTransitionHeight;
				if (TransitionHeight > TopOfLayer)
				{
					Opacity *= 1.0f - (TransitionHeight - TopOfLayer) / TransitionHeight;
				}
			}
			Opacities[LayerIndex] = Opacity;

			const FTexture* LutTexture = (bUsedLayer && Fog.m_pFogLUTTexture) ? Fog.m_pFogLUTTexture->Resource : GBlackTexture;
			if (mFogLUT[LayerIndex].IsBound() && LutTexture)
			{
				SetTextureParameter(GetPixelShader(),mFogLUT[LayerIndex],LutTexture);
			}
			if (PrecomputedFog)
			{
				PrecomputedFog->pLuts[LayerIndex] = (bUsedLayer && Fog.m_pFogLUTTexture) ? &Fog.m_pFogLUTTextureData : NULL;
			}
		}

		// DISHONORED(port): the sun comes from the first layer only, and only when that layer is the sun layer.
		const FDisFogSceneInfo& FirstFog = *DisFogs[0];
		const FVector4 SunDirection(-FirstFog.mSunDirection.X,-FirstFog.mSunDirection.Y,-FirstFog.mSunDirection.Z,FirstFog.mIsSun ? FirstFog.mSunPower : 0.0f);

		if (PrecomputedFog)
		{
			PrecomputedFog->mins = MinHeights;
			PrecomputedFog->maxs = MaxHeights;
			PrecomputedFog->nears = NearPlanes;
			PrecomputedFog->fars = InvFarMinusNearPlanes;
			PrecomputedFog->opacities = Opacities;
			PrecomputedFog->heightdensityfactors = HeightDensityFactors;
			PrecomputedFog->sunDirection = SunDirection;
			for (INT ColorIndex = 0; ColorIndex < MAX_DISFOG_LAYERS; ColorIndex++)
			{
				PrecomputedFog->colors[ColorIndex] = Colors[ColorIndex];
			}
		}

		SetPixelShaderValue(GetPixelShader(),mLayerMinHeights,MinHeights);
		SetPixelShaderValue(GetPixelShader(),mLayerMaxHeights,MaxHeights);
		SetPixelShaderValue(GetPixelShader(),mLayerNearPlanes,NearPlanes);
		SetPixelShaderValue(GetPixelShader(),mLayerInvFarMinusNearPlanes,InvFarMinusNearPlanes);
		SetPixelShaderValue(GetPixelShader(),mLayerNoFogPlanes,NoFogPlanes);
		SetPixelShaderValue(GetPixelShader(),mLayerHeightDensityFactor,HeightDensityFactors);
		SetPixelShaderValues(GetPixelShader(),mLayerColors,Colors,MAX_DISFOG_LAYERS);
		SetPixelShaderValue(GetPixelShader(),mLayerOpacities,Opacities);
		SetPixelShaderValue(GetPixelShader(),mSunDirection,SunDirection);

		// DISHONORED(bringup, agent EE): agent DB's defect 4 reads "FDisFogPixelShader binds MaskTexture and never
		// sets it", and retail's SetParameters (2013 rva 0x41c940) does not set it either. This counts the cooked
		// permutations whose MaskTexture sampler is actually bound, which is what decides whether there is anything to
		// fix: a parameter the cook never bound is a dead name in the shader source, not a missing producer.
		GDisCensusFogMaskTextureBound = mMaskTextureParameter.IsBound() ? 1 : 0;

		if (mBloomParts.IsBound())
		{
			// DISHONORED(port): the bloom parts render target, or black when RenderBloomParts drew nothing this frame.
			const FTexture2DRHIRef& BloomTexture = GSceneRenderTargets.GetBloomPartsTexture();
			if (bHasBloom && IsValidRef(BloomTexture))
			{
				SetTextureParameterDirectly(GetPixelShader(),mBloomParts,TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI(),BloomTexture);
			}
			else
			{
				SetTextureParameter(GetPixelShader(),mBloomParts,GBlackTexture);
			}
		}
	}

private:
	FSceneTextureShaderParameters mSceneTextureParameters;
	FShaderParameter mLayerMinHeights;
	FShaderParameter mLayerMaxHeights;
	FShaderParameter mLayerNearPlanes;
	FShaderParameter mLayerNoFogPlanes;
	FShaderParameter mLayerHeightDensityFactor;
	FShaderParameter mLayerInvFarMinusNearPlanes;
	FShaderParameter mLayerColors;
	FShaderParameter mLayerOpacities;
	FShaderResourceParameter mMaskTextureParameter;
	FShaderResourceParameter mBloomParts;
	FShaderResourceParameter mFogLUT[MAX_DISFOG_LAYERS];
	FShaderParameter mSunDirection;
};

/**
 * DISHONORED(retail): the 15 policies of each frequency, with the retail type names, source file and gate
 * (2013 rva 0xb7f640 ff. / 0xb7fa00 ff.: "DisFogVertexShader" / "DisFogPixelShader", entry point Main, 797 / 23).
 */
#define IMPLEMENT_DISFOG_SHADER_TYPE(Layers,Luts) \
	typedef TDisFogVertexShader<FDisFogPolicy<Layers,Luts> > FDisFogVertexShader##Layers##Luts##LayerType; \
	typedef TDisFogPixelShader<FDisFogPolicy<Layers,Luts> > FDisFogPixelShader##Layers##Luts##LayerType; \
	IMPLEMENT_SHADER_TYPE_NAMED(template<>,FDisFogVertexShader##Layers##Luts##LayerType,TEXT("FDisFogVertexShader") TEXT(#Layers) TEXT(#Luts) TEXT("Layer"),TEXT("DisFogVertexShader"),TEXT("Main"),SF_Vertex,797,23); \
	IMPLEMENT_SHADER_TYPE_NAMED(template<>,FDisFogPixelShader##Layers##Luts##LayerType,TEXT("FDisFogPixelShader") TEXT(#Layers) TEXT(#Luts) TEXT("Layer"),TEXT("DisFogPixelShader"),TEXT("Main"),SF_Pixel,797,23);

IMPLEMENT_DISFOG_SHADER_TYPE(0,0);
IMPLEMENT_DISFOG_SHADER_TYPE(1,0);
IMPLEMENT_DISFOG_SHADER_TYPE(1,1);
IMPLEMENT_DISFOG_SHADER_TYPE(2,0);
IMPLEMENT_DISFOG_SHADER_TYPE(2,1);
IMPLEMENT_DISFOG_SHADER_TYPE(2,2);
IMPLEMENT_DISFOG_SHADER_TYPE(3,0);
IMPLEMENT_DISFOG_SHADER_TYPE(3,1);
IMPLEMENT_DISFOG_SHADER_TYPE(3,2);
IMPLEMENT_DISFOG_SHADER_TYPE(3,3);
IMPLEMENT_DISFOG_SHADER_TYPE(4,0);
IMPLEMENT_DISFOG_SHADER_TYPE(4,1);
IMPLEMENT_DISFOG_SHADER_TYPE(4,2);
IMPLEMENT_DISFOG_SHADER_TYPE(4,3);
IMPLEMENT_DISFOG_SHADER_TYPE(4,4);

/**
 * DISHONORED(port): the fog mask shaders (2013 rva 0xb7f600 "HeightFogVertexShader" / MaskMain, 786 / 15, and
 * 0xb7fdc0 "HeightFogCubeMapPixelShader" / MaskMain, 797 / 19). FSceneRenderer::RenderFogMaskStencil (0x433f80)
 * draws the fog mask meshes with them to build the stencil the fog pass tests against.
 */
class FHeightFogMaskVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FHeightFogMaskVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	FHeightFogMaskVertexShader() {}

	FHeightFogMaskVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		TransformParameter.Bind(Initializer.ParameterMap,TEXT("Transform"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << TransformParameter;
		return bShaderHasOutdatedParameters;
	}

	/** DISHONORED(port): 2013 rva 0x411e60 - the local-to-projection transform of the mask mesh. */
	void SetParameters(const FMatrix& Transform)
	{
		SetVertexShaderValue(GetVertexShader(),TransformParameter,Transform);
	}

private:
	FShaderParameter TransformParameter;
};

/** DISHONORED(port): the mask pixel shader writes one constant (2013 rva 0x4167e0). */
template<UINT FillMode>
class FHeightFogMaskPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FHeightFogMaskPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, FShaderCompilerEnvironment& OutEnvironment) {}

	FHeightFogMaskPixelShader() {}

	FHeightFogMaskPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		FillValueParameter.Bind(Initializer.ParameterMap,TEXT("FillValue"),TRUE);
	}

	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << FillValueParameter;
		return bShaderHasOutdatedParameters;
	}

	void SetParameters(FLOAT FillValue)
	{
		SetPixelShaderValue(GetPixelShader(),FillValueParameter,FillValue);
	}

private:
	FShaderParameter FillValueParameter;
};

IMPLEMENT_SHADER_TYPE(,FHeightFogMaskVertexShader,TEXT("HeightFogVertexShader"),TEXT("MaskMain"),SF_Vertex,786,15);
// DISHONORED(retail): the cooked type name has the spaces of the template argument, "FHeightFogMaskPixelShader< 0 >" (0xb7fdc0).
typedef FHeightFogMaskPixelShader<0> FHeightFogMaskPixelShader0Type;
IMPLEMENT_SHADER_TYPE_NAMED(template<>,FHeightFogMaskPixelShader0Type,TEXT("FHeightFogMaskPixelShader< 0 >"),TEXT("HeightFogCubeMapPixelShader"),TEXT("MaskMain"),SF_Pixel,797,19);

/**
 * DISHONORED(bringup): Engine is a static library, so an object file nothing references is dropped from the exe and
 * its shader type initializers never run - the cooked records then report the types as undeclared (agent AG found this
 * with the Arkane mesh-material units). The Arkane post-process units have no caller until their passes are ported, so
 * one symbol of each is referenced here. Drop this once the FArkPp graph is wired in.
 */
extern void DishonoredLinkArkPpBlurShaderTypes();
extern void DishonoredLinkArkPpDofShaderTypes();
extern void DishonoredLinkArkPpKuwaShaderTypes();
extern void DishonoredLinkArkPpAAShaderTypes();
// DISHONORED(bringup, agent DC): agent BD's GFx shader link anchor is gone from this list. It existed
// because nothing in GFxUI referenced gfxuishaders.cpp, so the 50 GFx shader types of the cooked global
// cache would have been dropped by the linker; agent CC's GetUIPixelShaderInterface2_RenderThread now
// pulls that unit in from gfxuirenderer.cpp, and the movie player reaches the renderer every frame, so
// the unit is referenced for real. The four ark post-process anchors stay: nothing references those.
void (*GDishonoredArkPostProcessLinkAnchors[])() =
{
	&DishonoredLinkArkPpBlurShaderTypes,
	&DishonoredLinkArkPpDofShaderTypes,
	&DishonoredLinkArkPpKuwaShaderTypes,
	&DishonoredLinkArkPpAAShaderTypes,
};

/** The fog vertex declaration resource type. */
class FFogVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	// Destructor
	virtual ~FFogVertexDeclaration() {}

	virtual void InitRHI()
	{
		FVertexDeclarationElementList Elements;
		Elements.AddItem(FVertexElement(0,0,VET_Float2,VEU_Position,0));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}

	virtual void ReleaseRHI()
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/** Vertex declaration for the light function fullscreen 2D quad. */
TGlobalResource<FFogVertexDeclaration> GFogVertexDeclaration;

void FSceneRenderer::InitFogConstants()
{
	// console command override
	FLOAT FogDensityOverride = -1.0f;
	FLOAT FogStartDistanceOverride = -1.0f;

#if !FINAL_RELEASE
	{
		// console variable override
		static IConsoleVariable* CVar = GConsoleManager->FindConsoleVariable(TEXT("FogDensity")); 

		FogDensityOverride = CVar->GetFloat();
	}

	{
		// console variable override
		static IConsoleVariable* CVar = GConsoleManager->FindConsoleVariable(TEXT("FogStartDistance")); 

		FogStartDistanceOverride = CVar->GetFloat();
	}
#endif // !FINAL_RELEASE

	for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
	{
		FViewInfo& View = Views(ViewIndex);

		// set fog consts based on height fog components
		if(ShouldRenderFog(View.Family->ShowFlags))
		{
			// Remap the fog layers into back to front order.
			INT FogLayerMap[4];
			INT NumFogLayers = 0;
			for(INT AscendingFogIndex = Min(Scene->Fogs.Num(),4) - 1;AscendingFogIndex >= 0;AscendingFogIndex--)
			{
				const FHeightFogSceneInfo& FogSceneInfo = Scene->Fogs(AscendingFogIndex);
				if(FogSceneInfo.Height > View.ViewOrigin.Z)
				{
					for(INT DescendingFogIndex = 0;DescendingFogIndex <= AscendingFogIndex;DescendingFogIndex++)
					{
						FogLayerMap[NumFogLayers++] = DescendingFogIndex;
					}
					break;
				}
				FogLayerMap[NumFogLayers++] = AscendingFogIndex;
			}

			// Calculate the fog constants.
			for(INT LayerIndex = 0;LayerIndex < NumFogLayers;LayerIndex++)
			{
				// remapped fog layers in ascending order
				const FHeightFogSceneInfo& FogSceneInfo = Scene->Fogs(FogLayerMap[LayerIndex]);
				// log2(1-density)
				View.HeightFogParams.FogDistanceScale[LayerIndex] = appLoge(1.0f - FogSceneInfo.Density) / appLoge(2.0f);
				if(FogLayerMap[LayerIndex] + 1 < NumFogLayers)
				{
					// each min height is adjusted to aligned with the max height of the layer above
					View.HeightFogParams.FogMinHeight[LayerIndex] = Scene->Fogs(FogLayerMap[LayerIndex] + 1).Height;
				}
				else
				{
					// lowest layer extends down
					View.HeightFogParams.FogMinHeight[LayerIndex] = -HALF_WORLD_MAX;
				}
				// max height is set by the actor's height
				View.HeightFogParams.FogMaxHeight[LayerIndex] = FogSceneInfo.Height;
				// This formula is incorrect, but must be used to support legacy content.  The in-scattering color should be computed like this:
				// FogInScattering[LayerIndex] = FLinearColor(FogComponent->LightColor) * (FogComponent->LightBrightness / (appLoge(2.0f) * FogDistanceScale[LayerIndex]));
				View.HeightFogParams.FogInScattering[LayerIndex] = FogSceneInfo.LightColor / appLoge(0.5f);
				// anything beyond the extinction distance goes to full fog
				View.HeightFogParams.FogExtinctionDistance[LayerIndex] = FogSceneInfo.ExtinctionDistance;
				// start distance where fog affects the scene
				View.HeightFogParams.FogStartDistance[LayerIndex] = Max( 0.f, FogSceneInfo.StartDistance );			
			}

			if (Scene->ExponentialFogs.Num() > 0)
			{
				const FLightSceneInfo* DominantDirectionalLight = NULL;
				for(TSparseArray<FLightSceneInfoCompact>::TConstIterator LightIt(Scene->Lights); LightIt; ++LightIt)
				{
					const FLightSceneInfo* const LightSceneInfo = LightIt->LightSceneInfo;

					if (LightSceneInfo->LightType == LightType_DominantDirectional)
					{
						DominantDirectionalLight = LightSceneInfo;
						break;
					}
				}

				const FExponentialHeightFogSceneInfo& FogInfo = Scene->ExponentialFogs(0);
				const FLOAT CosTerminatorAngle = Clamp(appCos(FogInfo.LightTerminatorAngle * PI / 180.0f), -1.0f + DELTA, 1.0f - DELTA);

				FLOAT FogDensity = FogInfo.FogDensity;

				// console variable override
				if(FogDensityOverride >= 0.0f)
				{
					// Scale the densities back down to their real scale
					// Artists edit the densities scaled up so they aren't entering in minuscule floating point numbers
					FogDensity = FogDensityOverride / 1000.0f;
				}

				FLOAT FogStartDistance = FogInfo.StartDistance;

				// console variable override
				if(FogStartDistanceOverride >= 0.0f)
				{
					FogStartDistance = FogStartDistanceOverride;
				}

				const FLOAT CollapsedFogParameter = FogDensity * appPow(2.0f, -FogInfo.FogHeightFalloff * (View.ViewOrigin.Z - FogInfo.FogHeight));

				View.bRenderExponentialFog = TRUE;

				// bring -1..1 into range 1..0
				FLOAT NormalizedAngle = 0.5f - 0.5f * CosTerminatorAngle;
				// LogHalf = log(0.5f)
				const FLOAT LogHalf = -0.30103f;
				// precompute a constant so the shader needs less ALU
				FLOAT PowFactor = LogHalf / appLoge(NormalizedAngle);

				View.ExponentialFogParameters = FVector4(CollapsedFogParameter, FogInfo.FogHeightFalloff, PowFactor, FogStartDistance);
				View.ExponentialFogColor = FVector(FogInfo.OppositeLightColor.R, FogInfo.OppositeLightColor.G, FogInfo.OppositeLightColor.B);
				View.LightInscatteringColor = FVector(FogInfo.LightInscatteringColor.R, FogInfo.LightInscatteringColor.G, FogInfo.LightInscatteringColor.B);
				// Use up in world space if there is no dominant directional light
				View.DominantDirectionalLightDirection = DominantDirectionalLight ? -DominantDirectionalLight->GetDirection() : FVector(0,0,1);
				View.FogMaxOpacity = FogInfo.FogMaxOpacity;
			}
		}
	}
}

FGlobalBoundShaderState DownsampleDepthAndFogBoundShaderState;

FGlobalBoundShaderState ExponentialBoundShaderState[MSAASF_Num];
FGlobalBoundShaderState OneLayerFogBoundShaderState[MSAASF_Num];
FGlobalBoundShaderState FourLayerFogBoundShaderState[MSAASF_Num];

/** Sets the bound shader state for either the per-pixel or per-sample fog pass. */
template<EMSAAShaderFrequency MSAAShaderFrequency>
void SetFogShaders(FScene* Scene,const FViewInfo& View)
{
	const INT NumSceneFogs = Clamp<INT>(Scene->Fogs.Num(), 0, 4);
	if (Scene->ExponentialFogs.Num() > 0)
	{
		TShaderMapRef<THeightFogVertexShader<1> > VertexShader(GetGlobalShaderMap());
		TShaderMapRef<TExponentialHeightFogPixelShader<MSAAShaderFrequency> > ExponentialHeightFogPixelShader(GetGlobalShaderMap());

		SetGlobalBoundShaderState(ExponentialBoundShaderState[MSAAShaderFrequency], GFogVertexDeclaration.VertexDeclarationRHI, *VertexShader, *ExponentialHeightFogPixelShader, sizeof(FVector2D));
		VertexShader->SetParameters(View);
		ExponentialHeightFogPixelShader->SetParameters(View);
	}
	//use the optimized one layer version if there is only one height fog layer
	else if (NumSceneFogs == 1)
	{
		TShaderMapRef<THeightFogVertexShader<1> > VertexShader(GetGlobalShaderMap());
		TShaderMapRef<THeightFogPixelShader<1,MSAAShaderFrequency> > OneLayerHeightFogPixelShader(GetGlobalShaderMap());

		SetGlobalBoundShaderState(OneLayerFogBoundShaderState[MSAAShaderFrequency], GFogVertexDeclaration.VertexDeclarationRHI, *VertexShader, *OneLayerHeightFogPixelShader, sizeof(FVector2D));
		VertexShader->SetParameters(View);
		OneLayerHeightFogPixelShader->SetParameters(View, NumSceneFogs);
	}
	//otherwise use the four layer version
	else
	{
		TShaderMapRef<THeightFogVertexShader<4> > VertexShader(GetGlobalShaderMap());
		TShaderMapRef<THeightFogPixelShader<4,MSAAShaderFrequency> > FourLayerHeightFogPixelShader(GetGlobalShaderMap());

		SetGlobalBoundShaderState(FourLayerFogBoundShaderState[MSAAShaderFrequency], GFogVertexDeclaration.VertexDeclarationRHI, *VertexShader, *FourLayerHeightFogPixelShader, sizeof(FVector2D));
		VertexShader->SetParameters(View);
		FourLayerHeightFogPixelShader->SetParameters(View, NumSceneFogs);
	}
}

/**
 * DISHONORED(bringup): the reference height-fog pass. Retail has no shader for any of the types it binds
 * (renderer.md 4), so it is only reachable through -referencefog; FSceneRenderer::RenderFog below is the retail one.
 */
UBOOL FSceneRenderer::RenderReferenceFog(UINT DPGIndex)
{
	const INT NumSceneFogLayers = Scene->Fogs.Num();
	if (DPGIndex == SDPG_World && (NumSceneFogLayers > 0 || Scene->ExponentialFogs.Num() > 0))
	{
		SCOPED_DRAW_EVENT(EventFog)(DEC_SCENE_ITEMS,TEXT("Fog"));

		static const FVector2D Vertices[4] =
		{
			FVector2D(-1,-1),
			FVector2D(-1,+1),
			FVector2D(+1,+1),
			FVector2D(+1,-1),
		};
		static const WORD Indices[6] =
		{
			0, 1, 2,
			0, 2, 3
		};

		GSceneRenderTargets.BeginRenderingSceneColor();
		for(INT ViewIndex = 0;ViewIndex < Views.Num();ViewIndex++)
		{
			const FViewInfo& View = Views(ViewIndex);
			if (View.bOneLayerHeightFogRenderedInAO || !View.IsPerspectiveProjection())
			{
				// Skip one layer height fog for this view since it was already rendered with ambient occlusion
				continue;
			}

			// Set the device viewport for the view.
			RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
			RHISetViewParameters(View);
			RHISetMobileHeightFogParams(View.HeightFogParams);

			if(GRHIShaderPlatform == SP_PCD3D_SM5 && GSystemSettings.UsesMSAA())
			{
				// Clear the stencil buffer to zero.
				RHIClear(FALSE,FLinearColor(0,0,0),FALSE,0,TRUE,0);
			}

			// depth tests (to cull pixels in front of the start distance), no backface culling.
			RHISetDepthState(TStaticDepthState<FALSE,CF_LessEqual>::GetRHI());

			RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());

			RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_SourceAlpha>::GetRHI());

			// disable alpha writes in order to preserve scene depth values on PC
			RHISetColorWriteMask(CW_RED|CW_GREEN|CW_BLUE);

			// Do a separate per-pixel and per-sample pass on D3D11, but only the per-pixel pass on other platforms.
			if(GRHIShaderPlatform == SP_PCD3D_SM5 && GSystemSettings.UsesMSAA())
			{
				for(UINT PassIndex = 0;PassIndex < 2;++PassIndex)
				{
					SCOPED_CONDITIONAL_DRAW_EVENT(EventRenderPerSample,PassIndex==1)(DEC_SCENE_ITEMS,TEXT("PerSample"));

					if(PassIndex == 0)
					{
						// Set the per-pixel shaders, and stencil state that sets the stencil buffer to 1 where the per-pixel shader executes.
						SetFogShaders<MSAASF_PerPixel>(Scene,View);
						RHISetStencilState(TStaticStencilState<
							TRUE,CF_Always,SO_Keep,SO_Keep,SO_Replace,
							FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
							0xff,0xff,1
							>::GetRHI());
					}
					else
					{
						// Set the per-sample shaders, and stencil state that culls fragments which were written to by the per-pixel shader.
						SetFogShaders<MSAASF_PerFragment>(Scene,View);
						RHISetStencilState(TStaticStencilState<
							TRUE,CF_Equal,SO_Keep,SO_Keep,SO_Keep,
							FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,
							0xff,0xff,0
							>::GetRHI());
					}

					// Draw a quad covering the view.
					RHIDrawIndexedPrimitiveUP(
						PT_TriangleList,
						0,
						ARRAY_COUNT(Vertices),
						2,
						Indices,
						sizeof(Indices[0]),
						Vertices,
						sizeof(Vertices[0])
						);
				}
			}
			else
			{
				// Set the non-MSAA shaders.
				SetFogShaders<MSAASF_NoMSAA>(Scene,View);

				// Draw a quad covering the view.
				RHIDrawIndexedPrimitiveUP(
					PT_TriangleList,
					0,
					ARRAY_COUNT(Vertices),
					2,
					Indices,
					sizeof(Indices[0]),
					Vertices,
					sizeof(Vertices[0])
					);
			}

			// restore color write mask
			RHISetColorWriteMask(CW_RED|CW_GREEN|CW_BLUE|CW_ALPHA);

			// Restore the stencil state.
			RHISetStencilState(TStaticStencilState<>::GetRHI());
		}

		//no need to resolve since we used alpha blending
		GSceneRenderTargets.FinishRenderingSceneColor(FALSE);
		return TRUE;
	}

	return FALSE;
}

/** 
 * Renders fog scattering values and max depths into 1/4 size RT0 and RT1. 
 * Called from AmbientOcclusion's DownsampleDepth()
 *
 * @param Scene - current scene
 * @param View - current view
 * @param DPGIndex - depth priority group
 * @param DownsampleDimensions - dimensions for downsampling
 */
UBOOL RenderQuarterDownsampledDepthAndFog(const FScene* Scene, const FViewInfo& View, UINT DPGIndex, const FDownsampleDimensions& DownsampleDimensions)
{
#if XBOX
	const INT NumSceneFogLayers = Scene->Fogs.Num();
	// Currently only works with one layer height fog
	if (DPGIndex == SDPG_World && NumSceneFogLayers == 1)
	{
		// Verify that the ambient occlusion downsample factor matches the fog downsample factor
		checkSlow(DownsampleDimensions.Factor == 2);

		SCOPED_DRAW_EVENT(EventFog)(DEC_SCENE_ITEMS,TEXT("DepthAndFogDownsample"));

		static const FVector2D Vertices[4] =
		{
			FVector2D(-1,-1),
			FVector2D(-1,+1),
			FVector2D(+1,+1),
			FVector2D(+1,-1),
		};
		static const WORD Indices[6] =
		{
			0, 1, 2,
			0, 2, 3
		};

		GSceneRenderTargets.BeginRenderingFogBuffer();

		RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
		// Disable depth test and writes
		RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
		RHISetBlendState(TStaticBlendState<>::GetRHI());

		// Set the viewport to the current view in the occlusion buffer.
		RHISetViewport(DownsampleDimensions.TargetX, DownsampleDimensions.TargetY, 0.0f, 
			DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX, DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY, 1.0f);				

		TShaderMapRef<THeightFogVertexShader<1> > VertexShader(GetGlobalShaderMap());
		TShaderMapRef<THeightFogPixelShader<0,MSAASF_NoMSAA> > DownsampleDepthAndFogPixelShader(GetGlobalShaderMap());

		SetGlobalBoundShaderState(DownsampleDepthAndFogBoundShaderState, GFogVertexDeclaration.VertexDeclarationRHI, *VertexShader, *DownsampleDepthAndFogPixelShader, sizeof(FVector2D));
		VertexShader->SetParameters(View);
		DownsampleDepthAndFogPixelShader->SetParameters(View, NumSceneFogLayers);

		// Draw a quad covering the view.
		RHIDrawIndexedPrimitiveUP(
			PT_TriangleList,
			0,
			ARRAY_COUNT(Vertices),
			2,
			Indices,
			sizeof(Indices[0]),
			Vertices,
			sizeof(Vertices[0])
			);

		GSceneRenderTargets.FinishRenderingFogBuffer(
			FResolveParams(FResolveRect(
				DownsampleDimensions.TargetX,
				DownsampleDimensions.TargetY, 
				DownsampleDimensions.TargetX + DownsampleDimensions.TargetSizeX,
				DownsampleDimensions.TargetY + DownsampleDimensions.TargetSizeY
				))
			);

		return TRUE;
	}
#endif
	return FALSE;
}


/*-----------------------------------------------------------------------------
	DISHONORED(port): the DisFog pass (FSceneRenderer::RenderFog, 2013 rva 0x4370a0).
-----------------------------------------------------------------------------*/

/** DISHONORED(bringup): per-pass draw counts for the census line in SceneRendering.cpp. */
extern INT GDisCensusFogScene;
extern INT GDisCensusFogLayers;
extern INT GDisCensusFogDraws;

namespace
{
	/** The shader map references of the 15 policies, resolved once (2013 rva 0x4365f0: gDisFogVertexShaders / gDisFogPixelShaders). */
	FDisFogVertexShaderInterface* GDisFogVertexShaders[15] = {0};
	FDisFogPixelShaderInterface* GDisFogPixelShaders[15] = {0};
	FGlobalBoundShaderState GDisFogBoundShaderStates[15];
	UBOOL GDisFogShadersResolved = FALSE;

	/** LayerBaseOffset[Layers] + Luts is the index of a policy (0x4365f0). */
	const UINT GDisFogLayerBaseOffset[5] = { 0, 1, 3, 6, 10 };

	template<UINT Layers,UINT Luts>
	void ResolveDisFogPolicy()
	{
		const UINT Index = GDisFogLayerBaseOffset[Layers] + Luts;
		TShaderMapRef<TDisFogVertexShader<FDisFogPolicy<Layers,Luts> > > VertexShader(GetGlobalShaderMap());
		TShaderMapRef<TDisFogPixelShader<FDisFogPolicy<Layers,Luts> > > PixelShader(GetGlobalShaderMap());
		GDisFogVertexShaders[Index] = *VertexShader;
		GDisFogPixelShaders[Index] = *PixelShader;
	}

	void ResolveDisFogShaders()
	{
		if (GDisFogShadersResolved)
		{
			return;
		}
		ResolveDisFogPolicy<0,0>();
		ResolveDisFogPolicy<1,0>();
		ResolveDisFogPolicy<1,1>();
		ResolveDisFogPolicy<2,0>();
		ResolveDisFogPolicy<2,1>();
		ResolveDisFogPolicy<2,2>();
		ResolveDisFogPolicy<3,0>();
		ResolveDisFogPolicy<3,1>();
		ResolveDisFogPolicy<3,2>();
		ResolveDisFogPolicy<3,3>();
		ResolveDisFogPolicy<4,0>();
		ResolveDisFogPolicy<4,1>();
		ResolveDisFogPolicy<4,2>();
		ResolveDisFogPolicy<4,3>();
		ResolveDisFogPolicy<4,4>();
		GDisFogShadersResolved = TRUE;
	}

	/** DISHONORED(port): 2013 rva 0x4365f0 - bind the policy's pair and set its parameters. */
	void FlushDisFogShader(UINT Type,const FDisFogSceneInfo* const* DisFogs,UINT DisFogCount,UINT LayerLutCount,FScene& Scene,FViewInfo& View,UBOOL bHasBloom)
	{
		ResolveDisFogShaders();

		const UINT Index = GDisFogLayerBaseOffset[Min<UINT>(DisFogCount,4)] + Min<UINT>(LayerLutCount,4);
		FDisFogVertexShaderInterface* VertexShader = GDisFogVertexShaders[Index];
		FDisFogPixelShaderInterface* PixelShader = GDisFogPixelShaders[Index];
		// DISHONORED(bringup): one line the first time a policy is used, so a run says which shader pair it bound.
		{
			static UBOOL bLoggedPolicy[15] = {0};
			if (!bLoggedPolicy[Index])
			{
				bLoggedPolicy[Index] = TRUE;
				debugf(TEXT("DISHONORED(bringup): DisFog policy %i (%i layers, %i luts): vertex shader %s, pixel shader %s"),
					Index,DisFogCount,LayerLutCount,VertexShader ? TEXT("found") : TEXT("MISSING"),PixelShader ? TEXT("found") : TEXT("MISSING"));
				for (UINT FogIndex = 0; FogIndex < DisFogCount; FogIndex++)
				{
					const FDisFogSceneInfo& Fog = *DisFogs[FogIndex];
					debugf(TEXT("DISHONORED(bringup): DisFog layer %i: origin %.1f height %.1f near %.1f far %.1f nofog %.1f density %.3f opacity %.3f colour (%.2f %.2f %.2f) interior %i sun %i exclusive %i custom %i lut %s"),
						FogIndex,Fog.mOrigin,Fog.mHeight,Fog.mNearPlane,Fog.mFarPlane,Fog.mNoFogPlane,Fog.mHeightDensityFactor,Fog.mOpacity,
						Fog.mLightColor.R,Fog.mLightColor.G,Fog.mLightColor.B,
						(INT)Fog.mInterior,(INT)Fog.mIsSun,(INT)Fog.mIsExclusive,(INT)Fog.m_bCustomTransition,
						Fog.m_pFogLUTTexture ? TEXT("set") : TEXT("none"));
				}
			}
		}

		if (!VertexShader || !PixelShader)
		{
			return;
		}

		// DISHONORED(port): 2013 rva 0x4365f0 - the two-float-position declaration, stride 8.
		SetGlobalBoundShaderState(GDisFogBoundShaderStates[Index],ArkGetCommonVertexDeclaration(ARK_COMMON_VD_FLOAT2),VertexShader,PixelShader,sizeof(FVector2D));
		VertexShader->SetParameters(View,DisFogs,DisFogCount);
		// DISHONORED(port): only the exterior pass fills the precomputed fog the base pass reads.
		PixelShader->SetParameters(View,Type == 1 ? &View.DisPrecomputedFogs : NULL,DisFogs,DisFogCount,bHasBloom);
	}
}

/**
 * DISHONORED(port): 2013 rva 0x436d10. Layers whose custom transition puts them entirely below the camera are
 * dropped, the fog mask stencil is built, and one full-screen triangle is drawn with the policy's shader pair.
 */
UBOOL FSceneRenderer::RenderFogPass(UINT Type,const FDisFogSceneInfo* const* DisFogs,UINT DisFogCount,UBOOL bRestoreStencilToZero)
{
	SCOPED_DRAW_EVENT(EventFogPass)(DEC_SCENE_ITEMS,Type == 1 ? TEXT("Exterior") : TEXT("Interior"));

	if (!Views.Num())
	{
		return FALSE;
	}
	FViewInfo& View = Views(0);
	const FLOAT ViewHeight = View.ViewOrigin.Z;

	const FDisFogSceneInfo* ViewDisFogs[16];
	UINT ViewDisFogCount = 0;
	for (UINT FogIndex = 0; FogIndex < DisFogCount; FogIndex++)
	{
		const FDisFogSceneInfo* Fog = DisFogs[FogIndex];
		if (Fog->m_bCustomTransition)
		{
			const FLOAT TopOfLayer = Max((Fog->mHeight + Fog->mOrigin) - ViewHeight,Fog->mOrigin - ViewHeight);
			if (TopOfLayer <= 0.0f)
			{
				continue;
			}
		}
		ViewDisFogs[ViewDisFogCount++] = Fog;
	}

	RHISetViewport(View.RenderTargetX,View.RenderTargetY,0.0f,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY,1.0f);
	RHISetViewParameters(View);
	RHISetScissorRect(TRUE,View.RenderTargetX,View.RenderTargetY,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY);

	// DISHONORED(bringup): FSceneRenderer::RenderFogMaskStencil (2013 rva 0x433f80, 9,806 bytes) is what makes
	// bHasMask TRUE, and it writes STENCIL ONLY: it clears stencil to (the listener cell's interior flag == Type),
	// sets TStaticStencilState<TRUE,CF_Always,...,SO_Replace,...> with reference 0 or 1 and
	// RHISetColorWriteEnable(FALSE), and draws the portal quads of the audio cell the view origin is in. So interior
	// fog covers the cell and is punched out at each portal, or the other way round outside it. It does NOT fill the
	// fog-mask render target: no colour is written at all (agentDB.md defect 4 attributed the MaskTexture sampler to
	// this function; the measurement below is what settles it).
	// It is not ported because its only input is the audio cell graph: retail calls
	// GWorld->m_pAudioSystem->GetCellAtPoint(View.ViewOrigin, GetCurrentCachedCell()) and then walks that cell's
	// AGenericPortal list, and UDishonoredAudioSystem::GetCellAtPoint (2013 rva 0x792d10, DishonoredGame) is a
	// hand-over, not this package's file. With no cell retail keeps its default interior flag of 1, which masks
	// interior fog out of the whole view and lets exterior fog cover it - so porting the pass with the cell lookup
	// missing would change the frame for the wrong reason. See agents/agentEE.md 4.
	const UBOOL bHasMask = FALSE;
	GDisCensusFogMaskStencilDraws = 0;

	// DISHONORED(port): 2013 rva 0x436d10 sets the states in this order and ends with an opaque blend: the fog pixel
	// shader samples scene colour itself (FSceneTextureShaderParameters) and writes the composited result, so the
	// draw overwrites RGB instead of blending into it.
	RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_SourceAlpha,BO_Add,BF_One,BF_Zero>::GetRHI());
	RHISetDepthState(TStaticDepthState<FALSE,CF_Always>::GetRHI());
	RHISetRasterizerState(TStaticRasterizerState<FM_Solid,CM_None>::GetRHI());
	if (bHasMask)
	{
		RHISetStencilState(TStaticStencilState<TRUE,CF_NotEqual,SO_Keep,SO_Keep,SO_Keep,FALSE,CF_Always,SO_Keep,SO_Keep,SO_Keep,0xff,0xff,0>::GetRHI());
	}
	else
	{
		RHISetStencilState(TStaticStencilState<>::GetRHI());
	}
	RHISetColorWriteEnable(TRUE);
	RHISetColorWriteMask(CW_RGB);
	RHISetBlendState(TStaticBlendState<BO_Add,BF_One,BF_Zero,BO_Add,BF_One,BF_Zero>::GetRHI());

	// the leading run of layers that carry a colour lookup texture (the array is sorted so they come first)
	UINT LayerLutCount = 0;
	while (LayerLutCount < ViewDisFogCount && ViewDisFogs[LayerLutCount]->m_pFogLUTTexture)
	{
		LayerLutCount++;
	}

	FlushDisFogShader(Type,ViewDisFogs,ViewDisFogCount,LayerLutCount,*Scene,View,m_BloomNeedBlit);

	// DISHONORED(port): one clip-space triangle of two-float positions (2013 rva 0x436d10, stride 8).
	RHIDrawPrimitiveUP(PT_TriangleList,1,ArkFullScreenTriangleFloat2Vertices,sizeof(FVector2D));
	GDisCensusFogDraws++;
	GDisCensusFogLayers += ViewDisFogCount;

	if (bRestoreStencilToZero && bHasMask)
	{
		RHIClear(FALSE,FLinearColor::Black,FALSE,0.0f,TRUE,0);
	}
	RHISetScissorRect(FALSE,0,0,0,0);
	return TRUE;
}

/**
 * DISHONORED(port): 2013 rva 0x4370a0. The scene's DisFog layers are split into interior and exterior sets; an
 * exclusive layer is moved to the front of its set and then it is the only one drawn; a sun layer is moved to the
 * front because the pixel shader takes the sun direction from layer 0. Both sets are clamped to four layers.
 */
UBOOL FSceneRenderer::RenderFog(UINT DPGIndex)
{
	if (DPGIndex != SDPG_World)
	{
		return FALSE;
	}

	const INT NumSceneDisFogLayers = Scene->DisFogs.Num();
	GDisCensusFogScene = NumSceneDisFogLayers;
	GDisCensusFogLayers = 0;
	GDisCensusFogDraws = 0;
	if (NumSceneDisFogLayers <= 0)
	{
		return FALSE;
	}

	const FDisFogSceneInfo* InteriorFogs[16];
	const FDisFogSceneInfo* ExteriorFogs[16];
	UINT InteriorCount = 0;
	UINT ExteriorCount = 0;
	for (INT FogIndex = 0; FogIndex < NumSceneDisFogLayers && FogIndex < 16; FogIndex++)
	{
		const FDisFogSceneInfo* Fog = &Scene->DisFogs(FogIndex);
		if (Fog->mInterior)
		{
			InteriorFogs[InteriorCount++] = Fog;
		}
		else
		{
			ExteriorFogs[ExteriorCount++] = Fog;
		}
	}

	// the sun layer first, so TDisFogPixelShader::SetParameters finds it at index 0
	for (UINT FogIndex = 1; FogIndex < ExteriorCount; FogIndex++)
	{
		if (ExteriorFogs[FogIndex]->mIsSun)
		{
			Exchange(ExteriorFogs[0],ExteriorFogs[FogIndex]);
			break;
		}
	}
	for (UINT FogIndex = 1; FogIndex < InteriorCount; FogIndex++)
	{
		if (InteriorFogs[FogIndex]->mIsSun)
		{
			Exchange(InteriorFogs[0],InteriorFogs[FogIndex]);
			break;
		}
	}

	// an exclusive exterior layer replaces the whole exterior set and switches the interior set off
	UBOOL bHasInterior = InteriorCount > 0;
	for (UINT FogIndex = 0; FogIndex < ExteriorCount; FogIndex++)
	{
		if (ExteriorFogs[FogIndex]->mIsExclusive)
		{
			Exchange(ExteriorFogs[0],ExteriorFogs[FogIndex]);
			ExteriorCount = 1;
			bHasInterior = FALSE;
			break;
		}
	}
	InteriorCount = Min<UINT>(InteriorCount,MAX_DISFOG_LAYERS);
	ExteriorCount = Min<UINT>(ExteriorCount,MAX_DISFOG_LAYERS);

	SCOPED_DRAW_EVENT(EventFog)(DEC_SCENE_ITEMS,TEXT("DisFog"));

	// DISHONORED(port): 2013 rva 0x4370a0 resolves scene colour first, so the fog pixel shader can sample it through
	// FSceneTextureShaderParameters, and then renders into the scene colour surface.
	if (!m_BloomNeedBlit)
	{
		GSceneRenderTargets.ResolveSceneColor();
	}
	GSceneRenderTargets.BeginRenderingSceneColor(FALSE);

	if (bHasInterior && InteriorCount > 0)
	{
		RenderFogPass(0,InteriorFogs,InteriorCount,ExteriorCount == 0);
	}
	if (ExteriorCount > 0)
	{
		RenderFogPass(1,ExteriorFogs,ExteriorCount,TRUE);
	}

	{
		const FViewInfo& View = Views(0);
		GSceneRenderTargets.FinishRenderingSceneColor(TRUE,FResolveRect(View.RenderTargetX,View.RenderTargetY,View.RenderTargetX + View.RenderTargetSizeX,View.RenderTargetY + View.RenderTargetSizeY));
	}

	RHISetColorWriteEnable(TRUE);
	RHISetColorWriteMask(CW_RGBA);
	RHISetDepthState(TStaticDepthState<TRUE,CF_LessEqual>::GetRHI());
	RHISetBlendState(TStaticBlendState<>::GetRHI());
	RHISetStencilState(TStaticStencilState<>::GetRHI());
	RHISetScissorRect(FALSE,0,0,0,0);
	return TRUE;
}

UBOOL ShouldRenderFog(const EShowFlags& ShowFlags)
{
	return (ShowFlags & SHOW_Fog) 
		&& (ShowFlags & SHOW_Materials) 
		&& !(ShowFlags & SHOW_TextureDensity) 
		&& !(ShowFlags & SHOW_ShaderComplexity) 
		&& !(ShowFlags & SHOW_LightMapDensity);
}
