/*=============================================================================
	UnSkeletalMesh.h: Unreal skeletal mesh objects.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

/*-----------------------------------------------------------------------------
	USkeletalMeshComponent.
-----------------------------------------------------------------------------*/

#ifndef __UNSKELETALMESH_H__
#define __UNSKELETALMESH_H__

#if WITH_FACEFX
	#include "UnFaceFXSupport.h"
#endif // WITH_FACEFX

#include "GPUSkinPublicDefs.h"

#include "NvApexManager.h"

#include "EnginePhysicsClasses.h"

// Forward references the FIApexClothing interface pointer, which is only valid if WITH_APEX is enabled.
class FIApexClothing; 


// Define that controls showing chart of distance factors for skel meshes during entire run of the game on exit.
#define CHART_DISTANCE_FACTORS 0

//
//	FAttachment
//
struct FAttachment
{
	UActorComponent*	Component;
	FName				BoneName;
	FVector				RelativeLocation;
	FRotator			RelativeRotation;
	FVector				RelativeScale;

	// Constructor.

	FAttachment(UActorComponent* InComponent,FName InBoneName,FVector InRelativeLocation,FRotator InRelativeRotation,FVector InRelativeScale):
		Component(InComponent),
		BoneName(InBoneName),
		RelativeLocation(InRelativeLocation),
		RelativeRotation(InRelativeRotation),
		RelativeScale(InRelativeScale)
	{}
	FAttachment() {}
};
template <> struct TIsPODType<FAttachment> { enum { Value = true }; };

//
//	FBoneRotationControl
//
struct FBoneRotationControl
{
	INT			BoneIndex;
	FName		BoneName;
	FRotator	BoneRotation;
	BYTE		BoneSpace;

	// Constructor.
	FBoneRotationControl(INT InBoneIndex, FName InBoneName, FRotator InBoneRotation, BYTE InBoneSpace):
		BoneIndex(InBoneIndex),
		BoneName(InBoneName),
		BoneRotation(InBoneRotation),
		BoneSpace(InBoneSpace)
	{}
	FBoneRotationControl() {}
};
template <> struct TIsPODType<FBoneRotationControl> { enum { Value = true }; };

//
//	FBoneTranslationControl
//
struct FBoneTranslationControl
{
	INT			BoneIndex;
	FName		BoneName;
	FVector		BoneTranslation;

	FBoneTranslationControl(INT InBoneIndex,  FName InBoneName, const FVector& InBoneTranslation):
		BoneIndex(InBoneIndex),
		BoneName(InBoneName),
		BoneTranslation(InBoneTranslation)
	{}
	FBoneTranslationControl() {}
};
template <> struct TIsPODType<FBoneTranslationControl> { enum { Value = true }; };

/** Struct used to indicate one active morph target that should be applied to this SkeletalMesh when rendered. */
struct FActiveMorph
{
	/** The morph target that we want to apply. */
	class UMorphTarget*	Target;

	/** Strength of the morph target, between 0.0 and 1.0 */
	FLOAT				Weight;

	FActiveMorph(class UMorphTarget* InTarget, FLOAT InWeight):
		Target(InTarget),
		Weight(InWeight)
	{}
	FActiveMorph() {}

	UBOOL operator==(const FActiveMorph& Other) const
	{
		// if target is same, we consider same 
		// any equal operator to check if it's same, 
		// we just check if this is same morph target
		return Target == Other.Target;
	}
};
template <> struct TIsPODType<FActiveMorph> { enum { Value = true }; };


/** A pair of bone names */
struct FBonePair
{
	FName Bones[2];

	UBOOL operator==(const FBonePair& Other) const
	{
		return Bones[0] == Other.Bones[0] && Bones[1] == Other.Bones[1];
	}

	UBOOL IsMatch(const FBonePair& Other) const
	{
		return	(Bones[0] == Other.Bones[0] || Bones[1] == Other.Bones[0]) &&
				(Bones[0] == Other.Bones[1] || Bones[1] == Other.Bones[1]);
	}
};

/** 
* A pair of bone indices
*/
struct FBoneIndexPair
{
	INT BoneIdx[2];

	UBOOL operator==(const FBoneIndexPair& Src) const
	{
		return (BoneIdx[0] == Src.BoneIdx[0]) && (BoneIdx[1] == Src.BoneIdx[1]);
	}

	friend FORCEINLINE DWORD GetTypeHash( const FBoneIndexPair& BonePair )
	{
		return appMemCrc(&BonePair, sizeof(FBoneIndexPair));
	}

	/**
	* Serialize to Archive
	*/
	friend FArchive& operator<<( FArchive& Ar, FBoneIndexPair& W )
	{
		return Ar << W.BoneIdx[0] << W.BoneIdx[1];
	}
};

enum ERootMotionMode
{
	RMM_Translate	= 0,
	RMM_Velocity	= 1,
	RMM_Ignore		= 2,
	RMM_Accel		= 3,
	RMM_Relative	= 4,
	RMM_MAX			= 5,
};

enum ERootMotionRotationMode
{
	RMRM_Ignore			= 0,
	RMRM_RotateActor	= 1,
	RMRM_MAX			= 2,
};

enum EFaceFXBlendMode
{
	FXBM_Overwrite	= 0,
	FXBM_Additive	= 1,
	FXBM_MAX		= 2,
};

enum EFaceFXRegOp
{
	FXRO_Add      = 0,	   
	FXRO_Multiply = 1, 
	FXRO_Replace  = 2,  
};

/** The valid BoneVisibilityStates values; A bone is only visible if it is *exactly* 1 */
enum EBoneVisibilityStatus
{
	BVS_HiddenByParent		= 0,	// Bone is hidden because it's parent is hidden
	BVS_Visible				= 1,	// Bone is visible
	BVS_ExplicitlyHidden	= 2,	// Bone is hidden directly
};

/** PhysicsBody options when bone is hiddne */
enum EPhysBodyOp
{
	PBO_None	= 0, // don't do anything
	PBO_Term	= 1, // terminate - if you terminate, you won't be able to re-init when unhidden
	PBO_Disable	= 2, // disable collision - it will enable collision when unhidden
};

enum EAnimRotationOnly
{
	/** Use settings defined in each AnimSet (default) */
	EARO_AnimSet = 0,
	/** Force AnimRotationOnly enabled on all AnimSets, but for this SkeletalMesh only */
	EARO_ForceEnabled = 1,
	/** Force AnimRotationOnly disabled on all AnimSets, but for this SkeletalMesh only */
	EARO_ForceDisabled = 2,
	EARO_MAX = 3,
};

/** Usage cases for toggling vertex weights */
enum EInstanceWeightUsage
{
	/** Weights are swapped for a subset of vertices. Requires a unique weights vertex buffer per skel component instance. */
	IWU_PartialSwap = 0,
	/** Weights are swapped for ALL vertices.  Shares a weights vertex buffer for all skel component instances. */
	IWU_FullSwap,
	IWU_Max
};

/** Which set of indices to select for TRISORT_CustomLeftRight sections. */
enum ECustomSortAlternateIndexMode
{
	CSAIM_Auto = 0,
	CSAIM_Left = 1,
	CSAIM_Right = 2,
};

/** 
 *  Enum to define how to scale max distance
 *  @see SetApexClothingMaxDistanceScale
 */
enum EMaxDistanceScaleMode
{
	MDSM_Multiply   =0,
	MDSM_Substract  =1,
	MDSM_MAX		=2,
};


/** LOD specific setup for the skeletal mesh component */
struct FSkelMeshComponentLODInfo
{
	/** Material corresponds to section. To show/hide each section, use this **/
	TArrayNoInit<UBOOL> HiddenMaterials;
	/** If TRUE, update the instanced vertex influences for this mesh during the next update */
	BITFIELD bNeedsInstanceWeightUpdate:1;
	/** If TRUE, always use instanced vertex influences for this mesh */
	BITFIELD bAlwaysUseInstanceWeights:1;
	/** Align the following byte */
	SCRIPT_ALIGN;
	/** Whether the instance weights are used for a partial/full swap */
	BYTE InstanceWeightUsage;
	/** Current index into the skeletal mesh VertexInfluences for the current LOD */
	INT InstanceWeightIdx;

	FSkelMeshComponentLODInfo()	 :
		  bNeedsInstanceWeightUpdate(FALSE)
		, bAlwaysUseInstanceWeights(FALSE)
		, InstanceWeightUsage(IWU_PartialSwap)
		, InstanceWeightIdx(INDEX_NONE)
	{ appMemzero(&HiddenMaterials, sizeof(TArray<UBOOL>));}
};

//
//	USkeletalMeshComponent
//
class USkeletalMesh;
struct FEdgeAnimData;
// DISHONORED(layout): 2012 PDB USkeletalMeshComponent is 1056 bytes (see the member block); FTickData m_TickData @976 (80 bytes, 16-aligned)
class USkeletalMeshComponent : public UMeshComponent
{
	DECLARE_CLASS_NOEXPORT(USkeletalMeshComponent,UMeshComponent,0,Engine)

	struct FTickData
	{
		FMatrix ParentTransform;
		BYTE bLODHasChanged;
		BYTE bUpdateKinematics;
		BYTE m_bUpdateSkelPoseCalled;
		BYTE m_bNeedUpdateTransform;
	};

	// DISHONORED(layout): 2012 PDB USkeletalMeshComponent is 1056 bytes; data members regenerated in types.json order
	// (reference declarations reused by name, Arkane members synthesized; see agents/agentM.md)
	USkeletalMesh*						SkeletalMesh;
	USkeletalMeshComponent*				AttachedToSkelComponent;
	class UAnimTree*					AnimTreeTemplate;
	class UAnimNode*					Animations;
	TArray<class USkelControlBase*>		SkelControlTickArray;
	class UPhysicsAsset*				PhysicsAsset;
	class UPhysicsAssetInstance*		PhysicsAssetInstance;
	FLOAT								PhysicsWeight;
	FLOAT								GlobalAnimRateScale;
	class FSkeletalMeshObject*			MeshObject;
	FColor								WireframeColor;
	TArray <FBoneAtom>				SpaceBases;
	TArray <FBoneAtom>					LocalAtoms;
	TArray <FBoneAtom>					CachedLocalAtoms;
	TArray <FBoneAtom>				CachedSpaceBases;
	INT									LowUpdateFrameRate;
	TArray<BYTE>						RequiredBones;
	TArray<BYTE>						ComposeOrderedRequiredBones;
	USkeletalMeshComponent*				ParentAnimComponent;
	TArrayNoInit<INT>					ParentBoneMap;
	TArrayNoInit<class UAnimSet*>			AnimSets;
	TArrayNoInit<class UAnimSet*> TemporarySavedAnimSets;
	TArrayNoInit<FAttachment>				Attachments;
	TArrayNoInit<BYTE>						SkelControlIndex;
	TArrayNoInit<BYTE>						PostPhysSkelControlIndex;
	INT									ForcedLodModel;
	INT									MinLodModel;
	INT									PredictedLODLevel;
	INT									OldPredictedLODLevel;
	FLOAT								MaxDistanceFactor;
	UBOOL								bForceWireframe;
	UBOOL								bForceRefpose;
	UBOOL								bOldForceRefPose;
	UBOOL								bNoSkeletonUpdate;
	UBOOL								bDisplayBones;
	UBOOL								bShowPrePhysBones;
	UBOOL								bHideSkin;
	UBOOL								bForceRawOffset;
	UBOOL								bIgnoreControllers;
	UBOOL								bTransformFromAnimParent;
	UINT m_bReallyInheritTransformFromAnimParent;  // DISHONORED(layout): 2012 PDB @732
	INT									TickTag;
	INT									InitTag;
	INT									CachedAtomsTag;
	UBOOL								bRequiredBonesUpToDate;
	FLOAT								MinDistFactorForKinematicUpdate;
	INT									FramesPhysicsAsleep;
	BITFIELD bSkipAllUpdateWhenPhysicsAsleep:1;
	BITFIELD bConsiderAllBodiesForBounds:1;
	BITFIELD bUpdateSkelWhenNotRendered:1;
	BITFIELD bIgnoreControllersWhenNotRendered:1;
	BITFIELD bTickAnimNodesWhenNotRendered:1;
	BITFIELD bNotUpdatingKinematicDueToDistance:1;
	BITFIELD bForceDiscardRootMotion:1;
	// DISHONORED(layout): retail 2013 removed bRootMotionModeChangeNotify / bRootMotionExtractedNotify (and the RootMotion* delegates)
	BITFIELD bDisableFaceFXMaterialInstanceCreation:1;
	BITFIELD bAnimTreeInitialised:1;
	BITFIELD bForceMeshObjectUpdate:1;
	BITFIELD bHasPhysicsAssetInstance:1;
	BITFIELD bUpdateKinematicBonesFromAnimation:1;
	BITFIELD bUpdateJointsFromAnimation:1;
	BITFIELD bSkelCompFixed:1;
	BITFIELD bHasHadPhysicsBlendedIn:1;
	BITFIELD bForceUpdateAttachmentsInTick:1;
	BITFIELD bEnableFullAnimWeightBodies:1;
	BITFIELD bPerBoneVolumeEffects:1;
	BITFIELD bSyncActorLocationToRootRigidBody:1;
	BITFIELD bUseRawData:1;
	BITFIELD bDisableWarningWhenAnimNotFound:1;
	BITFIELD bOverrideAttachmentOwnerVisibility:1;
	BITFIELD bNeedsToDeleteHitMask:1;
	BITFIELD bPauseAnims:1;
	BITFIELD m_bDisableFaceFx:1;  // DISHONORED(layout): 2012 PDB @763
	BITFIELD m_bSkipUpdate:1;  // DISHONORED(layout): 2012 PDB @763
	BITFIELD bEnableLineCheckWithBounds:1;
	FVector LineCheckBoundsScale;
	BITFIELD m_bDontUpdateKinematic:1;  // DISHONORED(layout): 2012 PDB @776
	BITFIELD bRecentlyRendered:1;
	BITFIELD bCacheAnimSequenceNodes:1;
	BITFIELD bUpdateComposeSkeletonPasses:1;
	BITFIELD bValidTemporarySavedAnimSets:1;
	TArrayNoInit<FBonePair> InstanceVertexWeightBones;
	TArrayNoInit<FSkelMeshComponentLODInfo> LODInfo;
	UMaterialInterface*					LimitMaterial;
	FBoneAtom	RootMotionDelta;
	FVector		RootMotionVelocity;
	FVector		RootBoneTranslation;
	FVector		RootMotionAccelScale;
	FLOAT RootRotationScale;  // DISHONORED(layout): 2012 PDB @884
	FRotator AdditionalRootRotation;  // DISHONORED(layout): 2012 PDB @888
	BYTE	RootMotionMode;
	BYTE	PreviousRMM;
	BYTE	PendingRMM;
	BYTE	OldPendingRMM;
	SCRIPT_ALIGN;
	INT		bRMMOneFrameDelay;
	BYTE	RootMotionRotationMode;
	BYTE	FaceFXBlendMode;
	SCRIPT_ALIGN;
#if WITH_FACEFX
	OC3Ent::Face::FxActorInstance* FaceFXActorInstance;
#else
	void* FaceFXActorInstance;
#endif
	class UFaceFXAsset* m_pFaceFXAsset;  // DISHONORED(layout): 2012 PDB @916
	FLOAT m_fFaceFxTickTime;  // DISHONORED(layout): 2012 PDB @920
	class UActorComponent* m_pFaceFxAudioHandler;  // DISHONORED(layout): 2012 PDB @924
	FBoneAtom LocalToWorldBoneAtom;
	float ProgressiveDrawingFraction;
	BYTE CustomSortAlternateIndexMode;
	FEdgeAnimData* m_pEdgeAnimData;  // DISHONORED(layout): 2012 PDB @968
	USkeletalMeshComponent::FTickData m_TickData;  // DISHONORED(layout): 2012 PDB @976
	FBoneAtom RawExtractedRootMotionDelta;  // DISHONORED(layout): retail 2013 only, appended after m_TickData (1056 -> 1088)

	// DISHONORED(layout): reference-only members absent from the 2012 PDB. Kept as storage-less C++17
	// inline statics (DISHONORED_SHIM_STATIC, Engine.h) so unported reference code still compiles; they are not part of the object layout
	// and the module port has to remove their uses (resources/docs/agents/agentM.md lists them).
	DISHONORED_SHIM_STATIC TArray<UAnimNode*> AnimTickArray;
	DISHONORED_SHIM_STATIC TArray<UAnimNode*> AnimAlwaysTickArray;
	DISHONORED_SHIM_STATIC TArray<INT> AnimTickRelevancyArray;
	DISHONORED_SHIM_STATIC TArray<FLOAT> AnimTickWeightsArray;
	DISHONORED_SHIM_STATIC FIApexClothing* ApexClothing;
	DISHONORED_SHIM_STATIC FLOAT StreamingDistanceMultiplier;
	DISHONORED_SHIM_STATIC TArrayNoInit<class UMorphTargetSet*> MorphSets;
	DISHONORED_SHIM_STATIC TArrayNoInit<FActiveMorph> ActiveMorphs;
	DISHONORED_SHIM_STATIC TArrayNoInit<FActiveMorph> ActiveCurveMorphs;
	DISHONORED_SHIM_STATIC TMap<FName, UMorphTarget*> MorphTargetIndexMap;
	DISHONORED_SHIM_STATIC FLOAT AnimationLODDistanceFactor;
	DISHONORED_SHIM_STATIC INT AnimationLODFrameRate;
	DISHONORED_SHIM_STATIC INT ChunkIndexPreview;
	DISHONORED_SHIM_STATIC INT SectionIndexPreview;
	DISHONORED_SHIM_STATIC BITFIELD bUseSingleBodyPhysics;
	DISHONORED_SHIM_STATIC INT SkipRateForTickAnimNodesAndGetBoneAtoms;
	DISHONORED_SHIM_STATIC BITFIELD bSkipTickAnimNodes;
	DISHONORED_SHIM_STATIC BITFIELD bSkipGetBoneAtoms;
	DISHONORED_SHIM_STATIC BITFIELD bInterpolateBoneAtoms;
	DISHONORED_SHIM_STATIC BITFIELD bHasValidBodies;
	DISHONORED_SHIM_STATIC BITFIELD bComponentUseFixedSkelBounds;
	DISHONORED_SHIM_STATIC BITFIELD bUseBoundsFromParentAnimComponent;
	DISHONORED_SHIM_STATIC BITFIELD bNotifyRootMotionProcessed;
	DISHONORED_SHIM_STATIC BITFIELD bProcessingRootMotion;
	DISHONORED_SHIM_STATIC BITFIELD bDisableFaceFX;
	DISHONORED_SHIM_STATIC BITFIELD bPerBoneMotionBlur;
	DISHONORED_SHIM_STATIC BITFIELD bChartDistanceFactor;
	DISHONORED_SHIM_STATIC BITFIELD bCanHighlightSelectedSections;
	DISHONORED_SHIM_STATIC BITFIELD bUpdateMorphWhenParentAnimComponentExists;
	DISHONORED_SHIM_STATIC BITFIELD bEnableClothSimulation;
	DISHONORED_SHIM_STATIC BITFIELD bDisableClothCollision;
	DISHONORED_SHIM_STATIC BITFIELD bClothFrozen;
	DISHONORED_SHIM_STATIC BITFIELD bAutoFreezeClothWhenNotRendered;
	DISHONORED_SHIM_STATIC BITFIELD bClothAwakeOnStartup;
	DISHONORED_SHIM_STATIC BITFIELD bClothBaseVelClamp;
	DISHONORED_SHIM_STATIC BITFIELD bClothBaseVelInterp;
	DISHONORED_SHIM_STATIC BITFIELD bAttachClothVertsToBaseBody;
	DISHONORED_SHIM_STATIC BITFIELD bIsClothOnStaticObject;
	DISHONORED_SHIM_STATIC BITFIELD bUpdatedFixedClothVerts;
	DISHONORED_SHIM_STATIC BITFIELD bClothPositionalDampening;
	DISHONORED_SHIM_STATIC BITFIELD bClothWindRelativeToOwner;
	DISHONORED_SHIM_STATIC FVector FrozenLocalToWorldPos;
	DISHONORED_SHIM_STATIC FRotator FrozenLocalToWorldRot;
	DISHONORED_SHIM_STATIC FVector ClothExternalForce;
	DISHONORED_SHIM_STATIC FVector ClothWind;
	DISHONORED_SHIM_STATIC FVector ClothBaseVelClampRange;
	DISHONORED_SHIM_STATIC FLOAT ClothBlendWeight;
	DISHONORED_SHIM_STATIC FLOAT ClothDynamicBlendWeight;
	DISHONORED_SHIM_STATIC FLOAT ClothBlendMinDistanceFactor;
	DISHONORED_SHIM_STATIC FLOAT ClothBlendMaxDistanceFactor;
	DISHONORED_SHIM_STATIC FVector MinPosDampRange;
	DISHONORED_SHIM_STATIC FVector MaxPosDampRange;
	DISHONORED_SHIM_STATIC FVector MinPosDampScale;
	DISHONORED_SHIM_STATIC FVector MaxPosDampScale;
	DISHONORED_SHIM_STATIC FPointer ClothSim;
	DISHONORED_SHIM_STATIC INT SceneIndex;
	DISHONORED_SHIM_STATIC TArray<FVector> ClothMeshPosData;
	DISHONORED_SHIM_STATIC TArray<FVector> ClothMeshNormalData;
	DISHONORED_SHIM_STATIC TArray<INT> ClothMeshIndexData;
	DISHONORED_SHIM_STATIC INT NumClothMeshVerts;
	DISHONORED_SHIM_STATIC INT NumClothMeshIndices;
	DISHONORED_SHIM_STATIC TArray<INT> ClothMeshParentData;
	DISHONORED_SHIM_STATIC INT NumClothMeshParentIndices;
	DISHONORED_SHIM_STATIC TArray<FVector> ClothMeshWeldedPosData;
	DISHONORED_SHIM_STATIC TArray<FVector> ClothMeshWeldedNormalData;
	DISHONORED_SHIM_STATIC TArray<INT> ClothMeshWeldedIndexData;
	DISHONORED_SHIM_STATIC INT ClothDirtyBufferFlag;
	DISHONORED_SHIM_STATIC BYTE ClothRBChannel;
	DISHONORED_SHIM_STATIC FRBCollisionChannelContainer ClothRBCollideWithChannels;
	DISHONORED_SHIM_STATIC FLOAT ClothForceScale;
	DISHONORED_SHIM_STATIC FLOAT ClothImpulseScale;
	DISHONORED_SHIM_STATIC FLOAT ClothAttachmentTearFactor;
	DISHONORED_SHIM_STATIC BITFIELD bClothUseCompartment;
	DISHONORED_SHIM_STATIC FLOAT MinDistanceForClothReset;
	DISHONORED_SHIM_STATIC FVector LastClothLocation;
	DISHONORED_SHIM_STATIC BYTE ApexClothingRBChannel;
	DISHONORED_SHIM_STATIC FRBCollisionChannelContainer ApexClothingRBCollideWithChannels;
	DISHONORED_SHIM_STATIC BYTE ApexClothingCollisionRBChannel;
	DISHONORED_SHIM_STATIC BITFIELD bAutoFreezeApexClothingWhenNotRendered;
	DISHONORED_SHIM_STATIC BITFIELD bLocalSpaceWind;
	DISHONORED_SHIM_STATIC FVector WindVelocity;
	DISHONORED_SHIM_STATIC FLOAT WindVelocityBlendTime;
	DISHONORED_SHIM_STATIC BITFIELD bSkipInitClothing;
	DISHONORED_SHIM_STATIC FPointer SoftBodySim;
	DISHONORED_SHIM_STATIC INT SoftBodySceneIndex;
	DISHONORED_SHIM_STATIC BITFIELD bEnableSoftBodySimulation;
	DISHONORED_SHIM_STATIC TArray<FVector> SoftBodyTetraPosData;
	DISHONORED_SHIM_STATIC TArray<INT> SoftBodyTetraIndexData;
	DISHONORED_SHIM_STATIC INT NumSoftBodyTetraVerts;
	DISHONORED_SHIM_STATIC INT NumSoftBodyTetraIndices;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyImpulseScale;
	DISHONORED_SHIM_STATIC BITFIELD bSoftBodyFrozen;
	DISHONORED_SHIM_STATIC BITFIELD bAutoFreezeSoftBodyWhenNotRendered;
	DISHONORED_SHIM_STATIC BITFIELD bSoftBodyAwakeOnStartup;
	DISHONORED_SHIM_STATIC BITFIELD bSoftBodyUseCompartment;
	DISHONORED_SHIM_STATIC ERBCollisionChannel SoftBodyRBChannel;
	DISHONORED_SHIM_STATIC FRBCollisionChannelContainer SoftBodyRBCollideWithChannels;
	DISHONORED_SHIM_STATIC FPointer SoftBodyASVPlane;
	DISHONORED_SHIM_STATIC BYTE AnimRotationOnly;
	DISHONORED_SHIM_STATIC UAudioComponent* CachedFaceFXAudioComp;
	DISHONORED_SHIM_STATIC TArrayNoInit <BYTE> BoneVisibilityStates;
	DISHONORED_SHIM_STATIC TArrayNoInit <FName> MorphTargetsQueried;
	DISHONORED_SHIM_STATIC BITFIELD bUseTickOptimization;
	DISHONORED_SHIM_STATIC INT TickCount;
	DISHONORED_SHIM_STATIC INT LastDropRate;
	DISHONORED_SHIM_STATIC FLOAT LastDropRateChange;
	DISHONORED_SHIM_STATIC FLOAT AccumulatedDroppedDeltaTime;
	DISHONORED_SHIM_STATIC FLOAT ComponentDroppedDeltaTime;





















	// Editor/debugging rendering mode flags.






#if WITH_EDITORONLY_DATA
#endif































	

























	// CLOTH















	


































	/** Align the following byte */
	SCRIPT_ALIGN;


	/** Align the following bitfields */
	SCRIPT_ALIGN;


















	/** Align the following byte */
	SCRIPT_ALIGN;





		





#if WITH_FACEFX
#else
#endif







	/*-----------------------------------------------------------------------------
		Tick time optimization.
	  -----------------------------------------------------------------------------*/







	// USkeletalMeshComponent interface
	void DeleteAnimTree();

	// FaceFX functions.
	void	UpdateFaceFX( TArray<FBoneAtom>& LocalTransforms, UBOOL bTickFaceFX );
	UBOOL	PlayFaceFXAnim(UFaceFXAnimSet* FaceFXAnimSetRef, const FString& AnimName, const FString& GroupName,class USoundCue* SoundCueToPlay);
	void	StopFaceFXAnim( void );
	UBOOL	IsPlayingFaceFXAnim();
	void	DeclareFaceFXRegister( const FString& RegName );
	FLOAT	GetFaceFXRegister( const FString& RegName );
	void	SetFaceFXRegister( const FString& RegName, FLOAT RegVal, BYTE RegOp, FLOAT InterpDuration );
	void	SetFaceFXRegisterEx( const FString& RegName, BYTE RegOp, FLOAT FirstValue, FLOAT FirstInterpDuration, FLOAT NextValue, FLOAT NextInterpDuration );

	/** Update the PredictedLODLevel and MaxDistanceFactor in the component from its MeshObject. */
	UBOOL UpdateLODStatus();
	/** Initialize the LOD entries for the component */
	void InitLODInfos();

	void UpdateSkelPose( FLOAT DeltaTime = 0.f, UBOOL bTickFaceFX = TRUE );
	void UpdateMorph( FLOAT DeltaTime = 0.f, UBOOL bTickFaceFX = TRUE );
	void ProcessRootMotion( FLOAT DeltaTime, FBoneAtom& ExtractedRootMotionDelta, INT& bHasRootMotion );
	void ComposeSkeleton();
	void ApplyControllersForBoneIndex(INT BoneIndex, UBOOL bPrePhysControls, UBOOL bPostPhysControls, const UAnimTree* Tree, UBOOL bRenderedRecently, const BYTE* BoneProcessed);
	void UpdateActiveMorphs();
	/** Builds required bones for 3 pass Compose Skeleton. */
	void BuildComposeSkeletonPasses();
	void BlendPhysicsBones( TArray<BYTE>& RequiredBones, FLOAT PhysicsWeight );
	void BlendInPhysics();
	UBOOL DoesBlendPhysics();

	/** Update bEnableFullAnimWeightBodies Flag - it does not turn off if already on **/
	void UpdateFullAnimWeightBodiesFlag();

	void RecalcRequiredBones(INT LODIndex);
	void RebuildVisibilityArray();

	void SetSkeletalMesh(USkeletalMesh* InSkelMesh, UBOOL bKeepSpaceBases = FALSE);
	void SetPhysicsAsset(UPhysicsAsset* InPhysicsAsset, UBOOL bForceReInit = FALSE);
	void SetForceRefPose(UBOOL bNewForceRefPose);
	virtual void SetMaterial(INT ElementIndex, UMaterialInterface* InMaterial);
	void SetParentAnimComponent(USkeletalMeshComponent* NewParentAnimComp);
	
	/**
	 *	Sets the value of the bForceWireframe flag and reattaches the component as necessary.
	 *
	 *	@param	InForceWireframe		New value of bForceWireframe.
	 */
	void SetForceWireframe(UBOOL InForceWireframe);

	/**
	 *	Set value of bHasPhysicsAssetInstance flag.
	 *	Will create/destroy PhysicsAssetInstance as desired.
	 *
	 *  @param bHasInstance - Sets value of flag
	 *  @param bUseCurrentPosition - If true, skip the skeletal update and use current positions
	 */
	void SetHasPhysicsAssetInstance(UBOOL bHasInstance, UBOOL bUseCurrentPosition = FALSE);

	/** Find a BodyInstance by BoneName */
	URB_BodyInstance* FindBodyInstanceNamed(FName BoneName);

	// Search through AnimSets to find an animation with the given name
	class UAnimSequence* FindAnimSequence(FName AnimSeqName) const;

	// Search through MorphSets to find a morph target with the given name
	class UMorphTarget* FindMorphTarget(FName MorphTargetName);
	class UMorphNodeBase* FindMorphNode(FName InNodeName);

	FMatrix	GetBoneMatrix(DWORD BoneIdx) const;

	/**
	 * returns the bone atom for the bone at the specified index
	 * @param BoneIdx - index of the bone you want an atom for
	 */
	FBoneAtom GetBoneAtom(DWORD BoneIdx) const;

	// Controller Interface
	INT	MatchRefBone(FName StartBoneName) const;
	FName GetParentBone( FName BoneName ) const;

	/** 
	 * Returns bone name linked to a given named socket on the skeletal mesh component.
	 * If you're unsure to deal with sockets or bones names, you can use this function to filter through, and always return the bone name.
	 * @input	bone name or socket name
	 * @output	bone name
	 */
	FName GetSocketBoneName(FName InSocketName);

	FQuat	GetBoneQuaternion(FName BoneName, INT Space=0) const;
	FVector GetBoneLocation(FName BoneName, INT Space=0) const;
	FVector GetBoneAxis( FName BoneName, BYTE Axis ) const;

	void TransformToBoneSpace(FName BoneName, const FVector & InPosition, const FRotator & InRotation, FVector & OutPosition, FRotator & OutRotation);
	void TransformFromBoneSpace(FName BoneName, const FVector & InPosition, const FRotator & InRotation, FVector & OutPosition, FRotator & OutRotation);

	FMatrix GetAttachmentLocalToWorld(const FAttachment& Attachment);

	FMatrix GetTransformMatrix();
	
	virtual void InitArticulated(UBOOL bFixed);
	virtual void TermArticulated(FRBPhysScene* Scene);


	void SetAnimTreeTemplate(UAnimTree* NewTemplate);

	void UpdateParentBoneMap();

	/** Update bHasValidBodies flag */
	void UpdateHasValidBodies();

	/** forces an update to the mesh's skeleton/attachments, even if bUpdateSkelWhenNotRendered is false and it has not been recently rendered
	* @note if bUpdateSkelWhenNotRendered is true, there is no reason to call this function (but doing so anyway will have no effect)
	*/
	void ForceSkelUpdate();

	/** 
	 * Force AnimTree to recache all animations.
	 * Call this when the AnimSets array has been changed.
	 */
	void UpdateAnimations();
	UBOOL GetBonesWithinRadius( const FVector& Origin, FLOAT Radius, DWORD TraceFlags, TArray<FName>& out_Bones );

	// SkelControls.

	FBoneAtom CalcComponentToFrameMatrix( INT BoneIndex, BYTE Space, FName OtherBoneName );
	void CalcBothComponentFrameMatrix(const INT BoneIndex, const BYTE Space, const FName OtherBoneName, FBoneAtom &ComponentToFrame, FBoneAtom& FrameToComponent) const;

	void InitAnimTree(UBOOL bForceReInit=TRUE);
	void InitSkelControls();
	/**
	*	Initialize MorphSets look up table : MorphTargetIndexMap
	*/
	void InitMorphTargets();
	void UpdateMorphTargetMaterial(const UMorphTarget* MorphTarget, const FLOAT Weight);

	/** 
	* Add Curve Keys to ActiveMorph Sets 
	*/
	void ApplyCurveKeys(FCurveKeyArray& CurveKeys);

	class UAnimNode*		FindAnimNode(FName InNodeName);
	class USkelControlBase* FindSkelControl(FName InControlName);
	void TickSkelControls(FLOAT DeltaSeconds);
	virtual UBOOL LegLineCheck(const FVector& Start, const FVector& End, FVector& HitLocation, FVector& HitNormal, const FVector& Extent = FVector(0.f));

	// Attachment interface.
	void AttachComponent(UActorComponent* Component,FName BoneName,FVector RelativeLocation = FVector(0,0,0),FRotator RelativeRotation = FRotator(0,0,0),FVector RelativeScale = FVector(1,1,1));
	void DetachComponent(UActorComponent* Component);

	UBOOL GetSocketWorldLocationAndRotation(FName InSocketName, FVector& OutLocation, FRotator* OutRotation, INT Space=0);
	void AttachComponentToSocket(UActorComponent* Component,FName SocketName);

	/**
	 * Detach any component that's attached if class of the component == ClassOfComponentToDetach or child
	 */
	void DetachAnyOf(UClass * ClassOfComponentToDetach);

    /**
     * Function returns whether or not CPU skinning should be applied
     * Allows the editor to override the skinning state for editor tools
     */
	virtual UBOOL ShouldCPUSkin();

    /** 
     * Function to operate on mesh object after its created, 
     * but before it's attached.
     * @param MeshObject - Mesh Object owned by this component
	 */
	virtual void PostInitMeshObject(class FSkeletalMeshObject* MeshObject) {}

#if USE_GAMEPLAY_PROFILER
    /** 
     * This function actually does the work for the GetProfilerAssetObject and is virtual.  
     * It should only be called from GetProfilerAssetObject as GetProfilerAssetObject is safe to call on NULL object pointers
     **/
	virtual UObject* GetProfilerAssetObjectInternal() const;
#endif

	/**
	 * This will return detail info about this specific object. (e.g. AudioComponent will return the name of the cue,
	 * ParticleSystemComponent will return the name of the ParticleSystem)  The idea here is that in many places
	 * you have a component of interest but what you really want is some characteristic that you can use to track
	 * down where it came from.  
	 *
	 */
	virtual FString GetDetailedInfoInternal() const;

	/** if bOverrideAttachmentOwnerVisibility is true, overrides the owner visibility values in the specified attachment with our own
	 * @param Component the attached primitive whose settings to override
	 */
	void SetAttachmentOwnerVisibility(UActorComponent* Component);

	/** finds the closest bone to the given location
	 * @param TestLocation the location to test against
	 * @param BoneLocation (optional, out) if specified, set to the world space location of the bone that was found, or (0,0,0) if no bone was found
	 * @param IgnoreScale (optional) if specified, only bones with scaling larger than the specified factor are considered
	 * @return the name of the bone that was found, or 'None' if no bone was found
	 */
	FName FindClosestBone(FVector TestLocation, FVector* BoneLocation = NULL, FLOAT IgnoreScale = -1.0f);

	/** Calculate the up-to-date transform of the supplied SkeletalMeshComponent, which should be attached to this component. */
	FMatrix CalcAttachedSkelCompMatrix(const USkeletalMeshComponent* AttachedComp);

	/** 
	 * Update the instanced vertex influences (weights/bones) 
	 * Uses the cached list of bones to find vertices that need to use instanced influences
	 * instead of the defaults from the skeletal mesh 
     * @param LODIdx - The LOD to update the weights for
	 */
	void UpdateInstanceVertexWeights(INT LODIdx);

	/** 
	 * Add a new bone to the list of instance vertex weight bones
	 *
	 * @param BoneNames - set of bones (implicitly parented) to use for finding vertices
	 */
	void AddInstanceVertexWeightBoneParented(FName BoneName, UBOOL bPairWithParent = TRUE);

	/** 
	 * Remove a new bone to the list of instance vertex weight bones
	 *
	 * @param BoneNames - set of bones (implicitly parented) to use for finding vertices
	 */
	void RemoveInstanceVertexWeightBoneParented(FName BoneName);

	/** 
	 * Find an existing bone pair entry in the list of InstanceVertexWeightBones
	 *
	 * @param BonePair - pair of bones to search for
	 * @return index of entry found or -1 if not found
	 */
	INT FindInstanceVertexweightBonePair(const FBonePair& BonePair) const;
	
	/** 
	 * Update the bones that specify which vertices will use instanced influences
	 * This will also trigger an update of the vertex weights.
	 *
	 * @param BonePairs - set of bone pairs to use for finding vertices
	 */
	void UpdateInstanceVertexWeightBones( const TArray<FBonePair>& BonePairs );
	
	/**
	 * Enabled or disable the instanced vertex weights buffer for the skeletal mesh object
	 *
	 * @param bEnable - TRUE to enable, FALSE to disable
	 * @param LODIdx - LOD to enable
	 */
	void ToggleInstanceVertexWeights( UBOOL bEnabled, INT LODIdx);

	/**
	 * Verify FaceFX Bone indices match SkeletalMesh bone indices
	 */
	void DebugVerifyFaceFXBoneList();

	/**
	 * Debug function that traces FaceFX bone list
	 * to see if more than one mesh is referencing/relinking master bone list
	 */
	void TraceFaceFX(UBOOL bOutput = FALSE);

	/**
	 * Checks/updates material usage on proxy based on current morph target usage
	 */
	void UpdateMorphMaterialUsageOnProxy();

	// UObject interface - to count memory
	virtual void Serialize(FArchive& Ar);
	virtual void PostLoad();

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	INT GetResourceSize();

	/**
	* Returns the FIApexClothing interface pointer.
	**/
	FIApexClothing * GetApexClothing(void) const { return ApexClothing; };
	/** 
	* Returns the number of clothing vertices
	**/
	INT GetApexClothingNumVertices(int LodIndex, int SectionIndex);
	/**
	*  Initialize Apex clothing, if this skeletal mesh has any materials marked as being used with clothing.
	*/
	void           InitApexClothing(FRBPhysScene* RBPhysScene);

	/**
	* Releases the Apex clothing interface if one was created.
	*/
	void           ReleaseApexClothing();

	/**
	* 'Ticks' the associated Apex clothing if there is one.  
	* Primarily detects if the underlying asset has changed and re-creates the clothing interface if needed.
    * Give up a time slice to synchronize apex clothing
	*/
	void           TickApexClothing(FLOAT DeltaTime);

	/**
	* Forward the MaxDistance Scale Notifications from Animations to Apex Clothing.
	*/
	void           SetApexClothingMaxDistanceScale(FLOAT StartScale, FLOAT EndScale, INT ScaleMode, FLOAT Duration);

	/**
	 *  Retrieve various actor metrics depending on the provided type.  All of
	 *  these will total the values for this component.
	 *
	 *  @param MetricsType The type of metric to calculate.
	 *
	 *  METRICS_VERTS    - Get the number of vertices.
	 *  METRICS_TRIS     - Get the number of triangles.
	 *  METRICS_SECTIONS - Get the number of sections.
	 *
	 *  @return INT The total of the given type for this component.
	 */
	virtual INT GetActorMetrics(EActorMetricsType MetricsType);

	// UActorComponent interface.
protected:
	virtual void SetParentToWorld(const FMatrix& ParentToWorld);
	virtual void Attach();
	virtual void UpdateTransform();
	virtual void UpdateChildComponents();
	virtual void Detach( UBOOL bWillReattach = FALSE );
	virtual void BeginPlay();
	virtual void Tick(FLOAT DeltaTime);

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);	

	/**
	 * Called by the editor to query whether a property of this object is allowed to be modified.
	 * The property editor uses this to disable controls for properties that should not be changed.
	 * When overriding this function you should always call the parent implementation first.
	 *
	 * @param	InProperty	The property to query
	 *
	 * @return	TRUE if the property can be modified in the editor, otherwise FALSE
	 */
	virtual UBOOL CanEditChange( const UProperty* InProperty ) const;

	/**
	 * Called to finish destroying the object.  After UObject::FinishDestroy is called, the object's memory should no longer be accessed.
	 */
	virtual void FinishDestroy();

	/** Internal function that updates physics objects to match the RBChannel/RBCollidesWithChannel info. */
	virtual void UpdatePhysicsToRBChannels();
public:
	void TickAnimNodes(FLOAT DeltaTime);

	virtual void InitComponentRBPhys(UBOOL bFixed);
	virtual void SetComponentRBFixed(UBOOL bFixed);
	virtual void TermComponentRBPhys(FRBPhysScene* InScene);
	virtual class URB_BodySetup* GetRBBodySetup();

	/** 
	 * Retrieves the materials used in this component 
	 * 
	 * @param OutMaterials	The list of used materials.
	 */
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials) const;

	/** Initialize physx cloth support. */
	void InitClothSim(FRBPhysScene* Scene);

	/** Destructs and frees cloth support. */
	void TermClothSim(FRBPhysScene* Scene);

	/** Initialize breakable cloth vertices. */
	void InitClothBreakableAttachments();

	/** Initialize "metal" cloth support... stiff cloth that is associated with rigid-bodies to simulate denting. */
	void InitClothMetal();

	/** Initialize physx soft-body support. */
	void InitSoftBodySim(FRBPhysScene* Scene, UBOOL bRunsInAnimSetViewer = false);

	/** Destructs and frees soft-body support. */
	void TermSoftBodySim(FRBPhysScene* Scene);

	/** Updates soft-body parameters from the UActor to the simulation. */
	void UpdateSoftBodyParams();

	/** Initialize soft-body to rigid-body attach points. */
	void InitSoftBodyAttachments();

	/** Freeze or unfreeze soft-body simulation. */
	void SetSoftBodyFrozen(UBOOL bNewFrozen);

	/** Force awake any soft body simulation on this component */
	void WakeSoftBody();

	/** Inialize internal soft-body memory buffers. */
	void InitSoftBodySimBuffers();

	virtual UBOOL IsValidComponent() const;

	// UPrimitiveComponent interface.
	virtual void SetTransformedToWorld();

	virtual void GetStreamingTextureInfo(TArray<FStreamingTexturePrimitiveInfo>& OutStreamingTextures) const;

	virtual UBOOL PointCheck(FCheckResult& Result,const FVector& Location,const FVector& Extent,DWORD TraceFlags);
	virtual UBOOL LineCheck(FCheckResult& Result,const FVector& End,const FVector& Start,const FVector& Extent,DWORD TraceFlags);

	virtual void UpdateBounds();
	void UpdateClothBounds();
	void UpdateApexClothingBounds();

	virtual void AddImpulse(FVector Impulse, FVector Position = FVector(0,0,0), FName BoneName = NAME_None, UBOOL bVelChange=false);
	virtual void AddRadialImpulse(const FVector& Origin, FLOAT Radius, FLOAT Strength, BYTE Falloff, UBOOL bVelChange=false);
	virtual void AddRadialForce(const FVector& Origin, FLOAT Radius, FLOAT Strength, BYTE Falloff);
	virtual void AddForceField(FForceApplicator* Applicator, const FBox& FieldBoundingBox, UBOOL bApplyToCloth, UBOOL bApplyToRigidBody);
	virtual void WakeRigidBody( FName BoneName = NAME_None );
	virtual void PutRigidBodyToSleep( FName BoneName = NAME_None );
	virtual UBOOL RigidBodyIsAwake( FName BoneName = NAME_None );
	virtual void SetRBLinearVelocity(const FVector& NewVel, UBOOL bAddToCurrent=false);
	virtual void SetRBAngularVelocity(const FVector& NewVel, UBOOL bAddToCurrent=false);
	virtual void RetardRBLinearVelocity(const FVector& RetardDir, FLOAT VelScale);
	virtual void SetRBPosition(const FVector& NewPos, FName BoneName = NAME_None);
	virtual void SetRBRotation(const FRotator& NewRot, FName BoneName = NAME_None);
	virtual void SetBlockRigidBody(UBOOL bNewBlockRigidBody);
	virtual void SetNotifyRigidBodyCollision(UBOOL bNewNotifyRigidBodyCollision);
	virtual void SetPhysMaterialOverride(UPhysicalMaterial* NewPhysMaterial);
	virtual URB_BodyInstance* GetRootBodyInstance();

	/** 
	 *	Used for creating one-way physics interactions.
	 *	@see RBDominanceGroup
	 */
	virtual void SetRBDominanceGroup(BYTE InDomGroup);

	/** Utility for calculating the current LocalToWorld matrix of this SkelMeshComp, given its parent transform. */
	FMatrix CalcCurrentLocalToWorld(const FMatrix& ParentMatrix);

	/** Simple, CPU evaluation of a vertex's skinned position (returned in component space) */
	FVector GetSkinnedVertexPosition(INT VertexIndex) const;

	void UpdateRBBonesFromSpaceBases(const FMatrix& CompLocalToWorld, UBOOL bMoveUnfixedBodies, UBOOL bTeleport);
	void UpdateRBJointMotors();
	void UpdateFixedClothVerts();

	/** Update forces applied to each cloth particle based on the ClothWind parameter. */
	void UpdateClothWindForces(FLOAT DeltaSeconds);

	/** Move all vertices in the cloth to the reference pose and zero their velocity. */
	void ResetClothVertsToRefPose();

	/** Forces apex clothing to use 'teleport and reset' for the next update */
	void ForceApexClothingTeleportAndReset();
	/** Forces apex clothing to use 'teleport' for the next update */
	void ForceApexClothingTeleport();

	/**
	* Looks up all bodies for broken constraints.
	* Makes sure child bodies of a broken constraints are not fixed and using bone springs, and child joints not motorized.
	*/
	void UpdateMeshForBrokenConstraints();

	/** 
	 *  Show/Hide Material - technical correct name for this is Section, but seems Material is mostly used
	 *  This disable rendering of certain Material ID (Section)
	 *
	 * @param MaterialID - id of the material to match a section on and to show/hide
	 * @param bShow - TRUE to show the section, otherwise hide it
	 * @param LODIndex - index of the lod entry since material mapping is unique to each LOD
	 */
	void ShowMaterialSection(INT MaterialID, UBOOL bShow, INT LODIndex);

	/** Enables or disables Gore Mesh Mode 
	 *  Disable only works in editor
	 */
	void EnableAltBoneWeighting(UBOOL bEnable, INT LOD=0);

	//Some get*() APIs
	FLOAT GetClothAttachmentResponseCoefficient();
	FLOAT GetClothAttachmentTearFactor();
	FLOAT GetClothBendingStiffness();
	FLOAT GetClothCollisionResponseCoefficient();
	FLOAT GetClothDampingCoefficient();
	INT GetClothFlags();
	FLOAT GetClothFriction();
	FLOAT GetClothPressure();
	FLOAT GetClothSleepLinearVelocity();
	INT GetClothSolverIterations();
	FLOAT GetClothStretchingStiffness();
	FLOAT GetClothTearFactor();
	FLOAT GetClothThickness();
	//some set*() APIs
	void SetClothAttachmentResponseCoefficient(FLOAT ClothAttachmentResponseCoefficient);
	void SetClothAttachmentTearFactor(FLOAT ClothAttachmentTearFactor);
	void SetClothBendingStiffness(FLOAT ClothBendingStiffness);
	void SetClothCollisionResponseCoefficient(FLOAT ClothCollisionResponseCoefficient);
	void SetClothDampingCoefficient(FLOAT ClothDampingCoefficient);
	void SetClothFlags(INT ClothFlags);
	void SetClothFriction(FLOAT ClothFriction);
	void SetClothPressure(FLOAT ClothPressure);
	void SetClothSleepLinearVelocity(FLOAT ClothSleepLinearVelocity);
	void SetClothSolverIterations(INT ClothSolverIterations);
	void SetClothStretchingStiffness(FLOAT ClothStretchingStiffness);
	void SetClothTearFactor(FLOAT ClothTearFactor);
	void SetClothThickness(FLOAT ClothThickness);
	//Other APIs
	void SetClothSleep(UBOOL IfClothSleep);
	void SetClothPosition(const FVector& ClothOffSet);
	void SetClothVelocity(const FVector& VelocityOffSet);
	//Attachment API
	void AttachClothToCollidingShapes(UBOOL AttatchTwoWay, UBOOL AttachTearable);
	//ValidBounds APIs
	void EnableClothValidBounds(UBOOL IfEnableClothValidBounds);
	void SetClothValidBounds(const FVector& ClothValidBoundsMin, const FVector& ClothValidBoundsMax);

	virtual void UpdateRBKinematicData();
	void SetEnableClothSimulation(UBOOL bInEnable);

	/** Toggle active simulation of cloth. Cheaper than doing SetEnableClothSimulation, and keeps its shape while frozen. */
	void SetClothFrozen(UBOOL bNewFrozen);

	/** Toggle active simulation of clothing and keeps its shape while frozen. */
	void SetEnableClothingSimulation(UBOOL bInEnable);

	void UpdateClothParams();
	void SetClothExternalForce(const FVector& InForce);

	/** Attach/detach verts from physics body that this components actor is attached to. */
	void SetAttachClothVertsToBaseBody(UBOOL bAttachVerts);

	/**
	 * Saves the skeletal component's current AnimSets to a temporary buffer.  You can restore them later by calling
	 * RestoreSavedAnimSets().  This is the C++ version of the method.  The script version just calls this one.
	 */
	void SaveAnimSets();

	/**
	 * Restores saved AnimSets to the master list of AnimSets and clears the temporary saved list of AnimSets.  This
	 * is the C++ version of the method.  The script version just calls this one.
	 */
	void RestoreSavedAnimSets();

	virtual void GenerateDecalRenderData(class FDecalState* Decal, TArray< FDecalRenderData* >& OutDecalRenderDatas) const;

	/**
	 * Transforms the specified decal info into reference pose space.
	 *
	 * @param	Decal			Info of decal to transform.
	 * @param	BoneIndex		The index of the bone hit by the decal.
	 * @return					A reference to the transformed decal info, or NULL if the operation failed.
	 */
	FDecalState* TransformDecalToRefPoseSpace(FDecalState* Decal, INT BoneIndex) const;

	/** 
	* @return TRUE if the primitive component can render decals
	*/
	virtual UBOOL SupportsDecalRendering() const;

	virtual FPrimitiveSceneProxy* CreateSceneProxy();	

#if WITH_NOVODEX
	virtual class NxActor* GetNxActor(FName BoneName = NAME_None);
	virtual class NxActor* GetIndexedNxActor(INT BodyIndex = INDEX_NONE);

	/** Utility for getting all physics bodies contained within this component. */
	virtual void GetAllNxActors(TArray<class NxActor*>& OutActors);

	virtual FVector NxGetPointVelocity(FVector LocationInWorldSpace);
#endif // WITH_NOVODEX

	void HideBone( INT BoneIndex, EPhysBodyOp PhysBodyOption );
	void UnHideBone( INT BoneIndex );
	UBOOL IsBoneHidden( INT BoneIndex );

	void HideBoneByName( FName BoneName, EPhysBodyOp PhysBodyOption);
	void UnHideBoneByName( FName BoneName );

	virtual FKCachedConvexData* GetBoneCachedPhysConvexData(const FVector& InScale3D, const FName& BoneName);

	// UMeshComponent interface.

	virtual UMaterialInterface* GetMaterial(INT MaterialIndex) const;
	virtual INT GetNumElements() const;

	/**
	 * Called by AnimNotify_PlayParticleEffect
	 * Looks for a socket name first then bone name
 	 *
	 * @param AnimNotifyData The AnimNotify_PlayParticleEffect which will have all of the various params on it
	 */
	 UBOOL eventPlayParticleEffect(const class UAnimNotify_PlayParticleEffect* AnimNotifyData)
	 {
	 	 Actor_eventPlayParticleEffect_Parms Parms(EC_EventParm);
	 	 Parms.AnimNotifyData=AnimNotifyData;
	 	 ProcessEvent(FindFunctionChecked(ENGINE_PlayParticleEffect),&Parms);
		return Parms.ReturnValue;
	 }

	 UBOOL eventCreateForceField(const class UAnimNotify_ForceField* AnimNotifyData)
	 {
		 Actor_eventCreateForceField_Parms Parms(EC_EventParm);
		 Parms.ReturnValue=FALSE;
		 Parms.AnimNotifyData=AnimNotifyData;
		 ProcessEvent(FindFunctionChecked(ENGINE_CreateForceField),&Parms);
		 return Parms.ReturnValue;
	 }

	UBOOL ExtractRootMotionCurve( FName AnimName, FLOAT SampleRate, FRootMotionCurve& out_RootMotionInterpCurve );

	// Script functions.
	DECLARE_FUNCTION(execSetMaterial);
	DECLARE_FUNCTION(execAttachComponent);
	DECLARE_FUNCTION(execDetachComponent);
	DECLARE_FUNCTION(execAttachComponentToSocket);
	DECLARE_FUNCTION(execGetSocketWorldLocationAndRotation);
	DECLARE_FUNCTION(execGetSocketByName);
	DECLARE_FUNCTION(execGetSocketBoneName);
	DECLARE_FUNCTION(execFindComponentAttachedToBone);
	DECLARE_FUNCTION(execIsComponentAttached);
	DECLARE_FUNCTION(execAttachedComponents);
	DECLARE_FUNCTION(execGetTransformMatrix);
	DECLARE_FUNCTION(execSetSkeletalMesh);
	DECLARE_FUNCTION(execSetPhysicsAsset);
	DECLARE_FUNCTION(execSetForceRefPose);
	DECLARE_FUNCTION(execSetParentAnimComponent);
	DECLARE_FUNCTION(execFindAnimSequence);
	DECLARE_FUNCTION(execFindMorphTarget);
	DECLARE_FUNCTION(execGetBoneQuaternion);
	DECLARE_FUNCTION(execGetBoneLocation);
	DECLARE_FUNCTION(execGetBoneAxis);
	DECLARE_FUNCTION(execTransformToBoneSpace);
	DECLARE_FUNCTION(execTransformFromBoneSpace);
	DECLARE_FUNCTION(execFindClosestBone);
	DECLARE_FUNCTION(execGetClosestCollidingBoneLocation);
	DECLARE_FUNCTION(execSetAnimTreeTemplate);
	DECLARE_FUNCTION(execUpdateParentBoneMap);
	DECLARE_FUNCTION(execInitSkelControls);
	DECLARE_FUNCTION(execInitMorphTargets);
	DECLARE_FUNCTION(execFindAnimNode);
	DECLARE_FUNCTION(execAllAnimNodes);
	DECLARE_FUNCTION(execFindSkelControl);
	DECLARE_FUNCTION(execFindMorphNode);
	DECLARE_FUNCTION(execFindConstraintIndex);
	DECLARE_FUNCTION(execFindConstraintBoneName);
	DECLARE_FUNCTION(execFindBodyInstanceNamed);
	DECLARE_FUNCTION(execForceSkelUpdate);
	DECLARE_FUNCTION(execUpdateAnimations);
	DECLARE_FUNCTION(execGetBonesWithinRadius);
	DECLARE_FUNCTION(execAddInstanceVertexWeightBoneParented);
	DECLARE_FUNCTION(execRemoveInstanceVertexWeightBoneParented);
	DECLARE_FUNCTION(execFindInstanceVertexweightBonePair);
	DECLARE_FUNCTION(execUpdateInstanceVertexWeightBones);
	DECLARE_FUNCTION(execToggleInstanceVertexWeights);
	DECLARE_FUNCTION(execSetHasPhysicsAssetInstance);
	DECLARE_FUNCTION(execUpdateRBBonesFromSpaceBases);
	DECLARE_FUNCTION(execPlayFaceFXAnim);
	DECLARE_FUNCTION(execStopFaceFXAnim);
	DECLARE_FUNCTION(execIsPlayingFaceFXAnim);
	DECLARE_FUNCTION(execDeclareFaceFXRegister);
	DECLARE_FUNCTION(execGetFaceFXRegister);
	DECLARE_FUNCTION(execSetFaceFXRegister);
	DECLARE_FUNCTION(execSetFaceFXRegisterEx);
	DECLARE_FUNCTION(execSetEnableClothingSimulation);
	DECLARE_FUNCTION(execSetEnableClothSimulation);
	DECLARE_FUNCTION(execSetClothFrozen);
	DECLARE_FUNCTION(execUpdateClothParams);
	DECLARE_FUNCTION(execSetClothExternalForce);
	DECLARE_FUNCTION(execSetAttachClothVertsToBaseBody);
	DECLARE_FUNCTION(execResetClothVertsToRefPose);
	DECLARE_FUNCTION(execForceApexClothingTeleportAndReset);
	DECLARE_FUNCTION(execForceApexClothingTeleport);
	DECLARE_FUNCTION(execUpdateMeshForBrokenConstraints);
	DECLARE_FUNCTION(execShowMaterialSection);

	//Some get*() APIs
	DECLARE_FUNCTION(execGetClothAttachmentResponseCoefficient);
	DECLARE_FUNCTION(execGetClothAttachmentTearFactor);
	DECLARE_FUNCTION(execGetClothBendingStiffness);
	DECLARE_FUNCTION(execGetClothCollisionResponseCoefficient);
	DECLARE_FUNCTION(execGetClothDampingCoefficient);
	DECLARE_FUNCTION(execGetClothFlags);
	DECLARE_FUNCTION(execGetClothFriction);
	DECLARE_FUNCTION(execGetClothPressure);
	DECLARE_FUNCTION(execGetClothSleepLinearVelocity);
	DECLARE_FUNCTION(execGetClothSolverIterations);
	DECLARE_FUNCTION(execGetClothStretchingStiffness);
	DECLARE_FUNCTION(execGetClothTearFactor);
	DECLARE_FUNCTION(execGetClothThickness);
	//some set*() APIs
	DECLARE_FUNCTION(execSetClothAttachmentResponseCoefficient);
	DECLARE_FUNCTION(execSetClothAttachmentTearFactor);
	DECLARE_FUNCTION(execSetClothBendingStiffness);
	DECLARE_FUNCTION(execSetClothCollisionResponseCoefficient);
	DECLARE_FUNCTION(execSetClothDampingCoefficient);
	DECLARE_FUNCTION(execSetClothFlags);
	DECLARE_FUNCTION(execSetClothFriction);
	DECLARE_FUNCTION(execSetClothPressure);
	DECLARE_FUNCTION(execSetClothSleepLinearVelocity);
	DECLARE_FUNCTION(execSetClothSolverIterations);
	DECLARE_FUNCTION(execSetClothStretchingStiffness);
	DECLARE_FUNCTION(execSetClothTearFactor);
	DECLARE_FUNCTION(execSetClothThickness);
	//Other APIs
	DECLARE_FUNCTION(execSetClothSleep);
	DECLARE_FUNCTION(execSetClothPosition);
	DECLARE_FUNCTION(execSetClothVelocity);
	//Attachment API
	DECLARE_FUNCTION(execAttachClothToCollidingShapes);
	//ValidBounds APIs
	DECLARE_FUNCTION(execEnableClothValidBounds);
	DECLARE_FUNCTION(execSetClothValidBounds);

	DECLARE_FUNCTION(execSaveAnimSets);
	DECLARE_FUNCTION(execRestoreSavedAnimSets);
	DECLARE_FUNCTION(execGetBoneMatrix);
	DECLARE_FUNCTION(execMatchRefBone);
	DECLARE_FUNCTION(execGetBoneName);
	DECLARE_FUNCTION(execGetParentBone);
	DECLARE_FUNCTION(execGetBoneNames);
	DECLARE_FUNCTION(execBoneIsChildOf);
	DECLARE_FUNCTION(execGetRefPosePosition);

	DECLARE_FUNCTION(execUpdateSoftBodyParams);
	DECLARE_FUNCTION(execSetSoftBodyFrozen);
	DECLARE_FUNCTION(execWakeSoftBody);

	DECLARE_FUNCTION(execHideBone);
	DECLARE_FUNCTION(execUnHideBone);
	DECLARE_FUNCTION(execIsBoneHidden);

	DECLARE_FUNCTION(execHideBoneByName);
	DECLARE_FUNCTION(execUnHideBoneByName);

	DECLARE_FUNCTION(execGetPosition);
	DECLARE_FUNCTION(execGetRotation);
};

class FSkeletalMeshComponentReattachContext
{
public:

	/** Initialization constructor. */
	FSkeletalMeshComponentReattachContext( class USkeletalMesh* SkeletalMesh )
	{
		for( TObjectIterator<USkeletalMeshComponent> It; It; ++It )
		{
			if ( It->SkeletalMesh == SkeletalMesh )
			{
				new(ReattachContexts) FComponentReattachContext( *It );
			}
		}

		// Flush the rendering commands generated by the detachments.
		FlushRenderingCommands();
	}

private:
	TIndirectArray<FComponentReattachContext> ReattachContexts;
};


/*-----------------------------------------------------------------------------
	USkeletalMesh.
-----------------------------------------------------------------------------*/

struct FMeshWedge
{
	DWORD			iVertex;			// Vertex index.
	FVector2D		UVs[MAX_TEXCOORDS];	// UVs.
	FColor			Color;			// Vertex color.
	friend FArchive &operator<<( FArchive& Ar, FMeshWedge& T )
	{
		if (Ar.Ver() < VER_DWORD_SKELETAL_MESH_INDICES)
		{
			WORD LegacyVert;
			Ar << LegacyVert;
			T.iVertex = LegacyVert;
		}
		else
		{
			Ar << T.iVertex;
		}
		
		if( Ar.Ver() < VER_ADDED_MULTIPLE_UVS_TO_SKELETAL_MESH )
		{
			// This package is older, just serialize the first set of UV's
			Ar << T.UVs[0].X << T.UVs[0].Y;
		}
		else
		{
			// This package has multiple UV's so serialize them all
			for( INT UVIdx = 0; UVIdx < MAX_TEXCOORDS; ++UVIdx )
			{
				Ar << T.UVs[UVIdx];
			}
		}

		if( Ar.Ver() < VER_ADDED_SKELETAL_MESH_VERTEX_COLORS )
		{
			// Initialize color to white.
			T.Color = FColor(255,255,255);
		}
		else
		{
			Ar << T.Color;
		}

		return Ar;
	}
};
template <> struct TIsPODType<FMeshWedge> { enum { Value = true }; };

struct FMeshFace
{
	DWORD		iWedge[3];			// Textured Vertex indices.
	WORD		MeshMaterialIndex;	// Source Material (= texture plus unique flags) index.

    FVector	TangentX[3];
    FVector	TangentY[3];
    FVector	TangentZ[3];
    UBOOL   bOverrideTangentBasis;  //override tangents data of unreal
	friend FArchive &operator<<( FArchive& Ar, FMeshFace& F )
	{
		if (Ar.Ver() < VER_DWORD_SKELETAL_MESH_INDICES)
		{
			WORD LegacyVertIdx[3];
			Ar << LegacyVertIdx[0] << LegacyVertIdx[1] << LegacyVertIdx[2];
			F.iWedge[0] = LegacyVertIdx[0];
			F.iWedge[1] = LegacyVertIdx[1];
			F.iWedge[2] = LegacyVertIdx[2];
		}
		else
		{
			Ar << F.iWedge[0] << F.iWedge[1] << F.iWedge[2];
		}
		Ar << F.MeshMaterialIndex;
		Ar << F.TangentX[0] << F.TangentX[1] << F.TangentX[2];
        Ar << F.TangentY[0] << F.TangentY[1] << F.TangentY[2];
        Ar << F.TangentZ[0] << F.TangentZ[1] << F.TangentZ[2];
        Ar << F.bOverrideTangentBasis;
		return Ar;
	}
};
template <> struct TIsPODType<FMeshFace> { enum { Value = true }; };

// A bone: an orientation, and a position, all relative to their parent.
struct VJointPos
{
	FQuat   	Orientation;  //
	FVector		Position;     //  needed or not ?

	FLOAT       Length;       //  For collision testing / debugging drawing...
	FLOAT       XSize;
	FLOAT       YSize;
	FLOAT       ZSize;

	friend FArchive &operator<<( FArchive& Ar, VJointPos& V )
	{
		return Ar << V.Orientation << V.Position;
	}
};
template <> struct TIsPODType<VJointPos> { enum { Value = true }; };

/*
This class is to keep compatibility with ActorX FQuat
*/
struct FQuatNoAlign
{
	// Variables.
	FLOAT X,Y,Z,W;

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FQuatNoAlign& F )
	{
		return Ar << F.X << F.Y << F.Z << F.W;
	}
};

// NoAlign VJointPos - To keep alignment working with ActorX
struct VJointPosNoAlign
{
	FQuatNoAlign   	Orientation;  //
	FVector			Position;     //  needed or not ?

	FLOAT       Length;       //  For collision testing / debugging drawing...
	FLOAT       XSize;
	FLOAT       YSize;
	FLOAT       ZSize;
};

// Reference-skeleton bone, the package-serializable version.
struct FMeshBone
{
	FName 		Name;		  // Bone's name.
	DWORD		Flags;        // reserved
	VJointPos	BonePos;      // reference position
	INT         ParentIndex;  // 0/NULL if this is the root bone.  
	INT 		NumChildren;  // children  // only needed in animation ?
	INT         Depth;        // Number of steps to root in the skeletal hierarcy; root=0.

	// DEBUG rendering
	FColor		BoneColor;		// Color to use when drawing bone on screen.

	UBOOL operator==( const FMeshBone& B ) const
	{
		return( Name == B.Name );
	}
	
	friend FArchive &operator<<( FArchive& Ar, FMeshBone& F)
	{
		Ar << F.Name << F.Flags << F.BonePos << F.NumChildren << F.ParentIndex;

		if( Ar.IsLoading() && Ar.Ver() < VER_SKELMESH_DRAWSKELTREEMANAGER )
		{
			F.BoneColor = FColor(255, 255, 255, 255);
		}
		else
		{
			Ar << F.BoneColor;
		}

		return Ar;
	}
};
template <> struct TIsPODType<FMeshBone> { enum { Value = true }; };

// Textured triangle.
struct VTriangle
{
	DWORD   WedgeIndex[3];	 // Point to three vertices in the vertex list.
	BYTE    MatIndex;	     // Materials can be anything.
	BYTE    AuxMatIndex;     // Second material from exporter (unused)
	DWORD   SmoothingGroups; // 32-bit flag for smoothing groups.

 FVector	TangentX[3];
    FVector	TangentY[3];
    FVector	TangentZ[3];
    UBOOL   bOverrideTangentBasis;  //override tangents data of unreal

	friend FArchive &operator<<( FArchive& Ar, VTriangle& V )
	{
		if (Ar.Ver() < VER_DWORD_SKELETAL_MESH_INDICES)
		{
			WORD LegacyVertIdx[3];
			Ar << LegacyVertIdx[0] << LegacyVertIdx[1] << LegacyVertIdx[2];
			V.WedgeIndex[0] = LegacyVertIdx[0];
			V.WedgeIndex[1] = LegacyVertIdx[1];
			V.WedgeIndex[2] = LegacyVertIdx[2];
		}
		else
		{
			Ar << V.WedgeIndex[0] << V.WedgeIndex[1] << V.WedgeIndex[2];
		}
		Ar << V.MatIndex << V.AuxMatIndex;
		Ar << V.SmoothingGroups;
        Ar << V.TangentX[0] << V.TangentX[1] << V.TangentX[2];
        Ar << V.TangentY[0] << V.TangentY[1] << V.TangentY[2];
        Ar << V.TangentZ[0] << V.TangentZ[1] << V.TangentZ[2];
		Ar << V.bOverrideTangentBasis;
		return Ar;
	}

	VTriangle& operator=( const VTriangle& Other)
	{
		this->AuxMatIndex = Other.AuxMatIndex;
		this->MatIndex        =  Other.MatIndex;
		this->SmoothingGroups =  Other.SmoothingGroups;
		this->WedgeIndex[0]   =  Other.WedgeIndex[0];
		this->WedgeIndex[1]   =  Other.WedgeIndex[1];
		this->WedgeIndex[2]   =  Other.WedgeIndex[2];
        this->TangentX[0]   =  Other.TangentX[0];
        this->TangentX[1]   =  Other.TangentX[1];
        this->TangentX[2]   =  Other.TangentX[2];

        this->TangentY[0]   =  Other.TangentY[0];
        this->TangentY[1]   =  Other.TangentY[1];
        this->TangentY[2]   =  Other.TangentY[2];

        this->TangentZ[0]   =  Other.TangentZ[0];
        this->TangentZ[1]   =  Other.TangentZ[1];
        this->TangentZ[2]   =  Other.TangentZ[2];

        this->bOverrideTangentBasis   =  Other.bOverrideTangentBasis;
		return *this;
	}
};
template <> struct TIsPODType<VTriangle> { enum { Value = true }; };

struct FVertInfluence 
{
	FLOAT Weight;
	DWORD VertIndex;
	WORD BoneIndex;
	friend FArchive &operator<<( FArchive& Ar, FVertInfluence& F )
	{
		if (Ar.Ver() < VER_DWORD_SKELETAL_MESH_INDICES)
		{
			WORD LegacyVertIdx;
			Ar << F.Weight << LegacyVertIdx << F.BoneIndex;
			F.VertIndex = LegacyVertIdx;
		}
		else
		{
			Ar << F.Weight << F.VertIndex << F.BoneIndex;
		}
		return Ar;
	}
};
template <> struct TIsPODType<FVertInfluence> { enum { Value = true }; };

/**
* Data needed for importing an extra set of vertex influences
*/
struct FSkelMeshExtraInfluenceImportData
{
	TArray<FMeshBone> RefSkeleton;
	TArray<FVertInfluence> Influences;
	TArray<FMeshWedge> Wedges;
	TArray<FMeshFace> Faces;
	TArray<FVector> Points;
	EInstanceWeightUsage Usage;
	INT MaxBoneCountPerChunk;
};

//
//	FSoftSkinVertex
//

struct FSoftSkinVertex
{
	FVector			Position;
	FPackedNormal	TangentX,	// Tangent, U-direction
					TangentY,	// Binormal, V-direction
					TangentZ;	// Normal
	FVector2D		UVs[MAX_TEXCOORDS]; // UVs
	FColor			Color;		// VertexColor
	BYTE			InfluenceBones[MAX_INFLUENCES];
	BYTE			InfluenceWeights[MAX_INFLUENCES];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,FSoftSkinVertex& V);
};

//
//	FRigidSkinVertex
//

struct FRigidSkinVertex
{
	FVector			Position;
	FPackedNormal	TangentX,	// Tangent, U-direction
					TangentY,	// Binormal, V-direction
					TangentZ;	// Normal
	FVector2D		UVs[MAX_TEXCOORDS]; // UVs
	FColor			Color;		// Vertex color.
	BYTE			Bone;

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,FRigidSkinVertex& V);
};

/**
 * A set of the skeletal mesh vertices which use the same set of <MAX_GPUSKIN_BONES bones.
 * In practice, there is a 1:1 mapping between chunks and sections, but for meshes which
 * were imported before chunking was implemented, there will be a single chunk for all
 * sections.
 */
struct FSkelMeshChunk
{
	/** The offset into the LOD's vertex buffer of this chunk's vertices. */
	UINT BaseVertexIndex;

	/** The rigid vertices of this chunk. */
	TArray<FRigidSkinVertex> RigidVertices;

	/** The soft vertices of this chunk. */
	TArray<FSoftSkinVertex> SoftVertices;

	/** The bones which are used by the vertices of this chunk. Indices of bones in the USkeletalMesh::RefSkeleton array */
	TArray<WORD> BoneMap;

	/** The number of rigid vertices in this chunk */
	INT NumRigidVertices;
	/** The number of soft vertices in this chunk */
	INT NumSoftVertices;

	/** max # of bones used to skin the vertices in this chunk */
	INT MaxBoneInfluences;

	FSkelMeshChunk()
		: BaseVertexIndex(0)
		, NumRigidVertices(0)
		, NumSoftVertices(0)
		, MaxBoneInfluences(4)
	{}

	FSkelMeshChunk(const FSkelMeshChunk& Other)
	{
		BaseVertexIndex = Other.BaseVertexIndex;
		RigidVertices = Other.RigidVertices;
		SoftVertices = Other.SoftVertices;
		BoneMap = Other.BoneMap;
		NumRigidVertices = Other.NumRigidVertices;
		NumSoftVertices = Other.NumSoftVertices;
		MaxBoneInfluences = Other.MaxBoneInfluences;
	}

	/**
	* @return total num rigid verts for this chunk
	*/
	FORCEINLINE INT GetNumRigidVertices() const
	{
		return NumRigidVertices;
	}

	/**
	* @return total num soft verts for this chunk
	*/
	FORCEINLINE INT GetNumSoftVertices() const
	{
		return NumSoftVertices;
	}

	/**
	* @return total number of soft and rigid verts for this chunk
	*/
	FORCEINLINE INT GetNumVertices() const
	{
		return GetNumRigidVertices() + GetNumSoftVertices();
	}

	/**
	* @return starting index for rigid verts for this chunk in the LOD vertex buffer
	*/
	FORCEINLINE INT GetRigidVertexBufferIndex() const
	{
        return BaseVertexIndex;
	}

	/**
	* @return starting index for soft verts for this chunk in the LOD vertex buffer
	*/
	FORCEINLINE INT GetSoftVertexBufferIndex() const
	{
        return BaseVertexIndex + NumRigidVertices;
	}

	/**
	* Calculate max # of bone influences used by this skel mesh chunk
	*/
	void CalcMaxBoneInfluences();

	/**
	* Serialize this class
	* @param Ar - archive to serialize to
	* @param C - skel mesh chunk to serialize
	*/
	friend FArchive& operator<<(FArchive& Ar,FSkelMeshChunk& C)
	{
		Ar << C.BaseVertexIndex;
		Ar << C.RigidVertices;
		Ar << C.SoftVertices;
		Ar << C.BoneMap;
		Ar << C.NumRigidVertices;
		Ar << C.NumSoftVertices;
		Ar << C.MaxBoneInfluences;
		return Ar;
	}
};

enum EBoneBreakOption
{
	BONEBREAK_SoftPreferred		=0, 
	BONEBREAK_AutoDetect		=1,
	BONEBREAK_RigidPreferred	=2
};

enum ETriangleSortOption
{
	TRISORT_None						= 0,
	TRISORT_CenterRadialDistance		= 1,
	TRISORT_Random						= 2,
	TRISORT_MergeContiguous				= 3,
	TRISORT_Custom						= 4,
	TRISORT_CustomLeftRight				= 5,
};

/** Helper to convert the above enum to string */
static const TCHAR* TriangleSortOptionToString(ETriangleSortOption Option)
{
	switch(Option)
	{
		case TRISORT_CenterRadialDistance:
			return TEXT("CenterRadialDistance");
		case TRISORT_Random:
			return TEXT("Random");
		case TRISORT_MergeContiguous:
			return TEXT("MergeContiguous");
		case TRISORT_Custom:
			return TEXT("Custom");
		case TRISORT_CustomLeftRight:
			return TEXT("CustomLeftRight");
	}
	return TEXT("None");
}


/** Enum indicating which method to use to generate per-vertex cloth vert movement scale (ClothMovementScale) */
enum EClothMovementScaleGen
{
	ECMDM_DistToFixedVert				= 0,
	ECMDM_VertexBoneWeight				= 1,
	ECMDM_Empty							= 2,
};

/**
 * A set of skeletal mesh triangles which use the same material and chunk.
 */
struct FSkelMeshSection
{
	/** Material (texture) used for this section. */
	WORD MaterialIndex;

	/** The chunk that vertices for this section are from. */
	WORD ChunkIndex;

	/** The offset of this section's indices in the LOD's index buffer. */
	DWORD BaseIndex;

	/** The number of triangles in this section. */
	DWORD NumTriangles;

	/** Current triangle sorting method */
	BYTE TriangleSorting;

	/** Is this mesh selected? */
	BYTE bSelected:1;

	FSkelMeshSection()
		: MaterialIndex(0)
		, ChunkIndex(0)
		, BaseIndex(0)
		, NumTriangles(0)
		, TriangleSorting(0)
		, bSelected(0)
	{}

	// Serialization.
	friend FArchive& operator<<(FArchive& Ar,FSkelMeshSection& S)
	{		
		Ar << S.MaterialIndex;
		Ar << S.ChunkIndex;
		Ar << S.BaseIndex;
		
		if (Ar.Ver() < VER_DWORD_SKELETAL_MESH_INDICES)
		{
			WORD NumTriangles;
			Ar << NumTriangles;
			S.NumTriangles = NumTriangles;
		}
		else
		{
			Ar << S.NumTriangles;
		}
		
		if( Ar.Ver() >= VER_SKELETAL_MESH_SORTING_OPTIONS )
		{
			Ar << S.TriangleSorting;
		}
		else if( Ar.IsLoading() )
		{
			S.TriangleSorting = TRISORT_None;
		}

		return Ar;
	}
};
template <> struct TIsPODType<FSkelMeshSection> { enum { Value = true }; };

/**
* Base vertex data for GPU skinned skeletal meshes
*/
struct FGPUSkinVertexBase
{
	FPackedNormal	TangentX,	// Tangent, U-direction
					TangentZ;	// Normal	
	BYTE			InfluenceBones[MAX_INFLUENCES];
	BYTE			InfluenceWeights[MAX_INFLUENCES];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	*/
	void Serialize(FArchive& Ar, FVector & OutPosition);
	void Serialize(FArchive& Ar);
};

/** 
* 16 bit UV version of skeletal mesh vertex
*/
template<UINT NumTexCoords=1>
struct TGPUSkinVertexFloat16Uvs : public FGPUSkinVertexBase
{
	/** full float position **/
	FVector			Position;
	/** half float UVs */
	FVector2DHalf	UVs[NumTexCoords];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,TGPUSkinVertexFloat16Uvs& V)
	{
		// If prior to VER_SKELETAL_MESH_SUPPORT_PACKED_POSITION, then 
		// you need to get position from parent class
		if( Ar.Ver() < VER_SKELETAL_MESH_SUPPORT_PACKED_POSITION )
		{
			V.Serialize(Ar, V.Position);
		}
		else
		{
			V.Serialize(Ar);
			Ar << V.Position;
		}

		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			Ar << V.UVs[UVIndex];
		}
		return Ar;
	}
};

/** 
* 32 bit UV version of skeletal mesh vertex
*/
template<UINT NumTexCoords=1>
struct TGPUSkinVertexFloat32Uvs : public FGPUSkinVertexBase
{
	/** full float position **/
	FVector			Position;
	/** full float UVs */
	FVector2D UVs[NumTexCoords];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,TGPUSkinVertexFloat32Uvs& V)
	{
		// If prior to VER_SKELETAL_MESH_SUPPORT_PACKED_POSITION, then 
		// you need to get position from parent class
		if( Ar.Ver() < VER_SKELETAL_MESH_SUPPORT_PACKED_POSITION )
		{
			V.Serialize(Ar, V.Position);
		}
		else
		{
			V.Serialize(Ar);
			Ar << V.Position;
		}

		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			Ar << V.UVs[UVIndex];
		}
		return Ar;
	}
};

/** 
* 16 bit XYZ/16 bit UV version of skeletal mesh vertex
*/
template<UINT NumTexCoords=1>
struct TGPUSkinVertexFloat16Uvs32Xyz : public FGPUSkinVertexBase
{
	FPackedPosition Position;
	/** half float UVs */
	FVector2DHalf	UVs[NumTexCoords];
	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,TGPUSkinVertexFloat16Uvs32Xyz& V)
	{
		V.Serialize(Ar);
		
		// If after VER_SKELETAL_MESH_SUPPORT_PACKED_POSITION, then 
		// you need to serialize position 
		// Otherwise, don't serialize. Just by any reason, serialize happens with this struct
		if( Ar.Ver() >= VER_SKELETAL_MESH_SUPPORT_PACKED_POSITION )
		{
			Ar << V.Position;
		}

		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			Ar << V.UVs[UVIndex];
		}
		return Ar;
	}
};

/** 
* 16 bit XYZ/32 bit UV version of skeletal mesh vertex
*/
template<UINT NumTexCoords=1>
struct TGPUSkinVertexFloat32Uvs32Xyz : public FGPUSkinVertexBase
{
	FPackedPosition Position;
	/** full float UVs */
	FVector2D	UVs[NumTexCoords];

	/**
	* Serializer
	*
	* @param Ar - archive to serialize with
	* @param V - vertex to serialize
	* @return archive that was used
	*/
	friend FArchive& operator<<(FArchive& Ar,TGPUSkinVertexFloat32Uvs32Xyz& V)
	{
		V.Serialize(Ar);
		// If after VER_SKELETAL_MESH_SUPPORT_PACKED_POSITION, then 
		// you need to serialize position 
		// Otherwise, don't serialize. Just by any reason, serialize happens with this struct		
		if( Ar.Ver() >= VER_SKELETAL_MESH_SUPPORT_PACKED_POSITION )
		{
			Ar << V.Position;
		}

		for(UINT UVIndex = 0;UVIndex < NumTexCoords;UVIndex++)
		{
			Ar << V.UVs[UVIndex];
		}
		return Ar;
	}
};

/**
 * A structure for holding a skeletal mesh vertex color
 */
struct FGPUSkinVertexColor
{
	/** VertexColor */
	FColor VertexColor;

	/**
	 * Serializer
	 *
	 * @param Ar - archive to serialize with
	 * @param V - vertex to serialize
	 * @return archive that was used
	 */
	friend FArchive& operator<<(FArchive& Ar, FGPUSkinVertexColor& V)
	{
		// Serialization really shouldn't get this far if our package version is older
		// Just in case, we should guard around it
		if( Ar.Ver() >= VER_ADDED_SKELETAL_MESH_VERTEX_COLORS )
		{
			Ar << V.VertexColor;
		}

		return Ar;
	}
};

/** An interface to the skel-mesh vertex data storage type. */
class FSkeletalMeshVertexDataInterface
{
public:

	/** Virtual destructor. */
	virtual ~FSkeletalMeshVertexDataInterface() {}

	/**
	* Resizes the vertex data buffer, discarding any data which no longer fits.
	* @param NumVertices - The number of vertices to allocate the buffer for.
	*/
	virtual void ResizeBuffer(UINT NumVertices) = 0;

	/** @return The stride of the vertex data in the buffer. */
	virtual UINT GetStride() const = 0;

	/** @return A pointer to the data in the buffer. */
	virtual BYTE* GetDataPointer() = 0;

	/** @return number of vertices in the buffer */
	virtual UINT GetNumVertices() = 0;

	/** @return A pointer to the FResourceArrayInterface for the vertex data. */
	virtual FResourceArrayInterface* GetResourceArray() = 0;

	/** Serializer. */
	virtual void Serialize(FArchive& Ar) = 0;
};


/** The implementation of the skeletal mesh vertex data storage type. */
template<typename VertexDataType>
class TSkeletalMeshVertexData :
	public FSkeletalMeshVertexDataInterface,
	public TResourceArray<VertexDataType,VERTEXBUFFER_ALIGNMENT>
{
public:
	typedef TResourceArray<VertexDataType,VERTEXBUFFER_ALIGNMENT> ArrayType;

	/**
	* Constructor
	* @param InNeedsCPUAccess - TRUE if resource array data should be CPU accessible
	*/
	TSkeletalMeshVertexData(UBOOL InNeedsCPUAccess=FALSE)
		:	TResourceArray<VertexDataType,VERTEXBUFFER_ALIGNMENT>(InNeedsCPUAccess)
	{
	}
	
	/**
	* Resizes the vertex data buffer, discarding any data which no longer fits.
	*
	* @param NumVertices - The number of vertices to allocate the buffer for.
	*/
	virtual void ResizeBuffer(UINT NumVertices)
	{
		if((UINT)ArrayType::Num() < NumVertices)
		{
			// Enlarge the array.
			ArrayType::Add(NumVertices - ArrayType::Num());
		}
		else if((UINT)ArrayType::Num() > NumVertices)
		{
			// Shrink the array.
			ArrayType::Remove(NumVertices,ArrayType::Num() - NumVertices);
		}
	}
	/**
	* @return stride of the vertex type stored in the resource data array
	*/
	virtual UINT GetStride() const
	{
		return sizeof(VertexDataType);
	}
	/**
	* @return BYTE pointer to the resource data array
	*/
	virtual BYTE* GetDataPointer()
	{
		return (BYTE*)&(*this)(0);
	}
	/**
	* @return number of vertices stored in the resource data array
	*/
	virtual UINT GetNumVertices()
	{
		return ArrayType::Num();
	}
	/**
	* @return resource array interface access
	*/
	virtual FResourceArrayInterface* GetResourceArray()
	{
		return this;
	}
	/**
	* Serializer for this class
	*
	* @param Ar - archive to serialize to
	* @param B - data to serialize
	*/
	virtual void Serialize(FArchive& Ar)
	{
		ArrayType::BulkSerialize(Ar);
	}
	/**
	* Assignment operator. This is currently the only method which allows for 
	* modifying an existing resource array
	*/
	TSkeletalMeshVertexData<VertexDataType>& operator=(const TArray<VertexDataType>& Other)
	{
		ArrayType::operator=(Other);
		return *this;
	}
};

/** 
* Vertex buffer with static lod chunk vertices for use with GPU skinning 
*/
class FSkeletalMeshVertexBuffer : public FVertexBuffer
{
public:
	/**
	* Constructor
	*/
	FSkeletalMeshVertexBuffer();

	/**
	* Destructor
	*/
	virtual ~FSkeletalMeshVertexBuffer();

	/**
	* Assignment. Assumes that vertex buffer will be rebuilt 
	*/
	FSkeletalMeshVertexBuffer& operator=(const FSkeletalMeshVertexBuffer& Other);
	/**
	* Constructor (copy)
	*/
	FSkeletalMeshVertexBuffer(const FSkeletalMeshVertexBuffer& Other);

	/** 
	* Delete existing resources 
	*/
	void CleanUp();

	/**
	* Initializes the buffer with the given vertices.
	* @param InVertices - The vertices to initialize the buffer with.
	*/
	void Init(const TArray<FSoftSkinVertex>& InVertices);

	/**
	* Serializer for this class
	* @param Ar - archive to serialize to
	* @param B - data to serialize
	*/
	friend FArchive& operator<<(FArchive& Ar,FSkeletalMeshVertexBuffer& VertexBuffer);

	// FRenderResource interface.

	/**
	* Initialize the RHI resource for this vertex buffer
	*/
	virtual void InitRHI();

	/**
	* @return text description for the resource type
	*/
	virtual FString GetFriendlyName() const;

	// Vertex data accessors.

	/** 
	* Const access to entry in vertex data array
	*
	* @param VertexIndex - index into the vertex buffer
	* @return pointer to vertex data cast to base vertex type
	*/
	FORCEINLINE const FGPUSkinVertexBase* GetVertexPtr(UINT VertexIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		return (FGPUSkinVertexBase*)(Data + VertexIndex * Stride);
	}
	/** 
	* Non=Const access to entry in vertex data array
	*
	* @param VertexIndex - index into the vertex buffer
	* @return pointer to vertex data cast to base vertex type
	*/
	FORCEINLINE FGPUSkinVertexBase* GetVertexPtr(UINT VertexIndex)
	{
		checkSlow(VertexIndex < GetNumVertices());
		return (FGPUSkinVertexBase*)(Data + VertexIndex * Stride);
	}
	/**
	* Get the vertex UV values at the given index in the vertex buffer
	*
	* @param VertexIndex - index into the vertex buffer
	* @param UVIndex - [0,MAX_TEXCOORDS] value to index into UVs array
	* @return 2D UV values
	*/
	FORCEINLINE FVector2D GetVertexUV(UINT VertexIndex,UINT UVIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		if( !bUseFullPrecisionUVs )
		{
#if CONSOLE
			if (GetUsePackedPosition())
			{
				return ((TGPUSkinVertexFloat16Uvs32Xyz<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex];
			}
			else
#endif
			{
				return ((TGPUSkinVertexFloat16Uvs<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex];
			}
		}
		else
		{
#if CONSOLE
			if (GetUsePackedPosition())
			{
				return ((TGPUSkinVertexFloat32Uvs32Xyz<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex];
			}
			else
#endif
			{
				return ((TGPUSkinVertexFloat32Uvs<MAX_TEXCOORDS>*)(Data + VertexIndex * Stride))->UVs[UVIndex];
			}
		}		
	}
	
	/**
	* Get the vertex XYZ values at the given index in the vertex buffer
	*
	* @param VertexIndex - index into the vertex buffer
	* @return FVector 3D position
	*/
	FORCEINLINE FVector GetVertexPosition(UINT VertexIndex) const
	{
		checkSlow(VertexIndex < GetNumVertices());
		return GetVertexPosition((const FGPUSkinVertexBase*)(Data + VertexIndex * Stride));
	}

	/**
	* Get the vertex XYZ values of the given SrcVertex
	*
	* @param FGPUSkinVertexBase *
	* @return FVector 3D position
	*/
	FORCEINLINE FVector GetVertexPosition(const FGPUSkinVertexBase* SrcVertex) const
	{
		if( !bUseFullPrecisionUVs )
		{
#if CONSOLE
			if (GetUsePackedPosition())
			{
				return (FVector)(((TGPUSkinVertexFloat16Uvs32Xyz<MAX_TEXCOORDS>*)(SrcVertex))->Position)* MeshExtension + MeshOrigin;
			}
			else
#endif
			{
				return ((TGPUSkinVertexFloat16Uvs<MAX_TEXCOORDS>*)(SrcVertex))->Position;
			}
		}
		else
		{
#if CONSOLE
			if (GetUsePackedPosition())
			{
				return (FVector)(((TGPUSkinVertexFloat32Uvs32Xyz<MAX_TEXCOORDS>*)(SrcVertex))->Position)* MeshExtension + MeshOrigin;
			}
			else
#endif
			{
				return ((TGPUSkinVertexFloat32Uvs<MAX_TEXCOORDS>*)(SrcVertex))->Position;
			}
		}		
	}
	// Other accessors.

	/** 
	* @return TRUE if using 32 bit floats for UVs 
	*/
	FORCEINLINE UBOOL GetUseFullPrecisionUVs() const
	{
		return bUseFullPrecisionUVs;
	}
	/** 
	* @param UseFull - set to TRUE if using 32 bit floats for UVs 
	*/
	FORCEINLINE void SetUseFullPrecisionUVs(UBOOL UseFull)
	{
		bUseFullPrecisionUVs = UseFull;
	}
	/** 
	* @return TRUE if using 16 bit floats for XYZs 
	*/
	FORCEINLINE UBOOL GetUsePackedPosition() const
	{
#if CONSOLE
	#if WITH_ES2_RHI
			return GUsingES2RHI ? FALSE : bUsePackedPosition;
	#else
		return bUsePackedPosition;
	#endif
#else
	#if SUPPORTS_SCRIPTPATCH_CREATION
		//@script patcher
		// during script patch creation, we can potentially read data cooked for console
		// so we need to honor this flag to make sure serialization works
		return GIsScriptPatcherActive ? bUsePackedPosition : FALSE;
	#else
		return FALSE;
	#endif	// SUPPORTS_SCRIPTPATCH_CREATION
#endif
	}
	/** 
	* @return TRUE if saved using 16 bit floats for XYZs 
	*/
	FORCEINLINE UBOOL GetSavedPackedPosition() const
	{
		return bUsePackedPosition;
	}
	/** 
	* @param UseFull - set to TRUE if using 32 bit floats for UVs 
	*/
	FORCEINLINE void SetUsePackedPosition(UBOOL InUsePackedPosition)
	{
		bUsePackedPosition = InUsePackedPosition;
	}
	/** 
	* @return number of vertices in this vertex buffer
	*/
	FORCEINLINE UINT GetNumVertices() const
	{
		return NumVertices;
	}
	/** 
	* @return cached stride for vertex data type for this vertex buffer
	*/
	FORCEINLINE UINT GetStride() const
	{
		return Stride;
	}
	/** 
	* @return total size of data in resource array
	*/
	FORCEINLINE DWORD GetVertexDataSize() const
	{
		return NumVertices * Stride;
	}
	/** 
	* @return Mesh Origin 
	*/
	FORCEINLINE const FVector& GetMeshOrigin() const 		
	{ 
		return MeshOrigin; 
	}

	/** 
	* @return Mesh Extension
	*/
	FORCEINLINE const FVector& GetMeshExtension() const
	{ 
		return MeshExtension;
	}

	/**
	 * @return the number of texture coordinate sets in this buffer
	 */
	FORCEINLINE UINT GetNumTexCoords() const 
	{
		return NumTexCoords;
	}

	/** 
	* @param UseCPUSkinning - set to TRUE if using cpu skinning with this vertex buffer
	*/
	void SetUseCPUSkinning(UBOOL UseCPUSkinning);

	/**
	 * @param InNumTexCoords	The number of texture coordinate sets that should be in this mesh
	 */
	FORCEINLINE void SetNumTexCoords( UINT InNumTexCoords ) 
	{
		NumTexCoords = InNumTexCoords;
	}

	/**
	 * Assignment operator. 
	 */
	template <UINT NumTexCoordsT>
	FSkeletalMeshVertexBuffer& operator=(const TArray< TGPUSkinVertexFloat32Uvs32Xyz<NumTexCoordsT> >& InVertices)
	{
		check(bUseFullPrecisionUVs);
		check(bUsePackedPosition);

		AllocateData();

		*(TSkeletalMeshVertexData< TGPUSkinVertexFloat32Uvs32Xyz<NumTexCoordsT> >*)VertexData = InVertices;

		Data = VertexData->GetDataPointer();
		Stride = VertexData->GetStride();
		NumVertices = VertexData->GetNumVertices();

		return *this;
	}

	
	/**
	 * Assignment operator. 
	 */
	template <UINT NumTexCoordsT>
	FSkeletalMeshVertexBuffer& operator=(const TArray< TGPUSkinVertexFloat16Uvs<NumTexCoordsT> >& InVertices)
	{
		check(!bUseFullPrecisionUVs);
	#if CONSOLE
		check(!bUsePackedPosition);
	#endif

		AllocateData();

		*(TSkeletalMeshVertexData< TGPUSkinVertexFloat16Uvs<NumTexCoordsT> >*)VertexData = InVertices;


		Data = VertexData->GetDataPointer();
		Stride = VertexData->GetStride();
		NumVertices = VertexData->GetNumVertices();

		return *this;
	}

	/**
	 * Assignment operator.  
	 */
	template <UINT NumTexCoordsT>
	FSkeletalMeshVertexBuffer& operator=(const TArray< TGPUSkinVertexFloat32Uvs<NumTexCoordsT> >& InVertices)
	{
		check(bUseFullPrecisionUVs);
	#if CONSOLE
		check(!bUsePackedPosition);
	#endif

		AllocateData();

		*(TSkeletalMeshVertexData< TGPUSkinVertexFloat32Uvs<NumTexCoordsT> >*)VertexData = InVertices;

		Data = VertexData->GetDataPointer();
		Stride = VertexData->GetStride();
		NumVertices = VertexData->GetNumVertices();

		return *this;
	}

	/** 
	 * Assignment operator. 
	 */
	template <UINT NumTexCoordsT>
	FSkeletalMeshVertexBuffer& operator=(const TArray< TGPUSkinVertexFloat16Uvs32Xyz<NumTexCoordsT> >& InVertices)
	{
		check(!bUseFullPrecisionUVs);
		check(bUsePackedPosition);

		AllocateData();

		*(TSkeletalMeshVertexData< TGPUSkinVertexFloat16Uvs32Xyz<NumTexCoordsT> >*)VertexData = InVertices;

		Data = VertexData->GetDataPointer();
		Stride = VertexData->GetStride();
		NumVertices = VertexData->GetNumVertices();

		return *this;
	}



	/**
	* Convert the existing data in this mesh from 16 bit to 32 bit UVs.
	* Without rebuilding the mesh (loss of precision)
	*/
	template<UINT NumTexCoordsT>
	void ConvertToFullPrecisionUVs();

	/**
	* Convert the existing data in this mesh from 12 bytes to 4 bytes XYZs.
	*/
	template<UINT NumTexCoordsT>
	void ConvertToPackedPosition();

private:
	/** InfluenceBones/InfluenceWeights byte order has been swapped */
	UBOOL bInflucencesByteSwapped;
	/** Corresponds to USkeletalMesh::bUseFullPrecisionUVs. if TRUE then 32 bit UVs are used */
	UBOOL bUseFullPrecisionUVs;
	/** TRUE if this vertex buffer will be used with CPU skinning. Resource arrays are set to cpu accessible if this is TRUE */
	UBOOL bUseCPUSkinning;
	/** Corresponds to USkeletalMesh::bUsePackedPosition. if TRUE then 4 byte XYZs are used */
	UBOOL bUsePackedPosition;
	/** Position data has already been packed. Used during cooking to avoid packing twice. */
	UBOOL bProcessedPackedPositions;
	/** The vertex data storage type */
	FSkeletalMeshVertexDataInterface* VertexData;
	/** The cached vertex data pointer. */
	BYTE* Data;
	/** The cached vertex stride. */
	UINT Stride;
	/** The cached number of vertices. */
	UINT NumVertices;
	/** The number of unique texture coordinate sets in this buffer */
	UINT NumTexCoords;

	/** The origin of Mesh **/
	FVector MeshOrigin;
	/** The scale of Mesh **/
	FVector MeshExtension;

	/** 
	* Allocates the vertex data storage type. Based on UV precision needed
	*/
	void AllocateData();	

	/** 
	* Allocates the vertex data to packed position type. 
	* This is to avoid confusion from using AllocateData
	* This only happens during cooking and other than that this won't be used
	*/

	template<UINT NumTexCoordsT>
	void AllocatePackedData(const TArray< TGPUSkinVertexFloat16Uvs32Xyz<NumTexCoordsT> >& InVertices);	
	
	template<UINT NumTexCoordsT>
	void AllocatePackedData(const TArray< TGPUSkinVertexFloat32Uvs32Xyz<NumTexCoordsT> >& InVertices);	

	/** 
	* Copy the contents of the source vertex to the destination vertex in the buffer 
	*
	* @param VertexIndex - index into the vertex buffer
	* @param SrcVertex - source vertex to copy from
	*/
	void SetVertex(UINT VertexIndex,const FSoftSkinVertex& SrcVertex);
};

/** 
 * A vertex buffer for holding skeletal mesh per vertex color information only. 
 * This buffer sits along side FSkeletalMeshVertexBuffer in each skeletal mesh lod
 */
class FSkeletalMeshVertexColorBuffer : public FVertexBuffer
{
public:
	/**
	 * Constructor
	 */
	FSkeletalMeshVertexColorBuffer();

	/**
	 * Destructor
	 */
	virtual ~FSkeletalMeshVertexColorBuffer();

	/**
	 * Assignment. Assumes that vertex buffer will be rebuilt 
	 */
	FSkeletalMeshVertexColorBuffer& operator=(const FSkeletalMeshVertexColorBuffer& Other);
	
	/**
	 * Constructor (copy)
	 */
	FSkeletalMeshVertexColorBuffer(const FSkeletalMeshVertexColorBuffer& Other);

	/** 
	 * Delete existing resources 
	 */
	void CleanUp();

	/**
	 * Initializes the buffer with the given vertices.
	 * @param InVertices - The vertices to initialize the buffer with.
	 */
	void Init(const TArray<FSoftSkinVertex>& InVertices);

	/**
	 * Serializer for this class
	 * @param Ar - archive to serialize to
	 * @param B - data to serialize
	 */
	friend FArchive& operator<<(FArchive& Ar,FSkeletalMeshVertexColorBuffer& VertexBuffer);

	// FRenderResource interface.

	/**
	 * Initialize the RHI resource for this vertex buffer
	 */
	virtual void InitRHI();

	/**
	 * @return text description for the resource type
	 */
	virtual FString GetFriendlyName() const;

	/** 
	 * @return number of vertices in this vertex buffer
	 */
	FORCEINLINE UINT GetNumVertices() const
	{
		return NumVertices;
	}

	/** 
	* @return cached stride for vertex data type for this vertex buffer
	*/
	FORCEINLINE UINT GetStride() const
	{
		return Stride;
	}
	/** 
	* @return total size of data in resource array
	*/
	FORCEINLINE DWORD GetVertexDataSize() const
	{
		return NumVertices * Stride;
	}

	/**
	 * @return the vertex color for the specified index
	 */
	FORCEINLINE const FColor& VertexColor( UINT VertexIndex ) const
	{
		checkSlow( VertexIndex < GetNumVertices() );
		BYTE* VertBase = Data + VertexIndex * Stride;
		return ((FGPUSkinVertexColor*)(VertBase))->VertexColor;
	}
private:
	/** The vertex data storage type */
	FSkeletalMeshVertexDataInterface* VertexData;
	/** The cached vertex data pointer. */
	BYTE* Data;
	/** The cached vertex stride. */
	UINT Stride;
	/** The cached number of vertices. */
	UINT NumVertices;

	/** 
	 * Allocates the vertex data storage type
	 */
	void AllocateData();	

	/** 
	 * Copy the contents of the source color to the destination vertex in the buffer 
	 *
	 * @param VertexIndex - index into the vertex buffer
	 * @param SrcColor - source color to copy from
	 */
	void SetColor(UINT VertexIndex,const FColor& SrcColor);
};

/**
* Vertex influence weights
*/
struct FInfluenceWeights
{
	union 
	{ 
		struct
		{ 
			BYTE InfluenceWeights[MAX_INFLUENCES];
		};
		// for byte-swapped serialization
		DWORD InfluenceWeightsDWORD; 
	};

	/**
	* Serialize to Archive
	*/
	friend FArchive& operator<<( FArchive& Ar, FInfluenceWeights& W )
	{
		return Ar << W.InfluenceWeightsDWORD;
	}
};

/**
* Vertex influence bones
*/
struct FInfluenceBones
{
	union 
	{ 
		struct
		{ 
			BYTE InfluenceBones[MAX_INFLUENCES];
		};
		// for byte-swapped serialization
		DWORD InfluenceBonesDWORD; 
	};

	/**
	* Serialize to Archive
	*/
	friend FArchive& operator<<( FArchive& Ar, FInfluenceBones& W )
	{
		return Ar << W.InfluenceBonesDWORD;
	}

};

/**
* Vertex influence weights and bones
*/
struct FVertexInfluence
{
	FInfluenceWeights Weights;
	FInfluenceBones Bones;

	friend FArchive& operator<<( FArchive& Ar, FVertexInfluence& W )
	{
		return Ar << W.Weights << W.Bones;
	}
};

/**
 * Array of vertex influences
 */
class FSkeletalMeshVertexInfluences : public FVertexBuffer
{
public:
	TResourceArray<FVertexInfluence, VERTEXBUFFER_ALIGNMENT> Influences;

	/** Array of vertex indices to swap by bone pair */
	TMap<struct FBoneIndexPair, TArray<DWORD> > VertexInfluenceMapping;

	/** Sections to swap to when vertex weights usage is IWU_FullSwap */
	TArray<FSkelMeshSection> Sections;

	/** Chunks to swap to when vertex weights usage is IWU_FullSwap */
	TArray<FSkelMeshChunk> Chunks;

	/** Array of all bones used by this alternate weighting */
	TArray<BYTE> RequiredBones;

	/** Usage type specified at import time */
	EInstanceWeightUsage Usage;

	/** A mapping of influence sections to regular sections, only used with CustomLeftRight triangle sorting, not serialized */
	TArray<INT> CustomLeftRightSectionMap;

	FSkeletalMeshVertexInfluences() : Influences(TRUE) {}

	/**
     * Initialize the RHI resource for this vertex buffer
     */
	void InitRHI();

	friend FArchive& operator<<( FArchive& Ar, FSkeletalMeshVertexInfluences& W )
	{
		Ar << W.Influences;

		if (Ar.Ver() >= VER_ADDED_EXTRA_SKELMESH_VERTEX_INFLUENCE_MAPPING)
		{
			if( Ar.Ver() < VER_DWORD_SKELETAL_MESH_INDICES_FIXUP )
			{
				if( Ar.Ver() >= VER_DWORD_SKELETAL_MESH_INDICES )
				{
					BYTE IndexSize;
					Ar << IndexSize;
				}

				TMap<struct FBoneIndexPair, TArray<WORD> > TempMapping;
				Ar << TempMapping;

				for( TMap<struct FBoneIndexPair, TArray<WORD> >::TConstIterator It(TempMapping); It; ++It )
				{
					const TArray<WORD>& TempArray = It.Value();
					TArray<DWORD> NewArray;
					for( INT I = 0; I < TempArray.Num(); ++I )
					{
						NewArray.AddItem( TempArray(I) );
					}

					W.VertexInfluenceMapping.Set( It.Key(), NewArray );
				}
			}
			else
			{
				Ar << W.VertexInfluenceMapping;
			}
		}
		if (Ar.Ver() >= VER_ADDED_CHUNKS_SECTIONS_VERTEX_INFLUENCE)
		{
			Ar << W.Sections;
			Ar << W.Chunks;
		}
		if (Ar.Ver() >= VER_ADDED_REQUIRED_BONES_VERTEX_INFLUENCE)
		{
			Ar << W.RequiredBones;
		}
		if (Ar.Ver() < VER_ADDED_USAGE_VERTEX_INFLUENCE)
		{
			W.Usage = IWU_PartialSwap;
		}
		else
		{
			BYTE Usage;
			if (Ar.IsLoading())
			{
				Ar << Usage;
				W.Usage = (EInstanceWeightUsage)Usage;
			}
			else
			{
				Usage = (BYTE)W.Usage;
				Ar << Usage;
			}
		}				  

		return Ar;
	}
};

struct FMultiSizeIndexContainerData
{
	TArray<DWORD> Indices;
	UINT DataTypeSize;
	UINT NumVertsPerInstance;
	UBOOL bNeedsCPUAccess;
	UBOOL bSetUpForInstancing;
};

/**
 * Skeletal mesh index buffers are 16 bit by default and 32 bit when called for.
 * This class adds a level of abstraction on top of the index buffers so that we can treat them all as 32 bit.
 */
class FMultiSizeIndexContainer
{
public:
	FMultiSizeIndexContainer(UBOOL bNeedsCPUAccess = FALSE)
	: NeedsCPUAccess(bNeedsCPUAccess)
	, DataTypeSize(sizeof(WORD))
	, IndexBuffer(NULL)
	{
	}

	~FMultiSizeIndexContainer();
	
	/**
	 * Initialize the index buffer's render resources.
	 */
	void InitResources();

	/**
	 * Releases the index buffer's render resources.
	 */	
	void ReleaseResources();

	/**
	 * Creates a new index buffer
	 */
	void CreateIndexBuffer(BYTE DataTypeSize);

	/**
	 * Repopulates the index buffer
	 */
	void RebuildIndexBuffer( const FMultiSizeIndexContainerData& InData );

	/**
	 * Returns a 32 bit version of the index buffer
	 */
	void GetIndexBuffer( TArray<DWORD>& OutArray ) const;

	/**
	 * Populates the index buffer with a new set of indices
	 */
	void CopyIndexBuffer(const TArray<DWORD>& NewArray);

	UBOOL IsIndexBufferValid() const { return IndexBuffer != NULL; }

	/**
	 * Accessors
	 */
	UBOOL GetNeedsCPUAccess() const { return NeedsCPUAccess; }
	BYTE GetDataTypeSize() const { return DataTypeSize; }
	FRawStaticIndexBuffer16or32Interface* GetIndexBuffer() 
	{ 
		check( IndexBuffer != NULL );
		return IndexBuffer; 
	}
	const FRawStaticIndexBuffer16or32Interface* GetIndexBuffer() const
	{ 
		check( IndexBuffer != NULL );
		return IndexBuffer; 
	}
	
#if WITH_EDITOR
	/**
	 * Retrieves index buffer related data
	 */
	void GetIndexBufferData( FMultiSizeIndexContainerData& OutData ) const;
	
	FMultiSizeIndexContainer(const FMultiSizeIndexContainer& Other);
	FMultiSizeIndexContainer& operator=(const FMultiSizeIndexContainer& Buffer);
#endif

#if WITH_EDITORONLY_DATA
	/**
	 * Strips all data from the index buffer.
	 */
	void StripData();
#endif // #if WITH_EDITORONLY_DATA

	friend FArchive& operator<<(FArchive& Ar, FMultiSizeIndexContainer& Buffer);

private:
	/** Specifies whether, or not, the index buffer array needs CPU access */
	UBOOL NeedsCPUAccess;
	/** Size of the index buffer's index type (should be 2 or 4 bytes) */
	BYTE DataTypeSize;
	/** The vertex index buffer */
	FRawStaticIndexBuffer16or32Interface* IndexBuffer;
};

/**
* All data to define a certain LOD model for a skeletal mesh.
* All necessary data to render smooth-parts is in SkinningStream, SmoothVerts, SmoothSections and SmoothIndexbuffer.
* For rigid parts: RigidVertexStream, RigidIndexBuffer, and RigidSections.
*/
class FStaticLODModel
{
public:
	/** Sections. */
	TArray<FSkelMeshSection> Sections;

	/** The vertex chunks which make up this LOD. */
	TArray<FSkelMeshChunk> Chunks;

	/** 
	* Bone hierarchy subset active for this chunk.
	* This is a map between the bones index of this LOD (as used by the vertex structs) and the bone index in the reference skeleton of this SkeletalMesh.
	*/
	TArray<WORD> ActiveBoneIndices;  
	
	/** 
	* Bones that should be updated when rendering this LOD. This may include bones that are not required for rendering.
	* All parents for bones in this array should be present as well - that is, a complete path from the root to each bone.
	* For bone LOD code to work, this array must be in strictly increasing order, to allow easy merging of other required bones.
	*/
	TArray<BYTE> RequiredBones;

	/** 
	* Rendering data.
	*/
	FMultiSizeIndexContainer	MultiSizeIndexContainer; 
	UINT						Size;
	UINT						NumVertices;
	/** The number of unique texture coordinate sets in this lod */
	UINT						NumTexCoords;

	/** Resources needed to render the model using PN-AEN */
	FMultiSizeIndexContainer	AdjacencyMultiSizeIndexContainer;

	/** static vertices from chunks for skinning on GPU */
	FSkeletalMeshVertexBuffer	VertexBufferGPUSkin;
	
	/** A buffer for vertex colors */
	FSkeletalMeshVertexColorBuffer	ColorVertexBuffer;

	/** Optional array of weight/bone influences that can be used by this mesh. Defaults are in VertexBufferGPUSkin */
	TArray<FSkeletalMeshVertexInfluences> VertexInfluences;
	
	/** Editor only data: array of the original point (wedge) indices for each of the vertices in a FStaticLODModel */
	FIntBulkData				RawPointIndices;
	FWordBulkData				LegacyRawPointIndices;

	/**
	* Initialize the LOD's render resources.
	*
	* @param Parent Parent mesh
	*/
	void InitResources(class USkeletalMesh* Parent);

	/**
	* Releases the LOD's render resources.
	*/
	void ReleaseResources();

	/** Constructor (default) */
	FStaticLODModel()
	:	MultiSizeIndexContainer(TRUE)	// needs to be CPU accessible for CPU-skinned decals.
	,	Size(0)
	,	NumVertices(0)
	,	AdjacencyMultiSizeIndexContainer(TRUE)
	{}

	/**
	 * Special serialize function passing the owning UObject along as required by FUnytpedBulkData
	 * serialization.
	 *
	 * @param	Ar		Archive to serialize with
	 * @param	Owner	UObject this structure is serialized within
	 * @param	Idx		Index of current array entry being serialized
	 */
	void Serialize( FArchive& Ar, UObject* Owner, INT Idx );

	/**
	* Fill array with vertex position and tangent data from skel mesh chunks.
	*
	* @param Vertices Array to fill.
	*/
	void GetVertices(TArray<FSoftSkinVertex>& Vertices) const;

	/**
	* Initialize postion and tangent vertex buffers from skel mesh chunks
	*
	* @param Mesh Parent mesh
	*/
	void BuildVertexBuffers(const class USkeletalMesh* Mesh, UBOOL bUsePackedPosition);

	/** Utility function for returning total number of faces in this LOD. */
	INT GetTotalFaces();

	/** Utility for finding the chunk that a particular vertex is in. */
	void GetChunkAndSkinType(INT InVertIndex, INT& OutChunkIndex, INT& OutVertIndex, UBOOL& bOutSoftVert) const;

	/** Sort the triangles with the specified sorting method */
	void SortTriangles( USkeletalMesh* SkelMesh, INT SectionIndex, ETriangleSortOption NewTriangleSorting );

	/** Ensures triangle sorting modes are set up properly for alternate vertex blend weight sections */
	void UpdateTriangleSortingForAltVertexInfluences();
};

/**
 * FSkeletalMeshSourceData - Source triangles and render data, editor-only.
 */
class FSkeletalMeshSourceData
{
public:
	FSkeletalMeshSourceData();
	~FSkeletalMeshSourceData();

#if WITH_EDITOR
	/** Initialize from static mesh render data. */
	void Init( const class USkeletalMesh* SkeletalMesh, FStaticLODModel& LODModel );

	/** Retrieve render data. */
	FORCEINLINE FStaticLODModel* GetModel() { return LODModel; }
#endif // #if WITH_EDITOR

#if WITH_EDITORONLY_DATA
	/** Free source data. */
	void Clear();
#endif // WITH_EDITORONLY_DATA

	/** Returns TRUE if the source data has been initialized. */
	FORCEINLINE UBOOL IsInitialized() const { return LODModel != NULL; }

	/** Serialization. */
	void Serialize( FArchive& Ar, USkeletalMesh* SkeletalMesh );

private:
	FStaticLODModel* LODModel;
};

enum ESkeletalMeshOptimizationImportance
{
	SMOI_Off		=0,
	SMOI_Lowest		=1,
	SMOI_Low		=2,
	SMOI_Normal		=3,
	SMOI_High		=4,
	SMOI_Highest	=5,
	SMOI_Max
};

/** Enum specifying the reduction type to use when simplifying skeletal meshes. */
enum SkeletalMeshOptimizationType
{
	SMOT_NumOfTriangles = 0,
	SMOT_MaxDeviation   = 1,
	SMOT_MAX,
};

enum ESkeletalMeshOptimizationNormalMode
{
	SMONM_RecalculateNormals		=0,
	SMONM_RecalculateNormalsSmooth	=1,
	SMONM_RecalculateNormalsHard	=2,
	SMONM_Max
};

/**
 * FSkeletalMeshOptimizationSettings - The settings used to optimize a skeletal mesh LOD.
 */
struct FSkeletalMeshOptimizationSettings
{
	/** Maximum deviation from the base mesh as a percentage of the bounding sphere. */
	FLOAT MaxDeviationPercentage;
	/** How important the shape of the geometry is (ESkeletalMeshOptimizationImportance). */
	BYTE SilhouetteImportance;
	/** How important texture density is (ESkeletalMeshOptimizationImportance). */
	BYTE TextureImportance;
	/** How important shading quality is. */
	BYTE ShadingImportance;
	/** How important skinning quality is (ESkeletalMeshOptimizationImportance). */
	BYTE SkinningImportance;
	/** How to compute normals for the optimized mesh (ESkeletalMeshOptimizationNormalMode). */
	BYTE NormalMode_DEPRECATED;
	/** The ratio of bones that will be removed from the mesh */
	FLOAT BoneReductionRatio;
	/** Maximum number of bones that can be assigned to each vertex. */
	INT MaxBonesPerVertex;
	/** The method to use when optimizing the skeletal mesh LOD */
	BYTE ReductionMethod;
	/** If ReductionMethod equals SMOT_NumOfTriangles this value is the ratio of triangles [0-1] to remove from the mesh */
	FLOAT NumOfTrianglesPercentage;
	/** The welding threshold distance. Vertices under this distance will be welded. */
	FLOAT WeldingThreshold; 
	/** Whether Normal smoothing groups should be preserved. If false then NormalsThreshold is used **/
	UBOOL bRecalcNormals;
	/** If the angle between two triangles are above this value, the normals will not be
	smooth over the edge between those two triangles. Set in degrees. This is only used when PreserveNormals is set to false*/
	FLOAT NormalsThreshold;

	FSkeletalMeshOptimizationSettings()
		: ReductionMethod(SMOT_MaxDeviation)
		, MaxDeviationPercentage( 0.0f )
		, NumOfTrianglesPercentage(1.0f)
		, WeldingThreshold(0.1f)
		, bRecalcNormals(TRUE)
		, NormalsThreshold(60.0f)
		, SilhouetteImportance( SMOI_Normal )
		, TextureImportance( SMOI_Normal )
		, ShadingImportance( SMOI_Normal )
		, SkinningImportance( SMOI_Normal )
		, NormalMode_DEPRECATED( SMONM_RecalculateNormals )
		, BoneReductionRatio(100.0f)
		, MaxBonesPerVertex(4)
	{
	}
};

/**
 *	Contains the vertices that are most dominated by that bone. Vertices are in Bone space.
 *	Not used at runtime, but useful for fitting physics assets etc.
 */
struct FBoneVertInfo
{
	// Invariant: Arrays should be same length!
	TArray<FVector>	Positions;
	TArray<FVector>	Normals;
};

/** Struct containing triangle sort settings for a particular section */
struct FTriangleSortSettings
{
	BYTE TriangleSorting;
	BYTE CustomLeftRightAxis;
	FName CustomLeftRightBoneName;
};

/** Struct containing information for a particular LOD level, such as materials and info for when to use it. */
struct FSkeletalMeshLODInfo
{
	/**	Indicates when to use this LOD. A smaller number means use this LOD when further away. */
	FLOAT								DisplayFactor;

	/**	Used to avoid 'flickering' when on LOD boundary. Only taken into account when moving from complex->simple. */
	FLOAT								LODHysteresis;

	/** Mapping table from this LOD's materials to the SkeletalMesh materials array. */
	TArray<INT>							LODMaterialMap;

	/** Per-section control over whether to enable shadow casting. */
	TArray<UBOOL>						bEnableShadowCasting;

	/** Per-section sorting options */
	TArray<BYTE>						OLD_TriangleSorting;	// deprecated
	TArray<FTriangleSortSettings>		TriangleSortSettings;

	/** If true, use 16 bit XYZs to save memory. If false, use 32 bit XYZs */
	BITFIELD							bDisableCompression:1;

	/** Whether to disable morph targets for this LOD. */
	BITFIELD							bHasBeenSimplified:1;
};

struct FBoneMirrorInfo
{
	/** The bone to mirror. */
	INT		SourceIndex;
	/** Axis the bone is mirrored across. */
	BYTE	BoneFlipAxis;
};
template <> struct TIsPODType<FBoneMirrorInfo> { enum { Value = true }; };

struct FBoneMirrorExport
{
	FName	BoneName;
	FName	SourceBoneName;
	BYTE	BoneFlipAxis;
};

/** Used to specify special properties for cloth vertices */
enum ClothBoneType
{
	/** The cloth Vertex is attached to the physics asset if available */
	CLOTHBONE_Fixed					= 0,

	/** The cloth Vertex is attached to the physics asset if available and made breakable */
	CLOTHBONE_BreakableAttachment	= 1,

	/** The Cloth Vertex is marked as a tearable vert */
	CLOTHBONE_TearLine				= 2,
};

struct FClothSpecialBoneInfo
{
	/** The bone name to attach to a cloth vertex */
	FName BoneName;

	/** The type of attachment */
	BYTE BoneType;

	/** Array used to cache cloth indices which will be attached to this bone, created in BuildClothMapping(),
	 * Note: These are welded indices.
	 */
	TArray<INT> AttachedVertexIndices;
};


struct FSoftBodyTetraLink
{
	INT TetIndex;
	FVector Bary;
};

struct FSoftBodyTetraLinkArray
{
	FSoftBodyTetraLinkArray(TArray<FSoftBodyTetraLink>& TL) : TetraLinks(TL)
	{
	}

	TArray<FSoftBodyTetraLink> TetraLinks;
};

/** Used to specify special properties for cloth vertices */
enum SoftBodyBoneType
{
	/** The softbody Vertex is attached to the physics asset if available */
	SOFTBODYBONE_Fixed					= 0,
	SOFTBODYBONE_BreakableAttachment	= 1,
	SOFTBODYBONE_TwoWayAttachment		= 2,
};

/**
* Used in editor only when showing tangents/normals/bone weights of information
*/
enum SkinColorRenderMode
{
	ESCRM_None,
	ESCRM_VertexTangent, 
	ESCRM_VertexNormal,
	ESCRM_VertexMirror,
	ESCRM_BoneWeights, 
	ESCRM_Max,
};

struct FSoftBodySpecialBoneInfo
{
	/** The bone name to attach to a softbody vertex */
	FName BoneName;

	/** The type of attachment */
	BYTE BoneType;

	/** Array used to cache softbody indices which will be attached to this bone, created in BuildSoftBodyMapping(),
	 * Note: These are welded indices.
	 */
	TArray<INT> AttachedVertexIndices;
};

/**
 * Used to store temporary data when building a soft-body mesh instead of overwriting existing data.
 */
struct FSoftBodyMeshInfo
{
public:

	/** Mapping between each vertex of the simulated soft-body's surface-mesh and the graphics mesh. */ 
	TArray<INT>					SurfaceToGraphicsVertMap; 

	/** Index buffer of the triangles of the soft-body's surface mesh. */
	TArray<INT>					SurfaceIndices; 

	/** Index buffer of the tetrahedral of the soft-body's tetra-mesh. */	
	TArray<INT>					TetraIndices; 

	/** Mapping between each vertex of the surface-mesh and its tetrahedron, with local positions given in barycentric coordinates. */
	TArray<FSoftBodyTetraLink>	TetraLinks; 

	/** Base array of tetrahedron vertex positions, used to generate the scaled versions from. */
	TArray<FVector>				TetraVertsUnscaled; 

	/** Array used to cache soft-body welded indices which will be attached to a bone. */
	TArray<TArray<INT> >		SpecialBoneAttachedVertexIndicies;

	/**
	 * Determines if the soft-body info contains valid data.
	 *
	 * To be valid, the mesh info must have entries for:
	 *		- SurfaceToGraphicsVertMap
	 *		- SurfaceIndices
	 *		- TetraIndices
	 *		- TetraLinks
	 *		- TetraVertsUnscaled
	 *
	 * Optional entries are:
	 *		- SpecialBoneAttachedVertexIndicies
	 *
	 * @return	TRUE if all the data contained in this structure is valid.
	 */
	UBOOL IsValid() const
	{
		return	(	(SurfaceToGraphicsVertMap.Num() > 0)
				&&	(SurfaceIndices.Num() > 0)
				&&	(TetraIndices.Num() > 0)
				&&	(TetraLinks.Num() > 0)
				&&	(TetraVertsUnscaled.Num() > 0)		);
	}
};
struct FApexClothingLodInfo
{
	FApexClothingLodInfo(INT NumSection)
	{
		ClothingSectionInfo.Empty(NumSection);
	}
	/** The mesh section that the clothing submesh will override. */
	TArray<INT>	ClothingSectionInfo;
};

struct FApexClothingAssetInfo
{
	FApexClothingAssetInfo(INT NumLOD, FName NameOfAsset)
	{
		ClothingLODInfo.Empty(NumLOD);
		ClothingAssetName = NameOfAsset;
	}
	/** Graphical Lod Info for the clothing asset */
	TArray<FApexClothingLodInfo>	ClothingLODInfo;
	/** Clothing Asset Name */
	FName							ClothingAssetName;		
};
/**
* Skeletal mesh.
*/
// DISHONORED(layout): 2012 PDB, Arkane script structs embedded in USkeletalMesh (528 bytes): FUserBounds m_UserBounds @56 (24),
// FBodyPart m_MaterialsToBodyParts elements (20), FSkeletalMesh_EditorOnly EditorOnlyInfo @444 (28).
struct FUserBounds
{
	FName m_BoneName;
	FVector m_Offset;
	FLOAT m_fRadius;
};

struct FBodyPart
{
	FName m_OwnerBone;
	FName m_CutBone;
	BITFIELD m_bShowIfCut:1;
};

struct FSkeletalMesh_EditorOnly
{
	class UPhysicsAsset* BoundsPreviewAsset;
	FStringNoInit SourceFilePath;
	FStringNoInit SourceFileTimestamp;
};

// DISHONORED(layout): 2012 PDB USkeletalMesh is 528 bytes; the data members below follow types.json order (see agents/agentM.md)
class USkeletalMesh : public UObject
{
	DECLARE_CLASS_NOEXPORT(USkeletalMesh, UObject, CLASS_SafeReplace | 0, Engine)

	// DISHONORED(layout): 2012 PDB USkeletalMesh is 528 bytes; data members regenerated in types.json order
	// (reference declarations reused by name, Arkane members synthesized; see agents/agentM.md)
	FUserBounds m_UserBounds;  // DISHONORED(layout): 2012 PDB @56
	FBoxSphereBounds				Bounds;
	TArray<UMaterialInterface*>		Materials;
	TArray<FBodyPart> m_MaterialsToBodyParts;  // DISHONORED(layout): 2012 PDB @120
	FVector 						Origin;
	FRotator						RotOrigin;
	FMatrix m_OriginTransform;  // DISHONORED(layout): 2012 PDB @160
	TArray<BYTE> m_EdgeSkeleton;  // DISHONORED(layout): 2012 PDB @224
	TArray<FMeshBone>				RefSkeleton;
	INT								SkeletalDepth;
	TMap<FName,INT>					NameIndexMap;
	TIndirectArray<FStaticLODModel>	LODModels;
	TArray<FBoneAtom>					RefBasesInvMatrix;	// @todo: wasteful ?!
	TArray<FBoneMirrorInfo>			SkelMirrorTable;
	BYTE							SkelMirrorAxis;
	BYTE							SkelMirrorFlipAxis;
	SCRIPT_ALIGN;
	TArray<USkeletalMeshSocket*>	Sockets;
	TArray<FString>					BoneBreakNames;
	TArray<BYTE>					BoneBreakOptions;
	TArray<FSkeletalMeshLODInfo>	LODInfo;
	TArray<FName>					PerPolyCollisionBones;
	TArray<FName>					AddToParentPerPolyCollisionBone;
	TArray<struct FPerPolyBoneCollisionData> PerPolyBoneKDOPs;
	BITFIELD						bPerPolyUseSoftWeighting:1;
	BITFIELD						bUseSimpleLineCollision:1;
	BITFIELD						bUseSimpleBoxCollision:1;
	BITFIELD						bForceCPUSkinning:1;
	BITFIELD						bUseFullPrecisionUVs:1;
	BITFIELD bUsePackedPosition:1;  // DISHONORED(layout): 2012 PDB @436
	UFaceFXAsset*					FaceFXAsset;
	FSkeletalMesh_EditorOnly EditorOnlyInfo;  // DISHONORED(layout): 2012 PDB @444
	INT								LODBiasPC;
	INT								LODBiasPS3;
	INT								LODBiasXbox360;
	BITFIELD						bHasVertexColors : 1;
	TArray<FLOAT>					CachedStreamingTextureFactors;
	FLOAT							StreamingDistanceMultiplier;
	FRenderCommandFence				ReleaseResourcesFence;
	QWORD							SkelMeshRUID;
	FName m_CachedPathName;  // DISHONORED(layout): 2012 PDB @516

	// DISHONORED(layout): reference-only members absent from the 2012 PDB. Kept as storage-less C++17
	// inline statics (DISHONORED_SHIM_STATIC, Engine.h) so unported reference code still compiles; they are not part of the object layout
	// and the module port has to remove their uses (resources/docs/agents/agentM.md lists them).
	DISHONORED_SHIM_STATIC TArray<class UApexClothingAsset *> ClothingAssets;
	DISHONORED_SHIM_STATIC TArray<FApexClothingAssetInfo> ClothingLodMap;
	DISHONORED_SHIM_STATIC FSkeletalMeshSourceData SourceData;
	DISHONORED_SHIM_STATIC TArray<FSkeletalMeshOptimizationSettings> OptimizationSettings;
	DISHONORED_SHIM_STATIC BITFIELD bHasBeenSimplified;
	DISHONORED_SHIM_STATIC UPhysicsAsset* BoundsPreviewAsset;
	DISHONORED_SHIM_STATIC TArray<UMorphTargetSet*> PreviewMorphSets;
	DISHONORED_SHIM_STATIC FStringNoInit SourceFilePath;
	DISHONORED_SHIM_STATIC FStringNoInit SourceFileTimestamp;
	DISHONORED_SHIM_STATIC TArray<FPointer> ClothMesh;
	DISHONORED_SHIM_STATIC TArray<FLOAT> ClothMeshScale;
	DISHONORED_SHIM_STATIC TArray<INT> ClothToGraphicsVertMap;
	DISHONORED_SHIM_STATIC TArray<FLOAT> ClothMovementScale;
	DISHONORED_SHIM_STATIC BYTE ClothMovementScaleGenMode;
	DISHONORED_SHIM_STATIC FLOAT ClothToAnimMeshMaxDist;
	DISHONORED_SHIM_STATIC BITFIELD bLimitClothToAnimMesh;
	DISHONORED_SHIM_STATIC TArray<INT> ClothWeldingMap;
	DISHONORED_SHIM_STATIC INT ClothWeldingDomain;
	DISHONORED_SHIM_STATIC TArray<INT> ClothWeldedIndices;
	DISHONORED_SHIM_STATIC BITFIELD bForceNoWelding;
	DISHONORED_SHIM_STATIC INT NumFreeClothVerts;
	DISHONORED_SHIM_STATIC TArray<INT> ClothIndexBuffer;
	DISHONORED_SHIM_STATIC TArray<FName> ClothBones;
	DISHONORED_SHIM_STATIC INT ClothHierarchyLevels;
	DISHONORED_SHIM_STATIC BITFIELD bEnableClothBendConstraints;
	DISHONORED_SHIM_STATIC BITFIELD bEnableClothDamping;
	DISHONORED_SHIM_STATIC BITFIELD bUseClothCOMDamping;
	DISHONORED_SHIM_STATIC FLOAT ClothStretchStiffness;
	DISHONORED_SHIM_STATIC FLOAT ClothBendStiffness;
	DISHONORED_SHIM_STATIC FLOAT ClothDensity;
	DISHONORED_SHIM_STATIC FLOAT ClothThickness;
	DISHONORED_SHIM_STATIC FLOAT ClothDamping;
	DISHONORED_SHIM_STATIC INT ClothIterations;
	DISHONORED_SHIM_STATIC INT ClothHierarchicalIterations;
	DISHONORED_SHIM_STATIC FLOAT ClothFriction;
	DISHONORED_SHIM_STATIC FLOAT ClothRelativeGridSpacing;
	DISHONORED_SHIM_STATIC FLOAT ClothPressure;
	DISHONORED_SHIM_STATIC FLOAT ClothCollisionResponseCoefficient;
	DISHONORED_SHIM_STATIC FLOAT ClothAttachmentResponseCoefficient;
	DISHONORED_SHIM_STATIC FLOAT ClothAttachmentTearFactor;
	DISHONORED_SHIM_STATIC FLOAT ClothSleepLinearVelocity;
	DISHONORED_SHIM_STATIC FLOAT HardStretchLimitFactor;
	DISHONORED_SHIM_STATIC BITFIELD bHardStretchLimit;
	DISHONORED_SHIM_STATIC BITFIELD bEnableClothOrthoBendConstraints;
	DISHONORED_SHIM_STATIC BITFIELD bEnableClothSelfCollision;
	DISHONORED_SHIM_STATIC BITFIELD bEnableClothPressure;
	DISHONORED_SHIM_STATIC BITFIELD bEnableClothTwoWayCollision;
	DISHONORED_SHIM_STATIC TArray<FClothSpecialBoneInfo> ClothSpecialBones;
	DISHONORED_SHIM_STATIC BITFIELD bEnableClothLineChecks;
	DISHONORED_SHIM_STATIC BITFIELD bClothMetal;
	DISHONORED_SHIM_STATIC FLOAT ClothMetalImpulseThreshold;
	DISHONORED_SHIM_STATIC FLOAT ClothMetalPenetrationDepth;
	DISHONORED_SHIM_STATIC FLOAT ClothMetalMaxDeformationDistance;
	DISHONORED_SHIM_STATIC BITFIELD bEnableClothTearing;
	DISHONORED_SHIM_STATIC FLOAT ClothTearFactor;
	DISHONORED_SHIM_STATIC INT ClothTearReserve;
	DISHONORED_SHIM_STATIC BITFIELD bEnableValidBounds;
	DISHONORED_SHIM_STATIC FVector ValidBoundsMin;
	DISHONORED_SHIM_STATIC FVector ValidBoundsMax;
	DISHONORED_SHIM_STATIC TMap<QWORD,INT> ClothTornTriMap;
	DISHONORED_SHIM_STATIC TArray<INT> SoftBodySurfaceToGraphicsVertMap;
	DISHONORED_SHIM_STATIC TArray<INT> SoftBodySurfaceIndices;
	DISHONORED_SHIM_STATIC TArray<FVector> SoftBodyTetraVertsUnscaled;
	DISHONORED_SHIM_STATIC TArray<INT> SoftBodyTetraIndices;
	DISHONORED_SHIM_STATIC TArray<FSoftBodyTetraLink> SoftBodyTetraLinks;
	DISHONORED_SHIM_STATIC TArray<FPointer> CachedSoftBodyMeshes;
	DISHONORED_SHIM_STATIC TArray<FLOAT> CachedSoftBodyMeshScales;
	DISHONORED_SHIM_STATIC TArray<FName> SoftBodyBones;
	DISHONORED_SHIM_STATIC TArray<FSoftBodySpecialBoneInfo> SoftBodySpecialBones;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyVolumeStiffness;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyStretchingStiffness;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyDensity;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyParticleRadius;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyDamping;
	DISHONORED_SHIM_STATIC INT SoftBodySolverIterations;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyFriction;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyRelativeGridSpacing;
	DISHONORED_SHIM_STATIC FLOAT SoftBodySleepLinearVelocity;
	DISHONORED_SHIM_STATIC BITFIELD bEnableSoftBodySelfCollision;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyAttachmentResponse;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyCollisionResponse;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyDetailLevel;
	DISHONORED_SHIM_STATIC INT SoftBodySubdivisionLevel;
	DISHONORED_SHIM_STATIC BITFIELD bSoftBodyIsoSurface;
	DISHONORED_SHIM_STATIC BITFIELD bEnableSoftBodyDamping;
	DISHONORED_SHIM_STATIC BITFIELD bUseSoftBodyCOMDamping;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyAttachmentThreshold;
	DISHONORED_SHIM_STATIC BITFIELD bEnableSoftBodyTwoWayCollision;
	DISHONORED_SHIM_STATIC FLOAT SoftBodyAttachmentTearFactor;
	DISHONORED_SHIM_STATIC BITFIELD bEnableSoftBodyLineChecks;
	DISHONORED_SHIM_STATIC TArray<UBOOL> GraphicsIndexIsCloth;
	DISHONORED_SHIM_STATIC BITFIELD bUseClothingAssetMaterial;


	


	/** Array of options that break bones for use in game/editor */



	









#if WITH_EDITORONLY_DATA

#endif // WITH_EDITORONLY_DATA


#if WITH_EDITORONLY_DATA

#endif // WITH_EDITORONLY_DATA

	// CLOTH
	// Under Development! Not a fully supported feature at the moment.


































































	/**
	* Initialize the mesh's render resources.
	*/
	void InitResources();

	/**
	* Releases the mesh's render resources.
	*/
	void ReleaseResources();

	/**
	 * Returns the scale dependent texture factor used by the texture streaming code.	
	 *
	 * @param RequestedUVIndex UVIndex to look at
	 * @return scale dependent texture factor
	 */
	FLOAT GetStreamingTextureFactor( INT RequestedUVIndex );

	// Object interface.
	virtual void PreEditChange(UProperty* PropertyAboutToChange);
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
	virtual void BeginDestroy();
	virtual UBOOL IsReadyForFinishDestroy();
	void Serialize( FArchive& Ar );
	virtual void PostLoad();
	virtual void PreSave();
	virtual void FinishDestroy();
	virtual void PostDuplicate();

	/**
	 * Used by various commandlets to purge editor only and platform-specific data from various objects
	 * 
	 * @param PlatformsToKeep Platforms for which to keep platform-specific data
	 * @param bStripLargeEditorData If TRUE, data used in the editor, but large enough to bloat download sizes, will be removed
	 */
	virtual void StripData(UE3::EPlatformType PlatformsToKeep, UBOOL bStripLargeEditorData);
	
	/** Re-generate the per-poly collision data in the PerPolyBoneKDOPs array, based on names in the PerPolyCollisionBones array. */
	void UpdatePerPolyKDOPs();

	/** 
	 * Returns a one line description of an object for viewing in the thumbnail view of the generic browser
	 */
	virtual FString GetDesc();

	/** 
	 * Returns detailed info to populate listview columns
	 */
	virtual FString GetDetailedDescription( INT InIndex );


	/**
	 * This will return detail info about this specific object. (e.g. AudioComponent will return the name of the cue,
	 * ParticleSystemComponent will return the name of the ParticleSystem)  The idea here is that in many places
	 * you have a component of interest but what you really want is some characteristic that you can use to track
	 * down where it came from.  
	 *
	*/
	virtual FString GetDetailedInfoInternal() const;

	/** Setup-only routines - not concerned with the instance. */
	
	/**
	 * Create all render specific (but serializable) data like e.g. the 'compiled' rendering stream,
	 * mesh sections and index buffer.
	 *
	 * @todo: currently only handles LOD level 0.
	 */
	UBOOL CreateSkinningStreams( 
		const TArray<FVertInfluence>& Influences, 
		const TArray<FMeshWedge>& Wedges, 
		const TArray<FMeshFace>& Faces, 
		const TArray<FVector>& Points,
		const FSkelMeshExtraInfluenceImportData* ExtraInfluenceData=NULL
		);

	/** Calculate the required bones for this LOD, including possible extra influences */
	void CalculateRequiredBones(INT LODIdx);

	void CalculateInvRefMatrices();
	void CalcBoneVertInfos( TArray<FBoneVertInfo>& Infos, UBOOL bOnlyDominant );

	/** Clear and create the NameIndexMap of bone name to bone index. */
	void InitNameIndexMap();

	UBOOL	IsCPUSkinned() const;
	INT		MatchRefBone( FName StartBoneName) const;
	UBOOL	BoneIsChildOf( INT ChildBoneIndex, INT ParentBoneIndex ) const;
	class USkeletalMeshSocket* FindSocket(FName InSocketName);

	FMatrix	GetRefPoseMatrix( INT BoneIndex ) const;

	/** Allocate and initialise bone mirroring table for this skeletal mesh. Default is source = destination for each bone. */
	void InitBoneMirrorInfo();

	/** Utility for copying and converting a mirroring table from another SkeletalMesh. */
	void CopyMirrorTableFrom(USkeletalMesh* SrcMesh);
	void ExportMirrorTable(TArray<FBoneMirrorExport> &MirrorExportInfo);
	void ImportMirrorTable(TArray<FBoneMirrorExport> &MirrorExportInfo);

	/** 
	 *	Utility for checking that the bone mirroring table of this mesh is good.
	 *	Return TRUE if mirror table is OK, false if there are problems.
	 *	@param	ProblemBones	Output string containing information on bones that are currently bad.
	 */
	UBOOL MirrorTableIsGood(FString& ProblemBones);

#if WITH_EDITOR
	/**
	 * Retrieves the source model for this skeletal mesh.
	 */
	FStaticLODModel& GetSourceModel();

	/**
	 * Copies off the source model for this skeletal mesh if necessary and returns it. This function should always be called before
	 * making destructive changes to the mesh's geometry, e.g. simplification.
	 */
	FStaticLODModel& PreModifyMesh();
#endif // #if WITH_EDITOR

	/**
	 * Returns the size of the object/ resource for display to artists/ LDs in the Editor.
	 *
	 * @return size of resource as to be displayed to artists/ LDs in the Editor.
	 */
	INT GetResourceSize();

	/** Uses the ClothBones array to analyze the graphics mesh and generate informaton needed to construct simulation mesh (ClothToGraphicsVertMap etc). */
	void BuildClothMapping();

	void BuildClothTornTriMap();

	/** Using whatever method is specified with ClothMovementScaleGenMode, regen the ClothMovementScale table */
	void GenerateClothMovementScale();

	/** Util to fill in the ClothMovementScale automatically based on how far a vertex is from a fixed one  */
	void GenerateClothMovementScaleFromDistToFixed();

	/** Util to fill in the ClothMovementScale automatically based on how verts are weighted to cloth bones  */
	void GenerateClothMovementScaleFromBoneWeight();

	/** Determines if this mesh is only cloth. */
	UBOOL IsOnlyClothMesh() const;

	/** Reset the store of cooked cloth meshes. Need to make sure you are not actually using any when you call this. */
	void ClearClothMeshCache();

#if WITH_NOVODEX && !NX_DISABLE_CLOTH
	/** Get the cooked NxClothMesh for this mesh at the given scale. */
	class NxClothMesh* GetClothMeshForScale(FLOAT InScale);

	/** Pull the cloth mesh positions from the SkeletalMesh skinning data. */
	UBOOL ComputeClothSectionVertices(TArray<FVector>& ClothSectionVerts, FLOAT InScale, UBOOL ForceNoWelding=FALSE);
#endif

	/** Clears internal soft-body buffers. */
	void ClearSoftBodyMeshCache();

	/** 
	* Replaces the given data with current data to recreate the soft body representation for the mesh
	*/
	void RecreateSoftBody(FSoftBodyMeshInfo& MeshInfo);

#if WITH_NOVODEX && !NX_DISABLE_SOFTBODY

	class NxSoftBodyMesh* GetSoftBodyMeshForScale(FLOAT InScale);

#endif //WITH_NOVODEX && !NX_DISABLE_SOFTBODY

	/**
	* Verify SkeletalMeshLOD is set up correctly	
	*/
	void DebugVerifySkeletalMeshLOD();

	/**
	 * Returns TRUE if the mesh has optimizations stored for the specified LOD.
	 * @param LODIndex - LOD index for which to look for optimization settings.
	 * @returns TRUE if the mesh has optimizations stored for the specified LOD.
	 */
	UBOOL HasOptimizationSettings( INT LODIndex ) const
	{
		return LODIndex >= 0 && LODIndex < OptimizationSettings.Num();
	}

	/**
	 * Retrieves the settings with which the LOD was optimized.
	 * @param LODIndex - LOD index for which to look up the optimization settings.
	 * @returns the optimization settings for the specified LOD.
	 */
	const FSkeletalMeshOptimizationSettings& GetOptimizationSettings( INT LODIndex ) const
	{
		check( LODIndex >= 0 && LODIndex < OptimizationSettings.Num() );
		return OptimizationSettings( LODIndex );
	}

	/**
	 * Stores the settings with which the LOD was optimized.
	 * @param LODIndex - LOD index for which to store optimization settings.
	 * @param Settings - Optimization settings for the specified LOD index.
	 */
	void SetOptimizationSettings( INT LODIndex, const FSkeletalMeshOptimizationSettings& Settings )
	{
		if ( LODIndex >= OptimizationSettings.Num() )
		{
			FSkeletalMeshOptimizationSettings DefaultSettings;
			const FSkeletalMeshOptimizationSettings& SettingsToCopy = OptimizationSettings.Num() ? OptimizationSettings.Last() : DefaultSettings;
			while ( LODIndex >= OptimizationSettings.Num() )
			{
				OptimizationSettings.AddItem( SettingsToCopy );
			}
		}
		check( LODIndex < OptimizationSettings.Num() );
		OptimizationSettings( LODIndex ) = Settings;
	}

	/**
	 * Removes an entry from the list of optimization settings.
	 * @param LODIndex - LOD index for which to remove optimization settings.
	 */
	void RemoveOptimizationSettings( INT LODIndex )
	{
		if ( LODIndex < OptimizationSettings.Num() )
		{
			OptimizationSettings.Remove( LODIndex );
		}
	}

	/**
	 * Clears max deviations.
	 */
	void ClearOptimizationSettings()
	{
		OptimizationSettings.Empty();
	}
	/**
	 * Initializes Lod information for clothing 
	 */
	void InitClothingLod();
};


#include "UnkDOP.h"

typedef TkDOPTreeCompact<class FSkelMeshCollisionDataProvider,WORD>	TSkeletalKDOPTree;
typedef TkDOPTree<class FSkelMeshCollisionDataProvider,WORD>	TSkeletalKDOPTreeLegacy;


/** Data used for doing line checks against triangles rigidly weighted to a specific bone. */
struct FPerPolyBoneCollisionData
{
	/** KDOP tree spacial data structure used for collision checks. */
	TSkeletalKDOPTree								KDOPTree;

	/** Collision vertices, in local bone space */
	TArray<FVector>									CollisionVerts;

	FPerPolyBoneCollisionData() {}

	friend FArchive& operator<<(FArchive& Ar,FPerPolyBoneCollisionData& Data)
	{
		TSkeletalKDOPTreeLegacy LegacykDOPTree;
		UBOOL bNeedsKDopConversion = FALSE;
		if( !Ar.IsLoading() || Ar.Ver() >= VER_COMPACTKDOPSTATICMESH )		
		{
			Ar << Data.KDOPTree;
		}
		else if (Ar.IsLoading())
		{
			Ar << LegacykDOPTree; 
			bNeedsKDopConversion = TRUE;
		}
		Ar << Data.CollisionVerts;

		if (bNeedsKDopConversion)
		{
			TArray<FkDOPBuildCollisionTriangle<WORD> > kDOPBuildTriangles;
			for (INT TriangleIndex = 0; TriangleIndex < LegacykDOPTree.Triangles.Num(); TriangleIndex++)
			{
				FkDOPCollisionTriangle<WORD>& OldTriangle = LegacykDOPTree.Triangles(TriangleIndex);
				new (kDOPBuildTriangles) FkDOPBuildCollisionTriangle<WORD>(
					OldTriangle.v1,
					OldTriangle.v2,
					OldTriangle.v3,
					OldTriangle.MaterialIndex,
					Data.CollisionVerts(OldTriangle.v1),
					Data.CollisionVerts(OldTriangle.v2),
					Data.CollisionVerts(OldTriangle.v3));
			}
			Data.KDOPTree.Build(kDOPBuildTriangles);
		}
		else if (Ar.IsLoading() && Ar.Ver() < VER_KDOP_ONE_NODE_FIX && Data.KDOPTree.Nodes.Num() == 2 )
		{
			TArray<FkDOPBuildCollisionTriangle<WORD> > kDOPBuildTriangles;
			for (INT TriangleIndex = 0; TriangleIndex < Data.KDOPTree.Triangles.Num(); TriangleIndex++)
			{
				FkDOPCollisionTriangle<WORD>& OldTriangle = Data.KDOPTree.Triangles(TriangleIndex);
				new (kDOPBuildTriangles) FkDOPBuildCollisionTriangle<WORD>(
					OldTriangle.v1,
					OldTriangle.v2,
					OldTriangle.v3,
					OldTriangle.MaterialIndex,
					Data.CollisionVerts(OldTriangle.v1),
					Data.CollisionVerts(OldTriangle.v2),
					Data.CollisionVerts(OldTriangle.v3));
			}
			Data.KDOPTree.Build(kDOPBuildTriangles);
		}
		return Ar;
	}
};

/** This struct provides the interface into the skeletal mesh collision data */
class FSkelMeshCollisionDataProvider
{
	/** The component this mesh is attached to */
	const USkeletalMeshComponent* Component;
	/** The mesh that is being collided against */
	class USkeletalMesh* Mesh;
	/** Index into PerPolyBoneKDOPs array within SkeletalMesh */
	INT BoneCollisionIndex;
	/** Index of bone that this collision is for within the skel mesh. */
	INT BoneIndex;
	/** Cached calculated bone transform. Includes scaling. */
	FMatrix BoneToWorld;
	/** Cached calculated inverse bone transform. Includes scaling. */
	FMatrix WorldToBone;

	/** Hide default ctor */
	FSkelMeshCollisionDataProvider(void)
	{
	}

public:
	/** Sets the component and mesh members */
	FORCEINLINE FSkelMeshCollisionDataProvider(const USkeletalMeshComponent* InComponent, USkeletalMesh* InMesh, INT InBoneIndex, INT InBoneCollisionIndex) :
		Component(InComponent),
		Mesh(InComponent->SkeletalMesh),
		BoneCollisionIndex(InBoneCollisionIndex),
		BoneIndex(InBoneIndex)
		{
			BoneToWorld = Component->GetBoneMatrix(BoneIndex);
			WorldToBone = BoneToWorld.InverseSafe();
		}

	FORCEINLINE const FVector& GetVertex(WORD Index) const
	{
		return Mesh->PerPolyBoneKDOPs(BoneCollisionIndex).CollisionVerts(Index);
	}

	FORCEINLINE UMaterialInterface* GetMaterial(WORD MaterialIndex) const
	{
		return Component->GetMaterial(MaterialIndex);
	}

	FORCEINLINE INT GetItemIndex(WORD MaterialIndex) const
	{
		return 0;
	}

	FORCEINLINE UBOOL ShouldCheckMaterial(INT MaterialIndex) const
	{
		return TRUE;
	}

	FORCEINLINE const TSkeletalKDOPTree& GetkDOPTree(void) const
	{
		return Mesh->PerPolyBoneKDOPs(BoneCollisionIndex).KDOPTree;
	}

	FORCEINLINE const FMatrix& GetLocalToWorld(void) const
	{
		return BoneToWorld;
	}

	FORCEINLINE const FMatrix& GetWorldToLocal(void) const
	{
		return WorldToBone;
	}

	FORCEINLINE FMatrix GetLocalToWorldTransposeAdjoint(void) const
	{
		return GetLocalToWorld().TransposeAdjoint();
	}

	FORCEINLINE FLOAT GetDeterminant(void) const
	{
		return GetLocalToWorld().Determinant();
	}
};




/*-----------------------------------------------------------------------------
FSkeletalMeshSceneProxy
-----------------------------------------------------------------------------*/

/**
 * A skeletal mesh component scene proxy.
 */
class FSkeletalMeshSceneProxy : public FPrimitiveSceneProxy
{
public:
	/** 
	 * Constructor. 
	 * @param	Component - skeletal mesh primitive being added
	 */
	FSkeletalMeshSceneProxy(const USkeletalMeshComponent* Component, const FColor& InBoneColor = FColor(230, 230, 255));

	// FPrimitiveSceneProxy interface.

	/** 
	* Draw the scene proxy as a dynamic element
	*
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	Flags - optional set of flags from EDrawDynamicElementFlags
	*/
	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags);

	/**
	* Draw only the secion of the material ID given of the scene proxy as a dynamic element
	*
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	Flags - optional set of flags from EDrawDynamicElementFlags
	* @param 	ForceLOD - Force this LOD. If -1, use current LOD of mesh. 
	* @param	InMaterial - which material section to draw
	*/
	virtual void DrawDynamicElementsByMaterial(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,DWORD Flags, INT ForceLOD, INT InMaterial);

	/**
	* Draws the primitive's dynamic decal elements.  This is called from the rendering thread for each frame of each view.
	* The dynamic elements will only be rendered if GetViewRelevance declares dynamic relevance.
	* Called in the rendering thread.
	*
	* @param	PDI						The interface which receives the primitive elements.
	* @param	View					The view which is being rendered.
	* @param	InDepthPriorityGroup	The DPG which is being rendered.
	* @param	bDynamicLightingPass	TRUE if drawing dynamic lights, FALSE if drawing static lights.
	* @param	bDrawOpaqueDecals		TRUE if we want to draw opaque decals
	* @param	bDrawTransparentDecals	TRUE if we want to draw transparent decals
	* @param	bTranslucentReceiverPass	TRUE during the decal pass for translucent receivers, FALSE for opaque receivers.
	*/
	virtual void DrawDynamicDecalElements(
		FPrimitiveDrawInterface* PDI,
		const FSceneView* View,
		UINT InDepthPriorityGroup,
		UBOOL bDynamicLightingPass,
		UBOOL bDrawOpaqueDecals,
		UBOOL bDrawTransparentDecals,
		UBOOL bTranslucentReceiverPass
		);

	/**
	 * Adds a decal interaction to the primitive.  This is called in the rendering thread by AddDecalInteraction_GameThread.
	 */
	virtual void AddDecalInteraction_RenderingThread(const FDecalInteraction& DecalInteraction);

	/**
	 * Removes a decal interaction from the primitive.  This is called in the rendering thread by RemoveDecalInteraction_GameThread.
	 */
	virtual void RemoveDecalInteraction_RenderingThread(UDecalComponent* DecalComponent);

	/**
	 * Returns the world transform to use for drawing.
	 * @param View - Current view
	 * @param OutLocalToWorld - Will contain the local-to-world transform when the function returns.
	 * @param OutWorldToLocal - Will contain the world-to-local transform when the function returns.
	 */
	virtual void GetWorldMatrices( const FSceneView* View, FMatrix& OutLocalToWorld, FMatrix& OutWorldToLocal );

	/**
	 * Relevance is always dynamic for skel meshes unless they are disabled
	 */
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View);

	/**
	 *	Called during FSceneRenderer::InitViews for view processing on scene proxies before rendering them
	 *  Only called for primitives that are visible and have bDynamicRelevance
 	 *
	 *	@param	ViewFamily		The ViewFamily to pre-render for
	 *	@param	VisibilityMap	A BitArray that indicates whether the primitive was visible in that view (index)
	 *	@param	FrameNumber		The frame number of this pre-render
	 */
	virtual void PreRenderView(const FSceneViewFamily* ViewFamily, const DWORD VisibilityMap, INT FrameNumber);

	/** Util for getting LOD index currently used by this SceneProxy. */
	INT GetCurrentLODIndex();

	/** 
	 * Render bones for debug display
	 */
	void DebugDrawBones(FPrimitiveDrawInterface* PDI,const FSceneView* View, const TArray<FBoneAtom>& InSpaceBases, const class FStaticLODModel& LODModel, const FColor& LineColor, INT ChunkIndexPreview);

	/** 
	 * Render physics asset for debug display
	 */
	void DebugDrawPhysicsAsset(FPrimitiveDrawInterface* PDI,const FSceneView* View);

	/** Render any per-poly collision data for tri's rigidly weighted to bones. */
	void DebugDrawPerPolyCollision(FPrimitiveDrawInterface* PDI, const TArray<FBoneAtom>& InSpaceBases);

	/** 
	 * Render soft body tetrahedra for debug display
	 */
	void DebugDrawSoftBodyTetras(FPrimitiveDrawInterface* PDI, const FSceneView* View);

	virtual DWORD GetMemoryFootprint( void ) const { return( sizeof( *this ) + GetAllocatedSize() ); }
	DWORD GetAllocatedSize( void ) const { return( FPrimitiveSceneProxy::GetAllocatedSize() + LODSections.GetAllocatedSize() ); }

	/**
	* Updates morph material usage for materials referenced by each LOD entry
	*
	* @param bNeedsMorphUsage - TRUE if the materials used by this skeletal mesh need morph target usage
	*/
	void UpdateMorphMaterialUsage(UBOOL bNeedsMorphUsage);

	friend class FSkeletalMeshSectionIter;

protected:
	AActor* Owner;
	const USkeletalMesh* SkeletalMesh;
	FSkeletalMeshObject* MeshObject;
	UPhysicsAsset* PhysicsAsset;

	/** data copied for rendering */
	FColor LevelColor;
	FColor PropertyColor;
	BITFIELD bCastShadow : 1;
	BITFIELD bShouldCollide : 1;
	BITFIELD bDisplayBones : 1;
	BITFIELD bForceWireframe : 1;
	BITFIELD bMaterialsNeedMorphUsage : 1;
	BITFIELD bIsCPUSkinned : 1;
	FMaterialViewRelevance MaterialViewRelevance;

	/** info for section element in an LOD */
	struct FSectionElementInfo
	{
		/*
		FSectionElementInfo() 
		:	Material(NULL)
		,	bEnableShadowCasting(TRUE)
		{}
		*/
		FSectionElementInfo(UMaterialInterface* InMaterial, UBOOL bInEnableShadowCasting, INT InUseMaterialIndex, INT InClothingAssetIndex)
		:	Material( InMaterial )
		,	bEnableShadowCasting( bInEnableShadowCasting )
		,	UseMaterialIndex( InUseMaterialIndex )
		,	ClothingAssetIndex( InClothingAssetIndex )
		{}
		UMaterialInterface* Material;
		/** Whether shadow casting is enabled for this section. */
		UBOOL bEnableShadowCasting;
		/** Index into the materials array of the skel mesh or the component after LOD mapping */
		INT UseMaterialIndex;
		/** Index of the Clothing Asset to use */
		INT ClothingAssetIndex;
	};

	/** Section elements for a particular LOD */
	struct FLODSectionElements
	{
		TArray<FSectionElementInfo> SectionElements;
		// mapping from new sections used when swapping to instance weights to the base SectionElements
		TArray< TArray<INT> > InstanceWeightsSectionElementsMapping;
	};
	
	/** Array of section elements for each LOD */
	TArray<FLODSectionElements> LODSections;
	
	/** This is the color used to render bones if bDisplayBones is set to TRUE */
	FColor BoneColor;

	/** The color used by the wireframe mesh overlay mode */
	FColor WireframeOverlayColor;

	/**
	* Draw only the section of the scene proxy as a dynamic element
	* This is to avoid redundant code of two functions (DrawDynamicElementsByMaterial & DrawDynamicElements)
	* 
	* @param	PDI - draw interface to render to
	* @param	View - current view
	* @param	DPGIndex - current depth priority 
	* @param	const FStaticLODModel& LODModel - LODModel 
	* @param	const FSkelMeshSection& Section - Section
	* @param	const FSkelMeshChunk& Chunk - Chunk
	* @param	const FSectionElementInfo& SectionElementInfo - SectionElementInfo - material ID
	*/
	void DrawDynamicElementsSection(FPrimitiveDrawInterface* PDI,const FSceneView* View,UINT DPGIndex,
		const FStaticLODModel& LODModel, const INT LODIndex, const FSkelMeshSection& Section, 
		const FSkelMeshChunk& Chunk, const FSectionElementInfo& SectionElementInfo, const FTwoVectors& CustomLeftRightVectors );

};


#endif // __UNSKELETALMESH_H__
