/*=============================================================================
	AkSilentHooks.cpp - AK::AllocHook / FreeHook / VirtualAllocHook / VirtualFreeHook.

	DISHONORED(bringup): the SDK contract puts these four in the game (AkModule.h has the rationale and the
	2012 rvas of retail's own copies in AkAudio/src/akaudiodevice.cpp). The backend defines them here so
	that it links on its own - Engine's akevent.cpp calls AK::SoundEngine::GetIDFromString and the layout
	probe links Engine without AkAudio - and routes them through DishonoredWwise::SetAllocator, which
	UAkAudioDevice::EnsureInitialized points at appMalloc / appFree. Until then they are the CRT's.

	VirtualAllocHook / VirtualFreeHook exist because Wwise's AkVirtualAlloc pool attribute reserves
	pages directly; with no such pool in the silent backend they fall through to the same allocator and
	the flags are ignored, which is recorded here rather than pretended away.
=============================================================================*/
#include "AkSilentInternal.h"

#include <cstdlib>

namespace
{
	void * CrtAlloc( size_t in_size )
	{
		return malloc( in_size );
	}

	void CrtFree( void * in_pMemory )
	{
		free( in_pMemory );
	}

	DishonoredWwise::AllocFunc g_pAlloc = &CrtAlloc;
	DishonoredWwise::FreeFunc g_pFree = &CrtFree;
}

namespace DishonoredWwise
{
	void SetAllocator( AllocFunc in_pAlloc, FreeFunc in_pFree )
	{
		g_pAlloc = in_pAlloc ? in_pAlloc : &CrtAlloc;
		g_pFree = in_pFree ? in_pFree : &CrtFree;
	}
}

namespace AK
{
	void * AllocHook( size_t in_size )
	{
		return g_pAlloc( in_size );
	}

	void FreeHook( void * in_pMemAddress )
	{
		if( in_pMemAddress )
		{
			g_pFree( in_pMemAddress );
		}
	}

	void * VirtualAllocHook( void * /*in_pMemAddress*/, size_t in_size, AkUInt32 /*in_dwAllocationType*/, AkUInt32 /*in_dwProtect*/ )
	{
		return g_pAlloc( in_size );
	}

	void VirtualFreeHook( void * in_pMemAddress, size_t /*in_size*/, AkUInt32 /*in_dwFreeType*/ )
	{
		if( in_pMemAddress )
		{
			g_pFree( in_pMemAddress );
		}
	}
}
