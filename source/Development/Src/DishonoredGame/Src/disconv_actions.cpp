// DishonoredGame/src/disconv_actions.cpp
// DISHONORED(port): agent FR (PHASE17 FR). UDisConv_Action::GetClassVars_Derived 2012 rva 0x8f8a30 (43 B): an action
// is a node a dialog can start on. GetActionText 0x8fbd10 is editor-only (agentFR.md inventory).
#include "DishonoredGame.h"

void UDisConv_Action::GetClassVars_Derived( FDisConvClassVars& _rVarsOut ) const
{
	UDisConv_Node::GetClassVars_Derived( _rVarsOut );
	_rVarsOut.m_bIsStartOfDialog = TRUE;
}
