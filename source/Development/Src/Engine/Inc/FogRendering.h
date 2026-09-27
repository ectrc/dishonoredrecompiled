/*=============================================================================
	FogRendering.h: 
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/** Parameters needed to render exponential height fog. */
class FExponentialHeightFogShaderParameters
{
public:

	/** Binds the parameters. */
	void Bind(const FShaderParameterMap& ParameterMap);

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FExponentialHeightFogShaderParameters& P);

	FShaderParameter ExponentialFogParameters;
	FShaderParameter ExponentialFogColorParameter;
	FShaderParameter LightInscatteringColorParameter;
	FShaderParameter LightVectorParameter;
};

/** Encapsulates parameters needed to calculate height fog in a vertex shader. */
class FHeightFogShaderParameters
{
public:

	/** Binds the parameters. */
	void Bind(const FShaderParameterMap& ParameterMap);

	/** 
	* Sets the parameter values, this must be called before rendering the primitive with the shader applied. 
	* @param VertexShader - the vertex shader to set the parameters on
	*/
	void SetVertexShader(
		const FVertexFactory* VertexFactory, 
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FMaterial& Material,
		const FSceneView* View,
		const UBOOL bAllowGlobalFog,
		FShader* VertexShader) const;

#if WITH_D3D11_TESSELLATION
	/** 
	* Sets the parameter values, this must be called before rendering the primitive with the shader applied. 
	* @param DomainShader - the vertex shader to set the parameters on
	*/
	void SetDomainShader(
		const FVertexFactory* VertexFactory, 
		const FMaterialRenderProxy* MaterialRenderProxy,
		const FMaterial& Material,
		const FSceneView* View,
		FShader* DomainShader) const;
#endif

	/** Serializer. */
	friend FArchive& operator<<(FArchive& Ar,FHeightFogShaderParameters& P);

private:

	template<typename ShaderRHIParamRef>
	void Set(
		const FVertexFactory* VertexFactory, 
		const FMaterialRenderProxy* MaterialRenderProxy, 
		const FMaterial& Material,
		const FSceneView* View,
		const UBOOL bAllowGlobalFog,
		ShaderRHIParamRef Shader) const;

	FShaderParameter	bUseExponentialHeightFogParameter;
	FExponentialHeightFogShaderParameters ExponentialParameters;
	FShaderParameter	FogDistanceScaleParameter;
	FShaderParameter	FogExtinctionDistanceParameter;
	FShaderParameter	FogMinHeightParameter;
	FShaderParameter	FogMaxHeightParameter;
	FShaderParameter	FogInScatteringParameter;
	FShaderParameter	FogStartDistanceParameter;
};

extern UBOOL ShouldRenderFog(const EShowFlags& ShowFlags);

/**
 * DISHONORED(layout): one layer of Arkane's fog, the render-thread copy of a UDisFogComponent (2012 PDB
 * FDisFogSceneInfo, 88 bytes; built by FScene::AddDisFog, 2013 rva 0x422340). Retail's fog is this and nothing else:
 * the reference height fog and exponential height fog have no cooked shader (renderer.md 4).
 */
class FDisFogSceneInfo
{
public:
	const class UDisFogComponent* mComponent;
	FLinearColor mLightColor;
	FLOAT mOrigin;
	FLOAT mHeight;
	FLOAT mFarPlane;
	FLOAT mNearPlane;
	FLOAT mNoFogPlane;
	FLOAT mHeightDensityFactor;
	FLOAT mOpacity;
	BITFIELD mInterior:1;
	BITFIELD mIsSun:1;
	BITFIELD mIsExclusive:1;
	BITFIELD m_bCustomTransition:1;
	FVector mSunDirection;
	FLOAT mSunPower;
	FLOAT m_fCustomTransitionHeight;
	class UTexture2D* m_pFogLUTTexture;
	TArray<BYTE> m_pFogLUTTextureData;

	FDisFogSceneInfo(): mComponent(NULL), mLightColor(FLinearColor::Black), mOrigin(0.0f), mHeight(0.0f),
		mFarPlane(0.0f), mNearPlane(0.0f), mNoFogPlane(0.0f), mHeightDensityFactor(0.0f), mOpacity(0.0f),
		mInterior(FALSE), mIsSun(FALSE), mIsExclusive(FALSE), m_bCustomTransition(FALSE),
		mSunDirection(0.0f,0.0f,1.0f), mSunPower(0.0f), m_fCustomTransitionHeight(0.0f), m_pFogLUTTexture(NULL)
	{}

	/** DISHONORED(port): 2013 rva 0x421bc0 - the component's own values, evaluated once per attach. */
	FDisFogSceneInfo(const class UDisFogComponent* InComponent);
};

/**
 * DISHONORED(port): the DisFog layers are sorted so that every layer with a colour lookup texture comes first
 * (2013 rva 0x40e770, the comparator of FScene::AddDisFog's sort): FSceneRenderer::RenderFogPass then counts the
 * leading run of layers with a lookup to choose FDisFogPolicy<Layers,Luts>.
 */
IMPLEMENT_COMPARE_CONSTREF(FDisFogSceneInfo,SceneCore,{ return (B.m_pFogLUTTexture != NULL ? 1 : 0) - (A.m_pFogLUTTexture != NULL ? 1 : 0); });

/**
 * DISHONORED(layout): the fog the base pass applies per vertex instead of per pixel (2012 PDB
 * FDisPrecomputedFogSceneInfo, 208 bytes, FViewInfo @4032). TDisFogPixelShader::SetParameters fills it for the
 * exterior pass, and TBasePassPixelShader<Policy,bSkyLight,TRUE> reads it (agent AG, BasePassRendering.h).
 */
struct FDisPrecomputedFogSceneInfo
{
	INT iNbFogs;
	FVector4 mins;
	FVector4 maxs;
	FVector4 nears;
	FVector4 fars;
	FVector4 opacities;
	FVector4 heightdensityfactors;
	FVector4 sunDirection;
	FLinearColor colors[4];
	const TArray<BYTE>* pLuts[4];

	FDisPrecomputedFogSceneInfo(): iNbFogs(0), mins(0,0,0,0), maxs(0,0,0,0), nears(0,0,0,0), fars(0,0,0,0),
		opacities(0,0,0,0), heightdensityfactors(0,0,0,0), sunDirection(0,0,0,0)
	{
		for (INT Index = 0; Index < 4; Index++)
		{
			colors[Index] = FLinearColor::Black;
			pLuts[Index] = NULL;
		}
	}
};
