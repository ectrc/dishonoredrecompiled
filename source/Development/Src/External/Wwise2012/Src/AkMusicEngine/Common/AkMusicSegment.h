/*=============================================================================
	AkMusicSegment.h - CAkMusicSegment (akmusicsegment.cpp): a music segment - its tracks (children), its duration and
	markers (entry cue first, exit cue last, user cues between), the sync point arithmetic of transitions (next bar /
	beat / grid / cue), the user-cue and grid notifications, and the segment contexts it creates.

	DISHONORED(port): agent IH4 (wave 26). Layout: CAkMusicNode (112) + m_markers (+112) + m_uDuration (+124) = 0x80,
	as CAkMusicSegment::Create 0x9667d0 allocates.
=============================================================================*/
#pragma once

#include "AkMusicNode.h"

class CAkMusicTrack;
class CAkSequencableSegmentCtx;
class CAkSegmentCtx;

class CAkMusicSegment : public CAkMusicNode
{
public:
	static CAkMusicSegment * Create( AkUniqueID in_ulID );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize );

	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_MusicSegment; }
	virtual AKRESULT Play( AkPBIParams & in_rPBIParams );
	virtual AKRESULT ExecuteAction( ActionParams & in_rAction );
	virtual AKRESULT ExecuteActionExcept( ActionParamsExcept & in_rAction );
	virtual AKRESULT CanAddChild( CAkParameterNodeBase * in_pAudioNode );
	virtual CAkMatrixAwareCtx * CreateContext( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );

	CAkSequencableSegmentCtx * CreateSegmentContext( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );
	CAkSegmentCtx * CreateLowLevelSegmentCtxAndAddRef( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams );

	void Duration( AkReal64 in_fDuration );
	AKRESULT SetMarkers( AkMusicMarkerWwise * in_pArrayMarkers, AkUInt32 in_ulNumMarkers );
	CAkMusicTrack * Track( AkUInt16 in_uIndex );

	AkInt32 PreEntryDuration();
	AkUInt32 PostExitDuration();
	AkUInt32 ActiveDuration();

	AKRESULT GetExitSyncPos( AkUInt32 in_uSrcMinTime, AkSyncType in_eSyncType, AkUInt32 & io_uCueFilterHash, bool in_bSkipEntryCue, AkUInt32 & out_uExitSyncPos );
	void GetEntrySyncPos( const AkMusicTransDestRule & in_rule, AkUInt32 in_uMinimumEntryPosition, AkUInt32 in_uCueHashForMatchSrc, AkUInt32 & out_uSelectedCueHash, AkUInt32 & out_uRequiredEntryPosition );
	AkInt32 GetClosestCuePosition( AkInt32 in_iPosition );
	void NotifyUserCuesInRange( AkPlayingID in_playingID, const AkMusicGrid & in_rGrid, AkInt32 in_iStartPosition, AkUInt32 in_uRangeSize );
	void GetNumMusicGridInRange( AkInt32 in_iStartPosition, AkUInt32 in_uRangeSize, AkUInt32 & out_uNumBars, AkUInt32 & out_uNumBeats, AkUInt32 & out_uNumGrids );
	bool GetStateSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes );

	AkArray<AkMusicMarker, const AkMusicMarker &, ArrayPoolDefault, 0>	m_markers;
	AkUInt32															m_uDuration;

protected:
	explicit CAkMusicSegment( AkUniqueID in_ulID );
	virtual ~CAkMusicSegment();

	AkUInt32 GetNumCuesWithFilter( AkUInt32 in_uCueFilter );
	AkUInt32 SelectCueWithFilter( AkUInt32 in_uIndex, AkUInt32 in_uCueFilter );
	AkUInt32 GetNextMarkerPosition( AkInt32 in_iPosition, AkUInt32 & io_uCueFilterHash, bool in_bDoSkipEntryCue );
	AKRESULT GetNextUserMarkerPosition( AkUInt32 in_uPosition, AkUInt32 & io_uCueFilterHash, AkUInt32 & out_uMarkerPosition );
	AKRESULT GetNextMusicGridValue( AkUInt32 in_uMinPosition, AkUInt32 in_uGridDuration, AkUInt32 in_uOffset, bool in_bSkipEntryCue, AkUInt32 & out_uExitPosition );
	AkInt32 GetRandomCue( AkUInt32 in_uCueFilter, bool in_bAvoidEntryCue, AkUInt32 & out_uSelectedCueHash );
};

static_assert( sizeof( CAkMusicSegment ) == 0x80, "CAkMusicSegment: 2012 PDB 0x80 (Create mallocs 0x80)" );
