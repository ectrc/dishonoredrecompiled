/*=============================================================================
	floor1.cpp - floor type 1 (Tremor lowmem floor1.c), with Audiokinetic's float spectral envelope.

	DISHONORED(port): agent IH2 (wave 26), from the 2012 decompile. Differences from Tremor that retail has (kept): the
	floor tables come from CAkVorbisAllocator; render_line multiplies the fixed-point residue by fFLOOR_fromdB_LOOKUP and
	stores the product as a float in place (the work vector is float from here on, mdct_backward takes it as float *);
	render_line has no n clip and floor1_inverse2 has no trailing "be certain" loop. Deviation (named): floor1_inverse1
	reads quant_look[mult - 1] where retail reads mask[mult + 32] (the same four values, see AkVorbisTables.cpp).
=============================================================================*/
#include "codec_internal.h"
#include <malloc.h>
#include <stdlib.h>

/** 2012 0x978900; 2013 0x9750e0. */
static void mergesort( char * index, AkUInt16 * vals, AkUInt16 n )
{
	char * A = index;
	char * B = (char *)_alloca( n );
	AkUInt16 j = 0;
	for( AkUInt16 i = 1; i < n; i <<= 1 )
	{
		for( j = 0; j + i < n; )
		{
			int k1 = j;
			const int mid = j + i;
			int k2 = mid;
			const int end = ( j + i * 2 < n ) ? j + i * 2 : n;
			while( k1 < mid && k2 < end )
			{
				if( vals[ A[ k1 ] ] < vals[ A[ k2 ] ] )
				{
					B[ j++ ] = A[ k1++ ];
				}
				else
				{
					B[ j++ ] = A[ k2++ ];
				}
			}
			while( k1 < mid )
			{
				B[ j++ ] = A[ k1++ ];
			}
			while( k2 < end )
			{
				B[ j++ ] = A[ k2++ ];
			}
		}
		for( ; j < n; ++j )
		{
			B[ j ] = A[ j ];
		}
		char * temp = A;
		A = B;
		B = temp;
	}
	if( B == index )
	{
		for( j = 0; j < n; ++j )
		{
			B[ j ] = A[ j ];
		}
	}
}

/** 2012 0x978a70; 2013 0x975250. */
int floor1_info_unpack( vorbis_info_floor1 * info, codec_setup_info * ci, oggpack_buffer * opb, CAkVorbisAllocator & VorbisAllocator )
{
	int maxclass = -1;
	info->partitions = oggpack_read( opb, 5 );
	info->partitionclass = (char *)VorbisAllocator.Alloc( info->partitions );
	for( int j = 0; j < info->partitions; ++j )
	{
		info->partitionclass[ j ] = (char)oggpack_read( opb, 4 );
		if( maxclass < info->partitionclass[ j ] )
		{
			maxclass = info->partitionclass[ j ];
		}
	}

	info->Class = (floor1class *)VorbisAllocator.Alloc( sizeof( floor1class ) * ( maxclass + 1 ) );
	for( int j = 0; j < maxclass + 1; ++j )
	{
		info->Class[ j ].class_dim = (char)( oggpack_read( opb, 3 ) + 1 );
		info->Class[ j ].class_subs = (char)oggpack_read( opb, 2 );
		if( opb->headend < 0 )
		{
			return -1;
		}
		if( info->Class[ j ].class_subs )
		{
			info->Class[ j ].class_book = (unsigned char)oggpack_read( opb, 8 );
			if( info->Class[ j ].class_book >= ci->books )
			{
				return -1;
			}
		}
		else
		{
			info->Class[ j ].class_book = 0;
		}
		if( info->Class[ j ].class_book >= ci->books )
		{
			return -1;
		}
		for( int k = 0; k < ( 1 << info->Class[ j ].class_subs ); ++k )
		{
			info->Class[ j ].class_subbook[ k ] = (unsigned char)( oggpack_read( opb, 8 ) - 1 );
			if( info->Class[ j ].class_subbook[ k ] >= ci->books && info->Class[ j ].class_subbook[ k ] != 0xff )
			{
				return -1;
			}
		}
	}

	info->mult = oggpack_read( opb, 2 ) + 1;
	const int rangebits = oggpack_read( opb, 4 );
	int count = 0;
	for( int j = 0; j < info->partitions; ++j )
	{
		count += info->Class[ info->partitionclass[ j ] ].class_dim;
	}
	info->postlist = (AkUInt16 *)VorbisAllocator.Alloc( ( count + 2 ) * sizeof( AkUInt16 ) );
	info->forward_index = (char *)VorbisAllocator.Alloc( count + 2 );
	info->loneighbor = (char *)VorbisAllocator.Alloc( count );
	info->hineighbor = (char *)VorbisAllocator.Alloc( count );

	count = 0;
	for( int j = 0, k = 0; j < info->partitions; ++j )
	{
		count += info->Class[ info->partitionclass[ j ] ].class_dim;
		for( ; k < count; ++k )
		{
			info->postlist[ k + 2 ] = (AkUInt16)oggpack_read( opb, rangebits );
			if( info->postlist[ k + 2 ] >= ( 1 << rangebits ) )
			{
				return -1;
			}
		}
	}
	if( opb->headend < 0 )
	{
		return -1;
	}
	info->postlist[ 0 ] = 0;
	info->postlist[ 1 ] = (AkUInt16)( 1 << rangebits );
	info->posts = count + 2;

	for( int j = 0; j < info->posts; ++j )
	{
		info->forward_index[ j ] = (char)j;
	}
	mergesort( info->forward_index, info->postlist, (AkUInt16)info->posts );

	for( int j = 0; j < info->posts - 2; ++j )
	{
		int lo = 0;
		int hi = 1;
		int lx = 0;
		int hx = info->postlist[ 1 ];
		const int currentx = info->postlist[ j + 2 ];
		for( int k = 0; k < j + 2; ++k )
		{
			const int x = info->postlist[ k ];
			if( x > lx && x < currentx )
			{
				lo = k;
				lx = x;
			}
			if( x < hx && x > currentx )
			{
				hi = k;
				hx = x;
			}
		}
		info->loneighbor[ j ] = (char)lo;
		info->hineighbor[ j ] = (char)hi;
	}
	return 0;
}

/** 2012 0x978eb0; 2013 0x975690. Writes float products over the long residue in place. */
static void render_line( int x0, int x1, int y0, int y1, long * d )
{
	const int dy = y1 - y0;
	const int adx = x1 - x0;
	const int base = dy / adx;
	const int sy = ( dy >= 0 ) ? base + 1 : base - 1;
	const int ady = abs( dy ) - abs( base * adx );
	float * f = (float *)d;
	int y = y0;
	int err = 0;
	f[ x0 ] = (float)d[ x0 ] * fFLOOR_fromdB_LOOKUP[ y ];
	for( int x = x0 + 1; x < x1; ++x )
	{
		err += ady;
		if( err < adx )
		{
			y += base;
		}
		else
		{
			err -= adx;
			y += sy;
		}
		f[ x ] = (float)d[ x ] * fFLOOR_fromdB_LOOKUP[ y ];
	}
}

static int render_point( int x0, int x1, int y0, int y1, int x )
{
	y0 &= 0x7fff;
	y1 &= 0x7fff;
	const int dy = y1 - y0;
	const int adx = x1 - x0;
	const int ady = abs( dy );
	const int off = ( ady * ( x - x0 ) ) / adx;
	return ( dy < 0 ) ? y0 - off : y0 + off;
}

static int ilog( AkUInt32 v )
{
	int ret = 0;
	while( v )
	{
		++ret;
		v >>= 1;
	}
	return ret;
}

/** 2012 0x979050; 2013 0x975830. render_point / ilog are inlined in retail. */
long * floor1_inverse1( vorbis_dsp_state * vd, vorbis_info_floor1 * info, long * fit_value )
{
	codebook * books = vd->csi->book_param;
	const int quant_q = quant_look[ info->mult - 1 ];

	if( oggpack_read( &vd->opb, 1 ) != 1 )
	{
		return NULL;
	}
	fit_value[ 0 ] = oggpack_read( &vd->opb, ilog( quant_q - 1 ) );
	fit_value[ 1 ] = oggpack_read( &vd->opb, ilog( quant_q - 1 ) );

	for( int i = 0, j = 2; i < info->partitions; ++i )
	{
		const int classv = info->partitionclass[ i ];
		const int cdim = info->Class[ classv ].class_dim;
		const int csubbits = info->Class[ classv ].class_subs;
		const int csub = 1 << csubbits;
		int cval = 0;
		if( csubbits )
		{
			cval = ak_vorbis_book_decode( books + info->Class[ classv ].class_book, &vd->opb );
			if( cval == -1 )
			{
				return NULL;
			}
		}
		for( int k = 0; k < cdim; ++k )
		{
			const int book = info->Class[ classv ].class_subbook[ cval & ( csub - 1 ) ];
			cval >>= csubbits;
			if( book != 0xff )
			{
				fit_value[ j + k ] = ak_vorbis_book_decode( books + book, &vd->opb );
				if( fit_value[ j + k ] == -1 )
				{
					return NULL;
				}
			}
			else
			{
				fit_value[ j + k ] = 0;
			}
		}
		j += cdim;
	}

	for( int i = 2; i < info->posts; ++i )
	{
		const int lo = info->loneighbor[ i - 2 ];
		const int hi = info->hineighbor[ i - 2 ];
		const int predicted = render_point( info->postlist[ lo ], info->postlist[ hi ], fit_value[ lo ], fit_value[ hi ], info->postlist[ i ] );
		const int hiroom = quant_q - predicted;
		const int loroom = predicted;
		const int room = ( hiroom < loroom ? hiroom : loroom ) << 1;
		int val = fit_value[ i ];
		if( val )
		{
			if( val >= room )
			{
				if( hiroom > loroom )
				{
					val = val - loroom;
				}
				else
				{
					val = -1 - ( val - hiroom );
				}
			}
			else if( val & 1 )
			{
				val = -( ( val + 1 ) >> 1 );
			}
			else
			{
				val >>= 1;
			}
			fit_value[ i ] = val + predicted;
			fit_value[ lo ] &= 0x7fff;
			fit_value[ hi ] &= 0x7fff;
		}
		else
		{
			fit_value[ i ] = predicted | 0x8000;
		}
	}
	return fit_value;
}

/** 2012 0x9792e0; 2013 0x975ac0. */
int floor1_inverse2( vorbis_dsp_state * vd, vorbis_info_floor1 * info, long * fit_value, long * out )
{
	if( !fit_value )
	{
		memset( out, 0, sizeof( *out ) * ( vd->csi->blocksizes[ vd->state.W ] / 2 ) );
		return 0;
	}
	int lx = 0;
	int ly = fit_value[ 0 ] * info->mult;
	for( int j = 1; j < info->posts; ++j )
	{
		const int current = info->forward_index[ j ];
		int hy = fit_value[ current ] & 0x7fff;
		if( hy == fit_value[ current ] )
		{
			hy *= info->mult;
			const int hx = info->postlist[ current ];
			render_line( lx, hx, ly, hy, out );
			lx = hx;
			ly = hy;
		}
	}
	return 1;
}
