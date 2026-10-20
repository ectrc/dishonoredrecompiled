/*=============================================================================
	AkMusicPBI.cpp - CAkMusicPBI (akmusicpbi.cpp, 2012 0x963b30..0x963ee0).

	DISHONORED(port): agent IH4 (wave 26).
	Deviation (named): OnLastFrame's TransParams has its curve set to linear; retail leaves the stack slot uninitialised
	(it is unused at transition time 0).
=============================================================================*/
#include "AkMusicPBI.h"
#include "AkMusicCtx.h"
#include "AkSegmentCtx.h"

/** 2012 0x963b30, 2013 0x960370. */
CAkMusicPBI::CAkMusicPBI( CAkMusicCtx * in_parent, CAkSoundBase * in_pSound, CAkSource * in_pSource, CAkRegisteredObj * in_pGameObj, UserParams & in_UserParams, const AkTrackSrc * in_pSrcInfo, PlayHistory & in_rPlayHistory, AkUniqueID in_SeqID, const PriorityInfoCurrent & in_Priority, AkUInt32 in_uSourceOffset, CAkLimiter * in_pAMLimiter, CAkLimiter * in_pBusLimiter )
	: CAkPBI( in_pSound, in_pSource, in_pGameObj, in_UserParams, in_rPlayHistory, in_SeqID, in_Priority, false, in_uSourceOffset, in_pAMLimiter, in_pBusLimiter )
	, CAkChildCtx( in_parent )
	, m_ulStopOffset( (AkUInt32)-1 )
	, m_pSrcInfo( in_pSrcInfo )
{
	m_bWasStoppedByUEngine = 0;
	m_bNeedsFadeOut = 0;
	m_bRequiresPreBuffering = 0;
}

/** 2012 0x963bc0, 2013 0x960400: unless its segment context stopped it, the segment context forgets it; it leaves its parent. */
void CAkMusicPBI::Term( bool in_bFailedToInit )
{
	if( !m_bWasStoppedByUEngine )
	{
		static_cast<CAkSegmentCtx *>( m_pParentCtx )->RemoveAllReferences( this );
	}
	m_pParentCtx->RemoveChild( this );
	CAkPBI::Term( in_bFailedToInit );
}

/** 2012 0x963c00, 2013 0x960440: joins its segment context first. */
AKRESULT CAkMusicPBI::Init( AkPathInfo * in_pPathInfo )
{
	Connect();
	return CAkPBI::Init( in_pPathInfo );
}

/** 2012 0x963c20, 2013 0x960460: a PBI waiting for its stop offset is not stopped before it started. */
void CAkMusicPBI::_Stop( AkPBIStopMode in_eStopMode, bool in_bIsFromTransition, bool in_bHasNotStarted )
{
	if( !in_bHasNotStarted || m_ulStopOffset == (AkUInt32)-1 )
	{
		CAkPBI::_Stop( in_eStopMode, in_bIsFromTransition, false );
	}
}

/** 2012 0x963c40, 2013 0x960480: volume and fades become PBI fade ratios (keyed by the automation type); LPF an offset. */
void CAkMusicPBI::SetAutomationValue( AkClipAutomationType in_eAutomationType, AkReal32 in_fValue )
{
	switch( in_eAutomationType )
	{
	case AutomationType_Volume:
	case AutomationType_FadeIn:
	case AutomationType_FadeOut:
		SetPBIFade( (AkUInt8 *)this + in_eAutomationType, ( in_fValue - -96.3f ) * 0.010384216f );
		break;
	case AutomationType_LPF:
		m_LPFAutomationOffset = in_fValue;
		CalculateEffectiveLPF();
		break;
	default:
		break;
	}
}

/** 2012 0x963cb0, 2013 0x9604f0: the earliest requested stop sample is kept; a stop with no offset (or a fade-out clip) is smoothed. */
void CAkMusicPBI::OnLastFrame( AkUInt32 in_uNumSamples )
{
	if( in_uNumSamples < m_ulStopOffset && in_uNumSamples != (AkUInt32)-1 )
	{
		m_ulStopOffset = in_uNumSamples;
	}
	TransParams transParams;
	transParams.TransitionTime = 0;
	transParams.eFadeCurve = AkCurveInterpolation_Linear;
	CAkPBI::_Stop( transParams, m_ulStopOffset == (AkUInt32)-1 || m_bNeedsFadeOut );
}

/** 2012 0x963d00, 2013 0x960540. */
void CAkMusicPBI::OnPaused()
{
	_Pause( true );
}

/** 2012 0x963d20, 2013 0x960560. */
void CAkMusicPBI::OnResumed()
{
	_Resume();
	m_fPauseResumeFadeRatio = 1.f;
	CalculateMutedEffectiveVolume();
}

/** 2012 0x963d50, 2013 0x960590: stopped by its segment context at a sample offset. */
void CAkMusicPBI::_Stop( AkUInt32 in_uStopOffset )
{
	m_bWasStoppedByUEngine = 1;
	OnLastFrame( in_uStopOffset );
}

/** 2012 0x963d70, 2013 0x9605b0: a persistent, per-PBI mute entry keyed by the owner (a context's fade ratio, or an automation). */
void CAkMusicPBI::SetPBIFade( void * in_pOwner, AkReal32 in_fFadeRatio )
{
	AkMutedMapItem mutedMapItem;
	mutedMapItem.m_Identifier = in_pOwner;
	mutedMapItem.m_bIsGlobal = 0;
	mutedMapItem.m_bIsPersistent = 1;
	SetMuteMapEntry( mutedMapItem, in_fFadeRatio );
}

/** 2012 0x963dc0, 2013 0x960600: the stop offset is handed to the voice once (not when a fade-out clip ends it). */
AkUInt32 CAkMusicPBI::GetAndClearStopOffset()
{
	const AkUInt32 uStopOffset = m_ulStopOffset;
	m_ulStopOffset = (AkUInt32)-1;
	return m_bNeedsFadeOut ? (AkUInt32)-1 : uStopOffset;
}

/** 2012 0x963de0, 2013 0x960620: music never pitches. */
void CAkMusicPBI::CalcEffectiveParams()
{
	CAkPBI::CalcEffectiveParams();
	m_EffectiveParams.Pitch = 0.f;
}

/**
 * 2012 0x963e50: a fade-in clip starts on a 1024-frame boundary: the start moves to the next boundary (skipping
 * source) when more than half a frame in or past the source offset, else back to the previous one.
 */
void CAkMusicPBI::FixStartTimeForFadeIn()
{
	const AkInt32 iFrameOffset = m_iFrameOffset;
	const AkInt32 iRemainder = iFrameOffset % AK_NUM_VOICE_REFILL_FRAMES;
	if( iRemainder > AK_NUM_VOICE_REFILL_FRAMES / 2 || iRemainder > (AkInt32)m_uSeekPosition )
	{
		m_uSeekPosition += AK_NUM_VOICE_REFILL_FRAMES - iRemainder;
		m_bSeekDirty = 1;
		m_bSeekRelativeToDuration = 0;
		m_bSnapSeekToMarker = 0;
		m_iFrameOffset = AK_NUM_VOICE_REFILL_FRAMES - iRemainder + iFrameOffset;
	}
	else if( iRemainder > 0 )
	{
		m_bNeedsFadeIn = 1;
		m_iFrameOffset = iFrameOffset - iRemainder;
		m_uSeekPosition -= iRemainder;
		m_bSeekDirty = 1;
		m_bSeekRelativeToDuration = 0;
		m_bSnapSeekToMarker = 0;
		return;
	}
	m_bNeedsFadeIn = 1;
}

/** 2012 0x963ee0, 2013 0x960720: a voice coming back from virtual restarts where its clip would be now, or stops. */
AkCtxVirtualHandlingResult CAkMusicPBI::NotifyVirtualOff( AkVirtualQueueBehavior /*in_eBehavior*/ )
{
	AkInt32 iFrameOffset;
	AkInt32 iSourceOffset;
	if( !static_cast<CAkSegmentCtx *>( m_pParentCtx )->GetSourceInfoForPlaybackRestart( this, iFrameOffset, iSourceOffset ) )
	{
		return VirtualHandling_ShouldStop;
	}
	m_uSeekPosition = (AkUInt32)iSourceOffset;
	m_iFrameOffset = iFrameOffset;
	m_bSeekDirty = 1;
	m_bSeekRelativeToDuration = 0;
	m_bSnapSeekToMarker = 0;
	return VirtualHandling_RequiresSeeking;
}
