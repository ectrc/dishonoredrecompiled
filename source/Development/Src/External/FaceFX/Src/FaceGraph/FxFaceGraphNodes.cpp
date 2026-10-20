// FaceFX 1.7.3.1: the node classes - FxCombinerNode, FxDeltaNode, FxCurrentTimeNode, FxGenericTargetNode,
// FxMorphTargetNode, FxBonePoseNode (one .cpp each in the SDK) and the UE3 target nodes of fxunrealsupport.cpp.
// Agent FX2, wave 26, ported from the 2012 decompile.
// Deviation: the seven SDK units are one file here.
#include "FxCombinerNode.h"
#include "FxDeltaNode.h"
#include "FxCurrentTimeNode.h"
#include "FxGenericTargetNode.h"
#include "FxBonePoseNode.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb24fb0 (class), 0xb10970 (ConstructObject). */
FX_IMPLEMENT_CLASS(FxCombinerNode, 0, FxFaceGraphNode)

/** 2012 0xb25040. */
FxCombinerNode::FxCombinerNode()
{
}

/** 2012 0xb25060. */
FxCombinerNode::~FxCombinerNode()
{
}

/** 2012 0xb25080. */
FxFaceGraphNode* FxCombinerNode::Clone()
{
	FxCombinerNode* pNode = new FxCombinerNode();
	CopyData(pNode);
	return pNode;
}

/** 2012 0xb25110. */
void FxCombinerNode::CopyData( FxFaceGraphNode* pOther )
{
	if( pOther )
	{
		FxFaceGraphNode::CopyData(pOther);
	}
}

/** 2012 0xb25130. */
void FxCombinerNode::Serialize( FxArchive& arc )
{
	FxFaceGraphNode::Serialize(arc);
	arc.SerializeClassVersion("FxCombinerNode");
}

/** 2012 0xb25980 (class), 0xb10b70 (ConstructObject). */
FX_IMPLEMENT_CLASS(FxDeltaNode, 0, FxFaceGraphNode)

/** 2012 0xb25a10. */
FxDeltaNode::FxDeltaNode()
{
}

/** 2012 0xb25a30. */
FxDeltaNode::~FxDeltaNode()
{
}

/** 2012 0xb25a50. */
FxFaceGraphNode* FxDeltaNode::Clone()
{
	FxDeltaNode* pNode = new FxDeltaNode();
	CopyData(pNode);
	return pNode;
}

/** 2012 0xb25ae0. */
void FxDeltaNode::CopyData( FxFaceGraphNode* pOther )
{
	if( pOther )
	{
		FxFaceGraphNode::CopyData(pOther);
	}
}

/** 2012 0xb25b00: one input only; the range becomes +-(input max - input min). */
FxBool FxDeltaNode::AddInputLink( const FxFaceGraphNodeLink& newInputLink )
{
	if( _inputs.Length() )
	{
		return FxFalse;
	}
	FxFaceGraphNode::AddInputLink(newInputLink);
	const FxFaceGraphNode* pNode = _inputs[0].GetNode();
	if( pNode )
	{
		FxReal difference = pNode->GetMax() - pNode->GetMin();
		SetMax(difference);
		SetMin(-difference);
	}
	return FxTrue;
}

/** 2012 0xb25b90. */
void FxDeltaNode::Serialize( FxArchive& arc )
{
	FxFaceGraphNode::Serialize(arc);
	arc.SerializeClassVersion("FxDeltaNode");
}

/** 2012 0xb25730 (class), 0xb10af0 (ConstructObject). */
FX_IMPLEMENT_CLASS(FxCurrentTimeNode, 0, FxFaceGraphNode)

/** 2012 0xb257c0. */
FxCurrentTimeNode::FxCurrentTimeNode()
{
}

/** 2012 0xb257e0. */
FxCurrentTimeNode::~FxCurrentTimeNode()
{
}

/** 2012 0xb25800. */
FxFaceGraphNode* FxCurrentTimeNode::Clone()
{
	FxCurrentTimeNode* pNode = new FxCurrentTimeNode();
	CopyData(pNode);
	return pNode;
}

/** 2012 0xb25890. */
void FxCurrentTimeNode::CopyData( FxFaceGraphNode* pOther )
{
	if( pOther )
	{
		FxFaceGraphNode::CopyData(pOther);
	}
}

/** 2012 0xb258b0: the clock takes no inputs. */
FxBool FxCurrentTimeNode::AddInputLink( const FxFaceGraphNodeLink& )
{
	return FxFalse;
}

/** 2012 0xb258c0. */
void FxCurrentTimeNode::Serialize( FxArchive& arc )
{
	FxFaceGraphNode::Serialize(arc);
	arc.SerializeClassVersion("FxCurrentTimeNode");
}

/** 2012 0xb251f0 (class), 0xb109f0 (ConstructObject). */
FX_IMPLEMENT_CLASS(FxGenericTargetNode, 0, FxFaceGraphNode)

/** 2012 0xb25280. */
FxFaceGraphNode* FxGenericTargetNode::Clone()
{
	FxGenericTargetNode* pNode = new FxGenericTargetNode();
	CopyData(pNode);
	return pNode;
}

/** 2012 0xb25310. */
void FxGenericTargetNode::CopyData( FxFaceGraphNode* pOther )
{
	if( pOther )
	{
		FxFaceGraphNode::CopyData(pOther);
	}
}

/** 2012 0xb25330. */
void FxGenericTargetNode::Serialize( FxArchive& arc )
{
	FxFaceGraphNode::Serialize(arc);
	arc.SerializeClassVersion("FxGenericTargetNode");
}

/** 2012 0xb25360. */
FxGenericTargetNode::FxGenericTargetNode()
{
}

/** 2012 0xb25380. */
FxGenericTargetNode::~FxGenericTargetNode()
{
}

/** 2012 0xb25430 (class), 0xb10a70 (ConstructObject). */
FX_IMPLEMENT_CLASS(FxMorphTargetNode, 0, FxGenericTargetNode)

/** 2012 0xb254c0: one string property, "Morph Target Name" = "Target". */
FxMorphTargetNode::FxMorphTargetNode()
{
	FxFaceGraphNodeUserProperty morphTargetNameProperty(UPT_String);
	morphTargetNameProperty.SetName(FxName("Morph Target Name"));
	morphTargetNameProperty.SetStringProperty(FxString("Target"));
	AddUserProperty(morphTargetNameProperty);
}

/** 2012 0xb255a0. */
FxMorphTargetNode::~FxMorphTargetNode()
{
}

/** 2012 0xb255c0. */
FxFaceGraphNode* FxMorphTargetNode::Clone()
{
	FxMorphTargetNode* pNode = new FxMorphTargetNode();
	CopyData(pNode);
	return pNode;
}

/** 2012 0xb25650. */
void FxMorphTargetNode::CopyData( FxFaceGraphNode* pOther )
{
	FxGenericTargetNode::CopyData(pOther);
}

/** 2012 0xb25670. */
void FxMorphTargetNode::Serialize( FxArchive& arc )
{
	FxGenericTargetNode::Serialize(arc);
	arc.SerializeClassVersion("FxMorphTargetNode");
}

/** 2012 0xb23810 (class), 0xb108f0 (ConstructObject). */
FX_IMPLEMENT_CLASS(FxBonePoseNode, 0, FxFaceGraphNode)

/** 2012 0xb238a0. */
FxBonePoseNode::FxBonePoseNode()
{
}

/** 2012 0xb23910. */
FxBonePoseNode::~FxBonePoseNode()
{
}

/** 2012 0xb23970. */
FxFaceGraphNode* FxBonePoseNode::Clone()
{
	FxBonePoseNode* pNode = new FxBonePoseNode();
	CopyData(pNode);
	return pNode;
}

/** 2012 0xb23a00. */
void FxBonePoseNode::CopyData( FxFaceGraphNode* pOther )
{
	if( !pOther )
	{
		return;
	}
	FxFaceGraphNode::CopyData(pOther);
	FxBonePoseNode* pBonePoseNode = FxCast<FxBonePoseNode>(pOther);
	if( pBonePoseNode )
	{
		pBonePoseNode->_bones = _bones;
	}
}

/** 2012 0xb23a50: the rotation is normalised on the way in. */
void FxBonePoseNode::AddBone( const FxBone& bone )
{
	FxBone temp(bone);
	FxQuat rot = temp.GetRot();
	rot.Normalize();
	temp.SetRot(rot);
	_bones.PushBack(temp);
}

/** 2012 0xb23af0. */
void FxBonePoseNode::RemoveBone( FxSize index )
{
	_bones.Remove(index);
}

/** 2012 0xb23b10. */
void FxBonePoseNode::RemoveAllBones()
{
	_bones.Clear();
}

/** 2012 0xb23b30. */
void FxBonePoseNode::Serialize( FxArchive& arc )
{
	FxFaceGraphNode::Serialize(arc);
	arc.SerializeClassVersion("FxBonePoseNode");
	arc << _bones;
}

/** 2012 0xb25c50 (class), 0xb26390 (ConstructObject). */
FX_IMPLEMENT_CLASS(FUnrealFaceFXNode, 0, FxGenericTargetNode)

/** 2012 0xb25ce0. */
FUnrealFaceFXNode::FUnrealFaceFXNode()
	: _shouldLink(FxFalse)
{
}

/** 2012 0xb25d10. */
FUnrealFaceFXNode::~FUnrealFaceFXNode()
{
}

/** 2012 0xb25d30. */
FxFaceGraphNode* FUnrealFaceFXNode::Clone()
{
	FUnrealFaceFXNode* pNode = new FUnrealFaceFXNode();
	CopyData(pNode);
	return pNode;
}

/** 2012 0xb25dc0. */
void FUnrealFaceFXNode::CopyData( FxFaceGraphNode* pOther )
{
	FxGenericTargetNode::CopyData(pOther);
}

/** 2012 0xb25de0. */
void FUnrealFaceFXNode::SetShouldLink( FxBool shouldLink )
{
	_shouldLink = shouldLink;
}

/** 2012 0xb25e00: a changed property makes the engine re-resolve its target. */
void FUnrealFaceFXNode::ReplaceUserProperty( const FxFaceGraphNodeUserProperty& userProperty )
{
	FxFaceGraphNode::ReplaceUserProperty(userProperty);
	_shouldLink = FxTrue;
}

/** 2012 0xb25e20. */
void FUnrealFaceFXNode::Serialize( FxArchive& arc )
{
	FxGenericTargetNode::Serialize(arc);
	arc.SerializeClassVersion("FUnrealFaceFXNode");
}

/** 2012 0xb25e50 (class), 0xb10bf0 (ConstructObject). */
FX_IMPLEMENT_CLASS(FUnrealFaceFXMorphNode, 0, FUnrealFaceFXNode)

/** 2012 0xb25ee0: one string property, "Target Name" = "Target". */
FUnrealFaceFXMorphNode::FUnrealFaceFXMorphNode()
{
	FxFaceGraphNodeUserProperty targetNameProperty(UPT_String);
	targetNameProperty.SetName(FxName("Target Name"));
	targetNameProperty.SetStringProperty(FxString("Target"));
	AddUserProperty(targetNameProperty);
}

/** 2012 0xb25fc0. */
FUnrealFaceFXMorphNode::~FUnrealFaceFXMorphNode()
{
}

/** 2012 0xb25fe0. */
FxFaceGraphNode* FUnrealFaceFXMorphNode::Clone()
{
	FUnrealFaceFXMorphNode* pNode = new FUnrealFaceFXMorphNode();
	CopyData(pNode);
	return pNode;
}

/** 2012 0xb26070. */
void FUnrealFaceFXMorphNode::CopyData( FxFaceGraphNode* pOther )
{
	FUnrealFaceFXNode::CopyData(pOther);
}

/** 2012 0xb26090. */
void FUnrealFaceFXMorphNode::Serialize( FxArchive& arc )
{
	FUnrealFaceFXNode::Serialize(arc);
	arc.SerializeClassVersion("FUnrealFaceFXMorphNode");
}

/** 2012 0xb260c0 (class), 0xb10c70 (ConstructObject). */
FX_IMPLEMENT_CLASS(FUnrealFaceFXMaterialParameterNode, 0, FUnrealFaceFXNode)

/** 2012 0xb26150: "Material Slot Id" = 0 and "Parameter Name" = "Param". */
FUnrealFaceFXMaterialParameterNode::FUnrealFaceFXMaterialParameterNode()
{
	FxFaceGraphNodeUserProperty materialSlotProperty(UPT_Integer);
	materialSlotProperty.SetName(FxName("Material Slot Id"));
	materialSlotProperty.SetIntegerProperty(0);
	AddUserProperty(materialSlotProperty);
	FxFaceGraphNodeUserProperty materialParameterNameProperty(UPT_String);
	materialParameterNameProperty.SetName(FxName("Parameter Name"));
	materialParameterNameProperty.SetStringProperty(FxString("Param"));
	AddUserProperty(materialParameterNameProperty);
}

/** 2012 0xb26290. */
FUnrealFaceFXMaterialParameterNode::~FUnrealFaceFXMaterialParameterNode()
{
}

/** 2012 0xb262b0. */
FxFaceGraphNode* FUnrealFaceFXMaterialParameterNode::Clone()
{
	FUnrealFaceFXMaterialParameterNode* pNode = new FUnrealFaceFXMaterialParameterNode();
	CopyData(pNode);
	return pNode;
}

/** 2012 0xb26340. */
void FUnrealFaceFXMaterialParameterNode::CopyData( FxFaceGraphNode* pOther )
{
	FUnrealFaceFXNode::CopyData(pOther);
}

/** 2012 0xb26360. */
void FUnrealFaceFXMaterialParameterNode::Serialize( FxArchive& arc )
{
	FUnrealFaceFXNode::Serialize(arc);
	arc.SerializeClassVersion("FUnrealFaceFXMaterialParameterNode");
}

} // namespace Face
} // namespace OC3Ent
