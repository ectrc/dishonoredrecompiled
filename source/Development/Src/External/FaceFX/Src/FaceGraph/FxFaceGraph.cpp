// FaceFX 1.7.3.1: FxFaceGraph (fxfacegraph.cpp). Agent FX2, wave 26, ported from the 2012 decompile.
// Nodes are owned by the graph and freed through their class size (FxFree(node, GetClassDesc()->GetSize())); the
// name hash (2048 bins) is created on the first AddNode or non-empty load. Jump lists group the nodes by exact class.
#include "FxFaceGraph.h"

namespace OC3Ent
{
namespace Face
{

/** 2012 0xb1f230 (AllocateStaticClassFxFaceGraph), 0xb20d20 (ConstructObject). */
FX_IMPLEMENT_CLASS(FxFaceGraph, 1, FxObject)

/** 2012 0xb1f2c0. */
FxFaceGraph::FxFaceGraphNodeJumpList::FxFaceGraphNodeJumpList( const FxClass* nodeType, FxFaceGraphNode* pNode )
	: _nodeType(nodeType)
{
	if( pNode )
	{
		_jumpList.PushBack(pNode);
	}
}

/** 2012 0xb1f340. */
FxFaceGraph::FxFaceGraphNodeJumpList::~FxFaceGraphNodeJumpList()
{
}

/** 2012 0xb1f360. */
void FxFaceGraph::FxFaceGraphNodeJumpList::SetNodeType( const FxClass* nodeType )
{
	_nodeType = nodeType;
}

/** 2012 0xb1f380. */
FxBool FxFaceGraph::FxFaceGraphNodeJumpList::FindNode( FxFaceGraphNode* pNode ) const
{
	return _jumpList.Find(pNode) != FxInvalidIndex;
}

/** 2012 0xb1f3b0. */
FxBool FxFaceGraph::FxFaceGraphNodeJumpList::AddNode( FxFaceGraphNode* pNode )
{
	if( pNode->IsA(_nodeType) && !FindNode(pNode) )
	{
		_jumpList.PushBack(pNode);
		return FxTrue;
	}
	return FxFalse;
}

/** 2012 0xb1f410. */
FxBool FxFaceGraph::FxFaceGraphNodeJumpList::RemoveNode( FxFaceGraphNode* pNode )
{
	if( !pNode->IsA(_nodeType) )
	{
		return FxFalse;
	}
	FxSize index = _jumpList.Find(pNode);
	if( index == FxInvalidIndex )
	{
		return FxFalse;
	}
	_jumpList.Remove(index);
	return FxTrue;
}

/** 2012 0xb1f470. */
void FxFaceGraph::FxFaceGraphNodeJumpList::Reserve( FxSize numNodes )
{
	_jumpList.Clear();
	_jumpList.Reserve(numNodes);
}

/** 2012 0xb1f4a0. */
FxFaceGraph::FxFaceGraph()
	: _pNodeHash(0)
{
}

/** 2012 0xb1f520. */
FxFaceGraph::FxFaceGraph( const FxFaceGraph& other )
	: FxObject()
	, _pNodeHash(0)
{
	_clone(other);
}

/** 2012 0xb1f5c0. */
FxFaceGraph& FxFaceGraph::operator=( const FxFaceGraph& other )
{
	if( this == &other )
	{
		return *this;
	}
	delete _pNodeHash;
	_pNodeHash = 0;
	_clone(other);
	return *this;
}

/** 2012 0xb1f640. */
FxFaceGraph::~FxFaceGraph()
{
	Clear();
	if( _pNodeHash )
	{
		delete _pNodeHash;
		_pNodeHash = 0;
	}
}

/** 2012 0xb1f700. */
void FxFaceGraph::Clear()
{
	FxSize numNodes = _nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxFaceGraphNode* pNode = _nodes[i];
		FxSize size = pNode->GetClassDesc()->GetSize();
		pNode->~FxFaceGraphNode();
		FxFree(pNode, size);
	}
	_nodes.Clear();
	if( _pNodeHash )
	{
		_pNodeHash->Clear();
	}
	_jumpLists.Clear();
}

/** 2012 0xb1f7c0. */
FxBool FxFaceGraph::AddNode( FxFaceGraphNode* pNode )
{
	if( !_pNodeHash )
	{
		_pNodeHash = new FxStringHash<FxFaceGraphNode*, 11, FxArray>();
	}
	if( !pNode || _findNode(pNode->GetName()) )
	{
		return FxFalse;
	}
	_nodes.PushBack(pNode);
	_pNodeHash->Insert(pNode->GetNameAsCstr(), pNode);
	_addNodeType(pNode);
	FxSize jumpListIndex = _findNodeType(pNode->GetClassDesc());
	if( jumpListIndex != FxInvalidIndex )
	{
		_jumpLists[jumpListIndex].AddNode(pNode);
	}
	return FxTrue;
}

/** 2012 0xb1f8f0. */
FxBool FxFaceGraph::RemoveNode( const FxName& name )
{
	FxFaceGraphNode* nodeToDelete = _findNode(name);
	if( !nodeToDelete )
	{
		return FxFalse;
	}
	_preRemoveNode(nodeToDelete);
	_pNodeHash->Remove(nodeToDelete->GetNameAsCstr());
	FxSize index = _nodes.Find(nodeToDelete);
	FxSize size = nodeToDelete->GetClassDesc()->GetSize();
	nodeToDelete->~FxFaceGraphNode();
	FxFree(nodeToDelete, size);
	_nodes.Remove(index);
	return FxTrue;
}

/** 2012 0xb1f9b0. */
FxFaceGraphNode* FxFaceGraph::FindNode( const FxName& name )
{
	return _findNode(name);
}

/** 2012 0xb1f9d0: the outputs' links keep the node by pointer, so only their names change. */
FxBool FxFaceGraph::RenameNode( const FxName& currName, FxName newName )
{
	FxFaceGraphNode* pCurrNode = _findNode(currName);
	FxFaceGraphNode* pDesiredNode = _findNode(newName);
	if( !pCurrNode || pDesiredNode )
	{
		return FxFalse;
	}
	_pNodeHash->Remove(pCurrNode->GetNameAsCstr());
	pCurrNode->SetName(newName);
	_pNodeHash->Insert(pCurrNode->GetNameAsCstr(), pCurrNode);
	FxSize numOutputs = pCurrNode->GetNumOutputs();
	for( FxSize i = 0; i < numOutputs; ++i )
	{
		FxFaceGraphNode* outputNode = pCurrNode->GetOutput(i);
		FxSize numLinks = outputNode->GetNumInputLinks();
		for( FxSize j = 0; j < numLinks; ++j )
		{
			FxFaceGraphNodeLink link(outputNode->GetInputLink(j));
			if( link.GetNodeName() == currName )
			{
				link.SetNode(pCurrNode);
				outputNode->ModifyInputLink(j, link);
			}
		}
	}
	return FxTrue;
}

/** 2012 0xb1fb70. */
FxSize FxFaceGraph::CountNodes( const FxClass* fgNodeClass )
{
	if( fgNodeClass == FxFaceGraphNode::StaticClass() )
	{
		return _nodes.Length();
	}
	FxSize jumpListIndex = _findNodeType(fgNodeClass);
	if( jumpListIndex == FxInvalidIndex )
	{
		return 0;
	}
	return _jumpLists[jumpListIndex].GetNumNodes();
}

/** 2012 0xb1fbd0: toNode reads fromNode through linkFn; a link that closes a cycle is taken back out. */
FxLinkErrorCode FxFaceGraph::Link( const FxName& toNode, const FxName& fromNode, const FxName& linkFn, const FxLinkFnParameters& linkFnParams )
{
	FxFaceGraphNode* pToNode = _findNode(toNode);
	FxFaceGraphNode* pFromNode = _findNode(fromNode);
	const FxLinkFn* pLinkFn = FxLinkFn::FindLinkFunction(linkFn);
	if( !pToNode || !pFromNode || !pLinkFn )
	{
		return LEC_Invalid;
	}
	if( pToNode == pFromNode )
	{
		return LEC_Cycle;
	}
	FxSize numInputLinks = pToNode->GetNumInputLinks();
	for( FxSize i = 0; i < numInputLinks; ++i )
	{
		if( pToNode->GetInputLink(i).GetNode() == pFromNode )
		{
			return LEC_Duplicate;
		}
	}
	FxFaceGraphNodeLink newLink(fromNode, pFromNode, linkFn, pLinkFn->GetType(), linkFnParams);
	if( !pToNode->AddInputLink(newLink) )
	{
		return LEC_Refused;
	}
	pFromNode->_outputs.PushBack(pToNode);
	if( pToNode->_outputs.Length() && pFromNode->_inputs.Length() && pToNode->_hasCycles() )
	{
		pToNode->RemoveInputLink(pToNode->GetNumInputLinks() - 1);
		pFromNode->_outputs.Remove(pFromNode->_outputs.Length() - 1);
		return LEC_Cycle;
	}
	return LEC_None;
}

/** 2012 0xb1fde0. */
FxBool FxFaceGraph::Unlink( const FxName& firstNodeName, const FxName& secondNodeName )
{
	FxFaceGraphNode* pInputNode = 0;
	FxFaceGraphNode* pOutputNode = 0;
	DetermineLinkDirection(firstNodeName, secondNodeName, pInputNode, pOutputNode);
	if( !pInputNode || !pOutputNode )
	{
		return FxFalse;
	}
	FxSize numInputLinks = pInputNode->GetNumInputLinks();
	for( FxSize i = 0; i < numInputLinks; ++i )
	{
		if( pInputNode->GetInputLink(i).GetNode() != pOutputNode )
		{
			continue;
		}
		pInputNode->RemoveInputLink(i);
		FxSize numOutputs = pOutputNode->_outputs.Length();
		for( FxSize j = 0; j < numOutputs; ++j )
		{
			if( pOutputNode->_outputs[j] == pInputNode )
			{
				pOutputNode->_outputs.Remove(j);
				return FxTrue;
			}
		}
		return FxTrue;
	}
	return FxFalse;
}

/** 2012 0xb1fee0. */
FxBool FxFaceGraph::GetLink( const FxName& firstNodeName, const FxName& secondNodeName, FxFaceGraphNodeLink& linkInfo )
{
	FxFaceGraphNode* pInputNode = 0;
	FxFaceGraphNode* pOutputNode = 0;
	DetermineLinkDirection(firstNodeName, secondNodeName, pInputNode, pOutputNode);
	if( !pInputNode || !pOutputNode )
	{
		return FxFalse;
	}
	FxSize numInputLinks = pInputNode->GetNumInputLinks();
	for( FxSize i = 0; i < numInputLinks; ++i )
	{
		if( pInputNode->GetInputLink(i).GetNode() == pOutputNode )
		{
			linkInfo = pInputNode->GetInputLink(i);
			return FxTrue;
		}
	}
	return FxFalse;
}

/** 2012 0xb1ff90. */
FxBool FxFaceGraph::SetLink( const FxName& firstNodeName, const FxName& secondNodeName, const FxFaceGraphNodeLink& linkInfo )
{
	FxFaceGraphNode* pInputNode = 0;
	FxFaceGraphNode* pOutputNode = 0;
	DetermineLinkDirection(firstNodeName, secondNodeName, pInputNode, pOutputNode);
	if( !pInputNode || !pOutputNode )
	{
		return FxFalse;
	}
	FxSize numInputLinks = pInputNode->GetNumInputLinks();
	for( FxSize i = 0; i < numInputLinks; ++i )
	{
		if( pInputNode->GetInputLink(i).GetNode() == pOutputNode )
		{
			pInputNode->ModifyInputLink(i, linkInfo);
			return FxTrue;
		}
	}
	return FxFalse;
}

/** 2012 0xb20030: pInputNode is the node that reads the other; always answers false, as in retail. */
FxBool FxFaceGraph::DetermineLinkDirection( const FxName& firstNodeName, const FxName& secondNodeName, FxFaceGraphNode*& pInputNode, FxFaceGraphNode*& pOutputNode )
{
	pInputNode = 0;
	pOutputNode = 0;
	FxFaceGraphNode* pFirstNode = _findNode(firstNodeName);
	FxFaceGraphNode* pSecondNode = _findNode(secondNodeName);
	if( !pFirstNode || !pSecondNode )
	{
		return FxFalse;
	}
	FxSize numInputLinks = pFirstNode->GetNumInputLinks();
	for( FxSize i = 0; i < numInputLinks; ++i )
	{
		if( pFirstNode->GetInputLink(i).GetNode() == pSecondNode )
		{
			pInputNode = pFirstNode;
			pOutputNode = pSecondNode;
			break;
		}
	}
	FxSize numSecondInputLinks = pSecondNode->GetNumInputLinks();
	for( FxSize index = 0; index < numSecondInputLinks; ++index )
	{
		if( pSecondNode->GetInputLink(index).GetNode() == pFirstNode )
		{
			pInputNode = pSecondNode;
			pOutputNode = pFirstNode;
			return FxFalse;
		}
	}
	return FxFalse;
}

/** 2012 0xb20130: version 1 added the jump list sizes (presized on load, rebuilt by _createJumpLists). */
void FxFaceGraph::Serialize( FxArchive& arc )
{
	if( arc.IsLoading() )
	{
		Clear();
	}
	FxObject::Serialize(arc);
	FxUInt16 version = arc.SerializeClassVersion("FxFaceGraph");
	arc << _nodes;
	if( arc.IsLoading() )
	{
		FxSize numNodes = _nodes.Length();
		if( numNodes && !_pNodeHash )
		{
			_pNodeHash = new FxStringHash<FxFaceGraphNode*, 11, FxArray>();
		}
		for( FxSize i = 0; i < numNodes; ++i )
		{
			_pNodeHash->Insert(_nodes[i]->GetNameAsCstr(), _nodes[i]);
		}
	}
	if( version )
	{
		FxSize numJumpLists = _jumpLists.Length();
		arc << numJumpLists;
		if( arc.IsSaving() )
		{
			for( FxSize index = 0; index < numJumpLists; ++index )
			{
				FxSize numNodes = _jumpLists[index].GetNumNodes();
				arc << numNodes;
			}
		}
		else if( arc.IsLoading() )
		{
			_jumpLists.Reserve(numJumpLists);
			for( FxSize j = 0; j < numJumpLists; ++j )
			{
				FxSize numNodes = 0;
				arc << numNodes;
				_jumpLists.PushBack(FxFaceGraphNodeJumpList(0, 0));
				_jumpLists.Back().Reserve(numNodes);
			}
		}
	}
	if( arc.IsLoading() )
	{
		_relink(version == 0);
		_createJumpLists(version == 0);
	}
}

/** 2012 0xb203e0: the new node takes the data, the input links and the user data, then the graph relinks. */
void FxFaceGraph::ChangeNodeType( const FxName& nodeToChangeTypeOf, const FxClass* pNewTypeClassDesc )
{
	if( !pNewTypeClassDesc || !pNewTypeClassDesc->IsKindOf(FxFaceGraphNode::StaticClass()) )
	{
		return;
	}
	FxFaceGraphNode* pNode = _findNode(nodeToChangeTypeOf);
	if( !pNode )
	{
		return;
	}
	FxFaceGraphNode* pNewNode = FxCast<FxFaceGraphNode>(pNewTypeClassDesc->ConstructObject());
	if( !pNewNode )
	{
		return;
	}
	pNode->CopyData(pNewNode);
	pNewNode->_inputs = pNode->_inputs;
	pNewNode->SetUserData(pNode->GetUserData());
	FxSize numNodes = _nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		if( _nodes[i] != pNode )
		{
			continue;
		}
		FxSize size = pNode->GetClassDesc()->GetSize();
		_pNodeHash->Remove(pNode->GetNameAsCstr());
		pNode->~FxFaceGraphNode();
		FxFree(pNode, size);
		pNode = 0;
		_nodes[i] = pNewNode;
		_pNodeHash->Insert(pNewNode->GetNameAsCstr(), pNewNode);
	}
	_relink(FxTrue);
	_createJumpLists(FxTrue);
}

/** 2012 0xb20550: re-keys the hash after a node was renamed behind the graph's back. */
void FxFaceGraph::NotifyRenamed( const FxChar* oldName, const FxChar* newName )
{
	FxFaceGraphNode* pNode = 0;
	FxSize numNodes = _nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		if( _nodes[i]->GetNameAsString() == newName )
		{
			pNode = _nodes[i];
			break;
		}
	}
	if( pNode )
	{
		_pNodeHash->Remove(oldName);
		_pNodeHash->Insert(newName, pNode);
	}
}

/** 2012 0xb20600. */
FxFaceGraphNode* FxFaceGraph::_findNode( const FxName& name )
{
	FxFaceGraphNode* pRetNode = 0;
	if( _pNodeHash )
	{
		_pNodeHash->Find(name.GetAsCstr(), pRetNode);
	}
	return pRetNode;
}

/** 2012 0xb20640: unlinks every input and output, then drops the node from its jump list (and an emptied list). */
void FxFaceGraph::_preRemoveNode( FxFaceGraphNode* nodeToDelete )
{
	if( !nodeToDelete )
	{
		return;
	}
	while( nodeToDelete->_inputs.Length() )
	{
		FxFaceGraphNode* inputNode = nodeToDelete->_inputs[0].GetNode();
		if( !inputNode )
		{
			continue;
		}
		for( FxSize j = 0; j < inputNode->_outputs.Length(); ++j )
		{
			if( inputNode->_outputs[j] && Unlink(inputNode->GetName(), nodeToDelete->GetName()) == FxTrue )
			{
				--j;
			}
		}
	}
	for( FxSize index = 0; index < nodeToDelete->_outputs.Length(); ++index )
	{
		if( nodeToDelete->_outputs[index] && Unlink(nodeToDelete->_outputs[index]->GetName(), nodeToDelete->GetName()) == FxTrue )
		{
			--index;
		}
	}
	FxSize jumpListIndex = _findNodeType(nodeToDelete->GetClassDesc());
	_jumpLists[jumpListIndex].RemoveNode(nodeToDelete);
	if( !_jumpLists[jumpListIndex].GetNumNodes() )
	{
		_jumpLists.Remove(jumpListIndex);
	}
}

/** 2012 0xb207e0: resolves every link's node by name and rebuilds the output lists. */
void FxFaceGraph::_relink( FxBool shouldClearOutputs )
{
	FxSize numNodes = _nodes.Length();
	if( shouldClearOutputs )
	{
		for( FxSize i = 0; i < numNodes; ++i )
		{
			if( _nodes[i] )
			{
				_nodes[i]->_outputs.Clear();
			}
		}
	}
	for( FxSize index = 0; index < numNodes; ++index )
	{
		FxFaceGraphNode* pNode = _nodes[index];
		if( !pNode )
		{
			continue;
		}
		FxSize numInputLinks = pNode->GetNumInputLinks();
		for( FxSize j = 0; j < numInputLinks; ++j )
		{
			FxFaceGraphNode* fromNode = _findNode(pNode->_inputs[j].GetNodeName());
			pNode->_inputs[j].SetNode(fromNode);
			if( fromNode )
			{
				fromNode->_outputs.PushBack(pNode);
			}
		}
	}
}

/** 2012 0xb20900. */
void FxFaceGraph::_createJumpLists( FxBool shouldClearJumpLists )
{
	if( shouldClearJumpLists )
	{
		_jumpLists.Clear();
	}
	FxSize numNodes = _nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		FxFaceGraphNode* pNode = _nodes[i];
		_addNodeType(pNode);
		FxSize jumpListIndex = _findNodeType(pNode->GetClassDesc());
		if( jumpListIndex != FxInvalidIndex )
		{
			_jumpLists[jumpListIndex].AddNode(pNode);
		}
	}
}

/** 2012 0xb209b0. */
FxBool FxFaceGraph::_hasCycles()
{
	FxSize numNodes = _nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		_nodes[i]->_hasBeenVisited = FxFalse;
	}
	for( FxSize index = 0; index < numNodes; ++index )
	{
		if( _nodes[index]->_hasCycles() )
		{
			return FxTrue;
		}
	}
	return FxFalse;
}

/** 2012 0xb20a40. */
FxSize FxFaceGraph::_findNodeType( const FxClass* nodeType ) const
{
	if( nodeType )
	{
		FxSize numJumpLists = _jumpLists.Length();
		for( FxSize i = 0; i < numJumpLists; ++i )
		{
			if( _jumpLists[i].GetNodeType() == nodeType )
			{
				return i;
			}
		}
	}
	return FxInvalidIndex;
}

/** 2012 0xb20ab0: a loaded, still untyped jump list takes the first new class. */
void FxFaceGraph::_addNodeType( FxFaceGraphNode* pNode )
{
	const FxClass* nodeType = pNode->GetClassDesc();
	if( _findNodeType(nodeType) != FxInvalidIndex )
	{
		return;
	}
	FxSize numJumpLists = _jumpLists.Length();
	for( FxSize i = 0; i < numJumpLists; ++i )
	{
		if( !_jumpLists[i].GetNodeType() )
		{
			_jumpLists[i].SetNodeType(nodeType);
			return;
		}
	}
	_jumpLists.PushBack(FxFaceGraphNodeJumpList(pNode->GetClassDesc(), 0));
}

/** 2012 0xb20bb0. */
void FxFaceGraph::_clone( const FxFaceGraph& other )
{
	Clear();
	FxSize numNodes = other._nodes.Length();
	for( FxSize i = 0; i < numNodes; ++i )
	{
		AddNode(other._nodes[i]->Clone());
	}
	for( FxSize index = 0; index < numNodes; ++index )
	{
		FxFaceGraphNode* pNode = other._nodes[index];
		FxSize numInputLinks = pNode->GetNumInputLinks();
		for( FxSize j = 0; j < numInputLinks; ++j )
		{
			const FxFaceGraphNodeLink& link = pNode->GetInputLink(j);
			Link(pNode->GetName(), link.GetNodeName(), link.GetLinkFnName(), link.GetLinkFnParams());
		}
	}
	_relink(FxTrue);
	_createJumpLists(FxTrue);
}

} // namespace Face
} // namespace OC3Ent
