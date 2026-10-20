/*=============================================================================
	AkTimeStretchFXFactory.h - the SDK public factory header for Wwise Time Stretch (2012.1). Agent IH4 (fork F2).
	Plug-in id 130 (Audiokinetic, effect). 2012 CreateTimeStretchFX 0x998000 (2013 0x98f430), CreateTimeStretchFXParams
	0x9982d0 (2013 0x98f700).
=============================================================================*/
#ifndef _AK_TIMESTRETCHFXFACTORY_H_
#define _AK_TIMESTRETCHFXFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKEFFECTID_TIMESTRETCH = 130;

AK::IAkPlugin * CreateTimeStretchFX( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreateTimeStretchFXParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
