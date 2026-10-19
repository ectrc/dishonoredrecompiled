/*=============================================================================
	tlsf.h - the Two-Level Segregated Fit allocator Wwise 2012.1 builds its variable-size pools on.

	DISHONORED(port): agent IH1. Retail links Matthew Conte's TLSF 2.x as wwise\sdk\source\soundengine\
	3rdparty\common\tlsf\tlsf.cpp (13 functions, 2012 0x960d30..0x961290, module akmemorymgr). The
	configuration is read off those bytes: SL_INDEX_COUNT_LOG2 5, ALIGN_SIZE_LOG2 2 (4-byte alignment),
	FL_INDEX_MAX 30, so pool_t = fl_bitmap + sl_bitmap[24] + blocks[24][32] = 3172 bytes and
	tlsf_overhead() = 3180 (2012 0x9610c0 returns the constant); block_null is a global, not a pool member.
=============================================================================*/
#pragma once

#include <stddef.h>

typedef void * tlsf_pool;

tlsf_pool tlsf_create( void * mem, size_t bytes );
void * tlsf_malloc( tlsf_pool tlsf, size_t bytes );
void * tlsf_memalign( tlsf_pool tlsf, size_t align, size_t bytes );
void tlsf_free( tlsf_pool tlsf, void * ptr );
size_t tlsf_block_size( void * ptr );
size_t tlsf_overhead();
