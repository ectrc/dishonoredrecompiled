/*=============================================================================
	AkRSIterator.h - AkRSIterator, RSStackItem (akrsiterator.cpp): the walk of a music playlist tree. A stack of the
	groups being played (each with its loop count and its continuous selection state), the current segment and its
	loop count, the global step selection states saved on first use so a context that never plays can revert them
	(EndInit commits them), and JumpTo (a playlist item id) for "jump to playlist item" transitions.

	DISHONORED(port): agent IH4 (wave 26). Layouts: RSStackItem 12 bytes, AkRSIterator 44 bytes (2012 PDB; the
	m_bDoSaveOriginalGlobalRSInfo bit is bit 0 of +40, m_uSegmentLoopCount +42).
=============================================================================*/
#pragma once

#include "AkMusicRanSeqCntr.h"

class RSStackItem
{
public:
	RSStackItem() : pRSNode( NULL ), pLocalRSInfo( NULL )
	{
		m_Loop.lLoopCount = 0;
		m_Loop.bIsEnabled = 0;
		m_Loop.bIsInfinite = 0;
	}

	AKRESULT Init( CAkRSSub * in_pSub );
	void Clear();

	CAkRSSub *				pRSNode;
	AkLoop					m_Loop;
	CAkContainerBaseInfo *	pLocalRSInfo;
};

class AkRSIterator
{
public:
	explicit AkRSIterator( CAkMusicRanSeqCntr * in_pRSCntr );

	AKRESULT Init();
	void Term();
	void EndInit();
	void JumpNext();
	AKRESULT JumpTo( AkUniqueID in_playlistElementID );

	CAkMusicRanSeqCntr *												m_pRSCntr;
	AkArray<RSStackItem, const RSStackItem &, ArrayPoolDefault, 5>		m_stack;
	AkUniqueID															m_actualSegment;
	AkUniqueID															m_LastSegmentPlayingID;
	bool																m_bIsSegmentValid;
	CAkKeyArray<CAkRSSub *, CAkContainerBaseInfo *, 8>					m_arOriginalGlobalRSInfo;
	AkUInt16															m_bDoSaveOriginalGlobalRSInfo : 1;
	AkInt16																m_uSegmentLoopCount;

private:
	bool CanPlayPosition( CAkRSSub * in_pSub, CAkRandomInfo * in_pRandomInfo, AkUInt16 in_wPosition );
	AKRESULT SetCurrentSegmentToNode( CAkRSNode * in_pNode );
	CAkRSNode * PopObsoleteStackedItems( CAkRSNode * in_pNode );
	void FlushStack();
	void PopLast();
	void RevertGlobalRSInfo();
	AKRESULT FindAndSelect( CAkRSNode * in_pNode, AkUniqueID in_playlistElementID, AkArray<CAkRSNode *, CAkRSNode *, ArrayPoolDefault, 16> & io_jmpList, bool & io_bFound );
	AKRESULT StackItem( CAkRSSub * in_pSub );
	void SaveOriginalGlobalRSInfo( CAkRSSub * in_pSub, CAkContainerBaseInfo * in_pRSInfo );
	void ForceSelectSequentially( CAkRSNode * in_pForcedNode );
	AkUInt16 SelectSequentially( RSStackItem & in_rStackItem, bool & out_bIsEnd );
	void UpdateRandomItem( CAkRSSub * in_pSub, AkUInt16 in_wPosition, AkArray<CAkRSNode *, CAkRSNode *, ArrayPoolDefault, 16> * in_pRSList, CAkRandomInfo * in_pRanInfo );
	AkUInt16 SelectRandomly( RSStackItem & in_rStackItem, bool & out_bIsEnd );
	void ForceSelectRandomly( CAkRSNode * in_pForcedNode );
	AKRESULT JumpNextInternal();
	AkUInt16 Select( RSStackItem & in_rStackItem, bool & out_bIsEnd );
};

static_assert( sizeof( RSStackItem ) == 12, "RSStackItem: 2012 PDB 12" );
static_assert( sizeof( AkRSIterator ) == 44, "AkRSIterator: 2012 PDB 44" );
