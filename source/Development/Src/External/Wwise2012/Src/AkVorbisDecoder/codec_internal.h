/*=============================================================================
	codec_internal.h - the decoder state of Wwise 2012.1's Vorbis decoder: Xiph's Tremor (lowmem branch) with Audiokinetic's
	changes - packed setup headers whose codebooks are indices into g_packedCodebooks, float floor / MDCT / windowing
	(residue stays fixed point), a 1-bit mode field, no Ogg framing (2-byte packet sizes).

	DISHONORED(port): agent IH2 (wave 26). Every record is the 2012 PDB's (resources/docs/types/all_types.h). The functions
	are declared with retail's signatures (the PDB's `long` is AkInt32 = int-sized on x86).
=============================================================================*/
#pragma once

#include "ogg.h"
#include "AkVorbisTables.h"

struct codebook
{
	int dim;
	int entries;
	int used_entries;
	int dec_maxlength;
	void * dec_table;
	int dec_nodeb;
	int dec_leafw;
	int dec_type;
	int q_min;
	int q_minp;
	int q_del;
	int q_delp;
	int q_bits;
	int q_pack;
	void * q_val;
};

struct floor1class
{
	char class_dim;
	char class_subs;
	unsigned char class_book;
	unsigned char class_subbook[ 8 ];
};

struct vorbis_info_floor1
{
	floor1class * Class;
	char * partitionclass;
	AkUInt16 * postlist;
	char * forward_index;
	char * hineighbor;
	char * loneighbor;
	int partitions;
	int posts;
	int mult;
};

struct vorbis_info_residue
{
	int type;
	unsigned char * stagemasks;
	unsigned char * stagebooks;
	int begin;
	int end;
	int grouping;
	char partitions;
	unsigned char groupbook;
	char stages;
};

struct coupling_step
{
	unsigned char mag;
	unsigned char ang;
};

struct submap
{
	char floor;
	char residue;
};

struct vorbis_info_mapping
{
	int submaps;
	unsigned char * chmuxlist;
	submap * submaplist;
	int coupling_steps;
	coupling_step * coupling;
};

struct vorbis_info_mode
{
	unsigned char blockflag;
	unsigned char mapping;
};

struct codec_setup_info
{
	int blocksizes[ 2 ];
	int modes;
	int maps;
	int floors;
	int residues;
	int books;
	vorbis_info_mode * mode_param;
	vorbis_info_mapping * map_param;
	vorbis_info_floor1 * floor_param;
	vorbis_info_residue * residue_param;
	codebook * book_param;
};

struct vorbis_dsp_state2
{
	int out_begin;
	int out_end;
	int lW;
	int W;
	AkUInt16 extra_samples_begin;
	AkUInt16 extra_samples_end;
};

struct vorbis_dsp_state
{
	oggpack_buffer opb;
	int channels;
	codec_setup_info * csi;
	int * work[ 6 ];
	int * mdctright[ 6 ];
	vorbis_dsp_state2 state;
};

static_assert( sizeof( codebook ) == 60, "codebook: 2012 PDB sizeof 60 (vorbis_unpack_books allocates 60 * books)" );
static_assert( sizeof( floor1class ) == 11, "floor1class: 2012 PDB sizeof 11" );
static_assert( sizeof( vorbis_info_floor1 ) == 36, "vorbis_info_floor1: 2012 PDB sizeof 36" );
static_assert( sizeof( vorbis_info_residue ) == 28, "vorbis_info_residue: 2012 PDB sizeof 28" );
static_assert( sizeof( vorbis_info_mapping ) == 20, "vorbis_info_mapping: 2012 PDB sizeof 20" );
static_assert( sizeof( codec_setup_info ) == 48, "codec_setup_info: 2012 PDB sizeof 48" );
static_assert( sizeof( vorbis_dsp_state ) == 88, "vorbis_dsp_state: 2012 PDB sizeof 88" );

/* codebook.cpp */
void vorbis_book_unpack( oggpack_buffer * opb, codebook * s, CAkVorbisAllocator & VorbisAllocator );
long ak_vorbis_book_decode( codebook * book, oggpack_buffer * b );
void decode_map( codebook * s, oggpack_buffer * b, long * v, int point );
void vorbis_book_decodev_add( codebook * book, long * a, oggpack_buffer * b, int n, int point );
void vorbis_book_decodevv_add_2ch( codebook * book, long ** a, long offset, oggpack_buffer * b, int n, int point );

/* info.cpp */
int vorbis_info_init( codec_setup_info * csi, long in_blocksize0, long in_blocksize1 );
int vorbis_unpack_books( codec_setup_info * ci, int channels, oggpack_buffer * opb, CAkVorbisAllocator & VorbisAllocator );

/* floor1.cpp */
int floor1_info_unpack( vorbis_info_floor1 * info, codec_setup_info * ci, oggpack_buffer * opb, CAkVorbisAllocator & VorbisAllocator );
long * floor1_inverse1( vorbis_dsp_state * vd, vorbis_info_floor1 * info, long * fit_value );
int floor1_inverse2( vorbis_dsp_state * vd, vorbis_info_floor1 * info, long * fit_value, long * out );

/* res012.cpp */
int res_unpack( vorbis_info_residue * info, codec_setup_info * ci, oggpack_buffer * opb, CAkVorbisAllocator & VorbisAllocator );
int res_inverse( vorbis_dsp_state * vd, vorbis_info_residue * info, long ** in, int * nonzero, int ch );

/* mapping0.cpp */
int mapping_info_unpack( vorbis_info_mapping * info, codec_setup_info * ci, int channels, oggpack_buffer * opb, CAkVorbisAllocator & VorbisAllocator );
int mapping_inverse( vorbis_dsp_state * vd, vorbis_info_mapping * info );

/* mdct.cpp */
void mdct_backward( int n, float * in );
void mdct_shift_right( int n, long * in, long * right );
void mdct_unroll_lap( int n0, int n1, int lW, int W, float * in, float * right, const float * w0, const float * w1, short * out, int step, int start, int end );

/* dsp.cpp */
int vorbis_dsp_restart( vorbis_dsp_state * v, AkUInt16 in_uExtraSamplesBegin, AkUInt16 in_uExtraSamplesEnd );
int vorbis_dsp_init( vorbis_dsp_state * v, int channels );
void vorbis_dsp_clear( vorbis_dsp_state * v );
int vorbis_dsp_pcmout( vorbis_dsp_state * v, short * pcm, int samples );
void vorbis_dsp_synthesis( vorbis_dsp_state * vd, ogg_packet * op );
