// FaceFX 1.7.3.1: FxBonePoseNode (fxboneposenode.h). Agent FX2, wave 26.
#ifndef FxBonePoseNode_H__
#define FxBonePoseNode_H__

#include "FxFaceGraphNode.h"
#include "FxBone.h"

namespace OC3Ent
{
namespace Face
{

class FxBonePoseNode : public FxFaceGraphNode
{
	FX_DECLARE_CLASS(FxBonePoseNode, FxFaceGraphNode)
public:
	FxBonePoseNode();
	virtual ~FxBonePoseNode();

	virtual FxFaceGraphNode* Clone();
	virtual void CopyData( FxFaceGraphNode* pOther );

	/** 2012 0xb0f1a0. */
	FxSize GetNumBones() const { return _bones.Length(); }
	/** 2012 0xb0f1c0. */
	const FxBone& GetBone( FxSize index ) const { return _bones[index]; }
	void AddBone( const FxBone& bone );
	void RemoveBone( FxSize index );
	void RemoveAllBones();

	virtual void Serialize( FxArchive& arc );

private:
	FxArray<FxBone> _bones;
};

} // namespace Face
} // namespace OC3Ent

#endif
