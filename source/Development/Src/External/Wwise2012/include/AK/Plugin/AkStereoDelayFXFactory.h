/*=============================================================================
	AkStereoDelayFXFactory.h - the SDK public factory header for Wwise Stereo Delay (2012.1). Agent IH4 (fork F2).
	Plug-in id 135 (Audiokinetic, effect). Linked into the 2012 exe (CreateStereoDelayFX 0x998850,
	CreateStereoDelayFXParams 0x998f60) but neither registered by Dishonored's glue nor used by any bank; the 2013 exe
	does not contain it.
=============================================================================*/
#ifndef _AK_STEREODELAYFXFACTORY_H_
#define _AK_STEREODELAYFXFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKEFFECTID_STEREODELAY = 135;

AK::IAkPlugin * CreateStereoDelayFX( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreateStereoDelayFXParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
