// FaceFX 1.7.3.1: FxGenericTargetNode (fxgenerictargetnode.h), FxMorphTargetNode (fxmorphtargetnode.h) and the
// UE3 integration's target nodes FUnrealFaceFXNode / FUnrealFaceFXMorphNode / FUnrealFaceFXMaterialParameterNode
// (fxunrealsupport.h, compiled into the SDK as FxUnrealSupport.obj). Agent FX2, wave 26.
// Deviation: the Unreal node classes live here instead of FxUnrealSupport.h (FX1 owns the rest of that unit).
#ifndef FxGenericTargetNode_H__
#define FxGenericTargetNode_H__

#include "FxFaceGraphNode.h"

// The user property slots the UE3 integration reads (UnFaceFXMorphNode.h / UnFaceFXMaterialNode.h of the integration).
#define FX_MORPH_NODE_TARGET_NAME_INDEX 0
#define FX_MATERIAL_PARAMETER_NODE_MATERIAL_SLOT_ID_INDEX 0
#define FX_MATERIAL_PARAMETER_NODE_PARAMETER_NAME_INDEX 1

namespace OC3Ent
{
namespace Face
{

class FxGenericTargetNode : public FxFaceGraphNode
{
	FX_DECLARE_CLASS(FxGenericTargetNode, FxFaceGraphNode)
public:
	virtual FxFaceGraphNode* Clone();
	virtual void CopyData( FxFaceGraphNode* pOther );
	virtual void Serialize( FxArchive& arc );

protected:
	FxGenericTargetNode();
	virtual ~FxGenericTargetNode();
};

class FxMorphTargetNode : public FxGenericTargetNode
{
	FX_DECLARE_CLASS(FxMorphTargetNode, FxGenericTargetNode)
public:
	FxMorphTargetNode();
	virtual ~FxMorphTargetNode();

	virtual FxFaceGraphNode* Clone();
	virtual void CopyData( FxFaceGraphNode* pOther );
	virtual void Serialize( FxArchive& arc );
};

class FUnrealFaceFXNode : public FxGenericTargetNode
{
	FX_DECLARE_CLASS(FUnrealFaceFXNode, FxGenericTargetNode)
public:
	FUnrealFaceFXNode();
	virtual ~FUnrealFaceFXNode();

	virtual FxFaceGraphNode* Clone();
	virtual void CopyData( FxFaceGraphNode* pOther );
	FxBool ShouldLink() const { return _shouldLink; }
	void SetShouldLink( FxBool shouldLink );
	virtual void ReplaceUserProperty( const FxFaceGraphNodeUserProperty& userProperty );
	virtual void Serialize( FxArchive& arc );

protected:
	FxBool _shouldLink;
};

class FUnrealFaceFXMorphNode : public FUnrealFaceFXNode
{
	FX_DECLARE_CLASS(FUnrealFaceFXMorphNode, FUnrealFaceFXNode)
public:
	FUnrealFaceFXMorphNode();
	virtual ~FUnrealFaceFXMorphNode();

	virtual FxFaceGraphNode* Clone();
	virtual void CopyData( FxFaceGraphNode* pOther );
	virtual void Serialize( FxArchive& arc );
};

class FUnrealFaceFXMaterialParameterNode : public FUnrealFaceFXNode
{
	FX_DECLARE_CLASS(FUnrealFaceFXMaterialParameterNode, FUnrealFaceFXNode)
public:
	FUnrealFaceFXMaterialParameterNode();
	virtual ~FUnrealFaceFXMaterialParameterNode();

	virtual FxFaceGraphNode* Clone();
	virtual void CopyData( FxFaceGraphNode* pOther );
	virtual void Serialize( FxArchive& arc );
};

} // namespace Face
} // namespace OC3Ent

#endif
