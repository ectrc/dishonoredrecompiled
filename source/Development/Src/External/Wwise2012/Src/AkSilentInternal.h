/*=============================================================================
	AkSilentInternal.h - shared state of the silent Wwise 2012.1 backend.

	DISHONORED(bringup): private to source/Development/Src/External/Wwise2012/Src; the game side sees
	only include/DishonoredWwiseSilent.h. Everything here is plain C++ and the CRT: the backend must not
	depend on Core/Engine, because it stands in for a third-party static library.
=============================================================================*/
#pragma once

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCallback.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkQueryParameters.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <DishonoredWwiseSilent.h>

#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace AkSilent
{
	typedef std::wstring String;

	/**
	 * Wwise's name hash. DISHONORED(written): the 2012 PDB carries AK::FNVHash<AK::Hash32>, so the id is
	 * a 32-bit FNV hash of the lowercased name; the variant and constants are pinned against the retail
	 * content rather than assumed. FNV-1 (multiply then xor) with basis 2166136261 and prime 16777619
	 * reproduces Init.bnk's own BKHD bank id 0x50c63a23 for "Init" and the bank id inside all 311
	 * CookedPCConsole .pck packages (build/agentAN/bank_stid.py, 12/12 spot-checked then swept);
	 * FNV-1a (xor then multiply) matches none of them.
	 */
	AkUInt32 NameHash( const wchar_t * name );
	AkUInt32 NameHash( const char * name );

	/** Remembers name -> id so the log and DishonoredWwise::NameForID can print names. */
	AkUInt32 RegisterName( const wchar_t * name );
	AkUInt32 RegisterName( const char * name );

	void Logf( const wchar_t * fmt, ... );
	DishonoredWwise::Stats & MutableStats();

	/** One entry per LoadBank / PrepareBank the game asked for. */
	struct Bank
	{
		String		Name;
		AkBankID	ID;
		AkUInt32	HeaderVersion;		///< BKHD version read out of the file, 0 when it was not read
		AkBankID	HeaderBankID;		///< BKHD bank id, to cross-check the name hash
		AkInt64		FileSize;
		bool		bPrepared;			///< came in through PrepareBank, so metadata only
		bool		bFileFound;
	};

	struct GameObject
	{
		String				Name;
		AkSoundPosition		Position;
		AkUInt32			ListenerMask;
		AkReal32			AttenuationScale;
		AkReal32			ObstructionLevel;
		AkReal32			OcclusionLevel;
		AkReal32			DryLevel;
	};

	/** One live PostEvent. The backend has no voices, so an event "plays" until it is stopped. */
	struct Voice
	{
		AkPlayingID		ID;
		AkUniqueID		EventID;
		AkGameObjectID	GameObject;
		AkUInt32		Flags;
		AkCallbackFunc	Callback;
		void *			Cookie;
	};

	struct Engine
	{
		bool									bSoundEngineInit;
		bool									bMemoryMgrInit;
		bool									bMusicEngineInit;
		AkPlayingID								NextPlayingID;
		AkPanningRule							PanningRule;
		AkChannelMask							SpeakerConfig;
		AkReal32								VolumeThreshold;
		AkUInt16								MaxNumVoices;
		AkListenerPosition						Listeners[8];
		std::map<AkGameObjectID, GameObject>	GameObjects;
		std::map<AkBankID, Bank>				Banks;
		std::map<AkPlayingID, Voice>			Voices;
		std::map<AkRtpcID, AkRtpcValue>			GlobalRtpc;
		std::map<AkSwitchGroupID, AkSwitchStateID>	Switches;	///< global part; per-object in ObjectSwitches
		std::map<AkStateGroupID, AkStateID>		States;
		std::map<AkUInt64, AkSwitchStateID>		ObjectSwitches;	///< (gameObj << 32) | switchGroup
		std::vector<AkGlobalCallbackFunc>		GlobalCallbacks;
		std::vector<AkGlobalCallbackFunc>		BehaviouralExtensions;
	};

	Engine & Get();

	AKRESULT AddCallback( std::vector<AkGlobalCallbackFunc> & list, AkGlobalCallbackFunc callback );
	AKRESULT RemoveCallback( std::vector<AkGlobalCallbackFunc> & list, AkGlobalCallbackFunc callback );

	/** Reads the BKHD chunk of a bank through the game's registered low-level IO hook. */
	bool ReadBankHeader( const wchar_t * bankFileName, AkUInt32 & outVersion, AkBankID & outBankID, AkInt64 & outFileSize );
}
