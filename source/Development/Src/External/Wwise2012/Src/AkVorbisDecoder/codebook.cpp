/*=============================================================================
	codebook.cpp - codebook unpacking and Huffman / VQ decoding (Tremor lowmem codebook.c, Audiokinetic's packed variant).

	DISHONORED(port): agent IH2 (wave 26), from the 2012 decompile. Audiokinetic's packed codebook format (the books of
	g_packedCodebookData): dim 4 bits, entries 14 bits, ordered flag; ordered lengths start at 5 bits + 1; unordered
	lengths use a 3-bit length-of-length field; maptype is 1 bit (0 or 1), q_seq is read and ignored, no 24-bit sync
	pattern; dec_type 3 / maptype 2 do not exist. Tables come from CAkVorbisAllocator instead of malloc.

	Retail behaviour kept as is (named): _make_decode_table does not return after the dec_nodeb == 4 table (retail falls
	through, builds the work array a second time and replaces dec_table, 2012 0x979c30 bytes 0x979c3b..0x979c94); the
	only deviation is a 16-byte pad on the work array's _alloca, because retail's __alloca_probe_16 rounding is what makes
	the zero-length array of a one-entry book writable. decode_map ignores q_seq (retail never reads it back).
=============================================================================*/
#include "codec_internal.h"
#include <malloc.h>

/** 2012 0x9799a0; 2013 0x976180. */
static AkUInt32 decpack( long entry, long used_entry, long quantvals, codebook * b, oggpack_buffer * opb, int maptype )
{
	AkUInt32 ret = 0;
	switch( b->dec_type )
	{
	case 0:
		return (AkUInt32)entry;
	case 1:
		if( maptype == 1 )
		{
			for( int j = 0; j < b->dim; ++j )
			{
				const AkUInt32 off = entry % quantvals;
				entry /= quantvals;
				ret |= ( (AkUInt16 *)b->q_val )[ off ] << ( b->q_bits * j );
			}
		}
		else
		{
			for( int j = 0; j < b->dim; ++j )
			{
				ret |= oggpack_read( opb, b->q_bits ) << ( b->q_bits * j );
			}
		}
		return ret;
	case 2:
		for( int j = 0; j < b->dim; ++j )
		{
			const AkUInt32 off = entry % quantvals;
			entry /= quantvals;
			ret |= off << ( b->q_pack * j );
		}
		return ret;
	case 3:
		return (AkUInt32)used_entry;
	}
	return 0;
}

/** 2012 0x979a60; 2013 0x976240. */
static int _make_words( char * l, long n, AkUInt32 * r, long quantvals, codebook * b, oggpack_buffer * opb, int maptype )
{
	long count = 0;
	long top = 0;
	AkUInt32 marker[ 33 ];

	if( n < 2 )
	{
		r[ 0 ] = 0x80000000;
		return 0;
	}
	memset( marker, 0, sizeof( marker ) );
	for( long i = 0; i < n; ++i )
	{
		const long length = l[ i ];
		if( !length )
		{
			continue;
		}
		AkUInt32 entry = marker[ length ];
		long chase = 0;
		if( count && !entry )
		{
			return -1;
		}
		long j;
		for( j = 0; j < length - 1; ++j )
		{
			const int bit = ( entry >> ( length - j - 1 ) ) & 1;
			if( chase >= top )
			{
				++top;
				r[ chase * 2 ] = top;
				r[ chase * 2 + 1 ] = 0;
			}
			else if( !r[ chase * 2 + bit ] )
			{
				r[ chase * 2 + bit ] = top;
			}
			chase = r[ chase * 2 + bit ];
		}
		{
			const int bit = ( entry >> ( length - j - 1 ) ) & 1;
			if( chase >= top )
			{
				++top;
				r[ chase * 2 + 1 ] = 0;
			}
			r[ chase * 2 + bit ] = decpack( i, count++, quantvals, b, opb, maptype ) | 0x80000000;
		}
		for( j = length; j > 0; --j )
		{
			if( marker[ j ] & 1 )
			{
				marker[ j ] = marker[ j - 1 ] << 1;
				break;
			}
			++marker[ j ];
		}
		for( j = length + 1; j < 33; ++j )
		{
			if( ( marker[ j ] >> 1 ) == entry )
			{
				entry = marker[ j ];
				marker[ j ] = marker[ j - 1 ] << 1;
			}
			else
			{
				break;
			}
		}
	}
	return 0;
}

/** 2012 0x979c30; 2013 0x976410. */
static void _make_decode_table( codebook * s, CAkVorbisAllocator & VorbisAllocator, char * lengthlist, long quantvals, oggpack_buffer * opb, int maptype )
{
	if( s->dec_nodeb == 4 )
	{
		s->dec_table = VorbisAllocator.Alloc( ( s->used_entries * 2 + 1 ) * sizeof( AkUInt32 ) );
		_make_words( lengthlist, s->entries, (AkUInt32 *)s->dec_table, quantvals, s, opb, maptype );
	}

	AkUInt32 * work = (AkUInt32 *)_alloca( ( s->used_entries * 2 - 2 ) * sizeof( AkUInt32 ) + 16 );
	_make_words( lengthlist, s->entries, work, quantvals, s, opb, maptype );
	s->dec_table = VorbisAllocator.Alloc( ( s->used_entries * ( s->dec_leafw + 1 ) - 2 ) * s->dec_nodeb );

	if( s->dec_leafw == 1 )
	{
		switch( s->dec_nodeb )
		{
		case 1:
			for( long i = 0; i < s->used_entries * 2 - 2; ++i )
			{
				( (unsigned char *)s->dec_table )[ i ] = (unsigned char)( ( ( work[ i ] & 0x80000000UL ) >> 24 ) | work[ i ] );
			}
			break;
		case 2:
			for( long i = 0; i < s->used_entries * 2 - 2; ++i )
			{
				( (AkUInt16 *)s->dec_table )[ i ] = (AkUInt16)( ( ( work[ i ] & 0x80000000UL ) >> 16 ) | work[ i ] );
			}
			break;
		}
		return;
	}

	long top = s->used_entries * 3 - 2;
	if( s->dec_nodeb == 1 )
	{
		unsigned char * out = (unsigned char *)s->dec_table;
		for( long i = s->used_entries * 2 - 4; i >= 0; i -= 2 )
		{
			if( work[ i ] & 0x80000000UL )
			{
				if( work[ i + 1 ] & 0x80000000UL )
				{
					top -= 4;
					out[ top ] = (unsigned char)( ( work[ i ] >> 8 ) | 0x80 );
					out[ top + 1 ] = (unsigned char)( ( work[ i + 1 ] >> 8 ) | 0x80 );
					out[ top + 2 ] = (unsigned char)work[ i ];
					out[ top + 3 ] = (unsigned char)work[ i + 1 ];
				}
				else
				{
					top -= 3;
					out[ top ] = (unsigned char)( ( work[ i ] >> 8 ) | 0x80 );
					out[ top + 1 ] = (unsigned char)work[ work[ i + 1 ] * 2 ];
					out[ top + 2 ] = (unsigned char)work[ i ];
				}
			}
			else
			{
				if( work[ i + 1 ] & 0x80000000UL )
				{
					top -= 3;
					out[ top ] = (unsigned char)work[ work[ i ] * 2 ];
					out[ top + 1 ] = (unsigned char)( ( work[ i + 1 ] >> 8 ) | 0x80 );
					out[ top + 2 ] = (unsigned char)work[ i + 1 ];
				}
				else
				{
					top -= 2;
					out[ top ] = (unsigned char)work[ work[ i ] * 2 ];
					out[ top + 1 ] = (unsigned char)work[ work[ i + 1 ] * 2 ];
				}
			}
			work[ i ] = top;
		}
	}
	else
	{
		AkUInt16 * out = (AkUInt16 *)s->dec_table;
		for( long i = s->used_entries * 2 - 4; i >= 0; i -= 2 )
		{
			if( work[ i ] & 0x80000000UL )
			{
				if( work[ i + 1 ] & 0x80000000UL )
				{
					top -= 4;
					out[ top ] = (AkUInt16)( ( work[ i ] >> 16 ) | 0x8000 );
					out[ top + 1 ] = (AkUInt16)( ( work[ i + 1 ] >> 16 ) | 0x8000 );
					out[ top + 2 ] = (AkUInt16)work[ i ];
					out[ top + 3 ] = (AkUInt16)work[ i + 1 ];
				}
				else
				{
					top -= 3;
					out[ top ] = (AkUInt16)( ( work[ i ] >> 16 ) | 0x8000 );
					out[ top + 1 ] = (AkUInt16)work[ work[ i + 1 ] * 2 ];
					out[ top + 2 ] = (AkUInt16)work[ i ];
				}
			}
			else
			{
				if( work[ i + 1 ] & 0x80000000UL )
				{
					top -= 3;
					out[ top ] = (AkUInt16)work[ work[ i ] * 2 ];
					out[ top + 1 ] = (AkUInt16)( ( work[ i + 1 ] >> 16 ) | 0x8000 );
					out[ top + 2 ] = (AkUInt16)work[ i + 1 ];
				}
				else
				{
					top -= 2;
					out[ top ] = (AkUInt16)work[ work[ i ] * 2 ];
					out[ top + 1 ] = (AkUInt16)work[ work[ i + 1 ] * 2 ];
				}
			}
			work[ i ] = top;
		}
	}
}

/** 2012 0x979f70; 2013 0x976750. */
static long _book_maptype1_quantvals( codebook * b )
{
	int bits = 0;
	for( AkUInt32 v = b->entries; v; v >>= 1 )
	{
		++bits;
	}
	long vals = b->entries >> ( ( bits - 1 ) * ( b->dim - 1 ) / b->dim );
	for( ;; )
	{
		long acc = 1;
		long acc1 = 1;
		for( int i = 0; i < b->dim; ++i )
		{
			acc *= vals;
			acc1 *= vals + 1;
		}
		if( acc <= b->entries && acc1 > b->entries )
		{
			return vals;
		}
		if( acc > b->entries )
		{
			--vals;
		}
		else
		{
			++vals;
		}
	}
}

static int _ilog( AkUInt32 v )
{
	int ret = 0;
	while( v )
	{
		++ret;
		v >>= 1;
	}
	return ret;
}

static int _determine_node_bytes( long used, int leafwidth )
{
	if( used < 2 )
	{
		return 4;
	}
	if( leafwidth == 3 )
	{
		leafwidth = 4;
	}
	if( _ilog( 3 * used - 6 ) + 1 <= leafwidth * 4 )
	{
		return leafwidth / 2 ? leafwidth / 2 : 1;
	}
	return leafwidth;
}

static int _determine_leaf_words( int nodeb, int leafwidth )
{
	return leafwidth > nodeb ? 2 : 1;
}

static long _float32_unpack( long val, int * point )
{
	long mant = val & 0x1fffff;
	const AkUInt32 sign = val & 0x80000000;
	*point = ( ( val & 0x7fe00000L ) >> 21 ) - 788;
	if( mant )
	{
		while( !( mant & 0x40000000 ) )
		{
			mant <<= 1;
			*point -= 1;
		}
		if( sign )
		{
			mant = -mant;
		}
	}
	else
	{
		*point = -9999;
	}
	return mant;
}

/**
 * 2012 0x979fe0; 2013 0x9767c0. _determine_node_bytes / _determine_leaf_words / _float32_unpack / _ilog are inlined in
 * retail (no symbols); they are kept as the Tremor helpers they were compiled from.
 */
void vorbis_book_unpack( oggpack_buffer * opb, codebook * s, CAkVorbisAllocator & VorbisAllocator )
{
	long quantvals = 0;
	memset( s, 0, sizeof( *s ) );
	s->dim = oggpack_read( opb, 4 );
	s->entries = oggpack_read( opb, 14 );
	char * lengthlist = (char *)_alloca( s->entries );

	if( oggpack_read( opb, 1 ) )
	{
		long length = oggpack_read( opb, 5 ) + 1;
		s->used_entries = s->entries;
		for( long i = 0; i < s->entries; )
		{
			const long num = oggpack_read( opb, _ilog( s->entries - i ) );
			for( long j = 0; j < num && i < s->entries; ++j, ++i )
			{
				lengthlist[ i ] = (char)length;
			}
			s->dec_maxlength = length;
			++length;
		}
	}
	else
	{
		const int lengthbits = oggpack_read( opb, 3 );
		if( oggpack_read( opb, 1 ) )
		{
			for( long i = 0; i < s->entries; ++i )
			{
				if( oggpack_read( opb, 1 ) )
				{
					const long num = oggpack_read( opb, lengthbits );
					lengthlist[ i ] = (char)( num + 1 );
					++s->used_entries;
					if( num + 1 > s->dec_maxlength )
					{
						s->dec_maxlength = num + 1;
					}
				}
				else
				{
					lengthlist[ i ] = 0;
				}
			}
		}
		else
		{
			s->used_entries = s->entries;
			for( long i = 0; i < s->entries; ++i )
			{
				const long num = oggpack_read( opb, lengthbits );
				lengthlist[ i ] = (char)( num + 1 );
				if( num + 1 > s->dec_maxlength )
				{
					s->dec_maxlength = num + 1;
				}
			}
		}
	}

	const int maptype = oggpack_read( opb, 1 );
	if( !maptype )
	{
		s->dec_nodeb = _determine_node_bytes( s->used_entries, _ilog( s->entries ) / 8 + 1 );
		s->dec_leafw = _determine_leaf_words( s->dec_nodeb, _ilog( s->entries ) / 8 + 1 );
		s->dec_type = 0;
		_make_decode_table( s, VorbisAllocator, lengthlist, quantvals, opb, maptype );
		return;
	}

	s->q_min = _float32_unpack( oggpack_read( opb, 32 ), &s->q_minp );
	s->q_del = _float32_unpack( oggpack_read( opb, 32 ), &s->q_delp );
	s->q_bits = oggpack_read( opb, 4 ) + 1;
	oggpack_read( opb, 1 );
	s->q_del >>= s->q_bits;
	s->q_delp += s->q_bits;

	quantvals = _book_maptype1_quantvals( s );
	const long total1 = ( s->q_bits * s->dim + 8 ) / 8;
	const long total2 = ( _ilog( quantvals - 1 ) * s->dim + 8 ) / 8 + ( s->q_bits + 7 ) / 8;
	if( total1 <= 4 && total1 <= total2 )
	{
		s->q_val = _alloca( sizeof( AkUInt16 ) * quantvals );
		for( long i = 0; i < quantvals; ++i )
		{
			( (AkUInt16 *)s->q_val )[ i ] = (AkUInt16)oggpack_read( opb, s->q_bits );
		}
		s->dec_type = 1;
		s->dec_nodeb = _determine_node_bytes( s->used_entries, ( s->q_bits * s->dim + 8 ) / 8 );
		s->dec_leafw = _determine_leaf_words( s->dec_nodeb, ( s->q_bits * s->dim + 8 ) / 8 );
		_make_decode_table( s, VorbisAllocator, lengthlist, quantvals, opb, maptype );
		s->q_val = NULL;
	}
	else
	{
		if( s->q_bits <= 8 )
		{
			s->q_val = VorbisAllocator.Alloc( quantvals );
			for( long i = 0; i < quantvals; ++i )
			{
				( (unsigned char *)s->q_val )[ i ] = (unsigned char)oggpack_read( opb, s->q_bits );
			}
		}
		else
		{
			s->q_val = VorbisAllocator.Alloc( quantvals * 2 );
			for( long i = 0; i < quantvals; ++i )
			{
				( (AkUInt16 *)s->q_val )[ i ] = (AkUInt16)oggpack_read( opb, s->q_bits );
			}
		}
		s->q_pack = _ilog( quantvals - 1 );
		s->dec_type = 2;
		s->dec_nodeb = _determine_node_bytes( s->used_entries, ( _ilog( quantvals - 1 ) * s->dim + 8 ) / 8 );
		s->dec_leafw = _determine_leaf_words( s->dec_nodeb, ( _ilog( quantvals - 1 ) * s->dim + 8 ) / 8 );
		_make_decode_table( s, VorbisAllocator, lengthlist, quantvals, opb, maptype );
	}
}

/** 2012 0x97a580; 2013 0x976d60. No look-ahead shortening and no end-of-packet test: retail always advances i + 1 bits. */
static AkUInt32 decode_packed_entry_number( codebook * book, oggpack_buffer * b )
{
	AkUInt32 chase = 0;
	const int read = book->dec_maxlength;
	const long lok = oggpack_look( b, read );
	int i = 0;

	if( book->dec_nodeb == 1 )
	{
		const unsigned char * t = (const unsigned char *)book->dec_table;
		if( book->dec_leafw == 1 )
		{
			for( i = 0; i < read; ++i )
			{
				chase = t[ chase * 2 + ( ( lok >> i ) & 1 ) ];
				if( chase & 0x80UL )
				{
					break;
				}
			}
			chase &= 0x7fUL;
		}
		else
		{
			for( i = 0; i < read; ++i )
			{
				const int bit = ( lok >> i ) & 1;
				const int next = t[ chase + bit ];
				if( next & 0x80 )
				{
					chase = ( next << 8 ) | t[ chase + bit + 1 + ( !bit || ( t[ chase ] & 0x80 ) ) ];
					break;
				}
				chase = next;
			}
			chase &= 0x7fffUL;
		}
	}
	else if( book->dec_nodeb == 2 )
	{
		const AkUInt16 * t = (const AkUInt16 *)book->dec_table;
		if( book->dec_leafw == 1 )
		{
			for( i = 0; i < read; ++i )
			{
				chase = t[ chase * 2 + ( ( lok >> i ) & 1 ) ];
				if( chase & 0x8000UL )
				{
					break;
				}
			}
			chase &= 0x7fffUL;
		}
		else
		{
			for( i = 0; i < read; ++i )
			{
				const int bit = ( lok >> i ) & 1;
				const int next = t[ chase + bit ];
				if( next & 0x8000 )
				{
					chase = ( next << 16 ) | t[ chase + bit + 1 + ( !bit || ( t[ chase ] & 0x8000 ) ) ];
					break;
				}
				chase = next;
			}
			chase &= 0x7fffffffUL;
		}
	}
	else
	{
		const AkUInt32 * t = (const AkUInt32 *)book->dec_table;
		for( i = 0; i < read; ++i )
		{
			chase = t[ chase * 2 + ( ( lok >> i ) & 1 ) ];
			if( chase & 0x80000000UL )
			{
				break;
			}
		}
		chase &= 0x7fffffffUL;
	}
	oggpack_adv( b, i + 1 );
	return chase;
}

/** 2012 0x97a730; 2013 0x976f10. */
long ak_vorbis_book_decode( codebook * book, oggpack_buffer * b )
{
	if( book->dec_type )
	{
		return -1;
	}
	return decode_packed_entry_number( book, b );
}

/** 2012 0x97a750; 2013 0x976f30. */
void decode_map( codebook * s, oggpack_buffer * b, long * v, int point )
{
	AkUInt32 entry = decode_packed_entry_number( s, b );
	if( s->dec_type == 1 )
	{
		const int bits = s->q_bits;
		for( int i = 0; i < s->dim; ++i )
		{
			v[ i ] = entry & ( ( 1 << bits ) - 1 );
			entry >>= bits;
		}
	}
	else
	{
		const int pack = s->q_pack;
		const AkUInt32 packMask = ( 1 << pack ) - 1;
		if( s->q_bits > 8 )
		{
			for( int i = 0; i < s->dim; ++i )
			{
				v[ i ] = ( (AkUInt16 *)s->q_val )[ entry & packMask ];
				entry >>= pack;
			}
		}
		else
		{
			for( int i = 0; i < s->dim; ++i )
			{
				v[ i ] = ( (unsigned char *)s->q_val )[ entry & packMask ];
				entry >>= pack;
			}
		}
	}

	const int shiftM = point - s->q_delp;
	long add = point - s->q_minp;
	if( add > 0 )
	{
		add = s->q_min >> add;
	}
	else
	{
		add = s->q_min << -add;
	}
	if( shiftM > 0 )
	{
		for( int i = 0; i < s->dim; ++i )
		{
			v[ i ] = add + ( ( v[ i ] * s->q_del ) >> shiftM );
		}
	}
	else
	{
		for( int i = 0; i < s->dim; ++i )
		{
			v[ i ] = add + ( ( v[ i ] * s->q_del ) << -shiftM );
		}
	}
}

/** 2012 0x97a870; 2013 0x977050. */
void vorbis_book_decodev_add( codebook * book, long * a, oggpack_buffer * b, int n, int point )
{
	long v[ 8 ];
	for( int i = 0; i < n; )
	{
		decode_map( book, b, v, point );
		for( int j = 0; j < book->dim; ++j )
		{
			a[ i++ ] += v[ j ];
		}
	}
}

/** 2012 0x97a8d0; 2013 0x9770b0. The two-channel case of Tremor's decodevv_add (residue type 2 is always stereo here). */
void vorbis_book_decodevv_add_2ch( codebook * book, long ** a, long offset, oggpack_buffer * b, int n, int point )
{
	long v[ 8 ];
	int chptr = 0;
	for( long i = offset; i < offset + n; )
	{
		decode_map( book, b, v, point );
		for( int j = 0; j < book->dim; ++j )
		{
			a[ chptr ][ i ] += v[ j ];
			i += chptr & 1;
			chptr = ( chptr - 1 ) & 1;
		}
	}
}
