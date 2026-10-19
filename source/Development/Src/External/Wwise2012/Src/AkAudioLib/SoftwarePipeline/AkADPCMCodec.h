/*=============================================================================
	AkADPCMCodec.h - CAkADPCMCodec (akadpcmcodec.cpp/.h): Wwise IMA ADPCM, 64 samples in a 36-byte block per channel.

	DISHONORED(port): agent IH2 fork S (wave 26).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

#define ADPCM_SAMPLES_PER_BLOCK 64
#define ADPCM_BLOCK_SIZE 36

class CAkADPCMCodec
{
public:
	static bool Decode( AkUInt8 * pbSrc, AkUInt8 * pbDst, AkUInt32 cBlocks, AkUInt32 nBlockAlignment, AkUInt32 nChannels );

	static const AkInt16 m_asStep[ 89 ];
	static const AkInt16 m_asNextStep[ 16 ];
};
