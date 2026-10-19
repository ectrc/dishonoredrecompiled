/*=============================================================================
	AkPositionRepository.cpp - CAkPositionRepository (akpositionrepository.cpp, 2012 0x9287a0..0x9289e0; 2013
	0x924e50..0x9250f0 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26). g_pPositionRepository is defined here; creating it is
	AK::SoundEngine::PreInit's (see the header).
=============================================================================*/
#include "AkPositionRepository.h"

CAkPositionRepository * g_pPositionRepository = NULL;

/** 2012 0x9287a0; 2013 0x924e50: room for 8 playing IDs. */
AKRESULT CAkPositionRepository::Init()
{
	return m_mapPosInfo.Reserve( 8 );
}

/** 2012 0x9288a0; 2013 0x924fb0. */
void CAkPositionRepository::AddSource( AkPlayingID in_PlayingID, void * in_cookie )
{
	if( m_mapPosInfo.Exists( in_PlayingID ) )
	{
		return;
	}
	AkAutoLock<CAkLock> lock( m_lock );
	MapStruct<AkPlayingID, AkPositionInfo> * pEntry = m_mapPosInfo.AddLast();
	if( pEntry )
	{
		pEntry->key = in_PlayingID;
		pEntry->item.cookie = in_cookie;
		pEntry->item.bufferPosInfo.Clear();
	}
}

/** 2012 0x9287e0; 2013 0x924ef0: only the source that added the entry removes it. */
void CAkPositionRepository::RemoveSource( AkPlayingID in_PlayingID, void * in_cookie )
{
	MapStruct<AkPlayingID, AkPositionInfo> * pEntry = m_mapPosInfo.FindEx( in_PlayingID );
	if( pEntry && pEntry->item.cookie == in_cookie )
	{
		AkAutoLock<CAkLock> lock( m_lock );
		m_mapPosInfo.Erase( (AkUInt32)( pEntry - m_mapPosInfo.m_pItems ) );
	}
}

/** 2012 0x9624e0 (folded with CAkSegmentInfoRepository::RemoveEntry; read from the shipping executable's bytes): the
	playing ID ended. */
void CAkPositionRepository::RemovePlayingID( AkPlayingID in_PlayingID )
{
	AkAutoLock<CAkLock> lock( m_lock );
	m_mapPosInfo.Unset( in_PlayingID );
}

/** 2012 0x928940; 2013 0x925050. */
void CAkPositionRepository::UpdatePositionInfo( AkPlayingID in_PlayingID, AkBufferPosInformation * in_pPosInfo, void * in_cookie )
{
	AkPositionInfo * pPosInfo = m_mapPosInfo.Exists( in_PlayingID );
	if( !pPosInfo )
	{
		pPosInfo = m_mapPosInfo.Set( in_PlayingID );
		if( !pPosInfo )
		{
			return;
		}
		if( !m_mapPosInfo.IsEmpty() )
		{
			UpdateTime();
		}
		pPosInfo->cookie = in_cookie;
	}
	else if( in_cookie != pPosInfo->cookie )
	{
		return;
	}
	AkAutoLock<CAkLock> lock( m_lock );
	pPosInfo->bufferPosInfo = *in_pPosInfo;
	pPosInfo->timeUpdated = m_i64LastTimeUpdated;
}

/** 2012 0x9289e0; 2013 0x9250f0: ms, extrapolated from the last update at the last rate, clamped to the file end. */
AKRESULT CAkPositionRepository::GetCurrPosition( AkPlayingID in_PlayingID, AkTimeMs * out_puPos, bool in_bExtrapolate )
{
	AkAutoLock<CAkLock> lock( m_lock );
	AkPositionInfo * pPosInfo = m_mapPosInfo.Exists( in_PlayingID );
	if( !pPosInfo || !pPosInfo->bufferPosInfo.uSampleRate )
	{
		*out_puPos = 0;
		return AK_Fail;
	}
	const AkReal64 dSampleRate = (AkReal64)pPosInfo->bufferPosInfo.uSampleRate;
	const AkReal32 fStartPos = (AkReal32)( (AkReal64)pPosInfo->bufferPosInfo.uStartPos * 1000.0 / dSampleRate );
	const AkUInt32 uFileEnd = (AkUInt32)(AkInt64)( 1000.0 * (AkReal64)pPosInfo->bufferPosInfo.uFileEnd / dSampleRate );
	AkReal64 dPosition = fStartPos;
	if( in_bExtrapolate )
	{
		AkInt64 iCurrTime;
		AKPLATFORM::PerformanceCounter( &iCurrTime );
		dPosition = (AkReal64)( iCurrTime - pPosInfo->timeUpdated ) / AK::g_fFreqRatio * pPosInfo->bufferPosInfo.fLastRate + fStartPos;
	}
	AkUInt32 uPosition = (AkUInt32)(AkInt64)dPosition;
	if( uPosition >= uFileEnd )
	{
		uPosition = uFileEnd;
	}
	*out_puPos = (AkTimeMs)uPosition;
	return AK_Success;
}

/** 2012 0x928840; 2013 0x924f50: a pause (rate 0) or a resume restarts the extrapolation from the last frame time. */
void CAkPositionRepository::SetRate( AkPlayingID in_PlayingID, AkReal32 in_fNewRate )
{
	AkPositionInfo * pPosInfo = m_mapPosInfo.Exists( in_PlayingID );
	if( pPosInfo )
	{
		AkAutoLock<CAkLock> lock( m_lock );
		pPosInfo->timeUpdated = m_i64LastTimeUpdated;
		pPosInfo->bufferPosInfo.fLastRate = in_fNewRate;
	}
}
