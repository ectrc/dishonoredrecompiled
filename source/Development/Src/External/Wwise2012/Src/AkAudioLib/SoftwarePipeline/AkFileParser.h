/*=============================================================================
	AkFileParser.h - AkFileParser (akfileparserbase.cpp/.h): the RIFF/RIFX header of a Wwise media file.

	DISHONORED(port): agent IH2 fork S (wave 26). WaveFormatExtensible is the decoder's (AkVorbisCodec.h, 24 bytes,
	packed to 2), the same record every codec's 'fmt ' chunk starts with.
=============================================================================*/
#pragma once

#include "AkLEngineStructs.h"
#include "AkVorbisCodec.h"

class CAkMarkers;

#define AK_WAVE_FORMAT_ADPCM 0x0002
#define AK_WAVE_FORMAT_EXTENSIBLE 0xFFFE
#define AK_WAVE_FORMAT_VORBIS 0xFFFF

namespace AkFileParser
{
	struct FormatInfo
	{
		AkUInt32 uFormatSize;
		WaveFormatExtensible * pFormat;
	};

	struct SeekInfo
	{
		AkUInt32 uSeekChunkSize;
		void * pSeekTable;
	};

	AKRESULT Parse( const void * in_pvBuffer, AkUInt32 in_ulBufferSize, FormatInfo & out_pFormatInfo, CAkMarkers * out_pMarkers, AkUInt32 * out_pulLoopStart, AkUInt32 * out_pulLoopEnd, AkUInt32 * out_pulDataSize, AkUInt32 * out_pulDataOffset, SeekInfo * out_pSeekTableInfo );
}
