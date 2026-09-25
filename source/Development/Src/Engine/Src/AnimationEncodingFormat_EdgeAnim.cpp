/*=============================================================================
	AnimationEncodingFormat_EdgeAnim.cpp: ACF_EdgeAnim sequences in the reference CPU blend tree.
	DISHONORED(port): Plan B (resources/docs/edgeanim.md §5). EdgeAnimEvaluateSkeletonPose (EdgeAnimEvaluate.cpp) is the
	leaf part of retail's Edge job (AnimLeafCallback, 2013 rva 0x54ee60); this file is retail UpdateSkelPoseEnd's copy
	of the joints into local atoms (2012 rva 0x354c10): rotation and translation as they are, scale 1, no W flip.
=============================================================================*/

#include "EnginePrivate.h"
#include "EngineAnimClasses.h"
#include "EdgeAnim.h"
#include "AnimationEncodingFormat_EdgeAnim.h"

namespace
{

const DWORD EdgeAnimationTag = 0x45413035;
const DWORD EdgeSkeletonTag = 0x45533033;

TArray<EdgeAnimJointTransform> GEdgeSkeletonPose;

void JointToAtom(const EdgeAnimJointTransform& Joint, FBoneAtom& OutAtom)
{
	OutAtom.SetComponents(FQuat(Joint.rotation.X, Joint.rotation.Y, Joint.rotation.Z, Joint.rotation.W), FVector(Joint.translation.X, Joint.translation.Y, Joint.translation.Z));
}

UBOOL EvaluatePose(const UAnimSequence* Seq, const USkeletalMesh* Mesh, FLOAT Time)
{
	check(IsInGameThread());
	return EdgeAnimEvaluateSkeletonPose((const EdgeAnimAnimation*)Seq->CompressedByteStream.GetData(), (const EdgeAnimSkeleton*)Mesh->m_EdgeSkeleton.GetData(), Time, Seq->bIsAdditive, GEdgeSkeletonPose);
}

} // namespace

UBOOL FEdgeAnimSequencePose::CanEvaluate(const UAnimSequence* Seq, const USkeletalMesh* Mesh)
{
	return Seq && Mesh
		&& Seq->RotationCompressionFormat == ACF_EdgeAnim
		&& Seq->CompressedByteStream.Num() >= (INT)sizeof(EdgeAnimAnimation)
		&& ((const EdgeAnimAnimation*)Seq->CompressedByteStream.GetData())->tag == EdgeAnimationTag
		&& Mesh->m_EdgeSkeleton.Num() >= (INT)sizeof(EdgeAnimSkeleton)
		&& ((const EdgeAnimSkeleton*)Mesh->m_EdgeSkeleton.GetData())->tag == EdgeSkeletonTag;
}

UBOOL FEdgeAnimSequencePose::GetAnimationPose(const UAnimSequence* Seq, const USkeletalMesh* Mesh, FLOAT Time, FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones)
{
	if (!EvaluatePose(Seq, Mesh, Time))
	{
		return FALSE;
	}
	for (INT Index = 0; Index < DesiredBones.Num(); Index++)
	{
		const INT BoneIndex = DesiredBones(Index);
		if (BoneIndex < GEdgeSkeletonPose.Num())
		{
			JointToAtom(GEdgeSkeletonPose(BoneIndex), Atoms(BoneIndex));
		}
		else
		{
			Atoms(BoneIndex).SetIdentity();
		}
	}
	return TRUE;
}

UBOOL FEdgeAnimSequencePose::GetBoneAtom(const UAnimSequence* Seq, const USkeletalMesh* Mesh, INT BoneIndex, FLOAT Time, FBoneAtom& OutAtom)
{
	if (!EvaluatePose(Seq, Mesh, Time) || BoneIndex >= GEdgeSkeletonPose.Num())
	{
		return FALSE;
	}
	JointToAtom(GEdgeSkeletonPose(BoneIndex), OutAtom);
	return TRUE;
}
