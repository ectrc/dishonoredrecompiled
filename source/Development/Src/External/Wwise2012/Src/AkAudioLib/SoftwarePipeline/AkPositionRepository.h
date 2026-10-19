/*=============================================================================
	AkPositionRepository.h - CAkPositionRepository (akpositionrepository.cpp): the last buffer position of every
	playing ID that registered AK_EnableGetSourcePlayPosition, extrapolated with the performance counter on query.

	DISHONORED(port): agent IH2 fork S (wave 26). Layout is the 2012 PDB's, 0x30 bytes - the same size and the same
	(empty-array) constructor as CAkSegmentInfoRepository, so retail's linker folded the two: AK::SoundEngine::PreInit
	(2012 0x919cc0) mallocs 0x30 from g_DefaultPoolId, calls CAkSegmentInfoRepository::CAkSegmentInfoRepository on it and
	stores it as g_pPositionRepository, and AK::SoundEngine::Term (0x91a090) runs the folded AkArray::Term, deletes
	m_lock and frees it. CAkPlayingMgr::CheckRemovePlayingID (0x928590) calls the folded
	CAkSegmentInfoRepository::RemoveEntry (0x9624e0) on it - RemovePlayingID here.
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"

struct __declspec( align( 8 ) ) AkPositionInfo
{
	AkBufferPosInformation bufferPosInfo;
	AkInt64 timeUpdated;
	void * cookie;
};

class CAkPositionRepository
{
public:
	AKRESULT Init();
	void Term() { m_mapPosInfo.Term(); }
	void AddSource( AkPlayingID in_PlayingID, void * in_cookie );
	void RemoveSource( AkPlayingID in_PlayingID, void * in_cookie );
	void RemovePlayingID( AkPlayingID in_PlayingID );
	void UpdatePositionInfo( AkPlayingID in_PlayingID, AkBufferPosInformation * in_pPosInfo, void * in_cookie );
	AKRESULT GetCurrPosition( AkPlayingID in_PlayingID, AkTimeMs * out_puPos, bool in_bExtrapolate );
	void SetRate( AkPlayingID in_PlayingID, AkReal32 in_fNewRate );
	void UpdateTime() { AKPLATFORM::PerformanceCounter( &m_i64LastTimeUpdated ); }

	CAkKeyArray<AkPlayingID, AkPositionInfo, 1> m_mapPosInfo;
	CAkLock m_lock;
	AkInt64 m_i64LastTimeUpdated;
};
static_assert( sizeof( CAkPositionRepository ) == 0x30, "CAkPositionRepository: PreInit mallocs 0x30" );

extern CAkPositionRepository * g_pPositionRepository;
