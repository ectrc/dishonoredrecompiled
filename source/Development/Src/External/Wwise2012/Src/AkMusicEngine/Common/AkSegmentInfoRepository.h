/*=============================================================================
	AkSegmentInfoRepository.h - CAkSegmentInfoRepository (aksegmentinforepository.cpp): the last segment info of each
	playing id registered with AK_EnableGetMusicPlayPosition, stamped with the performance counter, read by
	AK::MusicEngine::GetPlayingSegmentInfo from any thread (extrapolated from the stamp when asked).

	DISHONORED(port): agent IH4 (wave 26). Layout: the key array + CAkLock (2012 PDB); records are 40 bytes (key, the
	20-byte AkSegmentInfo, the 64-bit stamp at +8 of the item).
=============================================================================*/
#pragma once

#include <AkPrivateTypes.h>
#include <AK/SoundEngine/Common/AkTypes.h>

struct AkSegmentInfoRecord
{
	AkSegmentInfo	segmentInfo;
	AkInt64			timeUpdated;
};

class CAkSegmentInfoRepository
{
public:
	CAkSegmentInfoRepository() {}
	~CAkSegmentInfoRepository() {}

	AKRESULT CreateEntry( AkPlayingID in_PlayingID );
	void UpdateSegmentInfo( AkPlayingID in_PlayingID, const AkSegmentInfo & in_segmentInfo );
	AKRESULT GetSegmentInfo( AkPlayingID in_PlayingID, AkSegmentInfo & out_info, bool in_bExtrapolate );
	void RemoveEntry( AkPlayingID in_PlayingID );
	/** 2012 0x962300, 2013 0x940a10. */
	void Term() { m_mapSegmentInfo.Term(); }

	CAkKeyArray<AkPlayingID, AkSegmentInfoRecord, 1>	m_mapSegmentInfo;
	CAkLock												m_lock;
};

static_assert( sizeof( MapStruct<AkPlayingID, AkSegmentInfoRecord> ) == 40, "MapStruct<AkPlayingID, AkSegmentInfoRecord>: 2012 PDB 40" );
