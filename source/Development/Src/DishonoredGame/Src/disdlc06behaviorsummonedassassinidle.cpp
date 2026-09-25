// DishonoredGame/src/disdlc06behaviorsummonedassassinidle.cpp
// DLC06 class (2013 only, no 2012 PDB unit).

#include "DishonoredGame.h"

// DISHONORED(written): exec 2013 rva 0x5e47a0 -> vtable +424 = 2013 rva 0x8e71e0 (`m_bIsFinished = TRUE`), not overridden
void UDisDLC06BehaviorSummonedAssassinIdle::execRequestStateExitCallback_GenericAction( FFrame& Stack, RESULT_DECL )
{
	P_GET_OBJECT(UDishonoredNativeState, _pThisState);
	P_FINISH;
	m_bIsFinished = TRUE;
}
