/*=============================================================================
	EdgeAnim.h: Sony Edge animation runtime data as Dishonored cooks and uses it.
	DISHONORED(layout): every struct below is the 2012 Shipping PDB layout (resources/docs/types/types.json);
	the retail 2013 exe links the same Edge build (Sony's x86 SSE port, 2012 PDB modules edge/ps3/disjobs).
	Declarations only: the on-disk format and the evaluator are described in resources/docs/edgeanim.md.
=============================================================================*/

#ifndef __EDGEANIM_H__
#define __EDGEANIM_H__

/** Vectormath::Aos::Quat / Point3 / Vector4: four floats (2012 PDB 16 bytes, align 4). */
struct FEdgeAnimVec4
{
	FLOAT X;
	FLOAT Y;
	FLOAT Z;
	FLOAT W;
};

/** EdgeAnimAnimation: header of one cooked Edge animation (UAnimSequence::CompressedByteStream), tag '50AE'. */
struct EdgeAnimAnimation
{
	DWORD tag;
	FLOAT duration;
	FLOAT sampleFrequency;
	WORD sizeHeader;
	WORD numJoints;
	WORD numFrames;
	WORD numFrameSets;
	WORD evalBufferSizeRequired;
	WORD numConstRChannels;
	WORD numConstTChannels;
	WORD numConstSChannels;
	WORD numConstUserChannels;
	WORD numAnimRChannels;
	WORD numAnimTChannels;
	WORD numAnimSChannels;
	WORD numAnimUserChannels;
	WORD flags;
	DWORD sizeJointsWeightArray;
	DWORD eaUserJointWeightArray;
	DWORD pad1;
	DWORD offsetJointsWeightArray;
	DWORD offsetFrameSetDmaArray;
	DWORD offsetFrameSetInfoArray;
	DWORD offsetConstRData;
	DWORD offsetConstTData;
	DWORD offsetConstSData;
	DWORD offsetConstUserData;
	DWORD offsetPackingSpecs;
	DWORD offsetCustomData;
	DWORD sizeCustomData;
	DWORD offsetLocomotionDelta;
	// followed by WORD channelTables[]
};

/** EdgeAnimSkeleton: USkeletalMesh::m_EdgeSkeleton. */
struct EdgeAnimSkeleton
{
	DWORD tag;
	DWORD sizeTotal;
	DWORD sizeCustomData;
	DWORD sizeNameHashes;
	WORD numJoints;
	WORD numUserChannels;
	WORD numSimdHierarchyQuads;
	WORD locomotionJointIndex;
	DWORD offsetBasePose;
	DWORD offsetParentIndicesArray;
	DWORD offsetJointNameHashArray;
	DWORD offsetUserChannelNameHashArray;
	DWORD offsetUserChannelNodeNameHashArray;
	DWORD offsetUserChannelFlagsArray;
	DWORD offsetCustomData;
	DWORD pad0[3];
	// followed by WORD simdHierarchy[]
};

struct EdgeAnimFrameSetInfo
{
	WORD baseFrame;
	WORD numIntraFrames;
};

struct EdgeAnimJointTransform
{
	FEdgeAnimVec4 rotation;
	FEdgeAnimVec4 translation;
	FEdgeAnimVec4 scale;
};

struct EdgeAnimBlendLeaf
{
	union
	{
		DWORD animationHeaderEa;
		DWORD poseAddr;
	};
	WORD animationHeaderSize;
	BYTE flags;
	BYTE cacheIndex;
	FLOAT evalTime;
	DWORD userVal;
};

struct EdgeAnimBlendBranch
{
	WORD operation;
	WORD left;
	WORD right;
	BYTE flags;
	BYTE cacheIndex;
	FLOAT alpha;
	union
	{
		DWORD userVal;
		FLOAT userFloat;
	};
};

struct EdgeAnimPoseInfo
{
	EdgeAnimJointTransform* jointArray;
	BYTE* weightArray;
	FLOAT* userChannelArray;
	BYTE* userChannelWeightArray;
	DWORD* flags;
	BYTE pad[12];
};

struct EdgeAnimCustomDataTable
{
	DWORD numEntries;
	DWORD offsetHashArray;
	DWORD offsetEntrySizes;
	DWORD offsetEntries;
};

/** Per mesh bone: which animation joint drives it and whether the reference pose overrides it. */
struct FEdgeSkelToAnimMapping
{
	WORD m_AnimJointIndex;
	BYTE m_bUseRefPoseRotation;
	BYTE m_bUseRefPoseTranslation;
};

struct FEdgeAnimToSkelMapping
{
	WORD m_SkelJointIndex;
};

struct FLocomotionStateBase
{
	FEdgeAnimVec4 m_LastRootRotation;
	FEdgeAnimVec4 m_LastRootTranslation;
	FLOAT m_fLastRootTime;
	INT m_LoopCount;
	BYTE Pad[8];
};

struct FLocomotionState : public FLocomotionStateBase
{
};

/** UAnimNode::FEdgeAnimTreeContext in retail (nested there; standalone here until the node side is ported). */
struct FEdgeAnimTreeContext
{
	TArray<EdgeAnimBlendBranch> m_BlendBranches;
	TArray<EdgeAnimBlendLeaf> m_BlendLeaves;
	class USkeletalMeshComponent* m_pSkelComponent;
	TMap<class UAnimNode*, WORD> m_NodeCacheMap;
	INT m_NodeCacheCount;
	INT m_AnimJointMaxCount;
};

/** USkeletalMeshComponent::m_pEdgeAnimData (stays NULL until the whole-tree job path is ported). */
MS_ALIGN(16) struct FEdgeAnimData
{
	FEdgeAnimTreeContext m_Context;
	TArray<EdgeAnimJointTransform> m_OutputJoints;
	DWORD m_JobId;
	INT m_TickTag;
	class USkeletalMesh* m_pSkelMesh;
	FBoneAtom ExtractedRootMotionDelta;
	UBOOL bHasRootMotion;
	UBOOL bOldIgnoreControllers;
	UBOOL bJustEnabledKinematicUpdate;
} GCC_ALIGN(16);

template<typename T>
struct FJobBuffer
{
	DWORD m_Size;
	T* m_pData;
};

struct FDisJobRunner
{
	void* m_pImpl;
};

struct FEdgeAnimManager
{
	FDisJobRunner m_JobRunner;
	TArray<class USkeletalMeshComponent*> m_PendingComponents;
};

struct FDisJobDesc
{
	void (*m_ProcessFunc)(FDisJobDesc*);
	DWORD m_ScratchBufferSize;
	DWORD m_OutputBufferSize;
	DWORD m_SizeOfJobDesc;
	DWORD m_JobId;
	void* m_pScratchBuffer;
	void* m_pOutputBuffer;
};

struct FEdgeAnimJobDesc : public FDisJobDesc
{
	FJobBuffer<EdgeAnimSkeleton> m_Skeleton;
	FJobBuffer<EdgeAnimBlendBranch> m_BlendBranches;
	FJobBuffer<EdgeAnimBlendLeaf> m_BlendLeaves;
	FEdgeAnimVec4 m_RotOrigin;
	DWORD m_OutputJointsEA;
	WORD m_NumBlendBranches;
	WORD m_NumBlendLeaves;
	WORD m_TreeRootNode;
	WORD m_AnchorBoneIndex;
	WORD m_MaxAnimJointCount;
	BYTE m_NodeCacheCount;
};

/** One evaluation point: a frame set and the position inside it (retail _edgeAnimProcessCommandList, 2012 rva 0x6238c0). */
struct FEdgeAnimFrameSample
{
	const BYTE* FrameSetData;
	DWORD FrameSetSize;
	DWORD IntraFrameCount;
	DWORD FrameInteger;
	FLOAT FrameFraction;
};

/** DISHONORED(port): Engine/Src/EdgeAnimEvaluate.cpp, plain C++ ports of the retail Edge evaluator (resources/docs/edgeanim.md). */
UBOOL EdgeAnimLocateFrame(const EdgeAnimAnimation* Anim, FLOAT EvalTime, FEdgeAnimFrameSample& OutSample);
void EdgeAnimEvaluate(EdgeAnimJointTransform* OutJoints, const EdgeAnimAnimation* Anim, const FEdgeAnimFrameSample& Sample);
INT EdgeAnimSkeletonGetJointIndexByHash(const EdgeAnimSkeleton* Skel, DWORD JointHash, INT& IndexHint);
/**
 * The skeleton-order pose of retail AnimLeafCallback (2013 rva 0x54ee60): Anim evaluated at Time, its joints mapped onto Skel by
 * name hash, the skeleton base pose (identity/zero when bAdditive) for every joint without a rotation or translation channel.
 * Game thread only (shared scratch). OutJoints gets Skel->numJoints entries.
 */
UBOOL EdgeAnimEvaluateSkeletonPose(const EdgeAnimAnimation* Anim, const EdgeAnimSkeleton* Skel, FLOAT Time, UBOOL bAdditive, TArray<EdgeAnimJointTransform>& OutJoints);

static_assert(sizeof(EdgeAnimAnimation) == 96, "EdgeAnimAnimation: 2012 PDB 96");
static_assert(STRUCT_OFFSET(EdgeAnimAnimation, flags) == 38, "EdgeAnimAnimation::flags: 2012 PDB @38");
static_assert(STRUCT_OFFSET(EdgeAnimAnimation, offsetLocomotionDelta) == 92, "EdgeAnimAnimation::offsetLocomotionDelta: 2012 PDB @92");
static_assert(sizeof(EdgeAnimSkeleton) == 64, "EdgeAnimSkeleton: 2012 PDB 64");
static_assert(STRUCT_OFFSET(EdgeAnimSkeleton, offsetBasePose) == 24, "EdgeAnimSkeleton::offsetBasePose: 2012 PDB @24");
static_assert(STRUCT_OFFSET(EdgeAnimSkeleton, offsetCustomData) == 48, "EdgeAnimSkeleton::offsetCustomData: 2012 PDB @48");
static_assert(sizeof(EdgeAnimFrameSetInfo) == 4, "EdgeAnimFrameSetInfo: 2012 PDB 4");
static_assert(sizeof(EdgeAnimJointTransform) == 48, "EdgeAnimJointTransform: 2012 PDB 48");
static_assert(sizeof(EdgeAnimBlendLeaf) == 16, "EdgeAnimBlendLeaf: 2012 PDB 16");
static_assert(sizeof(EdgeAnimBlendBranch) == 16, "EdgeAnimBlendBranch: 2012 PDB 16");
static_assert(sizeof(EdgeAnimPoseInfo) == 32, "EdgeAnimPoseInfo: 2012 PDB 32");
static_assert(sizeof(EdgeAnimCustomDataTable) == 16, "EdgeAnimCustomDataTable: 2012 PDB 16");
static_assert(sizeof(FEdgeSkelToAnimMapping) == 4, "FEdgeSkelToAnimMapping: 2012 PDB 4");
static_assert(sizeof(FEdgeAnimToSkelMapping) == 2, "FEdgeAnimToSkelMapping: 2012 PDB 2");
static_assert(sizeof(FLocomotionStateBase) == 48, "FLocomotionStateBase: 2012 PDB 48");
static_assert(sizeof(FLocomotionState) == 48, "FLocomotionState: 2012 PDB 48");
static_assert(sizeof(FEdgeAnimTreeContext) == 96, "UAnimNode::FEdgeAnimTreeContext: 2012 PDB 96");
static_assert(STRUCT_OFFSET(FEdgeAnimTreeContext, m_NodeCacheCount) == 88, "FEdgeAnimTreeContext::m_NodeCacheCount: 2012 PDB @88");
static_assert(sizeof(FEdgeAnimData) == 176, "FEdgeAnimData: 2012 PDB 176");
static_assert(STRUCT_OFFSET(FEdgeAnimData, m_pSkelMesh) == 116, "FEdgeAnimData::m_pSkelMesh: 2012 PDB @116");
static_assert(STRUCT_OFFSET(FEdgeAnimData, ExtractedRootMotionDelta) == 128, "FEdgeAnimData::ExtractedRootMotionDelta: 2012 PDB @128");
static_assert(STRUCT_OFFSET(FEdgeAnimData, bJustEnabledKinematicUpdate) == 168, "FEdgeAnimData::bJustEnabledKinematicUpdate: 2012 PDB @168");
static_assert(sizeof(FEdgeAnimManager) == 16, "FEdgeAnimManager: 2012 PDB 16");
static_assert(sizeof(FDisJobDesc) == 28, "FDisJobDesc: 2012 PDB 28");
static_assert(sizeof(FEdgeAnimJobDesc) == 84, "FEdgeAnimJobDesc: 2012 PDB 84");
static_assert(STRUCT_OFFSET(FEdgeAnimJobDesc, m_RotOrigin) == 52, "FEdgeAnimJobDesc::m_RotOrigin: 2012 PDB @52");
static_assert(STRUCT_OFFSET(FEdgeAnimJobDesc, m_NodeCacheCount) == 82, "FEdgeAnimJobDesc::m_NodeCacheCount: 2012 PDB @82");

#endif // __EDGEANIM_H__
