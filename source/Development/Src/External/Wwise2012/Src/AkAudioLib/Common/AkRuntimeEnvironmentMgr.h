/*=============================================================================
	AkRuntimeEnvironmentMgr.h - AK::AkRuntimeEnvironmentMgr (akruntimeenvironmentmgr.cpp): the CPU's SIMD support and
	cache sizes, read once with cpuid; CAkResampler::InitDSPFunctTable and CAkSinkDirectSound::DoRefill ask it for SSE2.

	DISHONORED(port): agent IH2 (wave 26). Layout of the 2012 PDB: vptr (IAkProcessorFeatures) + AkProcessorSupportInfo.
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>	// DISHONORED(IH4): IAkProcessorFeatures and AkSIMDProcessorSupport are public (IAkPlugin.h)

namespace AK
{
	struct AkProcessorSupportInfo
	{
		AkUInt32 uSIMDProcessorSupport;
		AkUInt32 uL2CacheSize;
		AkUInt32 uCacheLineSize;
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
