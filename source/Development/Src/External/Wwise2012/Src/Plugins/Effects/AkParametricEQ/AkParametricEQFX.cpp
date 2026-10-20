/*=============================================================================
	AkParametricEQFX.cpp - CAkParametricEQFX (2012 0x97b0c0..0x97bb70), Process (akparametriceqfxcommon.h, 0x97b6e0),
	CAkParameterEQFXParams (0x97bbc0..0x97bf40); 2013 matched by bytes. Agent IH4 fork F1 (wave 26).

	Deviations (named):
	  * SetParamsBlock reads the packed 17-byte bands field by field (retail copies 20 bytes per band, the last three
	    being the next band's first bytes, then overwrites them).
	  * Process's 4x unroll is not reproduced (same arithmetic per sample, see OnePoleFilter.h).
=============================================================================*/
#include "AkParametricEQFX.h"
#include "AkApplyGain.h"
#include <math.h>
#include <string.h>

/** 2012 0x97bb70. */
AK::IAkPlugin * CreateParametricEQFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkParametricEQFX() );
}

/** 2012 0x97bf40. */
AK::IAkPluginParam * CreateParametricEQFXParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkParameterEQFXParams() );
}

/** 2012 0x97bf20. */
CAkParameterEQFXParams::CAkParameterEQFXParams()
{
	m_bBandDirty[ 0 ] = m_bBandDirty[ 1 ] = m_bBandDirty[ 2 ] = true;
}

/** 2012 0x97bdc0. */
CAkParameterEQFXParams::CAkParameterEQFXParams( const CAkParameterEQFXParams & in_rCopy )
{
	m_Params = in_rCopy.m_Params;
	m_bBandDirty[ 0 ] = m_bBandDirty[ 1 ] = m_bBandDirty[ 2 ] = true;
}

/** 2012 0x97bdf0. */
AK::IAkPluginParam * CAkParameterEQFXParams::Clone( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkParameterEQFXParams( *this ) );
}

/** 2012 0x97bbd0: the block, else low shelf 120 Hz / peaking 2 kHz / high shelf 5 kHz, flat, Q 5, all on, 0 dB, LFE processed. */
AKRESULT CAkParameterEQFXParams::Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_uBlockSize )
{
	if( in_uBlockSize )
	{
		return SetParamsBlock( in_pParamsBlock, in_uBlockSize );
	}
	const AkFilterType aType[ 3 ] = { AKFILTERTYPE_LOWSHELF, AKFILTERTYPE_PEAKINGEQ, AKFILTERTYPE_HISHELF };
	const AkReal32 aFreq[ 3 ] = { 120.f, 2000.f, 5000.f };
	for( AkUInt32 i = 0; i < 3; ++i )
	{
		m_Params.Band[ i ].eFilterType = aType[ i ];
		m_Params.Band[ i ].fGain = 0.f;
		m_Params.Band[ i ].fFrequency = aFreq[ i ];
		m_Params.Band[ i ].fQFactor = 5.f;
		m_Params.Band[ i ].bOnOff = true;
		m_bBandDirty[ i ] = true;
	}
	m_Params.fOutputLevel = 0.f;
	m_Params.bProcessLFE = true;
	return AK_Success;
}

/** 2012 0x97be30. */
AKRESULT CAkParameterEQFXParams::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x97be60: 3 x { type u32, gain, frequency, Q, on u8 } (17 bytes each), output level, process LFE u8. */
AKRESULT CAkParameterEQFXParams::SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_uBlockSize )
{
	const AkUInt8 * p = (const AkUInt8 *)in_pParamsBlock;
	for( AkUInt32 i = 0; i < 3; ++i )
	{
		const AkUInt8 * pBand = p + 17 * i;
		AkUInt32 uType;
		memcpy( &uType, pBand, 4 );
		m_Params.Band[ i ].eFilterType = (AkFilterType)uType;
		memcpy( &m_Params.Band[ i ].fGain, pBand + 4, 4 );
		memcpy( &m_Params.Band[ i ].fFrequency, pBand + 8, 4 );
		memcpy( &m_Params.Band[ i ].fQFactor, pBand + 12, 4 );
		m_Params.Band[ i ].bOnOff = pBand[ 16 ] != 0;
		m_bBandDirty[ i ] = true;
	}
	memcpy( &m_Params.fOutputLevel, p + 51, 4 );
	m_Params.bProcessLFE = p[ 55 ] != 0;
	return AK_Success;
}

/** 2012 0x97bc80: ids above 16 are refused; the type arrives as a float. */
AKRESULT CAkParameterEQFXParams::SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_uParamSize )
{
	if( !in_pValue || (AkUInt16)in_ParamID > 16 )
	{
		return AK_InvalidParameter;
	}
	const AkReal32 fValue = *(const AkReal32 *)in_pValue;
	const AkUInt32 uBand = in_ParamID / 5;
	switch( in_ParamID )
	{
	case 0: case 5: case 10:
		m_Params.Band[ uBand ].eFilterType = (AkFilterType)(AkInt32)fValue;
		m_bBandDirty[ uBand ] = true;
		break;
	case 1: case 6: case 11:
		m_Params.Band[ uBand ].fGain = fValue;
		m_bBandDirty[ uBand ] = true;
		break;
	case 2: case 7: case 12:
		m_Params.Band[ uBand ].fFrequency = fValue;
		m_bBandDirty[ uBand ] = true;
		break;
	case 3: case 8: case 13:
		m_Params.Band[ uBand ].fQFactor = fValue;
		m_bBandDirty[ uBand ] = true;
		break;
	case 4: case 9: case 14:
		m_Params.Band[ uBand ].bOnOff = fValue != 0.f;
		m_bBandDirty[ uBand ] = true;
		break;
	case 15:
		m_Params.fOutputLevel = fValue;
		break;
	case 16:
		m_Params.bProcessLFE = *(const AkUInt8 *)in_pValue != 0;
		break;
	}
	return AK_Success;
}

/** 2012 0x97b1c0. */
AKRESULT CAkParametricEQFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bIsAsynchronous = false;
	return AK_Success;
}

/** 2012 0x97b0d0: four memories per channel and band; the LFE is not filtered unless bProcessLFE. */
AKRESULT CAkParametricEQFX::Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat )
{
	const AkUInt32 uNumChannels = in_rFormat.GetNumChannels();
	m_uNumProcessedChannels = uNumChannels;
	m_uSampleRate = in_rFormat.uSampleRate;
	m_pSharedParams = static_cast<CAkParameterEQFXParams *>( in_pParams );
	if( ( in_rFormat.uChannelMask & AK_SPEAKER_LOW_FREQUENCY ) && !m_pSharedParams->m_Params.bProcessLFE )
	{
		m_uNumProcessedChannels = uNumChannels - 1;
	}
	if( m_uNumProcessedChannels )
	{
		m_pfAllocatedMem = (AkReal32 *)AK_PLUGIN_ALLOC( in_pAllocator, 48 * m_uNumProcessedChannels );
		if( !m_pfAllocatedMem )
		{
			return AK_InsufficientMemory;
		}
	}
	memset( m_pfFiltCoefs, 0, sizeof( m_pfFiltCoefs ) );
	m_pSharedParams->m_bBandDirty[ 0 ] = true;
	m_pSharedParams->m_bBandDirty[ 1 ] = true;
	m_pSharedParams->m_bBandDirty[ 2 ] = true;
	m_fCurrentGain = (AkReal32)pow( 10.0, m_pSharedParams->m_Params.fOutputLevel * 0.050000001f );
	return AK_Success;
}

/** 2012 0x97b9e0. */
AKRESULT CAkParametricEQFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	if( m_pfAllocatedMem )
	{
		AK_PLUGIN_FREE( in_pAllocator, m_pfAllocatedMem );
		m_pfAllocatedMem = NULL;
	}
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x97b190. */
AKRESULT CAkParametricEQFX::Reset()
{
	if( m_pfAllocatedMem )
	{
		memset( m_pfAllocatedMem, 0, 48 * m_uNumProcessedChannels );
	}
	return AK_Success;
}

/** 2012 0x97b1e0: the band's RBJ design (shelf slope 1, Butterworth low / high pass), frequency clamped to 0.45 fs, gain
    in dB; stored b0 b1 b2 -a1 -a2 normalised by a0. */
void CAkParametricEQFX::ComputeBiquadCoefs( AkBandNumber in_eBandNum, EQModuleParams * in_sModuleParams )
{
	const AkReal32 fSampleRate = (AkReal32)m_uSampleRate;
	AkReal32 fFreq = in_sModuleParams->fFrequency;
	if( fFreq >= fSampleRate * 0.44999999f )
	{
		fFreq = fSampleRate * 0.44999999f;
	}
	const AkReal32 fQ = in_sModuleParams->fQFactor;
	AkReal32 fB0, fB1, fB2, fA0, fA1, fA2;
	switch( in_sModuleParams->eFilterType )
	{
	case AKFILTERTYPE_LOWPASS:
	{
		const AkReal32 fK = (AkReal32)( 1.0 / tan( fFreq * 3.1415927 / fSampleRate ) );
		fA0 = 1.f;
		fB0 = 1.f / ( ( fK * fK + 1.f ) + fK * 1.4142135f );
		fB1 = fB0 * 2.f;
		fB2 = fB0;
		fA1 = ( 1.f - fK * fK ) * ( fB0 * 2.f );
		fA2 = ( ( fK * fK + 1.f ) - fK * 1.4142135f ) * fB0;
		break;
	}
	case AKFILTERTYPE_HIPASS:
	{
		const AkReal32 fK = (AkReal32)tan( fFreq * 3.1415927 / fSampleRate );
		fA0 = 1.f;
		fB0 = 1.f / ( ( fK * fK + 1.f ) + fK * 1.4142135f );
		fB1 = fB0 * -2.f;
		fB2 = fB0;
		fA1 = -( ( fK * fK - 1.f ) * ( fB0 * -2.f ) );
		fA2 = ( ( fK * fK + 1.f ) - fK * 1.4142135f ) * fB0;
		break;
	}
	case AKFILTERTYPE_BANDPASS:
	{
		const double dOmega = fFreq * 6.2831855 / fSampleRate;
		const AkReal32 fAlpha = (AkReal32)( sin( dOmega ) / ( fQ + fQ ) );
		fB0 = fAlpha;
		fB1 = 0.f;
		fB2 = -fAlpha;
		fA0 = fAlpha + 1.f;
		fA1 = (AkReal32)( cos( dOmega ) * -2.0 );
		fA2 = 1.f - fAlpha;
		break;
	}
	case AKFILTERTYPE_NOTCH:
	{
		const double dOmega = fFreq * 6.2831855 / fSampleRate;
		const AkReal32 fAlpha = (AkReal32)( sin( dOmega ) / ( fQ + fQ ) );
		fB0 = 1.f;
		fB2 = 1.f;
		fB1 = (AkReal32)( cos( dOmega ) * -2.0 );
		fA0 = fAlpha + 1.f;
		fA1 = fB1;
		fA2 = 1.f - fAlpha;
		break;
	}
	case AKFILTERTYPE_LOWSHELF:
	{
		const double dA = pow( 10.0, in_sModuleParams->fGain * 0.025 );
		const AkReal32 fA = (AkReal32)dA;
		const double dOmega = fFreq * 6.2831855 / fSampleRate;
		const AkReal32 fCos = (AkReal32)cos( dOmega );
		const AkReal32 fAm1Cos = ( fA - 1.f ) * fCos;
		const AkReal32 fAp1Cos = ( fA + 1.f ) * fCos;
		const AkReal32 fT = ( fA + 1.f ) - fAm1Cos;
		const AkReal32 fTwoSqrtAAlpha = (AkReal32)( sqrt( ( 1.0 / dA + dA ) * 0.0 + 2.0 ) * sin( dOmega ) * 0.5 * sqrt( dA ) * 2.0 );
		fA1 = ( fAp1Cos + ( fA - 1.f ) ) * -2.f;
		fB1 = ( ( ( fA - 1.f ) - fAp1Cos ) * fA ) * 2.f;
		fB0 = ( fT + fTwoSqrtAAlpha ) * fA;
		fB2 = ( fT - fTwoSqrtAAlpha ) * fA;
		fA0 = ( fAm1Cos + fTwoSqrtAAlpha ) + ( fA + 1.f );
		fA2 = ( fAm1Cos + ( fA + 1.f ) ) - fTwoSqrtAAlpha;
		break;
	}
	case AKFILTERTYPE_HISHELF:
	{
		const double dA = pow( 10.0, in_sModuleParams->fGain * 0.025 );
		const AkReal32 fA = (AkReal32)dA;
		const double dOmega = fFreq * 6.2831855 / fSampleRate;
		const AkReal32 fCos = (AkReal32)cos( dOmega );
		const AkReal32 fAm1Cos = ( fA - 1.f ) * fCos;
		const AkReal32 fAp1Cos = ( fA + 1.f ) * fCos;
		const AkReal32 fTwoSqrtAAlpha = (AkReal32)( sqrt( ( 1.0 / dA + dA ) * 0.0 + 2.0 ) * sin( dOmega ) * 0.5 * sqrt( dA ) * 2.0 );
		fA1 = ( ( fA - 1.f ) - fAp1Cos ) * 2.f;
		fB1 = ( ( fAp1Cos + ( fA - 1.f ) ) * fA ) * -2.f;
		fB0 = ( ( fAm1Cos + fTwoSqrtAAlpha ) + ( fA + 1.f ) ) * fA;
		fB2 = ( ( fAm1Cos + ( fA + 1.f ) ) - fTwoSqrtAAlpha ) * fA;
		fA0 = ( ( fA + 1.f ) - fAm1Cos ) + fTwoSqrtAAlpha;
		fA2 = ( ( fA + 1.f ) - fAm1Cos ) - fTwoSqrtAAlpha;
		break;
	}
	case AKFILTERTYPE_PEAKINGEQ:
	{
		const double dOmega = fFreq * 6.2831855 / fSampleRate;
		const AkReal32 fCos = (AkReal32)cos( dOmega );
		const AkReal32 fA = (AkReal32)pow( 10.0, in_sModuleParams->fGain * 0.025 );
		const AkReal32 fAlpha = (AkReal32)( sin( dOmega ) / ( fQ + fQ ) );
		fB1 = fCos * -2.f;
		fB2 = 1.f - fAlpha * fA;
		fB0 = fAlpha * fA + 1.f;
		fA0 = fAlpha / fA + 1.f;
		fA1 = fCos * -2.f;
		fA2 = 1.f - fAlpha / fA;
		break;
	}
	default:
		return;
	}
	const AkReal32 fNorm = 1.f / fA0;
	AkReal32 * pCoefs = m_pfFiltCoefs[ in_eBandNum ];
	pCoefs[ 1 ] = fNorm * fB1;
	pCoefs[ 0 ] = fNorm * fB0;
	pCoefs[ 2 ] = fNorm * fB2;
	pCoefs[ 3 ] = -( fNorm * fA1 );
	pCoefs[ 4 ] = -( fNorm * fA2 );
}

/** 2012 0x97b6e0 (akparametriceqfxcommon.h): one band on the first channels, memories x1 x2 y1 y2 per channel;
    y = a1 y1 + (a2 y2 + (b1 x1 + (b2 x2 + b0 x))). */
static void Process( const AkReal32 * in_pfCoefs, AkAudioBuffer * io_pBuffer, AkReal32 * io_pfMems, AkUInt32 in_uNumProcessedChannels )
{
	const AkReal32 fB0 = in_pfCoefs[ 0 ];
	const AkReal32 fB1 = in_pfCoefs[ 1 ];
	const AkReal32 fB2 = in_pfCoefs[ 2 ];
	const AkReal32 fA1 = in_pfCoefs[ 3 ];
	const AkReal32 fA2 = in_pfCoefs[ 4 ];
	for( AkUInt32 uChan = 0; uChan < in_uNumProcessedChannels; ++uChan )
	{
		AkReal32 * pfMems = io_pfMems + 4 * uChan;
		AkReal32 fX1 = pfMems[ 0 ];
		AkReal32 fX2 = pfMems[ 1 ];
		AkReal32 fY1 = pfMems[ 2 ];
		AkReal32 fY2 = pfMems[ 3 ];
		AkReal32 * pfBuf = io_pBuffer->GetChannel( uChan );
		for( AkUInt32 i = 0; i < io_pBuffer->uValidFrames; ++i )
		{
			const AkReal32 fIn = pfBuf[ i ];
			const AkReal32 fFwd = fX1 * fB1 + ( fX2 * fB2 + fIn * fB0 );
			const AkReal32 fOut = fY1 * fA1 + ( fY2 * fA2 + fFwd );
			fY2 = fY1;
			fY1 = fOut;
			fX2 = fX1;
			fX1 = fIn;
			pfBuf[ i ] = fOut;
		}
		pfMems[ 0 ] = fX1;
		pfMems[ 1 ] = fX2;
		pfMems[ 2 ] = fY1;
		pfMems[ 3 ] = fY2;
	}
}

/** 2012 0x97ba20: dirty bands are recomputed (the on/off read before), the bands run in order, then the output gain
    ramps to the level. */
void CAkParametricEQFX::Execute( AkAudioBuffer * io_pBuffer )
{
	if( !m_uNumProcessedChannels || !io_pBuffer->uValidFrames )
	{
		return;
	}
	bool abOn[ 3 ];
	for( AkUInt32 i = 0; i < 3; ++i )
	{
		abOn[ i ] = m_pSharedParams->m_Params.Band[ i ].bOnOff;
		if( m_pSharedParams->m_bBandDirty[ i ] )
		{
			ComputeBiquadCoefs( (AkBandNumber)i, &m_pSharedParams->m_Params.Band[ i ] );
			m_pSharedParams->m_bBandDirty[ i ] = false;
		}
	}
	const AkReal32 fTargetGain = (AkReal32)pow( 10.0, m_pSharedParams->m_Params.fOutputLevel * 0.050000001f );
	for( AkUInt32 i = 0; i < 3; ++i )
	{
		if( abOn[ i ] )
		{
			Process( m_pfFiltCoefs[ i ], io_pBuffer, &m_pfAllocatedMem[ 4 * i * m_uNumProcessedChannels ], m_uNumProcessedChannels );
		}
	}
	AK::DSP::ApplyGain( io_pBuffer, m_fCurrentGain, fTargetGain, m_pSharedParams->m_Params.bProcessLFE );
	m_fCurrentGain = fTargetGain;
}
