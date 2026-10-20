/*=============================================================================
	AkRoomVerbFX.cpp - CAkRoomVerbFX (2012 0x988ec0..0x98c580; CreateRoomVerbFX 0x98bb10, 2013 matched by bytes).
	Agent IH4 fork F1 (wave 26).

	Deviations (named):
	  * ProcessSpread1Out / 2Out / 3Out (0x989960, 0x98a060, 0x98aab0) share their identical front half (wet input,
	    reverb and ER delays, ER unit and its tone filters, ER front/back delays, diffusion, FDN units, gain, DC
	    filters) in ProcessFront; the three channel mixes are retail's, call for call.
	  * ComputeTCCoefs1/2/3 (0x988ff0, 0x989100, 0x989220) are one function of the filter number.
=============================================================================*/
#include "AkRoomVerbFX.h"
#include "DelayLengths.h"
#include "ConstantPowerChannelMixdown.h"
#include "AkApplyGain.h"
#include <math.h>
#include <string.h>

/** 2012 0x98bb10. */
AK::IAkPlugin * CreateRoomVerbFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkRoomVerbFX() );
}

/** 2012 0x98b530. */
CAkRoomVerbFX::CAkRoomVerbFX()
	: m_pReverbUnitsState( NULL )
	, m_pTCFiltersState( NULL )
	, m_pERUnit( NULL )
	, m_pParams( NULL )
	, m_pAllocator( NULL )
{
	memset( &m_Reverb.uTailLength, 0, sizeof( ReverbState ) - offsetof( ReverbState, uTailLength ) );
	memset( &m_PrevRTPCParams, 0, sizeof( m_PrevRTPCParams ) );
	memset( &m_PrevInvariantParams, 0, sizeof( m_PrevInvariantParams ) );
}

/** 2012 0x989540. */
AKRESULT CAkRoomVerbFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bIsAsynchronous = false;
	return AK_Success;
}

/** 2012 0x98bb30: a send (environment) instance outputs no dry signal; a mono-LFE bus gets no early reflections. */
AKRESULT CAkRoomVerbFX::Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat )
{
	m_Reverb.uSampleRate = in_rFormat.uSampleRate;
	m_pParams = static_cast<CAkRoomVerbFXParams *>( in_pParams );
	m_pAllocator = in_pAllocator;
	m_Reverb.uNumReverbUnits = m_pParams->sInvariantParams.uNumReverbUnits;
	m_Reverb.bIsSentMode = in_pFXCtx->IsSendModeEffect();
	if( m_Reverb.bIsSentMode )
	{
		m_pParams->sRTPCParams.fDryLevel = 0.f;
	}
	m_PrevRTPCParams = m_pParams->sRTPCParams;
	m_Reverb.uNumERSignals = m_pParams->sInvariantParams.bEnableEarlyReflections ? 2 : 0;
	if( in_rFormat.uChannelMask == AK_SPEAKER_LOW_FREQUENCY )
	{
		m_Reverb.uNumERSignals = 0;
	}
	SetupDCFilters();
	AKRESULT eResult = SetupToneControlFilters();
	if( eResult == AK_Success )
	{
		eResult = SetupFDNs( in_pAllocator );
	}
	if( eResult == AK_Success )
	{
		eResult = SetupERDelay( in_pAllocator );
	}
	if( eResult == AK_Success )
	{
		eResult = SetupReverbDelay( in_pAllocator );
	}
	if( eResult == AK_Success )
	{
		eResult = SetupERUnit( in_pAllocator );
	}
	if( eResult == AK_Success )
	{
		eResult = SetupERFrontBackDelay( in_pAllocator, in_rFormat.uChannelMask );
	}
	if( eResult == AK_Success )
	{
		eResult = SetupDiffusionAPF( in_pAllocator );
	}
	if( eResult == AK_Success )
	{
		m_Reverb.uTailLength = (AkUInt32)(AkInt64)( ( m_pParams->sInvariantParams.fReverbDelay * 0.001 + m_pParams->sRTPCParams.fDecayTime ) * (double)m_Reverb.uSampleRate );
	}
	return eResult;
}

/** 2012 0x98b460. */
AKRESULT CAkRoomVerbFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	m_Reverb.ERDelay.Term( in_pAllocator );
	m_Reverb.ReverbDelay.Term( in_pAllocator );
	m_Reverb.ERFrontBackDelay[ 0 ].Term( in_pAllocator );
	m_Reverb.ERFrontBackDelay[ 1 ].Term( in_pAllocator );
	TermERUnit( in_pAllocator );
	if( m_pTCFiltersState )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pTCFiltersState );
		m_pTCFiltersState = NULL;
	}
	TermFDNs( in_pAllocator );
	TermDiffusionAPF( in_pAllocator );
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x989700. */
AKRESULT CAkRoomVerbFX::Reset()
{
	m_Reverb.ERDelay.Reset();
	m_Reverb.ReverbDelay.Reset();
	m_Reverb.ERFrontBackDelay[ 0 ].Reset();
	m_Reverb.ERFrontBackDelay[ 1 ].Reset();
	ResetERUnit();
	ResetDCFilters();
	ResetToneControlFilters();
	ResetFDNs();
	ResetDiffusionAPF();
	return AK_Success;
}

/** 2012 0x98bd80: the ER line holds the delay before the pattern's first tap (scaled by room size). */
AKRESULT CAkRoomVerbFX::SetupERDelay( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( !m_Reverb.uNumERSignals )
	{
		return AK_Success;
	}
	const AkRoomVerb::ERPatternInfo & rPattern = AkRoomVerb::g_ERPatternsList[ m_pParams->sInvariantParams.uERPattern ];
	const AkReal32 fScale = (AkReal32)pow( 2.0, m_pParams->sInvariantParams.fRoomSize * 0.0099999998f );
	const AkReal32 fRight = rPattern.TapInfoRight->fTapTime * fScale;
	const AkReal32 fLeft = rPattern.TapInfoLeft->fTapTime * fScale;
	const AkReal32 fMinTapTime = fRight <= fLeft ? fRight : fLeft;
	const AkUInt32 uLength = (AkUInt32)(AkInt64)( (double)m_Reverb.uSampleRate * fMinTapTime * 0.001 );
	if( !uLength )
	{
		return AK_Success;
	}
	return m_Reverb.ERDelay.Init( in_pAllocator, uLength );
}

/** 2012 0x98be70. */
AKRESULT CAkRoomVerbFX::SetupReverbDelay( AK::IAkPluginMemAlloc * in_pAllocator )
{
	const AkUInt32 uLength = (AkUInt32)(AkInt64)( (double)m_Reverb.uSampleRate * m_pParams->sInvariantParams.fReverbDelay * 0.001 );
	if( !uLength )
	{
		return AK_Success;
	}
	return m_Reverb.ReverbDelay.Init( in_pAllocator, uLength );
}

/** 2012 0x98bef0: one line per back channel. */
AKRESULT CAkRoomVerbFX::SetupERFrontBackDelay( AK::IAkPluginMemAlloc * in_pAllocator, AkUInt32 in_uChannelMask )
{
	if( !m_Reverb.uNumERSignals )
	{
		return AK_Success;
	}
	const AkUInt32 uLength = (AkUInt32)(AkInt64)( (double)m_Reverb.uSampleRate * m_pParams->sInvariantParams.fERFrontBackDelay * 0.001 );
	if( !uLength )
	{
		return AK_Success;
	}
	if( in_uChannelMask & AK_SPEAKER_BACK_LEFT )
	{
		const AKRESULT eResult = m_Reverb.ERFrontBackDelay[ 0 ].Init( in_pAllocator, uLength );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	if( in_uChannelMask & AK_SPEAKER_BACK_RIGHT )
	{
		return m_Reverb.ERFrontBackDelay[ 1 ].Init( in_pAllocator, uLength );
	}
	return AK_Success;
}

/** 2012 0x98bfa0. */
AKRESULT CAkRoomVerbFX::SetupERUnit( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( !m_Reverb.uNumERSignals )
	{
		return AK_Success;
	}
	const AkRoomVerb::ERPatternInfo & rPattern = AkRoomVerb::g_ERPatternsList[ m_pParams->sInvariantParams.uERPattern ];
	m_pERUnit = AK_PLUGIN_NEW( in_pAllocator, DSP::ERUnitDual() );
	if( !m_pERUnit )
	{
		return AK_InsufficientMemory;
	}
	return m_pERUnit->Init( in_pAllocator, m_pParams->sInvariantParams.fRoomSize, rPattern.TapInfoLeft, rPattern.TapInfoRight, rPattern.uNumTapsLeft, rPattern.uNumTapsRight, m_Reverb.uSampleRate );
}

/** 2012 0x98c250: [0] is the number of tone-control biquads (an ER-only filter needs one per ER signal), [1..3] each
    filter's first biquad. */
AKRESULT CAkRoomVerbFX::SetupToneControlFilters()
{
	AkUInt8 uNumFilters = 0;
	const InvariantParams & rInvariant = m_pParams->sInvariantParams;
	if( rInvariant.bEnableToneControls )
	{
		const FilterInsertType aPos[ 3 ] = { rInvariant.eFilter1Pos, rInvariant.eFilter2Pos, rInvariant.eFilter3Pos };
		for( AkUInt32 i = 0; i < 3; ++i )
		{
			m_Reverb.uTCFilterIndex[ i + 1 ] = uNumFilters;
			if( aPos[ i ] == FILTERINSERTTYPE_ERONLY )
			{
				uNumFilters += m_Reverb.uNumERSignals;
			}
			else if( aPos[ i ] == FILTERINSERTTYPE_REVERBONLY || aPos[ i ] == FILTERINSERTTYPE_ERANDREVERB )
			{
				++uNumFilters;
			}
		}
	}
	m_Reverb.uTCFilterIndex[ 0 ] = uNumFilters;
	if( uNumFilters )
	{
		m_pTCFiltersState = (ToneControlsState *)AK_PLUGIN_ALLOC( m_pAllocator, sizeof( ToneControlsState ) * uNumFilters );
		if( !m_pTCFiltersState )
		{
			return AK_InsufficientMemory;
		}
		for( AkUInt32 i = 0; i < uNumFilters; ++i )
		{
			new( &m_pTCFiltersState[ i ].Filter ) DSP::BiquadFilter();
		}
	}
	ComputeTCCoefs1();
	ComputeTCCoefs2();
	ComputeTCCoefs3();
	return AK_Success;
}

/** 2012 0x98c490: four allpasses from the longest jittered delay down, gains from the diffusion (staggered by 0.618). */
AKRESULT CAkRoomVerbFX::SetupDiffusionAPF( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AkReal32 fAPFDelayTimes[ 4 ];
	AkUInt32 uAPFDelayLengths[ 4 ];
	DelayLengths::ComputeDiffusionFiltersDelayTimes( m_pParams->sAlgoTunings, 4, fAPFDelayTimes );
	DelayLengths::ComputePrimeDelayLengths( m_Reverb.uSampleRate, uAPFDelayLengths, fAPFDelayTimes, 4 );
	AKRESULT eResult = AK_Success;
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		const double dGain = m_pParams->sRTPCParams.fDiffusion * 0.0247212 - (double)( 3 - i ) * 0.61803001;
		AkReal32 fGain = 0.f;
		if( dGain > 0.0 )
		{
			fGain = (AkReal32)dGain;
			if( fGain >= 0.61803001f )
			{
				fGain = 0.61803001f;
			}
		}
		eResult = m_Reverb.DiffusionFilters[ i ].Init( in_pAllocator, uAPFDelayLengths[ i ], fGain );
		if( eResult != AK_Success )
		{
			break;
		}
	}
	return eResult;
}

/** 2012 0x98c580: N units of four prime lines each (line i of unit u is the (u + iN)-th sorted delay), the mix gain
    1/sqrt(N), each unit behind its own jittered input delay. */
AKRESULT CAkRoomVerbFX::SetupFDNs( AK::IAkPluginMemAlloc * in_pAllocator )
{
	m_pReverbUnitsState = (ReverbUnitState *)AK_PLUGIN_ALLOC( in_pAllocator, sizeof( ReverbUnitState ) * m_Reverb.uNumReverbUnits );
	if( !m_pReverbUnitsState )
	{
		return AK_InsufficientMemory;
	}
	for( AkUInt32 i = 0; i < m_Reverb.uNumReverbUnits; ++i )
	{
		memset( &m_pReverbUnitsState[ i ], 0, sizeof( ReverbUnitState ) );
	}
	const AkUInt32 uNumUnits = m_Reverb.uNumReverbUnits;
	m_Reverb.fReverbUnitsMixGain = 1.f;
	if( uNumUnits > 1 )
	{
		m_Reverb.fReverbUnitsMixGain = (AkReal32)( 1.0 / sqrt( (double)uNumUnits ) );
	}
	AkReal32 fFDNDelayTimes[ 64 ];
	AkUInt32 uFDNDelayLengths[ 64 ];
	AkUInt32 uReorderedFDNDelayLengths[ 64 ];
	AkReal32 fFDNInputDelayTimes[ 16 ];
	AkUInt32 uFDNInputDelayLengths[ 16 ];
	DelayLengths::ComputeFDNDelayTimes( m_pParams->sAlgoTunings, m_pParams->sInvariantParams.fDensity, m_pParams->sInvariantParams.fRoomShape, 4 * uNumUnits, fFDNDelayTimes );
	DelayLengths::ComputePrimeDelayLengths( m_Reverb.uSampleRate, uFDNDelayLengths, fFDNDelayTimes, 4 * uNumUnits );
	for( AkUInt32 u = 0; u < uNumUnits; ++u )
	{
		uReorderedFDNDelayLengths[ 4 * u + 0 ] = uFDNDelayLengths[ u ];
		uReorderedFDNDelayLengths[ 4 * u + 1 ] = uFDNDelayLengths[ u + uNumUnits ];
		uReorderedFDNDelayLengths[ 4 * u + 2 ] = uFDNDelayLengths[ u + 2 * uNumUnits ];
		uReorderedFDNDelayLengths[ 4 * u + 3 ] = uFDNDelayLengths[ u + 3 * uNumUnits ];
	}
	DelayLengths::ComputeFDNInputDelayTimes( m_pParams->sAlgoTunings, uNumUnits, fFDNInputDelayTimes );
	const double dSampleRate = (double)m_Reverb.uSampleRate;
	for( AkUInt32 u = 0; u < uNumUnits; ++u )
	{
		uFDNInputDelayLengths[ u ] = (AkUInt32)(AkInt64)( fFDNInputDelayTimes[ u ] * dSampleRate * 0.001 );
	}
	for( AkUInt32 u = 0; u < m_Reverb.uNumReverbUnits; ++u )
	{
		AKRESULT eResult = m_pReverbUnitsState[ u ].ReverbUnits.Init( in_pAllocator, &uReorderedFDNDelayLengths[ 4 * u ], m_pParams->sRTPCParams.fDecayTime, m_pParams->sRTPCParams.fHFDamping, m_Reverb.uSampleRate );
		if( eResult != AK_Success )
		{
			return eResult;
		}
		eResult = m_pReverbUnitsState[ u ].RUInputDelay.Init( in_pAllocator, uFDNInputDelayLengths[ u ] );
		if( eResult != AK_Success )
		{
			return eResult;
		}
	}
	return AK_Success;
}

/** 2012 0x988ed0: a1 = 1 - 2 pi fc / fs on the six reverb outputs. */
void CAkRoomVerbFX::SetupDCFilters()
{
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		m_Reverb.DCFilter[ i ].SetCoefs( (AkReal32)( 1.0 - m_pParams->sAlgoTunings.fDCFilterCutFreq * 6.2831855f / (double)m_Reverb.uSampleRate ) );
	}
}

/** 2012 0x988ff0 / 0x989100 / 0x989220: the filter's biquad (two for an ER-only filter with two ER signals). */
void CAkRoomVerbFX::ComputeTCCoefs( AkUInt32 in_uFilter )
{
	const InvariantParams & rInvariant = m_pParams->sInvariantParams;
	if( !rInvariant.bEnableToneControls )
	{
		return;
	}
	const FilterInsertType aPos[ 3 ] = { rInvariant.eFilter1Pos, rInvariant.eFilter2Pos, rInvariant.eFilter3Pos };
	const FilterCurveType aCurve[ 3 ] = { rInvariant.eFilter1Curve, rInvariant.eFilter2Curve, rInvariant.eFilter3Curve };
	const RTPCParams & rRTPC = m_pParams->sRTPCParams;
	const AkReal32 aFreq[ 3 ] = { rRTPC.fFilter1Freq, rRTPC.fFilter2Freq, rRTPC.fFilter3Freq };
	const AkReal32 aGain[ 3 ] = { rRTPC.fFilter1Gain, rRTPC.fFilter2Gain, rRTPC.fFilter3Gain };
	const AkReal32 aQ[ 3 ] = { rRTPC.fFilter1Q, rRTPC.fFilter2Q, rRTPC.fFilter3Q };
	const FilterInsertType ePos = aPos[ in_uFilter ];
	if( ePos == FILTERINSERTTYPE_OFF || ( ePos == FILTERINSERTTYPE_ERONLY && !m_Reverb.uNumERSignals ) )
	{
		return;
	}
	const AkUInt32 uIndex = m_Reverb.uTCFilterIndex[ in_uFilter + 1 ];
	m_pTCFiltersState[ uIndex ].Filter.ComputeCoefs( (DSP::BiquadFilter::FilterType)aCurve[ in_uFilter ], m_Reverb.uSampleRate, aFreq[ in_uFilter ], aGain[ in_uFilter ], aQ[ in_uFilter ] );
	m_pTCFiltersState[ uIndex ].FilterPos = ePos;
	if( ePos == FILTERINSERTTYPE_ERONLY && m_Reverb.uNumERSignals == 2 )
	{
		m_pTCFiltersState[ uIndex + 1 ].Filter.ComputeCoefs( (DSP::BiquadFilter::FilterType)aCurve[ in_uFilter ], m_Reverb.uSampleRate, aFreq[ in_uFilter ], aGain[ in_uFilter ], aQ[ in_uFilter ] );
		m_pTCFiltersState[ uIndex + 1 ].FilterPos = ePos;
	}
}

/** 2012 0x989340. */
void CAkRoomVerbFX::TermToneControlFilters( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( m_pTCFiltersState )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pTCFiltersState );
		m_pTCFiltersState = NULL;
	}
}

/** 2012 0x989370. */
void CAkRoomVerbFX::TermFDNs( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( !m_pReverbUnitsState )
	{
		return;
	}
	for( AkUInt32 u = 0; u < m_Reverb.uNumReverbUnits; ++u )
	{
		m_pReverbUnitsState[ u ].ReverbUnits.Term( in_pAllocator );
		m_pReverbUnitsState[ u ].RUInputDelay.Term( in_pAllocator );
	}
	AK_PLUGIN_FREE( in_pAllocator, m_pReverbUnitsState );
	m_pReverbUnitsState = NULL;
}

/** 2012 0x9893e0. */
void CAkRoomVerbFX::TermDiffusionAPF( AK::IAkPluginMemAlloc * in_pAllocator )
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		m_Reverb.DiffusionFilters[ i ].Term( in_pAllocator );
	}
}

/** 2012 0x9896b0. */
void CAkRoomVerbFX::TermERUnit( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( m_Reverb.uNumERSignals && m_pERUnit )
	{
		m_pERUnit->Term( in_pAllocator );
		AK_PLUGIN_FREE( in_pAllocator, m_pERUnit );
		m_pERUnit = NULL;
	}
}

/** 2012 0x989410. */
void CAkRoomVerbFX::ResetFDNs()
{
	if( !m_pReverbUnitsState )
	{
		return;
	}
	for( AkUInt32 u = 0; u < m_Reverb.uNumReverbUnits; ++u )
	{
		m_pReverbUnitsState[ u ].ReverbUnits.Reset();
		m_pReverbUnitsState[ u ].RUInputDelay.Reset();
	}
}

/** 2012 0x989460. */
void CAkRoomVerbFX::ResetDiffusionAPF()
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		m_Reverb.DiffusionFilters[ i ].Reset();
	}
}

/** 2012 0x989490. */
void CAkRoomVerbFX::ResetDCFilters()
{
	for( AkUInt32 i = 0; i < 6; ++i )
	{
		m_Reverb.DCFilter[ i ].Reset();
	}
}

/** 2012 0x9894e0. */
void CAkRoomVerbFX::ResetToneControlFilters()
{
	if( !m_pParams->sInvariantParams.bEnableToneControls )
	{
		return;
	}
	for( AkUInt32 i = 0; i < m_Reverb.uTCFilterIndex[ 0 ]; ++i )
	{
		m_pTCFiltersState[ i ].Filter.Reset();
	}
}

/** 2012 0x989530. */
void CAkRoomVerbFX::ResetERUnit()
{
	if( m_pERUnit )
	{
		m_pERUnit->Reset();
	}
}

/** 2012 0x98c040: allpass gains (0 .. 0.618) staggered by 0.618 along the diffusion. */
void CAkRoomVerbFX::ChangeDiffusion()
{
	const AkReal32 fDiffusion = m_pParams->sRTPCParams.fDiffusion * 0.0247212f;
	const AkReal32 aOffset[ 4 ] = { 1.85409f, 1.23606f, 0.61803001f, 0.f };
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		AkReal32 fGain = fDiffusion - aOffset[ i ];
		if( fGain <= 0.f )
		{
			fGain = 0.f;
		}
		else if( fGain >= 0.61803001f )
		{
			fGain = 0.61803001f;
		}
		m_Reverb.DiffusionFilters[ i ].fG = fGain;
	}
}

/** 2012 0x98c100. */
void CAkRoomVerbFX::ChangeDecay()
{
	for( AkUInt32 u = 0; u < m_Reverb.uNumReverbUnits; ++u )
	{
		m_pReverbUnitsState[ u ].ReverbUnits.ChangeDecay( m_pParams->sRTPCParams.fDecayTime, m_pParams->sRTPCParams.fHFDamping, m_Reverb.uSampleRate );
	}
}

/** 2012 0x98b590: rebuild what an invariant parameter change invalidates; true = an allocation failed (no output). */
bool CAkRoomVerbFX::LiveParametersUpdate( AkAudioBuffer * io_pBuffer )
{
	InvariantParams & rNew = m_pParams->sInvariantParams;
	if( m_PrevInvariantParams.uNumReverbUnits != rNew.uNumReverbUnits || m_PrevInvariantParams.fRoomShape != rNew.fRoomShape || m_PrevInvariantParams.fDensity != rNew.fDensity )
	{
		TermFDNs( m_pAllocator );
		m_Reverb.uNumReverbUnits = rNew.uNumReverbUnits;
		if( SetupFDNs( m_pAllocator ) != AK_Success )
		{
			return true;
		}
		ResetFDNs();
	}
	const bool bToneFiltersPositioned = rNew.bEnableToneControls && ( rNew.eFilter1Pos || rNew.eFilter2Pos || rNew.eFilter3Pos );
	if( m_PrevInvariantParams.bEnableEarlyReflections != rNew.bEnableEarlyReflections )
	{
		m_Reverb.ERFrontBackDelay[ 0 ].Term( m_pAllocator );
		m_Reverb.ERFrontBackDelay[ 1 ].Term( m_pAllocator );
		m_Reverb.ERDelay.Term( m_pAllocator );
		TermERUnit( m_pAllocator );
		if( bToneFiltersPositioned && m_pTCFiltersState )
		{
			AK_PLUGIN_FREE( m_pAllocator, m_pTCFiltersState );
			m_pTCFiltersState = NULL;
		}
		m_Reverb.uNumERSignals = rNew.bEnableEarlyReflections ? 2 : 0;
		if( io_pBuffer->uChannelMask == AK_SPEAKER_LOW_FREQUENCY )
		{
			m_Reverb.uNumERSignals = 0;
		}
		if( SetupERFrontBackDelay( m_pAllocator, io_pBuffer->uChannelMask ) != AK_Success )
		{
			return true;
		}
		m_Reverb.ERFrontBackDelay[ 0 ].Reset();
		m_Reverb.ERFrontBackDelay[ 1 ].Reset();
		if( SetupERDelay( m_pAllocator ) != AK_Success )
		{
			return true;
		}
		m_Reverb.ERDelay.Reset();
		if( SetupERUnit( m_pAllocator ) != AK_Success )
		{
			return true;
		}
		ResetERUnit();
		if( bToneFiltersPositioned )
		{
			if( SetupToneControlFilters() != AK_Success )
			{
				return true;
			}
			ResetToneControlFilters();
		}
	}
	if( m_PrevInvariantParams.fERFrontBackDelay != rNew.fERFrontBackDelay )
	{
		m_Reverb.ERFrontBackDelay[ 0 ].Term( m_pAllocator );
		m_Reverb.ERFrontBackDelay[ 1 ].Term( m_pAllocator );
		if( SetupERFrontBackDelay( m_pAllocator, io_pBuffer->uChannelMask ) != AK_Success )
		{
			return true;
		}
		m_Reverb.ERFrontBackDelay[ 0 ].Reset();
		m_Reverb.ERFrontBackDelay[ 1 ].Reset();
	}
	if( m_PrevInvariantParams.fReverbDelay != rNew.fReverbDelay )
	{
		m_Reverb.ReverbDelay.Term( m_pAllocator );
		if( SetupReverbDelay( m_pAllocator ) != AK_Success )
		{
			return true;
		}
		m_Reverb.ReverbDelay.Reset();
		m_Reverb.uTailLength = (AkUInt32)(AkInt64)( ( rNew.fReverbDelay * 0.001 + m_pParams->sRTPCParams.fDecayTime ) * (double)m_Reverb.uSampleRate );
	}
	if( m_PrevInvariantParams.uERPattern != rNew.uERPattern || m_PrevInvariantParams.fRoomSize != rNew.fRoomSize )
	{
		m_Reverb.ERDelay.Term( m_pAllocator );
		if( SetupERDelay( m_pAllocator ) != AK_Success )
		{
			return true;
		}
		m_Reverb.ERDelay.Reset();
		TermERUnit( m_pAllocator );
		if( SetupERUnit( m_pAllocator ) != AK_Success )
		{
			return true;
		}
		ResetERUnit();
	}
	if( ( m_PrevInvariantParams.bEnableToneControls != rNew.bEnableToneControls || m_PrevInvariantParams.eFilter1Pos != rNew.eFilter1Pos
			|| m_PrevInvariantParams.eFilter2Pos != rNew.eFilter2Pos || m_PrevInvariantParams.eFilter3Pos != rNew.eFilter3Pos )
		&& bToneFiltersPositioned )
	{
		TermToneControlFilters( m_pAllocator );
		if( SetupToneControlFilters() != AK_Success )
		{
			return true;
		}
		ResetToneControlFilters();
	}
	if( m_PrevInvariantParams.eFilter1Curve != rNew.eFilter1Curve )
	{
		ComputeTCCoefs1();
	}
	if( m_PrevInvariantParams.eFilter2Curve != rNew.eFilter2Curve )
	{
		ComputeTCCoefs2();
	}
	if( m_PrevInvariantParams.eFilter3Curve != rNew.eFilter3Curve )
	{
		ComputeTCCoefs3();
	}
	m_PrevInvariantParams = rNew;
	rNew.bDirty = false;
	return false;
}

/** 2012 0x9897c0: an environment instance keeps its dry level at 0 whatever the parameters say. */
void CAkRoomVerbFX::RTPCParametersUpdate()
{
	RTPCParams & rNew = m_pParams->sRTPCParams;
	if( rNew.fDecayTime != m_PrevRTPCParams.fDecayTime || rNew.fHFDamping != m_PrevRTPCParams.fHFDamping )
	{
		ChangeDecay();
		m_Reverb.uTailLength = (AkUInt32)(AkInt64)( ( m_pParams->sInvariantParams.fReverbDelay * 0.001 + rNew.fDecayTime ) * (double)m_Reverb.uSampleRate );
	}
	if( rNew.fDiffusion != m_PrevRTPCParams.fDiffusion )
	{
		ChangeDiffusion();
	}
	if( rNew.fFilter1Gain != m_PrevRTPCParams.fFilter1Gain || rNew.fFilter1Freq != m_PrevRTPCParams.fFilter1Freq || rNew.fFilter1Q != m_PrevRTPCParams.fFilter1Q )
	{
		ComputeTCCoefs1();
	}
	if( rNew.fFilter2Gain != m_PrevRTPCParams.fFilter2Gain || rNew.fFilter2Freq != m_PrevRTPCParams.fFilter2Freq || rNew.fFilter2Q != m_PrevRTPCParams.fFilter2Q )
	{
		ComputeTCCoefs2();
	}
	if( rNew.fFilter3Gain != m_PrevRTPCParams.fFilter3Gain || rNew.fFilter3Freq != m_PrevRTPCParams.fFilter3Freq || rNew.fFilter3Q != m_PrevRTPCParams.fFilter3Q )
	{
		ComputeTCCoefs3();
	}
	if( m_Reverb.bIsSentMode )
	{
		rNew.fDryLevel = 0.f;
	}
	rNew.bDirty = false;
}

/** 2012 0x989560: the constant-power mono sum, through the ER-and-reverb tone filters. */
void CAkRoomVerbFX::WetPreProcess( AkAudioBuffer * in_pBuffer, AkReal32 * out_pfWetIn, AkUInt32 in_uNumFrames, AkUInt32 in_uFrameOffset )
{
	DSP::ConstantPowerChannelMixdown( in_pBuffer, in_uNumFrames, in_uFrameOffset, out_pfWetIn, in_pBuffer->uChannelMask,
		m_pParams->sInvariantParams.fInputCenterLevel, m_pParams->sInvariantParams.fInputLFELevel );
	if( !m_pParams->sInvariantParams.bEnableToneControls )
	{
		return;
	}
	for( AkUInt32 i = 0; i < m_Reverb.uTCFilterIndex[ 0 ]; ++i )
	{
		if( m_pTCFiltersState[ i ].FilterPos == FILTERINSERTTYPE_ERANDREVERB )
		{
			m_pTCFiltersState[ i ].Filter.ProcessBuffer( out_pfWetIn, in_uNumFrames );
		}
	}
}

/** 2012 0x9895f0: the reverb-only tone filters. */
void CAkRoomVerbFX::ReverbPreProcess( AkReal32 * io_pfBuffer, AkUInt32 in_uNumFrames )
{
	if( !m_pParams->sInvariantParams.bEnableToneControls )
	{
		return;
	}
	for( AkUInt32 i = 0; i < m_Reverb.uTCFilterIndex[ 0 ]; ++i )
	{
		if( m_pTCFiltersState[ i ].FilterPos == FILTERINSERTTYPE_REVERBONLY )
		{
			m_pTCFiltersState[ i ].Filter.ProcessBuffer( io_pfBuffer, in_uNumFrames );
		}
	}
}

/** 2012 0x989650: the unit mix gain, then a DC blocker per output. */
void CAkRoomVerbFX::ReverbPostProcess( AkReal32 ** out_ppfReverb, AkUInt32 in_uNumOutSignals, AkReal32 in_fGain, AkUInt32 in_uNumFrames )
{
	for( AkUInt32 i = 0; i < in_uNumOutSignals; ++i )
	{
		AK::DSP::ApplyGain( out_ppfReverb[ i ], in_uNumFrames, in_fGain );
	}
	for( AkUInt32 i = 0; i < in_uNumOutSignals; ++i )
	{
		m_Reverb.DCFilter[ i ].ProcessBuffer( out_ppfReverb[ i ], in_uNumFrames );
	}
}

/** The front half shared by the three ProcessSpread functions (see the banner). in_pfTemp: [0..255] wet / ER input,
    [256..511] reverb input. io_ppfEROut [0], [1] the ER outputs, [2], [3] their back copies (front/back delay or alias). */
void CAkRoomVerbFX::ProcessFront( AkAudioBuffer * io_pBuffer, AkReal32 * in_pfTemp, AkReal32 ** io_ppfEROut, bool in_bFrontBack, AkReal32 ** out_ppfReverbOut, AkUInt32 in_uNumReverbOut, AkUInt32 in_uNumFrames, AkUInt32 in_uFrameOffset )
{
	AkReal32 * pfWet = in_pfTemp;
	AkReal32 * pfReverbIn = in_pfTemp + 256;
	WetPreProcess( io_pBuffer, pfWet, in_uNumFrames, in_uFrameOffset );
	if( m_Reverb.ReverbDelay.uDelayLineLength )
	{
		m_Reverb.ReverbDelay.ProcessBuffer( pfWet, pfReverbIn, in_uNumFrames );
	}
	else
	{
		memcpy( pfReverbIn, pfWet, sizeof( AkReal32 ) * in_uNumFrames );
	}
	if( m_Reverb.ERDelay.uDelayLineLength )
	{
		m_Reverb.ERDelay.ProcessBuffer( pfWet, in_uNumFrames );
	}
	if( m_Reverb.uNumERSignals )
	{
		m_pERUnit->ProcessBuffer( pfWet, io_ppfEROut[ 0 ], io_ppfEROut[ 1 ], in_uNumFrames );
		if( m_pParams->sInvariantParams.bEnableToneControls )
		{
			for( AkUInt32 i = 0; i < m_Reverb.uTCFilterIndex[ 0 ]; ++i )
			{
				if( m_pTCFiltersState[ i ].FilterPos == FILTERINSERTTYPE_ERONLY )
				{
					m_pTCFiltersState[ i ].Filter.ProcessBuffer( io_ppfEROut[ 0 ], in_uNumFrames );
					m_pTCFiltersState[ i + 1 ].Filter.ProcessBuffer( io_ppfEROut[ 1 ], in_uNumFrames );
					++i;
				}
			}
		}
		if( in_bFrontBack )
		{
			if( m_Reverb.ERFrontBackDelay[ 0 ].uDelayLineLength )
			{
				m_Reverb.ERFrontBackDelay[ 0 ].ProcessBuffer( io_ppfEROut[ 0 ], io_ppfEROut[ 2 ], in_uNumFrames );
				m_Reverb.ERFrontBackDelay[ 1 ].ProcessBuffer( io_ppfEROut[ 1 ], io_ppfEROut[ 3 ], in_uNumFrames );
			}
			else
			{
				io_ppfEROut[ 2 ] = io_ppfEROut[ 0 ];
				io_ppfEROut[ 3 ] = io_ppfEROut[ 1 ];
			}
		}
	}
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		m_Reverb.DiffusionFilters[ i ].ProcessBuffer( pfReverbIn, in_uNumFrames );
	}
	ReverbPreProcess( pfReverbIn, in_uNumFrames );
	memset( out_ppfReverbOut[ 0 ], 0, sizeof( AkReal32 ) * 256 * in_uNumReverbOut );
	for( AkUInt32 u = 0; u < m_Reverb.uNumReverbUnits; ++u )
	{
		m_pReverbUnitsState[ u ].RUInputDelay.ProcessBuffer( pfReverbIn, in_uNumFrames );
		DSP::FDN4 & rUnit = m_pReverbUnitsState[ u ].ReverbUnits;
		if( in_uNumReverbOut == 2 )
		{
			rUnit.ProcessBufferAccum( pfReverbIn, out_ppfReverbOut[ u & 1 ], in_uNumFrames );
		}
		else if( in_uNumReverbOut == 4 )
		{
			rUnit.ProcessBufferAccum( pfReverbIn, out_ppfReverbOut[ u & 1 ], out_ppfReverbOut[ ( u & 1 ) + 2 ], in_uNumFrames );
		}
		else
		{
			rUnit.ProcessBufferAccum( pfReverbIn, out_ppfReverbOut[ u & 1 ], out_ppfReverbOut[ ( u & 1 ) + 2 ], out_ppfReverbOut[ ( u & 1 ) + 4 ], in_uNumFrames );
		}
	}
	ReverbPostProcess( out_ppfReverbOut, in_uNumReverbOut, (AkReal32)( m_Reverb.fReverbUnitsMixGain * 1.4142135f ), in_uNumFrames );
}

/** The stereo-width panning pair: g1 = 0.7071 + 0.2929 width/180, g2 = sqrt(1 - g1^2). */
static void AkStereoWidthGains( AkReal32 in_fStereoWidth, AkReal32 & out_fGain1, AkReal32 & out_fGain2 )
{
	out_fGain1 = in_fStereoWidth * 0.0055555557f * 0.29289401f + 0.70710599f;
	const AkReal32 fSquare = 1.f - out_fGain1 * out_fGain1;
	out_fGain2 = fSquare <= 0.f ? 0.f : (AkReal32)sqrt( fSquare );
}

/** 2012 0x989960: mono, stereo, or front + center / LFE layouts without back channels. */
AKRESULT CAkRoomVerbFX::ProcessSpread1Out( AkAudioBuffer * io_pBuffer )
{
	AkReal32 * pfTemp = (AkReal32 *)AK_PLUGIN_ALLOC( m_pAllocator, ( m_Reverb.uNumERSignals + 4 ) << 10 );
	if( !pfTemp )
	{
		return AK_InsufficientMemory;
	}
	AkReal32 * pfReverbOut[ 2 ] = { pfTemp + 512, pfTemp + 768 };
	AkReal32 * pfEROut[ 4 ] = { NULL, NULL, NULL, NULL };
	if( m_Reverb.uNumERSignals )
	{
		pfEROut[ 0 ] = pfTemp + 1024;
		pfEROut[ 1 ] = pfTemp + 1280;
	}
	AkReal32 fPrevGain1, fPrevGain2, fGain1, fGain2;
	AkStereoWidthGains( m_PrevRTPCParams.fStereoWidth, fPrevGain1, fPrevGain2 );
	AkStereoWidthGains( m_pParams->sRTPCParams.fStereoWidth, fGain1, fGain2 );
	const AkUInt32 uChannelMask = io_pBuffer->uChannelMask;
	AkUInt32 uFrameOffset = 0;
	AkUInt32 uNumFramesRemaining = io_pBuffer->uValidFrames;
	while( uNumFramesRemaining )
	{
		const AkUInt32 uFrames = uNumFramesRemaining >= 256 ? 256 : uNumFramesRemaining;
		ProcessFront( io_pBuffer, pfTemp, pfEROut, false, pfReverbOut, 2, uFrames, uFrameOffset );
		const RTPCParams & rNew = m_pParams->sRTPCParams;
		AkUInt32 uChannel = 0;
		if( uChannelMask & AK_SPEAKER_FRONT_LEFT )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 0 ], pfReverbOut[ 1 ], m_PrevRTPCParams.fDryLevel, rNew.fDryLevel,
				m_PrevRTPCParams.fReverbLevel * fPrevGain1, rNew.fReverbLevel * fGain1, m_PrevRTPCParams.fReverbLevel * fPrevGain2, rNew.fReverbLevel * fGain2, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				DSP::Mix3Interp( pfOut, pfEROut[ 0 ], pfEROut[ 1 ], 1.f, 1.f,
					m_PrevRTPCParams.fERLevel * fPrevGain1, rNew.fERLevel * fGain1, m_PrevRTPCParams.fERLevel * fPrevGain2, rNew.fERLevel * fGain2, uFrames );
			}
			uChannel = 1;
		}
		if( uChannelMask & AK_SPEAKER_FRONT_RIGHT )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset + uChannel * io_pBuffer->uMaxFrames;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 0 ], pfReverbOut[ 1 ], m_PrevRTPCParams.fDryLevel, rNew.fDryLevel,
				m_PrevRTPCParams.fReverbLevel * fPrevGain2, rNew.fReverbLevel * fGain2, m_PrevRTPCParams.fReverbLevel * fPrevGain1, rNew.fReverbLevel * fGain1, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				DSP::Mix3Interp( pfOut, pfEROut[ 0 ], pfEROut[ 1 ], 1.f, 1.f,
					m_PrevRTPCParams.fERLevel * fPrevGain2, rNew.fERLevel * fGain2, m_PrevRTPCParams.fERLevel * fPrevGain1, rNew.fERLevel * fGain1, uFrames );
			}
			++uChannel;
		}
		if( uChannelMask & AK_SPEAKER_FRONT_CENTER )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset + uChannel * io_pBuffer->uMaxFrames;
			const AkReal32 fTarget = rNew.fReverbLevel * 0.70710599f;
			const AkReal32 fCurrent = 0.70710599f * m_PrevRTPCParams.fReverbLevel;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 0 ], pfReverbOut[ 1 ], m_PrevRTPCParams.fDryLevel, rNew.fDryLevel, fCurrent, fTarget, fCurrent, fTarget, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				const AkReal32 fERTarget = m_pParams->sRTPCParams.fERLevel * 0.70710599f;
				const AkReal32 fERCurrent = 0.70710599f * m_PrevRTPCParams.fERLevel;
				DSP::Mix3Interp( pfOut, pfEROut[ 0 ], pfEROut[ 1 ], 1.f, 1.f, fERCurrent, fERTarget, fERCurrent, fERTarget, uFrames );
			}
			++uChannel;
		}
		if( uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset + uChannel * io_pBuffer->uMaxFrames;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fLFELevel * 0.70710599f;
			const AkReal32 fCurrent = 0.70710599f * ( m_PrevRTPCParams.fReverbLevel * m_PrevRTPCParams.fLFELevel );
			DSP::Mix3Interp( pfOut, pfReverbOut[ 0 ], pfReverbOut[ 1 ], m_PrevRTPCParams.fDryLevel, rNew.fDryLevel, fCurrent, fTarget, fCurrent, fTarget, uFrames );
		}
		uFrameOffset += uFrames;
		uNumFramesRemaining -= uFrames;
		m_PrevRTPCParams.fDryLevel = m_pParams->sRTPCParams.fDryLevel;
		m_PrevRTPCParams.fERLevel = m_pParams->sRTPCParams.fERLevel;
		fPrevGain1 = fGain1;
		m_PrevRTPCParams.fReverbLevel = m_pParams->sRTPCParams.fReverbLevel;
		fPrevGain2 = fGain2;
		m_PrevRTPCParams.fLFELevel = m_pParams->sRTPCParams.fLFELevel;
	}
	AK_PLUGIN_FREE( m_pAllocator, pfTemp );
	return AK_Success;
}

/** 2012 0x98a060: 3.0, 4.0 (with LFE) and 4.0 / quad layouts: front pair, center, back pair (ER delayed to the back). */
AKRESULT CAkRoomVerbFX::ProcessSpread2Out( AkAudioBuffer * io_pBuffer )
{
	const AkUInt32 uChannelMask = io_pBuffer->uChannelMask;
	const AkUInt32 uNumBackChannels = m_pParams->sInvariantParams.bEnableEarlyReflections ? ( ( uChannelMask & ( AK_SPEAKER_BACK_LEFT | AK_SPEAKER_BACK_RIGHT ) ) ? 2 : 0 ) : 0;
	AkReal32 * pfTemp = (AkReal32 *)AK_PLUGIN_ALLOC( m_pAllocator, ( m_Reverb.uNumERSignals + uNumBackChannels + 6 ) << 10 );
	if( !pfTemp )
	{
		return AK_InsufficientMemory;
	}
	AkReal32 * pfReverbOut[ 4 ] = { pfTemp + 512, pfTemp + 768, pfTemp + 1024, pfTemp + 1280 };
	AkReal32 * pfEROut[ 4 ] = { NULL, NULL, NULL, NULL };
	if( m_Reverb.uNumERSignals )
	{
		pfEROut[ 0 ] = pfTemp + 1536;
		pfEROut[ 1 ] = pfTemp + 1792;
		if( uNumBackChannels == 2 )
		{
			pfEROut[ 2 ] = pfTemp + 2048;
			pfEROut[ 3 ] = pfTemp + 2304;
		}
	}
	AkReal32 fPrevGain1, fPrevGain2, fGain1, fGain2;
	AkStereoWidthGains( m_PrevRTPCParams.fStereoWidth, fPrevGain1, fPrevGain2 );
	AkStereoWidthGains( m_pParams->sRTPCParams.fStereoWidth, fGain1, fGain2 );
	if( !( uChannelMask & ( AK_SPEAKER_BACK_LEFT | AK_SPEAKER_BACK_RIGHT ) ) )
	{
		m_pParams->sRTPCParams.fFrontLevel = 1.f;
		m_pParams->sRTPCParams.fRearLevel = 1.f;
	}
	AkUInt32 uFrameOffset = 0;
	AkUInt32 uNumFramesRemaining = io_pBuffer->uValidFrames;
	while( uNumFramesRemaining )
	{
		const AkUInt32 uFrames = uNumFramesRemaining >= 256 ? 256 : uNumFramesRemaining;
		ProcessFront( io_pBuffer, pfTemp, pfEROut, uNumBackChannels == 2, pfReverbOut, 4, uFrames, uFrameOffset );
		const RTPCParams & rNew = m_pParams->sRTPCParams;
		const RTPCParams & rPrev = m_PrevRTPCParams;
		AkUInt32 uChannel = 0;
		if( uChannelMask & AK_SPEAKER_FRONT_LEFT )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fFrontLevel;
			const AkReal32 fCurrent = rPrev.fFrontLevel * rPrev.fReverbLevel;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 0 ], pfReverbOut[ 1 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent * fPrevGain1, fTarget * fGain1, fCurrent * fPrevGain2, fTarget * fGain2, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				const AkReal32 fERTarget = rNew.fERLevel * rNew.fFrontLevel;
				const AkReal32 fERCurrent = rPrev.fFrontLevel * rPrev.fERLevel;
				DSP::Mix3Interp( pfOut, pfEROut[ 0 ], pfEROut[ 1 ], 1.f, 1.f, fERCurrent * fPrevGain1, fERTarget * fGain1, fERCurrent * fPrevGain2, fERTarget * fGain2, uFrames );
			}
			uChannel = 1;
		}
		if( uChannelMask & AK_SPEAKER_FRONT_RIGHT )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset + uChannel * io_pBuffer->uMaxFrames;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fFrontLevel;
			const AkReal32 fCurrent = rPrev.fFrontLevel * rPrev.fReverbLevel;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 0 ], pfReverbOut[ 1 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent * fPrevGain2, fTarget * fGain2, fCurrent * fPrevGain1, fTarget * fGain1, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				const AkReal32 fERTarget = rNew.fERLevel * rNew.fFrontLevel;
				const AkReal32 fERCurrent = rPrev.fFrontLevel * rPrev.fERLevel;
				DSP::Mix3Interp( pfOut, pfEROut[ 0 ], pfEROut[ 1 ], 1.f, 1.f, fERCurrent * fPrevGain2, fERTarget * fGain2, fERCurrent * fPrevGain1, fERTarget * fGain1, uFrames );
			}
			++uChannel;
		}
		if( uChannelMask & AK_SPEAKER_FRONT_CENTER )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset + uChannel * io_pBuffer->uMaxFrames;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fCenterLevel * 0.70710599f;
			const AkReal32 fCurrent = 0.70710599f * ( rPrev.fReverbLevel * rPrev.fCenterLevel );
			DSP::Mix3Interp( pfOut, pfReverbOut[ 2 ], pfReverbOut[ 3 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent, fTarget, fCurrent, fTarget, uFrames );
			++uChannel;
		}
		if( uChannelMask & AK_SPEAKER_BACK_LEFT )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset + uChannel * io_pBuffer->uMaxFrames;
			const AkReal32 fCurrent = rPrev.fRearLevel * rPrev.fReverbLevel;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fRearLevel;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 2 ], pfReverbOut[ 3 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent * fPrevGain1, fTarget * fGain1, fCurrent * fPrevGain2, fTarget * fGain2, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				const AkReal32 fERTarget = rNew.fERLevel * rNew.fRearLevel;
				const AkReal32 fERCurrent = rPrev.fRearLevel * rPrev.fERLevel;
				DSP::Mix3Interp( pfOut, pfEROut[ 2 ], pfEROut[ 3 ], 1.f, 1.f, fERCurrent * fPrevGain1, fERTarget * fGain1, fERCurrent * fPrevGain2, fERTarget * fGain2, uFrames );
			}
			++uChannel;
		}
		if( uChannelMask & AK_SPEAKER_BACK_RIGHT )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset + uChannel * io_pBuffer->uMaxFrames;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fRearLevel;
			const AkReal32 fCurrent = rPrev.fRearLevel * rPrev.fReverbLevel;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 2 ], pfReverbOut[ 3 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent * fPrevGain2, fTarget * fGain2, fCurrent * fPrevGain1, fTarget * fGain1, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				const AkReal32 fERTarget = rNew.fERLevel * rNew.fRearLevel;
				const AkReal32 fERCurrent = rPrev.fRearLevel * rPrev.fERLevel;
				DSP::Mix3Interp( pfOut, pfEROut[ 2 ], pfEROut[ 3 ], 1.f, 1.f, fERCurrent * fPrevGain2, fERTarget * fGain2, fERCurrent * fPrevGain1, fERTarget * fGain1, uFrames );
			}
			++uChannel;
		}
		if( uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
		{
			AkReal32 * pfOut = (AkReal32 *)io_pBuffer->pData + uFrameOffset + uChannel * io_pBuffer->uMaxFrames;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fLFELevel * 0.70710599f;
			const AkReal32 fCurrent = 0.70710599f * ( rPrev.fReverbLevel * rPrev.fLFELevel );
			DSP::Mix3Interp( pfOut, pfReverbOut[ 2 ], pfReverbOut[ 3 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent, fTarget, fCurrent, fTarget, uFrames );
		}
		uFrameOffset += uFrames;
		uNumFramesRemaining -= uFrames;
		m_PrevRTPCParams.fDryLevel = m_pParams->sRTPCParams.fDryLevel;
		m_PrevRTPCParams.fERLevel = m_pParams->sRTPCParams.fERLevel;
		fPrevGain1 = fGain1;
		m_PrevRTPCParams.fReverbLevel = m_pParams->sRTPCParams.fReverbLevel;
		fPrevGain2 = fGain2;
		m_PrevRTPCParams.fFrontLevel = m_pParams->sRTPCParams.fFrontLevel;
		m_PrevRTPCParams.fCenterLevel = m_pParams->sRTPCParams.fCenterLevel;
		m_PrevRTPCParams.fRearLevel = m_pParams->sRTPCParams.fRearLevel;
		m_PrevRTPCParams.fLFELevel = m_pParams->sRTPCParams.fLFELevel;
	}
	AK_PLUGIN_FREE( m_pAllocator, pfTemp );
	return AK_Success;
}

/** 2012 0x98aab0: 5.0 / 5.1 layouts: fronts from outputs 0/1, center and LFE from 2/3, backs from 4/5. */
AKRESULT CAkRoomVerbFX::ProcessSpread3Out( AkAudioBuffer * io_pBuffer )
{
	const AkUInt32 uChannelMask = io_pBuffer->uChannelMask;
	AkReal32 * pfTemp = (AkReal32 *)AK_PLUGIN_ALLOC( m_pAllocator, ( 2 * m_Reverb.uNumERSignals + 8 ) << 10 );
	if( !pfTemp )
	{
		return AK_InsufficientMemory;
	}
	AkReal32 * pfReverbOut[ 6 ] = { pfTemp + 512, pfTemp + 768, pfTemp + 1024, pfTemp + 1280, pfTemp + 1536, pfTemp + 1792 };
	AkReal32 * pfEROut[ 4 ] = { NULL, NULL, NULL, NULL };
	if( m_Reverb.uNumERSignals )
	{
		pfEROut[ 0 ] = pfTemp + 2048;
		pfEROut[ 1 ] = pfTemp + 2304;
		pfEROut[ 2 ] = pfTemp + 2560;
		pfEROut[ 3 ] = pfTemp + 2816;
	}
	AkReal32 fPrevGain1, fPrevGain2, fGain1, fGain2;
	AkStereoWidthGains( m_PrevRTPCParams.fStereoWidth, fPrevGain1, fPrevGain2 );
	AkStereoWidthGains( m_pParams->sRTPCParams.fStereoWidth, fGain1, fGain2 );
	AkUInt32 uFrameOffset = 0;
	AkUInt32 uNumFramesRemaining = io_pBuffer->uValidFrames;
	while( uNumFramesRemaining )
	{
		const AkUInt32 uFrames = uNumFramesRemaining >= 256 ? 256 : uNumFramesRemaining;
		ProcessFront( io_pBuffer, pfTemp, pfEROut, true, pfReverbOut, 6, uFrames, uFrameOffset );
		const RTPCParams & rNew = m_pParams->sRTPCParams;
		const RTPCParams & rPrev = m_PrevRTPCParams;
		const AkUInt32 uMaxFrames = io_pBuffer->uMaxFrames;
		AkReal32 * pfData = (AkReal32 *)io_pBuffer->pData + uFrameOffset;
		{
			const AkReal32 fCurrent = rPrev.fFrontLevel * rPrev.fReverbLevel;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fFrontLevel;
			DSP::Mix3Interp( pfData, pfReverbOut[ 0 ], pfReverbOut[ 1 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent * fPrevGain1, fTarget * fGain1, fCurrent * fPrevGain2, fTarget * fGain2, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				const AkReal32 fERTarget = rNew.fERLevel * rNew.fFrontLevel;
				const AkReal32 fERCurrent = rPrev.fFrontLevel * rPrev.fERLevel;
				DSP::Mix3Interp( pfData, pfEROut[ 0 ], pfEROut[ 1 ], 1.f, 1.f, fERCurrent * fPrevGain1, fERTarget * fGain1, fERCurrent * fPrevGain2, fERTarget * fGain2, uFrames );
			}
		}
		{
			AkReal32 * pfOut = pfData + uMaxFrames;
			const AkReal32 fCurrent = rPrev.fFrontLevel * rPrev.fReverbLevel;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fFrontLevel;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 0 ], pfReverbOut[ 1 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent * fPrevGain2, fTarget * fGain2, fCurrent * fPrevGain1, fTarget * fGain1, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				const AkReal32 fERTarget = rNew.fERLevel * rNew.fFrontLevel;
				const AkReal32 fERCurrent = rPrev.fFrontLevel * rPrev.fERLevel;
				DSP::Mix3Interp( pfOut, pfEROut[ 0 ], pfEROut[ 1 ], 1.f, 1.f, fERCurrent * fPrevGain2, fERTarget * fGain2, fERCurrent * fPrevGain1, fERTarget * fGain1, uFrames );
			}
		}
		AkUInt32 uChannel = 2;
		if( uChannelMask & AK_SPEAKER_FRONT_CENTER )
		{
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fCenterLevel * 0.70710599f;
			const AkReal32 fCurrent = 0.70710599f * ( rPrev.fReverbLevel * rPrev.fCenterLevel );
			DSP::Mix3Interp( pfData + 2 * uMaxFrames, pfReverbOut[ 2 ], pfReverbOut[ 3 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent, fTarget, fCurrent, fTarget, uFrames );
			uChannel = 3;
		}
		{
			AkReal32 * pfOut = pfData + uChannel * uMaxFrames;
			const AkReal32 fCurrent = rPrev.fReverbLevel * rPrev.fRearLevel;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fRearLevel;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 4 ], pfReverbOut[ 5 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent * fPrevGain1, fTarget * fGain1, fCurrent * fPrevGain2, fTarget * fGain2, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				const AkReal32 fERTarget = rNew.fERLevel * rNew.fRearLevel;
				const AkReal32 fERCurrent = rPrev.fERLevel * rPrev.fRearLevel;
				DSP::Mix3Interp( pfOut, pfEROut[ 2 ], pfEROut[ 3 ], 1.f, 1.f, fERCurrent * fPrevGain1, fERTarget * fGain1, fERCurrent * fPrevGain2, fERTarget * fGain2, uFrames );
			}
		}
		{
			AkReal32 * pfOut = pfData + ( uChannel + 1 ) * uMaxFrames;
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fRearLevel;
			const AkReal32 fCurrent = rPrev.fReverbLevel * rPrev.fRearLevel;
			DSP::Mix3Interp( pfOut, pfReverbOut[ 4 ], pfReverbOut[ 5 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent * fPrevGain2, fTarget * fGain2, fCurrent * fPrevGain1, fTarget * fGain1, uFrames );
			if( m_Reverb.uNumERSignals )
			{
				const AkReal32 fERTarget = rNew.fERLevel * rNew.fRearLevel;
				const AkReal32 fERCurrent = rPrev.fERLevel * rPrev.fRearLevel;
				DSP::Mix3Interp( pfOut, pfEROut[ 2 ], pfEROut[ 3 ], 1.f, 1.f, fERCurrent * fPrevGain2, fERTarget * fGain2, fERCurrent * fPrevGain1, fERTarget * fGain1, uFrames );
			}
		}
		if( uChannelMask & AK_SPEAKER_LOW_FREQUENCY )
		{
			const AkReal32 fTarget = rNew.fReverbLevel * rNew.fLFELevel * 0.70710599f;
			const AkReal32 fCurrent = 0.70710599f * ( rPrev.fReverbLevel * rPrev.fLFELevel );
			DSP::Mix3Interp( pfData + ( uChannel + 2 ) * uMaxFrames, pfReverbOut[ 2 ], pfReverbOut[ 3 ], rPrev.fDryLevel, rNew.fDryLevel, fCurrent, fTarget, fCurrent, fTarget, uFrames );
		}
		uFrameOffset += uFrames;
		uNumFramesRemaining -= uFrames;
		m_PrevRTPCParams.fDryLevel = m_pParams->sRTPCParams.fDryLevel;
		m_PrevRTPCParams.fERLevel = m_pParams->sRTPCParams.fERLevel;
		fPrevGain1 = fGain1;
		m_PrevRTPCParams.fReverbLevel = m_pParams->sRTPCParams.fReverbLevel;
		fPrevGain2 = fGain2;
		m_PrevRTPCParams.fFrontLevel = m_pParams->sRTPCParams.fFrontLevel;
		m_PrevRTPCParams.fCenterLevel = m_pParams->sRTPCParams.fCenterLevel;
		m_PrevRTPCParams.fRearLevel = m_pParams->sRTPCParams.fRearLevel;
		m_PrevRTPCParams.fLFELevel = m_pParams->sRTPCParams.fLFELevel;
	}
	AK_PLUGIN_FREE( m_pAllocator, pfTemp );
	return AK_Success;
}

/** 2012 0x98ba20: parameter updates, the tail, then the layout's mix; the RTPC set is remembered for the next ramps. */
void CAkRoomVerbFX::Execute( AkAudioBuffer * io_pBuffer )
{
	if( m_pParams->sInvariantParams.bDirty && LiveParametersUpdate( io_pBuffer ) )
	{
		return;
	}
	if( m_pParams->sRTPCParams.bDirty )
	{
		RTPCParametersUpdate();
	}
	m_Reverb.FXTailHandler.HandleTail( io_pBuffer, m_Reverb.uTailLength );
	if( !io_pBuffer->uValidFrames )
	{
		return;
	}
	switch( io_pBuffer->uChannelMask )
	{
	case 0x3:
	case 0x4:
	case 0x8:
	case 0xB:
	case 0xC:
		ProcessSpread1Out( io_pBuffer );
		break;
	case 0x7:
	case 0xF:
	case 0x33:
		ProcessSpread2Out( io_pBuffer );
		break;
	case 0x37:
	case 0x3B:
	case 0x3F:
		ProcessSpread3Out( io_pBuffer );
		break;
	default:
		break;
	}
	m_PrevRTPCParams = m_pParams->sRTPCParams;
}
