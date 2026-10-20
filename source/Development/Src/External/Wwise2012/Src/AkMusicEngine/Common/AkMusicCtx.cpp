/*=============================================================================
	AkMusicCtx.cpp - CAkChildCtx (akchildctx.cpp, 2012 0x9680f0..0x968120) and CAkMusicCtx (akmusicctx.cpp, 2012
	0x963f40..0x964770).

	DISHONORED(port): agent IH4 (wave 26). Fades run in dB on the transition manager (-96.3 dB = silent) and reach the
	children as a linear ratio (dB + 96.3) / 96.3, retail's constant 0.010384216 = 1 / 96.3.
=============================================================================*/
#include "AkMusicCtx.h"
#include "AkMusicRenderer.h"
#include "AkMatrixAwareCtx.h"

static const AkReal32 AK_MUSIC_SILENCE_DB = -96.3f;
static const AkReal32 AK_MUSIC_DB_TO_RATIO = 0.010384216f;

// ------------------------------------------------------------------------------------------------------------
// CAkChildCtx
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x9680f0, 2013 0x9649a0. */
CAkChildCtx::CAkChildCtx( CAkMusicCtx * in_pParentCtx )
	: pNextLightItem( NULL )
	, m_pParentCtx( in_pParentCtx )
{
}

/** 2012 0x968110, 2013 0x9649c0. */
CAkChildCtx::~CAkChildCtx()
{
}

/** 2012 0x968120, 2013 0x9649d0. */
void CAkChildCtx::Connect()
{
	m_pParentCtx->AddChild( this );
}

// ------------------------------------------------------------------------------------------------------------
// CAkMusicCtx
// ------------------------------------------------------------------------------------------------------------

/** 2012 0x964390, 2013 0x960bd0. */
CAkMusicCtx::CAkMusicCtx( CAkMusicCtx * in_parent )
	: CAkChildCtx( in_parent )
	, m_uRefCount( 0 )
	, m_fPlayStopFadeRatio( 0.f )
	, m_fPauseResumeFadeRatio( 1.f )
	, m_uNumLastSamples( (AkUInt32)-1 )
	, m_uPauseCount( 0 )
{
	m_listChildren.m_pFirst = NULL;
	m_PBTrans.pvPSTrans = NULL;
	m_PBTrans.pvPRTrans = NULL;
	m_PBTrans.bIsPSTransFading = 0;
	m_PBTrans.bIsPRTransFading = 0;
	m_eStatus = CtxStatusIdle;
	m_bIsPaused = 0;
}

/** 2012 0x9644f0, 2013 0x960d30. */
CAkMusicCtx::~CAkMusicCtx()
{
	if( m_PBTrans.pvPSTrans )
	{
		g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPSTrans, this );
	}
	if( m_PBTrans.pvPRTrans )
	{
		g_pTransitionManager->RemoveTransitionUser( m_PBTrans.pvPRTrans, this );
	}
	m_listChildren.m_pFirst = NULL;
}

/** 2012 0x963f40, 2013 0x960780: a child context joins its parent. */
void CAkMusicCtx::Init( CAkRegisteredObj * /*in_GameObject*/, UserParams & /*in_rUserparams*/ )
{
	if( m_pParentCtx )
	{
		Connect();
	}
}

/** 2012 0x963f50, 2013 0x960790: the fade value (dB) becomes the children's fade ratio; the end of a stop / pause completes it. */
void CAkMusicCtx::TransUpdateValue( TransitionTargets in_eTarget, AkReal32 in_fValue, bool in_bIsTerminated )
{
	bool bIsStopOrPause = false;
	switch( in_eTarget )
	{
	case TransTarget_Pause:
		bIsStopOrPause = true;
		// fall through
	case TransTarget_Resume:
		m_fPauseResumeFadeRatio = ( in_fValue - AK_MUSIC_SILENCE_DB ) * AK_MUSIC_DB_TO_RATIO;
		SetPBIFade( &m_fPauseResumeFadeRatio, m_fPauseResumeFadeRatio );
		if( in_bIsTerminated )
		{
			m_PBTrans.pvPRTrans = NULL;
			if( bIsStopOrPause )
			{
				OnPaused();
			}
		}
		break;
	case TransTarget_Stop:
		bIsStopOrPause = true;
		// fall through
	case TransTarget_Play:
		m_fPlayStopFadeRatio = ( in_fValue - AK_MUSIC_SILENCE_DB ) * AK_MUSIC_DB_TO_RATIO;
		SetPBIFade( &m_fPlayStopFadeRatio, m_fPlayStopFadeRatio );
		if( in_bIsTerminated )
		{
			m_PBTrans.pvPSTrans = NULL;
			if( bIsStopOrPause )
			{
				OnLastFrame( (AkUInt32)-1 );
			}
		}
		break;
	default:
		break;
	}
}

/** 2012 0x964040, 2013 0x960880. */
void CAkMusicCtx::OnPlayed()
{
	m_eStatus = CtxStatusPlaying;
}

/** 2012 0x964050, 2013 0x960890. */
void CAkMusicCtx::OnStopped()
{
	m_eStatus = CtxStatusStopped;
}

/** 2012 0x964060, 2013 0x9608a0. */
void CAkMusicCtx::VirtualAddRef()
{
	++m_uRefCount;
}

/** 2012 0x964770, 2013 0x960fb0. */
void CAkMusicCtx::VirtualRelease()
{
	Release();
}

/**
 * 2012 0x964070: an idle or playing context starts (or reverses a running stop fade); a fade-in starts at silence and
 * its start is pushed by the fade offset (whole audio frames).
 */
void CAkMusicCtx::_Play( AkMusicFade & in_fadeParams )
{
	if( m_eStatus > CtxStatusPlaying )
	{
		return;
	}
	if( m_PBTrans.pvPSTrans )
	{
		g_pTransitionManager->ChangeParameter( m_PBTrans.pvPSTrans, TransTarget_Play, 0.f, in_fadeParams.transitionTime, in_fadeParams.eFadeCurve, AkValueMeaning_Default );
		OnPlayed();
		return;
	}
	if( in_fadeParams.transitionTime > 0 )
	{
		TransitionParameters params;
		params.pUser = this;
		params.eTarget = TransTarget_Play;
		params.fStartValue = AK_MUSIC_SILENCE_DB;
		params.fTargetValue = 0.f;
		params.lDuration = in_fadeParams.transitionTime;
		params.eFadeCurve = in_fadeParams.eFadeCurve;
		params.bdBs = true;
		params.bUseReciprocalCurve = true;
		CAkTransition * pTransition = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
		m_PBTrans.bIsPSTransFading = 1;
		m_PBTrans.pvPSTrans = pTransition;
		if( !pTransition )
		{
			TransUpdateValue( params.eTarget, params.fTargetValue, true );
			OnPlayed();
			return;
		}
		if( in_fadeParams.iFadeOffset )
		{
			pTransition->m_fStartTimeInBufferTick += (AkReal32)( in_fadeParams.iFadeOffset / AK_NUM_VOICE_REFILL_FRAMES );
		}
	}
	OnPlayed();
}

/** 2012 0x964190, 2013 0x9609d0. */
void CAkMusicCtx::_Cancel()
{
	OnLastFrame( 0 );
}

/** 2012 0x9641a0, 2013 0x9609e0. */
void CAkMusicCtx::_Pause( TransParams & in_transParams )
{
	++m_uPauseCount;
	if( m_PBTrans.pvPRTrans )
	{
		g_pTransitionManager->ChangeParameter( m_PBTrans.pvPRTrans, TransTarget_Pause, AK_MUSIC_SILENCE_DB, in_transParams.TransitionTime, in_transParams.eFadeCurve, AkValueMeaning_Default );
		return;
	}
	TransitionParameters params;
	params.pUser = this;
	params.eTarget = TransTarget_Pause;
	params.fStartValue = 0.f;
	params.fTargetValue = AK_MUSIC_SILENCE_DB;
	params.lDuration = in_transParams.TransitionTime;
	params.eFadeCurve = in_transParams.eFadeCurve;
	params.bdBs = true;
	params.bUseReciprocalCurve = true;
	CAkTransition * pTransition = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
	m_PBTrans.bIsPRTransFading = 1;
	m_PBTrans.pvPRTrans = pTransition;
	if( !pTransition )
	{
		TransUpdateValue( params.eTarget, params.fTargetValue, true );
	}
}

/** 2012 0x964260, 2013 0x960aa0: the last resume of nested pauses (or a master resume) fades back from the current ratio. */
void CAkMusicCtx::_Resume( TransParams & in_transParams, bool in_bIsMasterResume )
{
	if( !in_bIsMasterResume && m_uPauseCount > 1 )
	{
		--m_uPauseCount;
		return;
	}
	m_uPauseCount = 0;
	if( m_PBTrans.pvPRTrans )
	{
		g_pTransitionManager->ChangeParameter( m_PBTrans.pvPRTrans, TransTarget_Resume, 0.f, in_transParams.TransitionTime, in_transParams.eFadeCurve, AkValueMeaning_Default );
		OnResumed();
		return;
	}
	if( in_transParams.TransitionTime <= 0 )
	{
		m_fPauseResumeFadeRatio = 1.f;
		SetPBIFade( &m_fPauseResumeFadeRatio, 1.f );
	}
	else
	{
		TransitionParameters params;
		params.pUser = this;
		params.eTarget = TransTarget_Resume;
		params.fStartValue = m_fPauseResumeFadeRatio * 96.3f - 96.3f;
		params.fTargetValue = 0.f;
		params.lDuration = in_transParams.TransitionTime;
		params.eFadeCurve = in_transParams.eFadeCurve;
		params.bdBs = true;
		params.bUseReciprocalCurve = true;
		CAkTransition * pTransition = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
		m_PBTrans.bIsPRTransFading = 1;
		m_PBTrans.pvPRTrans = pTransition;
		if( !pTransition )
		{
			TransUpdateValue( params.eTarget, params.fTargetValue, true );
		}
	}
	OnResumed();
}

/** 2012 0x964410, 2013 0x960c50: a paused, idle or unfaded context stops at once (after in_uNumLastSamples); else it fades out. */
void CAkMusicCtx::_Stop( TransParams & in_transParams, AkUInt32 in_uNumLastSamples )
{
	if( m_bIsPaused )
	{
		OnLastFrame( in_uNumLastSamples );
		return;
	}
	if( m_PBTrans.pvPSTrans )
	{
		g_pTransitionManager->ChangeParameter( m_PBTrans.pvPSTrans, TransTarget_Stop, AK_MUSIC_SILENCE_DB, in_transParams.TransitionTime, in_transParams.eFadeCurve, AkValueMeaning_Default );
		return;
	}
	if( in_transParams.TransitionTime <= 0 || !( m_eStatus & ( CtxStatusPlaying | CtxStatusLastFrame ) ) )
	{
		OnLastFrame( in_uNumLastSamples );
		return;
	}
	TransitionParameters params;
	params.pUser = this;
	params.eTarget = TransTarget_Stop;
	params.fStartValue = 0.f;
	params.fTargetValue = AK_MUSIC_SILENCE_DB;
	params.lDuration = in_transParams.TransitionTime;
	params.eFadeCurve = in_transParams.eFadeCurve;
	params.bdBs = true;
	params.bUseReciprocalCurve = true;
	CAkTransition * pTransition = g_pTransitionManager->AddTransitionToList( params, true, CAkTransitionManager::TC_Fade );
	m_PBTrans.bIsPSTransFading = 1;
	m_PBTrans.pvPSTrans = pTransition;
	if( !pTransition )
	{
		TransUpdateValue( params.eTarget, params.fTargetValue, true );
	}
}

/** 2012 0x964550, 2013 0x960d90: a child holds a reference on its parent. */
void CAkMusicCtx::AddChild( CAkChildCtx * in_pChildCtx )
{
	m_listChildren.AddFirst( in_pChildCtx );
	++m_uRefCount;
}

/** 2012 0x964580, 2013 0x960dc0: every child pauses, then this context (its stop fade pauses too). */
void CAkMusicCtx::OnPaused()
{
	for( CAkChildCtx * pChild = m_listChildren.m_pFirst; pChild; pChild = pChild->pNextLightItem )
	{
		pChild->OnPaused();
	}
	m_bIsPaused = 1;
	if( m_PBTrans.pvPSTrans )
	{
		g_pTransitionManager->Pause( m_PBTrans.pvPSTrans );
	}
}

/** 2012 0x9645c0, 2013 0x960e00. */
void CAkMusicCtx::OnResumed()
{
	for( CAkChildCtx * pChild = m_listChildren.m_pFirst; pChild; pChild = pChild->pNextLightItem )
	{
		pChild->OnResumed();
	}
	m_bIsPaused = 0;
	if( m_PBTrans.pvPSTrans )
	{
		g_pTransitionManager->Resume( m_PBTrans.pvPSTrans );
	}
}

/** 2012 0x964600, 2013 0x960e40. */
void CAkMusicCtx::SetPBIFade( void * in_pOwner, AkReal32 in_fFadeRatio )
{
	for( CAkChildCtx * pChild = m_listChildren.m_pFirst; pChild; pChild = pChild->pNextLightItem )
	{
		pChild->SetPBIFade( in_pOwner, in_fFadeRatio );
	}
}

/** 2012 0x964650, 2013 0x960e90. */
void CAkMusicCtx::RemoveChild( CAkChildCtx * in_pChildCtx )
{
	if( m_listChildren.Remove( in_pChildCtx ) == AK_Success )
	{
		Release();
	}
}

/** 2012 0x964670, 2013 0x960eb0: the last reference unlinks from the parent (or the music renderer) and frees the context. */
void CAkMusicCtx::Release()
{
	if( m_uRefCount-- != 1 )
	{
		return;
	}
	if( m_pParentCtx )
	{
		CAkMusicCtx * pParent = m_pParentCtx;
		if( pParent->m_listChildren.Remove( this ) == AK_Success )
		{
			pParent->Release();
		}
	}
	else
	{
		CAkMusicRenderer::m_pMusicRenderer->RemoveChild( static_cast<CAkMatrixAwareCtx *>( this ) );
	}
	this->~CAkMusicCtx();
	AK::MemoryMgr::Free( g_DefaultPoolId, this );
}

/**
 * 2012 0x9646d0: every child gets the last frame (with a reference held across); the context then stops at the
 * earliest requested sample (playing: it goes to "last frame" and stops at the end of the frame) or at once.
 */
void CAkMusicCtx::OnLastFrame( AkUInt32 in_uNumSamples )
{
	++m_uRefCount;
	for( CAkChildCtx * pChild = m_listChildren.m_pFirst; pChild; pChild = pChild->pNextLightItem )
	{
		pChild->VirtualAddRef();
		pChild->OnLastFrame( in_uNumSamples );
	}
	CAkChildCtx * pChild = m_listChildren.m_pFirst;
	while( pChild )
	{
		CAkChildCtx * pNext = pChild->pNextLightItem;
		pChild->VirtualRelease();
		pChild = pNext;
	}
	if( in_uNumSamples < m_uNumLastSamples && in_uNumSamples != (AkUInt32)-1 )
	{
		m_uNumLastSamples = in_uNumSamples;
	}
	if( m_uNumLastSamples && m_eStatus != CtxStatusIdle && !m_bIsPaused )
	{
		if( m_eStatus <= CtxStatusLastFrame )
		{
			m_eStatus = CtxStatusLastFrame;
		}
	}
	else
	{
		OnStopped();
	}
	Release();
}
