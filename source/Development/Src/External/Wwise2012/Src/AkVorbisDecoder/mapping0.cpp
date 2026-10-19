/*=============================================================================
	mapping0.cpp - mapping type 0 (Tremor lowmem mapping0.c): channel coupling, submaps, and the per-packet inverse.

	DISHONORED(port): agent IH2 (wave 26), from the 2012 decompile. Retail differences kept: no mapping type field, floor
	type 1 only, tables from CAkVorbisAllocator, chmuxlist only allocated (and read) when there are several submaps, the
	submap's unused time-config byte is read and dropped.
=============================================================================*/
#include "codec_internal.h"
#include <malloc.h>

/** 2012 0x977fe0; 2013 0x9747c0. */
static int ilog( AkUInt32 v )
{
	int ret = 0;
	if( v )
	{
		for( --v; v; v >>= 1 )
		{
			++ret;
		}
	}
	return ret;
}

/** 2012 0x978000; 2013 0x9747e0. */
int mapping_info_unpack( vorbis_info_mapping * info, codec_setup_info * ci, int channels, oggpack_buffer * opb, CAkVorbisAllocator & VorbisAllocator )
{
	memset( info, 0, sizeof( *info ) );
	if( oggpack_read( opb, 1 ) )
	{
		info->submaps = oggpack_read( opb, 4 ) + 1;
	}
	else
	{
		info->submaps = 1;
	}

	if( oggpack_read( opb, 1 ) )
	{
		info->coupling_steps = oggpack_read( opb, 8 ) + 1;
		info->coupling = (coupling_step *)VorbisAllocator.Alloc( info->coupling_steps * sizeof( coupling_step ) );
		const int bits = ilog( channels );
		for( int i = 0; i < info->coupling_steps; ++i )
		{
			const int testM = info->coupling[ i ].mag = (unsigned char)oggpack_read( opb, bits );
			const int testA = info->coupling[ i ].ang = (unsigned char)oggpack_read( opb, bits );
			if( testM == testA || testM >= channels || testA >= channels )
			{
				return -1;
			}
		}
	}

	if( oggpack_read( opb, 2 ) > 0 )
	{
		return -1;
	}

	if( info->submaps > 1 )
	{
		info->chmuxlist = (unsigned char *)VorbisAllocator.Alloc( channels );
		for( int i = 0; i < channels; ++i )
		{
			info->chmuxlist[ i ] = (unsigned char)oggpack_read( opb, 4 );
			if( info->chmuxlist[ i ] >= info->submaps )
			{
				return -1;
			}
		}
	}

	info->submaplist = (submap *)VorbisAllocator.Alloc( info->submaps * sizeof( submap ) );
	for( int i = 0; i < info->submaps; ++i )
	{
		oggpack_read( opb, 8 );
		info->submaplist[ i ].floor = (char)oggpack_read( opb, 8 );
		if( info->submaplist[ i ].floor >= ci->floors )
		{
			return -1;
		}
		info->submaplist[ i ].residue = (char)oggpack_read( opb, 8 );
		if( info->submaplist[ i ].residue >= ci->residues )
		{
			return -1;
		}
	}
	return 0;
}

/** 2012 0x978220; 2013 0x974a00. */
int mapping_inverse( vorbis_dsp_state * vd, vorbis_info_mapping * info )
{
	codec_setup_info * ci = vd->csi;
	const long n = ci->blocksizes[ vd->state.W ];
	long ** pcmbundle = (long **)_alloca( sizeof( *pcmbundle ) * vd->channels );
	int * zerobundle = (int *)_alloca( sizeof( *zerobundle ) * vd->channels );
	int * nonzero = (int *)_alloca( sizeof( *nonzero ) * vd->channels );
	long ** floormemo = (long **)_alloca( sizeof( *floormemo ) * vd->channels );

	for( int i = 0; i < vd->channels; ++i )
	{
		int submapIndex = 0;
		if( info->submaps > 1 )
		{
			submapIndex = info->chmuxlist[ i ];
		}
		vorbis_info_floor1 * floorInfo = &ci->floor_param[ info->submaplist[ submapIndex ].floor ];
		floormemo[ i ] = (long *)_alloca( sizeof( long ) * floorInfo->posts );
		floormemo[ i ] = floor1_inverse1( vd, floorInfo, floormemo[ i ] );
		nonzero[ i ] = floormemo[ i ] != NULL;
		memset( vd->work[ i ], 0, sizeof( *vd->work[ i ] ) * n / 2 );
	}

	for( int i = 0; i < info->coupling_steps; ++i )
	{
		if( nonzero[ info->coupling[ i ].mag ] || nonzero[ info->coupling[ i ].ang ] )
		{
			nonzero[ info->coupling[ i ].mag ] = 1;
			nonzero[ info->coupling[ i ].ang ] = 1;
		}
	}

	for( int i = 0; i < info->submaps; ++i )
	{
		int ch_in_bundle = 0;
		for( int j = 0; j < vd->channels; ++j )
		{
			if( !info->chmuxlist || info->chmuxlist[ j ] == i )
			{
				zerobundle[ ch_in_bundle ] = nonzero[ j ] != 0;
				pcmbundle[ ch_in_bundle++ ] = (long *)vd->work[ j ];
			}
		}
		res_inverse( vd, &ci->residue_param[ info->submaplist[ i ].residue ], pcmbundle, zerobundle, ch_in_bundle );
	}

	for( int i = info->coupling_steps - 1; i >= 0; --i )
	{
		long * pcmM = (long *)vd->work[ info->coupling[ i ].mag ];
		long * pcmA = (long *)vd->work[ info->coupling[ i ].ang ];
		for( int j = 0; j < n / 2; ++j )
		{
			const long mag = pcmM[ j ];
			const long ang = pcmA[ j ];
			if( mag > 0 )
			{
				if( ang > 0 )
				{
					pcmM[ j ] = mag;
					pcmA[ j ] = mag - ang;
				}
				else
				{
					pcmA[ j ] = mag;
					pcmM[ j ] = mag + ang;
				}
			}
			else if( ang > 0 )
			{
				pcmM[ j ] = mag;
				pcmA[ j ] = mag + ang;
			}
			else
			{
				pcmA[ j ] = mag;
				pcmM[ j ] = mag - ang;
			}
		}
	}

	for( int i = 0; i < vd->channels; ++i )
	{
		int submapIndex = 0;
		if( info->submaps > 1 )
		{
			submapIndex = info->chmuxlist[ i ];
		}
		floor1_inverse2( vd, &ci->floor_param[ info->submaplist[ submapIndex ].floor ], floormemo[ i ], (long *)vd->work[ i ] );
	}

	for( int i = 0; i < vd->channels; ++i )
	{
		mdct_backward( n, (float *)vd->work[ i ] );
	}
	return 0;
}
