// DishonoredGame/src/disconv_tryfallbacktree.cpp
// DISHONORED(port): agent FR (PHASE17 FR). PlayNode_Derived 2012 rva 0x8f8b60 (29 B). GetClassVars_Derived 0x8f8b80 and
// GetActionText 0x8ff120 are the base action's / editor's.
#include "DishonoredGame.h"

INT UDisConv_TryFallbackTree::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	_pRunningInstance->m_bTryFallbackTree = TRUE;
	return -1;
}
