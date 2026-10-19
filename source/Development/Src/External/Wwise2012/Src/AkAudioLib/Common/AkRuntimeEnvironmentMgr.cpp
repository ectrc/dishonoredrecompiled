/*=============================================================================
	AkRuntimeEnvironmentMgr.cpp - port of akruntimeenvironmentmgr.cpp (2012 PDB, d:\branches\wwise_v2012.1\).

	DISHONORED(port): agent IH2 (wave 26). cpuid through the __cpuid intrinsic instead of inline asm (same leaves, same
	bits: leaf 1 EDX 25/26 = SSE/SSE2, ECX 0/9 = SSE3/SSSE3; leaf 0x80000006 ECX = line size / L2 size, else 64 / 1024).
=============================================================================*/
#include "AkRuntimeEnvironmentMgr.h"
#include <intrin.h>

namespace AK
{
	/** 2012 0x934b20; 2013 0x931440. */
	bool AkRuntimeEnvironmentMgr::GetSIMDSupport( AkSIMDProcessorSupport in_eSIMD )
	{
		return ( in_eSIMD & ProcessorInfo.uSIMDProcessorSupport ) != 0;
	}

	/** 2012 0x934b30; 2013 0x931460. */
	AkRuntimeEnvironmentMgr::AkRuntimeEnvironmentMgr()
	{
		ProcessorInfo.uSIMDProcessorSupport = 0;
		ProcessorInfo.uL2CacheSize = 0;
		ProcessorInfo.uCacheLineSize = 0;
		int regs[ 4 ];
		AkUInt32 uECX = 0;
		AkUInt32 uEDX = 0;
		__cpuid( regs, 0 );
		if( (AkUInt32)regs[ 0 ] >= 1 )
		{
			__cpuid( regs, 1 );
			uECX = (AkUInt16)regs[ 2 ];
			uEDX = (AkUInt32)regs[ 3 ];
		}
		__cpuid( regs, 0x80000000 );
		if( (AkUInt32)regs[ 0 ] < 0x80000006 )
		{
			ProcessorInfo.uCacheLineSize = 64;
			ProcessorInfo.uL2CacheSize = 1024;
		}
		else
		{
			__cpuid( regs, 0x80000006 );
			ProcessorInfo.uCacheLineSize = (AkUInt8)regs[ 2 ];
			ProcessorInfo.uL2CacheSize = (AkUInt32)regs[ 2 ] >> 16;
		}
		if( uEDX & 0x2000000 )
		{
			ProcessorInfo.uSIMDProcessorSupport |= AK_SIMD_SSE;
		}
		if( uEDX & 0x4000000 )
		{
			ProcessorInfo.uSIMDProcessorSupport |= AK_SIMD_SSE2;
		}
		if( uECX & 1 )
		{
			ProcessorInfo.uSIMDProcessorSupport |= AK_SIMD_SSE3;
		}
		if( uECX & 0x200 )
		{
			ProcessorInfo.uSIMDProcessorSupport |= AK_SIMD_SSSE3;
		}
	}

	/** 2012 0x934c50; 2013 0x931560. The function-local static is retail's g_RuntimeEnvMgr. */
	AkRuntimeEnvironmentMgr * AkRuntimeEnvironmentMgr::Instance()
	{
		static AkRuntimeEnvironmentMgr g_RuntimeEnvMgr;
		return &g_RuntimeEnvMgr;
	}
}
