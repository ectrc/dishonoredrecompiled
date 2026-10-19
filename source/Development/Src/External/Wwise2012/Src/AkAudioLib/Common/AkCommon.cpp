/*=============================================================================
	AkCommon.cpp - port of akcommon.cpp and the out-of-line akcommon.h helpers (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26). The cached buffers come from CAkLEngine's per-size cache
	(CAkLEngine::GetCachedAudioBuffer / ReleaseCachedAudioBuffer, aklengine_softwarepipeline.cpp).
=============================================================================*/
#include "AkCommon.h"
#include "AkLEngine.h"
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <string.h>

/** 2012 0x946d00; 2013 0x943590. */
void AkPipelineBuffer::FreeMarkers()
{
	if( pMarkers )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, pMarkers );
	}
	uNumMarkers = 0;
	pMarkers = NULL;
}

/** 2012 0x946d30; 2013 0x9435c0. */
AKRESULT AkPipelineBufferBase::GetCachedBuffer( AkUInt16 in_uMaxFrames, AkChannelMask in_uChannelMask )
{
	void * pCachedData = CAkLEngine::GetCachedAudioBuffer( AkGetNumChannels( in_uChannelMask ) * in_uMaxFrames * sizeof( AkReal32 ) );
	if( !pCachedData )
	{
		return AK_InsufficientMemory;
	}
	uChannelMask = in_uChannelMask;
	pData = pCachedData;
	uMaxFrames = in_uMaxFrames;
	uValidFrames = 0;
	return AK_Success;
}

/** 2012 0x946d90; 2013 0x943620. */
void AkPipelineBufferBase::ReleaseCachedBuffer()
{
	CAkLEngine::ReleaseCachedAudioBuffer( AkGetNumChannels( uChannelMask ) * uMaxFrames * sizeof( AkReal32 ), pData );
	pData = NULL;
	uMaxFrames = 0;
}

/** 2012 0x93acd0; 2013 0x937670. */
void ZeroPadBuffer( AkAudioBuffer * io_pAudioBuffer )
{
	const AkUInt32 uNumZeroFrames = io_pAudioBuffer->uMaxFrames - io_pAudioBuffer->uValidFrames;
	if( uNumZeroFrames )
	{
		const AkUInt32 uNumChannels = io_pAudioBuffer->NumChannels();
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			memset( io_pAudioBuffer->GetChannel( i ) + io_pAudioBuffer->uValidFrames, 0, uNumZeroFrames * sizeof( AkReal32 ) );
		}
	}
}

/** 2012 0x9415c0; 2013 0x93def0. */
void ZeroPrePadBufferInterleaved( AkAudioBuffer * io_pAudioBuffer, AkUInt32 in_ulNumFramestoZero )
{
	if( in_ulNumFramestoZero )
	{
		memset( io_pAudioBuffer->pData, 0, in_ulNumFramestoZero * io_pAudioBuffer->NumChannels() * sizeof( AkReal32 ) );
	}
}

/** 2012 0x941ae0; 2013 0x93e410. */
void ZeroPrePadBuffer( AkAudioBuffer * io_pAudioBuffer, AkUInt32 in_ulNumFramestoZero )
{
	if( in_ulNumFramestoZero )
	{
		const AkUInt32 uNumChannels = io_pAudioBuffer->NumChannels();
		for( AkUInt32 i = 0; i < uNumChannels; ++i )
		{
			memset( io_pAudioBuffer->GetChannel( i ), 0, in_ulNumFramestoZero * sizeof( AkReal32 ) );
		}
	}
}

/** 2012 0x95eb90; 2013 0x95b380. */
void AbsoluteToRelativeSourceOffset( AkUInt32 in_uAbsoluteSourcePosition, AkUInt32 in_uLoopStart, AkUInt32 in_uLoopEnd, AkUInt16 in_usLoopCount, AkUInt32 & out_uRelativeSourceOffset, AkUInt16 & out_uRemainingLoops )
{
	out_uRemainingLoops = in_usLoopCount;
	if( in_usLoopCount == 1 || in_uAbsoluteSourcePosition <= in_uLoopEnd || in_uLoopEnd <= in_uLoopStart )
	{
		out_uRelativeSourceOffset = in_uAbsoluteSourcePosition;
		return;
	}
	const AkUInt32 uLoopSize = in_uLoopEnd - in_uLoopStart + 1;
	const AkUInt32 uNumLoopsDone = ( in_uAbsoluteSourcePosition - in_uLoopStart ) / uLoopSize;
	const AkUInt32 uOffsetInLoop = ( in_uAbsoluteSourcePosition - in_uLoopStart ) % uLoopSize;
	if( uNumLoopsDone >= in_usLoopCount && in_usLoopCount )
	{
		out_uRelativeSourceOffset = in_uAbsoluteSourcePosition - uLoopSize * ( in_usLoopCount - 1 );
		out_uRemainingLoops = 1;
		return;
	}
	out_uRemainingLoops = in_usLoopCount ? (AkUInt16)( in_usLoopCount - uNumLoopsDone ) : 0;
	out_uRelativeSourceOffset = in_uLoopStart + uOffsetInLoop;
}
