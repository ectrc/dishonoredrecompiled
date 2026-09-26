/*=============================================================================
	AkAudioWwise.h - the Wwise 2012.1 headers AkAudio's classes need, plus the few constants and the one
	coordinate conversion that are Arkane's rather than Audiokinetic's.

	DISHONORED(written): AkAudioClasses.h includes this next to AkAudioNames.h so that the generated class
	bodies can declare AK-typed methods (Inc/CppText/UAkAudioDevice.h and friends). It is hand written and
	must be re-added to AkAudioClasses.h if that generated header is ever regenerated
	(resources/tools/symbols/gen_classes_header.py --sdk); the one line is marked there.

	The AK headers themselves live in source/Development/Src/External/Wwise2012/include, or in the
	installed SDK when DISHONORED_WWISE_SDK points at one (cmake/Wwise.cmake). Everything here compiles
	away when DISHONORED_WITH_WWISE is 0.
=============================================================================*/
#ifndef _INC_AKAUDIOWWISE
#define _INC_AKAUDIOWWISE

#ifndef DISHONORED_WITH_WWISE
#define DISHONORED_WITH_WWISE 0
#endif

#if DISHONORED_WITH_WWISE

#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/AkQueryParameters.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>

#if DISHONORED_WWISE_SILENT
#include <DishonoredWwiseSilent.h>
#endif

/**
 * The game object every non-positional call goes to. DISHONORED(retail): UAkAudioDevice::EnsureInitialized
 * (2013 rva 0x5b1430) ends with RegisterGameObj(2, "Unreal Global") followed by SetPosition(2, origin), and
 * PostEvent / SetSwitch / SeekOnEvent (0x5b2150, 0x5b22e0, 0x5b20d0) fall back to 2 when they have no actor.
 * SetRTPCValue and PostTrigger (0x5b2290, 0x5b2250) fall back to AK_INVALID_GAME_OBJECT instead, i.e. the
 * global RTPC / trigger scope - that asymmetry is retail's, not a slip.
 */
#define AKGLOBALSOUNDOBJECT				((AkGameObjectID)2)

/** AkFileSystemFlags values. DISHONORED(retail): RegisterCodec(0, 4, ...) at 0x5b1430 is AkVorbisDecoder. */
#define AKCOMPANYID_AUDIOKINETIC		0
#define AKCODECID_BANK					0
#define AKCODECID_VORBIS				4

/** DISHONORED(retail): Init.bnk, the only bank loaded by name before the referenced ones (0x5b2b80). */
#define AKINITBANKNAME					TEXT("Init")
#define AKBANKFILEEXTENSION				TEXT(".bnk")
/** The file-package extension of a non-seek-free bank; the seek-free BK_ ones use .<language> (0xc7210). */
#define AKPACKAGEFILEEXTENSION			TEXT(".pck")
/** DISHONORED(retail): UAkBank::Load compares the first three characters of the bank name (0xc7210). */
#define AKSEEKFREEBANKPREFIX			TEXT("BK_")

/**
 * DISHONORED(retail): the AK_EndOfEvent handler UAkAudioDevice::PostEvent installs when the caller brought
 * no cookie of its own (2013 rva 0x5b2150 passes DisAkEndOfEventCB with the world's audio system as the
 * cookie). Defined with the other Wwise callbacks in akaudioclasses.cpp.
 */
void DisAkEndOfEventCallback( AkCallbackType InType, AkCallbackInfo* InInfo );

/**
 * Unreal to Wwise. DISHONORED(retail): UAkAudioDevice::SetListener (2013 rva 0x5aecb0) writes each vector as
 * (-X, Z, Y): Unreal is Z-up left-handed, Wwise Y-up left-handed, and the X flip keeps left and right where
 * the mix expects them.
 */
FORCEINLINE AkVector AkVectorFromUnreal( const FVector& In )
{
	AkVector Out;
	Out.X = -In.X;
	Out.Y = In.Z;
	Out.Z = In.Y;
	return Out;
}

#endif // DISHONORED_WITH_WWISE

#endif // _INC_AKAUDIOWWISE
