// DishonoredGame/src/disconv_hook_kismetactivated.cpp
// DISHONORED(port): agent FR (PHASE17 FR). 2013 rvas (2012 in parentheses): InitNode 0x8bbbb0 (0x907010, 89),
// PlayNode_Derived 0x8ac350 (0x8fb740, 187), EvaluateForFiring 0x8a9200 (0x8f9c00, 17), MatchHooksToOutputLinks
// (0x903270, 370). 2013's PlayNode_Derived reads the hook parameters of the binding's own speaker (m_pBoundSpeaker_Me),
// not the initiator's - the Kismet action stamps m_KismetDialogHook on every Target speaker before firing, so the two
// agree; the guard for a speaker without parameters is this tree's.
#include "DishonoredGame.h"
#include "frdialogdiag.h"

void UDisConv_Hook_KismetActivated::InitNode( UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode )
{
	UDisConv_DialogHook::InitNode( _pOwner, _bForEditor, _bIsNewNode );
	MatchHooksToOutputLinks( _bForEditor );
}

INT UDisConv_Hook_KismetActivated::EvaluateForFiring( FDisDialogRunningInstance* _pRunningInstance ) const
{
	return m_KismetDialogHooks.Num() > 0 ? 0 : -1;
}

INT UDisConv_Hook_KismetActivated::PlayNode_Derived( UBOOL& _rbNotFinished, FDisDialogRunningInstance* _pRunningInstance ) const
{
	IDisConvSpeakerInterface* Me = _pRunningInstance->m_pTreeGameBinding->m_pBoundSpeaker_Me.GetObject() ? (IDisConvSpeakerInterface*)_pRunningInstance->m_pTreeGameBinding->m_pBoundSpeaker_Me.GetInterface() : NULL;
	const FDisHookParameters* Params = Me ? Me->GetHookParameters() : NULL;
	if( Params )
	{
		for( INT i = 0; i < m_KismetDialogHooks.Num(); i++ )
		{
			if( m_KismetDialogHooks(i) == Params->m_KismetDialogHook )
			{
				FRDIAG( "bind kismet hook %s: input '%s' is output %d of %d", *GetPathName(), *Params->m_KismetDialogHook.ToString(), i, m_OutputLinks.Num() );
				return i;
			}
		}
	}
	FRDIAG( "bind kismet hook %s: input '%s' matches none of %d name(s); trying the fallback tree", *GetPathName(),
		Params ? *Params->m_KismetDialogHook.ToString() : TEXT("<no hook parameters>"), m_KismetDialogHooks.Num() );
	_pRunningInstance->m_bTryFallbackTree = TRUE;
	return -1;
}

void UDisConv_Hook_KismetActivated::MatchHooksToOutputLinks( UBOOL _bForEditor )
{
	const INT NumHooks = m_KismetDialogHooks.Num();
	for( INT i = 0; i < NumHooks; i++ )
	{
		if( i >= m_OutputLinks.Num() )
		{
			m_OutputLinks.AddZeroed();
		}
		if( _bForEditor )
		{
			m_OutputLinks(i).m_Description = m_KismetDialogHooks(i).ToString();
		}
	}
	if( NumHooks < m_OutputLinks.Num() )
	{
		m_OutputLinks.Remove( NumHooks, m_OutputLinks.Num() - NumHooks );
	}
}
