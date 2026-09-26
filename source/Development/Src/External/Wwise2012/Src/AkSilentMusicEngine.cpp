/*=============================================================================
	AkSilentMusicEngine.cpp - AK::MusicEngine without an interactive-music hierarchy.

	DISHONORED(bringup): the music engine's whole job is to schedule music segments against the transport
	and fire the AK_MusicSync* callbacks. That schedule lives in the banks' music hierarchy, which the
	silent backend does not read, so Init / Term only flip the flag UAkAudioDevice::EnsureInitialized
	(2013 rva 0x5b1430) and ::Teardown (0x5b3de0) depend on, and GetPlayingSegmentInfo reports an empty
	segment. Nothing on the startup or map path reads a segment; Dishonored's music mix is bank logic.
=============================================================================*/
#include "AkSilentInternal.h"

#include <AK/MusicEngine/Common/AkMusicEngine.h>

namespace AK
{
	namespace MusicEngine
	{
		void GetDefaultInitSettings( AkMusicSettings & out_settings )
		{
			out_settings.fStreamingLookAheadRatio = 1.f;
		}

		AKRESULT Init( AkMusicSettings * /*in_pSettings*/ )
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( !e.bSoundEngineInit )
			{
				return AK_Fail;
			}
			e.bMusicEngineInit = true;
			AkSilent::Logf( L"MusicEngine::Init: silent backend (no music hierarchy)" );
			return AK_Success;
		}

		void Term()
		{
			AkSilent::Get().bMusicEngineInit = false;
		}

		AKRESULT GetPlayingSegmentInfo( AkPlayingID in_PlayingID, AkSegmentInfo & out_segmentInfo, bool /*in_bExtrapolate*/ )
		{
			memset( &out_segmentInfo, 0, sizeof( out_segmentInfo ) );
			return AkSilent::Get().Voices.count( in_PlayingID ) ? AK_Success : AK_Fail;
		}
	}
}
