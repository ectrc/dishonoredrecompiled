/*=============================================================================
	info.cpp - the setup header of Wwise's packed Vorbis (Tremor info.c: vorbis_info_init + the books / floors / residues /
	maps / modes sections of the setup packet).

	DISHONORED(port): agent IH2 (wave 26), from the 2012 decompile. Wwise's setup packet has no identification / comment
	headers and no packet-type / "vorbis" preamble; the block sizes come from the 'vorb' chunk. Each codebook is a 10-bit
	index into g_packedCodebooks, unpacked from the static library (headend 874, retail's constant). Only floor type 1
	exists (no floor type field), and the time-domain section is absent. Retail's error code is -133 (OV_EBADHEADER).
=============================================================================*/
#include "codec_internal.h"

/** 2012 0x9785f0; 2013 0x974dd0. */
int vorbis_info_init( codec_setup_info * csi, long in_blocksize0, long in_blocksize1 )
{
	memset( csi, 0, sizeof( *csi ) );
	csi->blocksizes[ 0 ] = 1 << in_blocksize0;
	csi->blocksizes[ 1 ] = 1 << in_blocksize1;
	if( csi->blocksizes[ 0 ] >= 64 && csi->blocksizes[ 1 ] >= csi->blocksizes[ 0 ] && csi->blocksizes[ 1 ] <= 8192 )
	{
		return 0;
	}
	return -133;
}

/** 2012 0x978640; 2013 0x974e20. */
int vorbis_unpack_books( codec_setup_info * ci, int channels, oggpack_buffer * opb, CAkVorbisAllocator & VorbisAllocator )
{
	ci->books = oggpack_read( opb, 8 ) + 1;
	ci->book_param = (codebook *)VorbisAllocator.Calloc( sizeof( codebook ) * ci->books );
	for( int i = 0; i < ci->books; ++i )
	{
		oggpack_buffer opbBook;
		opbBook.headptr = (unsigned char *)g_packedCodebooks[ oggpack_read( opb, 10 ) ];
		opbBook.headbit = 0;
		opbBook.headend = 874;
		vorbis_book_unpack( &opbBook, &ci->book_param[ i ], VorbisAllocator );
	}

	ci->floors = oggpack_read( opb, 6 ) + 1;
	ci->floor_param = (vorbis_info_floor1 *)VorbisAllocator.Calloc( sizeof( vorbis_info_floor1 ) * ci->floors );
	for( int i = 0; i < ci->floors; ++i )
	{
		if( floor1_info_unpack( &ci->floor_param[ i ], ci, opb, VorbisAllocator ) )
		{
			return -133;
		}
	}

	ci->residues = oggpack_read( opb, 6 ) + 1;
	ci->residue_param = (vorbis_info_residue *)VorbisAllocator.Alloc( sizeof( vorbis_info_residue ) * ci->residues );
	for( int i = 0; i < ci->residues; ++i )
	{
		if( res_unpack( &ci->residue_param[ i ], ci, opb, VorbisAllocator ) )
		{
			return -133;
		}
	}

	ci->maps = oggpack_read( opb, 6 ) + 1;
	ci->map_param = (vorbis_info_mapping *)VorbisAllocator.Alloc( sizeof( vorbis_info_mapping ) * ci->maps );
	for( int i = 0; i < ci->maps; ++i )
	{
		if( mapping_info_unpack( &ci->map_param[ i ], ci, channels, opb, VorbisAllocator ) )
		{
			return -133;
		}
	}

	ci->modes = oggpack_read( opb, 6 ) + 1;
	ci->mode_param = (vorbis_info_mode *)VorbisAllocator.Alloc( sizeof( vorbis_info_mode ) * ci->modes );
	for( int i = 0; i < ci->modes; ++i )
	{
		ci->mode_param[ i ].blockflag = (unsigned char)oggpack_read( opb, 1 );
		ci->mode_param[ i ].mapping = (unsigned char)oggpack_read( opb, 8 );
		if( ci->mode_param[ i ].mapping >= ci->maps )
		{
			return -133;
		}
	}
	return 0;
}
