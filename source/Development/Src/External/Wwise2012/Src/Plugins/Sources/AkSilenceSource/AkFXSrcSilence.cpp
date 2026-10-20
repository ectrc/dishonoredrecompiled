/*=============================================================================
	AkFXSrcSilence.cpp - CAkFXSrcSilence and CAkFxSrcSilenceParams (plugins\sources\aksilencesource\akfxsrcsilence.cpp,
	Wwise 2012.1; 2012 0x97a940..0x97ae40, 2013 matched by bytes): the Silence source, silence for a duration (randomised
	once per instance within -minus .. +plus seconds, at least 1 ms) times the loop count (0 = infinite).

	DISHONORED(port): agent IH4 fork F1 (wave 26). Layouts are the 2012 PDB's (CAkFXSrcSilence 36 bytes,
	CAkFxSrcSilenceParams 16, AkFXSrcSilenceParams 12). The randomisation uses the CRT rand() as retail.
=============================================================================*/
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <stdlib.h>
#include <string.h>

struct AkFXSrcSilenceParams
{
	AkReal32 fDuration;
	AkReal32 fRandomizedLengthMinus;
	AkReal32 fRandomizedLengthPlus;
};

class CAkFxSrcSilenceParams : public AK::IAkPluginParam
{
public:
	CAkFxSrcSilenceParams() {}
	CAkFxSrcSilenceParams( const CAkFxSrcSilenceParams & in_rCopy ) : m_Params( in_rCopy.m_Params ) {}
	virtual ~CAkFxSrcSilenceParams() {}

	/** 2012 0x97a990: 0 duration, 1 minus, 2 plus (seconds). */
	virtual AKRESULT SetParam( AkPluginParamID in_ParamID, const void * in_pValue, AkUInt32 in_uParamSize )
	{
		if( !in_pValue )
		{
			return AK_InvalidParameter;
		}
		const AkReal32 fValue = *(const AkReal32 *)in_pValue;
		switch( in_ParamID )
		{
		case 0:		m_Params.fDuration = fValue; return AK_Success;
		case 1:		m_Params.fRandomizedLengthMinus = fValue; return AK_Success;
		case 2:		m_Params.fRandomizedLengthPlus = fValue; return AK_Success;
		default:	return AK_InvalidParameter;
		}
	}

	/** 2012 0x97ac10. */
	virtual AK::IAkPluginParam * Clone( AK::IAkPluginMemAlloc * in_pAllocator )
	{
		return AK_PLUGIN_NEW( in_pAllocator, CAkFxSrcSilenceParams( *this ) );
	}

	/** 2012 0x97a950: the block, else 1 s, no randomisation. */
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_uBlockSize )
	{
		if( in_uBlockSize )
		{
			return SetParamsBlock( in_pParamsBlock, in_uBlockSize );
		}
		m_Params.fDuration = 1.f;
		m_Params.fRandomizedLengthMinus = 0.f;
		m_Params.fRandomizedLengthPlus = 0.f;
		return AK_Success;
	}

	/** 2012 0x97ac50. */
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator )
	{
		AK_PLUGIN_DELETE( in_pAllocator, this );
		return AK_Success;
	}

	/** 2012 0x97ac80. */
	virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_uBlockSize )
	{
		memcpy( &m_Params, in_pParamsBlock, sizeof( m_Params ) );
		return AK_Success;
	}

	AkFXSrcSilenceParams m_Params;
};

class CAkFXSrcSilence : public AK::IAkSourcePlugin
{
public:
	/** 2012 0x97ade0. */
	CAkFXSrcSilence()
		: m_uSampleRate( 0 ), m_uBytesPerSample( 0 ), m_ulOutByteCount( 0 ), m_fDurationModifier( 0.f ), m_sNumLoops( 1 ), m_sCurLoopCount( 0 )
		, m_fInitDuration( 0.f ), m_pSharedParams( NULL ), m_pSourceFXContext( NULL )
	{
	}
	virtual ~CAkFXSrcSilence() {}

	/** 2012 0x97ad60. */
	virtual AKRESULT Term( AK::IAkPluginMemAlloc * in_pAllocator )
	{
		AK_PLUGIN_DELETE( in_pAllocator, this );
		return AK_Success;
	}

	/** 2012 0x97a9f0. */
	virtual AKRESULT Reset()
	{
		m_sCurLoopCount = 0;
		m_ulOutByteCount = 0;
		return AK_Success;
	}

	/** 2012 0x97aa10. */
	virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo )
	{
		out_rPluginInfo.eType = AkPluginTypeSource;
		out_rPluginInfo.bIsInPlace = true;
		out_rPluginInfo.bIsAsynchronous = false;
		return AK_Success;
	}

	/** 2012 0x97acb0: the duration modifier is drawn once (rand / 32767 between minus and plus). */
	virtual AKRESULT Init( AK::IAkPluginMemAlloc * in_pAllocator, AK::IAkSourcePluginContext * in_pSourceFXContext, AK::IAkPluginParam * in_pParams, AkAudioFormat & io_rFormat )
	{
		m_pSourceFXContext = in_pSourceFXContext;
		m_uSampleRate = io_rFormat.uSampleRate;
		m_uBytesPerSample = io_rFormat.uBitsPerSample >> 3;
		m_sNumLoops = (AkInt16)in_pSourceFXContext->GetNumLoops();
		m_pSharedParams = static_cast<CAkFxSrcSilenceParams *>( in_pParams );
		const AkReal32 fPlusMax = m_pSharedParams->m_Params.fRandomizedLengthPlus;
		const AkReal32 fMinusMax = m_pSharedParams->m_Params.fRandomizedLengthMinus;
		m_fDurationModifier = (AkReal32)rand() * 0.000030518509f * ( fPlusMax - fMinusMax ) + fMinusMax;
		m_fInitDuration = m_pSharedParams->m_Params.fDuration + m_fDurationModifier;
		if( m_fInitDuration < 0.001f )
		{
			m_fInitDuration = 0.001f;
		}
		return AK_Success;
	}

	/** 2012 0x97ab00: milliseconds, all loops. */
	virtual AkReal32 GetDuration() const
	{
		return (AkReal32)( m_fInitDuration * 1000.0 * (double)m_sNumLoops );
	}

	/** 2012 0x97ab20. */
	virtual AKRESULT StopLooping()
	{
		m_sNumLoops = 1;
		return AK_Success;
	}

	/** 2012 0x97ab30. */
	virtual AKRESULT Seek( AkUInt32 in_uPosition )
	{
		if( m_sNumLoops )
		{
			const AkUInt32 uBytes = in_uPosition * m_uBytesPerSample;
			if( uBytes >= m_uBytesPerSample * m_sNumLoops * TotalFrames() )
			{
				return AK_Fail;
			}
			m_ulOutByteCount = uBytes;
		}
		return AK_Success;
	}

	/** 2012 0x97aa30: frames up to the end of the last loop (AK_NoMoreData there); infinite when looping forever. */
	virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames )
	{
		const AkUInt32 uRequested = m_uBytesPerSample * io_uFrames;
		AkUInt32 uProduced = uRequested;
		AKRESULT eResult = AK_DataReady;
		if( m_sNumLoops )
		{
			const AkUInt32 uTotal = m_uBytesPerSample * ( m_sNumLoops * TotalFrames() );
			uProduced = 0;
			if( m_ulOutByteCount < uTotal )
			{
				const AkUInt32 uLeft = uTotal - m_ulOutByteCount;
				uProduced = uRequested >= uLeft ? uLeft : uRequested;
				m_ulOutByteCount += uProduced;
				if( uProduced >= uRequested )
				{
					io_uFrames = (AkUInt16)( uProduced / m_uBytesPerSample );
					return eResult;
				}
			}
			eResult = AK_NoMoreData;
		}
		io_uFrames = (AkUInt16)( uProduced / m_uBytesPerSample );
		return eResult;
	}

	/** 2012 0x97ad90. */
	virtual void Execute( AkAudioBuffer * io_pBufferOut )
	{
		AkUInt32 uFrames = io_pBufferOut->MaxFrames();
		io_pBufferOut->eState = TimeSkip( uFrames );
		io_pBufferOut->uValidFrames = (AkUInt16)uFrames;
		memset( io_pBufferOut->pData, 0, uFrames * m_uBytesPerSample );
	}

	AkUInt32					m_uSampleRate;
	AkUInt32					m_uBytesPerSample;
	AkUInt32					m_ulOutByteCount;
	AkReal32					m_fDurationModifier;
	AkInt16						m_sNumLoops;
	AkInt16						m_sCurLoopCount;
	AkReal32					m_fInitDuration;
	CAkFxSrcSilenceParams *		m_pSharedParams;
	AK::IAkSourcePluginContext *	m_pSourceFXContext;

private:
	/** The frames of one loop (the current duration, at least 1 ms). */
	AkUInt32 TotalFrames() const
	{
		AkReal32 fDuration = m_pSharedParams->m_Params.fDuration + m_fDurationModifier;
		if( fDuration < 0.001f )
		{
			fDuration = 0.001f;
		}
		return (AkUInt32)(AkInt64)( (double)m_uSampleRate * fDuration );
	}
};

static_assert( sizeof( AkFXSrcSilenceParams ) == 12, "AkFXSrcSilenceParams: 2012 PDB sizeof 12" );
static_assert( sizeof( CAkFxSrcSilenceParams ) == 16, "CAkFxSrcSilenceParams: 2012 PDB sizeof 16" );
static_assert( sizeof( CAkFXSrcSilence ) == 36, "CAkFXSrcSilence: 2012 sizeof 36 (CreateSilenceSource)" );

/** 2012 0x97ae40. */
AK::IAkPlugin * CreateSilenceSource( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkFXSrcSilence() );
}

/** 2012 0x97ae20. */
AK::IAkPluginParam * CreateSilenceSourceParams( AK::IAkPluginMemAlloc * in_pAllocator )
{
	return AK_PLUGIN_NEW( in_pAllocator, CAkFxSrcSilenceParams() );
}
