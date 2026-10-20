/*=============================================================================
	AkTremoloFX.cpp - CAkTremoloFX, Wwise 2012.1 (plugins/effects/aktremolo/aktremolofx.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. Per processed channel the gain is
	(1 - depth) + depth * lfo, lfo in [0, 1], times the output gain; depth and output gain ramp linearly from the
	previous buffer's values over the buffer. The sine runs unsmoothed in double (x87 in retail), the other waveforms
	go through the LFO's one-pole smoothing filter (y = b0 x - a1 y1). A skipped center channel does not consume an LFO.
	Deviations (named): retail's four-frame unroll multiplies (window * input) * gain in some lanes and
	(window * gain) * input in others; one order is used here (last-ulp differences). TimeSkip is the COMDAT-folded
	"return AK_DataReady" retail shares between the in-place effects (2012 0x98c570).
=============================================================================*/
#include "AkTremoloFX.h"
#include <math.h>
#include <string.h>

#ifndef AK_SPEAKER_SETUP_3STEREO
#define AK_SPEAKER_SETUP_3STEREO ( AK_SPEAKER_SETUP_STEREO | AK_SPEAKER_FRONT_CENTER )
#endif

/** 2012 0x99ad90, 2013 0x993a90. */
AK::IAkPlugin * CreateTremoloFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkTremoloFX() );
}

/** 2012 0x999fe0, 2013 0x992ce0. */
CAkTremoloFX::CAkTremoloFX()
	: m_pSharedParams( NULL )
	, m_pAllocator( NULL )
{
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		DSP::LFO::State & rState = m_lfo.GetChannel( i ).GetState();
		rState.filter.fFFbk1 = 0.f;
		rState.filter.fB0 = 0.f;
		rState.filter.fA1 = 0.f;
		rState.fPhase = 0.f;
		rState.fPhaseDelta = 0.f;
		rState.eWaveform = DSP::LFO::WAVEFORM_SINE;
	}
	memset( &m_FXInfo.Params, 0, sizeof( m_FXInfo.Params ) );
	memset( &m_FXInfo.PrevParams, 0, sizeof( m_FXInfo.PrevParams ) );
}

/** 2012 0x999f80, 2013 0x992c80. */
CAkTremoloFX::~CAkTremoloFX()
{
}

/** 2012 0x99adb0, 2013 0x993ab0. */
AKRESULT CAkTremoloFX::Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * /*in_pFXCtx*/, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat )
{
	m_pSharedParams = static_cast<CAkTremoloFXParams *>( in_pParams );
	m_FXInfo.Params = m_pSharedParams->m_Params;
	m_pSharedParams->SetDirty( false );
	m_FXInfo.PrevParams = m_FXInfo.Params;
	m_FXInfo.uSampleRate = in_rFormat.uSampleRate;
	m_pAllocator = in_pAllocator;
	AkUInt32 uChannelMask = in_rFormat.uChannelMask;
	if( !m_FXInfo.Params.NonRTPC.bProcessCenter && ( uChannelMask & AK_SPEAKER_SETUP_3STEREO ) == AK_SPEAKER_SETUP_3STEREO )
	{
		uChannelMask &= ~AK_SPEAKER_FRONT_CENTER;
	}
	if( !m_FXInfo.Params.NonRTPC.bProcessLFE )
	{
		uChannelMask &= ~AK_SPEAKER_LOW_FREQUENCY;
	}
	m_lfo.Setup( uChannelMask, m_FXInfo.uSampleRate, m_FXInfo.Params.RTPC.modParams );
	return AK_Success;
}

/** 2012 0x999f90, 2013 0x992c90. */
AKRESULT CAkTremoloFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x999f70, 2013 0x992c70. */
AKRESULT CAkTremoloFX::Reset()
{
	return AK_Success;
}

/** 2012 0x999f50, 2013 0x992c50. */
AKRESULT CAkTremoloFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bIsAsynchronous = false;
	return AK_Success;
}

/** 2012 0x98c570 (folded), 2013 twin folded too. */
AKRESULT CAkTremoloFX::TimeSkip( AkUInt32 /*in_uFrames*/ )
{
	return AK_DataReady;
}

/** 2012 0x99a120, 2013 0x992e20. */
void CAkTremoloFX::SetupLFO( AkUInt32 in_uChannelMask )
{
	if( !m_FXInfo.Params.NonRTPC.bProcessCenter && ( in_uChannelMask & AK_SPEAKER_SETUP_3STEREO ) == AK_SPEAKER_SETUP_3STEREO )
	{
		in_uChannelMask &= ~AK_SPEAKER_FRONT_CENTER;
	}
	if( !m_FXInfo.Params.NonRTPC.bProcessLFE )
	{
		in_uChannelMask &= ~AK_SPEAKER_LOW_FREQUENCY;
	}
	m_lfo.Setup( in_uChannelMask, m_FXInfo.uSampleRate, m_FXInfo.Params.RTPC.modParams );
}

/** 2012 0x99a160, 2013 0x992e60: a change of the center / LFE processing re-creates the oscillators. */
void CAkTremoloFX::LiveParametersUpdate( AkAudioBuffer * io_pBuffer )
{
	const bool bProcessLFE = m_FXInfo.Params.NonRTPC.bProcessLFE;
	if( m_FXInfo.PrevParams.NonRTPC.bProcessLFE != bProcessLFE || m_FXInfo.PrevParams.NonRTPC.bProcessCenter != m_FXInfo.Params.NonRTPC.bProcessCenter )
	{
		AkUInt32 uChannelMask = io_pBuffer->uChannelMask;
		if( !m_FXInfo.Params.NonRTPC.bProcessCenter && ( uChannelMask & AK_SPEAKER_SETUP_3STEREO ) == AK_SPEAKER_SETUP_3STEREO )
		{
			uChannelMask &= ~AK_SPEAKER_FRONT_CENTER;
		}
		if( !bProcessLFE )
		{
			uChannelMask &= ~AK_SPEAKER_LOW_FREQUENCY;
		}
		m_lfo.Setup( uChannelMask, m_FXInfo.uSampleRate, m_FXInfo.Params.RTPC.modParams );
	}
}

/** 2012 0x999fc0, 2013 0x992cc0. */
void CAkTremoloFX::RTPCParametersUpdate()
{
	m_lfo.SetParams( m_FXInfo.uSampleRate, m_FXInfo.Params.RTPC.modParams.lfoParams );
}

/** 2012 0x99a1b0, 2013 0x992eb0. */
void CAkTremoloFX::Execute( AkAudioBuffer * io_pBuffer )
{
	if( !m_lfo.GetNumChannels() )
	{
		return;
	}
	m_FXInfo.Params = m_pSharedParams->m_Params;
	m_pSharedParams->SetDirty( false );
	if( m_FXInfo.Params.NonRTPC.bHasChanged )
	{
		LiveParametersUpdate( io_pBuffer );
	}
	if( m_FXInfo.Params.RTPC.bHasChanged )
	{
		m_lfo.SetParams( m_FXInfo.uSampleRate, m_FXInfo.Params.RTPC.modParams.lfoParams );
	}
	AkUInt32 uChannelMask = io_pBuffer->uChannelMask;
	if( !m_FXInfo.Params.NonRTPC.bProcessLFE )
	{
		uChannelMask &= ~AK_SPEAKER_LOW_FREQUENCY;
	}
	const bool bSkipCenterChannel = !m_FXInfo.Params.NonRTPC.bProcessCenter && ( uChannelMask & AK_SPEAKER_SETUP_3STEREO ) == AK_SPEAKER_SETUP_3STEREO;
	const AkUInt32 uNumFrames = io_pBuffer->uValidFrames;
	const AkReal32 fPrevDepth = m_FXInfo.PrevParams.RTPC.fModDepth;
	const AkReal32 fPrevOutputGain = m_FXInfo.PrevParams.RTPC.fOutputGain;
	const AkReal32 fDepthInc = ( m_FXInfo.Params.RTPC.fModDepth - fPrevDepth ) / (AkReal32)uNumFrames;
	const AkReal32 fOutInc = ( m_FXInfo.Params.RTPC.fOutputGain - fPrevOutputGain ) / (AkReal32)uNumFrames;
	const AkUInt32 uNumChannels = AkGetNumChannels( uChannelMask );
	AkUInt32 uLFOChannel = 0;
	for( AkUInt32 i = 0; i < uNumChannels; ++i )
	{
		if( bSkipCenterChannel && i == 2 )
		{
			continue;
		}
		AkReal32 * pfBuf = io_pBuffer->GetChannel( i );
		AkReal32 * const pfEnd = pfBuf + uNumFrames;
		DSP::LFO::State & rState = m_lfo.GetChannel( uLFOChannel ).GetState();
		const DSP::LFO::Waveform eWaveform = rState.eWaveform;
		AkReal32 fFilterMem = rState.filter.fFFbk1;
		const AkReal32 fB0 = rState.filter.fB0;
		const AkReal32 fA1 = rState.filter.fA1;
		AkReal32 fPhase = rState.fPhase;
		const AkReal32 fPhaseDelta = rState.fPhaseDelta;
		AkReal32 fDepth = fPrevDepth;
		AkReal32 fGain = fPrevOutputGain;
		double dPhaseOut = fPhase;
		switch( eWaveform )
		{
		case DSP::LFO::WAVEFORM_SINE:
			if( pfBuf < pfEnd )
			{
				double dPhase = fPhase;
				double dDepth = fDepth;
				double dGain = fGain;
				while( pfBuf < pfEnd )
				{
					dDepth = dDepth + fDepthInc;
					dGain = dGain + fOutInc;
					dPhase = dPhase + fPhaseDelta;
					*pfBuf = (AkReal32)( ( ( sin( dPhase ) + 1.0 ) * dDepth * 0.5 + 1.0 - dDepth ) * dGain * *pfBuf );
					++pfBuf;
				}
				dPhaseOut = dPhase;
			}
			break;
		case DSP::LFO::WAVEFORM_TRIANGLE:
			while( pfBuf < pfEnd )
			{
				fPhase = fPhaseDelta + fPhase;
				if( fPhase >= 1.f )
				{
					fPhase = fPhase - 1.f;
				}
				fDepth = fDepth + fDepthInc;
				fGain = fGain + fOutInc;
				const AkReal32 fTri = fPhase > 0.5f ? 1.f - fPhase : fPhase;
				fFilterMem = ( ( ( fTri * fB0 ) * fDepth ) * 2.f ) - ( fA1 * fFilterMem );
				*pfBuf = ( ( ( 1.f - fDepth ) + fFilterMem ) * fGain ) * *pfBuf;
				++pfBuf;
			}
			dPhaseOut = fPhase;
			break;
		case DSP::LFO::WAVEFORM_SQUARE:
			{
				const AkReal32 fPWM = m_FXInfo.Params.RTPC.modParams.lfoParams.fPWM;
				while( pfBuf < pfEnd )
				{
					fPhase = fPhaseDelta + fPhase;
					if( fPhase >= 1.f )
					{
						fPhase = fPhase - 1.f;
					}
					fDepth = fDepth + fDepthInc;
					fGain = fGain + fOutInc;
					const AkReal32 fSquare = fPhase > fPWM ? 0.f : fDepth;
					fFilterMem = ( fSquare * fB0 ) - ( fA1 * fFilterMem );
					*pfBuf = ( ( ( 1.f - fDepth ) + fFilterMem ) * fGain ) * *pfBuf;
					++pfBuf;
				}
				dPhaseOut = fPhase;
			}
			break;
		case DSP::LFO::WAVEFORM_SAW_UP:
			while( pfBuf < pfEnd )
			{
				fPhase = fPhaseDelta + fPhase;
				if( fPhase >= 1.f )
				{
					fPhase = fPhase - 1.f;
				}
				fDepth = fDepth + fDepthInc;
				fGain = fGain + fOutInc;
				fFilterMem = ( ( fPhase * fB0 ) * fDepth ) - ( fA1 * fFilterMem );
				*pfBuf = ( ( ( 1.f - fDepth ) + fFilterMem ) * fGain ) * *pfBuf;
				++pfBuf;
			}
			dPhaseOut = fPhase;
			break;
		case DSP::LFO::WAVEFORM_SAW_DOWN:
			while( pfBuf < pfEnd )
			{
				fPhase = fPhaseDelta + fPhase;
				if( fPhase >= 1.f )
				{
					fPhase = fPhase - 1.f;
				}
				fDepth = fDepth + fDepthInc;
				fGain = fGain + fOutInc;
				fFilterMem = ( ( ( 1.f - fPhase ) * fB0 ) * fDepth ) - ( fA1 * fFilterMem );
				*pfBuf = ( ( ( 1.f - fDepth ) + fFilterMem ) * fGain ) * *pfBuf;
				++pfBuf;
			}
			dPhaseOut = fPhase;
			break;
		default:
			break;
		}
		rState.filter.fFFbk1 = fFilterMem;
		rState.fPhase = (AkReal32)fmod( dPhaseOut, eWaveform != DSP::LFO::WAVEFORM_SINE ? 1.0 : 6.283185482025146 );
		++uLFOChannel;
	}
	m_FXInfo.PrevParams = m_FXInfo.Params;
}
