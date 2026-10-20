/*=============================================================================
	AkMusicSegment.cpp - CAkMusicSegment (akmusicsegment.cpp, 2012 0x965d20..0x966a30).

	DISHONORED(port): agent IH4 (wave 26). Positions are in samples at the pipeline frequency; "segment position" 0 is
	the entry cue (markers[0]), so the pre-entry is negative and the post-exit lies past ActiveDuration().
	Deviation (named): GetRandomCue uses the CRT rand() as retail (_rand at the same call site).
=============================================================================*/
#include "AkMusicSegment.h"
#include "AkMusicTrack.h"
#include "AkMusicRenderer.h"
#include "AkSequencableSegmentCtx.h"
#include "AkSegmentCtx.h"
#include "AkPlayingMgr.h"
#include <stdlib.h>

/** 2012 0x9667d0, 2013 0x962ff0. */
CAkMusicSegment * CAkMusicSegment::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicSegment ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkMusicSegment * pSegment = AkPlacementNew( pMem ) CAkMusicSegment( in_ulID );
	pSegment->AddToIndex();
	return pSegment;
}

CAkMusicSegment::CAkMusicSegment( AkUniqueID in_ulID )
	: CAkMusicNode( in_ulID )
	, m_uDuration( 0 )
{
}

/** 2012 0x966b90 (the scalar deleting destructor's body): the markers and their names go. */
CAkMusicSegment::~CAkMusicSegment()
{
	m_markers.Term();
}

/** 2012 0x965d30, 2013 0x962550: seek actions go to the music renderer; the rest is CAkMusicNode's. */
AKRESULT CAkMusicSegment::ExecuteAction( ActionParams & in_rAction )
{
	if( in_rAction.eType != ActionParamType_Seek )
	{
		return CAkMusicNode::ExecuteAction( in_rAction );
	}
	SeekActionParams & rSeek = static_cast<SeekActionParams &>( in_rAction );
	if( rSeek.bIsSeekRelativeToDuration )
	{
		AkReal32 fSeekPercent = rSeek.fSeekPercent;
		if( fSeekPercent < 0.f )
		{
			fSeekPercent = 0.f;
		}
		else if( fSeekPercent > 1.f )
		{
			fSeekPercent = 1.f;
		}
		CAkMusicRenderer::m_pMusicRenderer->SeekPercent( this, in_rAction.pGameObj, fSeekPercent, rSeek.bSnapToNearestMarker != 0 );
	}
	else
	{
		CAkMusicRenderer::m_pMusicRenderer->SeekTimeAbsolute( this, in_rAction.pGameObj, rSeek.iSeekTime, rSeek.bSnapToNearestMarker != 0 );
	}
	return AK_Success;
}

/** 2012 0x965dc0, 2013 0x9625e0. */
AKRESULT CAkMusicSegment::ExecuteActionExcept( ActionParamsExcept & in_rAction )
{
	if( in_rAction.eType != ActionParamType_Seek )
	{
		return CAkMusicNode::ExecuteActionExcept( in_rAction );
	}
	SeekActionParamsExcept & rSeek = static_cast<SeekActionParamsExcept &>( in_rAction );
	if( rSeek.bIsSeekRelativeToDuration )
	{
		AkReal32 fSeekPercent = rSeek.fSeekPercent;
		if( fSeekPercent < 0.f )
		{
			fSeekPercent = 0.f;
		}
		else if( fSeekPercent > 1.f )
		{
			fSeekPercent = 1.f;
		}
		CAkMusicRenderer::m_pMusicRenderer->SeekPercent( this, in_rAction.pGameObj, fSeekPercent, rSeek.bSnapToNearestMarker != 0 );
	}
	else
	{
		CAkMusicRenderer::m_pMusicRenderer->SeekTimeAbsolute( this, in_rAction.pGameObj, rSeek.iSeekTime, rSeek.bSnapToNearestMarker != 0 );
	}
	return AK_Success;
}

/** 2012 0x965e50, 2013 0x962670: ms to samples, rounded to nearest. */
void CAkMusicSegment::Duration( AkReal64 in_fDuration )
{
	const AkReal64 v = (AkReal64)AkAudioLibSettings::g_pipelineCoreFrequency * in_fDuration * 0.001;
	m_uDuration = (AkUInt32)(AkInt64)( v + ( v <= 0.0 ? -0.5 : 0.5 ) );
}

/** 2012 0x965ec0, 2013 0x9626e0: a top-level (or child) context wrapping the segment in a chain; a segment without markers can't play. */
CAkSequencableSegmentCtx * CAkMusicSegment::CreateSegmentContext( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams )
{
	if( m_markers.IsEmpty() )
	{
		return NULL;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSequencableSegmentCtx ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkSequencableSegmentCtx * pCtx = AkPlacementNew( pMem ) CAkSequencableSegmentCtx( this, in_pParentCtx );
	pCtx->AddRef();
	if( pCtx->Init( in_GameObject, in_rUserparams ) == AK_Success )
	{
		pCtx->Release();
		return pCtx;
	}
	pCtx->_Cancel();
	pCtx->Release();
	return NULL;
}

/** 2012 0x965f40, 2013 0x962760: the low-level context the chain schedules; returned with a reference. */
CAkSegmentCtx * CAkMusicSegment::CreateLowLevelSegmentCtxAndAddRef( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams )
{
	if( m_markers.IsEmpty() )
	{
		return NULL;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkSegmentCtx ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkSegmentCtx * pCtx = AkPlacementNew( pMem ) CAkSegmentCtx( this, in_pParentCtx );
	pCtx->AddRef();
	if( pCtx->Init( in_GameObject, in_rUserparams ) == AK_Success )
	{
		return pCtx;
	}
	pCtx->_Cancel();
	pCtx->Release();
	return NULL;
}

/** 2012 0x965fb0, 2013 0x9627d0: a top-level segment context played at once, its silent look-ahead as the fade offset. */
AKRESULT CAkMusicSegment::Play( AkPBIParams & in_rPBIParams )
{
	CAkSequencableSegmentCtx * pCtx = CreateSegmentContext( NULL, in_rPBIParams.pGameObj, in_rPBIParams.userParams );
	if( !pCtx )
	{
		return AK_Fail;
	}
	AkMusicFade fadeParams;
	fadeParams.transitionTime = in_rPBIParams.pTransitionParameters->TransitionTime;
	fadeParams.eFadeCurve = in_rPBIParams.pTransitionParameters->eFadeCurve;
	fadeParams.iFadeOffset = pCtx->GetSilentDuration();
	pCtx->_Play( fadeParams );
	return AK_Success;
}

/** 2012 0x966010, 2013 0x962830. */
CAkMusicTrack * CAkMusicSegment::Track( AkUInt16 in_uIndex )
{
	if( in_uIndex >= m_mapChildId.Length() )
	{
		return NULL;
	}
	return static_cast<CAkMusicTrack *>( m_mapChildId[ in_uIndex ] );
}

/** 2012 0x966030, 2013 0x962850: the user cues carrying this filter, plus the entry and exit cues. */
AkUInt32 CAkMusicSegment::GetNumCuesWithFilter( AkUInt32 in_uCueFilter )
{
	AkUInt32 uNumCues = 1;
	for( AkUInt32 i = 1; i + 1 < m_markers.Length(); ++i )
	{
		if( m_markers[ i ].id == in_uCueFilter )
		{
			++uNumCues;
		}
	}
	return uNumCues + 1;
}

/** 2012 0x966080, 2013 0x9628a0: the marker index of the in_uIndex-th user cue (1-based) carrying the filter; 0 if none. */
AkUInt32 CAkMusicSegment::SelectCueWithFilter( AkUInt32 in_uIndex, AkUInt32 in_uCueFilter )
{
	if( !in_uIndex )
	{
		return 0;
	}
	AkUInt32 uRemaining = in_uIndex - 1;
	for( AkUInt32 uMarker = 1; ; ++uMarker )
	{
		if( m_markers[ uMarker ].id == in_uCueFilter )
		{
			if( !uRemaining )
			{
				return uMarker;
			}
			--uRemaining;
		}
		if( uMarker >= m_markers.Length() )
		{
			return 0;
		}
	}
}

/** 2012 0x9660d0, 2013 0x9628f0. */
AkInt32 CAkMusicSegment::PreEntryDuration()
{
	return (AkInt32)m_markers.m_pItems->uPosition;
}

/** 2012 0x9660e0, 2013 0x962900. */
AkUInt32 CAkMusicSegment::PostExitDuration()
{
	return m_uDuration - ( m_markers.m_pItemsEnd - 1 )->uPosition;
}

/** 2012 0x9660f0, 2013 0x962910. */
AkUInt32 CAkMusicSegment::ActiveDuration()
{
	return ( m_markers.m_pItemsEnd - 1 )->uPosition - m_markers.m_pItems->uPosition;
}

/** 2012 0x966100, 2013 0x962920: the first marker at or after the position (matching the cue filter if any), else the exit cue. */
AkUInt32 CAkMusicSegment::GetNextMarkerPosition( AkInt32 in_iPosition, AkUInt32 & io_uCueFilterHash, bool in_bDoSkipEntryCue )
{
	AkMusicMarker * pMarker = m_markers.m_pItems;
	const AkUInt32 uAbsolutePosition = in_iPosition + pMarker->uPosition;
	if( in_bDoSkipEntryCue )
	{
		++pMarker;
	}
	for( ; pMarker != m_markers.m_pItemsEnd; ++pMarker )
	{
		if( pMarker->uPosition >= uAbsolutePosition && ( !io_uCueFilterHash || pMarker->id == io_uCueFilterHash ) )
		{
			io_uCueFilterHash = pMarker->id;
			return pMarker->uPosition - m_markers.m_pItems->uPosition;
		}
	}
	io_uCueFilterHash = ( m_markers.m_pItemsEnd - 1 )->id;
	return ( m_markers.m_pItemsEnd - 1 )->uPosition - m_markers.m_pItems->uPosition;
}

/** 2012 0x966170, 2013 0x962990: the next user cue strictly after the position (matching the filter if any); AK_Fail if none. */
AKRESULT CAkMusicSegment::GetNextUserMarkerPosition( AkUInt32 in_uPosition, AkUInt32 & io_uCueFilterHash, AkUInt32 & out_uMarkerPosition )
{
	const AkUInt32 uAbsolutePosition = in_uPosition + m_markers.m_pItems->uPosition;
	const AkUInt32 uLastUserCue = m_markers.Length() - 2;
	if( m_markers.Length() == 2 )
	{
		return AK_Fail;
	}
	AkUInt32 uMarker = 1;
	for( const AkMusicMarker * pMarker = m_markers.m_pItems + 1; pMarker->uPosition <= uAbsolutePosition || ( io_uCueFilterHash && pMarker->id != io_uCueFilterHash ); ++pMarker )
	{
		if( ++uMarker > uLastUserCue )
		{
			return AK_Fail;
		}
	}
	io_uCueFilterHash = m_markers[ uMarker ].id;
	out_uMarkerPosition = m_markers[ uMarker ].uPosition - m_markers.m_pItems->uPosition;
	return AK_Success;
}

/** 2012 0x966200, 2013 0x962a20: AK_MusicSyncUserCue for every user cue in [start, start + size). */
void CAkMusicSegment::NotifyUserCuesInRange( AkPlayingID in_playingID, const AkMusicGrid & in_rGrid, AkInt32 in_iStartPosition, AkUInt32 in_uRangeSize )
{
	const AkInt32 iStart = (AkInt32)m_markers.m_pItems->uPosition + in_iStartPosition;
	AkUInt32 uStartPosition;
	AkInt32 iEnd;
	if( iStart < 0 )
	{
		iEnd = (AkInt32)in_uRangeSize + iStart;
		if( iEnd <= 0 )
		{
			return;
		}
		uStartPosition = 0;
	}
	else
	{
		uStartPosition = (AkUInt32)iStart;
		iEnd = (AkInt32)in_uRangeSize + iStart;
	}
	const AkUInt32 uEndPosition = (AkUInt32)iEnd;
	const AkUInt32 uLastUserCue = m_markers.Length() - 2;
	for( AkUInt32 uMarker = 1; uMarker <= uLastUserCue && m_markers.Length() != 2; ++uMarker )
	{
		const AkMusicMarker & rMarker = m_markers[ uMarker ];
		if( rMarker.uPosition >= uStartPosition )
		{
			if( rMarker.uPosition >= uEndPosition )
			{
				return;
			}
			g_pPlayingMgr->NotifyMusicUserCues( in_playingID, in_rGrid, rMarker.pszName );
		}
	}
}

/** 2012 0x9662a0, 2013 0x962ac0: how many bar / beat / grid boundaries the range crosses within the active part of the segment. */
void CAkMusicSegment::GetNumMusicGridInRange( AkInt32 in_iStartPosition, AkUInt32 in_uRangeSize, AkUInt32 & out_uNumBars, AkUInt32 & out_uNumBeats, AkUInt32 & out_uNumGrids )
{
	AkUInt32 uStart = (AkUInt32)in_iStartPosition;
	AkUInt32 uSize = in_uRangeSize;
	if( in_iStartPosition < 0 )
	{
		if( in_uRangeSize <= (AkUInt32)-in_iStartPosition )
		{
			out_uNumBars = 0;
			out_uNumBeats = 0;
			out_uNumGrids = 0;
			return;
		}
		uSize = (AkUInt32)in_iStartPosition + in_uRangeSize;
		uStart = 0;
	}
	const AkUInt32 uActiveDuration = ActiveDuration();
	if( uStart + uSize > uActiveDuration )
	{
		if( uStart >= uActiveDuration )
		{
			out_uNumBars = 0;
			out_uNumBeats = 0;
			out_uNumGrids = 0;
			return;
		}
		uSize = uActiveDuration - uStart;
	}
	const AkMusicGrid & rGrid = GetMusicGrid();
	if( rGrid.uBarDuration )
	{
		const AkUInt32 uPhase = ( uStart + rGrid.uBarDuration ) % rGrid.uBarDuration + rGrid.uBarDuration - 1;
		out_uNumBars = ( uSize + uPhase ) / rGrid.uBarDuration - uPhase / rGrid.uBarDuration;
	}
	else
	{
		out_uNumBars = 0;
	}
	if( rGrid.uBeatDuration )
	{
		const AkUInt32 uPhase = ( uStart + rGrid.uBeatDuration ) % rGrid.uBeatDuration + rGrid.uBeatDuration - 1;
		out_uNumBeats = ( uSize + uPhase ) / rGrid.uBeatDuration - uPhase / rGrid.uBeatDuration;
	}
	else
	{
		out_uNumBeats = 0;
	}
	if( rGrid.uGridDuration )
	{
		const AkUInt32 uPhase = ( uStart + rGrid.uGridDuration - rGrid.uGridOffset ) % rGrid.uGridDuration + rGrid.uGridDuration - 1;
		out_uNumGrids = ( uPhase + uSize ) / rGrid.uGridDuration - uPhase / rGrid.uGridDuration;
	}
	else
	{
		out_uNumGrids = 0;
	}
}

/** 2012 0x9663b0, 2013 0x962bd0: the cue nearest the position (the exit cue is not a candidate). */
AkInt32 CAkMusicSegment::GetClosestCuePosition( AkInt32 in_iPosition )
{
	const AkUInt32 uEntryPosition = m_markers.m_pItems->uPosition;
	const AkInt32 iAbsolutePosition = (AkInt32)uEntryPosition + in_iPosition;
	const AkMusicMarker * pClosest = NULL;
	AkInt32 iSmallestDistance = (AkInt32)m_markers.Length() - 1;
	const AkUInt32 uNumCandidates = m_markers.Length() - 1;
	const AkMusicMarker * pMarker = m_markers.m_pItems;
	for( AkUInt32 i = 0; i < uNumCandidates; ++i, ++pMarker )
	{
		const AkInt32 iDistance = abs( (AkInt32)pMarker->uPosition - iAbsolutePosition );
		if( !pClosest || iDistance < iSmallestDistance )
		{
			pClosest = pMarker;
			iSmallestDistance = iDistance;
		}
	}
	return (AkInt32)( pClosest->uPosition - uEntryPosition );
}

/** 2012 0x966420, 2013 0x962c40: any track's state sync, else the segment's own chain. */
bool CAkMusicSegment::GetStateSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes )
{
	for( CAkParameterNodeBase ** it = m_mapChildId.m_pItems; it != m_mapChildId.m_pItemsEnd; ++it )
	{
		if( static_cast<CAkMusicTrack *>( *it )->GetStateSyncTypes( in_stateGroupID, io_pSyncTypes ) )
		{
			return true;
		}
	}
	return CAkParameterNodeBase::GetStateSyncTypes( in_stateGroupID, io_pSyncTypes, false );
}

/** 2012 0x966470, 2013 0x962c90: the first multiple of the grid past the minimum (skipping the entry cue); AK_Fail past the exit cue. */
AKRESULT CAkMusicSegment::GetNextMusicGridValue( AkUInt32 in_uMinPosition, AkUInt32 in_uGridDuration, AkUInt32 in_uOffset, bool in_bSkipEntryCue, AkUInt32 & out_uExitPosition )
{
	AkUInt32 uMinPosition = in_uMinPosition;
	if( in_bSkipEntryCue )
	{
		uMinPosition = in_uMinPosition + 1;
	}
	const AkUInt32 uActiveDuration = ActiveDuration();
	out_uExitPosition = in_uOffset;
	if( !in_uGridDuration )
	{
		out_uExitPosition = uMinPosition + 1;
		return uActiveDuration < out_uExitPosition ? AK_Fail : AK_Success;
	}
	if( in_uOffset <= uMinPosition )
	{
		do
		{
			out_uExitPosition += in_uGridDuration;
		}
		while( out_uExitPosition <= uMinPosition );
	}
	return uActiveDuration < out_uExitPosition ? AK_Fail : AK_Success;
}

/** 2012 0x966510, 2013 0x962d30. */
CAkMatrixAwareCtx * CAkMusicSegment::CreateContext( CAkMatrixAwareCtx * in_pParentCtx, CAkRegisteredObj * in_GameObject, UserParams & in_rUserparams )
{
	return CreateSegmentContext( in_pParentCtx, in_GameObject, in_rUserparams );
}

/** 2012 0x966520, 2013 0x962d40: only unparented music tracks, once each. */
AKRESULT CAkMusicSegment::CanAddChild( CAkParameterNodeBase * in_pAudioNode )
{
	const AkNodeCategory eCategory = in_pAudioNode->NodeCategory();
	if( Children() == 0xFFFF )
	{
		return AK_MaxReached;
	}
	if( eCategory != AkNodeCategory_MusicTrack )
	{
		return AK_NotCompatible;
	}
	if( in_pAudioNode->m_pParentNode )
	{
		return AK_ChildAlreadyHasAParent;
	}
	const AkUniqueID key = in_pAudioNode->key;
	if( m_mapChildId.Exists( key ) )
	{
		return AK_AlreadyConnected;
	}
	return this->key == key ? AK_CannotAddItseflAsAChild : AK_Success;
}

/** 2012 0x9665c0, 2013 0x962de0: the exit sync position at or after in_uSrcMinTime (samples from the entry cue). */
AKRESULT CAkMusicSegment::GetExitSyncPos( AkUInt32 in_uSrcMinTime, AkSyncType in_eSyncType, AkUInt32 & io_uCueFilterHash, bool in_bSkipEntryCue, AkUInt32 & out_uExitSyncPos )
{
	if( in_uSrcMinTime > ActiveDuration() )
	{
		return AK_Fail;
	}
	AKRESULT eResult = AK_Fail;
	switch( in_eSyncType )
	{
	case SyncTypeImmediate:
		io_uCueFilterHash = 0;
		out_uExitSyncPos = in_uSrcMinTime;
		eResult = AK_Success;
		break;
	case SyncTypeNextGrid:
		{
			const AkMusicGrid & rGrid = GetMusicGrid();
			eResult = GetNextMusicGridValue( in_uSrcMinTime, rGrid.uGridDuration, rGrid.uGridOffset, in_bSkipEntryCue, out_uExitSyncPos );
			io_uCueFilterHash = 0;
		}
		break;
	case SyncTypeNextBar:
		eResult = GetNextMusicGridValue( in_uSrcMinTime, GetMusicGrid().uBarDuration, 0, in_bSkipEntryCue, out_uExitSyncPos );
		io_uCueFilterHash = 0;
		break;
	case SyncTypeNextBeat:
		eResult = GetNextMusicGridValue( in_uSrcMinTime, GetMusicGrid().uBeatDuration, 0, in_bSkipEntryCue, out_uExitSyncPos );
		io_uCueFilterHash = 0;
		break;
	case SyncTypeNextMarker:
		out_uExitSyncPos = GetNextMarkerPosition( (AkInt32)in_uSrcMinTime, io_uCueFilterHash, in_bSkipEntryCue );
		eResult = AK_Success;
		break;
	case SyncTypeNextUserMarker:
		eResult = GetNextUserMarkerPosition( in_uSrcMinTime, io_uCueFilterHash, out_uExitSyncPos );
		break;
	case SyncTypeEntryMarker:
		if( !in_uSrcMinTime && !in_bSkipEntryCue )
		{
			io_uCueFilterHash = m_markers.m_pItems->id;
			out_uExitSyncPos = 0;
			eResult = AK_Success;
		}
		break;
	case SyncTypeExitMarker:
		out_uExitSyncPos = ActiveDuration();
		io_uCueFilterHash = ( m_markers.m_pItemsEnd - 1 )->id;
		eResult = AK_Success;
		break;
	default:
		break;
	}
	return eResult;
}

/** 2012 0x966730, 2013 0x962f50: a random cue among the filtered ones (avoiding the entry cue if asked); the entry cue if none. */
AkInt32 CAkMusicSegment::GetRandomCue( AkUInt32 in_uCueFilter, bool in_bAvoidEntryCue, AkUInt32 & out_uSelectedCueHash )
{
	const AkUInt32 uNumCues = in_uCueFilter ? GetNumCuesWithFilter( in_uCueFilter ) : m_markers.Length();
	if( uNumCues <= 2 )
	{
		out_uSelectedCueHash = m_markers.m_pItems->id;
		return 0;
	}
	AkUInt32 uSelected = ( in_bAvoidEntryCue ? 1 : 0 ) + rand() % ( uNumCues - ( in_bAvoidEntryCue ? 1 : 0 ) - 1 );
	if( in_uCueFilter )
	{
		uSelected = SelectCueWithFilter( uSelected, in_uCueFilter );
	}
	out_uSelectedCueHash = m_markers[ uSelected ].id;
	return (AkInt32)( m_markers[ uSelected ].uPosition - m_markers.m_pItems->uPosition );
}

/** 2012 0x966820, 2013 0x963040: ms positions to samples; the markers take over the names. */
AKRESULT CAkMusicSegment::SetMarkers( AkMusicMarkerWwise * in_pArrayMarkers, AkUInt32 in_ulNumMarkers )
{
	m_markers.Term();
	if( !in_ulNumMarkers )
	{
		return AK_Success;
	}
	if( m_markers.Reserve( in_ulNumMarkers ) != AK_Success )
	{
		return AK_Fail;
	}
	for( AkUInt32 i = 0; i < in_ulNumMarkers; ++i )
	{
		AkMusicMarker * pMarker = m_markers.AddLast();
		pMarker->id = in_pArrayMarkers[ i ].id;
		const AkReal64 v = (AkReal64)AkAudioLibSettings::g_pipelineCoreFrequency * in_pArrayMarkers[ i ].fPosition * 0.001;
		pMarker->uPosition = (AkUInt32)(AkInt64)( v + ( v <= 0.0 ? -0.5 : 0.5 ) );
		pMarker->pszName = in_pArrayMarkers[ i ].pszName;
	}
	return AK_Success;
}

/** 2012 0x966970, 2013 0x963190: where the destination enters, by the rule's entry type. */
void CAkMusicSegment::GetEntrySyncPos( const AkMusicTransDestRule & in_rule, AkUInt32 in_uMinimumEntryPosition, AkUInt32 in_uCueHashForMatchSrc, AkUInt32 & out_uSelectedCueHash, AkUInt32 & out_uRequiredEntryPosition )
{
	out_uSelectedCueHash = 0;
	switch( in_rule.eEntryType )
	{
	case EntryTypeEntryMarker:
		out_uRequiredEntryPosition = 0;
		out_uSelectedCueHash = m_markers.m_pItems->id;
		break;
	case EntryTypeSameTime:
		{
			const AkUInt32 uActiveDuration = ActiveDuration();
			out_uRequiredEntryPosition = uActiveDuration ? in_uMinimumEntryPosition % uActiveDuration : 0;
		}
		break;
	case EntryTypeRandomMarker:
	case EntryTypeRandomUserMarker:
		if( !in_rule.bDestMatchSourceCueName )
		{
			in_uCueHashForMatchSrc = in_rule.uCueFilterHash;
		}
		out_uRequiredEntryPosition = (AkUInt32)GetRandomCue( in_uCueHashForMatchSrc, in_rule.eEntryType == EntryTypeRandomUserMarker, out_uSelectedCueHash );
		break;
	default:
		out_uRequiredEntryPosition = 0;
		break;
	}
}

/**
 * 2012 0x966a30: the music node part, the duration (double ms), then the markers (id, double ms, name length + name;
 * names are copied and terminated).
 */
AKRESULT CAkMusicSegment::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize )
{
	AKRESULT eResult = SetMusicNodeParams( in_pData, in_ulDataSize, false );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	const AkReal64 fDuration = *(AkReal64 *)in_pData;
	in_pData += 8;
	Duration( fDuration );
	const AkUInt32 uNumMarkers = *(AkUInt32 *)in_pData;
	in_pData += 4;
	if( uNumMarkers )
	{
		AkMusicMarkerWwise * pArrayMarkers = (AkMusicMarkerWwise *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkMusicMarkerWwise ) * uNumMarkers );
		if( !pArrayMarkers )
		{
			return AK_InsufficientMemory;
		}
		for( AkUInt32 i = 0; i < uNumMarkers; ++i )
		{
			pArrayMarkers[ i ].id = *(AkUInt32 *)in_pData;
			in_pData += 4;
			pArrayMarkers[ i ].fPosition = *(AkReal64 *)in_pData;
			in_pData += 8;
			const AkUInt32 uStringSize = *(AkUInt32 *)in_pData;
			in_pData += 4;
			if( uStringSize )
			{
				const AkUInt8 * pString = in_pData;
				in_pData += uStringSize;
				pArrayMarkers[ i ].pszName = (char *)AK::MemoryMgr::Malloc( g_DefaultPoolId, uStringSize + 1 );
				if( !pArrayMarkers[ i ].pszName )
				{
					AK::MemoryMgr::Free( g_DefaultPoolId, pArrayMarkers );
					return AK_Fail;
				}
				memcpy( pArrayMarkers[ i ].pszName, pString, uStringSize );
				pArrayMarkers[ i ].pszName[ uStringSize ] = 0;
			}
			else
			{
				pArrayMarkers[ i ].pszName = NULL;
			}
		}
		eResult = SetMarkers( pArrayMarkers, uNumMarkers );
		AK::MemoryMgr::Free( g_DefaultPoolId, pArrayMarkers );
	}
	g_pHircParseEnd = in_pData;
	return eResult;
}
