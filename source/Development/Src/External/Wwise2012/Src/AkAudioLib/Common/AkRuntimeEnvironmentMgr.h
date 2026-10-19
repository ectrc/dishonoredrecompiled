/*=============================================================================
	AkRuntimeEnvironmentMgr.h - AK::AkRuntimeEnvironmentMgr (akruntimeenvironmentmgr.cpp): the CPU's SIMD support and
	cache sizes, read once with cpuid; CAkResampler::InitDSPFunctTable and CAkSinkDirectSound::DoRefill ask it for SSE2.

	DISHONORED(port): agent IH2 (wave 26). Layout of the 2012 PDB: vptr (IAkProcessorFeatures) + AkProcessorSupportInfo.
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>

namespace AK
{
	enum AkSIMDProcessorSupport
	{
		AK_SIMD_SSE		= 0x1,
		AK_SIMD_SSE2	= 0x2,
		AK_SIMD_SSE3	= 0x4,
		AK_SIMD_SSSE3	= 0x8
	};

	struct AkProcessorSupportInfo
	{
		AkUInt32 uSIMDProcessorSupport;
		AkUInt32 uL2CacheSize;
		AkUInt32 uCacheLineSize;
	};

	class IAkProcessorFeatures
	{
	public:
		virtual ~IAkProcessorFeatures() {}
		virtual bool GetSIMDSupport( AkSIMDProcessorSupport in_eSIMD ) = 0;
		virtual AkUInt32 GetCacheSize() = 0;
		virtual AkUInt32 GetCacheLineSize() = 0;
	};

	class AkRuntimeEnvironmentMgr : public IAkProcessorFeatures
	{
	public:
		static AkRuntimeEnvironmentMgr * Instance();

		virtual bool GetSIMDSupport( AkSIMDProcessorSupport in_eSIMD );
		virtual AkUInt32 GetCacheSize() { return ProcessorInfo.uL2CacheSize; }
		virtual AkUInt32 GetCacheLineSize() { return ProcessorInfo.uCacheLineSize; }

		AkProcessorSupportInfo ProcessorInfo;

	protected:
		AkRuntimeEnvironmentMgr();
	};
}
