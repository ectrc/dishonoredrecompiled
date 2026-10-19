/*=============================================================================
	tlsf.cpp - TLSF 2.x as Wwise 2012.1 ships it (wwise\sdk\source\soundengine\3rdparty\common\tlsf\tlsf.cpp).

	DISHONORED(port): agent IH1, from the 2012 decompile of every function of tlsf.obj:
	  remove_free_block 0x960d30, block_remove 0x960d80, block_insert 0x960e00, block_trim_free 0x960e70,
	  block_trim_free_leading 0x960ee0, block_locate_free 0x960f40, pool_construct 0x960fe0,
	  tlsf_block_size 0x9610a0, tlsf_overhead 0x9610c0, tlsf_create 0x9610d0, tlsf_malloc 0x961140,
	  tlsf_memalign 0x9611a0, tlsf_free 0x961290.
	Block header = { prev_phys_block, size, next_free, prev_free }; the user pointer is &next_free; size bit 0
	is "this block is free", bit 1 "the previous physical block is free". tlsf_destroy is empty in retail
	(AK::MemoryMgr::DestroyPool calls a folded no-op), so it is not declared.
=============================================================================*/
#include "tlsf.h"

#include <string.h>
#include <intrin.h>

namespace
{
	enum
	{
		SL_INDEX_COUNT_LOG2	= 5,
		ALIGN_SIZE_LOG2		= 2,
		ALIGN_SIZE			= ( 1 << ALIGN_SIZE_LOG2 ),
		FL_INDEX_MAX		= 30,
		SL_INDEX_COUNT		= ( 1 << SL_INDEX_COUNT_LOG2 ),
		FL_INDEX_SHIFT		= ( SL_INDEX_COUNT_LOG2 + ALIGN_SIZE_LOG2 ),
		FL_INDEX_COUNT		= ( FL_INDEX_MAX - FL_INDEX_SHIFT + 1 ),
		SMALL_BLOCK_SIZE	= ( 1 << FL_INDEX_SHIFT )
	};

	struct block_header_t
	{
		block_header_t *	prev_phys_block;
		size_t				size;
		block_header_t *	next_free;
		block_header_t *	prev_free;
	};

	const size_t block_header_free_bit = 1 << 0;
	const size_t block_header_prev_free_bit = 1 << 1;
	const size_t block_header_overhead = sizeof( size_t );
	const size_t block_start_offset = offsetof( block_header_t, size ) + sizeof( size_t );
	const size_t block_size_min = sizeof( block_header_t ) - sizeof( block_header_t * );
	const size_t block_size_max = (size_t)1 << FL_INDEX_MAX;

	struct pool_t
	{
		unsigned int		fl_bitmap;
		unsigned int		sl_bitmap[ FL_INDEX_COUNT ];
		block_header_t *	blocks[ FL_INDEX_COUNT ][ SL_INDEX_COUNT ];
	};

	static_assert( sizeof( pool_t ) == 3172, "tlsf pool_t: 3172 bytes in retail (tlsf_overhead 3180 - 2 * 4)" );

	block_header_t block_null;

	int tlsf_ffs( unsigned int word )
	{
		unsigned long index;
		return _BitScanForward( &index, word ) ? (int)index : -1;
	}

	int tlsf_fls( unsigned int word )
	{
		unsigned long index;
		return _BitScanReverse( &index, word ) ? (int)index : -1;
	}

	size_t block_size( const block_header_t * block ) { return block->size & ~( block_header_free_bit | block_header_prev_free_bit ); }
	void block_set_size( block_header_t * block, size_t size ) { block->size = size | ( block->size & ( block_header_free_bit | block_header_prev_free_bit ) ); }
	int block_is_free( const block_header_t * block ) { return (int)( block->size & block_header_free_bit ); }
	void block_set_free( block_header_t * block ) { block->size |= block_header_free_bit; }
	void block_set_used( block_header_t * block ) { block->size &= ~block_header_free_bit; }
	int block_is_prev_free( const block_header_t * block ) { return (int)( block->size & block_header_prev_free_bit ); }
	void block_set_prev_free( block_header_t * block ) { block->size |= block_header_prev_free_bit; }
	void block_set_prev_used( block_header_t * block ) { block->size &= ~block_header_prev_free_bit; }

	block_header_t * block_from_ptr( const void * ptr ) { return (block_header_t *)( (unsigned char *)ptr - block_start_offset ); }
	void * block_to_ptr( const block_header_t * block ) { return (void *)( (unsigned char *)block + block_start_offset ); }
	block_header_t * offset_to_block( const void * ptr, size_t size ) { return (block_header_t *)( (ptrdiff_t)ptr + size ); }

	block_header_t * block_next( const block_header_t * block )
	{
		return offset_to_block( block_to_ptr( block ), block_size( block ) - block_header_overhead );
	}

	block_header_t * block_link_next( block_header_t * block )
	{
		block_header_t * next = block_next( block );
		next->prev_phys_block = block;
		return next;
	}

	void block_mark_as_free( block_header_t * block )
	{
		block_header_t * next = block_link_next( block );
		block_set_prev_free( next );
		block_set_free( block );
	}

	void block_mark_as_used( block_header_t * block )
	{
		block_header_t * next = block_next( block );
		block_set_prev_used( next );
		block_set_used( block );
	}

	size_t align_up( size_t x, size_t align ) { return ( x + ( align - 1 ) ) & ~( align - 1 ); }
	size_t align_down( size_t x, size_t align ) { return x - ( x & ( align - 1 ) ); }
	void * align_ptr( const void * ptr, size_t align ) { return (void *)( ( (ptrdiff_t)ptr + ( align - 1 ) ) & ~( align - 1 ) ); }

	size_t adjust_request_size( size_t size, size_t align )
	{
		size_t adjust = 0;
		if( size && size < block_size_max )
		{
			const size_t aligned = align_up( size, align );
			adjust = aligned > block_size_min ? aligned : block_size_min;
		}
		return adjust;
	}

	void mapping_insert( size_t size, int * fli, int * sli )
	{
		int fl, sl;
		if( size < SMALL_BLOCK_SIZE )
		{
			fl = 0;
			sl = (int)size / ( SMALL_BLOCK_SIZE / SL_INDEX_COUNT );
		}
		else
		{
			fl = tlsf_fls( (unsigned int)size );
			sl = (int)( size >> ( fl - SL_INDEX_COUNT_LOG2 ) ) ^ ( 1 << SL_INDEX_COUNT_LOG2 );
			fl -= ( FL_INDEX_SHIFT - 1 );
		}
		*fli = fl;
		*sli = sl;
	}

	void mapping_search( size_t size, int * fli, int * sli )
	{
		if( size >= ( 1 << SL_INDEX_COUNT_LOG2 ) )
		{
			const size_t round = ( (size_t)1 << ( tlsf_fls( (unsigned int)size ) - SL_INDEX_COUNT_LOG2 ) ) - 1;
			size += round;
		}
		mapping_insert( size, fli, sli );
	}

	block_header_t * search_suitable_block( pool_t * pool, int * fli, int * sli )
	{
		int fl = *fli;
		int sl = *sli;
		unsigned int sl_map = pool->sl_bitmap[ fl ] & ( ~0u << sl );
		if( !sl_map )
		{
			const unsigned int fl_map = pool->fl_bitmap & ( ~0u << ( fl + 1 ) );
			if( !fl_map )
			{
				return 0;
			}
			fl = tlsf_ffs( fl_map );
			*fli = fl;
			sl_map = pool->sl_bitmap[ fl ];
		}
		sl = tlsf_ffs( sl_map );
		*sli = sl;
		return pool->blocks[ fl ][ sl ];
	}

	void remove_free_block( pool_t * pool, block_header_t * block, int fl, int sl )
	{
		block_header_t * prev = block->prev_free;
		block_header_t * next = block->next_free;
		next->prev_free = prev;
		prev->next_free = next;
		if( pool->blocks[ fl ][ sl ] == block )
		{
			pool->blocks[ fl ][ sl ] = next;
			if( next == &block_null )
			{
				pool->sl_bitmap[ fl ] &= ~( 1u << sl );
				if( !pool->sl_bitmap[ fl ] )
				{
					pool->fl_bitmap &= ~( 1u << fl );
				}
			}
		}
	}

	void insert_free_block( pool_t * pool, block_header_t * block, int fl, int sl )
	{
		block_header_t * current = pool->blocks[ fl ][ sl ];
		block->next_free = current;
		block->prev_free = &block_null;
		current->prev_free = block;
		pool->blocks[ fl ][ sl ] = block;
		pool->fl_bitmap |= ( 1u << fl );
		pool->sl_bitmap[ fl ] |= ( 1u << sl );
	}

	void block_remove( pool_t * pool, block_header_t * block )
	{
		int fl, sl;
		mapping_insert( block_size( block ), &fl, &sl );
		remove_free_block( pool, block, fl, sl );
	}

	void block_insert( pool_t * pool, block_header_t * block )
	{
		int fl, sl;
		mapping_insert( block_size( block ), &fl, &sl );
		insert_free_block( pool, block, fl, sl );
	}

	int block_can_split( block_header_t * block, size_t size )
	{
		return block_size( block ) >= sizeof( block_header_t ) + size;
	}

	block_header_t * block_split( block_header_t * block, size_t size )
	{
		block_header_t * remaining = offset_to_block( block_to_ptr( block ), size - block_header_overhead );
		const size_t remain_size = block_size( block ) - ( size + block_header_overhead );
		block_set_size( remaining, remain_size );
		block_set_size( block, size );
		block_mark_as_free( remaining );
		return remaining;
	}

	block_header_t * block_absorb( block_header_t * prev, block_header_t * block )
	{
		prev->size += block_size( block ) + block_header_overhead;
		block_link_next( prev );
		return prev;
	}

	block_header_t * block_merge_prev( pool_t * pool, block_header_t * block )
	{
		if( block_is_prev_free( block ) )
		{
			block_header_t * prev = block->prev_phys_block;
			block_remove( pool, prev );
			block = block_absorb( prev, block );
		}
		return block;
	}

	block_header_t * block_merge_next( pool_t * pool, block_header_t * block )
	{
		block_header_t * next = block_next( block );
		if( block_is_free( next ) )
		{
			block_remove( pool, next );
			block = block_absorb( block, next );
		}
		return block;
	}

	void block_trim_free( pool_t * pool, block_header_t * block, size_t size )
	{
		if( block_can_split( block, size ) )
		{
			block_header_t * remaining_block = block_split( block, size );
			block_link_next( block );
			block_set_prev_free( remaining_block );
			block_insert( pool, remaining_block );
		}
	}

	block_header_t * block_trim_free_leading( pool_t * pool, block_header_t * block, size_t size )
	{
		block_header_t * remaining_block = block;
		if( block_can_split( block, size ) )
		{
			remaining_block = block_split( block, size - block_header_overhead );
			block_set_prev_free( remaining_block );
			block_link_next( block );
			block_insert( pool, block );
		}
		return remaining_block;
	}

	block_header_t * block_locate_free( pool_t * pool, size_t size )
	{
		int fl = 0, sl = 0;
		block_header_t * block = 0;
		if( size )
		{
			mapping_search( size, &fl, &sl );
			block = search_suitable_block( pool, &fl, &sl );
		}
		if( block )
		{
			remove_free_block( pool, block, fl, sl );
		}
		return block;
	}

	void * block_prepare_used( pool_t * pool, block_header_t * block, size_t size )
	{
		void * p = 0;
		if( block )
		{
			block_trim_free( pool, block, size );
			block_mark_as_used( block );
			p = block_to_ptr( block );
		}
		return p;
	}

	void pool_construct( pool_t * pool )
	{
		block_null.next_free = &block_null;
		block_null.prev_free = &block_null;
		block_null.size = 0;
		block_null.prev_phys_block = 0;
		pool->fl_bitmap = 0;
		for( int i = 0; i < FL_INDEX_COUNT; ++i )
		{
			pool->sl_bitmap[ i ] = 0;
			for( int j = 0; j < SL_INDEX_COUNT; ++j )
			{
				pool->blocks[ i ][ j ] = &block_null;
			}
		}
	}
}

size_t tlsf_block_size( void * ptr )
{
	size_t size = 0;
	if( ptr )
	{
		size = block_size( block_from_ptr( ptr ) );
	}
	return size;
}

size_t tlsf_overhead()
{
	return sizeof( pool_t ) + 2 * block_header_overhead;
}

tlsf_pool tlsf_create( void * mem, size_t bytes )
{
	const size_t pool_overhead = tlsf_overhead();
	const size_t pool_bytes = align_down( bytes - pool_overhead, ALIGN_SIZE );
	if( pool_bytes < block_size_min || pool_bytes > block_size_max )
	{
		return 0;
	}

	pool_t * pool = (pool_t *)mem;
	pool_construct( pool );

	block_header_t * block = offset_to_block( (void *)pool, sizeof( pool_t ) - block_header_overhead );
	block->size = 0;
	block_set_size( block, pool_bytes );
	block_set_free( block );
	block_set_prev_used( block );
	block_insert( pool, block );

	block_header_t * next = block_link_next( block );
	next->size = 0;
	block_set_size( next, 0 );
	block_set_used( next );
	block_set_prev_free( next );
	return mem;
}

void * tlsf_malloc( tlsf_pool tlsf, size_t size )
{
	pool_t * pool = (pool_t *)tlsf;
	const size_t adjust = adjust_request_size( size, ALIGN_SIZE );
	block_header_t * block = block_locate_free( pool, adjust );
	return block_prepare_used( pool, block, adjust );
}

void * tlsf_memalign( tlsf_pool tlsf, size_t align, size_t size )
{
	pool_t * pool = (pool_t *)tlsf;
	const size_t adjust = adjust_request_size( size, ALIGN_SIZE );
	const size_t gap_minimum = sizeof( block_header_t );
	const size_t size_with_gap = adjust_request_size( adjust + align + gap_minimum, align );
	const size_t aligned_size = ( align <= ALIGN_SIZE ) ? adjust : size_with_gap;

	block_header_t * block = block_locate_free( pool, aligned_size );
	if( block )
	{
		void * ptr = block_to_ptr( block );
		void * aligned = align_ptr( ptr, align );
		size_t gap = (size_t)( (ptrdiff_t)aligned - (ptrdiff_t)ptr );
		if( gap && gap < gap_minimum )
		{
			const size_t gap_remain = gap_minimum - gap;
			const size_t offset = gap_remain > align ? gap_remain : align;
			const void * next_aligned = (void *)( (ptrdiff_t)aligned + offset );
			aligned = align_ptr( next_aligned, align );
			gap = (size_t)( (ptrdiff_t)aligned - (ptrdiff_t)ptr );
		}
		if( gap )
		{
			block = block_trim_free_leading( pool, block, gap );
		}
	}
	return block_prepare_used( pool, block, adjust );
}

void tlsf_free( tlsf_pool tlsf, void * ptr )
{
	if( ptr )
	{
		pool_t * pool = (pool_t *)tlsf;
		block_header_t * block = block_from_ptr( ptr );
		block_mark_as_free( block );
		block = block_merge_prev( pool, block );
		block = block_merge_next( pool, block );
		block_insert( pool, block );
	}
}
