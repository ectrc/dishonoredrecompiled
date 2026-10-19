/*=============================================================================
	bitwise.cpp - the LSb-first bit reader of the Vorbis decoder (Tremor's bitwise.c without its ogg_reference chain).

	DISHONORED(port): agent IH2 (wave 26), from the 2012 decompile. Retail reads straight from one contiguous packet:
	no end-of-buffer test in look / adv (headend is only decremented, the callers test headend < 0 where retail does).
=============================================================================*/
#include "codec_internal.h"

/** 2012 0x978520; 2013 0x974d00. */
long oggpack_look( oggpack_buffer * b, int bits )
{
	const int headbit = b->headbit;
	const unsigned char * headptr = b->headptr;
	const int end = headbit + bits;
	AkUInt32 ret = headptr[ 0 ] >> headbit;
	if( end > 8 )
	{
		ret |= (AkUInt32)headptr[ 1 ] << ( 8 - headbit );
		if( end > 16 )
		{
			ret |= (AkUInt32)headptr[ 2 ] << ( 16 - headbit );
			if( end > 24 )
			{
				ret |= (AkUInt32)headptr[ 3 ] << ( 24 - headbit );
				if( end > 32 && headbit )
				{
					ret |= (AkUInt32)headptr[ 4 ] << ( 32 - headbit );
				}
			}
		}
	}
	return mask[ bits ] & ret;
}

/** 2012 0x9785a0; 2013 0x974d80. */
void oggpack_adv( oggpack_buffer * b, int bits )
{
	bits += b->headbit;
	b->headend -= bits >> 3;
	b->headptr += bits >> 3;
	b->headbit = bits & 7;
}

/** 2012 0x9785c0; 2013 0x974da0. */
long oggpack_read( oggpack_buffer * b, int bits )
{
	const long ret = oggpack_look( b, bits );
	oggpack_adv( b, bits );
	return ret;
}
