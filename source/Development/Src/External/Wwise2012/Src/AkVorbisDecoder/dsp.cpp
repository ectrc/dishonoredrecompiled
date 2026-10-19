/*=============================================================================
	dsp.cpp - the decoder's per-voice state: work / overlap vectors, packet synthesis, windowed PCM output
	(Tremor lowmem dsp.c with Audiokinetic's extra-samples trimming).

	DISHONORED(port): agent IH2 (wave 26), from the 2012 decompile. Retail differences from Tremor kept: the work and
	right-hand overlap vectors of every channel are one AK::MemoryMgr block from g_LEngineDefaultPoolId (16-byte rounded
	halves); the mode is a 1-bit field; out_begin / out_end are trimmed by extra_samples_begin (seek / loop pre-roll) and,
	on the end-of-stream packet, by extra_samples_end; the first packet after a restart only primes the overlap.
=============================================================================*/
#include "codec_internal.h"

/** 2012 0x9767e0; 2013 0x972fc0. */
int vorbis_dsp_restart( vorbis_dsp_state * v, AkUInt16 in_uExtraSamplesBegin, AkUInt16 in_uExtraSamplesEnd )
{
	v->state.out_end = -1;
	v->state.out_begin = -1;
	v->state.extra_samples_begin = in_uExtraSamplesBegin;
	v->state.extra_samples_end = in_uExtraSamplesEnd;
	return 0;
}

/** 2012 0x976810; 2013 0x972ff0. */
int vorbis_dsp_init( vorbis_dsp_state * v, int channels )
{
	v->channels = channels;
	v->work[ 0 ] = NULL;
	const int blocksize1 = v->csi->blocksizes[ 1 ];
	const AkUInt32 uWorkSize = ( 4 * channels * ( blocksize1 >> 1 ) + 15 ) & ~15;
	const AkUInt32 uRightSize = ( 4 * channels * ( blocksize1 >> 2 ) + 15 ) & ~15;
	if( !( uWorkSize + uRightSize ) )
	{
		return -1;
	}
	unsigned char * pBlock = (unsigned char *)AK::MemoryMgr::Malloc( g_LEngineDefaultPoolId, uWorkSize + uRightSize );
	if( !pBlock )
	{
		return -1;
	}
	unsigned char * pRight = pBlock + uWorkSize;
	memset( pBlock, 0, uWorkSize + uRightSize );
	for( int i = 0; i < channels; ++i )
	{
		v->work[ i ] = (int *)( pBlock + i * ( uWorkSize / channels ) );
		v->mdctright[ i ] = (int *)( pRight + i * ( uRightSize / channels ) );
	}
	v->state.lW = 0;
	v->state.W = 0;
	return 0;
}

/** 2012 0x9768d0; 2013 0x9730b0. */
void vorbis_dsp_clear( vorbis_dsp_state * v )
{
	if( v->work[ 0 ] )
	{
		AK::MemoryMgr::Free( g_LEngineDefaultPoolId, v->work[ 0 ] );
		v->work[ 0 ] = NULL;
	}
}

/** 2012 0x976900; 2013 0x9730e0. */
static const float * _vorbis_window( int left )
{
	switch( left )
	{
	case 128:
		return vwin256;
	case 256:
		return vwin512;
	case 512:
		return vwin1024;
	case 1024:
		return vwin2048;
	case 2048:
		return vwin4096;
	}
	return NULL;
}

/** 2012 0x976950; 2013 0x973130. */
int vorbis_dsp_pcmout( vorbis_dsp_state * v, short * pcm, int samples )
{
	if( v->state.out_begin >= v->state.out_end )
	{
		return 0;
	}
	int n = v->state.out_end - v->state.out_begin;
	if( pcm )
	{
		codec_setup_info * ci = v->csi;
		if( n > samples )
		{
			n = samples;
		}
		const float * Window0 = _vorbis_window( ci->blocksizes[ 0 ] >> 1 );
		const float * Window1 = _vorbis_window( ci->blocksizes[ 1 ] >> 1 );
		for( int i = 0; i < v->channels; ++i )
		{
			mdct_unroll_lap( ci->blocksizes[ 0 ], ci->blocksizes[ 1 ], v->state.lW, v->state.W, (float *)v->work[ i ], (float *)v->mdctright[ i ],
				Window0, Window1, pcm + i, v->channels, v->state.out_begin, v->state.out_begin + n );
		}
		v->state.out_begin += n;
	}
	return n;
}

/** 2012 0x976a20; 2013 0x973200. */
void vorbis_dsp_synthesis( vorbis_dsp_state * vd, ogg_packet * op )
{
	codec_setup_info * ci = vd->csi;
	vd->opb.headbit = 0;
	vd->opb.headptr = op->buffer.data;
	vd->opb.headend = op->buffer.size;

	const int mode = oggpack_read( &vd->opb, 1 );
	vd->state.lW = vd->state.W;
	vd->state.W = ci->mode_param[ mode ].blockflag;
	const int blocksizeLW = ci->blocksizes[ vd->state.lW ];
	for( int i = 0; i < vd->channels; ++i )
	{
		mdct_shift_right( blocksizeLW, (long *)vd->work[ i ], (long *)vd->mdctright[ i ] );
	}

	if( vd->state.out_begin == -1 )
	{
		vd->state.out_begin = 0;
		vd->state.out_end = 0;
		if( vd->state.extra_samples_begin >= ci->blocksizes[ 1 ] / 2 )
		{
			return;
		}
	}
	else
	{
		vd->state.out_begin = 0;
		vd->state.out_end = blocksizeLW / 4 + ci->blocksizes[ vd->state.W ] / 4;
		if( vd->state.extra_samples_begin )
		{
			vd->state.out_begin = vd->state.extra_samples_begin;
			if( vd->state.extra_samples_begin <= vd->state.out_end )
			{
				vd->state.extra_samples_begin = 0;
			}
			else
			{
				vd->state.out_begin = vd->state.out_end;
				vd->state.extra_samples_begin = (AkUInt16)( vd->state.extra_samples_begin - vd->state.out_end );
				if( vd->state.extra_samples_begin >= ci->blocksizes[ 1 ] / 2 )
				{
					return;
				}
			}
		}
		if( op->e_o_s )
		{
			vd->state.out_end -= vd->state.extra_samples_end;
			if( vd->state.out_end < vd->state.out_begin )
			{
				vd->state.out_end = vd->state.out_begin;
			}
		}
	}
	mapping_inverse( vd, &ci->map_param[ ci->mode_param[ mode ].mapping ] );
}
