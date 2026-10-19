/*=============================================================================
	AkVorbisFactory.h - the Vorbis codec factories (the SDK public header include/AK/Plugin/AkVorbisFactory.h).

	DISHONORED(port): agent IH2 fork S (wave 26). The game registers them once at init:
	  AK::SoundEngine::RegisterCodec( AKCOMPANYID_AUDIOKINETIC, AKCODECID_VORBIS, CreateVorbisFilePlugin, CreateVorbisBankPlugin );
	Retail names: class IAkSoftwareCodec * __cdecl CreateVorbisFilePlugin(void *) (2012 0x975e00) and
	CreateVorbisBankPlugin(void *) (2012 0x9765e0).
=============================================================================*/
#ifndef _AK_VORBISFACTORY_H_
#define _AK_VORBISFACTORY_H_

class IAkSoftwareCodec;

IAkSoftwareCodec * CreateVorbisFilePlugin( void * in_pCtx );
IAkSoftwareCodec * CreateVorbisBankPlugin( void * in_pCtx );

#endif
