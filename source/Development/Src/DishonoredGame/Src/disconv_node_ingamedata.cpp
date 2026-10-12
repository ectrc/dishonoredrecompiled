// DishonoredGame/src/disconv_node_ingamedata.cpp
// DISHONORED(port): agent FR (PHASE17 FR). InitForConvNode 2012 rva 0x8d9830 (24 B): the owning node, then the derived
// initialiser. GameSave / GameLoad are agent EC's in dissavegame.cpp.
#include "DishonoredGame.h"

void UDisConv_Node_InGameData::InitForConvNode( const UDisConv_Node* _pNode )
{
	m_pOwnerConvNode = const_cast<UDisConv_Node*>( _pNode );
	InitForConvNode_Derived( _pNode );
}
