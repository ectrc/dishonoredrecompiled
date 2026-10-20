/*=============================================================================
	DishonoredWwiseSilent.h - the seam between our silent Wwise 2012.1 backend and the game.

	DISHONORED(bringup): this header is NOT part of the Wwise API (nothing under include/AK includes it).
	It exists because we implement the API ourselves: the backend has no output device and no DSP graph,
	so it keeps books instead - which banks were asked for and whether their files parse, which events
	were posted and under which playing id, the last value of every RTPC / switch / state / trigger, and
	which game objects are registered - and hands the game a hook to print all of that into the Unreal
	log. When the licensed Wwise 2012.1 SDK is dropped in (cmake/Wwise.cmake, DISHONORED_WWISE_SDK), this
	header keeps compiling but every function becomes a no-op stub, so the AkAudio side never changes.

	DISHONORED(port, agent IH2): the backend now decodes, mixes and outputs (the ported 2012.1 runtime); the name is
	historical. What still keeps books is the IH3 seam (RTPC / switch / state / trigger values).
	See resources/docs/agents/agentAN.md and agentIH2.md.
=============================================================================*/
#ifndef _DISHONORED_WWISE_SILENT_H_
#define _DISHONORED_WWISE_SILENT_H_

#include <AK/SoundEngine/Common/AkTypes.h>

namespace DishonoredWwise
{
	/** Sink for the backend's own log lines; AkAudio installs one that calls debugf. */
	typedef void ( *LogHook )( const AkOSChar * in_pszMessage );

	/** TRUE when the silent backend is linked (as opposed to a real Audiokinetic runtime). */
	bool IsSilentBackend();

	/**
	 * The allocator behind AK::AllocHook / AK::FreeHook. The SDK contract is that the *game* defines
	 * those four hooks (AkModule.h), and retail does exactly that in AkAudio/src/akaudiodevice.cpp. Our
	 * backend has to link on its own as well (Engine's akevent.cpp calls GetIDFromString, and the layout
	 * probe links Engine without AkAudio), so the hooks are defined inside the backend on top of this
	 * pair and default to the CRT; AkAudio installs appMalloc / appFree over them at device init.
	 */
	typedef void * ( *AllocFunc )( size_t in_size );
	typedef void ( *FreeFunc )( void * in_pMemory );
	void SetAllocator( AllocFunc in_pAlloc, FreeFunc in_pFree );

	void SetLogHook( LogHook in_pHook );

	/**
	 * -akdiag (agent IH1): log every bank load / prepare / unload with its HIRC object counts, every
	 * GetIDFromString with its answer, every Query::QueryAudioObjectIDs / GetPositioningInfo answer, every
	 * stream-manager file open and every PostEvent of an event no loaded bank declares.
	 */
	void SetDiagnostics( bool in_bEnabled );
	bool IsDiagnostics();

	/** -akwav=<file> (agent IH2): every buffer the sink is handed (16-bit, the sink speaker layout) is appended to a WAV file. */
	void SetWavCapture( const AkOSChar * in_pszPath );

	/** -akdump=<file> (agent IQ): a minidump of the first fatal exception, pure virtual call or abort of any thread. */
	void SetCrashDump( const wchar_t * in_pszPath );

	/** What the backend has been asked to do so far; the AkAudio `AKSTATS` exec prints it. */
	struct Stats
	{
		AkUInt32 BanksLoaded;			///< LoadBank calls that found their file and parsed a BKHD
		AkUInt32 BanksMissing;			///< LoadBank calls whose file the resolver could not open
		AkUInt32 BanksPrepared;			///< PrepareBank(Load) calls (the seek-free BK_ path)
		AkUInt32 FilePackages;			///< AKPK packages registered through the low-level IO hook
		AkUInt32 EventsPosted;			///< PostEvent calls that returned a playing id
		AkUInt32 EventsUnknown;			///< PostEvent calls for an event id no loaded bank declared
		AkUInt32 TriggersPosted;
		AkUInt32 RtpcSets;
		AkUInt32 SwitchSets;
		AkUInt32 StateSets;
		AkUInt32 GameObjects;			///< currently registered
		AkUInt32 PlayingIDsActive;		///< posted and not yet ended
	};

	const Stats & GetStats();

	/**
	 * The name behind an id, when the backend has seen the string (every GetIDFromString and every
	 * bank / event / switch name that passed through the API is remembered). NULL when unknown, so the
	 * log falls back to the raw id.
	 */
	const AkOSChar * NameForID( AkUInt32 in_id );

	/**
	 * DISHONORED(shim, agent IH2): agent ID kept a voice alive for a hinted number of seconds while the backend had no
	 * media length. The ported runtime ends a voice at its media end and fires AK_Duration itself, so this is a no-op
	 * kept for the conversation component's call; it answers whether the playing id is valid.
	 */
	bool SetDurationHint( AkPlayingID in_playingID, AkReal32 in_fSeconds );
}

#endif // _DISHONORED_WWISE_SILENT_H_
