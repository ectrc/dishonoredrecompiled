/*=============================================================================
	AkSilenceSourceFactory.h - the SDK public factory header for Wwise SilenceSource (2012.1). Agent IH4 (fork F1). Plug-in id 101 (Audiokinetic).
	2012 CreateSilenceSource 0x97ae40, CreateSilenceSourceParams 0x97ae20.
=============================================================================*/
#ifndef _AK_SILENCESOURCEFACTORY_H_
#define _AK_SILENCESOURCEFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKSOURCEID_SILENCE = 101;

AK::IAkPlugin * CreateSilenceSource( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreateSilenceSourceParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
