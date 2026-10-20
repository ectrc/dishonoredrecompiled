/*=============================================================================
	AkTimeStretchFX.cpp - CAkTimeStretchFX, Wwise 2012.1 (plugins/effects/aktimestretch/aktimestretchfx.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. The phase vocoder stretches by
	(stretch % + a random offset drawn once at Init from rand() through one LCG step), clamped to 25..1600 %; after 8
	consecutive buffers at exactly 100 % the vocoder re-initialises its phases ("no time-stretch mode"). When the window
	size changes or the scratch buffer cannot be allocated, the input is copied through (Bypass).
=============================================================================*/
#include "AkTimeStretchFX.h"
#include "AkApplyGain.h"
#include <stdlib.h>
#include <string.h>

/** 2012 0x998000, 2013 0x98f430. */
AK::IAkPlugin * CreateTimeStretchFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkTimeStretchFX() );
}

/** 2012 0x997fa0, 2013 0x98f3d0. */
CAkTimeStretchFX::CAkTimeStretchFX()
	: m_pSharedParams( NULL )
	, m_pAllocator( NULL )
{
}

/** 2012 0x9979b0, 2013 0x98ede0. */
CAkTimeStretchFX::~CAkTimeStretchFX()
{
}

/** 2012 0x9979d0, 2013 0x98ee00. */
AKRESULT CAkTimeStretchFX::Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * /*in_pFXCtx*/, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat )
{
	m_pSharedParams = static_cast<CAkTimeStretchFXParams *>( in_pParams );
	m_pAllocator = in_pAllocator;
	m_FXInfo.uNumChannels = AkGetNumChannels( in_rFormat.uChannelMask );
	m_FXInfo.uSampleRate = in_rFormat.uSampleRate;
	m_pSharedParams->GetParams( &m_FXInfo.Params );
	m_FXInfo.PrevParams = m_FXInfo.Params;
	const AkUInt32 uSeed = (AkUInt32)rand();
	const AkReal32 fRandom = m_FXInfo.Params.fTimeStretchRandom;
	m_FXInfo.fTSRandomOffset = (AkReal32)( -fRandom + ( fRandom - -fRandom ) * ( (double)(AkUInt32)( 214013 * uSeed + 2531011 ) * 2.3283064e-10f ) );
	const AKRESULT eResult = m_FXInfo.PhaseVocoder.Init( in_pAllocator, AkGetNumChannels( in_rFormat.uChannelMask ), in_rFormat.uSampleRate, m_FXInfo.Params.uWindowSize, true );
	m_FXInfo.bCanEnterNoTSMode = false;
	m_FXInfo.uNoTSModeCount = 0;
	m_fSkippedFrames = 0.f;
	return eResult;
}

/** 2012 0x997ad0, 2013 0x98ef00. */
AKRESULT CAkTimeStretchFX::Reset()
{
	m_FXInfo.PhaseVocoder.Reset();
	return AK_Success;
}

/** 2012 0x997ae0, 2013 0x98ef10. */
AKRESULT CAkTimeStretchFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = false;
	out_rPluginInfo.bIsAsynchronous = false;
	return AK_Success;
}

/** 2012 0x997b00, 2013 0x98ef30: the input frames a skip of io_uFrames output frames consumes, fractions carried. */
AKRESULT CAkTimeStretchFX::TimeSkip( AkUInt32 & io_uFrames )
{
	if( !io_uFrames )
	{
		return AK_NoMoreData;
	}
	m_pSharedParams->GetParams( &m_FXInfo.Params );
	AkReal32 fCurrentTSFactor = m_FXInfo.fTSRandomOffset + m_FXInfo.Params.fTimeStretch;
	if( fCurrentTSFactor >= 1600.f )
	{
		fCurrentTSFactor = 1600.f;
	}
	else if( fCurrentTSFactor <= 25.f )
	{
		fCurrentTSFactor = 25.f;
	}
	const AkReal32 fCurrentSkip = m_fSkippedFrames;
	const AkReal32 fSkipped = (AkReal32)( (double)io_uFrames / fCurrentTSFactor * 100.0 + fCurrentSkip );
	m_fSkippedFrames = fSkipped;
	io_uFrames = (AkUInt32)(AkInt64)( fSkipped - fCurrentSkip );
	return AK_DataReady;
}

/** 2012 0x997c80, 2013 0x98f0b0. */
AKRESULT CAkTimeStretchFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	m_FXInfo.PhaseVocoder.Term( in_pAllocator );
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x997cc0, 2013 0x98f0f0: copy as many frames as fit, on the smaller channel count. */
void CAkTimeStretchFX::Bypass( AkAudioBuffer * in_pBuffer, AkUInt32 in_uInOffset, AkAudioBuffer * out_pBuffer )
{
	const AkUInt32 uInChannels = in_pBuffer->NumChannels();
	const AkUInt32 uOutChannels = out_pBuffer->NumChannels();
	const AkUInt32 uNumChannels = ( uOutChannels && uInChannels < uOutChannels ) ? uInChannels : uOutChannels;
	AkUInt32 uNumFrames = out_pBuffer->uMaxFrames - out_pBuffer->uValidFrames;
	if( in_pBuffer->uValidFrames < uNumFrames )
	{
		uNumFrames = in_pBuffer->uValidFrames;
	}
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		memcpy( out_pBuffer->GetChannel( i ) + out_pBuffer->uValidFrames, in_pBuffer->GetChannel( i ) + in_uInOffset, sizeof( AkReal32 ) * uNumFrames );
	}
	out_pBuffer->uValidFrames = (AkUInt16)( out_pBuffer->uValidFrames + uNumFrames );
	in_pBuffer->uValidFrames = (AkUInt16)( in_pBuffer->uValidFrames - uNumFrames );
	if( in_pBuffer->eState == AK_NoMoreData && !in_pBuffer->uValidFrames )
	{
		out_pBuffer->eState = AK_NoMoreData;
	}
	else
	{
		out_pBuffer->eState = out_pBuffer->uValidFrames == out_pBuffer->uMaxFrames ? AK_DataReady : AK_DataNeeded;
	}
}

/** 2012 0x997dc0, 2013 0x98f1f0. */
void CAkTimeStretchFX::Execute( AkAudioBuffer * in_pBuffer, AkUInt32 in_uInOffset, AkAudioBuffer * out_pBuffer )
{
	m_fSkippedFrames = 0.f;
	m_pSharedParams->GetParams( &m_FXInfo.Params );
	if( m_FXInfo.Params.uWindowSize != m_FXInfo.PrevParams.uWindowSize )
	{
		m_FXInfo.PhaseVocoder.Term( m_pAllocator );
		if( m_FXInfo.PhaseVocoder.Init( m_pAllocator, m_FXInfo.uNumChannels, m_FXInfo.uSampleRate, m_FXInfo.Params.uWindowSize, true ) != AK_Success )
		{
			Bypass( in_pBuffer, in_uInOffset, out_pBuffer );
			return;
		}
		m_FXInfo.PhaseVocoder.Reset();
		m_FXInfo.PrevParams.uWindowSize = m_FXInfo.Params.uWindowSize;
	}
	const AkReal32 fTSRandomOffset = m_FXInfo.fTSRandomOffset;
	AkReal32 fCurrentTSFactor = m_FXInfo.Params.fTimeStretch + fTSRandomOffset;
	const AkUInt32 uTempStorageSize = sizeof( AkReal32 ) * m_FXInfo.PhaseVocoder.GetFFTSize();
	bool bNoStretch = false;
	if( fCurrentTSFactor >= 1600.f )
	{
		fCurrentTSFactor = 1600.f;
	}
	else if( fCurrentTSFactor <= 25.f )
	{
		fCurrentTSFactor = 25.f;
	}
	else if( fCurrentTSFactor == 100.f )
	{
		if( ( m_FXInfo.PrevParams.fTimeStretch + fTSRandomOffset ) != 100.f )
		{
			m_FXInfo.bCanEnterNoTSMode = true;
			m_FXInfo.uNoTSModeCount = 0;
		}
		if( m_FXInfo.bCanEnterNoTSMode )
		{
			++m_FXInfo.uNoTSModeCount;
		}
		bNoStretch = true;
	}
	if( !bNoStretch )
	{
		m_FXInfo.bCanEnterNoTSMode = false;
		m_FXInfo.uNoTSModeCount = 0;
	}
	const bool bEnterNoTSMode = m_FXInfo.uNoTSModeCount == 8;
	AkReal32 * pfTempStorage = (AkReal32 *)AK_PLUGIN_ALLOC( m_pAllocator, uTempStorageSize );
	if( !pfTempStorage )
	{
		Bypass( in_pBuffer, in_uInOffset, out_pBuffer );
		return;
	}
	m_FXInfo.PhaseVocoder.Execute( in_pBuffer, in_uInOffset, out_pBuffer, fCurrentTSFactor, bEnterNoTSMode, pfTempStorage );
	if( bEnterNoTSMode )
	{
		m_FXInfo.bCanEnterNoTSMode = false;
		m_FXInfo.uNoTSModeCount = 0;
	}
	AK_PLUGIN_FREE( m_pAllocator, pfTempStorage );
	if( out_pBuffer->eState == AK_DataReady || out_pBuffer->eState == AK_NoMoreData )
	{
		AK::DSP::ApplyGain( out_pBuffer, m_FXInfo.PrevParams.fOutputGain, m_FXInfo.Params.fOutputGain, true );
		m_FXInfo.PrevParams = m_FXInfo.Params;
	}
}
