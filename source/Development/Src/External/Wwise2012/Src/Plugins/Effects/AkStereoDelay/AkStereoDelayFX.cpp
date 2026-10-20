/*=============================================================================
	AkStereoDelayFX.cpp - CAkStereoDelayFX, Wwise 2012.1 (plugins/effects/akstereodelay/akstereodelayfx.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (no 2013 twins: the 2013 exe does not link this plug-in, and
	Dishonored's glue never registers it). Parameter changes are applied on the audio thread: the enables, the left delay
	/ feedback / cross-feed and the right delay / feedback / cross-feed recompute the tail (at most 60 s); either delay
	time re-creates both delay lines; the filter parameters request new feedback-filter coefficients. Kept as retail:
	Init clears only the two delay-time change bits, SanitizeParams exists but Init and Execute inline its body.
	TimeSkip is retail's folded "return AK_DataReady" (2012 0x98c570).
=============================================================================*/
#include "AkStereoDelayFX.h"
#include <math.h>

/** 2012 0x998850. */
AK::IAkPlugin * CreateStereoDelayFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkStereoDelayFX() );
}

/** 2012 0x998830. */
CAkStereoDelayFX::CAkStereoDelayFX()
	: m_pParams( NULL )
	, m_pAllocator( NULL )
{
}

/** 2012 0x998310. */
CAkStereoDelayFX::~CAkStereoDelayFX()
{
}

/** 2012 0x9984b0. */
void CAkStereoDelayFX::SanitizeParams( AkStereoDelayFXParams & io_FXParams )
{
	if( m_FXInfo.bSendMode )
	{
		io_FXParams.fDryLevel = 0.f;
	}
	if( !io_FXParams.bEnableFeedback )
	{
		io_FXParams.StereoDelayParams[ 0 ].fFeedback = 0.f;
		io_FXParams.StereoDelayParams[ 1 ].fFeedback = 0.f;
	}
	if( !io_FXParams.bEnableCrossFeed )
	{
		io_FXParams.StereoDelayParams[ 0 ].fCrossFeed = 0.f;
		io_FXParams.StereoDelayParams[ 1 ].fCrossFeed = 0.f;
	}
}

/** 2012 0x998320: the longer side's -60 dB time through its feedback (or its delay), plus the cross-feed loop's. */
void CAkStereoDelayFX::ComputeTailLength()
{
	const bool bEnableFeedback = m_FXInfo.Params.bEnableFeedback;
	const AkReal32 fDelayTime0 = m_FXInfo.Params.StereoDelayParams[ 0 ].fDelayTime;
	const AkReal32 fDelayTime1 = m_FXInfo.Params.StereoDelayParams[ 1 ].fDelayTime;
	AkReal32 fTailTime;
	if( bEnableFeedback )
	{
		double dGain = log10( (double)m_FXInfo.Params.StereoDelayParams[ 0 ].fFeedback ) * 20.0;
		const AkReal32 fGain0 = dGain >= -0.1 ? -0.1f : (AkReal32)dGain;
		const AkReal32 fTail0 = ( -60.f / fGain0 ) * fDelayTime0;
		dGain = log10( (double)m_FXInfo.Params.StereoDelayParams[ 1 ].fFeedback ) * 20.0;
		const AkReal32 fGain1 = dGain >= -0.1 ? -0.1f : (AkReal32)dGain;
		fTailTime = ( -60.f / fGain1 ) * fDelayTime1;
		if( fTail0 > fTailTime )
		{
			fTailTime = fTail0;
		}
	}
	else
	{
		fTailTime = fDelayTime0 <= fDelayTime1 ? fDelayTime1 : fDelayTime0;
	}
	if( m_FXInfo.Params.bEnableCrossFeed )
	{
		const double dGain = 20.0 * log10( (double)( m_FXInfo.Params.StereoDelayParams[ 1 ].fCrossFeed * m_FXInfo.Params.StereoDelayParams[ 0 ].fCrossFeed ) );
		const AkReal32 fGain = dGain >= -0.1 ? -0.1f : (AkReal32)dGain;
		const AkReal32 fIterations = -60.f / fGain;
		const AkReal32 fLoops = bEnableFeedback ? 2.f : 1.f;
		fTailTime = ( ( ( fDelayTime1 + fDelayTime0 ) * fLoops ) * fIterations ) + fTailTime;
	}
	if( fTailTime >= 60.f )
	{
		fTailTime = 60.f;
	}
	m_FXInfo.uTailLength = (AkUInt32)(AkInt64)( (double)m_FXInfo.uSampleRate * fTailTime );
}

/** 2012 0x9984f0. */
AKRESULT CAkStereoDelayFX::Reset()
{
	m_FXInfo.StereoDelay[ 0 ].Reset();
	m_FXInfo.StereoDelay[ 1 ].Reset();
	return AK_Success;
}

/** 2012 0x998510. */
AKRESULT CAkStereoDelayFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bIsAsynchronous = false;
	return AK_Success;
}

/** 2012 0x98c570 (folded). */
AKRESULT CAkStereoDelayFX::TimeSkip( AkUInt32 /*in_uFrames*/ )
{
	return AK_DataReady;
}

/** 2012 0x998530. */
AKRESULT CAkStereoDelayFX::Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat )
{
	m_pParams = static_cast<CAkStereoDelayFXParams *>( in_pParams );
	m_pAllocator = in_pAllocator;
	m_FXInfo.uSampleRate = in_rFormat.uSampleRate;
	m_FXInfo.bSendMode = in_pFXCtx->IsSendModeEffect();
	m_pParams->GetParams( &m_FXInfo.Params );
	SanitizeParams( m_FXInfo.Params );
	AkReal32 fDelayTimes[ 2 ] = { m_FXInfo.Params.StereoDelayParams[ 0 ].fDelayTime, m_FXInfo.Params.StereoDelayParams[ 1 ].fDelayTime };
	m_FXInfo.PrevParams = m_FXInfo.Params;
	m_FXInfo.bRecomputeFilterCoefs = true;
	AKRESULT eResult = m_FXInfo.StereoDelay[ 0 ].Init( in_pAllocator, fDelayTimes, m_FXInfo.uSampleRate );
	if( eResult == AK_Success )
	{
		m_pParams->m_ParamChangeHandler.ResetParamChange( AKSTEREODELAYPARAMID_LEFTDELAYTIME );
		eResult = m_FXInfo.StereoDelay[ 1 ].Init( in_pAllocator, fDelayTimes, m_FXInfo.uSampleRate );
		if( eResult == AK_Success )
		{
			m_pParams->m_ParamChangeHandler.ResetParamChange( AKSTEREODELAYPARAMID_RIGHTDELAYTIME );
		}
	}
	return eResult;
}

/** 2012 0x998630. */
AKRESULT CAkStereoDelayFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	m_FXInfo.StereoDelay[ 0 ].Term( in_pAllocator );
	m_FXInfo.StereoDelay[ 1 ].Term( in_pAllocator );
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x998670. */
void CAkStereoDelayFX::Execute( AkAudioBuffer * io_pBuffer )
{
	m_pParams->GetParams( &m_FXInfo.Params );
	SanitizeParams( m_FXInfo.Params );
	AK::AkFXParameterChangeHandler<AKSTEREODELAYPARAMID_NUM> & rChanges = m_pParams->m_ParamChangeHandler;
	if( rChanges.HasChanged( AKSTEREODELAYPARAMID_ENABLECROSSFEED ) || rChanges.HasChanged( AKSTEREODELAYPARAMID_ENABLEFEEDBACK )
		|| rChanges.HasChanged( AKSTEREODELAYPARAMID_LEFTDELAYTIME ) || rChanges.HasChanged( AKSTEREODELAYPARAMID_LEFTFEEDBACK )
		|| rChanges.HasChanged( AKSTEREODELAYPARAMID_LEFTCROSSFEED ) || rChanges.HasChanged( AKSTEREODELAYPARAMID_RIGHTDELAYTIME )
		|| rChanges.HasChanged( AKSTEREODELAYPARAMID_RIGHTFEEDBACK ) || rChanges.HasChanged( AKSTEREODELAYPARAMID_RIGHTCROSSFEED ) )
	{
		ComputeTailLength();
	}
	if( rChanges.HasChanged( AKSTEREODELAYPARAMID_RIGHTDELAYTIME ) || rChanges.HasChanged( AKSTEREODELAYPARAMID_LEFTDELAYTIME ) )
	{
		AkReal32 fDelayTimes[ 2 ] = { m_FXInfo.Params.StereoDelayParams[ 0 ].fDelayTime, m_FXInfo.Params.StereoDelayParams[ 1 ].fDelayTime };
		m_FXInfo.StereoDelay[ 0 ].Term( m_pAllocator );
		if( m_FXInfo.StereoDelay[ 0 ].Init( m_pAllocator, fDelayTimes, m_FXInfo.uSampleRate ) != AK_Success )
		{
			return;
		}
		m_FXInfo.StereoDelay[ 0 ].Reset();
		m_FXInfo.StereoDelay[ 1 ].Term( m_pAllocator );
		if( m_FXInfo.StereoDelay[ 1 ].Init( m_pAllocator, fDelayTimes, m_FXInfo.uSampleRate ) != AK_Success )
		{
			return;
		}
		m_FXInfo.StereoDelay[ 1 ].Reset();
	}
	m_FXInfo.bRecomputeFilterCoefs = rChanges.HasChanged( AKSTEREODELAYPARAMID_FILTERTYPE ) || rChanges.HasChanged( AKSTEREODELAYPARAMID_FILTERGAIN )
		|| rChanges.HasChanged( AKSTEREODELAYPARAMID_FILTERFREQUENCY ) || rChanges.HasChanged( AKSTEREODELAYPARAMID_FILTERQFACTOR );
	rChanges.ResetAllParamChanges();
	AkReal32 * pfStorage = (AkReal32 *)AK_PLUGIN_ALLOC( m_pAllocator, sizeof( AkReal32 ) * 4 * io_pBuffer->MaxFrames() );
	if( pfStorage )
	{
		AkStereoDelayDSPProcess( io_pBuffer, m_FXInfo, pfStorage );
		AK_PLUGIN_FREE( m_pAllocator, pfStorage );
	}
}
