/*=============================================================================
	AkFDNReverbFX.cpp - CAkFDNReverbFX, the Matrix Reverb, Wwise 2012.1 (plugins/effects/akfdnreverb/win32/akfdnreverbfx.cpp).

	Agent IH4 (fork F2): ported from the 2012 decompile, 2013 twins matched by bytes. The network: the input (the sum of
	the processed channels) goes through a DC blocker (10 Hz), the optional pre-delay and a one-zero high-frequency
	shelf (the HF ratio), then into N = 4/8/12/16 delay lines (prime lengths, sorted); each line's output passes its own
	one-pole low-pass (gain for the reverb time, damping for the HF ratio); the feedback matrix is a Householder
	reflection (x - 2/N sum(x)) followed by a rotation of the N lines by one; each output channel is a +-1 decorrelation
	vector product of the line outputs, mixed wet with its dry input.

	Named deviation: retail has 20 hand-unrolled routines (Mono/Stereo/5.0/5.1/N x 4/8/12/16 delays); here each of the
	20 named routines instantiates one template (ProcessFDN<groups, configuration>) that performs the same SSE operations
	in the same order: per-group IIR, per-channel products accumulated group 0 first, horizontal sums as
	(a3 + a1) + (a2 + a0), group sums for the Householder term accumulated in group order, the input sums of each
	configuration ((C + SL) + ((L + R) + SR) for 5.0, ((C + LFE) + SR) + ((L + R) + SL) for 5.1, left to right for
	N) and the per-channel, per-group decorrelation vectors read out of the twenty routines (vMinusOne products where
	retail negates). The function pointer is a member pointer. Unaligned SSE loads are used for the interleaved delay
	reads (retail loads scalars and interleaves them, the same values).
=============================================================================*/
#include "AkFDNReverbFX.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/** 2012 .rdata 0xda2790: the default delay times (ms) for delay lengths mode 0. */
static const AkReal32 g_fDefaultDelayLengths[ 16 ] =
{
	13.62f, 15.66f, 17.52f, 19.02f, 20.83f, 22.6f, 24.05f, 24.78f, 25.6f, 26.09f, 26.55f, 26.91f, 28.04f, 29.09f, 29.9f, 30.86f
};

/** 2012 .rdata 0xda27d0. */
static const AkReal32 DCFILTERCUTOFFFREQ = 10.f;

/** 2012 0xbab230 (dynamic initializer of IIRCOEFCALCCONST, 2013 0xb8f7b0): ln(10) / 4. */
static const double IIRCOEFCALCCONST = log( 10.0 ) * 0.25;

static const __m128 vMinusOne = { -1.f, -1.f, -1.f, -1.f };
static const __m128 vOutDecorrelationVectorA = { 1.f, -1.f, 1.f, -1.f };
static const __m128 vOutDecorrelationVectorB = { 1.f, 1.f, -1.f, -1.f };
static const __m128 vOutDecorrelationVectorC = { -1.f, 1.f, 1.f, -1.f };
static const __m128 vOutDecorrelationVectorD = { -1.f, -1.f, -1.f, 1.f };
static const __m128 vOutDecorrelationVectorE = { 1.f, 1.f, -1.f, 1.f };
static const __m128 vOutDecorrelationVectorF = { 1.f, -1.f, -1.f, -1.f };

/** 2012 .rdata 0xda2850: the generic routine's vectors, [channel][group]. */
static const __m128 vOutDecorrelationVector[ 6 ][ 4 ] =
{
	{ { 1.f, -1.f, 1.f, -1.f }, { 1.f, -1.f, 1.f, -1.f }, { 1.f, -1.f, 1.f, -1.f }, { 1.f, -1.f, 1.f, -1.f } },
	{ { 1.f, 1.f, -1.f, -1.f }, { 1.f, 1.f, -1.f, -1.f }, { 1.f, 1.f, -1.f, -1.f }, { 1.f, 1.f, -1.f, -1.f } },
	{ { -1.f, 1.f, 1.f, -1.f }, { -1.f, 1.f, 1.f, -1.f }, { -1.f, 1.f, 1.f, -1.f }, { -1.f, 1.f, 1.f, -1.f } },
	{ { 1.f, 1.f, -1.f, -1.f }, { -1.f, -1.f, 1.f, 1.f }, { 1.f, 1.f, -1.f, -1.f }, { -1.f, -1.f, 1.f, 1.f } },
	{ { -1.f, -1.f, -1.f, 1.f }, { 1.f, 1.f, -1.f, 1.f }, { -1.f, -1.f, 1.f, 1.f }, { 1.f, 1.f, -1.f, -1.f } },
	{ { 1.f, -1.f, -1.f, -1.f }, { 1.f, 1.f, -1.f, 1.f }, { 1.f, -1.f, -1.f, 1.f }, { -1.f, 1.f, -1.f, 1.f } }
};

namespace
{
	enum FDNConfig
	{
		FDN_MONO = 0,
		FDN_STEREO,
		FDN_FIVEPOINTZERO,
		FDN_FIVEPOINTONE,
		FDN_N
	};

	/** The specialised routines' vector per output channel (deinterleaved order L, R, C, SL, SR, LFE) and group:
	    0..5 = A..F, +8 = negated (retail's vMinusOne product). */
	const int k_SpecialVectors[ 6 ][ 4 ] =
	{
		{ 0, 0, 0, 0 },			// L:   A  A  A  A
		{ 1, 1, 1, 1 },			// R:   B  B  B  B
		{ 2, 2, 2, 2 },			// C:   C  C  C  C
		{ 3, 4, 9, 1 },			// SL:  D  E -B  B
		{ 5, 11, 10, 8 },		// SR:  F -D -C -A
		{ 1, 9, 1, 9 }			// LFE: B -B  B -B (5.1)
	};

	inline __m128 SpecialVector( int in_iCode )
	{
		static const __m128 * const s_Vectors[ 6 ] = { &vOutDecorrelationVectorA, &vOutDecorrelationVectorB, &vOutDecorrelationVectorC, &vOutDecorrelationVectorD, &vOutDecorrelationVectorE, &vOutDecorrelationVectorF };
		const __m128 v = *s_Vectors[ in_iCode & 7 ];
		return ( in_iCode & 8 ) ? _mm_mul_ps( vMinusOne, v ) : v;
	}

	inline AkReal32 HorizontalSum( __m128 in_v )
	{
		const __m128 vHalf = _mm_add_ps( _mm_movehl_ps( in_v, in_v ), in_v );
		return _mm_cvtss_f32( _mm_shuffle_ps( vHalf, vHalf, 85 ) ) + _mm_cvtss_f32( vHalf );
	}
}

/** 2012 0x9881c0 (2013 twin not matched). */
AK::IAkPlugin * CreateMatrixReverbFX( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkFDNReverbFX() );
}

/** 2012 0x987ea0, 2013 0x9849c0. */
CAkFDNReverbFX::CAkFDNReverbFX()
	: m_fpPerformDSP( NULL )
	, m_pParams( NULL )
	, m_pAllocator( NULL )
	, m_fCachedReverbTime( 0.f )
	, m_fCachedHFRatio( 0.f )
	, m_pfPreDelayStart( NULL )
	, m_pfPreDelayRW( NULL )
	, m_pfPreDelayEnd( NULL )
{
	m_FXTailHandler.uTailFramesRemaining = (AkUInt32)-1;
	m_FXTailHandler.uTotalTailFrames = 0;
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		m_pfDelayStart[ i ] = NULL;
		m_pfDelayEnd[ i ] = NULL;
		m_pfDelayWrite[ i ] = NULL;
	}
	memset( m_pfDelayRead, 0, sizeof( m_pfDelayRead ) );
	memset( m_uNominalDelayLength, 0, sizeof( m_uNominalDelayLength ) );
	memset( m_vIIRLPFB0, 0, sizeof( m_vIIRLPFB0 ) );
	memset( m_vIIRLPFA1, 0, sizeof( m_vIIRLPFA1 ) );
	memset( m_vIIRLPFMem, 0, sizeof( m_vIIRLPFMem ) );
}

/** 2012 0x97cb60, 2013 0x979360. */
CAkFDNReverbFX::~CAkFDNReverbFX()
{
}

/** 2012 0x987f80, 2013 0x984780. */
AKRESULT CAkFDNReverbFX::Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkEffectPluginContext * in_pFXCtx, AK::IAkPluginParam * in_pParams, AkAudioFormat & in_rFormat )
{
	m_uSampleRate = in_rFormat.uSampleRate;
	m_bIsSentMode = in_pFXCtx->IsSendModeEffect();
	m_pAllocator = in_pAllocator;
	m_pParams = static_cast<CAkFDNReverbFXParams *>( in_pParams );
	m_fCurrentDry = m_pParams->RTPC.fDryLevel;
	m_fCurrentWet = m_pParams->RTPC.fWetLevel;
	const double dSampleRate = (double)m_uSampleRate;
	m_fDCCoef = (AkReal32)( 1.0 - 62.831856f / dSampleRate );
	m_uTailLength = (AkUInt32)(AkInt64)( dSampleRate * m_pParams->RTPC.fReverbTime );
	return InitDelayLines( in_rFormat.uChannelMask );
}

/** 2012 0x97cc90, 2013 0x979490. */
AKRESULT CAkFDNReverbFX::Term( AK::IAkPluginMemAlloc * in_pAllocator )
{
	TermDelayLines();
	AK_PLUGIN_DELETE( in_pAllocator, this );
	return AK_Success;
}

/** 2012 0x97cb70, 2013 0x979370. */
AKRESULT CAkFDNReverbFX::Reset()
{
	if( m_pfPreDelayStart )
	{
		memset( m_pfPreDelayStart, 0, sizeof( AkReal32 ) * m_uPreDelayLength );
	}
	m_fFIRLPFMem = 0.f;
	for( AkUInt32 i = 0; i < m_pParams->NonRTPC.uNumberOfDelays / 4; ++i )
	{
		m_vIIRLPFMem[ i ] = _mm_setzero_ps();
		if( m_pfDelayStart[ i ] )
		{
			memset( m_pfDelayStart[ i ], 0, sizeof( AkReal32 ) * 4 * m_uNominalDelayLength[ 4 * i + 3 ] );
		}
	}
	m_fDCFwdMem = 0.f;
	m_fDCFbkMem = 0.f;
	return AK_Success;
}

/** 2012 0x97cc20, 2013 0x979420. */
AKRESULT CAkFDNReverbFX::GetPluginInfo( AkPluginInfo & out_rPluginInfo )
{
	out_rPluginInfo.eType = AkPluginTypeEffect;
	out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bIsAsynchronous = false;
	return AK_Success;
}

/** 2012 0x98c570 (folded). */
AKRESULT CAkFDNReverbFX::TimeSkip( AkUInt32 /*in_uFrames*/ )
{
	return AK_DataReady;
}

/** 2012 0x97cc40, 2013 0x979440. */
void CAkFDNReverbFX::TermDelayLines()
{
	for( AkUInt32 i = 0; i < 4; ++i )
	{
		if( m_pfDelayStart[ i ] )
		{
			AK_PLUGIN_FREE( m_pAllocator, m_pfDelayStart[ i ] );
			m_pfDelayStart[ i ] = NULL;
		}
	}
	if( m_pfPreDelayStart )
	{
		AK_PLUGIN_FREE( m_pAllocator, m_pfPreDelayStart );
		m_pfPreDelayStart = NULL;
	}
}

/** 2012 0x97cb50, 2013 0x979350. */
static int __cdecl AkFDNQSortCompare( const void * a, const void * b )
{
	return (int)( *(const AkUInt32 *)a - *(const AkUInt32 *)b );
}

/** 2012 0x9882f0, 2013 0x984af0: the next odd number with no odd divisor below its square root + 1. */
void CAkFDNReverbFX::MakePrimeNumber( AkUInt32 & in_uIn )
{
	if( ( in_uIn & 1 ) == 0 )
	{
		++in_uIn;
	}
	const AkUInt32 uLimit = (AkUInt32)(int)sqrt( (double)in_uIn ) + 1;
	while( 1 )
	{
		AkUInt32 uDivisor = 3;
		if( uLimit <= 3 )
		{
			return;
		}
		while( in_uIn % uDivisor )
		{
			uDivisor += 2;
			if( uDivisor >= uLimit )
			{
				return;
			}
		}
		in_uIn += 2;
	}
}

/** 2012 0x9879d0, 2013 0x9841d0. */
AKRESULT CAkFDNReverbFX::InitDelayLines( AkChannelMask in_uChannelMask )
{
	TermDelayLines();
	if( !m_pParams->NonRTPC.uDelayLengthsMode )
	{
		for( AkUInt32 i = 0; i < m_pParams->NonRTPC.uNumberOfDelays; ++i )
		{
			m_pParams->NonRTPC.fDelayTime[ i ] = g_fDefaultDelayLengths[ i ];
		}
	}
	AkChannelMask uChannelMask = in_uChannelMask;
	if( !m_pParams->NonRTPC.uProcessLFE )
	{
		uChannelMask &= ~AK_SPEAKER_LOW_FREQUENCY;
	}
	m_uNumProcessedChannels = AkGetNumChannels( uChannelMask );
	const AkUInt32 uNumberOfDelays = m_pParams->NonRTPC.uNumberOfDelays;
	static const PerformDSPFunc s_Routines[ 5 ][ 4 ] =
	{
		{ &CAkFDNReverbFX::ProcessStereo4, &CAkFDNReverbFX::ProcessStereo8, &CAkFDNReverbFX::ProcessStereo12, &CAkFDNReverbFX::ProcessStereo16 },
		{ &CAkFDNReverbFX::ProcessMono4, &CAkFDNReverbFX::ProcessMono8, &CAkFDNReverbFX::ProcessMono12, &CAkFDNReverbFX::ProcessMono16 },
		{ &CAkFDNReverbFX::ProcessFivePointZero4, &CAkFDNReverbFX::ProcessFivePointZero8, &CAkFDNReverbFX::ProcessFivePointZero12, &CAkFDNReverbFX::ProcessFivePointZero16 },
		{ &CAkFDNReverbFX::ProcessFivePointOne4, &CAkFDNReverbFX::ProcessFivePointOne8, &CAkFDNReverbFX::ProcessFivePointOne12, &CAkFDNReverbFX::ProcessFivePointOne16 },
		{ &CAkFDNReverbFX::ProcessN4, &CAkFDNReverbFX::ProcessN8, &CAkFDNReverbFX::ProcessN12, &CAkFDNReverbFX::ProcessN16 }
	};
	int iRow;
	switch( uChannelMask )
	{
	case AK_SPEAKER_SETUP_STEREO:	iRow = 0; break;
	case AK_SPEAKER_FRONT_CENTER:	iRow = 1; break;
	case 0x37:						iRow = 2; break;
	case AK_SPEAKER_SETUP_5POINT1:	iRow = 3; break;
	default:						iRow = 4; break;
	}
	switch( uNumberOfDelays )
	{
	case 4:		m_fpPerformDSP = s_Routines[ iRow ][ 0 ]; break;
	case 8:		m_fpPerformDSP = s_Routines[ iRow ][ 1 ]; break;
	case 12:	m_fpPerformDSP = s_Routines[ iRow ][ 2 ]; break;
	case 16:	m_fpPerformDSP = s_Routines[ iRow ][ 3 ]; break;
	default:	break;
	}

	m_uPreDelayLength = (AkUInt32)(AkInt64)( (double)m_uSampleRate * m_pParams->NonRTPC.fPreDelay );
	if( m_uPreDelayLength )
	{
		m_pfPreDelayStart = (AkReal32 *)AK_PLUGIN_ALLOC( m_pAllocator, sizeof( AkReal32 ) * m_uPreDelayLength );
		if( !m_pfPreDelayStart )
		{
			return AK_InsufficientMemory;
		}
		m_pfPreDelayRW = m_pfPreDelayStart;
		m_pfPreDelayEnd = m_pfPreDelayStart + m_uPreDelayLength;
	}
	for( AkUInt32 i = 0; i < m_pParams->NonRTPC.uNumberOfDelays; ++i )
	{
		m_uNominalDelayLength[ i ] = (AkUInt32)(AkInt64)( m_pParams->NonRTPC.fDelayTime[ i ] * 0.001 * (double)m_uSampleRate );
		MakePrimeNumber( m_uNominalDelayLength[ i ] );
	}
	qsort( m_uNominalDelayLength, m_pParams->NonRTPC.uNumberOfDelays, sizeof( AkUInt32 ), AkFDNQSortCompare );
	for( AkUInt32 i = 0; i < m_pParams->NonRTPC.uNumberOfDelays / 4; ++i )
	{
		const AkUInt32 * puLengths = &m_uNominalDelayLength[ 4 * i ];
		const AkUInt32 uMaxLength = puLengths[ 3 ];
		AkReal32 * pfStart = (AkReal32 *)AK_PLUGIN_ALLOC( m_pAllocator, sizeof( AkReal32 ) * 4 * uMaxLength );
		m_pfDelayStart[ i ] = pfStart;
		if( !pfStart )
		{
			return AK_InsufficientMemory;
		}
		m_pfDelayWrite[ i ] = pfStart;
		m_pfDelayEnd[ i ] = pfStart + 4 * uMaxLength;
		m_pfDelayRead[ 4 * i + 0 ] = pfStart + 4 * ( uMaxLength - puLengths[ 0 ] );
		m_pfDelayRead[ 4 * i + 1 ] = pfStart + 4 * ( uMaxLength - puLengths[ 1 ] ) + 1;
		m_pfDelayRead[ 4 * i + 2 ] = pfStart + 4 * ( uMaxLength - puLengths[ 2 ] ) + 2;
		m_pfDelayRead[ 4 * i + 3 ] = pfStart + 3;
	}
	Reset();
	m_pParams->NonRTPC.bDirty = false;
	return AK_Success;
}

/** 2012 0x9881e0, 2013 0x9849e0: per line g = 10^(-3 L T / RT); the HF damping a1 is log10(g) ln(10)/4 (1 - 1/HF^-2),
	limited so the longest line stays stable; B0 = g (1 - a1). */
void CAkFDNReverbFX::ComputeIIRLPFCoefs()
{
	const AkReal32 fSamplingPeriod = (AkReal32)( 1.0 / (double)m_uSampleRate );
	const AkReal32 fInvHFRatio = 1.f / m_pParams->RTPC.fHFRatio;
	AkReal32 fReverbTimeRatioScale = 1.f - 1.f / ( fInvHFRatio * fInvHFRatio );
	const AkUInt32 uLongest = m_uNominalDelayLength[ m_pParams->NonRTPC.uNumberOfDelays - 1 ];
	const double dMaxCoef = log10( pow( 0.001, fSamplingPeriod * (double)uLongest * ( 1.f / m_pParams->RTPC.fReverbTime ) ) ) * IIRCOEFCALCCONST;
	if( fReverbTimeRatioScale * dMaxCoef > 1.0 )
	{
		fReverbTimeRatioScale = (AkReal32)( 1.0 / dMaxCoef );
	}
	AkReal32 * pfB0 = (AkReal32 *)m_vIIRLPFB0;
	AkReal32 * pfA1 = (AkReal32 *)m_vIIRLPFA1;
	for( AkUInt32 i = 0; i < m_pParams->NonRTPC.uNumberOfDelays; ++i )
	{
		const AkReal32 fGain = (AkReal32)pow( 0.001, ( 1.f / m_pParams->RTPC.fReverbTime ) * ( (double)m_uNominalDelayLength[ i ] * fSamplingPeriod ) );
		const AkReal32 fA1 = (AkReal32)( log10( (double)fGain ) * IIRCOEFCALCCONST * fReverbTimeRatioScale );
		pfB0[ i ] = fGain * ( 1.f - fA1 );
		pfA1[ i ] = fA1;
	}
}

/** 2012 0x988030, 2013 0x984830. */
void CAkFDNReverbFX::Execute( AkAudioBuffer * io_pBuffer )
{
	if( m_pParams->NonRTPC.bDirty )
	{
		if( InitDelayLines( io_pBuffer->uChannelMask ) != AK_Success )
		{
			return;
		}
		Reset();
	}
	if( m_bIsSentMode )
	{
		const AkChannelMask uChannelMask = io_pBuffer->uChannelMask;
		if( ( uChannelMask & AK_SPEAKER_LOW_FREQUENCY ) && !m_pParams->NonRTPC.uProcessLFE )
		{
			memset( io_pBuffer->GetChannel( AkGetNumChannels( uChannelMask ) - 1 ), 0, sizeof( AkReal32 ) * io_pBuffer->uValidFrames );
		}
	}
	if( !m_uNumProcessedChannels )
	{
		return;
	}
	if( m_pParams->RTPC.fReverbTime != m_fCachedReverbTime || m_pParams->RTPC.fHFRatio != m_fCachedHFRatio )
	{
		ComputeIIRLPFCoefs();
		const AkReal32 fInvHFRatio = 1.f / m_pParams->RTPC.fHFRatio;
		const AkReal32 fCoef = ( 1.f - fInvHFRatio ) / ( fInvHFRatio + 1.f );
		m_fFIRLPFB0 = 1.f / ( 1.f - fCoef );
		m_fFIRLPFB1 = fCoef * ( -1.f / ( 1.f - fCoef ) );
		m_fCachedReverbTime = m_pParams->RTPC.fReverbTime;
		m_fCachedHFRatio = m_pParams->RTPC.fHFRatio;
		m_uTailLength = (AkUInt32)(AkInt64)( (double)m_uSampleRate * m_pParams->RTPC.fReverbTime );
	}
	if( m_bIsSentMode )
	{
		m_fCurrentDry = 0.f;
		m_pParams->RTPC.fDryLevel = 0.f;
	}
	m_FXTailHandler.HandleTail( io_pBuffer, m_uTailLength );
	if( io_pBuffer->uValidFrames )
	{
		( this->*m_fpPerformDSP )( io_pBuffer );
		m_fCurrentDry = m_pParams->RTPC.fDryLevel;
		m_fCurrentWet = m_pParams->RTPC.fWetLevel;
	}
}

template < AkUInt32 NUMGROUPS, AkUInt32 CONFIG >
void CAkFDNReverbFX::ProcessFDN( AkAudioBuffer * io_pBuffer )
{
	const AkUInt32 uNumChannels = CONFIG == FDN_MONO ? 1 : CONFIG == FDN_STEREO ? 2 : CONFIG == FDN_FIVEPOINTZERO ? 5 : CONFIG == FDN_FIVEPOINTONE ? 6 : m_uNumProcessedChannels;
	const AkReal32 fFIRLPFB0 = m_fFIRLPFB0;
	const AkReal32 fFIRLPFB1 = m_fFIRLPFB1;
	const AkReal32 fDCCoef = m_fDCCoef;
	const AkReal32 fMaxFrames = (AkReal32)io_pBuffer->uMaxFrames;
	const AkReal32 fWetInc = ( m_pParams->RTPC.fWetLevel - m_fCurrentWet ) / fMaxFrames;
	const AkReal32 fDryInc = ( m_pParams->RTPC.fDryLevel - m_fCurrentDry ) / fMaxFrames;
	const __m128 vHouseholder = _mm_set1_ps( (AkReal32)( -2.0 / (double)m_pParams->NonRTPC.uNumberOfDelays ) );
	AkReal32 fFIRLPFMem = m_fFIRLPFMem;
	AkReal32 fDCFwdMem = m_fDCFwdMem;
	AkReal32 fDCFbkMem = m_fDCFbkMem;
	AkReal32 * pfPreDelayRW = m_pfPreDelayRW;
	AkReal32 fCurrentDry = m_fCurrentDry;
	AkReal32 fCurrentWet = m_fCurrentWet;

	__m128 vMem[ NUMGROUPS ];
	__m128 vB0[ NUMGROUPS ];
	__m128 vA1[ NUMGROUPS ];
	AkReal32 * pfRead[ NUMGROUPS * 4 ];
	AkReal32 * pfWrite[ NUMGROUPS ];
	for( AkUInt32 g = 0; g < NUMGROUPS; ++g )
	{
		vMem[ g ] = m_vIIRLPFMem[ g ];
		vB0[ g ] = m_vIIRLPFB0[ g ];
		vA1[ g ] = m_vIIRLPFA1[ g ];
		pfWrite[ g ] = m_pfDelayWrite[ g ];
		for( AkUInt32 j = 0; j < 4; ++j )
		{
			pfRead[ 4 * g + j ] = m_pfDelayRead[ 4 * g + j ];
		}
	}
	__m128 vVectors[ 6 ][ NUMGROUPS ];
	for( AkUInt32 c = 0; c < uNumChannels && c < 6; ++c )
	{
		for( AkUInt32 g = 0; g < NUMGROUPS; ++g )
		{
			vVectors[ c ][ g ] = CONFIG == FDN_N ? vOutDecorrelationVector[ c ][ g ] : SpecialVector( k_SpecialVectors[ c ][ g ] );
		}
	}

	AkReal32 * pData = (AkReal32 *)io_pBuffer->pData;
	const AkUInt32 uStride = io_pBuffer->uMaxFrames;
	for( AkUInt32 uFrame = io_pBuffer->uValidFrames; uFrame; --uFrame )
	{
		AkReal32 fLine[ NUMGROUPS * 4 ];
		for( AkUInt32 g = 0; g < NUMGROUPS; ++g )
		{
			for( AkUInt32 j = 0; j < 4; ++j )
			{
				AkReal32 *& rpfRead = pfRead[ 4 * g + j ];
				fLine[ 4 * g + j ] = *rpfRead;
				rpfRead += 4;
				if( rpfRead >= m_pfDelayEnd[ g ] )
				{
					rpfRead = m_pfDelayStart[ g ] + j;
				}
			}
		}
		fCurrentDry = fCurrentDry + fDryInc;
		fCurrentWet = fCurrentWet + fWetInc;
		for( AkUInt32 g = 0; g < NUMGROUPS; ++g )
		{
			vMem[ g ] = _mm_add_ps( _mm_mul_ps( _mm_loadu_ps( &fLine[ 4 * g ] ), vB0[ g ] ), _mm_mul_ps( vMem[ g ], vA1[ g ] ) );
		}

		AkReal32 fIn[ 6 ];
		for( AkUInt32 c = 0; c < uNumChannels && c < 6; ++c )
		{
			fIn[ c ] = pData[ c * uStride ];
		}
		for( AkUInt32 c = 0; c < uNumChannels && c < 6; ++c )
		{
			__m128 vAcc = _mm_mul_ps( vMem[ 0 ], vVectors[ c ][ 0 ] );
			for( AkUInt32 g = 1; g < NUMGROUPS; ++g )
			{
				vAcc = _mm_add_ps( _mm_mul_ps( vMem[ g ], vVectors[ c ][ g ] ), vAcc );
			}
			pData[ c * uStride ] = ( fIn[ c ] * fCurrentDry ) + ( HorizontalSum( vAcc ) * fCurrentWet );
		}

		__m128 vSum = vMem[ 0 ];
		for( AkUInt32 g = 1; g < NUMGROUPS; ++g )
		{
			vSum = _mm_add_ps( vMem[ g ], vSum );
		}
		const __m128 vReflect = _mm_mul_ps( _mm_set1_ps( HorizontalSum( vSum ) ), vHouseholder );
		__m128 vFeedback[ NUMGROUPS ];
		for( AkUInt32 g = 0; g < NUMGROUPS; ++g )
		{
			vFeedback[ g ] = _mm_add_ps( vReflect, vMem[ g ] );
		}

		AkReal32 fInput;
		switch( CONFIG )
		{
		case FDN_MONO:
			fInput = fIn[ 0 ];
			break;
		case FDN_STEREO:
			fInput = fIn[ 0 ] + fIn[ 1 ];
			break;
		case FDN_FIVEPOINTZERO:
			fInput = ( fIn[ 2 ] + fIn[ 3 ] ) + ( ( fIn[ 0 ] + fIn[ 1 ] ) + fIn[ 4 ] );
			break;
		case FDN_FIVEPOINTONE:
			fInput = ( ( fIn[ 2 ] + fIn[ 5 ] ) + fIn[ 4 ] ) + ( ( fIn[ 0 ] + fIn[ 1 ] ) + fIn[ 3 ] );
			break;
		default:
			fInput = fIn[ 0 ];
			for( AkUInt32 c = 1; c < uNumChannels && c < 6; ++c )
			{
				fInput = fInput + fIn[ c ];
			}
			break;
		}
		++pData;

		const AkReal32 fDCOut = ( ( fDCCoef * fDCFbkMem ) + fInput ) - fDCFwdMem;
		fDCFwdMem = fInput;
		fDCFbkMem = fDCOut;
		AkReal32 fPreDelayed;
		if( m_pfPreDelayStart )
		{
			fPreDelayed = *pfPreDelayRW;
			*pfPreDelayRW++ = fDCOut;
			if( pfPreDelayRW == m_pfPreDelayEnd )
			{
				pfPreDelayRW = m_pfPreDelayStart;
			}
		}
		else
		{
			fPreDelayed = fDCOut;
		}
		const AkReal32 fFIROut = ( fFIRLPFB0 * fPreDelayed ) + ( fFIRLPFB1 * fFIRLPFMem );
		fFIRLPFMem = fPreDelayed;
		const __m128 vFIROut = _mm_set1_ps( fFIROut );
		for( AkUInt32 g = 0; g < NUMGROUPS; ++g )
		{
			const __m128 vThis = vFeedback[ g ];
			const __m128 vNext = vFeedback[ ( g + 1 ) % NUMGROUPS ];
			const __m128 vRotated = _mm_shuffle_ps( vThis, _mm_shuffle_ps( vThis, vNext, 15 ), 201 );
			_mm_storeu_ps( pfWrite[ g ], _mm_add_ps( vFIROut, vRotated ) );
			pfWrite[ g ] += 4;
			if( pfWrite[ g ] >= m_pfDelayEnd[ g ] )
			{
				pfWrite[ g ] = m_pfDelayStart[ g ];
			}
		}
	}

	m_fFIRLPFMem = fFIRLPFMem;
	m_fDCFwdMem = fDCFwdMem;
	m_fDCFbkMem = fDCFbkMem;
	m_pfPreDelayRW = pfPreDelayRW;
	for( AkUInt32 g = 0; g < NUMGROUPS; ++g )
	{
		m_vIIRLPFMem[ g ] = vMem[ g ];
		m_pfDelayWrite[ g ] = pfWrite[ g ];
		for( AkUInt32 j = 0; j < 4; ++j )
		{
			m_pfDelayRead[ 4 * g + j ] = pfRead[ 4 * g + j ];
		}
	}
}

/** 2012 0x97ccc0, 2013 0x9794c0. */
void CAkFDNReverbFX::ProcessMono4( AkAudioBuffer * io_pBuffer ) { ProcessFDN<1, FDN_MONO>( io_pBuffer ); }
/** 2012 0x97d120, 2013 0x979920. */
void CAkFDNReverbFX::ProcessMono8( AkAudioBuffer * io_pBuffer ) { ProcessFDN<2, FDN_MONO>( io_pBuffer ); }
/** 2012 0x97d770, 2013 0x979f70. */
void CAkFDNReverbFX::ProcessMono12( AkAudioBuffer * io_pBuffer ) { ProcessFDN<3, FDN_MONO>( io_pBuffer ); }
/** 2012 0x97dfc0, 2013 0x97a7c0. */
void CAkFDNReverbFX::ProcessMono16( AkAudioBuffer * io_pBuffer ) { ProcessFDN<4, FDN_MONO>( io_pBuffer ); }
/** 2012 0x97ea20, 2013 0x97b220. */
void CAkFDNReverbFX::ProcessStereo4( AkAudioBuffer * io_pBuffer ) { ProcessFDN<1, FDN_STEREO>( io_pBuffer ); }
/** 2012 0x97ef10, 2013 0x97b710. */
void CAkFDNReverbFX::ProcessStereo8( AkAudioBuffer * io_pBuffer ) { ProcessFDN<2, FDN_STEREO>( io_pBuffer ); }
/** 2012 0x97f610, 2013 0x97be10. */
void CAkFDNReverbFX::ProcessStereo12( AkAudioBuffer * io_pBuffer ) { ProcessFDN<3, FDN_STEREO>( io_pBuffer ); }
/** 2012 0x97ff00, 2013 0x97c700. */
void CAkFDNReverbFX::ProcessStereo16( AkAudioBuffer * io_pBuffer ) { ProcessFDN<4, FDN_STEREO>( io_pBuffer ); }
/** 2012 0x980a30, 2013 0x97d230. */
void CAkFDNReverbFX::ProcessFivePointZero4( AkAudioBuffer * io_pBuffer ) { ProcessFDN<1, FDN_FIVEPOINTZERO>( io_pBuffer ); }
/** 2012 0x981050, 2013 0x97d850. */
void CAkFDNReverbFX::ProcessFivePointZero8( AkAudioBuffer * io_pBuffer ) { ProcessFDN<2, FDN_FIVEPOINTZERO>( io_pBuffer ); }
/** 2012 0x9818a0, 2013 0x97e0a0. */
void CAkFDNReverbFX::ProcessFivePointZero12( AkAudioBuffer * io_pBuffer ) { ProcessFDN<3, FDN_FIVEPOINTZERO>( io_pBuffer ); }
/** 2012 0x982360, 2013 0x97eb60. */
void CAkFDNReverbFX::ProcessFivePointZero16( AkAudioBuffer * io_pBuffer ) { ProcessFDN<4, FDN_FIVEPOINTZERO>( io_pBuffer ); }
/** 2012 0x983080, 2013 0x97f880. */
void CAkFDNReverbFX::ProcessFivePointOne4( AkAudioBuffer * io_pBuffer ) { ProcessFDN<1, FDN_FIVEPOINTONE>( io_pBuffer ); }
/** 2012 0x983700, 2013 0x97ff00. */
void CAkFDNReverbFX::ProcessFivePointOne8( AkAudioBuffer * io_pBuffer ) { ProcessFDN<2, FDN_FIVEPOINTONE>( io_pBuffer ); }
/** 2012 0x983fd0, 2013 0x9807d0. */
void CAkFDNReverbFX::ProcessFivePointOne12( AkAudioBuffer * io_pBuffer ) { ProcessFDN<3, FDN_FIVEPOINTONE>( io_pBuffer ); }
/** 2012 0x984b00, 2013 0x981300. */
void CAkFDNReverbFX::ProcessFivePointOne16( AkAudioBuffer * io_pBuffer ) { ProcessFDN<4, FDN_FIVEPOINTONE>( io_pBuffer ); }
/** 2012 0x9858c0, 2013 0x9820c0. */
void CAkFDNReverbFX::ProcessN4( AkAudioBuffer * io_pBuffer ) { ProcessFDN<1, FDN_N>( io_pBuffer ); }
/** 2012 0x985e20, 2013 0x982620. */
void CAkFDNReverbFX::ProcessN8( AkAudioBuffer * io_pBuffer ) { ProcessFDN<2, FDN_N>( io_pBuffer ); }
/** 2012 0x986550, 2013 0x982d50. */
void CAkFDNReverbFX::ProcessN12( AkAudioBuffer * io_pBuffer ) { ProcessFDN<3, FDN_N>( io_pBuffer ); }
/** 2012 0x986e80, 2013 0x983680. */
void CAkFDNReverbFX::ProcessN16( AkAudioBuffer * io_pBuffer ) { ProcessFDN<4, FDN_N>( io_pBuffer ); }
