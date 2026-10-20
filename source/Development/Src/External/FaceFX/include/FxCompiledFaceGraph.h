// FaceFX 1.7.3.1: FxCompiledFaceGraph (fxcompiledfacegraph.h), the flat, topologically ordered graph the runtime
// evaluates. Agent FX2, wave 26.
// FxRegister and FxRegister::GetUserOpForRegOp are FxActorInstance.obj's (FX3, 2012 0xb0b990 / 0xb0bc30); they are
// defined here because Tick reads and writes them.
#ifndef FxCompiledFaceGraph_H__
#define FxCompiledFaceGraph_H__

#include "FxFaceGraph.h"

namespace OC3Ent
{
namespace Face
{

enum FxCompiledFaceGraphNodeType
{
	NT_Invalid = -1,
	NT_First = 0,
	NT_Combiner = NT_First,
	NT_Delta,
	NT_CurrentTime,
	NT_GenericTarget,
	NT_BonePose,
	NT_MorphTarget,
	NT_MaterialParameterUE3,
	NT_MorphTargetUE3,
	NT_Last = NT_MorphTargetUE3,
	NT_NumNodeTypes
};

enum FxValueOp
{
	VO_Invalid = -1,
	VO_Add = 0,
	VO_Multiply,
	VO_Replace,
	VO_None
};

enum FxRegisterOp
{
	RO_Invalid = -1,
	RO_Add = 0,
	RO_Multiply,
	RO_Replace,
	RO_LoadAdd,
	RO_LoadMultiply,
	RO_LoadReplace,
	RO_None
};

struct FxRegister
{
	/** 2012 0xb0b990. */
	FxRegister()
		: value(0.0f), interpStartValue(0.0f), interpEndValue(0.0f), interpNextEndValue(0.0f), interpLastValue(0.0f)
		, interpInverseDuration(0.0f), interpNextInverseDuration(0.0f), interpStartTime(static_cast<FxReal>(FX_INT32_MIN))
		, scratch(0.0f), firstRegOp(VO_None), nextRegOp(RO_None), isInterpolating(FxFalse) {}

	/** 2012 0xb0bc30. */
	static FxValueOp GetUserOpForRegOp( FxRegisterOp regOp )
	{
		switch( regOp )
		{
		case RO_Add:
		case RO_LoadAdd:
			return VO_Add;
		case RO_Multiply:
		case RO_LoadMultiply:
			return VO_Multiply;
		case RO_Replace:
		case RO_LoadReplace:
			return VO_Replace;
		default:
			return VO_None;
		}
	}

	FxReal value;
	FxReal interpStartValue;
	FxReal interpEndValue;
	FxReal interpNextEndValue;
	FxReal interpLastValue;
	FxReal interpInverseDuration;
	FxReal interpNextInverseDuration;
	FxReal interpStartTime;
	FxReal scratch;
	FxValueOp firstRegOp;
	FxRegisterOp nextRegOp;
	FxBool isInterpolating;
};

struct FxCompiledFaceGraphLink
{
	FxCompiledFaceGraphLink();
	FxCompiledFaceGraphLink( FxSize iNodeIndex, FxLinkFnType iLinkFnType, const FxLinkFnParameters& iLinkFnParams );

	FxSize nodeIndex;
	FxLinkFnType linkFnType;
	FxLinkFnParameters linkFnParams;
};

FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraphLink& link );

struct FxCompiledFaceGraph;

struct FxCompiledFaceGraphNode
{
	FxCompiledFaceGraphNode();
	FxCompiledFaceGraphNode( const FxFaceGraphNode* pNode, const FxCompiledFaceGraph* pCG );

	FxFaceGraphNode* Decompile() const;

	FxCompiledFaceGraphNodeType nodeType;
	FxName name;
	FxReal nodeMin;
	FxReal oneOverNodeMin;
	FxReal nodeMax;
	FxReal oneOverNodeMax;
	FxReal trackValue;
	FxReal userValue;
	FxReal finalValue;
	FxInputOp inputOperation;
	FxValueOp userValueOperation;
	void* pUserData;
	FxArray<FxCompiledFaceGraphLink> inputLinks;
	FxArray<FxFaceGraphNodeUserProperty> userProperties;

private:
	FxCompiledFaceGraphNodeType _determineNodeType( const FxFaceGraphNode* pNode ) const;
};

FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraphNode& node );

struct FxNodeTypeLookupTableEntry
{
	FxCompiledFaceGraphNodeType nodeType;
	const FxChar* nodeClass;
	FxObject* (*constructNodeFunc)();
};

extern const FxNodeTypeLookupTableEntry FxNodeTypeLookupTable[NT_NumNodeTypes];

typedef void (*FxPreDestroyCallbackFunc)( FxCompiledFaceGraph& );
typedef void (*FxCompilationCallbackFunc)( FxCompiledFaceGraph&, FxFaceGraph& );

struct FxCompiledFaceGraph
{
	FxCompiledFaceGraph();
	~FxCompiledFaceGraph();

	static void SetPreDestroyCallback( FxPreDestroyCallbackFunc preDestroy );
	static void SetCompilationCallbacks( FxCompilationCallbackFunc preCompile, FxCompilationCallbackFunc postCompile,
		FxCompilationCallbackFunc preDecompile, FxCompilationCallbackFunc postDecompile );
	static FxBool IsNodeTypeRelevantForCollapse( FxCompiledFaceGraphNodeType nodeType );

	void Clear();
	void Tick( FxReal time, FxArray<FxRegister>& registers, FxBool& hasBeenTicked );
	void Compile( FxFaceGraph& faceGraph );
	void Decompile( FxFaceGraph& faceGraph );
	FxSize FindNodeIndex( const FxName& name ) const;
	FxSize CountNodesOfType( FxCompiledFaceGraphNodeType nodeType ) const;

	FxArray<FxCompiledFaceGraphNode> nodes;

private:
	friend FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraph& cg );

	void _compileNode( const FxFaceGraphNode* pNode, FxArray<const FxFaceGraphNode*>& traversal, FxStringHash<FxSize, 11, FxArray>& traversalHash );

	FxStringHash<FxSize, 11, FxArray> _nodeHash;
};

FxArchive& operator<<( FxArchive& arc, FxCompiledFaceGraph& cg );

} // namespace Face
} // namespace OC3Ent

#endif
