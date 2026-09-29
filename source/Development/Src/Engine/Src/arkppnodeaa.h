/*===========================================================================
    arkppnodeaa.h - the parameter structs and the three shared shader types of Arkane's antialiasing node.

    DISHONORED(port): retail declares FFXAAVertexShader, FMLAAVertexShader and FMLAAComputeLineLengthPixelShader in
    arkppnodeaa.cpp together with the node's passes (the 2012 PDB attributes FFXAAVertexShader::SetParameters to
    arkppnodeaa.cpp:212 and FMLAAVertexShader::SetParameters to arkppnodeaa.cpp:494). This tree inherited them in
    PostProcessAA.cpp, where agent AH gave them retail's parameter layout and where the reference antialiasing pass
    still names them, so they live here in a header that both units include; their SetParameters bodies are in
    arkppnodeaa.cpp, which is where retail has them.
===========================================================================*/
#ifndef _INC_ARKPPNODEAA
#define _INC_ARKPPNODEAA

/**
 * DISHONORED(layout): 2012 PDB FArkPpFxAaParameters (128 bytes) and FArkPpMlaaParameters (128), the argument every
 * FXAA / MLAA pass builds and hands to both shaders.
 *
 * DISHONORED(retail): retail's member is an FBox2D, which Dishonored's branch redefines as a single
 * `FVector4 m_MinX_MinY_MaxX_MaxY` (types/all_types.h). The reference FBox2D of this tree is UE3's Min/Max/bIsValid
 * one, so the member is the FVector4 itself; the four components are (MinX, MinY, MaxX, MaxY) as in retail.
 */
struct FArkPpFxAaParameters
{
	FVector mLumEquation;
	UINT mSizeX;
	UINT mSizeY;
	FVector4 m_Viewport;
	UBOOL mbUseSceneColorLdr;
	FTexture mSceneColor;
	const FSceneView* mView;

	FArkPpFxAaParameters()
		: mLumEquation(0.0f,0.0f,0.0f)
		, mSizeX(0)
		, mSizeY(0)
		, m_Viewport(0.0f,0.0f,0.0f,0.0f)
		, mbUseSceneColorLdr(FALSE)
		, mView(NULL)
	{}
};

struct FArkPpMlaaParameters
{
	FVector mLumEquation;
	FLOAT mEdgeThresold;
	UINT mSizeX;
	UINT mSizeY;
	UBOOL mbUseSceneColorLdr;
	FTexture mSceneColor;
	const FSceneView* mView;
	FVector4 m_Viewport;

	FArkPpMlaaParameters()
		: mLumEquation(0.0f,0.0f,0.0f)
		, mEdgeThresold(1.0f)
		, mSizeX(0)
		, mSizeY(0)
		, mbUseSceneColorLdr(FALSE)
		, mView(NULL)
		, m_Viewport(0.0f,0.0f,0.0f,0.0f)
	{}
};

/*-----------------------------------------------------------------------------
	FMLAAVertexShader
-----------------------------------------------------------------------------*/
class FMLAAVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FMLAAVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return IsPCPlatform(Platform);
	}

	/** Default constructor. */
	FMLAAVertexShader() {}

	/** Initialization constructor. */
	FMLAAVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		InvTextureSizeParameter.Bind(Initializer.ParameterMap, TEXT("InvTextureSize"), TRUE);
		TexCoordScaleBiasParameter.Bind(Initializer.ParameterMap, TEXT("TexCoordScaleBias"), TRUE);
	}

	/**
	 * Serializer
	 * DISHONORED(layout): the cooked FMLAAVertexShader is Arkane's (arkppnodeaa.cpp, FArkPpNodeAAProxy): two parameters
	 * (2013 rva 0x50e170 SetParameters: (1/w, 1/h) @108 and (scale.xy, offset.xy) of the source rectangle @114; Serialize
	 * 0x411fe0). The names are not recoverable from the shipping exe. The reference MLAA pass that binds this type is
	 * never run (FSceneRenderer::RenderFinish); the Arkane AA node comes with the FArkPp graph.
	 */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << InvTextureSizeParameter;
		Ar << TexCoordScaleBiasParameter;
		return bShaderHasOutdatedParameters;
	}

	/** DISHONORED(port): 2013 rva 0x50e170 (2012 0x54e900) - arkppnodeaa.cpp. */
	void SetParameters(const FArkPpMlaaParameters& iParams);

	FShaderParameter InvTextureSizeParameter;
	FShaderParameter TexCoordScaleBiasParameter;
};

/*-----------------------------------------------------------------------------
	FMLAAComputeLineLengthPixelShader
-----------------------------------------------------------------------------*/
class FMLAAComputeLineLengthPixelShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FMLAAComputeLineLengthPixelShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return IsPCPlatform(Platform);
	}

	/** Default constructor. */
	FMLAAComputeLineLengthPixelShader() {}

	/** Initialization constructor. */
	FMLAAComputeLineLengthPixelShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		EdgeMaskTextureParameter.Bind(Initializer.ParameterMap, TEXT("EdgeMaskTexture"), TRUE);
		MLAAParameter.Bind(Initializer.ParameterMap,TEXT("gParam"),TRUE);
	}

	/** Serializer */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << EdgeMaskTextureParameter;
		Ar << MLAAParameter;
		return bShaderHasOutdatedParameters;
	}

	FShaderResourceParameter EdgeMaskTextureParameter;
	FShaderParameter MLAAParameter;
};

/*-----------------------------------------------------------------------------
	FFXAAVertexShader
-----------------------------------------------------------------------------*/
class FFXAAVertexShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FFXAAVertexShader,Global);
public:

	static UBOOL ShouldCache(EShaderPlatform Platform)
	{
		return TRUE;
	}

	/** Default constructor. */
	FFXAAVertexShader() {}

	/** Initialization constructor. */
	FFXAAVertexShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer):
		FGlobalShader(Initializer)
	{
		fxaaQualityRcpFrameParameter.Bind(Initializer.ParameterMap, TEXT("fxaaQualityRcpFrame"), TRUE);
		TexCoordScaleBiasParameter.Bind(Initializer.ParameterMap, TEXT("TexCoordScaleBias"), TRUE);
	}

	/**
	 * Serializer
	 * DISHONORED(layout): the cooked FFXAAVertexShader is Arkane's (arkppnodeaa.cpp): two parameters (2013 rva 0x50e090
	 * SetParameters: (1/w, 1/h) @108 and the source rectangle scale/offset @114), same shape as FMLAAVertexShader above.
	 */
	virtual UBOOL Serialize(FArchive& Ar)
	{
		UBOOL bShaderHasOutdatedParameters = FShader::Serialize(Ar);
		Ar << fxaaQualityRcpFrameParameter;
		Ar << TexCoordScaleBiasParameter;
		return bShaderHasOutdatedParameters;
	}

	/** DISHONORED(port): 2013 rva 0x50e090 (2012 0x54e7b0) - arkppnodeaa.cpp. */
	void SetParameters(const FArkPpFxAaParameters& iParams);

	FShaderParameter fxaaQualityRcpFrameParameter;
	FShaderParameter TexCoordScaleBiasParameter;
};

#endif // _INC_ARKPPNODEAA
