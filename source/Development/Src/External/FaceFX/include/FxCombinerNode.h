// FaceFX 1.7.3.1: FxCombinerNode (fxcombinernode.h). Agent FX2, wave 26.
#ifndef FxCombinerNode_H__
#define FxCombinerNode_H__

#include "FxFaceGraphNode.h"

namespace OC3Ent
{
namespace Face
{

class FxCombinerNode : public FxFaceGraphNode
{
	FX_DECLARE_CLASS(FxCombinerNode, FxFaceGraphNode)
public:
	FxCombinerNode();
	virtual ~FxCombinerNode();

	virtual FxFaceGraphNode* Clone();
	virtual void CopyData( FxFaceGraphNode* pOther );
	virtual void Serialize( FxArchive& arc );
};

} // namespace Face
} // namespace OC3Ent

#endif
