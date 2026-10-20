/*=============================================================================
	AkFXTailHandler.h - the SDK public plug-in service include/AK/Plugin/PluginServices/AkFXTailHandler.h (2012.1): an
	effect keeps producing (zero-padded input) for its tail length once its input ended.

	DISHONORED(port): agent IH4 fork F1 (wave 26). 8 bytes (PDB). HandleTail 2012 0x97c640 (one copy per plug-in).
=============================================================================*/
#ifndef _AK_FX_TAIL_HANDLER_H_
#define _AK_FX_TAIL_HANDLER_H_

#include <AK/SoundEngine/Common/AkCommonDefs.h>

class AkFXTailHandler
{
public:
	AkFXTailHandler() : uTailFramesRemaining( (AkUInt32)-1 ), uTotalTailFrames( 0 ) {}

	/** 2012 0x97c640: while the input plays the tail is re-armed; once it ended the buffer is zero padded and the state
	    stays AK_DataReady until the tail (grown, not shrunk, by a longer tail request) has run out. */
	void HandleTail( AkAudioBuffer * io_pBuffer, AkUInt32 in_uTotalTailFrames )
	{
		if( io_pBuffer->eState != AK_NoMoreData )
		{
			uTailFramesRemaining = (AkUInt32)-1;
			return;
		}
		if( !uTailFramesRemaining )
		{
			return;
		}
		if( uTailFramesRemaining == (AkUInt32)-1 )
		{
			uTotalTailFrames = in_uTotalTailFrames;
			uTailFramesRemaining = in_uTotalTailFrames;
		}
		else if( in_uTotalTailFrames > uTotalTailFrames )
		{
			uTailFramesRemaining = in_uTotalTailFrames + uTailFramesRemaining - uTotalTailFrames;
			uTotalTailFrames = in_uTotalTailFrames;
		}
		AkUInt32 uFrames = io_pBuffer->uMaxFrames - io_pBuffer->uValidFrames;
		if( uTailFramesRemaining < uFrames )
		{
			uFrames = uTailFramesRemaining;
		}
		uTailFramesRemaining -= uFrames;
		io_pBuffer->ZeroPadToMaxFrames();
		if( uTailFramesRemaining )
		{
			io_pBuffer->eState = AK_DataReady;
		}
	}

	AkUInt32 uTailFramesRemaining;
	AkUInt32 uTotalTailFrames;
};

static_assert( sizeof( AkFXTailHandler ) == 8, "AkFXTailHandler: 2012 PDB sizeof 8" );

#endif
