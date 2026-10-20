/*=============================================================================
	AkDelayFXFactory.h - the SDK public factory header for Wwise Delay (2012.1; the SDK sample samples/plugins/akdelay,
	shipped as AkDelayFX.lib). Agent IH4 (fork F2). Plug-in id 106 (Audiokinetic, effect).
	2012 CreateDelayFX 0x97c150 (2013 0x978950), CreateDelayFXParams 0x97c410 (2013 0x978c10).
=============================================================================*/
#ifndef _AK_DELAYFXFACTORY_H_
#define _AK_DELAYFXFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKEFFECTID_DELAY = 106;

AK::IAkPlugin * CreateDelayFX( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreateDelayFXParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
