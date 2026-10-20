/*=============================================================================
	AkHarmonizerFXFactory.h - the SDK public factory header for Wwise Harmonizer (2012.1). Agent IH4 (fork F2).
	Plug-in id 139 (Audiokinetic, effect). Linked into the 2012 exe (CreateHarmonizerFX 0x990460,
	CreateHarmonizerFXParams 0x990cf0) but neither registered by Dishonored's glue nor used by any bank; the 2013 exe
	does not contain it.
=============================================================================*/
#ifndef _AK_HARMONIZERFXFACTORY_H_
#define _AK_HARMONIZERFXFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKEFFECTID_HARMONIZER = 139;

AK::IAkPlugin * CreateHarmonizerFX( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreateHarmonizerFXParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
