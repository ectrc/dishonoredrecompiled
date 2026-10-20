/*=============================================================================
	AkPitchShifterFX.cpp - CAkPitchShifterFX, Wwise 2012.1 (plugins/effects/akpitchshifter/akpitchshifterfx.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. One delay-line pitch voice over the
	processed channels (input type, plus the LFE when asked), an optional biquad on the wet path, an optional dry delay of
	half the delay time to stay in sync with the wet path. Parameter changes are applied on the audio thread from the
	change handler as retail: input type / process LFE / delay time re-create the voice and the dry delay; sync dry
	re-creates the dry delay; pitch, filter type, gain, frequency and Q update in place. The wet path's scratch buffer is
	allocated and freed per Execute through the plug-in allocator, as retail. TimeSkip is retail's folded
	"return AK_DataReady" (2012 0x98c570, which is this class's own symbol).
=============================================================================*/
#include "AkPitchShifterFX.h"

/** 2012 0x9966a0, 2013 0x98cb00. */
AK::IAkPlugin * CreatePitchShifterFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkPitchShifterFX() );
}

/** 2012 0x996680, 2013 0x98cae0. */
CAkPitchShifterFX::CAkPitchShifterFX()
	: m_pParams( NULL )
	, m_pAllocator( NULL )
{
}

/** 2012 0x995f90, 2013 0x98c3f0. */
CAkPitchShifterFX::~CAkPitchShifterFX()
{
}

/** 2012 0x995fa0, 2013 0x98c400. */
void CAkPitchShifterFX::ComputeNumProcessedChannels( AkChannelMask in_eChannelMask )
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

/** 2012 0x9960d0, 2013 0x98c530. */
void CAkPitchShifterFX::ComputeTailLength()
{
	m_FXInfo.uTailLength = (AkUInt32)(AkInt64)( (double)m_FXInfo.uSampleRate * m_FXInfo.Params.fDelayTime * 0.001 );
}

/** 2012 0x996130, 2013 0x98c590. */
AKRESULT CAkPitchShifterFX::InitPitchVoice()
{
	if( m_FXInfo.uNumProcessedChannels )
	{
		const AKRESULT eResult = m_FXInfo.PitchShifter.Init( m_pAllocator, m_FXInfo.Params.fDelayTime, m_FXInfo.uNumProcessedChannels, m_FXInfo.uSampleRate );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		m_FXInfo.PitchShifter.SetPitchFactor( m_FXInfo.Params.Voice.fPitchFactor );
		if( m_FXInfo.Params.Voice.Filter.eFilterType != AKFILTERTYPE_NONE )
		{
			m_FXInfo.Filter.ComputeCoefs( (AK::DSP::MultiChannelBiquadFilter<6>::FilterType)( m_FXInfo.Params.Voice.Filter.eFilterType - 1 ),
				m_FXInfo.uSampleRate, m_FXInfo.Params.Voice.Filter.fFilterFrequency, m_FXInfo.Params.Voice.Filter.fFilterGain, m_FXInfo.Params.Voice.Filter.fFilterQFactor );
		}
	}
	return AK_Success;
}

/** 2012 0x9961c0, 2013 0x98c620. */
void CAkPitchShifterFX::TermPitchVoice()
{
	if( m_FXInfo.uNumProcessedChannels )
	{
		m_FXInfo.PitchShifter.Term( m_pAllocator );
	}
}

/** 2012 0x9961e0, 2013 0x98c640. */
void CAkPitchShifterFX::ResetPitchVoice()
{
	if( m_FXInfo.uNumProcessedChannels )
	{
		m_FXInfo.PitchShifter.Reset();
		m_FXInfo.Filter.Reset();
	}
}

/** 2012 0x996210, 2013 0x98c670. */
AKRESULT CAkPitchShifterFX::InitDryDelay()
{
	const AkUInt32 uDryDelayLength = m_FXInfo.uTailLength / 2;
	if( m_FXInfo.Params.bSyncDry )
	{
		for( AkUInt32 i = 0; i < m_FXInfo.uTotalNumChannels; ++i )
		{
			const AKRESULT eResult = m_FXInfo.DryDelay[ i ].Init( m_pAllocator, uDryDelayLength );
			if( eResult != AK_Success )
			{
				return eResult;
			}
		}
	}
	return AK_Success;
}

/** 2012 0x996260, 2013 0x98c6c0. */
void CAkPitchShifterFX::TermDryDelay()
{
	for( AkUInt32 i = 0; i < m_FXInfo.uTotalNumChannels; ++i )
	{
		m_FXInfo.DryDelay[ i ].Term( m_pAllocator );
	}
}

/** 2012 0x996290, 2013 0x98c6f0. */
void CAkPitchShifterFX::ResetDryDelay()
{
	for( AkUInt32 i = 0; i < m_FXInfo.uTotalNumChannels; ++i )
	{
		m_FXInfo.DryDelay[ i ].Reset();
	}
}

/** 2012 0x9962c0, 2013 0x98c720. */
AKRESULT CAkPitchShifterFX::Reset()
{
	ResetPitchVoice();
	ResetDryDelay();
	return AK_Success;
}

/** 2012 0x996310, 2013 0x98c770. */
AKRESULT CAkPitchShifterFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bIsAsynchronous = false;
	return AK_Success;
}

/** 2012 0x98c570 (folded with every in-place effect's TimeSkip). */
AKRESULT CAkPitchShifterFX::TimeSkip( AkUInt32 /*in_uFrames*/ )
{
	return AK_DataReady;
}

/** 2012 0x996330, 2013 0x98c790. */
AKRESULT CAkPitchShifterFX::Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat )
{
	m_pAllocator = in_pAllocator;
	m_pParams = static_cast<CAkPitchShifterFXParams *>( in_pParams );
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
	ComputeTailLength();
	AKRESULT eResult = InitPitchVoice();
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

/** 2012 0x996440, 2013 0x98c8a0. */
AKRESULT CAkPitchShifterFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	TermPitchVoice();
	TermDryDelay();
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x9964b0, 2013 0x98c910. */
void CAkPitchShifterFX::Execute( AkAudioBuffer * io_pBuffer )
{
	m_pParams->GetParams( &m_FXInfo.Params );
	if( m_FXInfo.bSendMode )
	{
		m_FXInfo.Params.fDryLevel = 0.f;
	}
	AK::AkFXParameterChangeHandler<AKPITCHSHIFTERPARAMID_NUM> & rChanges = m_pParams->m_ParamChangeHandler;
	if( rChanges.HasAnyChanged() )
	{
		if( rChanges.HasChanged( AKPITCHSHIFTERPARAMID_DELAYTIME ) || rChanges.HasChanged( AKPITCHSHIFTERPARAMID_INPUT ) || rChanges.HasChanged( AKPITCHSHIFTERPARAMID_PROCESSLFE ) )
		{
			TermPitchVoice();
			TermDryDelay();
			ComputeTailLength();
			ComputeNumProcessedChannels( io_pBuffer->uChannelMask );
			if( InitPitchVoice() != AK_Success || InitDryDelay() != AK_Success )
			{
				return;
			}
			ResetPitchVoice();
			ResetDryDelay();
		}
		if( rChanges.HasChanged( AKPITCHSHIFTERPARAMID_SYNCDRY ) )
		{
			TermDryDelay();
			if( InitDryDelay() != AK_Success )
			{
				return;
			}
			ResetDryDelay();
		}
		if( rChanges.HasChanged( AKPITCHSHIFTERPARAMID_PITCH ) )
		{
			m_FXInfo.PitchShifter.SetPitchFactor( m_FXInfo.Params.Voice.fPitchFactor );
		}
		if( ( rChanges.HasChanged( AKPITCHSHIFTERPARAMID_FILTERTYPE ) || rChanges.HasChanged( AKPITCHSHIFTERPARAMID_FILTERFREQUENCY )
			|| rChanges.HasChanged( AKPITCHSHIFTERPARAMID_FILTERGAIN ) || rChanges.HasChanged( AKPITCHSHIFTERPARAMID_FILTERQFACTOR ) )
			&& m_FXInfo.Params.Voice.Filter.eFilterType != AKFILTERTYPE_NONE )
		{
			m_FXInfo.Filter.ComputeCoefs( (AK::DSP::MultiChannelBiquadFilter<6>::FilterType)( m_FXInfo.Params.Voice.Filter.eFilterType - 1 ),
				m_FXInfo.uSampleRate, m_FXInfo.Params.Voice.Filter.fFilterFrequency, m_FXInfo.Params.Voice.Filter.fFilterGain, m_FXInfo.Params.Voice.Filter.fFilterQFactor );
		}
	}
	rChanges.ResetAllParamChanges();
	AkReal32 * pfBufferStorage = NULL;
	if( m_FXInfo.uNumProcessedChannels )
	{
		pfBufferStorage = (AkReal32 *)AK_PLUGIN_ALLOC( m_pAllocator, sizeof( AkReal32 ) * io_pBuffer->MaxFrames() );
		if( !pfBufferStorage )
		{
			return;
		}
	}
	AkPitchShifterDSPProcess( io_pBuffer, m_FXInfo, pfBufferStorage, NULL );
	if( pfBufferStorage )
	{
		AK_PLUGIN_FREE( m_pAllocator, pfBufferStorage );
	}
}
