// FaceFX 1.7.3.1: FxFaceGraph (fxfacegraph.h), the editable node graph. Agent FX2, wave 26.
// The runtime never evaluates this graph: archives carry the compiled graph (FxCompiledFaceGraph.h) and FxActor
// decompiles it only on request. Iterator/Begin/End are compiled into FxMasterBoneList.obj (2012 0xb0f1e0-0xb0f3a0).
#ifndef FxFaceGraph_H__
#define FxFaceGraph_H__

#include "FxFaceGraphNode.h"

namespace OC3Ent
{
namespace Face
{

class FxFaceGraph : public FxObject
{
	FX_DECLARE_CLASS(FxFaceGraph, FxObject)
public:
	class Iterator
	{
	public:
		explicit Iterator( FxFaceGraphNode* const* fgNodeListIter = 0 ) : _fgNodeListIter(fgNodeListIter) {}
		/** 2012 0xb0f1e0. */
		Iterator& operator++() { ++_fgNodeListIter; return *this; }
		FxBool operator==( const Iterator& other ) const { return _fgNodeListIter == other._fgNodeListIter; }
		FxBool operator!=( const Iterator& other ) const { return _fgNodeListIter != other._fgNodeListIter; }
		/** 2012 0xb0f200. */
		FxFaceGraphNode* GetNode() const { return *_fgNodeListIter; }

	private:
		FxFaceGraphNode* const* _fgNodeListIter;
	};

	class FxFaceGraphNodeJumpList
	{
	public:
		FxFaceGraphNodeJumpList( const FxClass* nodeType = 0, FxFaceGraphNode* pNode = 0 );
		~FxFaceGraphNodeJumpList();

		const FxClass* GetNodeType() const { return _nodeType; }
		void SetNodeType( const FxClass* nodeType );
		/** 2012 0xb20f50 (folded with FxFaceGraph::GetNumNodes). */
		FxSize GetNumNodes() const { return _jumpList.Length(); }
		FxBool FindNode( FxFaceGraphNode* pNode ) const;
		FxBool AddNode( FxFaceGraphNode* pNode );
		FxBool RemoveNode( FxFaceGraphNode* pNode );
		void Reserve( FxSize numNodes );
		/** 2012 0xb0f2e0. */
		FxFaceGraphNode* const* Begin() const { return _jumpList.Begin(); }
		/** 2012 0xb0f3a0. */
		FxFaceGraphNode* const* End() const { return _jumpList.End(); }

	private:
		const FxClass* _nodeType;
		FxArray<FxFaceGraphNode*> _jumpList;
	};

	FxFaceGraph();
	FxFaceGraph( const FxFaceGraph& other );
	FxFaceGraph& operator=( const FxFaceGraph& other );
	virtual ~FxFaceGraph();

	void Clear();
	FxBool AddNode( FxFaceGraphNode* pNode );
	FxBool RemoveNode( const FxName& name );
	FxFaceGraphNode* FindNode( const FxName& name );
	FxBool RenameNode( const FxName& currName, FxName newName );
	/** 2012 0xb20f50. */
	FxSize GetNumNodes() const { return _nodes.Length(); }
	FxSize CountNodes( const FxClass* fgNodeClass );

	/** 2012 0xb0f220. */
	Iterator Begin( const FxClass* fgNodeClass ) const
	{
		if( fgNodeClass == FxFaceGraphNode::StaticClass() )
		{
			return Iterator(_nodes.Begin());
		}
		FxSize jumpListIndex = _findNodeType(fgNodeClass);
		if( jumpListIndex == FxInvalidIndex )
		{
			return Iterator(_nodes.End());
		}
		return Iterator(_jumpLists[jumpListIndex].Begin());
	}
	/** 2012 0xb0f300. */
	Iterator End( const FxClass* fgNodeClass ) const
	{
		if( fgNodeClass == FxFaceGraphNode::StaticClass() )
		{
			return Iterator(_nodes.End());
		}
		FxSize jumpListIndex = _findNodeType(fgNodeClass);
		if( jumpListIndex == FxInvalidIndex )
		{
			return Iterator(_nodes.End());
		}
		return Iterator(_jumpLists[jumpListIndex].End());
	}

	FxLinkErrorCode Link( const FxName& toNode, const FxName& fromNode, const FxName& linkFn, const FxLinkFnParameters& linkFnParams );
	FxBool Unlink( const FxName& firstNodeName, const FxName& secondNodeName );
	FxBool GetLink( const FxName& firstNodeName, const FxName& secondNodeName, FxFaceGraphNodeLink& linkInfo );
	FxBool SetLink( const FxName& firstNodeName, const FxName& secondNodeName, const FxFaceGraphNodeLink& linkInfo );
	FxBool DetermineLinkDirection( const FxName& firstNodeName, const FxName& secondNodeName, FxFaceGraphNode*& pInputNode, FxFaceGraphNode*& pOutputNode );

	void ChangeNodeType( const FxName& nodeToChangeTypeOf, const FxClass* pNewTypeClassDesc );
	void NotifyRenamed( const FxChar* oldName, const FxChar* newName );

	virtual void Serialize( FxArchive& arc );

private:
	FxFaceGraphNode* _findNode( const FxName& name );
	void _preRemoveNode( FxFaceGraphNode* nodeToDelete );
	void _relink( FxBool shouldClearOutputs );
	void _createJumpLists( FxBool shouldClearJumpLists );
	FxBool _hasCycles();
	FxSize _findNodeType( const FxClass* nodeType ) const;
	void _addNodeType( FxFaceGraphNode* pNode );
	void _clone( const FxFaceGraph& other );

	FxArray<FxFaceGraphNode*> _nodes;
	FxStringHash<FxFaceGraphNode*, 11, FxArray>* _pNodeHash;
	FxArray<FxFaceGraphNodeJumpList> _jumpLists;
};

} // namespace Face
} // namespace OC3Ent

#endif
