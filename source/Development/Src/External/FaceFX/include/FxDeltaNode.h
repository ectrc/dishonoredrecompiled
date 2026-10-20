// FaceFX 1.7.3.1: FxDeltaNode (fxdeltanode.h). Agent FX2, wave 26.
#ifndef FxDeltaNode_H__
#define FxDeltaNode_H__

#include "FxFaceGraphNode.h"

namespace OC3Ent
{
namespace Face
{

class FxDeltaNode : public FxFaceGraphNode
{
	FX_DECLARE_CLASS(FxDeltaNode, FxFaceGraphNode)
public:
	FxDeltaNode();
	virtual ~FxDeltaNode();

	virtual FxFaceGraphNode* Clone();
	virtual void CopyData( FxFaceGraphNode* pOther );
	virtual FxBool AddInputLink( const FxFaceGraphNodeLink& newInputLink );
	virtual void Serialize( FxArchive& arc );
};

} // namespace Face
} // namespace OC3Ent

#endif
