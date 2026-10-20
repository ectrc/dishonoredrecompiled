/*=============================================================================
	AkMusicStructs.h - the interactive-music data the bank and the music contexts share (akmusicstructs.h,
	akmusictransaware.h, akmusicsegment.h, akmusictrack.h, privatestructures.h of
	d:\branches\wwise_v2012.1\wwise\sdk\source\soundengine\akmusicengine\common).

	DISHONORED(port): agent IH4 (wave 26). Every layout is the 2012 PDB's (resources/docs/types/all_types.h), sizes
	static_asserted below. AkMusicTransDestRule really holds a second AkMusicFade that shadows the base's: the bank
	reader (CAkMusicTransAware::SetRules 0x96c5a0) writes the destination fade at rule+44 and CAkChainCtx::Prepare passes
	&destRule+16 to CAkScheduledItem::Prepare, so the base fade of a destination rule is never read (measured in the
	2012 disassembly of SetRules: [esi+2Ch]/[esi+30h]/[esi+34h]).
=============================================================================*/
#pragma once

#include <AkAudioLibGlobals.h>
#include <AkPrivateTypes.h>

#if defined(_MSC_VER)
#pragma pack( push, 8 )
#endif

enum AkSyncType
{
	SyncTypeImmediate		= 0,
	SyncTypeNextGrid		= 1,
	SyncTypeNextBar			= 2,
	SyncTypeNextBeat		= 3,
	SyncTypeNextMarker		= 4,
	SyncTypeNextUserMarker	= 5,
	SyncTypeEntryMarker		= 6,
	SyncTypeExitMarker		= 7
};

enum AkEntryType
{
	EntryTypeEntryMarker		= 0,
	EntryTypeSameTime			= 1,
	EntryTypeRandomMarker		= 2,
	EntryTypeRandomUserMarker	= 3
};

enum AkMusicActionType
{
	MusicActionTypePlay	= 0,
	MusicActionTypeStop	= 1
};

enum AkClipAutomationType
{
	AutomationType_Volume	= 0,
	AutomationType_LPF		= 1,
	AutomationType_FadeIn	= 2,
	AutomationType_FadeOut	= 3
};

enum AkMusicTrackRanSeqType
{
	AkMusicTrackRanSeqType_Normal	= 0,
	AkMusicTrackRanSeqType_Random	= 1,
	AkMusicTrackRanSeqType_Sequence	= 2
};

enum AkAssociatedActionType
{
	AssocActionTypeStinger	= 0,
	AssocActionTypeState	= 1
};

enum RSType
{
	RSType_ContinuousSequence	= 0,
	RSType_StepSequence			= 1,
	RSType_ContinuousRandom		= 2,
	RSType_StepRandom			= 3
};

/** 16 bytes: grid of a music node in samples (CAkMusicNode::MeterInfo). */
struct AkMusicGrid
{
	AkUInt32	uBeatDuration;
	AkUInt32	uBarDuration;
	AkUInt32	uGridDuration;
	AkUInt32	uGridOffset;
};

/** 28 bytes: the meter as the bank writes it (grid period / offset in ms, tempo, time signature). */
struct AkMeterInfo
{
	AkReal64	fGridPeriod;
	AkReal64	fGridOffset;
	AkReal32	fTempo;
	AkUInt8		uTimeSigNumBeatsBar;
	AkUInt8		uTimeSigBeatValue;
};

struct AkMusicFade
{
	AkTimeMs				transitionTime;
	AkCurveInterpolation	eFadeCurve;
	AkInt32					iFadeOffset;
};

struct AkMusicTransRuleBase
{
	AkMusicFade	fadeParams;
	AkUInt32	uCueFilterHash;
};

struct AkMusicTransDestRule : public AkMusicTransRuleBase
{
	AkMusicFade	fadeParams;
	AkUniqueID	uJumpToID;
	AkUInt16	eEntryType : 3;
	AkUInt16	bPlayPreEntry : 1;
	AkUInt16	bDestMatchSourceCueName : 1;
};

struct AkMusicTransSrcRule : public AkMusicTransRuleBase
{
	AkUInt32	eSyncType : 5;
	AkUInt32	bPlayPostExit : 1;
};

struct AkMusicTransitionObject
{
	AkUniqueID	segmentID;
	AkMusicFade	fadeInParams;
	AkMusicFade	fadeOutParams;
	AkUInt32	bPlayPreEntry : 1;
	AkUInt32	bPlayPostExit : 1;
};

class AkMusicTransitionRule
{
public:
	AkMusicTransitionRule() : pTransObj( NULL ) {}

	/** 2012 0x96c160, 2013 0x968980. */
	~AkMusicTransitionRule()
	{
		if( pTransObj )
		{
			AK::MemoryMgr::Free( g_DefaultPoolId, pTransObj );
		}
	}

	AkUniqueID					srcID;
	AkUniqueID					destID;
	AkMusicTransSrcRule			srcRule;
	AkMusicTransDestRule		destRule;
	AkMusicTransitionObject *	pTransObj;
};

/** 92 bytes: one transition rule as the bank stores it (read by CAkMusicTransAware::SetMusicTransNodeParams). */
struct AkWwiseMusicTransitionRule
{
	AkUniqueID	srcID;
	AkUniqueID	destID;
	AkMusicFade	srcFade;
	AkUInt32	eSrcSyncType;
	AkUInt32	uSrcCueFilterHash;
	AkUInt8		bSrcPlayPostExit;
	AkMusicFade	destFade;
	AkUInt32	uDestCueFilterHash;
	AkUniqueID	uDestJumpToID;
	AkUInt16	eDestEntryType;
	AkUInt8		bDestPlayPreEntry;
	AkUInt8		bDestMatchSourceCueName;
	AkUInt8		bIsTransObjectEnabled;
	AkUniqueID	segmentID;
	AkMusicFade	transFadeIn;
	AkMusicFade	transFadeOut;
	AkUInt8		bPlayPreEntry;
	AkUInt8		bPlayPostExit;
};

/** 12 bytes; the name is owned (AkArray<AkMusicMarker,..,0>::RemoveAll 0x9664d0 frees it). */
class AkMusicMarker
{
public:
	AkMusicMarker() : id( 0 ), uPosition( 0 ), pszName( NULL ) {}

	~AkMusicMarker()
	{
		if( pszName )
		{
			AK::MemoryMgr::Free( g_DefaultPoolId, pszName );
		}
	}

	AkUniqueID	id;
	AkUInt32	uPosition;
	char *		pszName;
};

/** 24 bytes: a marker as the bank stores it (position in ms). */
struct AkMusicMarkerWwise
{
	AkUniqueID	id;
	AkReal64	fPosition;
	char *		pszName;
};

/** 24 bytes. */
class CAkStinger
{
public:
	AkTriggerID	m_TriggerID;
	AkUniqueID	m_SegmentID;
	AkSyncType	m_SyncPlayAt;
	AkUInt32	m_uCueFilterHash;
	AkTimeMs	m_DontRepeatTime;
	AkUInt32	m_numSegmentLookAhead;
};

/** 24 bytes: one clip of a music track, in samples (CAkMusicTrack::AddPlaylistItem). */
struct AkTrackSrc
{
	AkUInt32	uSubTrackIndex;
	AkUniqueID	srcID;
	AkUInt32	uClipStartPosition;
	AkUInt32	uClipDuration;
	AkUInt32	uSrcDuration;
	AkInt32		iSourceTrimOffset;
};

/** 40 bytes: one clip as the bank stores it (ms). */
struct AkTrackSrcInfo
{
	AkUInt32	trackID;
	AkUniqueID	sourceID;
	AkReal64	fPlayAt;
	AkReal64	fBeginTrimOffset;
	AkReal64	fEndTrimOffset;
	AkReal64	fSrcDuration;
};

struct AkMusicSwitchAssoc
{
	AkUInt32	switchID;
	AkUniqueID	nodeID;
};

/** 28 bytes: one playlist item as the bank stores it. */
struct AkMusicRanSeqPlaylistItem
{
	AkUniqueID	m_SegmentID;
	AkUniqueID	m_playlistItemID;
	AkUInt32	m_NumChildren;
	RSType		m_eRSType;
	AkInt16		m_Loop;
	AkUInt32	m_Weight;
	AkUInt16	m_wAvoidRepeatCount;
	bool		m_bIsUsingWeight;
	bool		m_bIsShuffle;
};

/** privatestructures.h: the sample <-> time conversions at the pipeline frequency, rounded to nearest. */
namespace AkTimeConv
{
	/** 2012 0x967920. */
	inline AkInt32 SecondsToSamples( AkReal64 in_seconds )
	{
		const AkReal64 v = (AkReal64)AkAudioLibSettings::g_pipelineCoreFrequency * in_seconds;
		return v <= 0.0 ? (AkInt32)( v + -0.5 ) : (AkInt32)( v + 0.5 );
	}

	/** Inlined in 2012 (AddPlaylistItem / MeterInfo): ms * frequency / 1000, rounded to nearest. */
	inline AkInt32 MillisecondsToSamples( AkReal64 in_milliseconds )
	{
		const AkReal64 v = (AkReal64)AkAudioLibSettings::g_pipelineCoreFrequency * in_milliseconds * 0.001;
		return v <= 0.0 ? (AkInt32)( v + -0.5 ) : (AkInt32)( v + 0.5 );
	}

	/** 2012 0x96c9b0. */
	inline AkInt32 SamplesToMilliseconds( AkInt32 in_samples )
	{
		const AkReal64 v = (AkReal64)in_samples * 1000.0 / (AkReal64)AkAudioLibSettings::g_pipelineCoreFrequency;
		return v <= 0.0 ? (AkInt32)( v + -0.5 ) : (AkInt32)( v + 0.5 );
	}
}

#if defined(_MSC_VER)
#pragma pack( pop )
#endif

static_assert( sizeof( AkMusicGrid ) == 16, "AkMusicGrid: 2012 PDB 16" );
static_assert( sizeof( AkMusicFade ) == 12, "AkMusicFade: 2012 PDB 12" );
static_assert( sizeof( AkMusicTransDestRule ) == 36, "AkMusicTransDestRule: 2012 PDB 36" );
static_assert( sizeof( AkMusicTransSrcRule ) == 20, "AkMusicTransSrcRule: 2012 PDB 20" );
static_assert( sizeof( AkMusicTransitionObject ) == 32, "AkMusicTransitionObject: 2012 PDB 0x20 (SetRules mallocs 0x20)" );
static_assert( sizeof( AkMusicTransitionRule ) == 68, "AkMusicTransitionRule: 2012 PDB 68 (Reserve mallocs 68 * n)" );
static_assert( sizeof( AkWwiseMusicTransitionRule ) == 92, "AkWwiseMusicTransitionRule: 2012 PDB 92" );
static_assert( sizeof( AkMusicMarker ) == 12, "AkMusicMarker: 2012 PDB 12" );
static_assert( sizeof( AkMusicMarkerWwise ) == 24, "AkMusicMarkerWwise: 2012 PDB 24" );
static_assert( sizeof( CAkStinger ) == 24, "CAkStinger: 2012 PDB 24" );
static_assert( sizeof( AkTrackSrc ) == 24, "AkTrackSrc: 2012 PDB 24" );
static_assert( sizeof( AkTrackSrcInfo ) == 40, "AkTrackSrcInfo: 2012 PDB 40" );
static_assert( sizeof( AkMusicRanSeqPlaylistItem ) == 28, "AkMusicRanSeqPlaylistItem: 2012 PDB 28" );
static_assert( sizeof( AkMeterInfo ) == 24, "AkMeterInfo: 2012 PDB align 4 (24)" );
