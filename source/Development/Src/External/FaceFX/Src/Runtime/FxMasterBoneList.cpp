/*=============================================================================
	FxMasterBoneList.cpp - FaceFX 1.7.3.1 master bone list (fxmasterbonelist.cpp).

	DISHONORED(port): agent FX3 (wave 26), from the 2012 decompile (FxMasterBoneList.obj). The cooked heads store
	FxMasterBoneList class version 2: the entry array as written below; version 0/1 (bone, index[, refWeight]) is
	read for completeness.
=============================================================================*/
#include "FxMasterBoneList.h"
#include "FxBonePoseNode.h"

namespace OC3Ent
{
namespace Face
{

static_assert(sizeof(FxMasterBoneList::FxMasterBoneListEntry::FxBoneLink) == 52, "FxBoneLink: 2012 PDB sizeof 52");
static_assert(sizeof(FxMasterBoneList::FxMasterBoneListEntry) == 88, "FxMasterBoneListEntry: 2012 PDB sizeof 88");
static_assert(sizeof(FxMasterBoneList) == 16, "FxMasterBoneList: 2012 PDB sizeof 16");

/** 2012 0xb0e3a0. */
FxMasterBoneList::FxMasterBoneListEntry::FxBoneLink::FxBoneLink()
	: nodeIndex(FxInvalidIndex)
{
}

/** 2012 0xb0e3d0. */
FxMasterBoneList::FxMasterBoneListEntry::FxBoneLink::FxBoneLink(FxSize iNodeIndex, const FxName& optimizedBoneName,
	const FxVec3& optimizedBonePos, const FxQuat& optimizedBoneRot, const FxVec3& optimizedBoneScale)
	: nodeIndex(iNodeIndex)
	, optimizedBone(optimizedBoneName, optimizedBonePos, optimizedBoneRot, optimizedBoneScale)
{
}

/** 2012 0xb0e410. */
FxMasterBoneList::FxMasterBoneListEntry::FxBoneLink::~FxBoneLink()
{
}

/** 2012 0xb0e430. */
FxArchive& operator<<(FxArchive& arc, FxMasterBoneList::FxMasterBoneListEntry::FxBoneLink& link)
{
	return arc << link.nodeIndex << link.optimizedBone;
}

/** 2012 0xb0e460. */
FxMasterBoneList::FxMasterBoneListEntry::FxMasterBoneListEntry()
	: index(FX_INT32_MAX)
	, refWeight(1.0f)
	, weight(1.0f)
{
}

/** 2012 0xb0e4e0. */
FxMasterBoneList::FxMasterBoneListEntry::FxMasterBoneListEntry(const FxBone& bone)
	: refBone(bone)
	, refBoneInverseRot(bone.GetRot().GetInverse())
	, index(FX_INT32_MAX)
	, refWeight(1.0f)
	, weight(1.0f)
{
}

/** 2012 0xb0e580. */
FxMasterBoneList::FxMasterBoneListEntry::~FxMasterBoneListEntry()
{
}

/** 2012 0xb0e5e0. A link per bone pose node posing this bone: the pose's offset from the reference (position, scale) and
	its absolute rotation. */
void FxMasterBoneList::FxMasterBoneListEntry::PullBone(FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg)
{
	links.Clear();
	FxSize numLinks = 0;
	FxFaceGraph::Iterator bonePoseNodeIter = faceGraph.Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator bonePoseNodeEndIter = faceGraph.End(FxBonePoseNode::StaticClass());
	for( ; bonePoseNodeIter != bonePoseNodeEndIter; ++bonePoseNodeIter )
	{
		FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(bonePoseNodeIter.GetNode());
		FxSize numBones = pBonePoseNode->GetNumBones();
		for( FxSize i = 0; i < numBones; ++i )
		{
			if( pBonePoseNode->GetBone(i).GetName() == refBone.GetName() )
			{
				++numLinks;
			}
		}
	}
	links.Reserve(numLinks);
	for( bonePoseNodeIter = faceGraph.Begin(FxBonePoseNode::StaticClass()); bonePoseNodeIter != bonePoseNodeEndIter; ++bonePoseNodeIter )
	{
		FxBonePoseNode* pBonePoseNode = static_cast<FxBonePoseNode*>(bonePoseNodeIter.GetNode());
		FxSize numBones = pBonePoseNode->GetNumBones();
		for( FxSize i = 0; i < numBones; ++i )
		{
			const FxBone& poseBone = pBonePoseNode->GetBone(i);
			if( poseBone.GetName() == refBone.GetName() )
			{
				links.PushBack(FxBoneLink(cg.FindNodeIndex(pBonePoseNode->GetName()), poseBone.GetName(),
					poseBone.GetPos() - refBone.GetPos(), poseBone.GetRot(), poseBone.GetScale() - refBone.GetScale()));
			}
		}
	}
}

/** 2012 0xb0e860. */
void FxMasterBoneList::FxMasterBoneListEntry::PushBone(FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg)
{
	FxSize numLinks = links.Length();
	for( FxSize i = 0; i < numLinks; ++i )
	{
		FxBonePoseNode* pBonePoseNode = static_cast<FxBonePoseNode*>(faceGraph.FindNode(cg.nodes[links[i].nodeIndex].name));
		FxBone poseBone(links[i].optimizedBone);
		poseBone.SetPos(refBone.GetPos() + poseBone.GetPos());
		poseBone.SetScale(refBone.GetScale() + poseBone.GetScale());
		pBonePoseNode->AddBone(poseBone);
	}
}

/** 2012 0xb0f4f0. Removes the pose bones that equal the reference pose. */
void FxMasterBoneList::FxMasterBoneListEntry::Prune(FxFaceGraph& faceGraph)
{
	FxFaceGraph::Iterator bonePoseNodeIter = faceGraph.Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator faceGraphEndIter = faceGraph.End(FxBonePoseNode::StaticClass());
	for( ; bonePoseNodeIter != faceGraphEndIter; ++bonePoseNodeIter )
	{
		FxBonePoseNode* pBonePoseNode = static_cast<FxBonePoseNode*>(bonePoseNodeIter.GetNode());
		if( pBonePoseNode )
		{
			for( FxSize i = 0; i < pBonePoseNode->GetNumBones(); ++i )
			{
				const FxBone& poseBone = pBonePoseNode->GetBone(i);
				if( poseBone.GetName() == refBone.GetName() && poseBone == refBone )
				{
					pBonePoseNode->RemoveBone(i--);
				}
			}
		}
	}
}

/** 2012 0xb0e980. */
FxArchive& operator<<(FxArchive& arc, FxMasterBoneList::FxMasterBoneListEntry& entry)
{
	if( arc.GetSDKVersion() < 0x6A4 && arc.IsLoading() )
	{
		FxUInt16 version = 2;
		arc << version;
	}
	return arc << entry.refBone << entry.refBoneInverseRot << entry.index << entry.refWeight << entry.links;
}

/** 2012 0xb0ea10. */
FxMasterBoneList::FxMasterBoneList()
{
}

/** 2012 0xb0ea40. */
FxMasterBoneList::~FxMasterBoneList()
{
}

/** 2012 0xb0ea60. */
void FxMasterBoneList::Clear()
{
	_refBones.Clear();
}

/** 2012 0xb0ea80. The bone's rotation is normalised; a bone already in the list is replaced. */
void FxMasterBoneList::AddRefBone(const FxBone& bone)
{
	FxBone temp(bone);
	FxQuat rot = bone.GetRot();
	rot.Normalize();
	temp.SetRot(rot);
	FxBool alreadyInList = FxFalse;
	FxSize numRefBones = _refBones.Length();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		if( _refBones[i].refBone.GetName() == temp.GetName() )
		{
			_refBones[i].refBone = temp;
			alreadyInList = FxTrue;
		}
	}
	if( !alreadyInList )
	{
		_refBones.PushBack(FxMasterBoneListEntry(temp));
	}
}

/** 2012 0xb0ebe0. */
void FxMasterBoneList::SetRefBoneClientIndex(FxSize refBoneIndex, FxInt32 clientIndex)
{
	_refBones[refBoneIndex].index = clientIndex;
}

/** 2012 0xb0ec10. */
void FxMasterBoneList::PullBones(FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg)
{
	FxSize numRefBones = _refBones.Length();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		_refBones[i].PullBone(faceGraph, cg);
	}
}

/** 2012 0xb0ec70. */
void FxMasterBoneList::PushBones(FxFaceGraph& faceGraph, FxCompiledFaceGraph& cg)
{
	FxSize numRefBones = _refBones.Length();
	for( FxSize i = 0; i < numRefBones; ++i )
	{
		_refBones[i].PushBone(faceGraph, cg);
	}
}

/** 2012 0xb0ecd0. Drops the pose bones no reference bone names, then each entry's reference-equal poses. */
void FxMasterBoneList::Prune(FxFaceGraph& faceGraph)
{
	FxSize numRefBones = _refBones.Length();
	FxFaceGraph::Iterator bonePoseNodeIter = faceGraph.Begin(FxBonePoseNode::StaticClass());
	FxFaceGraph::Iterator faceGraphEndIter = faceGraph.End(FxBonePoseNode::StaticClass());
	for( ; bonePoseNodeIter != faceGraphEndIter; ++bonePoseNodeIter )
	{
		FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(bonePoseNodeIter.GetNode());
		if( pBonePoseNode )
		{
			for( FxSize i = 0; i < pBonePoseNode->GetNumBones(); ++i )
			{
				const FxBone& poseBone = pBonePoseNode->GetBone(i);
				FxBool foundBone = FxFalse;
				for( FxSize j = 0; j < numRefBones; ++j )
				{
					if( poseBone.GetName() == _refBones[j].refBone.GetName() )
					{
						foundBone = FxTrue;
					}
				}
				if( !foundBone )
				{
					pBonePoseNode->RemoveBone(i--);
				}
			}
		}
	}
	for( FxSize index = 0; index < numRefBones; ++index )
	{
		_refBones[index].Prune(faceGraph);
	}
}

/** 2012 0xb0ee30. */
FxArchive& operator<<(FxArchive& arc, FxMasterBoneList& masterBoneList)
{
	FxUInt16 version = arc.SerializeClassVersion("FxMasterBoneList", FxTrue, 2);
	if( version >= 2 )
	{
		arc << masterBoneList._refBones;
	}
	else if( arc.IsLoading() )
	{
		masterBoneList._refBones.Clear();
		FxSize length = 0;
		arc << length;
		masterBoneList._refBones.Reserve(length);
		for( FxSize i = 0; i < length; ++i )
		{
			FxBone bone;
			arc << bone;
			FxMasterBoneList::FxMasterBoneListEntry newEntry(bone);
			FxInt32 index;
			arc << index;
			newEntry.index = index;
			if( version )
			{
				FxReal refWeight;
				arc << refWeight;
				newEntry.refWeight = refWeight;
			}
			masterBoneList._refBones.PushBack(newEntry);
		}
	}
	return arc;
}

} // namespace Face
} // namespace OC3Ent
