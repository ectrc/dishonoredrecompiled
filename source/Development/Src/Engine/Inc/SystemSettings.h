/*=============================================================================
	SystemSettings.cpp: Unreal engine HW compat scalability system.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __SYSTEMSETTINGS_H__
#define __SYSTEMSETTINGS_H__

/*-----------------------------------------------------------------------------
	System settings and scalability options.
-----------------------------------------------------------------------------*/

/** 
 * The type of the system setting
 */
enum ESystemSettingType
{
	SST_UNKNOWN = 0,
	SST_STRING,
	SST_INT,
	SST_ENUM,
	SST_FLOAT,
	SST_BOOL,
	SST_ANY
};

/**
 * The intent of the system setting
 */
enum ESystemSettingIntent
{
	// No idea what this setting is used for
	SSI_UNKNOWN = 0,
	// Only use this setting for debugging; do not use in a retail environment
	SSI_DEBUG,
	// Compatibility setting that needs to be set for certain hardware
	SSI_COMPATIBILITY,
	// Use the setting to adjust the performance/quality ratio
	SSI_SCALABILITY,
	// Use the setting to adjust the performance/quality ratio for mobile devices
	SSI_MOBILE_SCALABILITY,
	// Set according to the user's desire
	SSI_PREFERENCE,
};

/**
 * How to update to the next setting depending on type
 */
class FVSS
{
public:
	virtual UBOOL GetNextSetting( UBOOL CurrentSetting )
	{
		return FALSE;
	}

	virtual INT GetNextSetting( INT CurrentSetting )
	{
		return 0;
	}

	virtual FLOAT GetNextSetting( FLOAT CurrentSetting )
	{
		return 0.0f;
	}

	virtual INT GetMinIntSetting( void )
	{
		return 0;
	}

	virtual INT GetMaxIntSetting( void )
	{
		return 0;
	}

	virtual FLOAT GetMinFloatSetting( void )
	{
		return 0.0f;
	}

	virtual FLOAT GetMaxFloatSetting( void )
	{
		return 0.0f;
	}

	virtual INT GetSettingCount( void )
	{
		return 1;
	}
};

class FVSSBool : public FVSS
{
	virtual UBOOL GetNextSetting( UBOOL CurrentSetting )
	{
		return !CurrentSetting;
	}
};

class FVSSGenericInt : public FVSS
{
public:
	FVSSGenericInt( INT InMin, INT InMax, INT InStep )
	{
		Min = InMin;
		Max = InMax;
		Step = InStep;
		if( Step == 0 )
		{
			Step = 1;
		}
	}

	virtual INT GetNextSetting( INT CurrentSetting )
	{
		if( CurrentSetting + Step > Max )
		{
			return Min;
		}

		return CurrentSetting + Step;
	}

	virtual INT GetMinIntSetting( void )
 {
		return Min;
	}

	virtual INT GetMaxIntSetting( void )
	{
		return Max;
	}

	virtual INT GetSettingCount( void )
	{
		return ( Max - Min ) / Step;
	}

	INT Min;
	INT Max;
	INT Step;
};

class FVSSGenericFloat : public FVSS
{
public:
	FVSSGenericFloat( FLOAT InMin, FLOAT InMax )
	{
		Min = InMin;
		Max = InMax;
		Step = ( Max - Min ) / 100.0f;
	}

	virtual FLOAT GetNextSetting( FLOAT CurrentSetting )
	{
		if( CurrentSetting + Step > Max )
		{
			return Min;
		}

		return CurrentSetting + Step;
	}

	virtual FLOAT GetMinFloatSetting( void )
	{
		return Min;
	}

	virtual FLOAT GetMaxFloatSetting( void )
	{
		return Max;
	}

	virtual INT GetSettingCount( void )
	{
		return ( Max - Min ) / Step;
	}

	FLOAT Min;
	FLOAT Max;
	FLOAT Step;
};

/**
 * The container for a single system setting
 */
class FSystemSetting
{
public:
	ESystemSettingType SettingType;
	ESystemSettingIntent SettingIntent;
	const TCHAR* SettingName;
	void* SettingAddress;
	FVSS* SettingUpdate;
	const TCHAR* SettingHelp;
	UBOOL bFound;
 };

/** Augments TextureLODSettings with access to TextureLODGroups. */
struct FExposedTextureLODSettings : public FTextureLODSettings
{
public:
	/** @return		A handle to the indexed LOD group. */
	FTextureLODGroup& GetTextureLODGroup(INT GroupIndex)
	{
		check( GroupIndex >= 0 && GroupIndex < TEXTUREGROUP_MAX );
		return TextureLODGroups[GroupIndex];
	}
	/** @return		A handle to the indexed LOD group. */
	const FTextureLODGroup& GetTextureLODGroup(INT GroupIndex) const
	{
		check( GroupIndex >= 0 && GroupIndex < TEXTUREGROUP_MAX );
		return TextureLODGroups[GroupIndex];
	}
};

/**
 * Struct that holds the actual data for the system settings.
 */
/**
 * DISHONORED(layout): the retail settings are FSystemSettingsData (2012 PDB 1048 bytes; 2013 1052: FSystemSettingsData::LoadFromIni
 * 2013 rva 0x1806c0 stores bAllowRatsShadow @76, iType_AntiAlias @128 ... SpeakerConfiguration @1048, and FSystemSettings::Initialize
 * 2013 rva 0x1844e0 copies 0x41C bytes) made of ten bases; FSystemSettings = FExec @0, FSystemSettingsData @4, RenderThreadSettings @1056,
 * bIsEditor @1100, CurrentSplitScreenLevel @1104, Defaults[5][2] @1108 (2104-byte stride) = 11628 bytes (2012 PDB 11584).
 * Offsets below are 2013 FSystemSettingsData offsets; the 2012 PDB order is the same except for the two 2013-only members.
 */
struct FSystemSettingsDataWorldDetail  // DISHONORED(layout): 2013 @0, 144 bytes
{
	INT DetailMode;
	UBOOL bUseMaxQualityMode;
	UBOOL bAllowSpeedTreeLeaves;
	UBOOL bAllowSpeedTreeFronds;
	UBOOL bAllowStaticDecals;
	UBOOL bAllowDynamicDecals;
	UBOOL bAllowUnbatchedDecals;
	FLOAT DecalCullDistanceScale;
	UBOOL bAllowDynamicLights;
	UBOOL bUseCompositeDynamicLights;
	UBOOL bAllowSHSecondaryLighting;
	UBOOL bAllowDirectionalLightMaps;
	UBOOL bAllowMotionBlur;
	UBOOL bAllowMotionBlurPause;
	UBOOL bAllowDepthOfField;
	UBOOL bAllowAmbientOcclusion;
	UBOOL bAllowBloom;
	UBOOL bUseHighQualityBloom;
	UBOOL bAllowLightShafts;
	UBOOL bAllowRatsShadow;  // DISHONORED(layout): 2013-only (key bAllowRatsShadow @76); every later WorldDetail member is 4 bytes after its 2012 PDB offset
	UBOOL bAllowDistortion;
	UBOOL bAllowFilteredDistortion;
	UBOOL bAllowParticleDistortionDropping;
	UBOOL bAllowDownsampledTranslucency;
	UBOOL bAllowLensFlares;
	UBOOL bAllowFogVolumes;
	UBOOL bAllowFloatingPointRenderTargets;
	UBOOL bAllowOneFrameThreadLag;
	INT SkeletalMeshLODBias;
	INT SkeletalLODDistanceFactorMultiplier;
	INT StaticLODDistanceFactorMultiplier;
	INT TextureForcedLODBias;
	INT iType_AntiAlias;  // DISHONORED(layout): 2013-only (key iType_AntiAlias @128); replaces the 2012 bAllowMLAA @136
	INT ParticleLODBias;
	UBOOL bAllowD3D10;
	UBOOL bAllowRadialBlur;
};

struct FSystemSettingsDataTextureDetail  // DISHONORED(layout): 2013 @144, 744 bytes
{
	FExposedTextureLODSettings TextureLODSettings;
	UBOOL bOnlyStreamInTextures;
	INT MaxAnisotropy;
	FLOAT SceneCaptureStreamingMultiplier;
	FLOAT FoliageDrawRadiusMultiplier;
};

struct FSystemSettingsDataVSync  // DISHONORED(layout): 2013 @888, 4 bytes
{
	UBOOL bUseVSync;
};

struct FSystemSettingsDataScreenPercentage  // DISHONORED(layout): 2013 @892, 8 bytes
{
	FLOAT ScreenPercentage;
	UBOOL bUpscaleScreenPercentage;
};

struct FSystemSettingsDataResolution  // DISHONORED(layout): 2013 @900, 12 bytes
{
	INT ResX;
	INT ResY;
	UBOOL bFullscreen;
};

struct FSystemSettingsDataMSAA  // DISHONORED(layout): 2013 @912, 4 bytes
{
	INT MaxMultiSamples;
};

struct FSystemSettingsDataShadowDetail  // DISHONORED(layout): 2013 @916, 104 bytes
{
	UBOOL bAllowDynamicShadows;
	UBOOL bAllowLightEnvironmentShadows;
	INT ShadowFilterQualityBias;
	INT MinShadowResolution;
	INT MinPreShadowResolution;
	INT MaxShadowResolution;
	INT MaxWholeSceneDominantShadowResolution;
	FLOAT ShadowTexelsPerPixel;
	FLOAT PreShadowResolutionFactor;
	UBOOL bEnableBranchingPCFShadows;
	UBOOL bAllowHardwareShadowFiltering;
	UBOOL bAllowBetterModulatedShadows;
	UBOOL bEnableForegroundShadowsOnWorld;
	UBOOL bEnableForegroundSelfShadowing;
	UBOOL bAllowWholeSceneDominantShadows;
	FLOAT ShadowFilterRadius;
	FLOAT ShadowDepthBias;
	FLOAT CSMSplitPenumbraScale;
	FLOAT CSMSplitSoftTransitionDistanceScale;
	FLOAT CSMSplitDepthBiasScale;
	FLOAT UnbuiltWholeSceneDynamicShadowRadius;
	INT UnbuiltNumWholeSceneDynamicShadowCascades;
	INT WholeSceneShadowUnbuiltInteractionThreshold;
	INT ShadowFadeResolution;
	INT PreShadowFadeResolution;
	FLOAT ShadowFadeExponent;
};

struct FSystemSettingsDataFracturedDetail  // DISHONORED(layout): 2013 @1020, 20 bytes
{
	UBOOL bAllowFracturedDamage;
	FLOAT NumFracturedPartsScale;
	FLOAT FractureDirectSpawnChanceScale;
	FLOAT FractureRadialSpawnChanceScale;
	FLOAT FractureCullDistanceScale;
};

struct FSystemSettingsDataMesh  // DISHONORED(layout): 2013 @1040, 8 bytes
{
	UBOOL bForceCPUAccessToGPUSkinVerts;
	UBOOL bDisableSkeletalInstanceWeights;
};

struct FSystemSettingsDataAudio  // DISHONORED(layout): 2013 @1048, 4 bytes
{
	INT SpeakerConfiguration;  // DISHONORED(layout): 2012 PDB spelling m_SpeakerConfiguration; the 2013 key is SpeakerConfiguration
};

struct FSystemSettingsData : public FSystemSettingsDataWorldDetail, public FSystemSettingsDataTextureDetail, public FSystemSettingsDataVSync, public FSystemSettingsDataScreenPercentage, public FSystemSettingsDataResolution, public FSystemSettingsDataMSAA, public FSystemSettingsDataShadowDetail, public FSystemSettingsDataFracturedDetail, public FSystemSettingsDataMesh, public FSystemSettingsDataAudio
{
};

class FSystemSettings : public FExec, public FSystemSettingsData
{
public:
	// DISHONORED(layout): 2012 PDB FSystemSettings::FRenderThreadSettings (44 bytes, @1052 in 2012, @1056 in 2013); storage only,
	// ApplySystemSettingsToRenderThread still hands the reference render thread GSystemSettings itself
	struct FRenderThreadSettings
	{
		UBOOL bAllowMotionBlur;
		UBOOL bAllowAmbientOcclusion;
		UBOOL bAllowDynamicShadows;
		UBOOL bAllowHardwareShadowFiltering;
		UBOOL bAllowFogVolumes;
		UBOOL bAllowMLAA;
		INT MaxMultiSamples;
		INT MinShadowResolution;
		INT MaxShadowResolution;
		INT MaxWholeSceneDominantShadowResolution;
		INT bAllowUnbatchedDecals;
	};
	FRenderThreadSettings RenderThreadSettings;

	/** Since System Settings is called into before GIsEditor is set, we must cache this value. */
	UBOOL bIsEditor;

	INT CurrentSplitScreenLevel;  // DISHONORED(layout): 2012 PDB ESplitScreenLevel

	// DISHONORED(layout): [i][0] = [AppCompatBucket<i+1>] of GCompatIni, [i][1] = [SystemSettingsSplitScreen2] (2013 rva 0x1844e0); not filled yet
	FSystemSettingsData Defaults[5][2];

	// DISHONORED(layout): reference-only members below are storage-less shims (DISHONORED_SHIM_STATIC): not in the retail
	// FSystemSettingsData, not read from the ini by the retail table (SystemSettings.cpp); every use is a porting TODO
	// DISHONORED(retail): MaxDrawDistanceScale removed. Retail's FSystemSettingsData has no such member and no
	// draw-distance test scales by it (ProcessVisible 2013 rva 0x45f060, ProcessPrimitiveCulling<0> 0x4626a0). As a
	// storage-less shim it read 0, which zeroed every AdjustedMaxDrawDistanceSquared and left the static draw lists
	// with nothing visible to draw.
	/** Whether to allow sub-surface scattering to render.				*/
	DISHONORED_SHIM_STATIC UBOOL	bAllowSubsurfaceScattering;
	/** Whether to allow image reflections to render.					*/
	DISHONORED_SHIM_STATIC UBOOL	bAllowImageReflections;
	/** Whether to allow image reflections to be shadowed.				*/
	DISHONORED_SHIM_STATIC UBOOL	bAllowImageReflectionShadowing;
	/** State of the console variable MotionBlurSkinning.				*/
	DISHONORED_SHIM_STATIC INT		MotionBlurSkinning;
	/** Whether to use high-precision GBuffers. */
	DISHONORED_SHIM_STATIC UBOOL	bHighPrecisionGBuffers;
	/** Whether to keep separate translucency (for better Depth of Field), experimental */
	DISHONORED_SHIM_STATIC UBOOL	bAllowSeparateTranslucency;
	/** Whether to allow post process MLAA to render. requires extra memory	*/
	DISHONORED_SHIM_STATIC UBOOL	bAllowPostprocessMLAA;
	/** Whether to use high quality materials when low quality exist	*/
	DISHONORED_SHIM_STATIC UBOOL	bAllowHighQualityMaterials;
	/** Max filter sample count (clamp can cause boxy appearance but allows for better performance, only numbers below 16 have effect)	*/
	DISHONORED_SHIM_STATIC INT		MaxFilterBlurSampleCount;
	/** Whether to use safe and conservative shadow frustum creation that wastes some shadowmap space. */
	DISHONORED_SHIM_STATIC UBOOL	bUseConservativeShadowBounds;
	/** Minimum camera FOV for CSM, this is used to prevent shadow shimmering when animating the FOV lower than the min, for example when zooming */
	DISHONORED_SHIM_STATIC FLOAT	CSMMinimumFOV;
	/** The FOV will be rounded by this factor for the purposes of CSM, which turns shadow shimmering into discrete jumps */
	DISHONORED_SHIM_STATIC FLOAT	CSMFOVRoundFactor;

	DISHONORED_SHIM_STATIC UBOOL	bAllowD3D9MSAA;
	DISHONORED_SHIM_STATIC UBOOL	bAllowTemporalAA;
	DISHONORED_SHIM_STATIC FLOAT	TemporalAA_MinDepth;
	DISHONORED_SHIM_STATIC FLOAT	TemporalAA_StartDepthVelocityScale;

	/** Whether to allow independent, external displays */
	DISHONORED_SHIM_STATIC UBOOL bAllowSecondaryDisplays;
	/** The maximum width and height of any potentially allowed secondary displays (requires bAllowSecondaryDisplays == TRUE) */
	DISHONORED_SHIM_STATIC INT SecondaryDisplayMaximumWidth;
	DISHONORED_SHIM_STATIC INT SecondaryDisplayMaximumHeight;

	/** Enables sleeping once a frame to smooth out CPU usage */
	DISHONORED_SHIM_STATIC UBOOL bAllowPerFrameSleep;
	/** Enables yielding once a frame to give other processes time to run. Note that bAllowPerFrameSleep takes precedence */
	DISHONORED_SHIM_STATIC UBOOL bAllowPerFrameYield;

#if WITH_MOBILE_RHI
	/** The baseline feature level of the device */
	INT MobileFeatureLevel;
	/** Whether to allow fog on mobile*/
	UBOOL bAllowMobileFog;
	/** Whether to use height-fog on mobile, or simple gradient fog. */
	UBOOL bAllowMobileHeightFog;
	/** Whether to allow vertex specular on mobile */
	UBOOL bAllowMobileSpecular;
	/** Whether to allow bump offset on mobile */
	UBOOL bAllowMobileBumpOffset;
	/** Whether to allow normal mapping on mobile */
	UBOOL bAllowMobileNormalMapping;
	/** Whether to allow environment mapping on mobile */
	UBOOL bAllowMobileEnvMapping;
	/** Whether to allow rim lighting on mobile */
	UBOOL bAllowMobileRimLighting;
	/** Whether to allow color blending on mobile */
	UBOOL bAllowMobileColorBlending;
	/** Whether to allow color grading on mobile */
	UBOOL bAllowMobileColorGrading;
	/** Whether to allow vertex movement on mobile */
	UBOOL bAllowMobileVertexMovement;
	/** Whether to allow occlusion queries on mobile */
	UBOOL bAllowMobileOcclusionQueries;
	/** Global setting for gamma correction state*/
	UBOOL bMobileGlobalGammaCorrection;
	/** Whether to allow a level to override the gamma correction*/
	UBOOL bMobileAllowGammaCorrectionLevelOverride;
	/** Whether to enable a rendering depth pre-pass on mobile. */
	UBOOL bMobileAllowDepthPrePass;
#if WITH_GFx
	/** Whether to include gamma adjustment code in the scale form shaders*/
	UBOOL bMobileGfxGammaCorrection;
#endif
	/** Whether to use preprocessed shaders on mobile */
	UBOOL bUsePreprocessedShaders;
	/** Whether to flash the screen red (non-final release only) when a cached shader is not found at runtime */
	UBOOL bFlashRedForUncachedShaders;
	/** Whether to issue a "warm-up" draw call for mobile shaders as they are compiled */
	UBOOL bWarmUpPreprocessedShaders;
	/** Whether to dump out preprocessed shaders for mobile as they are encountered/compiled */
	UBOOL bCachePreprocessedShaders;
	/** Whether to run dumped out preprocessed shaders through the shader profiler */
	UBOOL bProfilePreprocessedShaders;
	/** Whether to run the C preprocessor on shaders  */
	UBOOL bUseCPreprocessorOnShaders;
	/** Whether to load the C preprocessed source  */
	UBOOL bLoadCPreprocessedShaders;
	/** Whether to share pixel shaders across multiple unreal shaders  */
	UBOOL bSharePixelShaders;
	/** Whether to share vertex shaders across multiple unreal shaders  */
	UBOOL bShareVertexShaders;
	/** Whether to share shaders program across multiple unreal shaders  */
	UBOOL bShareShaderPrograms;
	/** Whether to enable MSAA, if the OS supports it */
	UBOOL bEnableMSAA;
	/** The default global content scale factor to use on device (largely iOS specific) */
	FLOAT MobileContentScaleFactor;
	/** How much to bias all texture mip levels on mobile (usually 0 or negative) */
	FLOAT MobileLODBias;
	/** Scales the blur vector when using mobile RHI. */
	FLOAT MobileLightShaftRadialBlurPercentScale;
	/** Scale factor for the blur vector length, for the first pass */
	FLOAT MobileLightShaftRadialBlurFirstPassRatio;
	/** Scale factor for the blur vector length, for the second pass */
	FLOAT MobileLightShaftRadialBlurSecondPassRatio;
	/** The maximum number of bones supported for skinning */
	INT MobileBoneCount;
	/** The maximum number of bones influences per vertex supported for skinning */
	INT MobileBoneWeightCount;
	/** The size of the scratch buffer for vertices (in kB) */
	INT MobileVertexScratchBufferSize;
	/** The size of the scratch buffer for indices (in kB) */
	INT MobileIndexScratchBufferSize;
	/** TRUE if we try to support mobile modulated shadow */
	UBOOL bMobileModShadows;
	/** TRUE to enable the mobile tilt shift effect */
	UBOOL bMobileTiltShift;
	/** Position of the focused center of the tilt shift effect (in percent of the screen height) */
	FLOAT MobileTiltShiftPosition;
	/** Width of focused area in the tilt shift effect (in percent of the screen height) */
	FLOAT MobileTiltShiftFocusWidth;
	/** Width of transition area in the tilt shift effect, where it transitions from full focus to full blur (in percent of the screen height) */
	FLOAT MobileTiltShiftTransitionWidth;

	/** How far a dynamic shadow can extend outside the shadow caster's bounding box. */
	FLOAT MobileMaxShadowRange;

	/** Whether to clear the depth buffer between DPGs */
	UBOOL bMobileClearDepthBetweenDPG;

	/** The resolution of the shadow texture */
	INT	MobileShadowTextureResolution;

	/** Our perceived maximum memory available on the device */
	INT MobileMaxMemory;

	/** Without the resolve we might get the old depth buffer values but artifacts are minimal in many cases and it saves quite some performance. For correct results keep this set to TRUE. */
	UBOOL bMobileSceneDepthResolveForShadows;

	/** If we have enabled higher resolution timing for this device */
	UBOOL bMobileUsingHighResolutionTiming;

	/** LOD bias for mobile landscape rendering on this device (in addition to any per-landscape bias set) */
	INT MobileLandscapeLodBias;

	/** Whether to automatically put cooked startup objects in the StartupPackages shader group */
	UBOOL bMobileUseShaderGroupForStartupObjects;

	/** Whether to disable generating both fog shader permutations on mobile.  When TRUE, it decreases load times but increases GPU cost for materials/levels with fog enabled */
	UBOOL bMobileMinimizeFogShaders;

	/** Mobile FXAA quality level.  0 is off. */
	INT  MobileFXAAQuality;

#endif	//WITH_MOBILE_RHI

#if WITH_APEX
	/** Resource budget for APEX LOD. Higher values indicate the system can handle more APEX load.*/
	FLOAT ApexLODResourceBudget;
	/** The maximum number of active PhysX actors which represent dynamic groups of chunks (islands).  
		If a fracturing event would cause more islands to be created, then oldest islands are released 
		and the chunks they represent destroyed.*/
	INT ApexDestructionMaxChunkIslandCount;
	/** The maximum number of PhysX shapes which represent destructible chunks.  
		If a fracturing event would cause more shapes to be created, then oldest islands are released 
		and the chunks they represent destroyed.*/
	INT ApexDestructionMaxShapeCount;
	/** Every destructible asset defines a min and max lifetime, and maximum separation distance for its chunks.
		Chunk islands are destroyed after this time or separation from their origins. This parameter sets the
		lifetimes and max separations within their min-max ranges. The valid range is [0,1]. */
	FLOAT ApexDestructionMaxChunkSeparationLOD;
	/** If TRUE, allow APEX clothing fetch (skinning etc) to be done on multiple threads */
	UBOOL bEnableParallelApexClothingFetch;
	/** If TRUE, allow APEX skinning to occur without blocking fetch results.  bEnableParallelApexClothingFetch must be enabled
		for this to work. */
	UBOOL bApexClothingAsyncFetchResults;
	/** If set to true, destructible chunks with the lowest benefit would get removed first instead of the oldest */
	UBOOL bApexDestructionSortByBenefit;
	/** Lets the user throttle the number of SDK actor creates per frame (per scene) due to destruction, as this can be quite costly.
		The default is 0xffffffff (unlimited). */
	INT ApexDestructionMaxActorCreatesPerFrame;
	/** Lets the user throttle the number of fractures processed per frame (per scene) due to destruction, as this can be quite costly.
		The default is 0xffffffff (unlimited). */
	INT ApexDestructionMaxFracturesProcessedPerFrame;
	/** Average Simulation Frequency is estimated with the last n frames. This is used in Clothing when
		bAllowAdaptiveTargetFrequency is enabled.*/
	INT ApexClothingAvgSimFrequencyWindow;
	/**	Whether or not to use GPU Rigid Bodies	*/
	UBOOL bEnableApexGRB;
	/**	The size of the cells to divide the world into for GPU collision detection	*/
	FLOAT ApexGRBMeshCellSize;
	/**	Collision skin width, as in PhysX. */
	FLOAT ApexGRBSkinWidth;
	/** Number of non-penetration solver iterations. */
	INT ApexGRBNonPenSolverPosIterCount;
	/** Number of friction solver position iterations. */
	INT ApexGRBFrictionSolverPosIterCount;
	/** Number of friction solver velocity iterations. */
	INT ApexGRBFrictionSolverVelIterCount;
	/** Maximum linear acceleration. */
	FLOAT ApexGRBMaxLinAcceleration;
	/** Amount (in MB) of GPU memory to allocate for GRB scene data (shapes, actors etc). */
	INT ApexGRBGpuMemSceneSize;
	/** Amount (in MB) of GPU memory to allocate for GRB temporary data (broadphase pairs, contacts etc). */
	INT ApexGRBGpuMemTempDataSize;
	/** ClothingActors will cook in a background thread to speed up creation time */
	UBOOL bApexClothingAllowAsyncCooking;
	/** Allow APEX SDK to interpolate clothing matrices between the substeps. */
	UBOOL bApexClothingAllowApexWorkBetweenSubsteps;
#endif

	/** Constructor, initializing all member variables. */
	FSystemSettings();

/** 
	 * Exec handler implementation.
	 *
	 * @param Cmd	Command to parse
	 * @param Ar	Output device to log to
	 *
	 * @return TRUE if command was handled, FALSE otherwise
	 */
	UBOOL Exec( const TCHAR* Cmd, FOutputDevice& Ar );

/** 
	 * Initializes system settings and included texture LOD settings.
 *
	 * @param bSetupForEditor	Whether to initialize settings for Editor
 */
	void Initialize( UBOOL bSetupForEditor );

	/** loads settings from the given section in the given ini */
	void LoadFromIni( const FString IniSection, const TCHAR* IniFilename = GEngineIni, UBOOL bAllowMissingValues = TRUE );

	/** Loads settings from the ini. (purposely override the inherited name so people can't accidentally call it.) */
	UBOOL LoadFromIni( const TCHAR* Override );

	/** saves settings to the given section in the engine ini */
	void SaveToIni( const FString IniSection );

	/** Saves current settings to the ini. (purposely override the inherited name so people can't accidentally call it.) */
	void SaveToIni( void );

	/**
	 * Returns a string for the specified texture group LOD settings to the specified ini.
	 *
	 * @param	TextureGroupID		Index/enum of the group
	 * @param	GroupName			String representation of the texture group
	 */
	FString GetLODGroupString( TextureGroup TextureGroupID, const TCHAR* GroupName );

	void WriteTextureLODGroupToIni(TextureGroup TextureGroupID, const TCHAR* GroupName, const TCHAR* IniSection);

/**
	 * Writes all texture group LOD settings to the specified ini.
 *
	 * @param	IniFilename			The .ini file to save to.
	 * @param	IniSection			The .ini section to save to.
 */
	void WriteTextureLODGroupsToIni(const TCHAR* IniSection);

	/**
	 * Reads a single entry and parses it into the group array.
	 *
	 * @param	TextureGroupID		Index/enum of group to parse
	 * @param	MinLODSize			Minimum size, in pixels, below which the code won't bias.
	 * @param	MaxLODSize			Maximum size, in pixels, above which the code won't bias.
	 * @param	LODBias				Group LOD bias.
	 * @param	MipGenSettings		Defines how the the mip-map generation works, e.g. sharpening
	 */
	void SetTextureLODGroup( TextureGroup TextureGroupID, int MinLODSize, INT MaxLODSize, INT LODBias, TextureMipGenSettings MipGenSettings );

	/** Dumps the settings to the log file */
	void Dump( FOutputDevice& Ar, ESystemSettingIntent SettingIntent );
	void DumpTextures( FOutputDevice& Ar );
	void DumpTextureLODGroup( FOutputDevice& Ar, TextureGroup TextureGroupID, const TCHAR* GroupName );

	/** Finds the setting by name */
	FSystemSetting* FindSystemSetting( FString& SettingName, ESystemSettingType SettingType );

	/** Indicates whether upscaling is needed */
	UBOOL NeedsUpscale( void ) const;

	/** Indicates whether the hardware anti-aliasing is used */
	UBOOL UsesMSAA( void ) const
	{
		return MaxMultiSamples > 1 && ( GRHIShaderPlatform == SP_PCD3D_SM5 || ( GRHIShaderPlatform == SP_PCD3D_SM3 && bAllowD3D9MSAA ) );
	}

	/** 
	 * Sets the resolution and writes the values to Ini if changed but does not apply the changes (eg resize the viewport).
	 */
	void SetResolution(INT InSizeX, INT InSizeY, UBOOL InFullscreen);

	/**
	 * Scale X,Y offset/size of screen coordinates if the screen percentage is not at 100%
	 *
	 * @param X - in/out X screen offset
	 * @param Y - in/out Y screen offset
	 * @param SizeX - in/out X screen size
	 * @param SizeY - in/out Y screen size
	 */
	void ScaleScreenCoords( INT& X, INT& Y, UINT& SizeX, UINT& SizeY );

	/**
	 * Reverses the scale and offset done by ScaleScreenCoords() 
	 * if the screen percentage is not 100% and upscaling is allowed.
	 *
	 * @param OriginalX - out X screen offset
	 * @param OriginalY - out Y screen offset
	 * @param OriginalSizeX - out X screen size
	 * @param OriginalSizeY - out Y screen size
	 * @param InX - in X screen offset
	 * @param InY - in Y screen offset
	 * @param InSizeX - in X screen size
	 * @param InSizeY - in Y screen size
	 */
	void UnScaleScreenCoords( 
		INT &OriginalX, INT &OriginalY, 
		UINT &OriginalSizeX, UINT &OriginalSizeY, 
		FLOAT InX, FLOAT InY, 
		FLOAT InSizeX, FLOAT InSizeY);

	/** Applies setting overrides based on command line options. */
	void ApplyOverrides();

	/**
	 * Apply any settings that have changed
	 */
	void ApplySettings( FSystemSettings& OldSystemSettings );

	/**
	 * Sets new system settings (optionally writes out to the ini).
	 */ 
	void ApplyNewSettings( const FSystemSettings& NewSettings, UBOOL bWriteToIni );

	/**
	 * Ensures that the correct settings are being used based on split screen type.
	 */
	void UpdateSplitScreenSettings( void );

	/**
	 * Recreates texture resources and drops mips.
	 *
	 * @return		TRUE if the settings were applied, FALSE if they couldn't be applied immediately.
	 */
	UBOOL UpdateTextureStreaming( void );

	/**
	 * Makes System Settings take effect on the rendering thread
	 */
	static void SceneRenderTargetsUpdateRHI(const FSystemSettings& OldSettings, const FSystemSettings& NewSettings);

	/**
	 * Cause a call to UpdateRHI on GSceneRenderTargets
	 */
	static void UpdateSceneRenderTargetsRHI();
		
	/** Set to TRUE after this has been populated from the ini files */
	DISHONORED_SHIM_STATIC UBOOL bInit;

	/** Name of ini section used to set data */
	DISHONORED_SHIM_STATIC FString SystemSettingName;

	/** The number of system settings that can be modified */
	DISHONORED_SHIM_STATIC INT NumberOfSystemSettings;

	/** The master list of all configurable system settings */
	static FSystemSetting SystemSettings[];
};

/**
 * Global system settings accessor
 */
extern FSystemSettings GSystemSettings;

#endif // __SYSTEMSETTINGS_H__
