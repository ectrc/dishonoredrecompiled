/*=============================================================================
	AkMusicPBI.h - CAkMusicPBI (akmusicpbi.cpp): the PBI of one music track clip. It is a child context of its segment
	context: the segment context stops it at a sample offset inside a frame (the lower engine truncates the voice there,
	CAkVPLSrcCbxNode reads GetStopOffset / GetAndClearStopOffset), fades and pauses it with the music contexts, and
	drives its volume / LPF from the clip automation. Music PBIs never pitch (CalcEffectiveParams) and cannot seek.

	DISHONORED(port): agent IH4 (wave 26). Layout (2012 PDB, 0x170): CAkPBI (0x158) + CAkChildCtx (+344) +
	m_ulStopOffset (+356) + m_pSrcInfo (+360) + m_bWasStoppedByUEngine / m_bNeedsFadeOut (+364 bits 0 / 1).
=============================================================================*/
#pragma once

#include <AkPBI.h>
#include "AkChildCtx.h"
#include "AkMusicStructs.h"

class CAkMusicCtx;

class CAkMusicPBI : public CAkPBI, public CAkChildCtx
{
public:
	CAkMusicPBI( CAkMusicCtx * in_parent, CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, UserParams & in_UserParams, const AkTrackSrc * in_pSrcInfo, PlayHistory & in_rPlayHistory, AkUniqueID in_SeqID, const PriorityInfoCurrent & in_Priority, AkUInt32 in_uSourceOffset, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter );
	virtual ~CAkMusicPBI() {}

	virtual AKRESULT Init( AkPathInfo * in_pPathInfo );
	virtual void Term( bool in_bFailedToInit );
	virtual void _Stop( AkPBIStopMode in_eStopMode = AkPBIStopMode_Normal, bool in_bIsFromTransition = false, bool in_bHasNotStarted = false );
	virtual void SeekTimeAbsolute( AkTimeMs, bool ) {}
	virtual void SeekPercent( AkReal32, bool ) {}
	virtual void CalcEffectiveParams();
	/** 2012 0x963db0, 2013 0x9605f0. */
	virtual AkUInt32 GetStopOffset() const { return m_ulStopOffset; }
	virtual AkUInt32 GetAndClearStopOffset();
	virtual AkCtxVirtualHandlingResult NotifyVirtualOff( AkVirtualQueueBehavior in_eBehavior );

	using CAkPBI::_Stop;
	void _Stop( AkUInt32 in_uStopOffset );
	void SetAutomationValue( AkClipAutomationType in_eAutomationType, AkReal32 in_fValue );
	void FixStartTimeForFadeIn();

	virtual void OnLastFrame( AkUInt32 in_uNumSamples );
	virtual void OnPaused();
	virtual void OnResumed();
	virtual void SetPBIFade( void * in_pOwner, AkReal32 in_fFadeRatio );
	virtual void VirtualAddRef() {}
	virtual void VirtualRelease() {}

	AkUInt32			m_ulStopOffset;
	const AkTrackSrc *	m_pSrcInfo;
	AkUInt32			m_bWasStoppedByUEngine : 1;
	AkUInt32			m_bNeedsFadeOut : 1;
};

static_assert( sizeof( CAkPBI ) == 0x158, "CAkPBI: 2012 PDB 0x158" );
static_assert( sizeof( CAkMusicPBI ) == 0x170, "CAkMusicPBI: 2012 PDB 0x170 (CAkMusicRenderer::Play mallocs 0x170)" );
