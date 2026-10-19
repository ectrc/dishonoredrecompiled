/*=============================================================================
	res012.cpp - residue types 0/1/2 (Tremor lowmem res012.c, Audiokinetic's variant).

	DISHONORED(port): agent IH2 (wave 26), from the 2012 decompile. Retail differences kept: types 0 and 1 both decode with
	vorbis_book_decodev_add (no decodevs_add), type 2 always uses the two-channel vorbis_book_decodevv_add_2ch, only the
	type 0/1 partition word read tests for end of packet; tables come from CAkVorbisAllocator.
=============================================================================*/
#include "codec_internal.h"
#include <malloc.h>

/** 2012 0x979390; 2013 0x975b70. */
int res_unpack( vorbis_info_residue * info, codec_setup_info * ci, oggpack_buffer * opb, CAkVorbisAllocator & VorbisAllocator )
{
	memset( info, 0, sizeof( *info ) );
	info->type = oggpack_read( opb, 2 );
	info->begin = oggpack_read( opb, 24 );
	info->end = oggpack_read( opb, 24 );
	info->grouping = oggpack_read( opb, 24 ) + 1;
	info->partitions = (char)( oggpack_read( opb, 6 ) + 1 );
	info->groupbook = (unsigned char)oggpack_read( opb, 8 );
	if( info->groupbook >= ci->books )
	{
		return 1;
	}
	info->stagemasks = (unsigned char *)VorbisAllocator.Alloc( info->partitions );
	info->stagebooks = (unsigned char *)VorbisAllocator.Alloc( info->partitions * 8 );
	for( int j = 0; j < info->partitions; ++j )
	{
		int cascade = oggpack_read( opb, 3 );
		if( oggpack_read( opb, 1 ) )
		{
			cascade |= oggpack_read( opb, 5 ) << 3;
		}
		info->stagemasks[ j ] = (unsigned char)cascade;
	}
	for( int j = 0; j < info->partitions; ++j )
	{
		for( int k = 0; k < 8; ++k )
		{
			if( ( info->stagemasks[ j ] >> k ) & 1 )
			{
				const unsigned char book = (unsigned char)oggpack_read( opb, 8 );
				if( book >= ci->books )
				{
					return 1;
				}
				info->stagebooks[ j * 8 + k ] = book;
				if( k + 1 > info->stages )
				{
					info->stages = (char)( k + 1 );
				}
			}
			else
			{
				info->stagebooks[ j * 8 + k ] = 0xff;
			}
		}
	}
	return opb->headend < 0;
}

/** 2012 0x979530; 2013 0x975d10. */
int res_inverse( vorbis_dsp_state * vd, vorbis_info_residue * info, long ** in, int * nonzero, int ch )
{
	codec_setup_info * ci = vd->csi;
	codebook * phrasebook = ci->book_param + info->groupbook;
	int samples_per_partition = info->grouping;
	const int partitions_per_word = phrasebook->dim;
	const int pcmend = ci->blocksizes[ vd->state.W ];

	if( info->type < 2 )
	{
		const int max = pcmend >> 1;
		const int end = ( info->end < max ) ? info->end : max;
		const int n = end - info->begin;
		if( n <= 0 )
		{
			return 0;
		}
		const int partvals = n / samples_per_partition;
		const int partwords = ( partvals + partitions_per_word - 1 ) / partitions_per_word;
		int used = 0;
		for( int i = 0; i < ch; ++i )
		{
			if( nonzero[ i ] )
			{
				in[ used++ ] = in[ i ];
			}
		}
		if( !used )
		{
			return 0;
		}
		char * partword[ 6 ];
		partword[ 0 ] = (char *)_alloca( used * partwords * partitions_per_word );
		for( int j = 1; j < used; ++j )
		{
			partword[ j ] = partword[ 0 ] + j * partwords * partitions_per_word;
		}
		for( int s = 0; s < info->stages; ++s )
		{
			for( int i = 0; i < partvals; )
			{
				if( s == 0 )
				{
					partword[ 0 ][ i + partitions_per_word - 1 ] = 1;
					for( int k = partitions_per_word - 2; k >= 0; --k )
					{
						partword[ 0 ][ i + k ] = (char)( partword[ 0 ][ i + k + 1 ] * info->partitions );
					}
					for( int j = 1; j < used; ++j )
					{
						for( int k = partitions_per_word - 1; k >= 0; --k )
						{
							partword[ j ][ i + k ] = partword[ j - 1 ][ i + k ];
						}
					}
					for( int j = 0; j < used; ++j )
					{
						AkUInt32 temp = ak_vorbis_book_decode( phrasebook, &vd->opb );
						if( vd->opb.headend < 0 )
						{
							return 0;
						}
						for( int k = 0; k < partitions_per_word; ++k )
						{
							const AkUInt32 div = partword[ j ][ i + k ];
							partword[ j ][ i + k ] = (char)( temp / div );
							temp -= partword[ j ][ i + k ] * div;
						}
					}
				}
				for( int k = 0; k < partitions_per_word && i < partvals; ++k, ++i )
				{
					for( int j = 0; j < used; ++j )
					{
						const int offset = info->begin + i * samples_per_partition;
						const int partition = partword[ j ][ i ];
						if( info->stagemasks[ partition ] & ( 1 << s ) )
						{
							codebook * stagebook = ci->book_param + info->stagebooks[ ( partition << 3 ) + s ];
							vorbis_book_decodev_add( stagebook, in[ j ] + offset, &vd->opb, samples_per_partition, -8 );
						}
					}
				}
			}
		}
		return 0;
	}

	const int max = ( pcmend * ch ) >> 1;
	const int end = ( info->end < max ) ? info->end : max;
	const int n = end - info->begin;
	if( n <= 0 )
	{
		return 0;
	}
	const int partvals = n / samples_per_partition;
	const int partwords = ( partvals + partitions_per_word - 1 ) / partitions_per_word;
	char * partword = (char *)_alloca( partwords * partitions_per_word );
	const int beginoff = info->begin / ch;
	int i;
	for( i = 0; i < ch; ++i )
	{
		if( nonzero[ i ] )
		{
			break;
		}
	}
	if( i == ch )
	{
		return 0;
	}
	samples_per_partition /= ch;
	for( int s = 0; s < info->stages; ++s )
	{
		for( i = 0; i < partvals; )
		{
			if( s == 0 )
			{
				partword[ i + partitions_per_word - 1 ] = 1;
				for( int k = partitions_per_word - 2; k >= 0; --k )
				{
					partword[ i + k ] = (char)( partword[ i + k + 1 ] * info->partitions );
				}
				AkUInt32 temp = ak_vorbis_book_decode( phrasebook, &vd->opb );
				for( int k = 0; k < partitions_per_word; ++k )
				{
					const AkUInt32 div = partword[ i + k ];
					partword[ i + k ] = (char)( temp / div );
					temp -= partword[ i + k ] * div;
				}
			}
			for( int k = 0; k < partitions_per_word && i < partvals; ++k, ++i )
			{
				const int partition = partword[ i ];
				if( info->stagemasks[ partition ] & ( 1 << s ) )
				{
					codebook * stagebook = ci->book_param + info->stagebooks[ ( partition << 3 ) + s ];
					vorbis_book_decodevv_add_2ch( stagebook, in, i * samples_per_partition + beginoff, &vd->opb, samples_per_partition, -8 );
				}
			}
		}
	}
	return 0;
}
