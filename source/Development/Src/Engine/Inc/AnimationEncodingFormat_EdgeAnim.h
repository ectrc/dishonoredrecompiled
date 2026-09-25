/*=============================================================================
	AnimationEncodingFormat_EdgeAnim.h: ACF_EdgeAnim sequences in the reference CPU blend tree.
	DISHONORED(port): Plan B of resources/docs/edgeanim.md §5. Retail evaluates whole trees through Edge jobs
	(FEdgeAnimJobDesc) and leaves the UE codecs NULL for format 7 (AnimationFormat_SetInterfaceLinks, 2013 rva
	0xc8250), so this is not an AnimationEncodingFormat codec: UAnimNodeSequence asks it for a pose on the
	component's mesh, the leaf work of retail AnimLeafCallback (2013 rva 0x54ee60) + UpdateSkelPoseEnd (2012 0x354c10).
=============================================================================*/

#ifndef __ANIMATIONENCODINGFORMAT_EDGEANIM_H__
#define __ANIMATIONENCODINGFORMAT_EDGEANIM_H__

class FEdgeAnimSequencePose
{
public:
	/** TRUE when Seq carries an Edge blob and Mesh an Edge skeleton. */
	static UBOOL CanEvaluate(const UAnimSequence* Seq, const USkeletalMesh* Mesh);

	/**
	 * Local atoms of DesiredBones at Time: the channelled joints of the sequence (mapped by joint name hash onto
	 * Mesh's Edge skeleton), the skeleton base pose elsewhere (identity for additive sequences). No W flip:
	 * Edge quaternions are in the RefSkeleton convention.
	 */
	static UBOOL GetAnimationPose(const UAnimSequence* Seq, const USkeletalMesh* Mesh, FLOAT Time, FBoneAtomArray& Atoms, const TArray<BYTE>& DesiredBones);

	/** One bone of the same pose (root motion extraction). */
	static UBOOL GetBoneAtom(const UAnimSequence* Seq, const USkeletalMesh* Mesh, INT BoneIndex, FLOAT Time, FBoneAtom& OutAtom);
};

#endif // __ANIMATIONENCODINGFORMAT_EDGEANIM_H__
