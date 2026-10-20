/*=============================================================================
	AkSequenceCtx.h - CAkSequenceCtx (aksequencectx.cpp): the playing instance of a music playlist container: a chain
	grown segment by segment from the playlist iterator, each new item placed at the previous one's exit sync point by
	the container's transition rules (with transition segments), kept a few items ahead of the playing position.

	DISHONORED(port): agent IH4 (wave 26). Layout (2012 PDB, 0x98): CAkChainCtx (96) + m_pSequenceNode (+96) +
	m_PlayListIterator (+100, 44) + m_bIsChainValid (+144).
=============================================================================*/
#pragma once

#include "AkSegmentChain.h"
#include "AkRSIterator.h"

class CAkSequenceCtx : public CAkChainCtx
{
public:
	CAkSequenceCtx( CAkMusicRanSeqCntr * in_pSequenceNode, CAkMusicCtx * in_pParentCtx );
	virtual ~CAkSequenceCtx();

	AKRESULT Init( CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );

	virtual void EndInit();
	virtual CAkMusicNode * Node();
	virtual bool IsValid();
	virtual bool Grow();
	virtual void UpdateChainLength( AkInt64 in_iDesiredPosition );
	virtual CAkScheduledItem * HandleFatalError();

	CAkScheduledItem * JumpToSegment( AkUniqueID in_uJumpToID );

	CAkMusicRanSeqCntr *	m_pSequenceNode;
	AkRSIterator			m_PlayListIterator;
	bool					m_bIsChainValid;

private:
	CAkScheduledItem * ScheduleNextSegment( bool & out_bPlayPreEntry );
	CAkScheduledItem * AppendItem( const AkMusicTransitionRule & in_rule, CAkScheduledItem * in_pSrcItem, AkUniqueID in_nextID, AkUniqueID in_playlistItemID );
};

static_assert( sizeof( CAkSequenceCtx ) == 0x98, "CAkSequenceCtx: 2012 PDB 0x98 (CreateSequenceCtx mallocs 0x98)" );
