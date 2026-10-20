/*=============================================================================
	AkStereoDelayLine.cpp - AK::DSP::CStereoDelayLine, Wwise 2012.1 (plugins/effects/akstereodelay/akstereodelayline.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (no 2013 twins). Per four frames: the two delayed samples are
	the wet output; the input of each side gains its own delayed signal times its feedback and the other side's times
	the cross-feed (both ramped per frame); the result runs through the feedback biquads and is written to the lines.
	Deviation (named): unaligned SSE loads and stores where retail uses aligned ones (same values).
=============================================================================*/
#include "AkStereoDelayLine.h"
#include <math.h>
#include <string.h>
#include <xmmintrin.h>

namespace AK
{
namespace DSP
{

/** 2012 0x999020. */
AKRESULT CStereoDelayLine::Init( AK::IAkPluginMemAlloc * in_pAllocator, AkReal32 * const in_fDelayTimes, AkUInt32 in_uSampleRate )
{
	const double dSampleRate = (double)in_uSampleRate;
	m_uSampleRate = in_uSampleRate;
	const AKRESULT eResult = m_DelayLines[ 0 ].Init( in_pAllocator, (AkUInt32)(AkInt64)floor( dSampleRate * in_fDelayTimes[ 0 ] ), 1 );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	return m_DelayLines[ 1 ].Init( in_pAllocator, (AkUInt32)(AkInt64)floor( in_fDelayTimes[ 1 ] * dSampleRate ), 1 );
}

/** 2012 0x9990e0. */
void CStereoDelayLine::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	m_DelayLines[ 0 ].Term( in_pAllocator );
	m_DelayLines[ 1 ].Term( in_pAllocator );
}

/** 2012 0x999100. */
void CStereoDelayLine::Reset()
{
	m_DelayLines[ 0 ].Reset();
	m_DelayLines[ 1 ].Reset();
	m_FeedbackFilters[ 0 ].Reset();
	m_FeedbackFilters[ 1 ].Reset();
}

static inline __m128 RampStart( AkReal32 in_fStart, AkReal32 in_fInc )
{
	const AkReal32 f1 = in_fInc + in_fStart;
	const AkReal32 f2 = ( in_fInc + in_fStart ) + in_fInc;
	return _mm_setr_ps( in_fStart, f1, f2, f2 + in_fInc );
}

/** 2012 0x999130. */
void CStereoDelayLine::ProcessBuffer( AkAudioBuffer * in_pBuffer, AkAudioBuffer * out_pBuffer, AkStereoDelayChannelParams * const in_PrevStereoDelayParams,
	AkStereoDelayChannelParams * const in_CurStereoDelayParams, AkStereoDelayFilterParams & in_FilterParams, bool in_bRecomputeFilterParams )
{
	const AkUInt32 uNumFrames = in_pBuffer->uValidFrames;
	const AkUInt32 uLength0 = m_DelayLines[ 0 ].GetDelayLength();
	const AkUInt32 uLength1 = m_DelayLines[ 1 ].GetDelayLength();
	AkUInt32 uOffset0 = m_DelayLines[ 0 ].GetCurrentOffset();
	AkUInt32 uOffset1 = m_DelayLines[ 1 ].GetCurrentOffset();
	AkReal32 * pfDelay0 = m_DelayLines[ 0 ].GetChannel( 0 ) + uOffset0;
	AkReal32 * pfDelay1 = m_DelayLines[ 1 ].GetChannel( 0 ) + uOffset1;
	AkReal32 * pfOutL = out_pBuffer->GetChannel( 0 );
	AkReal32 * pfOutR = out_pBuffer->GetChannel( 1 );
	AkReal32 * pfInL = in_pBuffer->GetChannel( 0 );
	AkReal32 * pfInR = in_pBuffer->GetChannel( 1 );
	AkReal32 * const pfInLStart = pfInL;
	AkReal32 * const pfInRStart = pfInR;
	const AkReal32 fNumFrames = (AkReal32)uNumFrames;

	const AkReal32 fFeedback0Inc = ( in_CurStereoDelayParams[ 0 ].fFeedback - in_PrevStereoDelayParams[ 0 ].fFeedback ) / fNumFrames;
	__m128 vFeedback0 = RampStart( in_PrevStereoDelayParams[ 0 ].fFeedback, fFeedback0Inc );
	const __m128 vFeedback0Inc = _mm_set1_ps( fFeedback0Inc * 4.f );
	const AkReal32 fFeedback1Inc = ( in_CurStereoDelayParams[ 1 ].fFeedback - in_PrevStereoDelayParams[ 1 ].fFeedback ) / fNumFrames;
	__m128 vFeedback1 = RampStart( in_PrevStereoDelayParams[ 1 ].fFeedback, fFeedback1Inc );
	const __m128 vFeedback1Inc = _mm_set1_ps( fFeedback1Inc * 4.f );
	const AkReal32 fCrossFeed0Inc = ( in_CurStereoDelayParams[ 0 ].fCrossFeed - in_PrevStereoDelayParams[ 0 ].fCrossFeed ) / fNumFrames;
	__m128 vCrossFeed0 = RampStart( in_PrevStereoDelayParams[ 0 ].fCrossFeed, fCrossFeed0Inc );
	const __m128 vCrossFeed0Inc = _mm_set1_ps( fCrossFeed0Inc * 4.f );
	const AkReal32 fCrossFeed1Inc = ( in_CurStereoDelayParams[ 1 ].fCrossFeed - in_PrevStereoDelayParams[ 1 ].fCrossFeed ) / fNumFrames;
	__m128 vCrossFeed1 = RampStart( in_PrevStereoDelayParams[ 1 ].fCrossFeed, fCrossFeed1Inc );
	const __m128 vCrossFeed1Inc = _mm_set1_ps( fCrossFeed1Inc * 4.f );

	AkUInt32 uFramesRemaining = uNumFrames;
	AkUInt32 uFramesBeforeWrap = ( uLength0 - uOffset0 ) >= ( uLength1 - uOffset1 ) ? ( uLength1 - uOffset1 ) : ( uLength0 - uOffset0 );
	while( uFramesRemaining )
	{
		const AkUInt32 uFrames = uFramesRemaining >= uFramesBeforeWrap ? uFramesBeforeWrap : uFramesRemaining;
		for( AkUInt32 uVectors = uFrames >> 2; uVectors; --uVectors )
		{
			const __m128 vDelay0 = _mm_loadu_ps( pfDelay0 );
			const __m128 vDelay1 = _mm_loadu_ps( pfDelay1 );
			pfDelay0 += 4;
			pfDelay1 += 4;
			_mm_storeu_ps( pfOutL, vDelay0 );
			_mm_storeu_ps( pfInL, _mm_add_ps( _mm_add_ps( _mm_mul_ps( vCrossFeed1, vDelay1 ), _mm_loadu_ps( pfInL ) ), _mm_mul_ps( vFeedback0, vDelay0 ) ) );
			_mm_storeu_ps( pfOutR, vDelay1 );
			_mm_storeu_ps( pfInR, _mm_add_ps( _mm_add_ps( _mm_mul_ps( vCrossFeed0, vDelay0 ), _mm_loadu_ps( pfInR ) ), _mm_mul_ps( vFeedback1, vDelay1 ) ) );
			vCrossFeed1 = _mm_add_ps( vCrossFeed1, vCrossFeed1Inc );
			vFeedback1 = _mm_add_ps( vFeedback1, vFeedback1Inc );
			vFeedback0 = _mm_add_ps( vFeedback0, vFeedback0Inc );
			vCrossFeed0 = _mm_add_ps( vCrossFeed0, vCrossFeed0Inc );
			pfOutL += 4;
			pfOutR += 4;
			pfInL += 4;
			pfInR += 4;
		}
		uOffset0 += uFrames;
		uOffset1 += uFrames;
		if( uOffset0 == uLength0 )
		{
			pfDelay0 = m_DelayLines[ 0 ].GetChannel( 0 );
			uOffset0 = 0;
		}
		if( uOffset1 == uLength1 )
		{
			pfDelay1 = m_DelayLines[ 1 ].GetChannel( 0 );
			uOffset1 = 0;
		}
		uFramesRemaining -= uFrames;
		uFramesBeforeWrap = ( uLength0 - uOffset0 ) >= ( uLength1 - uOffset1 ) ? ( uLength1 - uOffset1 ) : ( uLength0 - uOffset0 );
	}

	if( in_FilterParams.eFilterType )
	{
		if( in_bRecomputeFilterParams )
		{
			m_FeedbackFilters[ 0 ].ComputeCoefs( (::DSP::BiquadFilter::FilterType)( in_FilterParams.eFilterType - 1 ), m_uSampleRate,
				in_FilterParams.fFilterFrequency, in_FilterParams.fFilterGain, in_FilterParams.fFilterQFactor );
			m_FeedbackFilters[ 1 ].m_Coefficients = m_FeedbackFilters[ 0 ].m_Coefficients;
		}
		m_FeedbackFilters[ 0 ].ProcessBuffer( pfInLStart, uNumFrames );
		m_FeedbackFilters[ 1 ].ProcessBuffer( pfInRStart, uNumFrames );
	}

	const AkUInt32 uWriteOffset0 = m_DelayLines[ 0 ].GetCurrentOffset();
	const AkUInt32 uWriteOffset1 = m_DelayLines[ 1 ].GetCurrentOffset();
	const AkUInt32 uBeforeWrap0 = uLength0 - uWriteOffset0;
	const AkUInt32 uBeforeWrap1 = uLength1 - uWriteOffset1;
	memcpy( m_DelayLines[ 0 ].GetChannel( 0 ) + uWriteOffset0, pfInLStart, sizeof( AkReal32 ) * ( uNumFrames >= uBeforeWrap0 ? uBeforeWrap0 : uNumFrames ) );
	if( uBeforeWrap0 < uNumFrames )
	{
		memcpy( m_DelayLines[ 0 ].GetChannel( 0 ), pfInLStart + uBeforeWrap0, sizeof( AkReal32 ) * ( uNumFrames - uBeforeWrap0 ) );
	}
	memcpy( m_DelayLines[ 1 ].GetChannel( 0 ) + uWriteOffset1, pfInRStart, sizeof( AkReal32 ) * ( uNumFrames >= uBeforeWrap1 ? uBeforeWrap1 : uNumFrames ) );
	if( uBeforeWrap1 < uNumFrames )
	{
		memcpy( m_DelayLines[ 1 ].GetChannel( 0 ), pfInRStart + uBeforeWrap1, sizeof( AkReal32 ) * ( uNumFrames - uBeforeWrap1 ) );
	}
	m_DelayLines[ 0 ].SetCurrentOffset( ( uWriteOffset0 + uNumFrames ) % uLength0 );
	m_DelayLines[ 1 ].SetCurrentOffset( ( uNumFrames + uWriteOffset1 ) % uLength1 );
}

}
}
