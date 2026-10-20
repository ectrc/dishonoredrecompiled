/*=============================================================================
	AkPitchShifterFXFactory.h - the SDK public factory header for Wwise Pitch Shifter (2012.1). Agent IH4 (fork F2).
	Plug-in id 136 (Audiokinetic, effect). 2012 CreatePitchShifterFX 0x9966a0 (2013 0x98cb00),
	CreatePitchShifterFXParams 0x996d60 (2013 0x98d1f0).
=============================================================================*/
#ifndef _AK_PITCHSHIFTERFXFACTORY_H_
#define _AK_PITCHSHIFTERFXFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKEFFECTID_PITCHSHIFTER = 136;

AK::IAkPlugin * CreatePitchShifterFX( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreatePitchShifterFXParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
