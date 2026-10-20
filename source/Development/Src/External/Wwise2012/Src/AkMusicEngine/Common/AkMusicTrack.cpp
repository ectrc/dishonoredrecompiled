/*=============================================================================
	AkMusicTrack.cpp - CAkMusicTrack, CAkClipAutomation (akmusictrack.cpp / .h, 2012 0x968180..0x968ab0, plus
	CAkMusicTrack::UnPrepareData 0x952990 which the PDB files under akfeedbacknode.cpp - a folded body).

	DISHONORED(port): agent IH4 (wave 26).
	Deviations (named):
	  * GetClipAutomation is the lookup ScheduleAudioClips (0x966e30) inlines four times; it is one function here.
	  * GetNextRS uses the CRT rand() as retail.
=============================================================================*/
#include "AkMusicTrack.h"
#include "AkMusicRenderer.h"
#include <AkBankMgr.h>
#include <stdlib.h>

/** 2012 0x968670, 2013 0x964e40. */
CAkMusicTrack * CAkMusicTrack::Create( AkUniqueID in_ulID )
{
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicTrack ) );
	if( !pMem )
	{
		return NULL;
	}
	CAkMusicTrack * pTrack = AkPlacementNew( pMem ) CAkMusicTrack( in_ulID );
	pTrack->AddToIndex();
	return pTrack;
}

/** 2012 0x968620, 2013 0x964df0. */
CAkMusicTrack::CAkMusicTrack( AkUniqueID in_ulID )
	: CAkSoundBase( in_ulID )
	, m_uNumSubTrack( 0 )
	, m_iLookAheadTime( 0 )
	, m_eRSType( AkMusicTrackRanSeqType_Normal )
	, m_SequenceIndex( (AkUInt16)-1 )
{
}

/** 2012 0x9687f0, 2013 0x965040. */
CAkMusicTrack::~CAkMusicTrack()
{
	RemoveAllSources();
	m_arSrcInfo.Term();
	m_arTrackPlaylist.Term();
	m_arClipAutomation.Term();
}

/** 2012 0x968180, 2013 0x964a30: a track only follows its node's transitions; the playing contexts are the music renderer's. */
AKRESULT CAkMusicTrack::ExecuteAction( ActionParams & in_rAction )
{
	if( in_rAction.bIsMasterCall )
	{
		PauseTransitions( in_rAction.eType == ActionParamType_Pause );
	}
	return AK_Success;
}

/** 2012 0x9681b0, 2013 0x964a60. */
AKRESULT CAkMusicTrack::ExecuteActionExcept( ActionParamsExcept & in_rAction )
{
	if( !in_rAction.pGameObj )
	{
		PauseTransitions( in_rAction.eType == ActionParamType_Pause );
	}
	return AK_Success;
}

/** 2012 0x9681e0, 2013 0x964a90: the track's own state chunks, else its output bus chain. */
bool CAkMusicTrack::GetStateSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes )
{
	if( CheckSyncTypes( in_stateGroupID, io_pSyncTypes ) )
	{
		return true;
	}
	CAkBus * pBus = (CAkBus *)m_pBusOutputNode;
	return pBus && pBus->GetStateSyncTypes( in_stateGroupID, io_pSyncTypes );
}

/** 2012 0x968220, 2013 0x964ad0: which sub-track a new segment context plays. */
AkUInt16 CAkMusicTrack::GetNextRS()
{
	switch( m_eRSType )
	{
	case AkMusicTrackRanSeqType_Random:
		return m_uNumSubTrack ? (AkUInt16)( rand() % m_uNumSubTrack ) : 0;
	case AkMusicTrackRanSeqType_Sequence:
		if( ++m_SequenceIndex >= m_uNumSubTrack )
		{
			m_SequenceIndex = 0;
		}
		return m_SequenceIndex;
	default:
		return 0;
	}
}

/** 2012 0x968390, 2013 0x964c40. */
bool CAkMusicTrack::HasBankSource()
{
	for( MapStruct<AkUniqueID, CAkMusicSource *> * it = m_arSrcInfo.m_pItems; it != m_arSrcInfo.m_pItemsEnd; ++it )
	{
		if( it->item->m_sSrcTypeInfo.mediaInfo.bHasSource )
		{
			return true;
		}
	}
	return false;
}

/** 2012 0x9683c0, 2013 0x964c70: the authored look-ahead (ms) scaled by AkMusicSettings::fStreamingLookAheadRatio, in samples. */
void CAkMusicTrack::LookAheadTime( AkTimeMs in_LookAheadTime )
{
	const AkReal64 v = (AkReal64)in_LookAheadTime * CAkMusicRenderer::m_musicSettings.fStreamingLookAheadRatio * (AkReal64)AkAudioLibSettings::g_pipelineCoreFrequency * 0.001;
	m_iLookAheadTime = (AkInt32)( v + ( v <= 0.0 ? -0.5 : 0.5 ) );
	for( MapStruct<AkUniqueID, CAkMusicSource *> * it = m_arSrcInfo.m_pItems; it != m_arSrcInfo.m_pItemsEnd; ++it )
	{
		it->item->m_uStreamingLookAhead = m_iLookAheadTime;
	}
}

/** 2012 0x968430, 2013 0x964ce0. */
AKRESULT CAkMusicTrack::PrepareData()
{
	AKRESULT eResult = AK_Success;
	MapStruct<AkUniqueID, CAkMusicSource *> * it = m_arSrcInfo.m_pItems;
	for( ; it != m_arSrcInfo.m_pItemsEnd; ++it )
	{
		eResult = it->item->PrepareData();
		if( eResult != AK_Success )
		{
			break;
		}
	}
	if( eResult != AK_Success )
	{
		for( MapStruct<AkUniqueID, CAkMusicSource *> * pUndo = m_arSrcInfo.m_pItems; pUndo != it; ++pUndo )
		{
			pUndo->item->UnPrepareData();
		}
	}
	return eResult;
}

/** 2012 0x952990. */
void CAkMusicTrack::UnPrepareData()
{
	for( MapStruct<AkUniqueID, CAkMusicSource *> * it = m_arSrcInfo.m_pItems; it != m_arSrcInfo.m_pItemsEnd; ++it )
	{
		it->item->UnPrepareData();
	}
}

/** 2012 0x968330, 2013 0x964be0. */
void CAkMusicTrack::ClipAutomationArray::Term()
{
	if( !m_pItems )
	{
		return;
	}
	for( CAkClipAutomation * it = m_pItems; it != m_pItemsEnd; ++it )
	{
		it->m_tableAutomation.Unset();
	}
	m_pItemsEnd = m_pItems;
	AK::MemoryMgr::Free( g_DefaultPoolId, m_pItems );
	m_pItems = NULL;
	m_pItemsEnd = NULL;
	m_ulReserved = 0;
}

/** 2012 0x968560, 2013 0x964d30: the curve's x axis is converted from seconds to samples (rounded to nearest); volume curves are dB. */
AKRESULT CAkClipAutomation::Set( AkUInt32 in_uClipIndex, AkClipAutomationType in_eAutoType, AkRTPCGraphPointBase<AkReal32> * in_arPoints, AkUInt32 in_uNumPoints )
{
	m_tableAutomation.Unset();
	m_uClipIndex = in_uClipIndex;
	m_eAutoType = in_eAutoType;
	if( in_uNumPoints && m_tableAutomation.Set( in_arPoints, in_uNumPoints, in_eAutoType != AutomationType_LPF ? AkCurveScaling_dB_96_3_NoCheck : AkCurveScaling_None ) == AK_Success )
	{
		for( AkUInt32 i = 0; i < in_uNumPoints; ++i )
		{
			const AkReal64 v = m_tableAutomation.m_pArrayGraphPoints[ i ].From * (AkReal64)AkAudioLibSettings::g_pipelineCoreFrequency;
			m_tableAutomation.m_pArrayGraphPoints[ i ].From = (AkReal32)(AkInt32)( v + ( v <= 0.0 ? -0.5 : 0.5 ) );
		}
	}
	return AK_Success;
}

/** Inlined in CAkSegmentCtx::ScheduleAudioClips (2012 0x966e30): the automation of one clip and type, if any. */
CAkClipAutomation * CAkMusicTrack::GetClipAutomation( AkUInt32 in_uClipIndex, AkClipAutomationType in_eAutomationType )
{
	for( CAkClipAutomation * it = m_arClipAutomation.m_pItems; it != m_arClipAutomation.m_pItemsEnd; ++it )
	{
		if( it->m_uClipIndex == in_uClipIndex && it->m_eAutoType == in_eAutomationType )
		{
			return it;
		}
	}
	return NULL;
}

/** 2012 0x9686b0, 2013 0x964e80: ms to samples; a clip with no audible length is skipped. */
AKRESULT CAkMusicTrack::AddPlaylistItem( AkTrackSrcInfo & in_srcInfo )
{
	const AkReal64 fClipDuration = in_srcInfo.fEndTrimOffset + in_srcInfo.fSrcDuration - in_srcInfo.fBeginTrimOffset;
	if( fClipDuration <= 0.0 )
	{
		return AK_Success;
	}
	AkTrackSrc * pSrc = m_arTrackPlaylist.AddLast();
	if( !pSrc )
	{
		return AK_Fail;
	}
	pSrc->uSubTrackIndex = in_srcInfo.trackID;
	pSrc->srcID = in_srcInfo.sourceID;
	pSrc->uClipStartPosition = (AkUInt32)AkTimeConv::MillisecondsToSamples( in_srcInfo.fPlayAt + in_srcInfo.fBeginTrimOffset );
	pSrc->uClipDuration = (AkUInt32)AkTimeConv::MillisecondsToSamples( fClipDuration );
	const AkUInt32 uSrcDuration = (AkUInt32)AkTimeConv::MillisecondsToSamples( in_srcInfo.fSrcDuration );
	pSrc->uSrcDuration = uSrcDuration;
	pSrc->iSourceTrimOffset = (AkInt32)( (AkUInt32)AkTimeConv::MillisecondsToSamples( in_srcInfo.fBeginTrimOffset ) % uSrcDuration );
	if( pSrc->iSourceTrimOffset < 0 )
	{
		pSrc->iSourceTrimOffset += uSrcDuration;
	}
	return AK_Success;
}

/** 2012 0x968790, 2013 0x964f60. */
void CAkMusicTrack::RemoveAllSources()
{
	m_uNumSubTrack = 0;
	m_arTrackPlaylist.m_pItemsEnd = m_arTrackPlaylist.m_pItems;
	for( MapStruct<AkUniqueID, CAkMusicSource *> * it = m_arSrcInfo.m_pItems; it != m_arSrcInfo.m_pItemsEnd; ++it )
	{
		if( it->item )
		{
			it->item->~CAkMusicSource();
			AK::MemoryMgr::Free( g_DefaultPoolId, it->item );
		}
	}
	m_arSrcInfo.m_pItemsEnd = m_arSrcInfo.m_pItems;
}

/** 2012 0x968860, 2013 0x9650b0. */
AKRESULT CAkMusicTrack::SetPlayList( AkUInt32 in_uNumPlaylistItem, AkTrackSrcInfo * in_pArrayPlaylistItems, AkUInt32 in_uNumSubTrack )
{
	m_arTrackPlaylist.Term();
	m_uNumSubTrack = in_uNumSubTrack;
	if( !in_uNumPlaylistItem )
	{
		return AK_Success;
	}
	if( m_arTrackPlaylist.Reserve( in_uNumPlaylistItem ) != AK_Success )
	{
		return AK_Fail;
	}
	for( AkUInt32 i = 0; i < in_uNumPlaylistItem; ++i )
	{
		AddPlaylistItem( in_pArrayPlaylistItems[ i ] );
	}
	return AK_Success;
}

/** 2012 0x9688f0, 2013 0x965140: one CAkMusicSource per source id (a second add of the same id is ignored). */
AKRESULT CAkMusicTrack::AddSource( AkUniqueID in_srcID, AkUInt32 in_pluginID, AkMediaInformation in_MediaInfo )
{
	if( m_arSrcInfo.Exists( in_srcID ) )
	{
		return AK_Success;
	}
	CAkMusicSource ** ppSource = m_arSrcInfo.Set( in_srcID );
	if( !ppSource )
	{
		return AK_Fail;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicSource ) );
	*ppSource = pMem ? AkPlacementNew( pMem ) CAkMusicSource : NULL;
	if( *ppSource )
	{
		( *ppSource )->SetSource( in_pluginID, in_MediaInfo );
		( *ppSource )->m_uStreamingLookAhead = m_iLookAheadTime;
	}
	else
	{
		m_arSrcInfo.Unset( in_srcID );
		return AK_Fail;
	}
	return AK_Success;
}

/** 2012 0x9689e0, 2013 0x965230. */
AKRESULT CAkMusicTrack::AddPluginSource( AkUniqueID in_srcID )
{
	CAkMusicSource ** ppSource = m_arSrcInfo.Set( in_srcID );
	if( !ppSource )
	{
		return AK_Fail;
	}
	void * pMem = AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( CAkMusicSource ) );
	*ppSource = pMem ? AkPlacementNew( pMem ) CAkMusicSource : NULL;
	if( *ppSource )
	{
		( *ppSource )->SetSource( in_srcID );
	}
	else
	{
		m_arSrcInfo.Unset( in_srcID );
		return AK_Fail;
	}
	return AK_Success;
}

/** 2012 0x968a60, 2013 0x9652b0. */
CAkMusicSource * CAkMusicTrack::GetSourcePtr( AkUniqueID SourceID )
{
	CAkMusicSource ** ppSource = m_arSrcInfo.Exists( SourceID );
	return ppSource ? *ppSource : NULL;
}

/**
 * 2012 0x968ab0: the sources, the clips (+ the sub-track count), the clip automation, the node base params, then
 * (a full load only) the random/sequence type and the look-ahead. The track id at the start is skipped.
 */
AKRESULT CAkMusicTrack::SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize, CAkUsageSlot * /*in_pUsageSlot*/, bool in_bIsPartialLoadOnly )
{
	in_pData += 4;
	const AkUInt32 uNumSources = *(AkUInt32 *)in_pData;
	in_pData += 4;
	AKRESULT eResult = AK_Success;
	for( AkUInt32 i = 0; i < uNumSources; ++i )
	{
		if( eResult != AK_Success )
		{
			continue;
		}
		AkBankSourceData oSourceInfo;
		eResult = CAkBankMgr::LoadSource( in_pData, in_ulDataSize, oSourceInfo );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		eResult = oSourceInfo.m_pParam ? AddPluginSource( oSourceInfo.m_MediaInfo.sourceID ) : AddSource( oSourceInfo.m_MediaInfo.sourceID, oSourceInfo.m_PluginID, oSourceInfo.m_MediaInfo );
		if( eResult != AK_Success )
		{
			return AK_Fail;
		}
	}
	const AkUInt32 uNumPlaylistItems = *(AkUInt32 *)in_pData;
	in_pData += 4;
	if( uNumPlaylistItems )
	{
		AkTrackSrcInfo * pPlaylist = (AkTrackSrcInfo *)AK::MemoryMgr::Malloc( g_DefaultPoolId, sizeof( AkTrackSrcInfo ) * uNumPlaylistItems );
		if( !pPlaylist )
		{
			return AK_InsufficientMemory;
		}
		for( AkUInt32 i = 0; i < uNumPlaylistItems; ++i )
		{
			pPlaylist[ i ].trackID = *(AkUInt32 *)in_pData;
			in_pData += 4;
			pPlaylist[ i ].sourceID = *(AkUInt32 *)in_pData;
			in_pData += 4;
			pPlaylist[ i ].fPlayAt = *(AkReal64 *)in_pData;
			in_pData += 8;
			pPlaylist[ i ].fBeginTrimOffset = *(AkReal64 *)in_pData;
			in_pData += 8;
			pPlaylist[ i ].fEndTrimOffset = *(AkReal64 *)in_pData;
			in_pData += 8;
			pPlaylist[ i ].fSrcDuration = *(AkReal64 *)in_pData;
			in_pData += 8;
		}
		const AkUInt32 uNumSubTrack = *(AkUInt32 *)in_pData;
		in_pData += 4;
		eResult = SetPlayList( uNumPlaylistItems, pPlaylist, uNumSubTrack );
		AK::MemoryMgr::Free( g_DefaultPoolId, pPlaylist );
	}
	if( eResult != AK_Success )
	{
		return eResult;
	}
	m_arClipAutomation.Term();
	const AkUInt32 uNumClipAutomationItem = *(AkUInt32 *)in_pData;
	in_pData += 4;
	if( uNumClipAutomationItem )
	{
		if( m_arClipAutomation.Reserve( uNumClipAutomationItem ) != AK_Success )
		{
			return AK_Fail;
		}
		for( AkUInt32 i = 0; i < uNumClipAutomationItem; ++i )
		{
			CAkClipAutomation * pClipAutomation = m_arClipAutomation.AddLast();
			const AkUInt32 uClipIndex = *(AkUInt32 *)in_pData;
			in_pData += 4;
			const AkClipAutomationType eAutoType = (AkClipAutomationType)*(AkUInt32 *)in_pData;
			in_pData += 4;
			const AkUInt32 uNumPoints = *(AkUInt32 *)in_pData;
			in_pData += 4;
			if( uNumPoints )
			{
				if( pClipAutomation->Set( uClipIndex, eAutoType, (AkRTPCGraphPointBase<AkReal32> *)in_pData, uNumPoints ) != AK_Success )
				{
					return AK_Fail;
				}
				in_pData += sizeof( AkRTPCGraphPointBase<AkReal32> ) * uNumPoints;
			}
		}
	}
	eResult = SetNodeBaseParams( in_pData, in_ulDataSize, in_bIsPartialLoadOnly );
	if( !in_bIsPartialLoadOnly && eResult == AK_Success )
	{
		m_eRSType = (AkMusicTrackRanSeqType)*(AkUInt32 *)in_pData;
		in_pData += 4;
		const AkTimeMs lookAheadTime = *(AkInt32 *)in_pData;
		in_pData += 4;
		LookAheadTime( lookAheadTime );
		g_pHircParseEnd = in_pData;
	}
	return eResult;
}
