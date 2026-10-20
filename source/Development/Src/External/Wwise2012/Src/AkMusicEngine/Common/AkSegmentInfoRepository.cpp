/*=============================================================================
	AkSegmentInfoRepository.cpp - CAkSegmentInfoRepository (aksegmentinforepository.cpp, 2012 0x962200..0x9624e0; its
	key array's Set / Unset / GrowArray 0x962210 / 0x9622a0 / 0x962150 are this tree's CAkKeyArray template).

	DISHONORED(port): agent IH4 (wave 26).
=============================================================================*/
#include "AkSegmentInfoRepository.h"

/** 2012 0x962340, 2013 0x95eb80: a zeroed record, stamped now. */
AKRESULT CAkSegmentInfoRepository::CreateEntry( AkPlayingID in_PlayingID )
{
	AkAutoLock<CAkLock> lock( m_lock );
	AkSegmentInfoRecord * pRecord = m_mapSegmentInfo.Set( in_PlayingID );
	if( !pRecord )
	{
		return AK_Fail;
	}
	pRecord->segmentInfo.iCurrentPosition = 0;
	pRecord->segmentInfo.iPreEntryDuration = 0;
	pRecord->segmentInfo.iActiveDuration = 0;
	pRecord->segmentInfo.iPostExitDuration = 0;
	pRecord->segmentInfo.iRemainingLookAheadTime = 0;
	QueryPerformanceCounter( (LARGE_INTEGER *)&pRecord->timeUpdated );
	return AK_Success;
}

/** 2012 0x9623a0, 2013 0x95ebe0. */
void CAkSegmentInfoRepository::UpdateSegmentInfo( AkPlayingID in_PlayingID, const AkSegmentInfo & in_segmentInfo )
{
	AkAutoLock<CAkLock> lock( m_lock );
	AkSegmentInfoRecord * pRecord = m_mapSegmentInfo.Exists( in_PlayingID );
	if( pRecord )
	{
		pRecord->segmentInfo = in_segmentInfo;
		QueryPerformanceCounter( (LARGE_INTEGER *)&pRecord->timeUpdated );
	}
}

/** 2012 0x962410, 2013 0x95ec50: the record, its position advanced by the time since its stamp when extrapolating a real segment. */
AKRESULT CAkSegmentInfoRepository::GetSegmentInfo( AkPlayingID in_PlayingID, AkSegmentInfo & out_info, bool in_bExtrapolate )
{
	AkAutoLock<CAkLock> lock( m_lock );
	AkSegmentInfoRecord * pRecord = m_mapSegmentInfo.Exists( in_PlayingID );
	if( !pRecord )
	{
		return AK_Fail;
	}
	out_info = pRecord->segmentInfo;
	if( in_bExtrapolate && ( out_info.iActiveDuration > 0 || out_info.iPreEntryDuration > 0 || out_info.iPostExitDuration > 0 ) )
	{
		AkInt64 currTime;
		QueryPerformanceCounter( (LARGE_INTEGER *)&currTime );
		currTime -= pRecord->timeUpdated;
		out_info.iCurrentPosition += (AkInt32)( (AkReal64)currTime / AK::g_fFreqRatio );
	}
	return AK_Success;
}

/** 2012 0x9624e0, 2013 0x95ed20. */
void CAkSegmentInfoRepository::RemoveEntry( AkPlayingID in_PlayingID )
{
	AkAutoLock<CAkLock> lock( m_lock );
	m_mapSegmentInfo.Unset( in_PlayingID );
}
