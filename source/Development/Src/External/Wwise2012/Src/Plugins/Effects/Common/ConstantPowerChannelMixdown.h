/*=============================================================================
	ConstantPowerChannelMixdown.h - DSP::ConstantPowerChannelMixdown (plugins\effects\common\constantpowerchannelmixdown.cpp,
	2012 0x98f2e0, 2013 matched by bytes): the mono sum of a buffer's channels at constant power, center and LFE scaled by
	their input levels (clamped to 1), the LFE added last.

	DISHONORED(port): agent IH4 fork F1 (wave 26) - written here because the RoomVerb needs it (F1_helpers.txt).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/IAkPlugin.h>

namespace DSP
{
	void ConstantPowerChannelMixdown( AkAudioBuffer * in_pAudioBufferIn, AkUInt32 in_uNumFrames, AkUInt32 in_uFrameOffset, AkReal32 * out_pfBufferOut,
		AkUInt32 in_uChannelMask, AkReal32 in_fInCenterGain, AkReal32 in_fInLFEGain );
}
