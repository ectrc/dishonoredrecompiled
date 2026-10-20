/*=============================================================================
	AkCommonDefs.h - the SDK public header include/AK/SoundEngine/Common/AkCommonDefs.h (2012.1): the audio format and
	the plug-in audio buffer every effect plug-in and the lower engine share.

	DISHONORED(port): agent IH4 fork F1 (wave 26). Moved out of AkLEngineStructs.h (agent IH2) unchanged: AkAudioFormat
	(8 bytes, the 2012 PDB bitfields) and AkAudioBuffer (16 bytes) are the PDB's. The SDK's AkAudioBuffer accessors are
	inline (GetChannel, NumChannels, MaxFrames, ...), the deinterleaved channel i starting at pData + i * uMaxFrames.
=============================================================================*/
#ifndef _AK_COMMON_DEFS_H_
#define _AK_COMMON_DEFS_H_

#include <AK/SoundEngine/Common/AkTypes.h>
#include <string.h>

inline AkUInt32 AkGetNumChannels( AkUInt32 in_uChannelMask )
{
	AkUInt32 num = 0;
	while( in_uChannelMask )
	{
		++num;
		in_uChannelMask &= in_uChannelMask - 1;
	}
	return num;
}

/** 8 bytes: the 2012 PDB's bitfields. */
struct AkAudioFormat
{
	AkUInt32	uSampleRate;
	AkUInt32	uChannelMask : 18;
	AkUInt32	uBitsPerSample : 6;
	AkUInt32	uBlockAlign : 5;
	AkUInt32	uTypeID : 2;
	AkUInt32	uInterleaveID : 1;

	AkUInt32 GetNumChannels() const { return AkGetNumChannels( uChannelMask ); }
	void SetAll( AkUInt32 in_uSampleRate, AkUInt32 in_uChannelMask, AkUInt32 in_uBitsPerSample, AkUInt32 in_uBlockAlign, AkUInt32 in_uTypeID, AkUInt32 in_uInterleaveID )
	{
		uSampleRate = in_uSampleRate;
		uChannelMask = in_uChannelMask;
		uBitsPerSample = in_uBitsPerSample;
		uBlockAlign = in_uBlockAlign;
		uTypeID = in_uTypeID;
		uInterleaveID = in_uInterleaveID;
	}
};
static_assert( sizeof( AkAudioFormat ) == 8, "AkAudioFormat: 2012 PDB sizeof 8" );

/** 16 bytes. */
struct AkAudioBuffer
{
	void *		pData;
	AkUInt32	uChannelMask;
	AKRESULT	eState;
	AkUInt16	uMaxFrames;
	AkUInt16	uValidFrames;

	AkUInt32 NumChannels() const { return AkGetNumChannels( uChannelMask ); }
	AkReal32 * GetChannel( AkUInt32 in_uIndex ) { return (AkReal32 *)pData + in_uIndex * uMaxFrames; }
	/** 2012 0x97c5a0: the LFE is the last channel of a deinterleaved buffer. */
	AkReal32 * GetLFE()
	{
		if( !( uChannelMask & AK_SPEAKER_LOW_FREQUENCY ) )
		{
			return NULL;
		}
		return (AkReal32 *)pData + ( NumChannels() - 1 ) * uMaxFrames;
	}
	void * GetInterleavedData() { return pData; }
	bool HasData() const { return pData != NULL; }
	AkUInt16 MaxFrames() const { return uMaxFrames; }
	void SetRequestSize( AkUInt16 in_uMaxFrames ) { uMaxFrames = in_uMaxFrames; }
	void Clear()
	{
		pData = NULL;
		uValidFrames = 0;
	}
	void ClearData()
	{
		if( pData )
		{
			memset( pData, 0, uMaxFrames * NumChannels() * sizeof( AkReal32 ) );
		}
	}
	void ZeroPadToMaxFrames()
	{
		const AkUInt32 uNumChannels = NumChannels();
		const AkUInt32 uNumZeroFrames = uMaxFrames - uValidFrames;
		if( uNumZeroFrames )
		{
			for( AkUInt32 i = 0; i < uNumChannels; ++i )
			{
				memset( GetChannel( i ) + uValidFrames, 0, uNumZeroFrames * sizeof( AkReal32 ) );
			}
			uValidFrames = uMaxFrames;
		}
	}
};
static_assert( sizeof( AkAudioBuffer ) == 16, "AkAudioBuffer: 2012 PDB sizeof 16" );

#endif // _AK_COMMON_DEFS_H_
