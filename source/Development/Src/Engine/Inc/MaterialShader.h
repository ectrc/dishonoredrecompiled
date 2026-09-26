/*=============================================================================
	MaterialShader.h: Material shader definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "ImageReflectionRendering.h"
#include "DepthOfFieldCommon.h"				// FDepthOfFieldParams, FDOFShaderParameters

/**
 * DISHONORED(retail): the retail material shader gates. UShaderCache::Load (2013 rva 0x164a60) skips a material shader map whose
 * saved version is below 786 / licensee 23 (`v55 < 786 || v57 < 23`). The material shader types are constructed with
 * MinPackageVersion 786 (0x312, e.g. TDepthOnlyVertexShader 0xb7f540, FHitProxyVertexShader 0xb815e0), 792 (0x318, the
 * TLight* types 0xb6f2d0..0xb71920), 798 (0x31e, TBasePassVertexShader<FNoLightMapPolicy> 0xb7ea00 /
 * TBasePassPixelShader<FNoLightMapPolicy,0,0> 0xb7ea40, TDepthOnlySolidPixelShader 0xb7f580, TLightMapDensity* 0xb6ff50..)
 * or 801 (0x321, TPpMaterialPixelShader 0xb83190) and licensee 23 (0x17; 24 for the TSoulPartMesh* types 0xb8efe0 / 0xb8f020);
 * build\agentAG\statictypes2013.csv lists every initializer. The cooked content is 801 / 30, so every retail gate passes; the
 * reference VER_INVALIDATE_SHADERCACHE5 (836) rejected every cooked map.
 */
#define VER_MIN_MATERIALSHADERMAP				786
/** The minimum package version to load material pixel shaders with. */
#define VER_MIN_MATERIAL_PIXELSHADER			786
/** The minimum package version to load material vertex shaders with. */
#define VER_MIN_MATERIAL_VERTEXSHADER			786

/** Same as VER_MIN_MATERIALSHADERMAP, but for the licensee package version. */
#define LICENSEE_VER_MIN_MATERIALSHADERMAP		23
/** Same as VER_MIN_MATERIAL_PIXELSHADER, but for the licensee package version. */
#define LICENSEE_VER_MIN_MATERIAL_PIXELSHADER	23
/** Same as VER_MIN_MATERIAL_VERTEXSHADER, but for the licensee package version. */
#define LICENSEE_VER_MIN_MATERIAL_VERTEXSHADER	23

/** A macro to implement material shaders which checks the package version for VER_MIN_MATERIAL_*SHADER and LICENSEE_VER_MIN_MATERIAL_*SHADER. */
#define IMPLEMENT_MATERIAL_SHADER_TYPE(TemplatePrefix,ShaderClass,SourceFilename,FunctionName,Frequency,MinPackageVersion,MinLicenseePackageVersion) \
	IMPLEMENT_SHADER_TYPE( \
		TemplatePrefix, \
		ShaderClass, \
		SourceFilename, \
		FunctionName, \
		Frequency, \
		Max((UINT)MinPackageVersion,Frequency == SF_Pixel ? Max((UINT)VER_MIN_COMPILEDMATERIAL, (UINT)VER_MIN_MATERIAL_PIXELSHADER) : Max((UINT)VER_MIN_COMPILEDMATERIAL, (UINT)VER_MIN_MATERIAL_VERTEXSHADER)), \
		Max((UINT)MinLicenseePackageVersion,Frequency == SF_Pixel ? Max((UINT)LICENSEE_VER_MIN_COMPILEDMATERIAL, (UINT)LICENSEE_VER_MIN_MATERIAL_PIXELSHADER) : Max((UINT)LICENSEE_VER_MIN_COMPILEDMATERIAL, (UINT)LICENSEE_VER_MIN_MATERIAL_VERTEXSHADER)) \
		);

/** Converts an EMaterialLightingModel to a string description. */
extern FString GetLightingModelString(EMaterialLightingModel LightingModel);

/** Converts an EBlendMode to a string description. */
extern FString GetBlendModeString(EBlendMode BlendMode);

/** Called for every material shader to update the appropriate stats. */
extern void UpdateMaterialShaderCompilingStats(const FMaterial* Material);

/**
 * Dump material stats for a given platform.
 * 
 * @param	Platform	Platform to dump stats for.
 */
extern void DumpMaterialStats( EShaderPlatform Platform );

template<typename ParameterType> 
struct TUniformParameter
{
	INT Index;
	ParameterType ShaderParameter;
	friend FArchive& operator<<(FArchive& Ar,TUniformParameter<ParameterType>& P)
	{
		return Ar << P.Index << P.ShaderParameter;
	}
};

typedef TArray<TUniformParameter<FShaderParameter> > FUniformShaderParameterArray;
typedef TArray<TUniformParameter<FShaderResourceParameter> > FUniformShaderResourceParameterArray;

/**
 * Base class of the material parameters for a shader.
 * DISHONORED(layout): retail FMaterialShaderParameters is 36 bytes = the six parameters below in this order (2012 PDB;
 * 2013 rva 0x3deca0 operator<<(FMaterialVertexShaderParameters&) serializes them at +0..+30 first). The reference
 * LocalToWorld / WorldToLocal / WorldToView / InvViewProjection / ViewProjection parameters exist in the pixel class only,
 * ViewToWorld, TemporalAA, ActorWorldPosition and the DOF parameters do not exist, and the uniform parameter arrays belong
 * to the frequency classes (pixel: scalar, vector, 2D, cube; vertex: scalar, vector).
 */
class FMaterialShaderParameters
{
public:

	void Bind(const FShaderParameterMap& ParameterMap);

	static void BindUniformParameters(
		const FShaderParameterMap& ParameterMap,
		EShaderFrequency Frequency,
		FUniformShaderParameterArray& UniformScalarShaderParameters,
		FUniformShaderParameterArray& UniformVectorShaderParameters,
		FUniformShaderResourceParameterArray* Uniform2DShaderResourceParameters);

	/** Sets the uniform expression parameters and the camera position (material specific but not FMeshBatch specific). */
	template<typename ShaderRHIParamRef>
	void SetShader(
		const ShaderRHIParamRef ShaderRHI,
		const FShaderFrequencyUniformExpressions& InExpressions,
		const FMaterialRenderContext& MaterialRenderContext,
		FShaderFrequencyUniformExpressionValues& InValues,
		const FUniformShaderParameterArray& UniformScalarShaderParameters,
		const FUniformShaderParameterArray& UniformVectorShaderParameters,
		const FUniformShaderResourceParameterArray* Uniform2DShaderResourceParameters) const;

	/**
	* Set the material shader parameters which depend on the mesh element being rendered.
	* @param Shader - The shader to set the parameters for.
	* @param View - The view that is being rendered.
	* @param Mesh - The mesh that is being rendered.
	*/
	template<typename ShaderRHIParamRef>
	void SetMeshShader(
		const ShaderRHIParamRef& Shader,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshBatch& Mesh,
		INT BatchElementIndex,
		const FSceneView& View
		) const;

	static UBOOL AreUniformParametersValid(
		const FShaderFrequencyUniformExpressions& UniformExpressions,
		const FUniformShaderParameterArray& UniformScalarShaderParameters,
		const FUniformShaderParameterArray& UniformVectorShaderParameters,
		const FUniformShaderResourceParameterArray* Uniform2DShaderResourceParameters);

	friend FArchive& operator<<(FArchive& Ar,FMaterialShaderParameters& Parameters);

protected:
	/** world-space camera position */
	FShaderParameter CameraWorldPositionParameter;
	/** Primitive component bounds origin and radius. */
	FShaderParameter ObjectWorldPositionAndRadiusParameter;
	FShaderParameter ObjectOrientationParameter;
	FShaderParameter WindDirectionAndSpeedParameter;
	FShaderParameter FoliageImpulseDirectionParameter;
	FShaderParameter FoliageNormalizedRotationAxisAndAngleParameter;
};

/**
 * DISHONORED(layout): Arkane's world cube map binding, 6 bytes = one FShaderResourceParameter (2012 PDB
 * FWorldCubeMapTextureShaderParameters, member of FMaterialPixelShaderParameters @144). Set (2012 rva 0x4715a0,
 * scenerendertargets.cpp:1977) samples FSceneView::SceneReflectionTexture->Resource when the parameter is bound. The shader
 * parameter name is not in the shipping exes (no compiler, no Bind); the binding only ever comes from the cooked caches.
 */
class FWorldCubeMapTextureShaderParameters
{
public:
	void Bind(const FShaderParameterMap& ParameterMap);
	void Set(const FSceneView* View, FShader* PixelShader) const;
	friend FArchive& operator<<(FArchive& Ar,FWorldCubeMapTextureShaderParameters& P);
private:
	FShaderResourceParameter CubemapTextureParameter;
};

/**
 * An encapsulation of the material parameters for a pixel shader.
 * DISHONORED(layout): retail FMaterialPixelShaderParameters is 192 bytes (2012 PDB): the 36-byte base, the four uniform
 * parameter arrays @36..84, LocalToWorld @84, WorldToLocal @90, WorldToView @96, InvViewProjection @102, ViewProjection @108,
 * FSceneTextureShaderParameters @114 (30 bytes), FWorldCubeMapTextureShaderParameters @144, TwoSidedSign @150, InvGamma @156,
 * DecalFarPlaneDistance @162, ObjectPostProjectionPosition @168, ObjectNDCPosition @174, ObjectMacroUVScales @180,
 * OcclusionPercentage @186. No screen-door fade, alpha sample, fluid detail normal or DOF parameters.
 */
class FMaterialPixelShaderParameters : public FMaterialShaderParameters
{
public:

	void Bind(const FShaderParameterMap& ParameterMap);

	/** Sets pixel parameters that are material specific but not FMeshBatch specific. */
	void Set(FShader* PixelShader,const FMaterialRenderContext& MaterialRenderContext, ESceneDepthUsage DepthUsage = SceneDepthUsage_Normal) const;

	/**
	* Set the material shader parameters which depend on the mesh element being rendered.
	* @param PixelShader - The pixel shader to set the parameters for.
	* @param View - The view that is being rendered.
	* @param Mesh - The mesh that is being rendered.
	* @param bBackFace - True if the backfaces of a two-sided material are being rendered.
	*/
	void SetMesh(
		FShader* PixelShader,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshBatch& Mesh,
		INT BatchElementIndex,
		const FSceneView& View,
		UBOOL bBackFace
		) const;

	friend FArchive& operator<<(FArchive& Ar,FMaterialPixelShaderParameters& Parameters);

	UBOOL IsUniformExpressionSetValid(const FUniformExpressionSet& InSet) const;

private:
	FUniformShaderParameterArray UniformPixelScalarShaderParameters;
	FUniformShaderParameterArray UniformPixelVectorShaderParameters;
	FUniformShaderResourceParameterArray UniformPixel2DShaderResourceParameters;
	FUniformShaderResourceParameterArray UniformPixelCubeShaderResourceParameters;
	/** matrix parameter for materials with a world transform */
	FShaderParameter LocalToWorldParameter;
	/** matrix parameter for materials with a local transform */
	FShaderParameter WorldToLocalParameter;
	/** matrix parameter for materials with a view transform */
	FShaderParameter WorldToViewParameter;
	/** matrix parameter for materials with a world position transform */
	FShaderParameter InvViewProjectionParameter;
	/** matrix parameter for materials with a world position node */
	FShaderParameter ViewProjectionParameter;
	/** The scene texture parameters. */
	FSceneTextureShaderParameters SceneTextureParameters;
	FWorldCubeMapTextureShaderParameters WorldCubemapParameters;
	/** Parameter indicating whether the front-side or the back-side of a two-sided material is being rendered. */
	FShaderParameter TwoSidedSignParameter;
	/** Inverse gamma parameter. Only used when USE_GAMMA_CORRECTION 1 */
	FShaderParameter InvGammaParameter;
	/** Parameter for distance to far plane for the decal (local or world space) */
	FShaderParameter DecalFarPlaneDistanceParameter;
	/** Object position in post projection space. */
	FShaderParameter ObjectPostProjectionPositionParameter;
	/** Object position in Normalized Device Coordinates. */
	FShaderParameter ObjectNDCPositionParameter;
	/** Scales to turn object position in post projection space into UVs in xy, NDC position into UVs in zw. */
	FShaderParameter ObjectMacroUVScalesParameter;
	/** Parameter for occlusion percentage of the object being rendered */
	FShaderParameter OcclusionPercentageParameter;
};

#if WITH_D3D11_TESSELLATION

/**
 * DISHONORED(retail): the domain and hull parameter classes are reference-only (retail has no D3D11 stages, no cooked
 * SF_Hull / SF_Domain record exists); kept as vertex-shaped shells so the tessellation shader types still compile.
 */
class FMaterialDomainShaderParameters : public FMaterialShaderParameters
{
public:

	void Bind(const FShaderParameterMap& ParameterMap);

	/** Sets domain shader parameters that are material specific but not FMeshBatch specific. */
	void Set(FShader* DomainShader,const FMaterialRenderContext& MaterialRenderContext) const;

	void SetMesh(
		FShader* DomainShader,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshBatch& Mesh,
		INT BatchElementIndex,
		const FSceneView& View
		) const;

	UBOOL IsUniformExpressionSetValid(const FUniformExpressionSet& ExpressionSet) const
	{
		return AreUniformParametersValid(ExpressionSet.GetExpresssions(SF_Domain), UniformScalarShaderParameters, UniformVectorShaderParameters, NULL);
	}

	friend FArchive& operator<<(FArchive& Ar,FMaterialDomainShaderParameters& Parameters);

private:
	FUniformShaderParameterArray UniformScalarShaderParameters;
	FUniformShaderParameterArray UniformVectorShaderParameters;
};

class FMaterialHullShaderParameters : public FMaterialShaderParameters
{
public:

	void Bind(const FShaderParameterMap& ParameterMap);

	/** Sets hull shader parameters that are material specific but not FMeshBatch specific. */
	void Set(FShader* HullShader,const FMaterialRenderContext& MaterialRenderContext) const;

	void SetMesh(
		FShader* HullShader,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshBatch& Mesh,
		INT BatchElementIndex,
		const FSceneView& View
		) const;

	UBOOL IsUniformExpressionSetValid(const FUniformExpressionSet& ExpressionSet) const
	{
		return AreUniformParametersValid(ExpressionSet.GetExpresssions(SF_Hull), UniformScalarShaderParameters, UniformVectorShaderParameters, NULL);
	}

	friend FArchive& operator<<(FArchive& Ar,FMaterialHullShaderParameters& Parameters);

private:
	FUniformShaderParameterArray UniformScalarShaderParameters;
	FUniformShaderParameterArray UniformVectorShaderParameters;
};

#endif

/**
 * An encapsulation of the material parameters for a vertex shader.
 * DISHONORED(layout): retail FMaterialVertexShaderParameters is 60 bytes (2012 PDB): the 36-byte base, then the
 * UniformVertexScalar @36 and UniformVertexVector @48 arrays (2013 rva 0x3deca0 serializes exactly these); no 2D texture
 * expressions in vertex shaders, no transform matrices (the vertex factory provides them).
 */
class FMaterialVertexShaderParameters : public FMaterialShaderParameters
{
public:

	void Bind(const FShaderParameterMap& ParameterMap);

	/** Sets vertex parameters that are material specific but not FMeshBatch specific. */
	void Set(FShader* VertexShader,const FMaterialRenderContext& MaterialRenderContext) const;

	/**
	 * Set the material shader parameters which depend on the mesh element being rendered.
	 * @param View - The view that is being rendered.
	 * @param Mesh - The mesh that is being rendered.
	 */
	void SetMesh(
		FShader* VertexShader,
		const FPrimitiveSceneInfo* PrimitiveSceneInfo,
		const FMeshBatch& Mesh,
		INT BatchElementIndex,
		const FSceneView& View
		) const;

	UBOOL IsUniformExpressionSetValid(const FUniformExpressionSet& ExpressionSet) const
	{
		return AreUniformParametersValid(ExpressionSet.GetExpresssions(SF_Vertex), UniformVertexScalarShaderParameters, UniformVertexVectorShaderParameters, NULL);
	}

	friend FArchive& operator<<(FArchive& Ar,FMaterialVertexShaderParameters& Parameters);

private:
	FUniformShaderParameterArray UniformVertexScalarShaderParameters;
	FUniformShaderParameterArray UniformVertexVectorShaderParameters;
};

/**
 * A shader meta type for material-linked shaders.
 */
class FMaterialShaderType : public FShaderType
{
public:

	/**
	 * Finds a FMaterialShaderType by name.
	 */
	static FMaterialShaderType* GetTypeByName(const FString& TypeName);

	struct CompiledShaderInitializerType : FGlobalShaderType::CompiledShaderInitializerType
	{
		CompiledShaderInitializerType(
			FShaderType* InType,
			const FShaderCompilerOutput& CompilerOutput
			):
			FGlobalShaderType::CompiledShaderInitializerType(InType,CompilerOutput)
		{}
	};
	typedef FShader* (*ConstructCompiledType)(const CompiledShaderInitializerType&);
	typedef UBOOL (*ShouldCacheType)(EShaderPlatform,const FMaterial*);

	FMaterialShaderType(
		const TCHAR* InName,
		const TCHAR* InSourceFilename,
		const TCHAR* InFunctionName,
		DWORD InFrequency,
		INT InMinPackageVersion,
		INT InMinLicenseePackageVersion,
		ConstructSerializedType InConstructSerializedRef,
		ConstructCompiledType InConstructCompiledRef,
		ModifyCompilationEnvironmentType InModifyCompilationEnvironmentRef,
		ShouldCacheType InShouldCacheRef
		):
		FShaderType(InName,InSourceFilename,InFunctionName,InFrequency,InMinPackageVersion,InMinLicenseePackageVersion,InConstructSerializedRef,InModifyCompilationEnvironmentRef),
		ConstructCompiledRef(InConstructCompiledRef),
		ShouldCacheRef(InShouldCacheRef)
	{}

	/**
	 * Enqueues a compilation for a new shader of this type.
	 * @param Material - The material to link the shader with.
	 * @param MaterialShaderCode - The shader code for the material.
	 */
	void BeginCompileShader(
		UINT ShaderMapId,
		const FMaterial* Material,
		const ANSICHAR* MaterialShaderCode,
		EShaderPlatform Platform
		);

	/**
	 * Either creates a new instance of this type or returns an equivalent existing shader.
	 * @param Material - The material to link the shader with.
	 * @param CurrentJob - Compile job that was enqueued by BeginCompileShader.
	 */
	FShader* FinishCompileShader(
		const FUniformExpressionSet& UniformExpressionSet,
		const FShaderCompileJob& CurrentJob
		);

	/**
	 * Checks if the shader type should be cached for a particular platform and material.
	 * @param Platform - The platform to check.
	 * @param Material - The material to check.
	 * @return True if this shader type should be cached.
	 */
	UBOOL ShouldCache(EShaderPlatform Platform,const FMaterial* Material) const
	{
		return (*ShouldCacheRef)(Platform,Material);
	}

	// Dynamic casting.
	virtual FMaterialShaderType* GetMaterialShaderType() { return this; }

private:
	ConstructCompiledType ConstructCompiledRef;
	ShouldCacheType ShouldCacheRef;
};

/**
 * DISHONORED(bringup): inventory of the cooked material shader maps (PHASE6 package AG accept line). Every shader reference of a
 * loaded map is one of: loaded (declared type, cooked record loaded), undeclared (no FShaderType of that name) or missing
 * (declared type whose cooked record was rejected by FShaderLoadArchive, i.e. a parameter layout mismatch, or skipped). Each
 * problem type is named once; FMaterial::InitShaderMap prints the totals whenever the number of loaded maps changed.
 */
struct FDishonoredShaderMapLoadStats
{
	INT NumMaterialShaderMaps;
	INT NumMeshShaderMapsWithoutVertexFactory;
	INT NumShaderRefs;
	INT NumUndeclaredRefs;
	INT NumMissingRefs;
	INT NumSkippedRefs;
	INT NumReportedMaps;
	TSet<FName> UndeclaredTypes;
	TSet<FName> MissingTypes;
	/** References of the mesh shader map being read; committed or discarded once its vertex factory name follows. */
	TArray<FName> PendingUndeclared;
	TArray<FName> PendingMissing;
	INT NumPendingRefs;

	FDishonoredShaderMapLoadStats():
		NumMaterialShaderMaps(0),
		NumMeshShaderMapsWithoutVertexFactory(0),
		NumShaderRefs(0),
		NumUndeclaredRefs(0),
		NumMissingRefs(0),
		NumSkippedRefs(0),
		NumReportedMaps(0),
		NumPendingRefs(0)
	{}

	static FDishonoredShaderMapLoadStats& Get();
	void NoteShaderRef(const FName& TypeName, FShaderType* Type, FShader* Shader);
	/**
	 * bHasVertexFactory FALSE = the map named a vertex factory this game does not have, so retail drops the whole map and
	 * every record in it (FindVertexFactoryType returns NULL, VertexFactory.cpp): those references are skips, not mismatches.
	 */
	void CommitPending(UBOOL bHasVertexFactory, const FString& MapName);
	void ReportIfChanged();
};

/**
 * DISHONORED(bringup): loads the TMap<FShaderType*,TRefCountPtr<FShader> > of a shader map by hand so the type names of the
 * cooked entries are known to the inventory. The format is the reference TMap one: INT Num, then per pair the key
 * (FShaderType* as an FName, 2013 rva 0x160310) and the value (TRefCountPtr<FShader> = FShader*: the FGuid Id followed by
 * the type name, 2013 rva 0x160590, resolved through FShaderType::FindShaderById). Entries whose type is undeclared or whose
 * shader did not load are dropped, as the reference TMap load would keep them as NULL pairs.
 */
template<typename ShaderMetaType>
void DishonoredLoadShaderMap(FArchive& Ar, TShaderMap<ShaderMetaType>& ShaderMap)
{
	INT NumEntries = 0;
	Ar << NumEntries;
	for (INT EntryIndex = 0; EntryIndex < NumEntries; EntryIndex++)
	{
		FName KeyTypeName = NAME_None;
		Ar << KeyTypeName;
		FGuid ShaderId;
		Ar << ShaderId;
		FName ValueTypeName = NAME_None;
		Ar << ValueTypeName;
		FShaderType* Type = FindShaderTypeByName(*ValueTypeName.ToString());
		FShader* Shader = Type ? Type->FindShaderById(ShaderId) : NULL;
		FDishonoredShaderMapLoadStats::Get().NoteShaderRef(ValueTypeName, Type, Shader);
		if (Type && Shader)
		{
			ShaderMap.AddShader((ShaderMetaType*)Type, Shader);
		}
	}
}

/**
 * The set of material shaders for a single material.
 */
class FMaterialShaderMap : public TShaderMap<FMaterialShaderType>, public FRefCountedObject
{
public:

	/**
	 * Finds the shader map for a material.
	 * @param StaticParameterSet - The static parameter set identifying the shader map
	 * @param Platform - The platform to lookup for
	 * @return NULL if no cached shader map was found.
	 */
	static FMaterialShaderMap* FindId(const FStaticParameterSet& StaticParameterSet, EShaderPlatform Platform);

	/** Flushes the given shader types from any loaded FMaterialShaderMap's. */
	static void FlushShaderTypes(TArray<FShaderType*>& ShaderTypesToFlush, TArray<const FVertexFactoryType*>& VFTypesToFlush);

	FMaterialShaderMap() :
		CompilingId(1),
		bRegistered(FALSE),
		bCompilationFinalized(TRUE),
		bCompiledSuccessfully(TRUE),
		bIsPersistent(TRUE)
	{}

	// Destructor.
	~FMaterialShaderMap();

	/**
	 * Compiles the shaders for a material and caches them in this shader map.
	 * @param Material - The material to compile shaders for.
	 * @param InStaticParameters - the set of static parameters to compile for
	 * @param MaterialShaderCode - The shader code for Material.
	 * @param Platform - The platform to compile to
	 * @param OutErrors - Upon compilation failure, OutErrors contains a list of the errors which occured.
	 * @param bDebugDump - Dump out the preprocessed and disassembled shader for debugging.
	 * @return True if the compilation succeeded.
	 */
	UBOOL Compile(
		FMaterial* Material,
		const FStaticParameterSet* InStaticParameters, 
		const TCHAR* MaterialShaderCode,
		const FUniformExpressionSet& InUniformExpressionSet,
		EShaderPlatform Platform,
		TArray<FString>& OutErrors,
		UBOOL bDebugDump = FALSE);

	/**
	 * Checks whether the material shader map is missing any shader types necessary for the given material.
	 * @param Material - The material which is checked.
	 * @return True if the shader map has all of the shader types necessary.
	 */
	UBOOL IsComplete(const FMaterial* Material, UBOOL bSilent) const;

	/** Returns TRUE if all the shaders in this shader map have their compressed shader code in Cache. */
	UBOOL IsCompressedShaderCacheComplete(const FCompressedShaderCodeCache* const Cache) const;

	UBOOL IsUniformExpressionSetValid() const;

	/**
	 * Builds a list of the shaders in a shader map.
	 */
	void GetShaderList(TMap<FGuid,FShader*>& OutShaders) const;

	/**
	 * Begins initializing the shaders used by the material shader map.
	 */
	void BeginInit();

	/**
	 * Begins releasing the shaders used by the material shader map.
	 */
	void BeginRelease();

	/**
	 * Registers a material shader map in the global map so it can be used by materials.
	 */
	void Register();

	FMaterialShaderMap* AttemptRegistration();

	/**
	 * Merges in OtherMaterialShaderMap's shaders and FMeshMaterialShaderMaps
	 */
	void Merge(const FMaterialShaderMap* OtherMaterialShaderMap);

	/**
     * AddGuidAliases - finds corresponding guids and adds them to the FShaders alias list
	 * @param OtherMaterialShaderMap contains guids that will exist in a compressed shader cache, but will not necessarily have FShaders
	 * @return FALSE if these two shader maps are not compatible
	 */
	UBOOL AddGuidAliases(const FMaterialShaderMap* OtherMaterialShaderMap);

	/**
	 * Removes all entries in the cache with exceptions based on a shader type
	 * @param ShaderType - The shader type to flush
	 */
	void FlushShadersByShaderType(FShaderType* ShaderType);

	/**
	 * Removes all entries in the cache with exceptions based on a vertex factory type
	 * @param ShaderType - The shader type to flush
	 */
	void FlushShadersByVertexFactoryType(const FVertexFactoryType* VertexFactoryType);

	// Serializer.
	void Serialize(FArchive& Ar);

	// Accessors.
	const class FMeshMaterialShaderMap* GetMeshShaderMap(FVertexFactoryType* VertexFactoryType) const;
	const FStaticParameterSet& GetMaterialId() const { return StaticParameters; }
	EShaderPlatform GetShaderPlatform() const { return Platform; }
	const FString& GetFriendlyName() const { return FriendlyName; }
	UINT GetCompilingId() const { return CompilingId; }
	UBOOL IsCompilationFinalized() const { return bCompilationFinalized; }
	UBOOL CompiledSuccessfully() const { return bCompiledSuccessfully; }

	const TArray<TRefCountPtr<FMaterialUniformExpressionTexture> >& GetUniform2DTextureExpressions() const { return UniformExpressionSet.PixelExpressions.Uniform2DTextureExpressions; }
	const TArray<TRefCountPtr<FMaterialUniformExpressionTexture> >& GetUniformCubeTextureExpressions() const { return UniformExpressionSet.UniformCubeTextureExpressions; }
	const FUniformExpressionSet& GetUniformExpressionSet() const { return UniformExpressionSet; }
	void SetUniformExpressions(const FUniformExpressionSet& InSet) { UniformExpressionSet = InSet; }

	/** Removes a material from ShaderMapsBeingCompiled. */
	static void RemovePendingMaterial(FMaterial* Material);

	UBOOL IdenticalToCompressedCache(const FCompressedShaderCodeCache* TestCache) const
	{
		return CompressedCache == TestCache;
	}

private:

	/** A global map from a material's static parameter set to any shader map cached for that material. */
	static TMap<FStaticParameterSet,FMaterialShaderMap*> GIdToMaterialShaderMap[SP_NumPlatforms];

	/** The material's cached shaders for vertex factory type dependent shaders. */
	TIndirectArray<class FMeshMaterialShaderMap> MeshShaderMaps;

	/** The material's mesh shader maps, indexed by VFType->GetId(), for fast lookup at runtime. */
	TArray<FMeshMaterialShaderMap*> OrderedMeshShaderMaps;

	/** 
	 * Compressed shader cache used with this material's shaders.  The cache is not accessed through this member, 
	 * But the ref counted pointer keeps the cache around as long as the shader map's shaders need it.
	 */
	TRefCountPtr<FCompressedShaderCodeCache> CompressedCache;

	/** The persistent GUID of this material shader map. */
	FGuid MaterialId;

	/** The material's user friendly name, typically the object name. */
	FString FriendlyName;

	/** The platform this shader map was compiled with */
	EShaderPlatform Platform;

	/** The static parameter set that this shader map was compiled with */
	FStaticParameterSet StaticParameters;

	/** Uniform expressions generated from the material compile. */
	FUniformExpressionSet UniformExpressionSet;

	/** Next value for CompilingId. */
	static UINT NextCompilingId;

	/** Tracks material resources and their shader maps that need to be compiled but whose compilation is being deferred. */
	static TMap<FMaterialShaderMap*, TArray<FMaterial*> > ShaderMapsBeingCompiled;

	/** 
	 * Map from shader map CompilingId to an ANSI version of the material's shader code.
	 * This is stored outside of the shader's environment includes to reduce memory usage, since many shaders share the same material shader code.
	 */
	static TMap<UINT, const ANSICHAR*> MaterialCodeBeingCompiled;

	/** Uniquely identifies this shader map during compilation, needed for deferred compilation where shaders from multiple shader maps are compiled together. */
	UINT CompilingId;

	/** Indicates whether this shader map has been registered in GIdToMaterialShaderMap */
	BITFIELD bRegistered : 1;

	/** 
	 * Indicates whether this shader map has had ProcessCompilationResults called after Compile.
	 * The shader map must not be used on the rendering thread unless bCompilationFinalized is TRUE.
	 */
	BITFIELD bCompilationFinalized : 1;

	BITFIELD bCompiledSuccessfully : 1;

	/** Indicates whether the shader map should be stored in the shader cache. */
	BITFIELD bIsPersistent : 1;

	/**
	 * Initializes OrderedMeshShaderMaps from the contents of MeshShaderMaps.
	 */
	void InitOrderedMeshShaderMaps();

	/** 
	 * Processes an array of completed shader compile jobs. 
	 * This is called by FShaderCompilingThreadManager after compilation of this shader map's shaders has completed.E.
	 */
	void ProcessCompilationResults(const TArray<TRefCountPtr<FShaderCompileJob> >& InCompilationResults, UBOOL bSuccess);

	friend class UShaderCache;
	friend void DumpMaterialStats( EShaderPlatform Platform );
	friend class FShaderCompilingThreadManager;
};
