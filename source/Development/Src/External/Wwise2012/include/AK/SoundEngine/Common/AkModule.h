/*=============================================================================
	AkModule.h - Wwise 2012.1 memory-manager initialisation and the allocation hooks the game supplies.

	DISHONORED(layout): AkMemSettings is a single AkUInt32 (2012 PDB: AkMemSettings size=4,
	uMaxNumPools). AK::MemoryMgr::Init(struct AkMemSettings *) takes it by pointer, not by reference
	(demangled signature, 2012 rva 0x960a70).

	DISHONORED(written): the four hooks below are the ones Arkane defines, not Wwise: the 2012 PDB
	attributes `void __cdecl AK::FreeHook(void *)` (0x5f5410), `void * __cdecl AK::VirtualAllocHook(void *,
	unsigned int, unsigned long, unsigned long)` (0x5f5420) and `void __cdecl AK::VirtualFreeHook(void *,
	unsigned int, unsigned long)` (0x5f5440) to AkAudio/src/akaudiodevice.cpp, so the game side owns them
	and the default Wwise pool allocator calls them. AK::AllocHook has no separate entry in the 2012 PDB
	(it inlines into the pool code), but the SDK contract pairs it with FreeHook and our own
	implementation calls both.
=============================================================================*/
#ifndef _AK_MODULE_H_
#define _AK_MODULE_H_

#include <AK/SoundEngine/Common/AkTypes.h>

#if defined(_MSC_VER)
#pragma pack (push, 8)
#endif

/** DISHONORED(layout): 4 bytes. */
struct AkMemSettings
{
	AkUInt32 uMaxNumPools;
};

#if defined(_MSC_VER)
#pragma pack (pop)
#endif

namespace AK
{
	/** Allocation hooks the game defines; AkAudio's akaudiodevice.cpp routes them to appMalloc/appFree. */
	void * AllocHook( size_t in_size );
	void FreeHook( void * in_pMemAddress );
	void * VirtualAllocHook( void * in_pMemAddress, size_t in_size, AkUInt32 in_dwAllocationType, AkUInt32 in_dwProtect );
	void VirtualFreeHook( void * in_pMemAddress, size_t in_size, AkUInt32 in_dwFreeType );

	namespace MemoryMgr
	{
		/** Brings up the pool manager; AK_Success or AK_InsufficientMemory. */
		AKRESULT Init( AkMemSettings * in_pSettings );

		void GetDefaultSettings( AkMemSettings & out_settings );
	}
}

#if defined(__cplusplus) && !defined(AK_NO_LAYOUT_CHECKS)
static_assert(sizeof(AkMemSettings) == 4, "AkMemSettings: 2012 PDB sizeof 4");
#endif

#endif // _AK_MODULE_H_
