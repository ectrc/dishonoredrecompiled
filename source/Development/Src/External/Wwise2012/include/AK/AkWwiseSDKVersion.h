/*=============================================================================
	AkWwiseSDKVersion.h - Wwise 2012.1 version stamp.

	DISHONORED(written): resources/docs/middleware.md section 2.4 - the retail exe statically links the
	2012.1 generation of the Wwise runtime (2012 PDB source root d:\branches\wwise_v2012.1\, 291 source
	files of aksoundengine/akmusicengine/akstreammgr/ak*fx/akvorbisdecoder; every cooked bank is
	BKHD version 65 = AK_BANK_READER_VERSION of 2012.1). The exe carries no version string, so the
	build/patch numbers below are the generation, not an exact patch.

	These headers are OURS: they are reconstructed from the 2012 symbolized build's PDB (type database
	resources/docs/types/types.json, function signatures resources/docs/symbols/functions.csv) so that
	AkAudio and Engine's audio path compile and link against the real Wwise 2012.1 API shape. Nothing of
	Audiokinetic's own source or headers is reproduced here. When the user installs the licensed
	Wwise 2012.1 SDK, cmake/Wwise.cmake points the same include path at SDK/include instead and the
	AkAudio port needs no change (the directory layout below mirrors the SDK's).
=============================================================================*/
#ifndef _AK_WWISE_SDKVERSION_H_
#define _AK_WWISE_SDKVERSION_H_

#define AK_WWISESDK_VERSION_MAJOR				2012
#define AK_WWISESDK_VERSION_MINOR				1
#define AK_WWISESDK_VERSION_SUBMINOR			0
#define AK_WWISESDK_VERSION_BUILD				0
#define AK_WWISESDK_VERSION_NAME				"v2012.1"

/** Bank reader version every 2012.1 patch accepts; the retail content is BKHD 65. */
#define AK_BANK_READER_VERSION					65

#endif // _AK_WWISE_SDKVERSION_H_
