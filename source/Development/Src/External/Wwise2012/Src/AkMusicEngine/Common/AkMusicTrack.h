/*=============================================================================
	AkMusicTrack.h - CAkMusicTrack (akmusictrack.cpp): a track of a music segment - its sources (CAkMusicSource, a
	CAkSource with a streaming look-ahead), its clips (AkTrackSrc, in samples, per sub-track) and clip automation
	(CAkClipAutomation: volume / LPF / fade-in / fade-out curves over the clip), and the random / sequence choice of the
	sub-track played by each segment context.

	DISHONORED(port): agent IH4 (wave 26). Layouts: CAkMusicSource 0x24, CAkClipAutomation 20, CAkMusicTrack 0x80 (2012
	PDB; Create mallocs 0x80, AddSource 0x24). CAkMusicTrack derives CAkSoundBase (76 bytes) as retail.
=============================================================================*/
#pragma once

#include "AkMusicStructs.h"
#include <AkHierarchy.h>

class CAkMusicSource : public CAkSource
{
public:
	CAkMusicSource() : m_uStreamingLookAhead( 0 ) {}
	virtual ~CAkMusicSource() {}

	AkUInt32 m_uStreamingLookAhead;
};

class CAkClipAutomation
{
public:
	CAkClipAutomation() : m_uClipIndex( 0 ), m_eAutoType( AutomationType_Volume ) {}

	AKRESULT Set( AkUInt32 in_uClipIndex, AkClipAutomationType in_eAutoType, AkRTPCGraphPointBase<AkReal32> * in_arPoints, AkUInt32 in_uNumPoints );

	AkUInt32												m_uClipIndex;
	AkClipAutomationType									m_eAutoType;
	CAkConversionTable<AkRTPCGraphPointBase<AkReal32>, AkReal32>	m_tableAutomation;
};

class CAkMusicTrack : public CAkSoundBase
{
public:
	class ClipAutomationArray : public AkArray<CAkClipAutomation, const CAkClipAutomation &, ArrayPoolDefault, 1>
	{
	public:
		void Term();
	};

	static CAkMusicTrack * Create( AkUniqueID in_ulID );

	AKRESULT SetInitialValues( AkUInt8 * in_pData, AkUInt32 in_ulDataSize, CAkUsageSlot * in_pUsageSlot, bool in_bIsPartialLoadOnly );

	virtual AkNodeCategory NodeCategory() { return AkNodeCategory_MusicTrack; }
	virtual AKRESULT ExecuteAction( ActionParams & in_rAction );
	virtual AKRESULT ExecuteActionExcept( ActionParamsExcept & in_rAction );
	virtual AKRESULT PrepareData();
	virtual void UnPrepareData();

	bool GetStateSyncTypes( AkStateGroupID in_stateGroupID, CAkStateSyncArray * io_pSyncTypes );
	AkUInt16 GetNextRS();
	bool HasBankSource();
	void LookAheadTime( AkTimeMs in_LookAheadTime );
	AKRESULT SetPlayList( AkUInt32 in_uNumPlaylistItem, AkTrackSrcInfo * in_pArrayPlaylistItems, AkUInt32 in_uNumSubTrack );
	AKRESULT AddPlaylistItem( AkTrackSrcInfo & in_srcInfo );
	AKRESULT AddSource( AkUniqueID in_srcID, AkUInt32 in_pluginID, AkMediaInformation in_MediaInfo );
	AKRESULT AddPluginSource( AkUniqueID in_srcID );
	void RemoveAllSources();
	CAkMusicSource * GetSourcePtr( AkUniqueID SourceID );
	CAkClipAutomation * GetClipAutomation( AkUInt32 in_uClipIndex, AkClipAutomationType in_eAutomationType );

	CAkKeyArray<AkUniqueID, CAkMusicSource *, 8>			m_arSrcInfo;
	ClipAutomationArray										m_arClipAutomation;
	AkUInt32												m_uNumSubTrack;
	AkArray<AkTrackSrc, const AkTrackSrc &, ArrayPoolDefault, 1>	m_arTrackPlaylist;
	AkInt32													m_iLookAheadTime;
	AkMusicTrackRanSeqType									m_eRSType;
	AkUInt16												m_SequenceIndex;

protected:
	explicit CAkMusicTrack( AkUniqueID in_ulID );
	virtual ~CAkMusicTrack();
};

static_assert( sizeof( CAkMusicSource ) == 0x24, "CAkMusicSource: 2012 PDB 0x24 (AddSource mallocs 0x24)" );
static_assert( sizeof( CAkClipAutomation ) == 20, "CAkClipAutomation: 2012 PDB 20" );
static_assert( sizeof( CAkMusicTrack ) == 0x80, "CAkMusicTrack: 2012 PDB 0x80 (Create mallocs 0x80)" );
