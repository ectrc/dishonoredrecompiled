/*=============================================================================
	AkMusicEngine.h - Wwise 2012.1 interactive-music engine surface.

	DISHONORED(written): the whole namespace as the retail exe links it
	  void __cdecl AK::MusicEngine::GetDefaultInitSettings(struct AkMusicSettings &)        0x961330
	  enum AKRESULT __cdecl AK::MusicEngine::Init(struct AkMusicSettings *)                 0x962110
	  void __cdecl AK::MusicEngine::Term(void)                                              0x962010
	  enum AKRESULT __cdecl AK::MusicEngine::GetPlayingSegmentInfo(unsigned long,
	      struct AkSegmentInfo &, bool)                                                     0x961380
	UAkAudioDevice::EnsureInitialized (2013 rva 0x5b1430) calls GetDefaultInitSettings then Init right
	after AK::SoundEngine::Init, and UAkAudioDevice::Teardown (0x5b3de0) calls Term first of all.
	Note Init takes a pointer while GetDefaultInitSettings takes a reference - that asymmetry is Wwise's.
=============================================================================*/
#ifndef _AK_MUSIC_ENGINE_H_
#define _AK_MUSIC_ENGINE_H_

#include <AK/SoundEngine/Common/AkTypes.h>

#if defined(_MSC_VER)
#pragma pack (push, 8)
#endif

/** DISHONORED(layout): 4 bytes, a single AkReal32 (2012 PDB AkMusicSettings). */
struct AkMusicSettings
{
	AkReal32 fStreamingLookAheadRatio;
};

#if defined(_MSC_VER)
#pragma pack (pop)
#endif

namespace AK
{
	namespace MusicEngine
	{
		void GetDefaultInitSettings( AkMusicSettings & out_settings );							///< 2012 rva 0x961330
		AKRESULT Init( AkMusicSettings * in_pSettings );										///< 2012 rva 0x962110
		void Term();																			///< 2012 rva 0x962010
		AKRESULT GetPlayingSegmentInfo( AkPlayingID in_PlayingID, AkSegmentInfo & out_segmentInfo, bool in_bExtrapolate = false );	///< 2012 rva 0x961380
	}
}

#if defined(__cplusplus) && !defined(AK_NO_LAYOUT_CHECKS)
static_assert(sizeof(AkMusicSettings) == 4, "AkMusicSettings: 2012 PDB sizeof 4");
#endif

#endif // _AK_MUSIC_ENGINE_H_
