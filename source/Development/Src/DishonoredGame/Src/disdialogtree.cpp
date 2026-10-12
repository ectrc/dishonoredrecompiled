// DishonoredGame/src/disdialogtree.cpp
// DISHONORED(written): Serialize 2013 rva 0x88b210 (2012 0x8f97f0), the resave flag.
// DISHONORED(port): agent FR (PHASE17 FR). The tree-level direct path, 2013 rvas (2012 in parentheses):
//   GetDialogHookInfo (0x914170, 32)   InitHookInfo 0x88d820 (0x90f8f0, 18552 / 18920 B)   BindToOneShot (0x8f86c0, 49)
//   InitNode (0x914190, 865)           SupportsDialogHook 0x892280 (0x8fad70, 108)        InFallbackChain (0x8f8720)
//   PostLoad 0x88b1f0 (0x8f8700, 28)
// The hook-info table is retail 2013's 126-entry g_DialogHookInfo (DDH_INVALID entries), read row by row out of
// 0x88d820 (build/agentFR/hookinfo_2013.txt is the extraction, build/agentFR/tools/extract_hookinfo3.py the reader,
// gen_hookinfo_table.py the emitter). 2012's table had 122 rows; the four 2013 additions are the DLC06/07 hooks 122..125
// and the rest is identical row for row. The editor-only strings (category, tooltip, initiator description) are kept
// because retail keeps them under GIsEditor and they cost nothing here.
// Not ported (agentFR.md inventory): the editor surface - ValidateAndFixupFolders 2012 0x90b040, SearchNodes 0x8922f0,
// GetDisplayName 0x893a20, MarkWholeTreePendingKill 0x8fad00, the constructors.
#include "DishonoredGame.h"
#include "frdialogdiag.h"

void UDisDialogTree::Serialize( FArchive& Ar )
{
	Super::Serialize( Ar );
	if( Ar.IsSaving() && !Ar.IsTransacting() )
	{
		m_bNeedsResaving = FALSE;
	}
}

// 2013 rva 0x88b1f0 (2012 0x8f8700, 28 B): the tree initialises its whole node graph on load - this is what stamps every
// node's m_iConvNodeIndex and every conversation's, which the cooked data does not carry (measured: every index read -1 on
// the boat tree until this ran, FR4.log, and the soiree node then had no in-game data).
void UDisDialogTree::PostLoad()
{
	Super::PostLoad();
	InitNode( NULL, FALSE, FALSE );
}

struct FDisDialogHookInfoRow
{
	const TCHAR* DisplayName;
	const TCHAR* HookCategory;
	const TCHAR* ToolTip;
	const TCHAR* InitiatorDescription;
	UClass* (*HookClass)();
	BYTE FireGroup;
	FLOAT RefireDelayMin;
	FLOAT RefireDelayMax;
	FLOAT RefireDelayGlobal;
	FLOAT BustTimeWhenFired;
	INT Flags;
};

static const FDisDialogHookInfoRow GDisDialogHookInfoRows[DDH_INVALID] =
{
#include "disdialogtree_hookinfo.inl"
};

static TArray<FDisDialogHookInfo> GDisDialogHookInfo;

void UDisDialogTree::InitHookInfo()
{
	GDisDialogHookInfo.Empty( DDH_INVALID );
	GDisDialogHookInfo.AddZeroed( DDH_INVALID );
	for( INT i = 1; i < DDH_INVALID; i++ )
	{
		const FDisDialogHookInfoRow& Row = GDisDialogHookInfoRows[i];
		FDisDialogHookInfo& Info = GDisDialogHookInfo(i);
		if( GIsEditor )
		{
			Info.m_HookCategory = Row.HookCategory ? Row.HookCategory : TEXT("");
			Info.m_ToolTip = Row.ToolTip ? Row.ToolTip : TEXT("");
			Info.m_InitiatorDescription = Row.InitiatorDescription ? Row.InitiatorDescription : TEXT("");
		}
		Info.m_DisplayName = Row.DisplayName ? Row.DisplayName : TEXT("");
		Info.m_pHookClass = Row.HookClass ? Row.HookClass() : NULL;
		Info.m_FireGroup = Row.FireGroup;
		Info.m_fRefire_DelayMin = Row.RefireDelayMin;
		Info.m_fRefire_DelayMax = Row.RefireDelayMax;
		Info.m_fRefire_Delay_Global = Row.RefireDelayGlobal;
		Info.m_fBustTimeWhenFired = Row.BustTimeWhenFired;
		Info.m_Flags = Row.Flags;
		Info.m_bInitialized = TRUE;
	}
}

TArray<FDisDialogHookInfo>& UDisDialogTree::GetDialogHookInfo()
{
	if( GDisDialogHookInfo.Num() != DDH_INVALID )
	{
		GDisDialogHookInfo.Reset();
		InitHookInfo();
	}
	return GDisDialogHookInfo;
}

void UDisDialogTree::BindToOneShot( ADisDialogOneShot* _pOneShot )
{
	m_bOneShot = TRUE;
	m_pBoundToOneShot = _pOneShot;
	InitNode( NULL, !GIsGame, FALSE );
}

// Retail's InitNode is the editor's fixup pass as well as the game's: the hook slots are moved to their enum index when a
// node's m_HookEnum disagrees with its slot, and a node with an out-of-range enum is unlinked and dropped
// (UDisConversation::BreakAllLinks / UDisConv_Node::OnDelete, both editor). Cooked data arrives consistent; the slot moves
// are kept, the drop is a clear.
void UDisDialogTree::InitNode( UDisConversation* _pOwner, UBOOL _bForEditor, UBOOL _bIsNewNode )
{
	m_Label = GetFName();
	const INT NumHooks = GetDialogHookInfo().Num();
	if( m_DialogHooks.Num() < NumHooks )
	{
		m_DialogHooks.AddZeroed( NumHooks - m_DialogHooks.Num() );
	}
	for( INT i = 0; i < m_DialogHooks.Num(); i++ )
	{
		UDisConv_DialogHook* Hook = m_DialogHooks(i);
		if( !Hook )
		{
			continue;
		}
		const INT HookEnum = Hook->m_HookEnum;
		if( HookEnum >= NumHooks )
		{
			m_DialogHooks(i) = NULL;
			m_Nodes.RemoveItem( Hook );
			continue;
		}
		if( HookEnum == i )
		{
			if( Hook->m_iHookIndex != HookEnum )
			{
				Hook->m_iHookIndex = HookEnum;
			}
		}
		else if( HookEnum < i )
		{
			m_DialogHooks( HookEnum ) = Hook;
			Hook->m_iHookIndex = HookEnum;
			m_DialogHooks(i) = NULL;
		}
		else
		{
			m_DialogHooks.InsertZeroed( i, HookEnum - i );
			i = HookEnum;
			Hook->m_iHookIndex = i;
		}
	}
	UDisConversation::InitNode( _pOwner, _bForEditor, _bIsNewNode );
	m_iConvNodeIndex = -1;
	for( INT i = 0; i < m_Conversations.Num(); i++ )
	{
		if( m_Conversations(i) )
		{
			m_Conversations(i)->m_iConvNodeIndex = i;
			m_Conversations(i)->InitNode( this, _bForEditor, _bIsNewNode );
		}
	}
	if( _bForEditor && m_pFallbackTree )
	{
		m_pFallbackTree->InitNode( _pOwner, _bForEditor, _bIsNewNode );
	}
}

UBOOL UDisDialogTree::SupportsDialogHook( eDisDialogHook _Hook ) const
{
	for( const UDisDialogTree* Tree = this; Tree; Tree = Tree->m_pFallbackTree )
	{
		if( _Hook >= 0 && _Hook < Tree->m_DialogHooks.Num() && Tree->m_DialogHooks( _Hook ) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

UBOOL UDisDialogTree::InFallbackChain( const UDisDialogTree* _pTree ) const
{
	if( !_pTree )
	{
		return FALSE;
	}
	for( const UDisDialogTree* Tree = this; Tree; Tree = Tree->m_pFallbackTree )
	{
		if( Tree == _pTree )
		{
			return TRUE;
		}
	}
	return FALSE;
}
