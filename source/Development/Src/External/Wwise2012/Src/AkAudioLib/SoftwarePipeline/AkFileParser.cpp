/*=============================================================================
	AkFileParser.cpp - AkFileParser::Parse (akfileparserbase.cpp, 2012 0x95f020; 2013 matched by bytes).

	DISHONORED(port): agent IH2 fork S (wave 26). Chunks: 'fmt ' (required before 'cue ' and 'data'), 'cue ' (markers),
	'LIST' (descended into: only its 4-byte 'adtl' type is skipped), 'labl' (a marker's label), 'smpl' (the first loop),
	'seek' (the seek table, if the caller wants it), 'data' (ends the parse). An odd chunk is followed by a zero pad byte.
=============================================================================*/
#include "AkFileParser.h"
#include "AkMarkers.h"

#define AkFourcc( a, b, c, d ) ( (AkUInt32)(AkUInt8)(a) | ( (AkUInt32)(AkUInt8)(b) << 8 ) | ( (AkUInt32)(AkUInt8)(c) << 16 ) | ( (AkUInt32)(AkUInt8)(d) << 24 ) )

namespace AkFileParser
{
	enum ParseStage
	{
		HaveFmt = 0x01,
		HaveCue = 0x04,
		HaveSmpl = 0x08,
		HaveSeek = 0x10
	};

	struct ChunkHeader
	{
		AkUInt32 ChunkId;
		AkUInt32 dwChunkSize;
	};

	struct CuePoint
	{
		AkUInt32 dwIdentifier;
		AkUInt32 dwPosition;
		AkUInt32 fccChunk;
		AkUInt32 dwChunkStart;
		AkUInt32 dwBlockStart;
		AkUInt32 dwSampleOffset;
	};

	struct SampleChunk
	{
		AkUInt32 dwManufacturer;
		AkUInt32 dwProduct;
		AkUInt32 dwSamplePeriod;
		AkUInt32 dwMIDIUnityNote;
		AkUInt32 dwMIDIPitchFraction;
		AkUInt32 dwSMPTEFormat;
		AkUInt32 dwSMPTEOffset;
		AkUInt32 dwSampleLoops;
		AkUInt32 cbSamplerData;
	};

	struct SampleLoop
	{
		AkUInt32 dwIdentifier;
		AkUInt32 dwType;
		AkUInt32 dwStart;
		AkUInt32 dwEnd;
		AkUInt32 dwFraction;
		AkUInt32 dwPlayCount;
	};

	/** 2012 0x95f020. */
	AKRESULT Parse( const void * in_pvBuffer, AkUInt32 in_ulBufferSize, FormatInfo & out_pFormatInfo, CAkMarkers * out_pMarkers, AkUInt32 * out_pulLoopStart, AkUInt32 * out_pulLoopEnd, AkUInt32 * out_pulDataSize, AkUInt32 * out_pulDataOffset, SeekInfo * out_pSeekTableInfo )
	{
		if( !in_pvBuffer || !in_ulBufferSize )
		{
			return AK_InvalidParameter;
		}
		*out_pulLoopStart = 0;
		*out_pulLoopEnd = 0;

		const AkUInt8 * pBufferBegin = (const AkUInt8 *)in_pvBuffer;
		const AkUInt8 * pBufferEnd = pBufferBegin + in_ulBufferSize;
		if( pBufferBegin + 12 > pBufferEnd )
		{
			return AK_InvalidFile;
		}
		const AkUInt32 * pRiff = (const AkUInt32 *)pBufferBegin;
		if( pRiff[ 0 ] != AkFourcc( 'R', 'I', 'F', 'F' ) )
		{
			return AK_InvalidFile;
		}
		if( pRiff[ 2 ] != AkFourcc( 'W', 'A', 'V', 'E' ) && pRiff[ 2 ] != AkFourcc( 'X', 'W', 'M', 'A' ) )
		{
			return AK_InvalidFile;
		}

		AkUInt8 uParseStage = 0;
		const AkUInt8 * pThis = pBufferBegin + 12;
		AkUInt32 ulBytesLeft = in_ulBufferSize - 12;
		if( ulBytesLeft < sizeof( ChunkHeader ) )
		{
			return AK_InvalidFile;
		}
		for( ;; )
		{
			const ChunkHeader * pChunkHeader = (const ChunkHeader *)pThis;
			AkUInt32 dwChunkSize = pChunkHeader->dwChunkSize;
			const AkUInt32 ChunkId = pChunkHeader->ChunkId;
			const AkUInt8 * pChunk = pThis + sizeof( ChunkHeader );
			if( ulBytesLeft - sizeof( ChunkHeader ) < dwChunkSize && ChunkId != AkFourcc( 'd', 'a', 't', 'a' ) )
			{
				return AK_InvalidFile;
			}

			if( ChunkId == AkFourcc( 'f', 'm', 't', ' ' ) )
			{
				if( !( uParseStage & HaveFmt ) )
				{
					uParseStage |= HaveFmt;
					out_pFormatInfo.uFormatSize = dwChunkSize;
					out_pFormatInfo.pFormat = (WaveFormatExtensible *)pChunk;
				}
			}
			else if( ChunkId == AkFourcc( 'd', 'a', 't', 'a' ) )
			{
				if( !( uParseStage & HaveFmt ) )
				{
					return AK_InvalidFile;
				}
				*out_pulDataSize = dwChunkSize;
				*out_pulDataOffset = (AkUInt32)( pChunk - pBufferBegin );
				return AK_Success;
			}
			else if( ChunkId == AkFourcc( 'c', 'u', 'e', ' ' ) )
			{
				if( !( uParseStage & HaveFmt ) )
				{
					return AK_InvalidFile;
				}
				if( !( uParseStage & HaveCue ) && out_pMarkers )
				{
					const AkUInt32 dwCuePoints = *(const AkUInt32 *)pChunk;
					if( dwCuePoints )
					{
						const AKRESULT eResult = out_pMarkers->Allocate( dwCuePoints );
						if( eResult != AK_Success )
						{
							return eResult;
						}
						const CuePoint * pCuePoint = (const CuePoint *)( pChunk + sizeof( AkUInt32 ) );
						for( AkUInt32 i = 0; i < out_pMarkers->m_hdrMarkers.uNumMarkers; ++i, ++pCuePoint )
						{
							out_pMarkers->m_pMarkers[ i ].dwIdentifier = pCuePoint->dwIdentifier;
							out_pMarkers->m_pMarkers[ i ].dwPosition = pCuePoint->dwPosition;
							out_pMarkers->m_pMarkers[ i ].strLabel = NULL;
						}
					}
					uParseStage |= HaveCue;
				}
			}
			else if( ChunkId == AkFourcc( 'L', 'I', 'S', 'T' ) )
			{
				dwChunkSize = sizeof( AkUInt32 );
			}
			else if( ChunkId == AkFourcc( 's', 'e', 'e', 'k' ) )
			{
				uParseStage |= HaveSeek;
				out_pSeekTableInfo->uSeekChunkSize = dwChunkSize;
				out_pSeekTableInfo->pSeekTable = (void *)pChunk;
			}
			else if( ChunkId == AkFourcc( 'l', 'a', 'b', 'l' ) )
			{
				if( ( uParseStage & HaveCue ) && out_pMarkers )
				{
					const AkUInt32 dwCuePointID = *(const AkUInt32 *)pChunk;
					for( AkUInt32 i = 0; i < out_pMarkers->m_hdrMarkers.uNumMarkers; ++i )
					{
						if( out_pMarkers->m_pMarkers[ i ].dwIdentifier == dwCuePointID )
						{
							out_pMarkers->SetLabel( i, (char *)pChunk + sizeof( AkUInt32 ), dwChunkSize - sizeof( AkUInt32 ) );
							break;
						}
					}
				}
			}
			else if( ChunkId == AkFourcc( 's', 'm', 'p', 'l' ) )
			{
				const SampleChunk * pSampleChunk = (const SampleChunk *)pChunk;
				if( pSampleChunk->dwSampleLoops )
				{
					const SampleLoop * pSampleLoop = (const SampleLoop *)( pChunk + sizeof( SampleChunk ) + pSampleChunk->cbSamplerData );
					*out_pulLoopStart = pSampleLoop->dwStart;
					*out_pulLoopEnd = pSampleLoop->dwEnd;
				}
				uParseStage |= HaveSmpl;
			}

			pThis = pChunk + dwChunkSize;
			if( ( dwChunkSize & 1 ) && !*pThis )
			{
				++pThis;
				if( pThis > pBufferEnd )
				{
					return AK_InvalidFile;
				}
			}
			ulBytesLeft = (AkUInt32)( pBufferEnd - pThis );
			if( ulBytesLeft < sizeof( ChunkHeader ) )
			{
				return AK_InvalidFile;
			}
		}
	}
}
