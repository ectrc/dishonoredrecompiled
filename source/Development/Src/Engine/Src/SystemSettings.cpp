/*=============================================================================
	ScalabilityOptions.cpp: Unreal engine HW compat scalability system.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "EnginePrivate.h"
#include "UnTerrain.h"
#include "EngineSpeedTreeClasses.h"
#include "EngineDecalClasses.h"
#include "EngineUserInterfaceClasses.h"
#include "SceneRenderTargets.h"
#if IPHONE
	#include "IPhoneObjCWrapper.h"
#endif
#if _WINDOWS
// DISHONORED(retail): registry overrides in LoadFromIni (2013 rva 0x1806c0) need winreg; same include pattern as UnConsoleTools.cpp
#include "PreWindowsApi.h"
#include <windows.h>
#include "PostWindowsApi.h"
#endif
#if WITH_OPEN_AUTOMATE
#include "OpenAutomate.h"
#endif

/*-----------------------------------------------------------------------------
	FSystemSettings
-----------------------------------------------------------------------------*/

/** Global accessor */
FSystemSettings GSystemSettings;

FVSSBool SimpleBool;
FVSSGenericInt VSSDetailMode( 0, 2, 1 );
FVSSGenericInt VSSSkeletalMeshLODBias( -2, 2, 1 );
FVSSGenericInt VSSMaxAnisotropy( 1, 16, 1 );
FVSSGenericInt VSSMaxMultiSamples( 1, 16, 1 );
FVSSGenericInt VSSMaxFilterBlurSampleCount( 1, 16, 1 );
FVSSGenericInt VSSMaxShadowResolution( 256, 2048, 256 );
FVSSGenericInt VSSResX( 640, 2560, 40 );
FVSSGenericInt VSSResY( 400, 1600, 20 );
FVSSGenericFloat VSSMaxDrawDistanceScale( 0.5f, 1.5f );
FVSSGenericFloat VSSScreenPercentage( 0.6f, 1.0f );
FVSSGenericFloat VSSShadowTexels( 0.5f, 2.5f );

FSystemSetting FSystemSettings::SystemSettings[] =
{
	// DISHONORED(retail): the 81 keys FSystemSettingsData::LoadFromIni reads (2013 rva 0x1806c0; 2012 rva 0x186830,
	// systemsettings.cpp) in its order: 43 switches (GetBool), 21 ints (GetInt), 17 floats (GetFloat). With the 26
	// TEXTUREGROUP_* entries of FTextureLODSettings::Initialize (2013 rva 0x17bb20) that is the 107-key retail section.
	// Offsets are the FSystemSettingsData members the 2013 table stores (names from the 2012 PDB; iType_AntiAlias and
	// bAllowRatsShadow exist in 2013 only). Intent/validator/help come from the reference row of the same key.
	// ---- switches (43) ----
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "StaticDecals" ), &GSystemSettings.bAllowStaticDecals, &SimpleBool, TEXT( "Whether to allow static decals." ) },	// @16
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "DynamicDecals" ), &GSystemSettings.bAllowDynamicDecals, &SimpleBool, TEXT( "Whether to allow dynamic decals." ) },	// @20
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "UnbatchedDecals" ), &GSystemSettings.bAllowUnbatchedDecals, &SimpleBool, TEXT( "Whether to allow decals that have not been placed in static draw lists and have dynamic view relevance." ) },	// @24
	{ SST_BOOL, SSI_DEBUG, TEXT( "DynamicLights" ), &GSystemSettings.bAllowDynamicLights, &SimpleBool, TEXT( "Whether to allow dynamic lights." ) },	// @32
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "DynamicShadows" ), &GSystemSettings.bAllowDynamicShadows, &SimpleBool, TEXT( "Whether to allow dynamic shadows." ) },	// @916
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "LightEnvironmentShadows" ), &GSystemSettings.bAllowLightEnvironmentShadows, &SimpleBool, TEXT( "Whether to allow dynamic light environments to cast shadows." ) },	// @920
	{ SST_BOOL, SSI_DEBUG, TEXT( "CompositeDynamicLights" ), &GSystemSettings.bUseCompositeDynamicLights, &SimpleBool, TEXT( "Whether to composte dynamic lights into light environments." ) },	// @36
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "SHSecondaryLighting" ), &GSystemSettings.bAllowSHSecondaryLighting, &SimpleBool, TEXT( "Whether to allow light environments to use SH lights for secondary lighting." ) },	// @40
	{ SST_BOOL, SSI_DEBUG, TEXT( "DirectionalLightmaps" ), &GSystemSettings.bAllowDirectionalLightMaps, &SimpleBool, TEXT( "Whether to allow directional lightmaps, which use the material's normal and specular." ) },	// @44
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "MotionBlur" ), &GSystemSettings.bAllowMotionBlur, &SimpleBool, TEXT( "Whether to allow motion blur." ) },	// @48
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "MotionBlurPause" ), &GSystemSettings.bAllowMotionBlurPause, &SimpleBool, TEXT( "Whether to allow motion blur to be paused." ) },	// @52
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "DepthOfField" ), &GSystemSettings.bAllowDepthOfField, &SimpleBool, TEXT( "Whether to allow depth of field." ) },	// @56
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "AmbientOcclusion" ), &GSystemSettings.bAllowAmbientOcclusion, &SimpleBool, TEXT( "Whether to allow ambient occlusion." ) },	// @60
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "Bloom" ), &GSystemSettings.bAllowBloom, &SimpleBool, TEXT( "Whether to allow bloom." ) },	// @64
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "UseHighQualityBloom" ), &GSystemSettings.bUseHighQualityBloom, &SimpleBool, TEXT( "Whether to use the high quality bloom path." ) },	// @68, retail-only key
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "bAllowLightShafts" ), &GSystemSettings.bAllowLightShafts, &SimpleBool, TEXT( "Whether to allow light shafts." ) },	// @72
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "bAllowRatsShadow" ), &GSystemSettings.bAllowRatsShadow, &SimpleBool, TEXT( "Whether rats cast shadows." ) },	// @76, retail-only key, 2013-only member
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "Distortion" ), &GSystemSettings.bAllowDistortion, &SimpleBool, TEXT( "Whether to allow distortion." ) },	// @80
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "FilteredDistortion" ), &GSystemSettings.bAllowFilteredDistortion, &SimpleBool, TEXT( "Whether to allow distortion to use bilinear filtering when sampling the scene color during its apply pass." ) },	// @84
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "DropParticleDistortion" ), &GSystemSettings.bAllowParticleDistortionDropping, &SimpleBool, TEXT( "Whether to allow dropping distortion on particles based on WorldInfo::bDropDetail." ) },	// @88
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "bAllowDownsampledTranslucency" ), &GSystemSettings.bAllowDownsampledTranslucency, &SimpleBool, TEXT( "Whether to allow downsampled transluency." ) },	// @92
	{ SST_BOOL, SSI_DEBUG, TEXT( "SpeedTreeLeaves" ), &GSystemSettings.bAllowSpeedTreeLeaves, &SimpleBool, TEXT( "Whether to allow rendering of SpeedTree leaves." ) },	// @8
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "bUseMaxQualityMode" ), &GSystemSettings.bUseMaxQualityMode, &SimpleBool, TEXT( "Whether to force the max quality mode (-MAXQUALITYMODE)." ) },	// @4, retail-only key
	{ SST_BOOL, SSI_DEBUG, TEXT( "SpeedTreeFronds" ), &GSystemSettings.bAllowSpeedTreeFronds, &SimpleBool, TEXT( "Whether to allow rendering of SpeedTree fronds." ) },	// @12
	{ SST_BOOL, SSI_DEBUG, TEXT( "OnlyStreamInTextures" ), &GSystemSettings.bOnlyStreamInTextures, &SimpleBool, TEXT( "If enabled, texture will only be streamed in, not out." ) },	// @872
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "LensFlares" ), &GSystemSettings.bAllowLensFlares, &SimpleBool, TEXT( "Whether to allow rendering of LensFlares." ) },	// @96
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "FogVolumes" ), &GSystemSettings.bAllowFogVolumes, &SimpleBool, TEXT( "Whether to allow fog volumes." ) },	// @100
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "FloatingPointRenderTargets" ), &GSystemSettings.bAllowFloatingPointRenderTargets, &SimpleBool, TEXT( "Whether to allow floating point render targets to be used." ) },	// @104
	{ SST_BOOL, SSI_DEBUG, TEXT( "OneFrameThreadLag" ), &GSystemSettings.bAllowOneFrameThreadLag, &SimpleBool, TEXT( "Whether to allow the rendering thread to lag one frame behind the game thread." ) },	// @108
	{ SST_BOOL, SSI_PREFERENCE, TEXT( "UseVsync" ), &GSystemSettings.bUseVSync, &SimpleBool, TEXT( "Whether to use VSync or not." ) },	// @888
	{ SST_BOOL, SSI_DEBUG, TEXT( "UpscaleScreenPercentage" ), &GSystemSettings.bUpscaleScreenPercentage, &SimpleBool, TEXT( "Whether to upscale the screen to take up the full front buffer." ) },	// @896
	{ SST_BOOL, SSI_PREFERENCE, TEXT( "Fullscreen" ), &GSystemSettings.bFullscreen, &SimpleBool, TEXT( "Fullscreen." ) },	// @908
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "AllowD3D10" ), &GSystemSettings.bAllowD3D10, &SimpleBool, TEXT( "Whether to allow D3D10." ) },	// @136, retail-only key
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "AllowRadialBlur" ), &GSystemSettings.bAllowRadialBlur, &SimpleBool, TEXT( "Whether to allow radial blur effects to render." ) },	// @140
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "bEnableBranchingPCFShadows" ), &GSystemSettings.bEnableBranchingPCFShadows, &SimpleBool, TEXT( "Toggle Branching PCF implementation for projected shadows." ) },	// @952
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "bAllowHardwareShadowFiltering" ), &GSystemSettings.bAllowHardwareShadowFiltering, &SimpleBool, TEXT( "Whether to allow hardware filtering optimizations like hardware PCF and Fetch4." ) },	// @956
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "bAllowBetterModulatedShadows" ), &GSystemSettings.bAllowBetterModulatedShadows, &SimpleBool, TEXT( "Whether to allow better modulated shadows." ) },	// @960, retail-only key
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "bEnableForegroundShadowsOnWorld" ), &GSystemSettings.bEnableForegroundShadowsOnWorld, &SimpleBool, TEXT( "hack to allow for foreground DPG objects to cast shadows on the world DPG." ) },	// @964
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "bEnableForegroundSelfShadowing" ), &GSystemSettings.bEnableForegroundSelfShadowing, &SimpleBool, TEXT( "Whether to allow foreground DPG self-shadowing." ) },	// @968
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "bAllowWholeSceneDominantShadows" ), &GSystemSettings.bAllowWholeSceneDominantShadows, &SimpleBool, TEXT( "Whether to allow whole scene dominant shadows." ) },	// @972
	{ SST_BOOL, SSI_SCALABILITY, TEXT( "bAllowFracturedDamage" ), &GSystemSettings.bAllowFracturedDamage, &SimpleBool, TEXT( "Whether to allow fractured meshes to take damage." ) },	// @1020
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "bForceCPUAccessToGPUSkinVerts" ), &GSystemSettings.bForceCPUAccessToGPUSkinVerts, &SimpleBool, TEXT( "Whether to force CPU access to GPU skinned vertex data." ) },	// @1040
	{ SST_BOOL, SSI_UNKNOWN, TEXT( "bDisableSkeletalInstanceWeights" ), &GSystemSettings.bDisableSkeletalInstanceWeights, &SimpleBool, TEXT( "Whether to disable instanced skeletal weights." ) },	// @1044
	// ---- ints (21) ----
	{ SST_INT, SSI_SCALABILITY, TEXT( "SkeletalMeshLODBias" ), &GSystemSettings.SkeletalMeshLODBias, &VSSSkeletalMeshLODBias, TEXT( "LOD bias for skeletal meshes." ) },	// @112
	{ SST_INT, SSI_UNKNOWN, TEXT( "SkeletalLODDistanceFactorMultiplier" ), &GSystemSettings.SkeletalLODDistanceFactorMultiplier, NULL, TEXT( "LOD distance factor multiplier for skeletal meshes." ) },	// @116, retail-only key
	{ SST_INT, SSI_UNKNOWN, TEXT( "StaticLODDistanceFactorMultiplier" ), &GSystemSettings.StaticLODDistanceFactorMultiplier, NULL, TEXT( "LOD distance factor multiplier for static meshes." ) },	// @120, retail-only key
	{ SST_INT, SSI_UNKNOWN, TEXT( "TextureForcedLODBias" ), &GSystemSettings.TextureForcedLODBias, NULL, TEXT( "Forced LOD bias for textures." ) },	// @124, retail-only key
	{ SST_INT, SSI_UNKNOWN, TEXT( "iType_AntiAlias" ), &GSystemSettings.iType_AntiAlias, NULL, TEXT( "Anti-aliasing type (EPpAa_None=0, EPpAa_Mlaa=1, EPpAa_Fxaa=2)." ) },	// @128, retail-only key, 2013-only member (2012 had bAllowMLAA @136 instead)
	{ SST_INT, SSI_DEBUG, TEXT( "ParticleLODBias" ), &GSystemSettings.ParticleLODBias, NULL, TEXT( "LOD bias for particle systems." ) },	// @132
	{ SST_INT, SSI_SCALABILITY, TEXT( "DetailMode" ), &GSystemSettings.DetailMode, &VSSDetailMode, TEXT( "Current detail mode; determines whether components of actors should be updated/ ticked." ) },	// @0
	{ SST_INT, SSI_UNKNOWN, TEXT( "ShadowFilterQualityBias" ), &GSystemSettings.ShadowFilterQualityBias, NULL, TEXT( "Quality bias for projected shadow buffer filtering. Higher values use better quality filtering." ) },	// @924
	{ SST_INT, SSI_SCALABILITY, TEXT( "MaxAnisotropy" ), &GSystemSettings.MaxAnisotropy, &VSSMaxAnisotropy, TEXT( "Maximum level of anisotropy used." ) },	// @876
	{ SST_INT, SSI_DEBUG, TEXT( "MaxMultisamples" ), &GSystemSettings.MaxMultiSamples, &VSSMaxMultiSamples, TEXT( "The maximum number of MSAA samples to use." ) },	// @912, retail spelling
	{ SST_INT, SSI_SCALABILITY, TEXT( "MinShadowResolution" ), &GSystemSettings.MinShadowResolution, NULL, TEXT( "min dimensions (in texels) allowed for rendering shadow subject depths." ) },	// @928
	{ SST_INT, SSI_SCALABILITY, TEXT( "MinPreShadowResolution" ), &GSystemSettings.MinPreShadowResolution, NULL, TEXT( "min dimensions (in texels) allowed for rendering preshadow depths." ) },	// @932
	{ SST_INT, SSI_SCALABILITY, TEXT( "MaxShadowResolution" ), &GSystemSettings.MaxShadowResolution, &VSSMaxShadowResolution, TEXT( "max square dimensions (in texels) allowed for rendering shadow subject depths." ) },	// @936
	{ SST_INT, SSI_SCALABILITY, TEXT( "MaxWholeSceneDominantShadowResolution" ), &GSystemSettings.MaxWholeSceneDominantShadowResolution, &VSSMaxShadowResolution, TEXT( "max square dimensions (in texels) allowed for rendering whole scene shadow depths." ) },	// @940
	{ SST_INT, SSI_PREFERENCE, TEXT( "ResX" ), &GSystemSettings.ResX, &VSSResX, TEXT( "Screen X resolution." ) },	// @900
	{ SST_INT, SSI_PREFERENCE, TEXT( "ResY" ), &GSystemSettings.ResY, &VSSResY, TEXT( "Screen Y resolution." ) },	// @904
	{ SST_INT, SSI_UNKNOWN, TEXT( "UnbuiltNumWholeSceneDynamicShadowCascades" ), &GSystemSettings.UnbuiltNumWholeSceneDynamicShadowCascades, NULL, TEXT( "NumWholeSceneDynamicShadowCascades to use when using CSM to preview unbuilt lighting from a directional light." ) },	// @1000
	{ SST_INT, SSI_UNKNOWN, TEXT( "WholeSceneShadowUnbuiltInteractionThreshold" ), &GSystemSettings.WholeSceneShadowUnbuiltInteractionThreshold, NULL, TEXT( "How many unbuilt light-primitive interactions there can be for a light before the light switches to whole scene shadows." ) },	// @1004
	{ SST_INT, SSI_UNKNOWN, TEXT( "ShadowFadeResolution" ), &GSystemSettings.ShadowFadeResolution, NULL, TEXT( "Resolution in texel below which shadows are faded out." ) },	// @1008
	{ SST_INT, SSI_UNKNOWN, TEXT( "PreShadowFadeResolution" ), &GSystemSettings.PreShadowFadeResolution, NULL, TEXT( "Resolution in texel below which preshadows are faded out." ) },	// @1012
	{ SST_INT, SSI_UNKNOWN, TEXT( "SpeakerConfiguration" ), &GSystemSettings.SpeakerConfiguration, NULL, TEXT( "Speaker configuration." ) },	// @1048, retail-only key (2012 PDB m_SpeakerConfiguration)
	// ---- floats (17) ----
	{ SST_FLOAT, SSI_DEBUG, TEXT( "ScreenPercentage" ), &GSystemSettings.ScreenPercentage, &VSSScreenPercentage, TEXT( "Percentage of screen main view should take up." ) },	// @892
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "SceneCaptureStreamingMultiplier" ), &GSystemSettings.SceneCaptureStreamingMultiplier, NULL, TEXT( "Scene capture streaming texture update distance scalar." ) },	// @880
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "FoliageDrawRadiusMultiplier" ), &GSystemSettings.FoliageDrawRadiusMultiplier, NULL, TEXT( "Multiplier for the foliage draw radius." ) },	// @884, retail-only key
	{ SST_FLOAT, SSI_SCALABILITY, TEXT( "ShadowTexelsPerPixel" ), &GSystemSettings.ShadowTexelsPerPixel, &VSSShadowTexels, TEXT( "The ratio of subject pixels to shadow texels." ) },	// @944
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "PreShadowResolutionFactor" ), &GSystemSettings.PreShadowResolutionFactor, NULL, TEXT( "UKNOWN" ) },	// @948
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "ShadowFilterRadius" ), &GSystemSettings.ShadowFilterRadius, NULL, TEXT( "Radius, in shadowmap texels, of the filter disk." ) },	// @976
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "ShadowDepthBias" ), &GSystemSettings.ShadowDepthBias, NULL, TEXT( "Depth bias that is applied in the depth pass for all types of projected shadows except VSM." ) },	// @980
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "CSMSplitPenumbraScale" ), &GSystemSettings.CSMSplitPenumbraScale, NULL, TEXT( "Scale applied to the penumbra size of Cascaded Shadow Map splits, useful for minimizing the transition between splits." ) },	// @984
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "CSMSplitSoftTransitionDistanceScale" ), &GSystemSettings.CSMSplitSoftTransitionDistanceScale, NULL, TEXT( "Scale applied to the soft comparison transition distance of Cascaded Shadow Map splits, useful for minimizing the transition between splits." ) },	// @988
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "CSMSplitDepthBiasScale" ), &GSystemSettings.CSMSplitDepthBiasScale, NULL, TEXT( "Scale applied to the depth bias of Cascaded Shadow Map splits, useful for minimizing the transition between splits." ) },	// @992
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "UnbuiltWholeSceneDynamicShadowRadius" ), &GSystemSettings.UnbuiltWholeSceneDynamicShadowRadius, NULL, TEXT( "WholeSceneDynamicShadowRadius to use when using CSM to preview unbuilt lighting from a directional light." ) },	// @996
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "ShadowFadeExponent" ), &GSystemSettings.ShadowFadeExponent, NULL, TEXT( "Controls the rate at which shadows are faded out." ) },	// @1016
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "NumFracturedPartsScale" ), &GSystemSettings.NumFracturedPartsScale, NULL, TEXT( "Scales the game-specific number of fractured physics objects allowed." ) },	// @1024
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "FractureDirectSpawnChanceScale" ), &GSystemSettings.FractureDirectSpawnChanceScale, NULL, TEXT( "Percent chance of a rigid body spawning after a fractured static mesh is damaged directly.  [0-1]" ) },	// @1028
	{ SST_FLOAT, SSI_UNKNOWN, TEXT( "FractureRadialSpawnChanceScale" ), &GSystemSettings.FractureRadialSpawnChanceScale, NULL, TEXT( "Percent chance of a rigid body spawning after a fractured static mesh is damaged by radial blast.  [0-1]" ) },	// @1032
	{ SST_FLOAT, SSI_SCALABILITY, TEXT( "FractureCullDistanceScale" ), &GSystemSettings.FractureCullDistanceScale, &VSSMaxDrawDistanceScale, TEXT( "Distance scale for whether a fractured static mesh should actually fracture when damaged." ) },	// @1036
	{ SST_FLOAT, SSI_SCALABILITY, TEXT( "DecalCullDistanceScale" ), &GSystemSettings.DecalCullDistanceScale, &VSSMaxDrawDistanceScale, TEXT( "Scale factor for distance culling decals." ) },	// @28
};

/**
 * Helpers for reading and writing to specific ini sections
 */
// DISHONORED(retail): GetSectionName, 2013 rva 0x16edb0 (2012 rva 0x178e20): "-SystemSettings=<Name>" wins for every caller
// (the "SystemSettings" prefix is stripped from <Name> before it is appended), otherwise "SystemSettingsEditor" /
// "SystemSettings". No "simmobile", no mobile sections, no compat bucket. The Override parameter is reference-only
// (LoadFromIni(Override)/SaveToIni() pass NULL or "") and kept for those callers.
static const FString GetSectionName( UBOOL bIsEditor, const TCHAR* Override )
{
	FString IniSectionName = TEXT( "SystemSettings" );

	FString OverrideName;
	if( Parse( appCmdLine(), TEXT( "-SystemSettings=" ), OverrideName ) )
	{
		if( !appStrnicmp( *OverrideName, *IniSectionName, IniSectionName.Len() ) )
		{
			OverrideName = OverrideName.Mid( IniSectionName.Len() );
		}
		return IniSectionName + OverrideName;
	}

	if( bIsEditor )
	{
		return FString( TEXT( "SystemSettingsEditor" ) );
	}

	if( Override != NULL )
	{
		IniSectionName += Override;
	}
	return IniSectionName;
}

/**
 * Finds the setting by name
*/
FSystemSetting* FSystemSettings::FindSystemSetting( FString& SettingName, ESystemSettingType SettingType )
{
	for( INT SettingIndex = 0; SettingIndex < ARRAY_COUNT( SystemSettings ); SettingIndex++ )
	{
		FSystemSetting* Setting = SystemSettings + SettingIndex;
		if( SettingType != SST_ANY && SettingType != Setting->SettingType )
		{
			continue;
		}

		if( !appStrnicmp( Setting->SettingName, *SettingName, SettingName.Len() ) )
		{
			return Setting;
		}
	}

	warnf( NAME_Warning, TEXT( "The System Setting %s could not be found." ), *SettingName );
	return NULL;
}

/**
 * ctor
 */
void FSystemSettings::LoadFromIni( const FString IniSection, const TCHAR* IniFilename, UBOOL bAllowMissingValues )
{
	UBOOL bCheckFoundValuesAtEnd = FALSE;
	
	if( !bAllowMissingValues )
	{
		// we need to check the FoundValues after at the end of this function
		bCheckFoundValuesAtEnd = TRUE;

		// Clear out the found status
		for( INT SettingIndex = 0; SettingIndex < ARRAY_COUNT( SystemSettings ); SettingIndex++ )
		{
			SystemSettings[SettingIndex].bFound = FALSE;
		}
	}

	// first, look for a parent section to base off of
	FString BasedOnSection;
	if( GConfig->GetString( *IniSection, TEXT( "BasedOn" ), BasedOnSection, IniFilename ) )
	{
		debugf( TEXT( "SystemSettings based on: %s" ), *BasedOnSection );
		// recurse with the BasedOn section if it existed, always allowing for missing values
		LoadFromIni( BasedOnSection, IniFilename, TRUE );
	}

	for( INT SettingIndex = 0; SettingIndex < ARRAY_COUNT( SystemSettings ); SettingIndex++ )
	{
		FSystemSetting* Setting = SystemSettings + SettingIndex;
		switch( Setting->SettingType )
		{
		case SST_BOOL:
			Setting->bFound |= GConfig->GetBool( *IniSection, Setting->SettingName, *( UBOOL* )Setting->SettingAddress, IniFilename );
			break;

		case SST_INT:
			Setting->bFound |= GConfig->GetInt( *IniSection, Setting->SettingName, *( INT* )Setting->SettingAddress, IniFilename );
			break;

		case SST_FLOAT:
			Setting->bFound |= GConfig->GetFloat( *IniSection, Setting->SettingName, *( FLOAT* )Setting->SettingAddress, IniFilename );
			break;
		}
	}

#if _WINDOWS
	// DISHONORED(retail): FSystemSettingsData::LoadFromIni, 2013 rva 0x1806c0 (not in the 2012 build): when the seek-free PC
	// console path reads the Engine ini itself, HKCU\Software\Arkane\Dishonored overrides the switches and ints (REG_DWORD
	// values named like the ini keys). The key's "Timestamp" value (REG_BINARY FILETIME of the ini) invalidates every stored
	// value once the ini changed. Floats and texture groups are never overridden; a missing key skips the block.
	if( GIsSeekFreePCConsole && !appStricmp( GEngineIni, IniFilename ) )
	{
		HKEY Key = NULL;
		if( RegOpenKeyExW( HKEY_CURRENT_USER, L"Software\\Arkane\\Dishonored", 0, KEY_ALL_ACCESS, &Key ) == ERROR_SUCCESS )
		{
			FILETIME IniWriteTime = { 0, 0 };
			HANDLE IniFile = CreateFileW( GEngineIni, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
			if( IniFile != INVALID_HANDLE_VALUE )
			{
				FILETIME WriteTime;
				if( GetFileTime( IniFile, NULL, NULL, &WriteTime ) )
				{
					IniWriteTime = WriteTime;
				}
				CloseHandle( IniFile );

				DWORD Type = 0;
				BYTE Data[16] = { 0 };
				DWORD DataSize = sizeof( Data );
				const LONG Result = RegQueryValueExW( Key, L"Timestamp", NULL, &Type, Data, &DataSize );
				UBOOL bResetValues;
				if( Result == ERROR_SUCCESS )
				{
					bResetValues = Type != REG_BINARY || appMemcmp( Data, &IniWriteTime, sizeof( FILETIME ) ) != 0;
				}
				else
				{
					bResetValues = Result == ERROR_FILE_NOT_FOUND || Type != REG_BINARY;
				}
				if( bResetValues )
				{
					DWORD NumValues = 0;
					RegQueryInfoKeyW( Key, NULL, NULL, NULL, NULL, NULL, NULL, &NumValues, NULL, NULL, NULL, NULL );
					for( INT ValueIndex = ( INT )NumValues - 1; ValueIndex >= 0; ValueIndex-- )
					{
						WCHAR ValueName[16384];
						ValueName[0] = 0;
						DWORD ValueNameLength = 0x3FFF;
						RegEnumValueW( Key, ValueIndex, ValueName, &ValueNameLength, NULL, NULL, NULL, NULL );
						RegDeleteValueW( Key, ValueName );
					}
					RegSetValueExW( Key, L"Timestamp", 0, REG_BINARY, ( const BYTE* )&IniWriteTime, sizeof( FILETIME ) );
				}
			}
			for( INT SettingIndex = 0; SettingIndex < ARRAY_COUNT( SystemSettings ); SettingIndex++ )
			{
				FSystemSetting* Setting = SystemSettings + SettingIndex;
				if( Setting->SettingType == SST_FLOAT )
				{
					continue;
				}
				DWORD Type = 0;
				DWORD Value = 0;
				DWORD ValueSize = sizeof( Value );
				if( RegQueryValueExW( Key, Setting->SettingName, NULL, &Type, ( BYTE* )&Value, &ValueSize ) == ERROR_SUCCESS && Type == REG_DWORD )
				{
					*( DWORD* )Setting->SettingAddress = Value;
				}
			}
			RegCloseKey( Key );
		}
	}
#endif

	// Read the texture group LOD settings.
	TextureLODSettings.Initialize( IniFilename, *IniSection );

	// DISHONORED(retail): the found flags are collected for the outermost call (2013 rva 0x1806c0 allocates the FoundValues
	// array itself) but never checked: no "Couldn't find system setting" string in the 2012 or 2013 exe, the reference checkf
	// is gone. The warning below is bring-up only (dropped from the golden diff by normalize_log.py).
	if( bCheckFoundValuesAtEnd )
	{
		for( INT SettingIndex = 0; SettingIndex < ARRAY_COUNT( SystemSettings ); SettingIndex++ )
		{
			if( !SystemSettings[SettingIndex].bFound )
			{
				warnf( NAME_Warning, TEXT( "DISHONORED(bringup): system setting %s not found in [%s] of %s" ), SystemSettings[SettingIndex].SettingName, *IniSection, IniFilename );
			}
		}
	}
}

/**
* Returns a string for the specified texture group LOD settings to the specified ini.
*
* @param	TextureGroupID		Index/enum of the group
* @param	GroupName			String representation of the texture group
*/
FString FSystemSettings::GetLODGroupString( TextureGroup TextureGroupID, const TCHAR* GroupName )
{
	const FExposedTextureLODSettings::FTextureLODGroup& Group = TextureLODSettings.GetTextureLODGroup(TextureGroupID);

	const INT MinLODSize = 1 << Group.MinLODMipCount;
	const INT MaxLODSize = 1 << Group.MaxLODMipCount;

	FName MinMagFilter = NAME_Aniso;
	FName MipFilter = NAME_Linear;
	switch(Group.Filter)
	{
		case SF_Point:
			MinMagFilter = NAME_Point;
			MipFilter = NAME_Point;
			break;
		case SF_Bilinear:
			MinMagFilter = NAME_Linear;
			MipFilter = NAME_Point;
			break;
		case SF_Trilinear:
			MinMagFilter = NAME_Linear;
			MipFilter = NAME_Linear;
			break;
		case SF_AnisotropicPoint:
			MinMagFilter = NAME_Aniso;
			MipFilter = NAME_Point;
			break;
		case SF_AnisotropicLinear:
			MinMagFilter = NAME_Aniso;
			MipFilter = NAME_Linear;
			break;
	}

	FString NumStreamedMipsText;
	if ( Group.NumStreamedMips >= 0 )
	{
		NumStreamedMipsText = FString::Printf( TEXT(",NumStreamedMips=%i"), Group.NumStreamedMips );
	}

	return FString::Printf( TEXT( "(MinLODSize=%i,MaxLODSize=%i,LODBias=%i,MinMagFilter=%s,MipFilter=%s%s,MipGenSettings=%s)" ),
		MinLODSize, MaxLODSize, Group.LODBias, *MinMagFilter.GetNameString(), *MipFilter.GetNameString(), *NumStreamedMipsText, UTexture::GetMipGenSettingsString( Group.MipGenSettings ) );
}

/**
 * Writes the specified texture group LOD settings to the specified ini.
 *
 * @param	TextureGroupID		Index/enum of the group to parse
 * @param	GroupName			String representation of the texture group, to be used as the ini key.
 * @param	IniSection			The .ini section to save to.
 */
void FSystemSettings::WriteTextureLODGroupToIni( TextureGroup TextureGroupID, const TCHAR* GroupName, const TCHAR* IniSection )
{
	const FString Entry = GetLODGroupString( TextureGroupID, GroupName );
	// DISHONORED(retail): [SystemSettings] lives in the Engine ini; neither the 2012 nor the 2013 exe has a "SystemSettings.ini"
	// string or a GSystemSettingsIni global (FSystemSettingsData::WriteTextureLODGroupsToIni, 2013 rva 0x17b650)
	GConfig->SetString( IniSection, GroupName, *Entry, GEngineIni );
}

/**
 * Saves the current settings to the compat ini
 */
void FSystemSettings::SaveToIni( const FString IniSection )
{
	// DISHONORED(retail): FSystemSettingsData::SaveToIni, 2013 rva 0x181250, writes every key to GEngineIni and flushes it
	for( INT SettingIndex = 0; SettingIndex < ARRAY_COUNT( SystemSettings ); SettingIndex++ )
	{
		FSystemSetting* Setting = SystemSettings + SettingIndex;
		switch( Setting->SettingType )
		{
		case SST_BOOL:
			GConfig->SetBool( *IniSection, Setting->SettingName, *( UBOOL* )Setting->SettingAddress, GEngineIni );
			break;

		case SST_INT:
			GConfig->SetInt( *IniSection, Setting->SettingName, *( INT* )Setting->SettingAddress, GEngineIni );
			break;

		case SST_FLOAT:
			GConfig->SetFloat( *IniSection, Setting->SettingName, *( FLOAT* )Setting->SettingAddress, GEngineIni );
			break;
		}
	}

	// Save the texture group LOD settings.
#define WRITETEXTURELODGROUPTOINI( Group ) WriteTextureLODGroupToIni( Group, TEXT( #Group ), *IniSection );
	FOREACH_ENUM_TEXTUREGROUP( WRITETEXTURELODGROUPTOINI )
#undef WRITETEXTURELODGROUPTOINI

	GConfig->Flush( FALSE, GEngineIni );
}

/**
 * Dump helpers
 */
void FSystemSettings::DumpTextureLODGroup( FOutputDevice& Ar, TextureGroup TextureGroupID, const TCHAR* GroupName )
{
	const FString Entry = GetLODGroupString( TextureGroupID, GroupName );
	// DISHONORED(retail): golden log :10-38 "Log: \tTEXTUREGROUP_World: (...)" (2012 ArkProfile build); the shipping exes keep the
	// GetLODGroupString calls of FSystemSettingsData::DumpTextureLODGroups (2013 rva 0x17b7f0) with the debugf compiled out
	Ar.Logf( TEXT( "\t%s: %s" ), GroupName, *Entry );
}

/**
 * Dump texture scalability
 */
void FSystemSettings::DumpTextures( FOutputDevice& Ar )
{
	// Dump the TextureLODSettings
#define DUMPTEXTURELODGROUP( Group ) DumpTextureLODGroup( Ar, Group, TEXT( #Group ) );
	FOREACH_ENUM_TEXTUREGROUP(DUMPTEXTURELODGROUP)
#undef DUMPTEXTURELODGROUP
}

/**
 * Dump scalability
 */
void FSystemSettings::Dump( FOutputDevice& Ar, ESystemSettingIntent SettingIntent )
{
	for( INT SettingIndex = 0; SettingIndex < ARRAY_COUNT( SystemSettings ); SettingIndex++ )
	{
		FSystemSetting* Setting = SystemSettings + SettingIndex;
		if( Setting->SettingIntent == SettingIntent )
		{
			switch( Setting->SettingType )
			{
			case SST_BOOL:
				Ar.Logf( TEXT( "    %s = %s (%s)" ), Setting->SettingName, *( UBOOL* )Setting->SettingAddress ? TEXT( "TRUE" ) : TEXT( "FALSE" ), Setting->SettingHelp );
				break;

			case SST_INT:
				Ar.Logf( TEXT( "    %s = %d (%s)" ), Setting->SettingName, *( INT* )Setting->SettingAddress, Setting->SettingHelp );
				break;

			case SST_FLOAT:
				Ar.Logf( TEXT( "    %s = %g (%s)" ), Setting->SettingName, *( FLOAT* )Setting->SettingAddress, Setting->SettingHelp );
				break;
			}
		}
	}
}

/**
 * Constructor, initializing all member variables.
 */
FSystemSettings::FSystemSettings( void ) :
	bInit( FALSE ),
	bIsEditor( FALSE )
{
	NumberOfSystemSettings = ARRAY_COUNT( SystemSettings );
}

/**
 * Initializes system settings and included texture LOD settings.
 *
 * @param bSetupForEditor	Whether to initialize settings for Editor
 */
void FSystemSettings::Initialize( UBOOL bSetupForEditor )
{
	// DISHONORED(retail): FSystemSettings::Initialize, 2013 rva 0x1844e0 (2012 rva 0x18a660, systemsettings.cpp:942):
	//   1. bIsEditor = bSetupForEditor
	//   2. DefaultSettings <- [SystemSettings] of GEngineIni (never the editor section; missing keys are counted, never checked)
	//   3. Defaults[i][0] <- DefaultSettings + [AppCompatBucket<i+1>] of GCompatIni (or the plain section again when the bucket
	//      section is missing), Defaults[i][1] <- DefaultSettings + [SystemSettingsSplitScreen2] (i = 0..4): not ported, the
	//      table-driven FSystemSettings has no FSystemSettingsData copies (deferred with the 1052/11628-byte layout convergence;
	//      only SetCompatibilityLevelWindows, 2013 rva 0x5b5070, and Exec read them)
	//   4. *this = DefaultSettings, then FSystemSettings::LoadFromIni() (editor-aware section, -vsync/-novsync)
	//   5. -MAXQUALITYMODE, ApplySystemSettingsToRenderThread (no render-thread copy in this reference: rendering reads GSystemSettings)
	// No command-line overrides (-SS:, -LODBIAS:, -MAXLOD:, -MSAA are reference-only: ApplyOverrides is not called) and no
	// [TextureStreaming] MinTextureResidentMipCount read (the string does not exist in either exe).
	bIsEditor = bSetupForEditor;

	LoadFromIni( GetSectionName( FALSE, NULL ), GEngineIni, FALSE );

	LoadFromIni( NULL );

	if( ParseParam( appCmdLine(), TEXT( "MAXQUALITYMODE" ) ) )
	{
		bUseMaxQualityMode = TRUE;
	}
	if( bUseMaxQualityMode )
	{
		ShadowFilterQualityBias++;
		MaxAnisotropy = 16;
		MinShadowResolution = 16;
		MinPreShadowResolution = 16;
		ShadowTexelsPerPixel = 4.0f;
		MaxShadowResolution = 4096;
		MaxWholeSceneDominantShadowResolution = 4096;
		ShadowFadeResolution = 1;
		PreShadowFadeResolution = 1;
		PreShadowResolutionFactor = 1.0f;
		GSceneRenderTargets.SetSceneColorBufferFormat( PF_A32B32G32R32F );
		for( INT GroupIndex = 0; GroupIndex < TEXTUREGROUP_MAX; GroupIndex++ )
		{
			FTextureLODSettings::FTextureLODGroup& Group = TextureLODSettings.GetTextureLODGroup( (TextureGroup)GroupIndex );
			Group.MinLODMipCount = 12;
			Group.MaxLODMipCount = 12;
			Group.LODBias = -1000;
			Group.Filter = SF_AnisotropicLinear;
		}
	}

	bInit = TRUE;

	// DISHONORED(retail): no [TextureStreaming] MinTextureResidentMipCount read: the string exists in neither the 2012 nor the
	// 2013 exe and the retail ini has no such key; GMinTextureResidentMipCount keeps its static initializer (RHI.cpp, 7 in
	// the 2012 .data at rva 0xe2d470)
}

/**
 * Apply any special required overrides
 */
void FSystemSettings::ApplyOverrides( void )
{
	if( ParseParam( appCmdLine(), TEXT( "MSAA" ) ) )
	{
		MaxMultiSamples = GOptimalMSAALevel;
	}

	// look for commandline overrides of specific system settings, the format is:
	//    -ss:name1=val1,name2=val2
	FString Settings;
	if (Parse(appCmdLine(), TEXT("-SS:"), Settings, FALSE))
	{
		debugf(TEXT("Overriding system settings from the commandline: %s"), *Settings);

		// break apart on the commas
		TArray<FString> SettingPairs;
		Settings.ParseIntoArray(&SettingPairs, TEXT(","), TRUE);
		for (INT Index = 0; Index < SettingPairs.Num(); Index++)
		{
			// set each one, by splitting on the =
			FString Key, Value;
			if (SettingPairs(Index).Split(TEXT("="), &Key, &Value))
			{
				Exec(*FString::Printf(TEXT("scale set %s %s"), *Key, *Value), *GLog);
			}
		}
	}

	// look for commandline overrides for per lod group LODBias settings, format is:
	//		-lodbias:group1=val1,group2=val2
	// for instance:
	//		-lodbias:world=1,CharacterNormalMap=5
	if (Parse(appCmdLine(), TEXT("-LODBIAS:"), Settings, FALSE))
	{
		debugf(TEXT("Overriding LOD biases from the commandline:"), *Settings);

		// break apart on the commas
		TArray<FString> SettingPairs;
		Settings.ParseIntoArray(&SettingPairs, TEXT(","), TRUE);
		for (INT Index = 0; Index < SettingPairs.Num(); Index++)
		{
			// set each one, by splitting on the =
			FString Key, Value;
			if (SettingPairs(Index).Split(TEXT("="), &Key, &Value))
			{
				FString FullGroupName = FString("TEXTUREGROUP_") + Key;
				INT Bias = appAtoi(*Value);
				debugf(TEXT("   Setting group %s to %d"), *FullGroupName, Bias);
#define CHECKFORLODOVERRIDE(Group) if (FullGroupName == TEXT(#Group)) TextureLODSettings.GetTextureLODGroup(Group).LODBias = Bias; else
				FOREACH_ENUM_TEXTUREGROUP(CHECKFORLODOVERRIDE)
					// final block for final else
				{ }
#undef CHECKFORLODOVERRIDE
			}
		}
	}

	// look for commandline overrides for per lod group LODBias settings, format is:
	//		-maxlod:group1=val1,group2=val2
	// for instance:
	//		-maxlod:world=1,CharacterNormalMap=5
	if (Parse(appCmdLine(), TEXT("-MAXLOD:"), Settings, FALSE))
	{
		debugf(TEXT("Overriding Max LOD sizes from the commandline:"), *Settings);

		// break apart on the commas
		TArray<FString> SettingPairs;
		Settings.ParseIntoArray(&SettingPairs, TEXT(","), TRUE);
		for (INT Index = 0; Index < SettingPairs.Num(); Index++)
		{
			// set each one, by splitting on the =
			FString Key, Value;
			if (SettingPairs(Index).Split(TEXT("="), &Key, &Value))
			{
				FString FullGroupName = FString("TEXTUREGROUP_") + Key;
				INT MaxLODSize = appAtoi(*Value);
				debugf(TEXT("   Setting group %s to %d"), *FullGroupName, MaxLODSize);
#define CHECKFORLODOVERRIDE(Group) \
				if (FullGroupName == TEXT(#Group)) \
				{ \
					FTextureLODSettings::FTextureLODGroup& LODGroup = TextureLODSettings.GetTextureLODGroup(Group); \
					LODGroup.MaxLODMipCount = appCeilLogTwo(MaxLODSize); \
					LODGroup.MinLODMipCount = Min(LODGroup.MinLODMipCount, LODGroup.MaxLODMipCount); \
				} \
			else
				FOREACH_ENUM_TEXTUREGROUP(CHECKFORLODOVERRIDE)
					// final block for final else
				{ }
#undef CHECKFORLODOVERRIDE
			}
		}
	}

#if CONSOLE
	// Overwrite resolution from Ini with resolution from the console RHI
	ResX = GScreenWidth;
	ResY = GScreenHeight;
#endif

	// if system settings = max quality mode
	// No point in changing scene color format if depth is not stored in the alpha
	if( SystemSettingName == TEXT( "ScreenShot" ) && !GSupportsDepthTextures )
	{
		// Use a 32 bit fp scene color and depth, which reduces distant shadow artifacts due to storing scene depth in 16 bit fp significantly
		GSceneRenderTargets.SetSceneColorBufferFormat( PF_A32B32G32R32F );
	}
}

/**
 * Exec handler implementation.
 *
 * @param Cmd	Command to parse
 * @param Ar	Output device to log to
 *
 * @return TRUE if command was handled, FALSE otherwise
 */
UBOOL FSystemSettings::Exec( const TCHAR* Cmd, FOutputDevice& Ar )
{
	FSystemSettings OldSystemSettings = *this;

	// Keep track whether the command was handled or not.
	UBOOL bHandledCommand = FALSE;

	if( ParseCommand(&Cmd,TEXT("SCALE")) )
	{
		if( ParseCommand(&Cmd,TEXT("DUMP")) )
		{
			Ar.Logf( TEXT( "Current scalability system settings:" ) );
			Dump( Ar, SSI_SCALABILITY );
			return TRUE;
		}
		else if( ParseCommand( &Cmd, TEXT( "DUMPMOBILE" ) ) )
		{
			Ar.Logf( TEXT( "Current mobile scalability system settings:" ) );
			Dump( Ar, SSI_MOBILE_SCALABILITY );
			return TRUE;
		}
		else if( ParseCommand( &Cmd, TEXT( "DUMPPREFS" ) ) )
		{
			Ar.Logf( TEXT( "Current preference system settings:" ) );
			Dump( Ar, SSI_PREFERENCE );
			return TRUE;
		}
		else if( ParseCommand( &Cmd, TEXT( "DUMPDEBUG" ) ) )
		{
			Ar.Logf( TEXT( "Current debug system settings:" ) );
			Dump( Ar, SSI_DEBUG );
			return TRUE;
		}
		else if( ParseCommand( &Cmd, TEXT( "DUMPUNKNOWN" ) ) )
			{
			Ar.Logf( TEXT( "Current unknown system settings:" ) );
			Dump( Ar, SSI_UNKNOWN );
			return TRUE;
		}
		else if( ParseCommand( &Cmd, TEXT( "DUMPTEXTURES" ) ) )
		{
			Ar.Logf( TEXT( "Current texture settings:" ) );
			DumpTextures( Ar );
			return TRUE;
		}
		else if( ParseCommand( &Cmd, TEXT( "BUCKET" ) ) )
		{
			FString Token = ParseToken( Cmd, FALSE );
			bHandledCommand = LoadFromIni( *Token );
			if( !bHandledCommand )
			{
				Ar.Logf( TEXT( "Could not find section named '%s'" ), *Token );
			}
		}
		else if( ParseCommand(&Cmd,TEXT("LOWEND")) )
		{
			bHandledCommand = LoadFromIni( TEXT( "Bucket1" ) );
			if( !bHandledCommand )
			{
				Ar.Logf( TEXT( "Could not find section named 'Bucket1'" ) );
			}
		}
		else if( ParseCommand(&Cmd,TEXT("HIGHEND")) )
		{
			// Apply bucket5
			bHandledCommand = LoadFromIni( TEXT( "Bucket5" ) );
			if( !bHandledCommand )
			{
				Ar.Logf( TEXT( "Could not find section named 'Bucket5'" ) );
			}
		}
		else if( ParseCommand( &Cmd, TEXT( "SCREENSHOT" ) ) )
		{
			// Apply bucket5
			bHandledCommand = LoadFromIni( TEXT( "Screenshot" ) );
			if( !bHandledCommand )
			{
				Ar.Logf( TEXT( "Could not find section named 'Screenshot'" ) );
			}
		}
		else if( ParseCommand(&Cmd,TEXT("RESET")) )
		{
			// Reset values to defaults from ini.
			bHandledCommand = LoadFromIni( NULL );

#if CONSOLE
			// Overwrite resolution from Ini with resolution from the console RHI
			ResX = GScreenWidth;
			ResY = GScreenHeight;
#endif
		}
		else if( ParseCommand(&Cmd,TEXT("SET")) )
		{
			FString Token = ParseToken( Cmd, FALSE );
			FSystemSetting* Setting = FindSystemSetting( Token, SST_ANY );
			if( Setting == NULL )
			{
				Ar.Logf( TEXT( "Could not find setting named '%s'" ), *Token );
				return TRUE;
			}

			UBOOL bNewBoolValue = FALSE;
			INT NewIntValue = 0;
			FLOAT NewFloatValue = 0.0f;

			switch( Setting->SettingType )
			{
			case SST_BOOL:
				bNewBoolValue = ParseCommand( &Cmd, TEXT( "TRUE" ) );
				*( UBOOL* )Setting->SettingAddress = bNewBoolValue;
				Ar.Logf( TEXT( "Bool %s set to %u" ), Setting->SettingName, bNewBoolValue );
				bHandledCommand	= TRUE;
				break;

			case SST_INT:
				NewIntValue = appAtoi( Cmd );
				*( INT* )Setting->SettingAddress = NewIntValue;
				Ar.Logf( TEXT("Int %s set to %u"), Setting->SettingName, NewIntValue);
				bHandledCommand = TRUE;
				break;

			case SST_FLOAT:
				NewFloatValue = appAtof( Cmd );
				*( FLOAT* )Setting->SettingAddress = NewFloatValue;
				Ar.Logf( TEXT( "Float %s set to %g"), Setting->SettingName, NewFloatValue);
				bHandledCommand	= TRUE;
				break;
			}
		}
		else if( ParseCommand(&Cmd,TEXT("TOGGLE")) )
		{
			FString Token = ParseToken( Cmd, FALSE );
			FSystemSetting* Setting = FindSystemSetting( Token, SST_BOOL );
			if( Setting == NULL )
			{
				Ar.Logf( TEXT( "Could not find BOOL setting named '%s'" ), *Token );
				return TRUE;
			}

			*( UBOOL* )Setting->SettingAddress = !*( UBOOL* )Setting->SettingAddress;
			Ar.Logf( TEXT( "Bool %s toggled, new value %u"), Setting->SettingName, *( UBOOL* )Setting->SettingAddress );
			bHandledCommand	= TRUE;
		}
		else if( ParseCommand( &Cmd, TEXT( "SHRINK" ) ) )
		{
			FLOAT Step = ( 16.0f / 1280.0f ) * 100.0f;
			ScreenPercentage = Clamp( ScreenPercentage - Step, Step, 100.0f );
			Ar.Logf( TEXT( "ScreenPercentage shrunk to %f" ), ScreenPercentage );
			bHandledCommand	= TRUE;
		}
		else if( ParseCommand( &Cmd, TEXT( "EXPAND" ) ) )
		{
			FLOAT Step = ( 160.f / 1280.0f ) * 100.0f;
			ScreenPercentage = Clamp( ScreenPercentage + Step, Step, 100.0f );
			Ar.Logf( TEXT( "ScreenPercentage expanded to %f" ), ScreenPercentage );
			bHandledCommand	= TRUE;
		}
		else if ( ParseCommand(&Cmd, TEXT("ADJUST")) )
		{
			static UBOOL Adjusting = FALSE;
			static FString SaveLS;
			static FString SaveRS;
			Adjusting = ! Adjusting;
			UPlayerInput *Input = GEngine->GamePlayers(0)->Actor->PlayerInput;
			if( Adjusting )
			{
				SaveLS= Input->GetBind( TEXT("XboxTypeS_LeftShoulder") );
				SaveRS = Input->GetBind( TEXT("XboxTypeS_RightShoulder") );
				Input->ScriptConsoleExec( TEXT("setbind XboxTypeS_LeftShoulder scale decr"), Ar, NULL );
				Input->ScriptConsoleExec( TEXT("setbind XboxTypeS_RightShoulder scale incr"), Ar, NULL );
			}
			else 
			{
				FString SetBind;
				SetBind = FString::Printf( TEXT("setbind XboxTypeS_LeftShoulder %s"), *SaveLS );
				Input->ScriptConsoleExec( *SetBind, Ar, NULL );
				SetBind = FString::Printf( TEXT("setbind XboxTypeS_RightShoulder %s"), *SaveRS );
				Input->ScriptConsoleExec( *SetBind, Ar, NULL );
			}
			bHandledCommand = TRUE;
		}

		if (!bHandledCommand)
		{
			Ar.Logf(TEXT("Unrecognized system setting"));
			Ar.Logf( TEXT( "  Scale <Command> [parameter] [parameter]" ) );
			Ar.Logf( TEXT( "  Scale Dump - displays all scalability settings." ) );
			Ar.Logf( TEXT( "  Scale DumpMobile - displays all mobile scalability settings." ) );
			Ar.Logf( TEXT( "  Scale DumpPrefs - displays all preferences." ) );
			Ar.Logf( TEXT( "  Scale DumpDebug - displays all debug settings." ) );
			Ar.Logf( TEXT( "  Scale DumpUnknown - displays all uncategorised settings." ) );
			Ar.Logf( TEXT( "  Scale DumpTextures - displays the texture settings." ) );
			Ar.Logf( TEXT( "  Scale Bucket BucketName - sets the current settings to the contents of the SystemSettings<BucketName> section." ) );
			Ar.Logf( TEXT( "  Scale LowEnd - sets the current settings to Bucket1." ) );
			Ar.Logf( TEXT( "  Scale HighEnd - sets the current settings to Bucket5." ) );
			Ar.Logf( TEXT( "  Scale Screenshot - sets the current settings to the contents of the SystemSettingsScreenShot section." ) );
			Ar.Logf( TEXT( "  Scale Reset - loads in the default settings." ) );
			Ar.Logf( TEXT( "  Scale Set Key Value - sets the bool, int or float value of Key to Value." ) );
			Ar.Logf( TEXT( "  Scale Toggle Key - toggles the state of the bool named Key." ) );
			Ar.Logf( TEXT( "  Scale Shrink - decrements ScreenPercentage." ) );
			Ar.Logf( TEXT( "  Scale Expand - increments ScreenPercentage." ) );
		}
		else
		{
			// Write the new settings to the INI.
			SaveToIni();

			// Apply the settings
			ApplySettings( OldSystemSettings );
		}
	}

	return bHandledCommand;
}


/**
 * Scale X,Y offset/size of screen coordinates if the screen percentage is not at 100%
 *
 * @param X - in/out X screen offset
 * @param Y - in/out Y screen offset
 * @param SizeX - in/out X screen size
 * @param SizeY - in/out Y screen size
 */
void FSystemSettings::ScaleScreenCoords( INT& X, INT& Y, UINT& SizeX, UINT& SizeY )
{
	// Take screen percentage option into account if percentage != 100.
	if( GSystemSettings.ScreenPercentage != 100.0f && !bIsEditor )
	{
		// Clamp screen percentage to reasonable range.
		FLOAT ScaleFactor = Clamp( GSystemSettings.ScreenPercentage / 100.f, 0.0f, 1.0f );

		FLOAT ScaleFactorX = ScaleFactor;
		FLOAT ScaleFactorY = ScaleFactor;
#if XBOX // When bUpscaleScreenPercentage is false, we use the hardware scaler, which has alignment constraints
		if( !bUpscaleScreenPercentage )
		{
			ScaleFactorX = ( INT( ScaleFactor * GScreenWidth  + 0.5f ) & ~0xf ) / FLOAT( GScreenWidth  );
			ScaleFactorY = ( INT( ScaleFactor * GScreenHeight + 0.5f ) & ~0xf ) / FLOAT( GScreenHeight );
		}
#endif
		INT	OrigX = X;
		INT OrigY = Y;
		UINT OrigSizeX = SizeX;
		UINT OrigSizeY = SizeY;

		// Scale though make sure we're at least covering 1 pixel.
		SizeX = Max(1,appTrunc(ScaleFactorX * OrigSizeX));
		SizeY = Max(1,appTrunc(ScaleFactorY * OrigSizeY));

		// Center scaled view.
		X = OrigX + (OrigSizeX - SizeX) / 2;
		Y = OrigY + (OrigSizeY - SizeY) / 2;
	}
}

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
void FSystemSettings::UnScaleScreenCoords( 
	INT &OriginalX, INT &OriginalY, 
	UINT &OriginalSizeX, UINT &OriginalSizeY, 
	FLOAT InX, FLOAT InY, 
	FLOAT InSizeX, FLOAT InSizeY)
{
	if (NeedsUpscale())
	{
		FLOAT ScaleFactor = Clamp( GSystemSettings.ScreenPercentage / 100.f, 0.0f, 1.0f );

		//undo scaling
		OriginalSizeX = appTrunc(InSizeX / ScaleFactor);
		OriginalSizeY = appTrunc(InSizeY / ScaleFactor);

		//undo centering
		OriginalX = appTrunc(InX - (OriginalSizeX - InSizeX) / 2.0f);
		OriginalY = appTrunc(InY - (OriginalSizeY - InSizeY) / 2.0f);
	}
	else
	{
		OriginalSizeX = appTrunc(InSizeX);
		OriginalSizeY = appTrunc(InSizeY);

		OriginalX = appTrunc(InX);
		OriginalY = appTrunc(InY);
	}
}

/** Indicates whether upscaling is needed */
UBOOL FSystemSettings::NeedsUpscale( void ) const
{
	return bUpscaleScreenPercentage && !bIsEditor && ( ScreenPercentage < 100.0f );
}

/**
 * Reads a single entry and parses it into the group array.
 *
 * @param	TextureGroupID		Index/enum of group to parse
 * @param	MinLODSize			Minimum size, in pixels, below which the code won't bias.
 * @param	MaxLODSize			Maximum size, in pixels, above which the code won't bias.
 * @param	LODBias				Group LOD bias.
 */
void FSystemSettings::SetTextureLODGroup(TextureGroup TextureGroupID, INT MinLODSize, INT MaxLODSize, INT LODBias, TextureMipGenSettings MipGenSettings)
{
	TextureLODSettings.GetTextureLODGroup(TextureGroupID).MinLODMipCount	= appCeilLogTwo( MinLODSize );
	TextureLODSettings.GetTextureLODGroup(TextureGroupID).MaxLODMipCount	= appCeilLogTwo( MaxLODSize );
	TextureLODSettings.GetTextureLODGroup(TextureGroupID).LODBias			= LODBias;
	TextureLODSettings.GetTextureLODGroup(TextureGroupID).MipGenSettings	= MipGenSettings;
}

/**
* Recreates texture resources and drops mips.
*
* @return		TRUE if the settings were applied, FALSE if they couldn't be applied immediately.
*/
UBOOL FSystemSettings::UpdateTextureStreaming( void )
{
	if ( GStreamingManager )
	{
		// Make sure textures can be streamed out so that we can unload current mips.
		const UBOOL bOldOnlyStreamInTextures = bOnlyStreamInTextures;
		bOnlyStreamInTextures = FALSE;

		for( TObjectIterator<UTexture2D> It; It; ++It )
		{
			UTexture* Texture = *It;

			// Update cached LOD bias.
			Texture->CachedCombinedLODBias = TextureLODSettings.CalculateLODBias( Texture );
		}

		// Make sure we iterate over all textures by setting it to high value.
		GStreamingManager->SetNumIterationsForNextFrame( 100 );
		// Update resource streaming with updated texture LOD bias/ max texture mip count.
		GStreamingManager->UpdateResourceStreaming( 0 );
		// Block till requests are finished.
		GStreamingManager->BlockTillAllRequestsFinished();

		// Restore streaming out of textures.
		bOnlyStreamInTextures = bOldOnlyStreamInTextures;
	}

	return TRUE;
}

/**
 * Makes System Settings take effect on the renderthread
 */
void FSystemSettings::SceneRenderTargetsUpdateRHI( const FSystemSettings& OldSettings, const FSystemSettings& NewSettings )
{
	// Shouldn't be reallocating render targets on console
#if !CONSOLE

	// Should we create or release certain rendertargets?
	const UBOOL bUpdateRenderTargets =
		(OldSettings.bAllowMotionBlur						!= NewSettings.bAllowMotionBlur) ||
		(OldSettings.bAllowAmbientOcclusion					!= NewSettings.bAllowAmbientOcclusion) ||
		(OldSettings.bAllowDynamicShadows					!= NewSettings.bAllowDynamicShadows) ||
		(OldSettings.bAllowHardwareShadowFiltering			!= NewSettings.bAllowHardwareShadowFiltering) ||
		(OldSettings.bAllowFogVolumes						!= NewSettings.bAllowFogVolumes) ||
		(OldSettings.bAllowSubsurfaceScattering				!= NewSettings.bAllowSubsurfaceScattering) ||
		(OldSettings.bHighPrecisionGBuffers					!= NewSettings.bHighPrecisionGBuffers) ||
		(OldSettings.bAllowTemporalAA						!= NewSettings.bAllowTemporalAA) ||
		(OldSettings.MaxMultiSamples						!= NewSettings.MaxMultiSamples) ||
		(OldSettings.bAllowPostprocessMLAA					!= NewSettings.bAllowPostprocessMLAA) ||
		(OldSettings.MinShadowResolution					!= NewSettings.MinShadowResolution) ||
		(OldSettings.MaxShadowResolution					!= NewSettings.MaxShadowResolution) ||
		(OldSettings.MaxWholeSceneDominantShadowResolution	!= NewSettings.MaxWholeSceneDominantShadowResolution);

	// Activate certain system settings on the renderthread
	if(bUpdateRenderTargets)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND(
			SceneRenderTargetsUpdateRHICommand,
		{
			UpdateSceneRenderTargetsRHI();
		});
	}
#endif
}

/**
 * Cause a call to UpdateRHI on GSceneRenderTargets
 */
void FSystemSettings::UpdateSceneRenderTargetsRHI()
{
	GSceneRenderTargets.UpdateRHI();
}

/** 
 * Sets the resolution and writes the values to Ini if changed but does not apply the changes (e.g. resize the viewport).
 */
void FSystemSettings::SetResolution(INT InSizeX, INT InSizeY, UBOOL InFullscreen)
{
	if (!bIsEditor)
	{
		const UBOOL bResolutionChanged = 
			ResX != InSizeX ||
			ResY != InSizeY ||
			bFullscreen != InFullscreen;

		if (bResolutionChanged)
		{
			ResX = InSizeX;
			ResY = InSizeY;
			bFullscreen = InFullscreen;
			SaveToIni();
		}
	}
}

/**
 * Overridden function that selects the proper ini section to read from or write to
 */
UBOOL FSystemSettings::LoadFromIni( const TCHAR* Override )
{
	FString SectionName = GetSectionName( bIsEditor, Override );

	// DISHONORED(retail): FSystemSettings::LoadFromIni(), 2013 rva 0x181af0 (2012 rva 0x187b70): reads GetSectionName(bIsEditor)
	// from GEngineIni with missing values allowed (no section-exists check), then the vsync command-line switches
	LoadFromIni( SectionName, GEngineIni, TRUE );

#if CONSOLE
	// Always default to using VSYNC on consoles.
	bUseVSync = TRUE;
#endif

	// Disable VSYNC if -novsync is on the command line.
	bUseVSync = bUseVSync && !ParseParam(appCmdLine(), TEXT("novsync"));

	// Enable VSYNC if -vsync is on the command line.
	bUseVSync = bUseVSync || ParseParam(appCmdLine(), TEXT("vsync"));

	return TRUE;
}

void FSystemSettings::SaveToIni( void )
{
	// don't write changes in the editor
	if (bIsEditor)
	{
		debugf(TEXT("Can't save system settings to ini in an editor mode"));
		return;
	}

	FSystemSettings::SaveToIni( GetSectionName( bIsEditor, TEXT( "" ) ) );
}

/**
 * Apply any settings that have changed
 */
void FSystemSettings::ApplySettings( FSystemSettings& OldSystemSettings )
{
	// Some of these settings are shared between threads, so we must flush the rendering thread before changing anything.
	FlushRenderingCommands();

	// We don't support switching lightmap type at run-time.
	if( bAllowDirectionalLightMaps != OldSystemSettings.bAllowDirectionalLightMaps )
	{
		debugf( TEXT( "Can't enable/disable directional lightmaps at run-time." ) );
		bAllowDirectionalLightMaps = OldSystemSettings.bAllowDirectionalLightMaps;
	}

	if( OldSystemSettings.bForceCPUAccessToGPUSkinVerts != bForceCPUAccessToGPUSkinVerts || OldSystemSettings.bDisableSkeletalInstanceWeights != bDisableSkeletalInstanceWeights )
	{
		debugf( TEXT( "Can't change bForceCPUAccessToGPUSkinVerts or bDisableSkeletalInstanceWeights at run-time." ) );
		bForceCPUAccessToGPUSkinVerts = OldSystemSettings.bForceCPUAccessToGPUSkinVerts;
		bDisableSkeletalInstanceWeights = OldSystemSettings.bDisableSkeletalInstanceWeights;
	}

	// Reattach components if world-detail settings have changed.
	if( OldSystemSettings.DetailMode != DetailMode || OldSystemSettings.bAllowHighQualityMaterials != bAllowHighQualityMaterials )
	{
		// decals should always reattach after all other primitives have been attached
		TArray<UClass*> ExcludeComponents;
		ExcludeComponents.AddItem( UDecalComponent::StaticClass() );
		ExcludeComponents.AddItem( UAudioComponent::StaticClass() );

		FGlobalComponentReattachContext PropagateDetailModeChanges( ExcludeComponents );
	}

	// Reattach decal components if needed
	if( OldSystemSettings.DetailMode != DetailMode )
	{
		TComponentReattachContext<UDecalComponent> PropagateDecalComponentChanges;
	}

	if( OldSystemSettings.bAllowRadialBlur != bAllowRadialBlur )
	{
		TComponentReattachContext<URadialBlurComponent> PropagateRadialBlurComponentChanges;
	}

	// Update the texture detail
	GSystemSettings.UpdateTextureStreaming();

	// Update the screen resolution
	if( OldSystemSettings.ResX != ResX || OldSystemSettings.ResY != ResY || OldSystemSettings.bFullscreen != bFullscreen )
	{
		if( GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportFrame )
		{
			GEngine->GameViewport->ViewportFrame->Resize( ResX, ResY, bFullscreen );
		}
	}

	// Activate certain system settings on the renderthread
	FSystemSettings::SceneRenderTargetsUpdateRHI( OldSystemSettings, *this );
}

/**
 * Sets new system settings (optionally writes out to the ini). 
 */
void FSystemSettings::ApplyNewSettings( const FSystemSettings& NewSettings, UBOOL bWriteToIni )
{
	// we can set any setting before the engine is initialized so don't bother restoring values.
	UBOOL bEngineIsInitialized = ( GEngine != NULL );

	// if the engine is running, there are certain values we can't set immediately
	if (bEngineIsInitialized)
	{
		// Make a copy of the existing settings so we can compare for changes
		FSystemSettings OldSystemSettings = *this;

		// Read settings from .ini.  This is necessary because settings which need to wait for a restart will be on disk
		// but may not be in memory.  Therefore, we read from disk before capturing old values to revert to.
		LoadFromIni( NULL );

		// apply settings to the runtime system.
		ApplySettings( OldSystemSettings );

		// If requested, save the settings to ini.
		if ( bWriteToIni )
		{
			SaveToIni();
		}
	}
	else
	{
		// if the engine is not initialized we don't need to worry about all the deferred settings etc. as we do above.
		( FSystemSettings& )( *this ) = NewSettings;

		// If requested, save the settings to ini.
		if( bWriteToIni )
		{
			SaveToIni();
		}
	}

	// DISHONORED(retail): FSystemSettings::ApplyNewSettings, 2013 rva 0x184be0 (2012 rva 0x192c80): no command-line overrides
	// (ApplyOverrides is reference-only) and, when writing to the ini outside the editor, the texture LOD groups are dumped
	// (golden log :10-38, reached through appSetCompatibilityLevel -> SetCompatibilityLevelWindows, 2013 rva 0x5b5070)
	if( bWriteToIni && !bIsEditor )
	{
		DumpTextures( *GLog );
	}
}

/**
 * Ensures that the correct settings are being used based on split screen type.
 */
void FSystemSettings::UpdateSplitScreenSettings( void )
{
}



