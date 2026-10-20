/*=============================================================================
	FxMasterBoneList.h - FaceFX 1.7.3.1 master bone list (fxmasterbonelist.h): every bone the face graph moves,
	its reference pose and, per bone pose node that moves it, a link (the node's index in the compiled graph and the
	bone's pose relative to the reference).

	DISHONORED(port): agent FX3 (wave 26). Layouts (2012 PDB): FxBoneLink 52 {nodeIndex, optimizedBone},
	FxMasterBoneListEntry 88 {refBone, refBoneInverseRot, index (the client bone index, FX_INT32_MAX = unlinked),
	refWeight, weight, links}, FxMasterBoneList 16 {vtable, _refBones}.
	Header inlines retail emitted out of line: SetRefBoneCurrentWeight 0xb0bae0, ResetRefBoneWeights 0xb0bb20,
	_findMasterBoneListEntryIndex 0xb0bb80 (FxActorInstance.obj).
	GetBlendedBone is the bone blend the engine inlined into USkeletalMeshComponent::UpdateFaceFX (2012 0x347b90,
	2013 0x325ea0): start from the reference pose; for each link whose node's final value w is not 0, add w times the
	link's position offset and turn the rotation by inverse(refRot) * slerp(refRot, linkRot, w); renormalise. Its
	quirks are retail's: the slerp start is the reference rotation, negated in place (for every later link too) when
	the link rotation is nearer its negation; a dot product above 0.55 lerps without renormalising. The scale and
	weight outputs are not read by the engine and do not appear in the inlined code: the scale accumulates like the
	position (link scale offset times w) and the weight is the entry's current weight - both reconstructions.
=============================================================================*/
#ifndef FxMasterBoneList_H__
#define FxMasterBoneList_H__

#include "FxCompiledFaceGraph.h"
#include "FxBone.h"
#include <math.h>

namespace OC3Ent
{
namespace Face
{

class FxMasterBoneList
{
public:
	class FxMasterBoneListEntry
	{
	public:
		class FxBoneLink
		{
		public:
			FxBoneLink();
			FxBoneLink(FxSize iNodeIndex, const FxName& optimizedBoneName, const FxVec3& optimizedBonePos,
				const FxQuat& optimizedBoneRot, const FxVec3& optimizedBoneScale);
			~FxBoneLink();

			FxSize nodeIndex;
			FxBone optimizedBone;
		};

		FxMasterBoneListEntry();
		FxMasterBoneListEntry(const FxBone& bone);
		~FxMasterBoneListEntry();

		void PullBone(FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg);
		void PushBone(FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg);
		void Prune(FxFaceGraph& faceGraph);

		FxBone refBone;
		FxQuat refBoneInverseRot;
		FxInt32 index;
		FxReal refWeight;
		FxReal weight;
		FxArray<FxBoneLink> links;
	};

	FxMasterBoneList();
	virtual ~FxMasterBoneList();

	void Clear();

	FxSize GetNumRefBones() const
	{
		return _refBones.Length();
	}

	const FxBone& GetRefBone(FxSize index) const
	{
		return _refBones[index].refBone;
	}

	void AddRefBone(const FxBone& bone);

	FxInt32 GetRefBoneClientIndex(FxSize refBoneIndex) const
	{
		return _refBones[refBoneIndex].index;
	}

	void SetRefBoneClientIndex(FxSize refBoneIndex, FxInt32 clientIndex);

	/** 2012 0xb0bae0. */
	void SetRefBoneCurrentWeight(const FxName& refBoneName, FxReal weight)
	{
		FxSize refBoneIndex = _findMasterBoneListEntryIndex(refBoneName);
		if( refBoneIndex != FxInvalidIndex )
		{
			_refBones[refBoneIndex].weight = weight;
		}
	}

	/** 2012 0xb0bb20. */
	void ResetRefBoneWeights()
	{
		FxSize numRefBones = _refBones.Length();
		for( FxSize i = 0; i < numRefBones; ++i )
		{
			_refBones[i].weight = _refBones[i].refWeight;
		}
	}

	/** The blend inlined into 2012 0x347b90 / 2013 0x325ea0 (banner). */
	void GetBlendedBone(FxSize index, const FxCompiledFaceGraph& cg, FxVec3& bonePos, FxQuat& boneRot, FxVec3& boneScale, FxReal& boneWeight) const
	{
		const FxMasterBoneListEntry& entry = _refBones[index];
		bonePos = entry.refBone.GetPos();
		boneRot = entry.refBone.GetRot();
		boneScale = entry.refBone.GetScale();
		boneWeight = entry.weight;
		FxQuat refRot = boneRot;
		const FxQuat& inverseRot = entry.refBoneInverseRot;
		FxSize numLinks = entry.links.Length();
		for( FxSize i = 0; i < numLinks; ++i )
		{
			const FxMasterBoneListEntry::FxBoneLink& link = entry.links[i];
			FxReal t = cg.nodes[link.nodeIndex].finalValue;
			if( FxRealEquality(t, 0.0f) )
			{
				continue;
			}
			const FxVec3& linkPos = link.optimizedBone.GetPos();
			const FxQuat& linkRot = link.optimizedBone.GetRot();
			const FxVec3& linkScale = link.optimizedBone.GetScale();
			bonePos.x = linkPos.x * t + bonePos.x;
			bonePos.y = linkPos.y * t + bonePos.y;
			bonePos.z = linkPos.z * t + bonePos.z;
			boneScale.x = linkScale.x * t + boneScale.x;
			boneScale.y = linkScale.y * t + boneScale.y;
			boneScale.z = linkScale.z * t + boneScale.z;

			FxReal dw = refRot.w - linkRot.w, dz = refRot.z - linkRot.z, dy = refRot.y - linkRot.y, dx = refRot.x - linkRot.x;
			FxReal sw = refRot.w + linkRot.w, sz = refRot.z + linkRot.z, sy = refRot.y + linkRot.y, sx = refRot.x + linkRot.x;
			if( dw * dw + dz * dz + dy * dy + dx * dx > sw * sw + sz * sz + sy * sy + sx * sx )
			{
				refRot = FxQuat(-refRot.w, -refRot.x, -refRot.y, -refRot.z);
			}
			FxReal cosOmega = refRot.w * linkRot.w + refRot.z * linkRot.z + refRot.y * linkRot.y + refRot.x * linkRot.x;
			FxQuat slerped;
			if( cosOmega <= 0.55f )
			{
				FxDReal omega = acos(static_cast<FxDReal>(cosOmega));
				FxDReal invSin = 1.0 / sin(omega);
				FxDReal startScale = sin((1.0 - t) * omega) * invSin;
				FxDReal endScale = invSin * sin(t * omega);
				slerped.w = static_cast<FxReal>(linkRot.w * endScale + refRot.w * startScale);
				slerped.x = static_cast<FxReal>(refRot.x * startScale + endScale * linkRot.x);
				slerped.y = static_cast<FxReal>(refRot.y * startScale + endScale * linkRot.y);
				slerped.z = static_cast<FxReal>(startScale * refRot.z + endScale * linkRot.z);
			}
			else
			{
				FxReal oneMinusT = 1.0f - t;
				slerped.w = t * linkRot.w + refRot.w * oneMinusT;
				slerped.x = refRot.x * oneMinusT + t * linkRot.x;
				slerped.y = refRot.y * oneMinusT + t * linkRot.y;
				slerped.z = refRot.z * oneMinusT + t * linkRot.z;
			}
			FxQuat delta(
				slerped.w * inverseRot.w - slerped.x * inverseRot.x - slerped.y * inverseRot.y - slerped.z * inverseRot.z,
				slerped.x * inverseRot.w + inverseRot.x * slerped.w + slerped.z * inverseRot.y - slerped.y * inverseRot.z,
				slerped.y * inverseRot.w - slerped.z * inverseRot.x + inverseRot.y * slerped.w + slerped.x * inverseRot.z,
				slerped.y * inverseRot.x + slerped.z * inverseRot.w - slerped.x * inverseRot.y + inverseRot.z * slerped.w);
			boneRot = FxQuat(
				delta.w * boneRot.w - delta.x * boneRot.x - delta.y * boneRot.y - delta.z * boneRot.z,
				delta.x * boneRot.w + delta.w * boneRot.x + delta.z * boneRot.y - delta.y * boneRot.z,
				delta.y * boneRot.w - delta.z * boneRot.x + delta.w * boneRot.y + delta.x * boneRot.z,
				delta.z * boneRot.w + delta.y * boneRot.x - delta.x * boneRot.y + delta.w * boneRot.z);
		}
		FxReal norm = boneRot.x * boneRot.x + boneRot.y * boneRot.y + boneRot.z * boneRot.z + boneRot.w * boneRot.w;
		if( !FxRealEquality(norm - 1.0f, 0.0f) )
		{
			FxReal length = FxSqrt(norm);
			if( length >= FX_REAL_EPSILON )
			{
				FxReal inverse = 1.0f / length;
				boneRot = FxQuat(inverse * boneRot.w, inverse * boneRot.x, inverse * boneRot.y, inverse * boneRot.z);
			}
			else
			{
				boneRot = FxQuat(1.0f, 0.0f, 0.0f, 0.0f);
			}
		}
	}

	void PullBones(FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg);
	void PushBones(FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg);
	void Prune(FxFaceGraph& faceGraph);

private:
	friend FxArchive& operator<<(FxArchive& arc, FxMasterBoneList& masterBoneList);

	/** 2012 0xb0bb80. */
	FxSize _findMasterBoneListEntryIndex(const FxName& refBoneName) const
	{
		FxSize numRefBones = _refBones.Length();
		for( FxSize i = 0; i < numRefBones; ++i )
		{
			if( _refBones[i].refBone.GetName() == refBoneName )
			{
				return i;
			}
		}
		return FxInvalidIndex;
	}

	FxArray<FxMasterBoneListEntry> _refBones;
};

FxArchive& operator<<(FxArchive& arc, FxMasterBoneList::FxMasterBoneListEntry::FxBoneLink& link);
FxArchive& operator<<(FxArchive& arc, FxMasterBoneList::FxMasterBoneListEntry& entry);
FxArchive& operator<<(FxArchive& arc, FxMasterBoneList& masterBoneList);

} // namespace Face
} // namespace OC3Ent

#endif
