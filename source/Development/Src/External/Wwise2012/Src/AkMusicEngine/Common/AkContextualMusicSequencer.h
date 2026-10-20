/*=============================================================================
	AkContextualMusicSequencer.h - CAkContextualMusicSequencer and the music actions (akcontextualmusicsequencer.cpp
	/ .h): the time-ordered queue of a segment context's clip commands - play a track's clip (source offset, look-ahead,
	clip automation) and stop a music PBI - in samples relative to the segment's entry cue.

	DISHONORED(port): agent IH4 (wave 26). Layouts (2012 PDB): AkMusicAction 12 (vptr, pNextItem, m_iTime),
	AkMusicActionPlay 0x20, AkMusicActionStop 0x14, AkMusicAutomation 0x10, CAkContextualMusicSequencer 16 (vptr + the
	AkListBare base).
=============================================================================*/
#pragma once

#include "AkMusicStructs.h"

class CAkMusicTrack;
class CAkMusicPBI;
class CAkClipAutomation;

struct AkMusicAutomation
{
	CAkClipAutomation *		pAutomationData;
	CAkMusicPBI *			pPBI;
	AkInt32					iTimeStart;
	AkMusicAutomation *		pNextLightItem;
};

class AkMusicAction
{
public:
	AkMusicAction( AkInt32 in_iTime ) : pNextItem( NULL ), m_iTime( in_iTime ) {}
	virtual ~AkMusicAction() {}
	virtual AkMusicActionType Type() = 0;

	AkMusicAction *	pNextItem;
	AkInt32			m_iTime;
};

class AkMusicActionPlay : public AkMusicAction
{
public:
	AkMusicActionPlay( AkInt32 in_iTime, CAkMusicTrack * in_pTrack, const AkTrackSrc & in_rTrackSrc, AkUInt32 in_uSourceOffset, AkUInt32 in_uPlayOffset )
		: AkMusicAction( in_iTime )
		, m_pTrack( in_pTrack )
		, m_rTrackSrc( &in_rTrackSrc )
		, m_uSourceOffset( in_uSourceOffset )
		, m_uPlayOffset( in_uPlayOffset )
	{
		m_listAutomation.m_pFirst = NULL;
	}
	virtual ~AkMusicActionPlay();
	virtual AkMusicActionType Type() { return MusicActionTypePlay; }

	void AttachClipAutomation( CAkClipAutomation * in_pAutomation, AkInt32 in_iTimeStart );

	CAkMusicTrack *							m_pTrack;
	const AkTrackSrc *						m_rTrackSrc;
	AkUInt32								m_uSourceOffset;
	AkUInt32								m_uPlayOffset;
	AkListBareLight<AkMusicAutomation>		m_listAutomation;
};

class AkMusicActionStop : public AkMusicAction
{
public:
	AkMusicActionStop( AkInt32 in_iTime, CAkMusicPBI * in_pTargetPBI ) : AkMusicAction( in_iTime ), pTargetPBI( in_pTargetPBI ), bHasAutomation( false ) {}
	virtual AkMusicActionType Type() { return MusicActionTypeStop; }

	CAkMusicPBI *	pTargetPBI;
	bool			bHasAutomation;
};

class CAkContextualMusicSequencer : public AkListBare<AkMusicAction>
{
public:
	CAkContextualMusicSequencer();
	virtual ~CAkContextualMusicSequencer();

	void ScheduleAction( AkMusicAction * in_pAction );
	AKRESULT PopImminentAction( AkInt32 in_iNow, AkInt32 in_iFrameDuration, AkMusicAction *& out_pAction );
	void ClearActionsByTarget( CAkMusicPBI * in_pTarget );
	void Flush();
};

static_assert( sizeof( AkMusicActionPlay ) == 0x20, "AkMusicActionPlay: 2012 PDB 0x20" );
static_assert( sizeof( AkMusicActionStop ) == 0x14, "AkMusicActionStop: 2012 PDB 0x14" );
static_assert( sizeof( AkMusicAutomation ) == 0x10, "AkMusicAutomation: 2012 PDB 0x10" );
static_assert( sizeof( CAkContextualMusicSequencer ) == 16, "CAkContextualMusicSequencer: 2012 PDB 16" );
