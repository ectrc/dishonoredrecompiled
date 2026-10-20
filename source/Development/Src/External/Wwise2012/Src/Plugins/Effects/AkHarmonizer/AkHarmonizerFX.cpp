/*=============================================================================
	AkHarmonizerFX.cpp - CAkHarmonizerFX, Wwise 2012.1 (plugins/effects/akharmonizer/akharmonizerfx.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile (no 2013 twins: the 2013 exe does not link this plug-in, and
	Dishonored's glue never registers it). Parameter changes apply on the audio thread as retail: a voice's pitch
	re-creates both voices; the window size, a voice enable, the input type or the LFE flag re-create voices and dry
	delays; sync dry re-creates the dry delays; each voice's filter parameters recompute its biquad. The scratch block
	(two buffers plus one FFT window when a voice is on) is allocated per Execute, as retail. TimeSkip is retail's
	folded "return AK_DataReady" (2012 0x98c570).
=============================================================================*/
#include "AkHarmonizerFX.h"

/** 2012 0x990460. */
AK::IAkPlugin * CreateHarmonizerFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkHarmonizerFX() );
}

/** 2012 0x990440. */
CAkHarmonizerFX::CAkHarmonizerFX()
	: m_pParams( NULL )
	, m_pAllocator( NULL )
{
}

/** 2012 0x990010. */
CAkHarmonizerFX::~CAkHarmonizerFX()
{
}

/** 2012 0x98fc30. */
void CAkHarmonizerFX::ComputeNumProcessedChannels( AkChannelMask in_eChannelMask )
{
	AkChannelMask eMask = 0;
	switch( m_FXInfo.Params.eInputType )
	{
	case AKINPUTTYPE_ASINPUT:
		m_FXInfo.uNumProcessedChannels = AkGetNumChannels( in_eChannelMask & ~AK_SPEAKER_LOW_FREQUENCY );
		m_FXInfo.eProcessChannelMask = in_eChannelMask & ~AK_SPEAKER_LOW_FREQUENCY;
		break;
	case AKINPUTTYPE_CENTER:			eMask = 0x04; break;
	case AKINPUTTYPE_STEREO:			eMask = 0x03; break;
	case AKINPUTCHANNELTYPE_3POINT0:	eMask = 0x07; break;
	case AKINPUTTYPE_4POINT0:			eMask = 0x33; break;
	case AKINPUTTYPE_5POINT0:			eMask = 0x37; break;
	case AKINPUTTYPE_LEFTONLY:			eMask = 0x01; break;
	default:
		break;
	}
	if( eMask )
	{
		m_FXInfo.uNumProcessedChannels = AkGetNumChannels( in_eChannelMask & eMask );
		m_FXInfo.eProcessChannelMask = eMask;
	}
	if( m_FXInfo.Params.bProcessLFE && ( in_eChannelMask & AK_SPEAKER_LOW_FREQUENCY ) )
	{
		++m_FXInfo.uNumProcessedChannels;
		m_FXInfo.eProcessChannelMask |= AK_SPEAKER_LOW_FREQUENCY;
	}
}

/** 2012 0x98fd90. */
void CAkHarmonizerFX::ComputeWetPathEnabled( AkChannelMask in_eChannelMask )
{
	m_FXInfo.bWetPathEnabled = ( m_FXInfo.Params.Voice[ 1 ].bEnable || m_FXInfo.Params.Voice[ 0 ].bEnable ) && ( in_eChannelMask & m_FXInfo.eProcessChannelMask ) != 0;
}

/** 2012 0x98fdd0. */
AKRESULT CAkHarmonizerFX::InitPitchVoices()
{
	for( AkUInt32 i = 0; i < 2; ++i )
	{
		const AkHarmonizerVoiceParams & rVoice = m_FXInfo.Params.Voice[ i ];
		if( rVoice.bEnable )
		{
			const AKRESULT eResult = m_FXInfo.PhaseVocoder[ i ].Init( m_pAllocator, m_FXInfo.uNumProcessedChannels, m_FXInfo.uSampleRate, m_FXInfo.Params.uWindowSize, false );
			if( eResult != AK_Success )
			{
				return eResult;
			}
			if( rVoice.Filter.eFilterType != AKFILTERTYPE_NONE )
			{
				m_FXInfo.Filter[ i ].ComputeCoefs( (AK::DSP::MultiChannelBiquadFilter<6>::FilterType)( rVoice.Filter.eFilterType - 1 ), m_FXInfo.uSampleRate,
					rVoice.Filter.fFilterFrequency, rVoice.Filter.fFilterGain, rVoice.Filter.fFilterQFactor );
			}
		}
	}
	return AK_Success;
}

/** 2012 0x98fe70. */
void CAkHarmonizerFX::TermPitchVoices()
{
	for( AkUInt32 i = 0; i < 2; ++i )
	{
		m_FXInfo.PhaseVocoder[ i ].Term( m_pAllocator );
	}
}

/** 2012 0x98fea0. */
void CAkHarmonizerFX::ResetPitchVoices()
{
	for( AkUInt32 i = 0; i < 2; ++i )
	{
		if( m_FXInfo.Params.Voice[ i ].bEnable )
		{
			m_FXInfo.PhaseVocoder[ i ].Reset();
			m_FXInfo.PhaseVocoder[ i ].ResetInputFill();
			m_FXInfo.Filter[ i ].Reset();
		}
	}
}

/** 2012 0x98fef0. */
AKRESULT CAkHarmonizerFX::InitDryDelay()
{
	const AkUInt32 uWindowSize = m_FXInfo.Params.uWindowSize;
	if( m_FXInfo.Params.bSyncDry && m_FXInfo.bWetPathEnabled )
	{
		for( AkUInt32 i = 0; i < m_FXInfo.uTotalNumChannels; ++i )
		{
			const AKRESULT eResult = m_FXInfo.DryDelay[ i ].Init( m_pAllocator, uWindowSize );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
	}
	return AK_Success;
}

/** 2012 0x98ff50. */
void CAkHarmonizerFX::TermDryDelay()
{
	for( AkUInt32 i = 0; i < m_FXInfo.uTotalNumChannels; ++i )
	{
		m_FXInfo.DryDelay[ i ].Term( m_pAllocator );
	}
}

/** 2012 0x98ff80. */
void CAkHarmonizerFX::ResetDryDelay()
{
	for( AkUInt32 i = 0; i < m_FXInfo.uTotalNumChannels; ++i )
	{
		m_FXInfo.DryDelay[ i ].Reset();
	}
}

/** 2012 0x98ffb0. */
AKRESULT CAkHarmonizerFX::Reset()
{
	ResetPitchVoices();
	ResetDryDelay();
	return AK_Success;
}

/** 2012 0x98fff0. */
AKRESULT CAkHarmonizerFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bIsAsynchronous = false;
	return AK_Success;
}

/** 2012 0x98c570 (folded). */
AKRESULT CAkHarmonizerFX::TimeSkip( AkUInt32 /*in_uFrames*/ )
{
	return AK_DataReady;
}

/** 2012 0x990050. */
AKRESULT CAkHarmonizerFX::Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat )
{
	m_pAllocator = in_pAllocator;
	m_pParams = static_cast<CAkHarmonizerFXParams *>( in_pParams );
	m_FXInfo.bSendMode = in_pFXCtx->IsSendModeEffect();
	m_FXInfo.uTotalNumChannels = AkGetNumChannels( in_rFormat.uChannelMask );
	m_pParams->GetParams( &m_FXInfo.Params );
	if( m_FXInfo.bSendMode )
	{
		m_FXInfo.Params.fDryLevel = 0.f;
	}
	m_FXInfo.PrevParams = m_FXInfo.Params;
	m_FXInfo.uSampleRate = in_rFormat.uSampleRate;
	ComputeNumProcessedChannels( in_rFormat.uChannelMask );
	ComputeWetPathEnabled( in_rFormat.uChannelMask );
	AKRESULT eResult = InitPitchVoices();
	if( eResult == AK_Success )
	{
		eResult = InitDryDelay();
		if( eResult == AK_Success )
		{
			m_pParams->m_ParamChangeHandler.ResetAllParamChanges();
		}
	}
	return eResult;
}

/** 2012 0x990140. */
AKRESULT CAkHarmonizerFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	TermPitchVoices();
	TermDryDelay();
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x9901b0. */
void CAkHarmonizerFX::Execute( AkAudioBuffer * io_pBuffer )
{
	m_pParams->GetParams( &m_FXInfo.Params );
	if( m_FXInfo.bSendMode )
	{
		m_FXInfo.Params.fDryLevel = 0.f;
	}
	AK::AkFXParameterChangeHandler<AKHARMONIZERPARAMID_NUM> & rChanges = m_pParams->m_ParamChangeHandler;
	if( rChanges.HasAnyChanged() )
	{
		if( rChanges.HasChanged( AKHARMONIZERPARAMID_VOICE1PITCH ) || rChanges.HasChanged( AKHARMONIZERPARAMID_VOICE2PITCH ) )
		{
			TermPitchVoices();
			if( InitPitchVoices() != AK_Success )
			{
				return;
			}
			ResetPitchVoices();
		}
		if( rChanges.HasChanged( AKHARMONIZERPARAMID_WINDOWSIZE ) || rChanges.HasChanged( AKHARMONIZERPARAMID_VOICE1ENABLE ) || rChanges.HasChanged( AKHARMONIZERPARAMID_VOICE2ENABLE )
			|| rChanges.HasChanged( AKHARMONIZERPARAMID_INPUT ) || rChanges.HasChanged( AKHARMONIZERPARAMID_PROCESSLFE ) )
		{
			TermPitchVoices();
			TermDryDelay();
			ComputeNumProcessedChannels( io_pBuffer->uChannelMask );
			ComputeWetPathEnabled( io_pBuffer->uChannelMask );
			if( InitPitchVoices() != AK_Success || InitDryDelay() != AK_Success )
			{
				return;
			}
			ResetPitchVoices();
			ResetDryDelay();
		}
		if( rChanges.HasChanged( AKHARMONIZERPARAMID_SYNCDRY ) )
		{
			TermDryDelay();
			if( InitDryDelay() != AK_Success )
			{
				return;
			}
			ResetDryDelay();
		}
		for( AkUInt32 i = 0; i < 2; ++i )
		{
			const AkPluginParamID uFirst = i ? AKHARMONIZERPARAMID_VOICE2FILTERTYPE : AKHARMONIZERPARAMID_VOICE1FILTERTYPE;
			const AkHarmonizerVoiceParams & rVoice = m_FXInfo.Params.Voice[ i ];
			if( ( rChanges.HasChanged( uFirst ) || rChanges.HasChanged( uFirst + 2 ) || rChanges.HasChanged( uFirst + 1 ) || rChanges.HasChanged( uFirst + 3 ) )
				&& rVoice.Filter.eFilterType != AKFILTERTYPE_NONE )
			{
				m_FXInfo.Filter[ i ].ComputeCoefs( (AK::DSP::MultiChannelBiquadFilter<6>::FilterType)( rVoice.Filter.eFilterType - 1 ), m_FXInfo.uSampleRate,
					rVoice.Filter.fFilterFrequency, rVoice.Filter.fFilterGain, rVoice.Filter.fFilterQFactor );
			}
		}
	}
	rChanges.ResetAllParamChanges();
	AkUInt32 uWindowBytes = 0;
	if( m_FXInfo.Params.Voice[ 0 ].bEnable )
	{
		uWindowBytes = sizeof( AkReal32 ) * m_FXInfo.PhaseVocoder[ 0 ].GetFFTSize();
	}
	if( m_FXInfo.Params.Voice[ 1 ].bEnable )
	{
		uWindowBytes = sizeof( AkReal32 ) * m_FXInfo.PhaseVocoder[ 1 ].GetFFTSize();
	}
	AkReal32 * pfTempStorage = (AkReal32 *)AK_PLUGIN_ALLOC( m_pAllocator, 2 * sizeof( AkReal32 ) * io_pBuffer->MaxFrames() + uWindowBytes );
	if( pfTempStorage )
	{
		AkHarmonizerDSPProcess( io_pBuffer, m_FXInfo, pfTempStorage );
		AK_PLUGIN_FREE( m_pAllocator, pfTempStorage );
	}
}
