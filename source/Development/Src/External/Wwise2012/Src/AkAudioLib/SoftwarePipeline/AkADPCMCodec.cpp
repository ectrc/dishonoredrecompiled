/*=============================================================================
	AkADPCMCodec.cpp - CAkADPCMCodec::Decode (akadpcmcodec.cpp, 2012 0x95fb90; 2013 matched by bytes) and its tables.

	DISHONORED(port): agent IH2 fork S (wave 26). m_asStep / m_asNextStep are the standard IMA tables; both were checked
	byte for byte against the 2012 shipping executable's .rdata (the index table immediately precedes the step table).
	A block is a 4-byte header (first sample, step index) and 32 bytes of 4-bit codes, low nibble first; channels are
	decoded one at a time into interleaved 16-bit output.
=============================================================================*/
#include "AkADPCMCodec.h"

const AkInt16 CAkADPCMCodec::m_asStep[ 89 ] =
{
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
	143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
	1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
	9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

const AkInt16 CAkADPCMCodec::m_asNextStep[ 16 ] =
{
	-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8
};

static inline AkInt32 AdpcmDecodeSample( AkInt32 in_iCode, AkInt32 & io_iSample, AkInt32 & io_iStepIndex )
{
	const AkInt32 iMagnitude = ( 2 * ( in_iCode & 7 ) + 1 ) * CAkADPCMCodec::m_asStep[ io_iStepIndex ];
	AkInt32 iSample = io_iSample + ( ( in_iCode & 8 ) ? iMagnitude / -8 : iMagnitude / 8 );
	if( (AkInt16)iSample != iSample )
	{
		iSample = ( iSample >= -32768 ) ? 32767 : -32768;
	}
	io_iStepIndex += CAkADPCMCodec::m_asNextStep[ in_iCode & 0xF ];
	if( io_iStepIndex < 0 )
	{
		io_iStepIndex = 0;
	}
	else if( io_iStepIndex >= 89 )
	{
		io_iStepIndex = 88;
	}
	io_iSample = iSample;
	return iSample;
}

/** 2012 0x95fb90. */
bool CAkADPCMCodec::Decode( AkUInt8 * pbSrc, AkUInt8 * pbDst, AkUInt32 cBlocks, AkUInt32 nBlockAlignment, AkUInt32 nChannels )
{
	const AkUInt32 uDstStride = 2 * nChannels;
	while( cBlocks-- )
	{
		AkInt32 iSample = *(AkInt16 *)pbSrc;
		AkInt32 iStepIndex = pbSrc[ 2 ];
		*(AkInt16 *)pbDst = (AkInt16)iSample;
		pbDst += uDstStride;
		const AkUInt8 * pbCodes = pbSrc + 4;
		for( AkUInt32 cPairs = 0; cPairs < 32; ++cPairs )
		{
			const AkInt32 iCodes = *pbCodes++;
			*(AkInt16 *)pbDst = (AkInt16)AdpcmDecodeSample( iCodes & 0xF, iSample, iStepIndex );
			pbDst += uDstStride;
			if( cPairs == 31 )
			{
				break;
			}
			*(AkInt16 *)pbDst = (AkInt16)AdpcmDecodeSample( iCodes >> 4, iSample, iStepIndex );
			pbDst += uDstStride;
		}
		pbSrc += nBlockAlignment;
	}
	return true;
}
