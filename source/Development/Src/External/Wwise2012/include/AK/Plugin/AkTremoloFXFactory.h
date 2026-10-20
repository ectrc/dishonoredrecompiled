/*=============================================================================
	AkTremoloFXFactory.h - the SDK public factory header for Wwise Tremolo (2012.1). Agent IH4 (fork F2).
	Plug-in id 131 (Audiokinetic, effect): AK::SoundEngine::RegisterPlugin( AkPluginTypeEffect, AKCOMPANYID_AUDIOKINETIC,
	AKEFFECTID_TREMOLO, CreateTremoloFX, CreateTremoloFXParams ). 2012 CreateTremoloFX 0x99ad90 (2013 0x993a90),
	CreateTremoloFXParams 0x99b570 (2013 0x994270).
=============================================================================*/
#ifndef _AK_TREMOLOFXFACTORY_H_
#define _AK_TREMOLOFXFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKEFFECTID_TREMOLO = 131;

AK::IAkPlugin * CreateTremoloFX( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreateTremoloFXParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
