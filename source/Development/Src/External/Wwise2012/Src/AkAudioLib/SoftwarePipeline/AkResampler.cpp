/*=============================================================================
	AkResampler.cpp - port of akresampler.cpp (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26). PitchDSPFuncTable is retail's 2012 .data 0xe37048 (3 x 8 entries, read back
	from the database); InitDSPFunctTable swaps in the SSE2 bypass routines exactly as 0x942010. The x87 double
	arithmetic of SetPitch / TimeInputToOutput / TimeOutputToInput is kept in double.
=============================================================================*/
#include "AkResampler.h"
#include "AkRuntimeEnvironmentMgr.h"
#include <math.h>

PitchDSPFuncPtr PitchDSPFuncTable[ NumPitchOperatingMode ][ NUM_PITCH_DSP_FUNCTIONS ] =
{
	{ Bypass_I16_NChanVec, Bypass_I16_2Chan, Bypass_I16_NChanVec, Bypass_I16_NChanVec, Bypass_Native_NChan, Bypass_Native_NChan, Bypass_Native_NChan, Bypass_Native_NChan },
	{ Fixed_I16_1Chan, Fixed_I16_2Chan, Fixed_I16_NChan, Fixed_I16_NChan, Fixed_Native_1Chan, Fixed_Native_2Chan, Fixed_Native_NChan, Fixed_Native_NChan },
	{ Interpolating_I16_1Chan, Interpolating_I16_2Chan, Interpolating_I16_NChan, Interpolating_I16_NChan, Interpolating_Native_1Chan, Interpolating_Native_2Chan, Interpolating_Native_NChan, Interpolating_Native_NChan }
};

namespace
{
	inline bool IsNativeFloatIndex( AkUInt8 in_uIndex )
	{
		return in_uIndex == 4 || in_uIndex == 5 || in_uIndex == 6 || in_uIndex == 7;
	}

	inline AkUInt32 FrameSkipFromPitch( AkReal32 in_fPitchVal, AkReal32 in_fSampleRateConvertRatio )
	{
		return (AkUInt32)(AkInt64)( pow( 2.0, (AkReal64)( in_fPitchVal * 0.00083333335f ) ) * in_fSampleRateConvertRatio * 65536.0 + 0.5 );
	}
}

/** 2012 0x942010; 2013 0x93e950. */
void CAkResampler::InitDSPFunctTable()
{
	AK::AkRuntimeEnvironmentMgr * pEnv = AK::AkRuntimeEnvironmentMgr::Instance();
	if( pEnv->GetSIMDSupport( AK::AK_SIMD_SSE2 ) )
	{
		PitchDSPFuncTable[ PitchOperatingMode_Bypass ][ 0 ] = Bypass_I16_NChanVecSSE2;
		PitchDSPFuncTable[ PitchOperatingMode_Bypass ][ 1 ] = Bypass_I16_2ChanSSE2;
		PitchDSPFuncTable[ PitchOperatingMode_Bypass ][ 2 ] = Bypass_I16_NChanVecSSE2;
		PitchDSPFuncTable[ PitchOperatingMode_Bypass ][ 3 ] = Bypass_I16_NChanVecSSE2;
	}
}

/** 2012 0x942050; 2013 0x93e990. */
AkReal32 CAkResampler::GetLastRate()
{
	return (AkReal32)( (AkReal64)m_InternalPitchState.uCurrentFrameSkip * 0.000015258789 / m_fSampleRateConvertRatio );
}

/** 2012 0x942070; 2013 0x93e9b0. */
void CAkResampler::SetLastValues( AkReal32 * in_pfLastValues )
{
	if( m_DSPFunctionIndex < 4 )
	{
		for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
		{
			AkReal32 fValue = in_pfLastValues[ i ] * 32767.f;
			if( fValue < 32767.f && fValue <= -32768.f )
			{
				fValue = -32768.f;
			}
			else if( fValue >= 32767.f )
			{
				fValue = 32767.f;
			}
			m_InternalPitchState.iLastValue[ i ] = (AkInt16)(AkInt32)fValue;
		}
	}
	else if( IsNativeFloatIndex( m_DSPFunctionIndex ) )
	{
		for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
		{
			m_InternalPitchState.fLastValue[ i ] = in_pfLastValues[ i ];
		}
	}
}

/** 2012 0x942120; 2013 0x93ea60. */
void CAkResampler::GetLastValues( AkReal32 * out_pfLastValues )
{
	if( m_DSPFunctionIndex < 4 )
	{
		for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
		{
			out_pfLastValues[ i ] = (AkReal32)m_InternalPitchState.iLastValue[ i ] * 0.000030517578f;
		}
	}
	else if( IsNativeFloatIndex( m_DSPFunctionIndex ) )
	{
		for( AkUInt32 i = 0; i < m_uNumChannels; ++i )
		{
			out_pfLastValues[ i ] = m_InternalPitchState.fLastValue[ i ];
		}
	}
}

/** 2012 0x9421a0; 2013 0x93eae0. */
AkUInt8 CAkResampler::GetDSPFunctionIndex( const AkAudioFormat & in_fmt ) const
{
	AkUInt8 uBase;
	if( in_fmt.uBitsPerSample == 16 )
	{
		uBase = 0;
	}
	else if( in_fmt.uBitsPerSample == 32 )
	{
		uBase = 4;
	}
	else
	{
		return (AkUInt8)-1;
	}
	switch( m_uNumChannels )
	{
	case 1:
		return uBase;
	case 2:
		return uBase + 1;
	case 3:
	case 4:
		return uBase + 2;
	case 5:
	case 6:
		return uBase + 3;
	default:
		return (AkUInt8)-1;
	}
}

/** 2012 0x942240; 2013 0x93eb80. */
void CAkResampler::Init( AkAudioFormat * io_pFormat, AkUInt32 in_uSampleRate )
{
	m_InternalPitchState.uOutFrameOffset = 0;
	m_InternalPitchState.uInFrameOffset = 0;
	m_InternalPitchState.uFloatIndex = FPMUL;
	m_InternalPitchState.uCurrentFrameSkip = 0;
	m_InternalPitchState.uTargetFrameSkip = 0;
	m_InternalPitchState.uInterpolationRampCount = 0;
	m_fTargetPitchVal = 0.f;
	m_bFirstSetPitch = true;
	m_InternalPitchState.uInterpolationRampInc = AK_CORE_SAMPLERATE / in_uSampleRate;
	m_fSampleRateConvertRatio = (AkReal32)( (AkReal64)io_pFormat->uSampleRate / (AkReal64)in_uSampleRate );
	m_uNumChannels = (AkUInt8)AkGetNumChannels( io_pFormat->uChannelMask );
	m_uInputBlockAlign = (AkUInt8)io_pFormat->uBlockAlign;
	m_PitchOperationMode = PitchOperatingMode_Bypass;
	m_DSPFunctionIndex = GetDSPFunctionIndex( *io_pFormat );
}

/** 2012 0x9422e0; 2013 0x93ec20. */
AKRESULT CAkResampler::Execute( AkAudioBuffer * io_pInBuffer, AkAudioBuffer * io_pOutBuffer )
{
	if( !io_pInBuffer->uValidFrames )
	{
		return AK_NoMoreData;
	}
	AKRESULT eResult;
	do
	{
		eResult = PitchDSPFuncTable[ m_PitchOperationMode ][ m_DSPFunctionIndex ]( io_pInBuffer, io_pOutBuffer, m_InternalPitchState.uRequestedFrames, &m_InternalPitchState );
		if( m_PitchOperationMode == PitchOperatingMode_Interpolating && m_InternalPitchState.uInterpolationRampCount >= PITCHRAMPLENGTH )
		{
			m_InternalPitchState.uCurrentFrameSkip = m_InternalPitchState.uTargetFrameSkip;
			m_PitchOperationMode = PitchOperatingMode_Fixed;
		}
	}
	while( io_pInBuffer->uValidFrames && io_pOutBuffer->uValidFrames < m_InternalPitchState.uRequestedFrames );
	return eResult;
}

/** 2012 0x942350; 2013 0x93ec90. */
bool CAkResampler::IsPostDeInterleaveRequired()
{
	return m_PitchOperationMode == PitchOperatingMode_Bypass && m_uNumChannels > 2 && !IsNativeFloatIndex( m_DSPFunctionIndex );
}

/** 2012 0x942380; 2013 0x93ed40 (callee). */
AKRESULT CAkResampler::DeinterleaveAndSwapOutput( AkPipelineBufferBase * io_pIOBuffer )
{
	AkPipelineBuffer DeinterleavedBuffer;
	*static_cast<AkAudioBuffer *>( &DeinterleavedBuffer ) = *io_pIOBuffer;
	const AKRESULT eResult = DeinterleavedBuffer.GetCachedBuffer( io_pIOBuffer->uMaxFrames, io_pIOBuffer->uChannelMask );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	DeinterleavedBuffer.uValidFrames = io_pIOBuffer->uValidFrames;
	Deinterleave_Native_NChan( io_pIOBuffer, &DeinterleavedBuffer );
	io_pIOBuffer->ReleaseCachedBuffer();
	io_pIOBuffer->pData = DeinterleavedBuffer.pData;
	io_pIOBuffer->uChannelMask = DeinterleavedBuffer.uChannelMask;
	io_pIOBuffer->eState = DeinterleavedBuffer.eState;
	io_pIOBuffer->uMaxFrames = DeinterleavedBuffer.uMaxFrames;
	io_pIOBuffer->uValidFrames = DeinterleavedBuffer.uValidFrames;
	return AK_Success;
}

/** 2012 0x942400; 2013 0x93ed40. */
AKRESULT CAkResampler::InterleaveAndSwapOutput( AkPipelineBufferBase * io_pIOBuffer )
{
	AkPipelineBuffer InterleavedBuffer;
	*static_cast<AkAudioBuffer *>( &InterleavedBuffer ) = *io_pIOBuffer;
	const AKRESULT eResult = InterleavedBuffer.GetCachedBuffer( io_pIOBuffer->uMaxFrames, io_pIOBuffer->uChannelMask );
	if( eResult != AK_Success )
	{
		return eResult;
	}
	InterleavedBuffer.uValidFrames = io_pIOBuffer->uValidFrames;
	Interleave_Native_NChan( io_pIOBuffer, &InterleavedBuffer );
	io_pIOBuffer->ReleaseCachedBuffer();
	io_pIOBuffer->pData = InterleavedBuffer.pData;
	io_pIOBuffer->uChannelMask = InterleavedBuffer.uChannelMask;
	io_pIOBuffer->eState = InterleavedBuffer.eState;
	io_pIOBuffer->uMaxFrames = InterleavedBuffer.uMaxFrames;
	io_pIOBuffer->uValidFrames = InterleavedBuffer.uValidFrames;
	return AK_Success;
}

/** 2012 0x942480; 2013 0x93edc0. */
void CAkResampler::TimeInputToOutput( AkUInt32 & io_uFrames )
{
	io_uFrames = (AkUInt32)(AkInt64)( (AkReal64)io_uFrames / ( (AkReal64)m_InternalPitchState.uCurrentFrameSkip * 0.000015258789 ) + 0.5 );
}

/** 2012 0x9424e0; 2013 0x93ee20. */
void CAkResampler::TimeOutputToInput( AkUInt32 & io_uFrames )
{
	io_uFrames = (AkUInt32)(AkInt64)( (AkReal64)m_InternalPitchState.uCurrentFrameSkip * 0.000015258789 * (AkReal64)io_uFrames + 0.5 );
}

/** 2012 0x942540; 2013 0x93ee80. */
void CAkResampler::ResetOffsets()
{
	m_InternalPitchState.uOutFrameOffset = 0;
	m_InternalPitchState.uInFrameOffset = 0;
}

/** 2012 0x942550; 2013 0x93ee90. */
void CAkResampler::SetPitch( AkReal32 in_fPitchVal )
{
	if( in_fPitchVal <= -2400.f )
	{
		in_fPitchVal = -2400.f;
	}
	else if( in_fPitchVal >= 2400.f )
	{
		in_fPitchVal = 2400.f;
	}
	if( m_bFirstSetPitch )
	{
		const AkUInt32 uFrameSkip = FrameSkipFromPitch( in_fPitchVal, m_fSampleRateConvertRatio );
		m_InternalPitchState.uCurrentFrameSkip = uFrameSkip;
		m_InternalPitchState.uTargetFrameSkip = uFrameSkip;
		m_InternalPitchState.uInterpolationRampCount = PITCHRAMPLENGTH;
		m_fTargetPitchVal = in_fPitchVal;
		m_bFirstSetPitch = false;
	}
	if( in_fPitchVal != m_fTargetPitchVal )
	{
		if( m_PitchOperationMode == PitchOperatingMode_Interpolating )
		{
			m_InternalPitchState.uCurrentFrameSkip += (AkInt32)( m_InternalPitchState.uInterpolationRampCount * ( m_InternalPitchState.uTargetFrameSkip - m_InternalPitchState.uCurrentFrameSkip ) ) / PITCHRAMPLENGTH;
		}
		m_InternalPitchState.uInterpolationRampCount = 0;
		m_fTargetPitchVal = in_fPitchVal;
		m_InternalPitchState.uTargetFrameSkip = FrameSkipFromPitch( in_fPitchVal, m_fSampleRateConvertRatio );
	}
	if( m_InternalPitchState.uCurrentFrameSkip == m_InternalPitchState.uTargetFrameSkip )
	{
		m_PitchOperationMode = m_InternalPitchState.uCurrentFrameSkip != FPMUL ? PitchOperatingMode_Fixed : PitchOperatingMode_Bypass;
	}
	else
	{
		m_PitchOperationMode = PitchOperatingMode_Interpolating;
	}
}

/** 2012 0x9426b0; 2013 0x93eff0. */
void CAkResampler::SetPitchForTimeSkip( AkReal32 in_fPitchVal )
{
	if( m_bFirstSetPitch || in_fPitchVal != m_fTargetPitchVal )
	{
		if( in_fPitchVal <= -2400.f )
		{
			in_fPitchVal = -2400.f;
		}
		else if( in_fPitchVal >= 2400.f )
		{
			in_fPitchVal = 2400.f;
		}
		const AkUInt32 uFrameSkip = FrameSkipFromPitch( in_fPitchVal, m_fSampleRateConvertRatio );
		m_InternalPitchState.uCurrentFrameSkip = uFrameSkip;
		m_InternalPitchState.uTargetFrameSkip = uFrameSkip;
		m_InternalPitchState.uInterpolationRampCount = PITCHRAMPLENGTH;
		m_fTargetPitchVal = in_fPitchVal;
	}
}

/** 2012 0x942760; 2013 0x93f0a0. */
void CAkResampler::SwitchTo( const AkAudioFormat & in_fmt, AkReal32 in_fPitch, AkPipelineBufferBase * io_pIOBuffer, AkUInt32 in_uSampleRate )
{
	AkReal32 fLastValues[ AK_VOICE_MAX_NUM_CHANNELS ];
	GetLastValues( fLastValues );
	const bool bWasInterleaved = IsPostDeInterleaveRequired();
	const AkReal32 fNewSampleRateConvertRatio = (AkReal32)( (AkReal64)in_fmt.uSampleRate / (AkReal64)in_uSampleRate );
	if( m_fSampleRateConvertRatio != fNewSampleRateConvertRatio )
	{
		m_fSampleRateConvertRatio = fNewSampleRateConvertRatio;
		m_bFirstSetPitch = true;
	}
	SetPitch( in_fPitch );
	m_DSPFunctionIndex = GetDSPFunctionIndex( in_fmt );
	const bool bIsInterleaved = IsPostDeInterleaveRequired();
	if( !bIsInterleaved && bWasInterleaved )
	{
		DeinterleaveAndSwapOutput( io_pIOBuffer );
	}
	else if( bIsInterleaved && !bWasInterleaved )
	{
		InterleaveAndSwapOutput( io_pIOBuffer );
	}
	SetLastValues( fLastValues );
}
