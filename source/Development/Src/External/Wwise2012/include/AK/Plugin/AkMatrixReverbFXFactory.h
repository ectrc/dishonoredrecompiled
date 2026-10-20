/*=============================================================================
	AkMatrixReverbFXFactory.h - the SDK public factory header for Wwise Matrix Reverb (2012.1). Agent IH4 (fork F2).
	Plug-in id 115 (Audiokinetic, effect). The class behind it is CAkFDNReverbFX (plugins/effects/akfdnreverb): "Matrix
	Reverb" is the feedback-delay-network reverb, one plug-in, not two. 2012 CreateMatrixReverbFX 0x9881c0 (the 2013
	twin is unmatched in the inventory), CreateMatrixReverbFXParams 0x988680 (2013 0x984e90).
=============================================================================*/
#ifndef _AK_MATRIXREVERBFXFACTORY_H_
#define _AK_MATRIXREVERBFXFACTORY_H_

#include <AK/SoundEngine/Common/IAkPlugin.h>

const AkUInt32 AKEFFECTID_MATRIXREVERB = 115;

AK::IAkPlugin * CreateMatrixReverbFX( AK::IAkPluginMemAlloc * in_pAllocator );
AK::IAkPluginParam * CreateMatrixReverbFXParams( AK::IAkPluginMemAlloc * in_pAllocator );

#endif
