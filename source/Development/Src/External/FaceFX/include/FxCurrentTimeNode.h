// FaceFX 1.7.3.1: FxCurrentTimeNode (fxcurrenttimenode.h). Agent FX2, wave 26.
#ifndef FxCurrentTimeNode_H__
#define FxCurrentTimeNode_H__

#include "FxFaceGraphNode.h"

namespace OC3Ent
{
namespace Face
{

class FxCurrentTimeNode : public FxFaceGraphNode
{
	FX_DECLARE_CLASS(FxCurrentTimeNode, FxFaceGraphNode)
public:
	FxCurrentTimeNode();
	virtual ~FxCurrentTimeNode();

	virtual FxFaceGraphNode* Clone();
	virtual void CopyData( FxFaceGraphNode* pOther );
	virtual FxBool AddInputLink( const FxFaceGraphNodeLink& newInputLink );
	virtual void Serialize( FxArchive& arc );
};

} // namespace Face
} // namespace OC3Ent

#endif
