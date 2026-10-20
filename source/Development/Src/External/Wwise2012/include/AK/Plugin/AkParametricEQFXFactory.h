/*=============================================================================
	AkParametricEQFXFactory.h - the SDK public factory header for Wwise ParametricEQ (2012.1). Agent IH4 (fork F1). Plug-in id 105 (Audiokinetic).
	2012 CreateParametricEQFX 0x97bb70, CreateParametricEQFXParams 0x97bf40.
=============================================================================*/
#ifndef _AK_PARAMETRICEQFXFACTORY_H_
#define _AK_PARAMETRICEQFXFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKEFFECTID_PARAMETRICEQ = 105;

AK::IAkPlugin * CreateParametricEQFX( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreateParametricEQFXParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
