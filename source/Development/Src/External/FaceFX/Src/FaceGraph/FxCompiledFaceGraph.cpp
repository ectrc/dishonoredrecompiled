// FaceFX 1.7.3.1: FxCompiledFaceGraph (fxcompiledfacegraph.cpp) - compile/decompile and Tick, the per-frame face
// graph evaluation. Agent FX2, wave 26, ported from the 2012 decompile.
// Compile orders the nodes so every node's inputs come before it: first the nodes with neither inputs nor outputs,
// then for each node without outputs a depth-first post-order walk of its inputs. Tick then evaluates in array order.
// Deviation: retail's x87 build rounds the float arithmetic through double; this build stays in single precision.
#include "FxCompiledFaceGraph.h"
#include "FxCombinerNode.h"
#include "FxDeltaNode.h"
#include "FxCurrentTimeNode.h"
#include "FxGenericTargetNode.h"
#include "FxBonePoseNode.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xdc5fc8 (data). */
const FxNodeTypeLookupTableEntry FxNodeTypeLookupTable[NT_NumNodeTypes] =
{
	{ NT_Combiner, "FxCombinerNode", FxCombinerNode::ConstructObject },
	{ NT_Delta, "FxDeltaNode", FxDeltaNode::ConstructObject },
	{ NT_CurrentTime, "FxCurrentTimeNode", FxCurrentTimeNode::ConstructObject },
	{ NT_GenericTarget, "FxGenericTargetNode", FxGenericTargetNode::ConstructObject },
	{ NT_BonePose, "FxBonePoseNode", FxBonePoseNode::ConstructObject },
	{ NT_MorphTarget, "FxMorphTargetNode", FxMorphTargetNode::ConstructObject },
	{ NT_MaterialParameterUE3, "FUnrealFaceFXMaterialParameterNode", FUnrealFaceFXMaterialParameterNode::ConstructObject },
	{ NT_MorphTargetUE3, "FUnrealFaceFXMorphNode", FUnrealFaceFXMorphNode::ConstructObject }
};

/** 2012 0xdc6028 (data): only the target node types survive a collapse. */
static const struct { FxCompiledFaceGraphNodeType nodeType; FxBool bIsRelevantForCollapse; } FxNodeRelevanceLookupTable[NT_NumNodeTypes] =
{
	{ NT_Combiner, FxFalse },
	{ NT_Delta, FxFalse },
	{ NT_CurrentTime, FxFalse },
	{ NT_GenericTarget, FxFalse },
	{ NT_BonePose, FxTrue },
	{ NT_MorphTarget, FxTrue },
	{ NT_MaterialParameterUE3, FxTrue },
	{ NT_MorphTargetUE3, FxTrue }
};

/** 2012 0xdc607c (data): the identity of each input operation. */
static const FxReal initialValueLookup[] = { 0.0f, 1.0f, -FX_REAL_MAX, FX_REAL_MAX };

/** 2012 0xb112f0. */
static void FxDefaultPreDestroyCallbackFunc( FxCompiledFaceGraph& )
{
}

/** 2012 0xb11300. */
static void FxDefaultPreCompileCallbackFunc( FxCompiledFaceGraph&, FxFaceGraph& )
{
}

/** 2012 0xb11310. */
static void FxDefaultPostCompileCallbackFunc( FxCompiledFaceGraph&, FxFaceGraph& )
{
}

/** 2012 0xb11320. */
static void FxDefaultPreDecompileCallbackFunc( FxCompiledFaceGraph&, FxFaceGraph& )
{
}

/** 2012 0xb11330. */
static void FxDefaultPostDecompileCallbackFunc( FxCompiledFaceGraph&, FxFaceGraph& )
{
}

static FxPreDestroyCallbackFunc preDestroy = FxDefaultPreDestroyCallbackFunc;
static FxCompilationCallbackFunc preCompile = FxDefaultPreCompileCallbackFunc;
static FxCompilationCallbackFunc postCompile = FxDefaultPostCompileCallbackFunc;
static FxCompilationCallbackFunc preDecompile = FxDefaultPreDecompileCallbackFunc;
static FxCompilationCallbackFunc postDecompile = FxDefaultPostDecompileCallbackFunc;

/** 2012 0xb10cf0. */
FxCompiledFaceGraphLink::FxCompiledFaceGraphLink()
	: nodeIndex(FxInvalidIndex)
	, linkFnType(LFT_Invalid)
{
}

/** 2012 0xb10d20. */
FxCompiledFaceGraphLink::FxCompiledFaceGraphLink( FxSize iNodeIndex, FxLinkFnType iLinkFnType, const FxLinkFnParameters& iLinkFnParams )
	: nodeIndex(iNodeIndex)
	, linkFnType(iLinkFnType)
	, linkFnParams(iLinkFnParams)
{
}

/** 2012 0xb10d50. */
FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraphLink& link )
{
	FxInt32 tempLinkFnType = link.linkFnType;
	arc << link.nodeIndex << tempLinkFnType << link.linkFnParams;
	link.linkFnType = static_cast<FxLinkFnType>(tempLinkFnType);
	if( link.linkFnType == LFT_Null )
	{
		link.linkFnParams.parameters.Clear();
	}
	return arc;
}

/** 2012 0xb10db0. */
FxCompiledFaceGraphNode::FxCompiledFaceGraphNode()
	: nodeType(NT_Invalid)
	, name(FxName::NullName)
	, nodeMin(0.0f)
	, oneOverNodeMin(0.0f)
	, nodeMax(0.0f)
	, oneOverNodeMax(0.0f)
	, trackValue(0.0f)
	, userValue(0.0f)
	, finalValue(0.0f)
	, inputOperation(IO_Invalid)
	, userValueOperation(VO_Invalid)
	, pUserData(0)
{
}

/** 2012 0xb10e90: the inputs must already be in pCG (Compile's order guarantees it). */
FxCompiledFaceGraphNode::FxCompiledFaceGraphNode( const FxFaceGraphNode* pNode, const FxCompiledFaceGraph* pCG )
{
	nodeType = _determineNodeType(pNode);
	name = pNode->GetName();
	nodeMin = pNode->GetMin();
	oneOverNodeMin = (nodeMin > 0.0f || nodeMin < 0.0f) ? 1.0f / nodeMin : 1.0f;
	nodeMax = pNode->GetMax();
	oneOverNodeMax = (nodeMax > 0.0f || nodeMax < 0.0f) ? 1.0f / nodeMax : 1.0f;
	finalValue = 0.0f;
	userValue = 0.0f;
	trackValue = 0.0f;
	inputOperation = pNode->GetNodeOperation();
	userValueOperation = VO_None;
	pUserData = 0;
	FxSize numUserProperties = pNode->GetNumUserProperties();
	userProperties.Reserve(numUserProperties);
	for( FxSize i = 0; i < numUserProperties; ++i )
	{
		userProperties.PushBack(pNode->GetUserProperty(i));
	}
	FxSize numInputLinks = pNode->GetNumInputLinks();
	inputLinks.Reserve(numInputLinks);
	for( FxSize index = 0; index < numInputLinks; ++index )
	{
		const FxFaceGraphNodeLink& link = pNode->GetInputLink(index);
		FxName inputName(link.GetNodeName().GetAsCstr());
		inputLinks.PushBack(FxCompiledFaceGraphLink(pCG->FindNodeIndex(inputName), link.GetLinkFnType(), link.GetLinkFnParams()));
	}
}

/** 2012 0xb11120: a fresh node of the type's class with the data and user properties, no links. */
FxFaceGraphNode* FxCompiledFaceGraphNode::Decompile() const
{
	FxFaceGraphNode* pDecompiledNode = FxCast<FxFaceGraphNode>(FxNodeTypeLookupTable[nodeType].constructNodeFunc());
	pDecompiledNode->SetName(name);
	pDecompiledNode->SetMin(nodeMin);
	pDecompiledNode->SetMax(nodeMax);
	pDecompiledNode->SetNodeOperation(inputOperation);
	FxSize numUserProperties = userProperties.Length();
	for( FxSize i = 0; i < numUserProperties; ++i )
	{
		pDecompiledNode->ReplaceUserProperty(userProperties[i]);
	}
	return pDecompiledNode;
}

/** 2012 0xb111d0. */
FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraphNode& node )
{
	FxInt32 tempNodeType = node.nodeType;
	FxInt32 tempInputOp = node.inputOperation;
	arc << tempNodeType << node.name << node.nodeMin << node.oneOverNodeMin << node.nodeMax << node.oneOverNodeMax
		<< tempInputOp << node.inputLinks << node.userProperties;
	node.nodeType = static_cast<FxCompiledFaceGraphNodeType>(tempNodeType);
	node.inputOperation = static_cast<FxInputOp>(tempInputOp);
	return arc;
}

/** 2012 0xb11280: by class name, so subclasses outside the table are NT_Invalid. */
FxCompiledFaceGraphNodeType FxCompiledFaceGraphNode::_determineNodeType( const FxFaceGraphNode* pNode ) const
{
	const FxName& nodeClassName = pNode->GetClassDesc()->GetName();
	for( FxInt32 i = 0; i < NT_NumNodeTypes; ++i )
	{
		if( nodeClassName == FxName(FxNodeTypeLookupTable[i].nodeClass) )
		{
			return FxNodeTypeLookupTable[i].nodeType;
		}
	}
	return NT_Invalid;
}

/** 2012 0xb11340. */
FxCompiledFaceGraph::FxCompiledFaceGraph()
{
}

/** 2012 0xb113a0. */
FxCompiledFaceGraph::~FxCompiledFaceGraph()
{
	preDestroy(*this);
	Clear();
}

/** 2012 0xb11410. */
void FxCompiledFaceGraph::SetPreDestroyCallback( FxPreDestroyCallbackFunc preDestroyFunc )
{
	if( preDestroyFunc )
	{
		preDestroy = preDestroyFunc;
	}
}

/** 2012 0xb11430. */
void FxCompiledFaceGraph::SetCompilationCallbacks( FxCompilationCallbackFunc preCompileFunc, FxCompilationCallbackFunc postCompileFunc,
	FxCompilationCallbackFunc preDecompileFunc, FxCompilationCallbackFunc postDecompileFunc )
{
	if( preCompileFunc )
	{
		preCompile = preCompileFunc;
	}
	if( postCompileFunc )
	{
		postCompile = postCompileFunc;
	}
	if( preDecompileFunc )
	{
		preDecompile = preDecompileFunc;
	}
	if( postDecompileFunc )
	{
		postDecompile = postDecompileFunc;
	}
}

/** 2012 0xb11470. */
FxBool FxCompiledFaceGraph::IsNodeTypeRelevantForCollapse( FxCompiledFaceGraphNodeType nodeType )
{
	return FxNodeRelevanceLookupTable[nodeType].bIsRelevantForCollapse;
}

/** 2012 0xb11480. */
void FxCompiledFaceGraph::Clear()
{
	_nodeHash.Clear();
	nodes.Clear();
}

/** 2012 0xb114a0: advances each node's register interpolation, then evaluates the node in array order.
    Registers are per node (registers[i] belongs to nodes[i]); the register's value op and value become the node's
    user value for this frame. Combiner-like nodes: the input op over the linked inputs (corrective links instead
    accumulate a correction), plus the track value, then the user value op, scaled by (1 - correction) and clamped to
    [min, max]. Delta nodes output the change of their single input since the last tick; current-time nodes the time. */
void FxCompiledFaceGraph::Tick( FxReal time, FxArray<FxRegister>& registers, FxBool& hasBeenTicked )
{
	nodes.Prefetch();
	registers.Prefetch();
	FxSize numNodes = nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxCompiledFaceGraphNode& node = nodes[i];
		FxRegister& reg = registers[i];
		if( reg.firstRegOp != VO_None )
		{
			if( !reg.isInterpolating )
			{
				reg.interpStartTime = time;
				reg.isInterpolating = FxTrue;
			}
			if( FxRealEquality(reg.interpInverseDuration, 0.0f) )
			{
				reg.interpLastValue = reg.interpEndValue;
				if( time - reg.interpStartTime > 0.0f && reg.nextRegOp != RO_Invalid )
				{
					reg.interpStartTime = time;
					reg.isInterpolating = FxTrue;
					if( reg.nextRegOp == RO_LoadAdd || reg.nextRegOp == RO_LoadMultiply || reg.nextRegOp == RO_LoadReplace )
					{
						reg.interpStartValue = reg.value;
					}
					else
					{
						reg.interpStartValue = reg.interpEndValue;
					}
					reg.interpEndValue = reg.interpNextEndValue;
					reg.interpInverseDuration = reg.interpNextInverseDuration;
					reg.firstRegOp = FxRegister::GetUserOpForRegOp(reg.nextRegOp);
					reg.nextRegOp = RO_Invalid;
					reg.interpNextEndValue = 0.0f;
					reg.interpNextInverseDuration = 0.0f;
				}
			}
			else
			{
				FxReal parametricTime = reg.interpInverseDuration * (time - reg.interpStartTime);
				if( reg.nextRegOp != RO_Invalid && parametricTime >= 1.0f )
				{
					reg.interpStartTime = time;
					reg.isInterpolating = FxTrue;
					if( reg.nextRegOp == RO_LoadAdd || reg.nextRegOp == RO_LoadMultiply || reg.nextRegOp == RO_LoadReplace )
					{
						reg.interpStartValue = reg.value;
					}
					else
					{
						reg.interpStartValue = reg.interpEndValue;
					}
					reg.interpEndValue = reg.interpNextEndValue;
					reg.interpInverseDuration = reg.interpNextInverseDuration;
					reg.firstRegOp = FxRegister::GetUserOpForRegOp(reg.nextRegOp);
					reg.nextRegOp = RO_Invalid;
					reg.interpNextEndValue = 0.0f;
					reg.interpNextInverseDuration = 0.0f;
					parametricTime = 0.0f;
				}
				reg.interpLastValue = FxHermiteInterpolate(reg.interpStartValue, reg.interpEndValue, parametricTime);
			}
		}
		node.userValueOperation = reg.firstRegOp;
		node.userValue = reg.interpLastValue;

		if( node.nodeType == NT_Delta )
		{
			node.finalValue = 0.0f;
			if( node.inputLinks.Length() )
			{
				const FxCompiledFaceGraphLink& inputLink = node.inputLinks[0];
				const FxCompiledFaceGraphNode& inputNode = nodes[inputLink.nodeIndex];
				FxReal currentValue = FxLinkFnEval(inputLink.linkFnType, inputNode.finalValue, inputLink.linkFnParams);
				if( hasBeenTicked )
				{
					node.finalValue = currentValue - reg.scratch;
				}
				reg.scratch = currentValue;
			}
		}
		else if( node.nodeType == NT_CurrentTime )
		{
			FxReal value = time;
			switch( node.userValueOperation )
			{
			case VO_Add:
				value = value + node.userValue;
				break;
			case VO_Multiply:
				value = value * node.userValue;
				break;
			case VO_Replace:
				value = node.userValue;
				break;
			default:
				break;
			}
			node.userValue = 0.0f;
			node.finalValue = value;
		}
		else
		{
			FxReal value = 0.0f;
			if( node.inputLinks.Length() )
			{
				value = initialValueLookup[node.inputOperation];
			}
			FxReal correction = 0.0f;
			node.inputLinks.Prefetch();
			FxSize numInputs = node.inputLinks.Length();
			for( FxSize j = 0; j < numInputs; ++j )
			{
				const FxCompiledFaceGraphLink& link = node.inputLinks[j];
				const FxCompiledFaceGraphNode& inputNode = nodes[link.nodeIndex];
				if( link.linkFnType == LFT_Corrective )
				{
					FxReal correctedValueRight = inputNode.finalValue * inputNode.oneOverNodeMax;
					FxReal correctedValueLeft = -inputNode.finalValue * inputNode.oneOverNodeMin;
					FxReal correctedValue = FxMax(correctedValueRight, correctedValueLeft);
					FxReal accumulated = correctedValue * link.linkFnParams.parameters[0] + correction;
					FxReal one = 1.0f;
					correction = FxMin(one, accumulated);
					continue;
				}
				FxReal transformedValue = FxLinkFnEval(link.linkFnType, inputNode.finalValue, link.linkFnParams);
				switch( node.inputOperation )
				{
				case IO_Sum:
					value = value + transformedValue;
					break;
				case IO_Multiply:
					value = value * transformedValue;
					break;
				case IO_Max:
					value = FxMax(transformedValue, value);
					break;
				case IO_Min:
					value = FxMin(transformedValue, value);
					break;
				default:
					break;
				}
			}
			value = value + node.trackValue;
			node.trackValue = 0.0f;
			switch( node.userValueOperation )
			{
			case VO_Add:
				value = value + node.userValue;
				break;
			case VO_Multiply:
				value = value * node.userValue;
				break;
			case VO_Replace:
				value = node.userValue;
				break;
			default:
				break;
			}
			node.userValue = 0.0f;
			value = (1.0f - correction) * value;
			node.finalValue = FxClamp(node.nodeMin, value, node.nodeMax);
			reg.value = node.finalValue;
		}
		reg.value = node.finalValue;
	}
	hasBeenTicked = FxTrue;
}

/** 2012 0xb11c60. */
void FxCompiledFaceGraph::Compile( FxFaceGraph& faceGraph )
{
	preCompile(*this, faceGraph);
	Clear();
	FxSize n = faceGraph.GetNumNodes();
	FxArray<FxFaceGraphNode*> terminalNodes;
	terminalNodes.Reserve(n);
	FxFaceGraph::Iterator end = faceGraph.End(FxFaceGraphNode::StaticClass());
	for( FxFaceGraph::Iterator node = faceGraph.Begin(FxFaceGraphNode::StaticClass()); node != end; ++node )
	{
		if( !node.GetNode()->GetNumOutputs() )
		{
			terminalNodes.PushBack(node.GetNode());
		}
	}
	nodes.Reserve(n);
	FxSize numTerminalNodes = terminalNodes.Length();
	for( FxSize index = 0; index < numTerminalNodes; ++index )
	{
		if( !terminalNodes[index]->GetNumInputLinks() )
		{
			nodes.PushBack(FxCompiledFaceGraphNode(terminalNodes[index], this));
			_nodeHash.Insert(terminalNodes[index]->GetNameAsCstr(), nodes.Length() - 1);
		}
	}
	for( FxSize i = 0; i < numTerminalNodes; ++i )
	{
		FxArray<const FxFaceGraphNode*> traversal;
		FxStringHash<FxSize, 11, FxArray> traversalHash;
		_compileNode(terminalNodes[i], traversal, traversalHash);
		FxSize numTraversed = traversal.Length();
		for( FxSize j = 0; j < numTraversed; ++j )
		{
			if( FindNodeIndex(traversal[j]->GetName()) == FxInvalidIndex )
			{
				nodes.PushBack(FxCompiledFaceGraphNode(traversal[j], this));
				_nodeHash.Insert(traversal[j]->GetNameAsCstr(), nodes.Length() - 1);
			}
		}
	}
	postCompile(*this, faceGraph);
}

/** 2012 0xb11ff0. */
void FxCompiledFaceGraph::Decompile( FxFaceGraph& faceGraph )
{
	preDecompile(*this, faceGraph);
	faceGraph.Clear();
	FxSize numNodes = nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		faceGraph.AddNode(nodes[i].Decompile());
	}
	for( FxSize index = 0; index < numNodes; ++index )
	{
		FxSize numInputs = nodes[index].inputLinks.Length();
		for( FxSize j = 0; j < numInputs; ++j )
		{
			const FxCompiledFaceGraphLink& link = nodes[index].inputLinks[j];
			const FxLinkFn* pLinkFn = FxLinkFn::FindLinkFunctionByType(link.linkFnType);
			faceGraph.Link(nodes[index].name, nodes[link.nodeIndex].name, pLinkFn->GetName(), link.linkFnParams);
		}
	}
	postDecompile(*this, faceGraph);
}

/** 2012 0xb12180. */
FxSize FxCompiledFaceGraph::FindNodeIndex( const FxName& name ) const
{
	FxSize nodeIndex = FxInvalidIndex;
	_nodeHash.Find(name.GetAsCstr(), nodeIndex);
	return nodeIndex;
}

/** 2012 0xb121c0. */
FxSize FxCompiledFaceGraph::CountNodesOfType( FxCompiledFaceGraphNodeType nodeType ) const
{
	FxSize numNodesOfType = 0;
	FxSize numNodes = nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		if( nodes[i].nodeType == nodeType )
		{
			++numNodesOfType;
		}
	}
	return numNodesOfType;
}

/** 2012 0xb12220. */
FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraph& cg )
{
	arc.SerializeClassVersion("FxCompiledFaceGraph", FxTrue);
	arc << cg.nodes << cg._nodeHash;
	return arc;
}

/** 2012 0xb12260: depth-first, inputs before the node, each node once per walk. */
void FxCompiledFaceGraph::_compileNode( const FxFaceGraphNode* pNode, FxArray<const FxFaceGraphNode*>& traversal, FxStringHash<FxSize, 11, FxArray>& traversalHash )
{
	if( traversalHash.Find(pNode->GetNameAsCstr()) )
	{
		return;
	}
	FxSize numInputs = pNode->GetNumInputLinks();
	for( FxSize i = 0; i < numInputs; ++i )
	{
		_compileNode(pNode->GetInputLink(i).GetNode(), traversal, traversalHash);
	}
	traversal.PushBack(pNode);
	traversalHash.Insert(pNode->GetNameAsCstr(), traversal.Length() - 1);
}

} // namespace Face
} // namespace OC3Ent
