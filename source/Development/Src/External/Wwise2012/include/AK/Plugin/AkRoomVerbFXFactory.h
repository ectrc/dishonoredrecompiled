/*=============================================================================
	AkRoomVerbFXFactory.h - the SDK public factory header for Wwise RoomVerb (2012.1). Agent IH4 (fork F1). Plug-in id 118 (Audiokinetic).
	2012 CreateRoomVerbFX 0x98bb10, CreateRoomVerbFXParams 0x98d350.
=============================================================================*/
#ifndef _AK_ROOMVERBFXFACTORY_H_
#define _AK_ROOMVERBFXFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKEFFECTID_ROOMVERB = 118;

AK::IAkPlugin * CreateRoomVerbFX( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreateRoomVerbFXParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
