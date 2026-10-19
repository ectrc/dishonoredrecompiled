/*=============================================================================
	AkDynamicSequence.h - AK::SoundEngine::DynamicSequence, the 2012.1 public API of dynamic sequences (a playlist of
	audio nodes the game feeds at run time). The Playlist / PlaylistItem types are the engine's
	(Src/AkAudioLib/Common/AkDynamicSequence.h).

	DISHONORED(port): agent IH3, fork C (wave 26). Dishonored's glue never calls this API (no AkAudio / DishonoredGame
	caller); it is here because retail links it (akaudiolib.cpp 2012 0x9178f0, 0x91a9d0..0x91ad10).
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCallback.h>

namespace AK
{
	namespace SoundEngine
	{
		namespace DynamicSequence
		{
			class Playlist;

			enum DynamicSequenceType
			{
				DynamicSequenceType_SampleAccurate		= 0,
				DynamicSequenceType_NormalTransition	= 1
			};

			AkPlayingID Open( AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags = 0, AkCallbackFunc in_pfnCallback = NULL, void * in_pCookie = NULL, DynamicSequenceType in_eDynamicSequenceType = DynamicSequenceType_SampleAccurate );
			AKRESULT Close( AkPlayingID in_playingID );
			AKRESULT Play( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );
			AKRESULT Pause( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );
			AKRESULT Resume( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );
			AKRESULT Stop( AkPlayingID in_playingID, AkTimeMs in_uTransitionDuration = 0, AkCurveInterpolation in_eFadeCurve = AkCurveInterpolation_Linear );
			AKRESULT Break( AkPlayingID in_playingID );
			Playlist * LockPlaylist( AkPlayingID in_playingID );
			AKRESULT UnlockPlaylist( AkPlayingID in_playingID );
		}
	}
}
