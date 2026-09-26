/*=============================================================================
	AkSilentSoundEngine.cpp - AK::SoundEngine on bookkeeping instead of a DSP graph.

	DISHONORED(bringup): what the backend does and does not do, so the next wave knows exactly where the
	line is.

	It DOES:
	  * hand out the real Wwise ids: GetIDFromString is the verified FNV-1 name hash (AkSilentInternal.h),
	    so every event, bank, RTPC, switch and state id the game computes matches the retail bank content;
	  * open every bank through the game's own IAkFileLocationResolver / IAkIOHookBlocking and read its
	    BKHD chunk, so a LoadBank either reports the file's real bank version and id or reports the file
	    as missing - that is what turns the log into evidence;
	  * hand out monotonic playing ids from PostEvent and remember which game object, event and callback
	    each one belongs to, then fire AK_EndOfEvent on StopAll / StopPlayingID /
	    ExecuteActionOnEvent(Stop) / Term, because USeqAct_AkPostEvent and UDishonoredAudioSystem's
	    end-of-event queue only finish when that callback arrives;
	  * record listener and game-object positions, the active listener mask, obstruction/occlusion,
	    environment values and the last value of every RTPC, switch, state and trigger, and answer the
	    AK::SoundEngine::Query getters from that;
	  * keep the registered game objects so UnregisterGameObj / UnregisterAllGameObj behave.

	It does NOT: decode a single byte of Vorbis media, evaluate any bank logic (an event's action list,
	attenuation curves, RTPC curves, state/switch containers, the music hierarchy), mix or output audio.
	Query::QueryAudioObjectIDs therefore returns no objects, so UAkEvent::ComputeMaxRadius keeps -1 and
	every event is treated as audible everywhere. Reaching real audio needs the licensed runtime; see
	resources/docs/agents/agentAN.md.
=============================================================================*/
#include "AkSilentInternal.h"

namespace AkSilent
{
	// Declared here because only this unit needs them; defined in AkSilentStreamMgr.cpp.
	AK::StreamMgr::IAkIOHookBlocking * BlockingHook();
	AK::StreamMgr::IAkFileLocationResolver * Resolver();

	/** AKCOMPANYID_AUDIOKINETIC / AKCODECID_BANK: the flags retail's own hook keys its lookups on. */
	static const AkUInt32 kCompanyAudiokinetic = 0;
	static const AkUInt32 kCodecBank = 0;

	bool ReadBankHeader( const wchar_t * bankFileName, AkUInt32 & outVersion, AkBankID & outBankID, AkInt64 & outFileSize )
	{
		outVersion = 0;
		outBankID = AK_INVALID_BANK_ID;
		outFileSize = 0;

		AK::StreamMgr::IAkFileLocationResolver * resolver = Resolver();
		AK::StreamMgr::IAkIOHookBlocking * hook = BlockingHook();
		if( !resolver || !hook )
		{
			return false;
		}

		AkFileSystemFlags flags;
		memset( &flags, 0, sizeof( flags ) );
		flags.uCompanyID = kCompanyAudiokinetic;
		flags.uCodecID = kCodecBank;

		AkFileDesc desc;
		memset( &desc, 0, sizeof( desc ) );
		bool bSyncOpen = true;
		if( resolver->Open( bankFileName, AK_OpenModeRead, &flags, bSyncOpen, desc ) != AK_Success )
		{
			return false;
		}
		outFileSize = desc.iFileSize;

		// BKHD: 'BKHD' + chunk size + bank generation + bank id. One aligned read covers it.
		const AkUInt32 blockSize = hook->GetBlockSize( desc );
		const AkUInt32 readSize = ( blockSize > 16 ) ? ( ( 16 + blockSize - 1 ) / blockSize ) * blockSize : 16;
		AkUInt8 * buffer = (AkUInt8 *)AK::AllocHook( readSize );
		bool bOk = false;
		if( buffer )
		{
			AkIoHeuristics heuristics;
			heuristics.fDeadline = 0.f;
			heuristics.priority = AK_DEFAULT_BANK_IO_PRIORITY;
			AkIOTransferInfo transfer;
			transfer.uFilePosition = 0;
			transfer.uBufferSize = readSize;
			transfer.uRequestedSize = readSize;
			if( hook->Read( desc, heuristics, buffer, transfer ) == AK_Success &&
				buffer[0] == 'B' && buffer[1] == 'K' && buffer[2] == 'H' && buffer[3] == 'D' )
			{
				memcpy( &outVersion, buffer + 8, 4 );
				memcpy( &outBankID, buffer + 12, 4 );
				bOk = true;
			}
			AK::FreeHook( buffer );
		}
		hook->Close( desc );
		return bOk;
	}

	/** Fires AK_EndOfEvent for one playing id and drops it. */
	static void EndPlaying( AkPlayingID id )
	{
		Engine & e = Get();
		std::map<AkPlayingID, Voice>::iterator it = e.Voices.find( id );
		if( it == e.Voices.end() )
		{
			return;
		}
		const Voice entry = it->second;
		e.Voices.erase( it );
		if( entry.Callback && ( entry.Flags & AK_EndOfEvent ) )
		{
			AkEventCallbackInfo info;
			info.pCookie = entry.Cookie;
			info.gameObjID = entry.GameObject;
			info.playingID = entry.ID;
			info.eventID = entry.EventID;
			entry.Callback( AK_EndOfEvent, &info );
		}
	}

	static void EndAllPlaying( AkGameObjectID gameObj )
	{
		Engine & e = Get();
		std::vector<AkPlayingID> doomed;
		for( std::map<AkPlayingID, Voice>::const_iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
		{
			if( gameObj == AK_INVALID_GAME_OBJECT || it->second.GameObject == gameObj )
			{
				doomed.push_back( it->first );
			}
		}
		for( size_t i = 0; i < doomed.size(); ++i )
		{
			EndPlaying( doomed[ i ] );
		}
	}

	static AkPlayingID PostEventCommon( AkUniqueID eventID, AkGameObjectID gameObj, AkUInt32 flags, AkCallbackFunc callback, void * cookie, AkPlayingID requestedID )
	{
		Engine & e = Get();
		if( !e.bSoundEngineInit )
		{
			return AK_INVALID_PLAYING_ID;
		}
		Voice entry;
		entry.ID = requestedID ? requestedID : e.NextPlayingID++;
		entry.EventID = eventID;
		entry.GameObject = gameObj;
		entry.Flags = flags;
		entry.Callback = callback;
		entry.Cookie = cookie;
		e.Voices[ entry.ID ] = entry;
		++MutableStats().EventsPosted;

		const AkOSChar * name = DishonoredWwise::NameForID( eventID );
		Logf( L"PostEvent %s (id 0x%08x) on game object %u -> playing id %u", name ? name : L"<id only>",
			eventID, (AkUInt32)gameObj, entry.ID );
		return entry.ID;
	}

	/** The bank entry LoadBank / PrepareBank produce; shared so both log the same way. */
	static AKRESULT LoadBankCommon( const wchar_t * bankName, AkBankID & outBankID, bool bPrepareOnly )
	{
		Engine & e = Get();
		if( !e.bSoundEngineInit )
		{
			return AK_Fail;
		}
		Bank bank;
		bank.Name = bankName ? bankName : L"";
		bank.ID = RegisterName( bank.Name.c_str() );
		bank.HeaderVersion = 0;
		bank.HeaderBankID = AK_INVALID_BANK_ID;
		bank.FileSize = 0;
		bank.bPrepared = bPrepareOnly;
		bank.bFileFound = false;
		outBankID = bank.ID;

		String fileName = bank.Name + L".bnk";
		bank.bFileFound = ReadBankHeader( fileName.c_str(), bank.HeaderVersion, bank.HeaderBankID, bank.FileSize );

		e.Banks[ bank.ID ] = bank;
		if( bank.bFileFound )
		{
			if( bPrepareOnly )
			{
				++MutableStats().BanksPrepared;
			}
			else
			{
				++MutableStats().BanksLoaded;
			}
			Logf( L"%s bank %s: id 0x%08x, BKHD version %u, bank id 0x%08x, %I64d bytes%s",
				bPrepareOnly ? L"Prepared" : L"Loaded", bank.Name.c_str(), bank.ID, bank.HeaderVersion,
				bank.HeaderBankID, bank.FileSize,
				bank.HeaderBankID == bank.ID ? L"" : L" (WARNING: bank id does not match the name hash)" );
			if( bank.HeaderVersion != AK_BANK_READER_VERSION )
			{
				Logf( L"bank %s is version %u, this generation reads %u", bank.Name.c_str(), bank.HeaderVersion, AK_BANK_READER_VERSION );
				return AK_WrongBankVersion;
			}
			return AK_Success;
		}
		++MutableStats().BanksMissing;
		Logf( L"bank %s (id 0x%08x) not found by the low-level IO hook", bank.Name.c_str(), bank.ID );
		return AK_FileNotFound;
	}

	AKRESULT AddCallback( std::vector<AkGlobalCallbackFunc> & list, AkGlobalCallbackFunc callback )
	{
		for( size_t i = 0; i < list.size(); ++i )
		{
			if( list[ i ] == callback )
			{
				return AK_Success;
			}
		}
		list.push_back( callback );
		return AK_Success;
	}

	AKRESULT RemoveCallback( std::vector<AkGlobalCallbackFunc> & list, AkGlobalCallbackFunc callback )
	{
		for( size_t i = 0; i < list.size(); ++i )
		{
			if( list[ i ] == callback )
			{
				list.erase( list.begin() + i );
				return AK_Success;
			}
		}
		return AK_Fail;
	}

	static String Widen( const char * narrow )
	{
		String wide;
		if( narrow )
		{
			for( const char * p = narrow; *p; ++p )
			{
				wide.push_back( (wchar_t)(unsigned char)*p );
			}
		}
		return wide;
	}
}

namespace DishonoredWwise
{
	void FlushEndOfEventCallbacks()
	{
		AkSilent::EndAllPlaying( AK_INVALID_GAME_OBJECT );
	}
}

namespace AK
{
	namespace SoundEngine
	{
		// -- lifetime ------------------------------------------------------------------------------

		bool IsInitialized()
		{
			return AkSilent::Get().bSoundEngineInit;
		}

		void GetDefaultInitSettings( AkInitSettings & out_settings )
		{
			memset( &out_settings, 0, sizeof( out_settings ) );
			out_settings.uMaxNumPaths = 255;
			out_settings.uMaxNumTransitions = 128;
			out_settings.uDefaultPoolSize = 2 * 1024 * 1024;
			out_settings.fDefaultPoolRatioThreshold = 0.f;
			out_settings.uCommandQueueSize = 256 * 1024;
			out_settings.uPrepareEventMemoryPoolID = AK_INVALID_POOL_ID;
			out_settings.bEnableGameSyncPreparation = false;
			out_settings.uContinuousPlaybackLookAhead = 1;
			out_settings.uMonitorPoolSize = 0;
			out_settings.uMonitorQueuePoolSize = 0;
		}

		void GetDefaultPlatformInitSettings( AkPlatformInitSettings & out_settings )
		{
			memset( &out_settings, 0, sizeof( out_settings ) );
			out_settings.uLEngineDefaultPoolSize = 8 * 1024 * 1024;
			out_settings.fLEngineDefaultPoolRatioThreshold = 0.f;
			out_settings.uNumRefillsInVoice = 4;
			out_settings.eAudioQuality = AkSoundQuality_High;
			out_settings.uChannelMask = AK_SPEAKER_SETUP_5POINT1;
			out_settings.bGlobalFocus = true;
		}

		AKRESULT Init( AkInitSettings * in_pSettings, AkPlatformInitSettings * in_pPlatformSettings )
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( !e.bMemoryMgrInit )
			{
				return AK_MemManagerNotInitialized;
			}
			if( !IAkStreamMgr::Get() )
			{
				return AK_StreamMgrNotInitialized;
			}
			e.bSoundEngineInit = true;
			if( in_pPlatformSettings )
			{
				e.SpeakerConfig = in_pPlatformSettings->uChannelMask ? in_pPlatformSettings->uChannelMask : AK_SPEAKER_SETUP_5POINT1;
			}
			AkSilent::Logf( L"SoundEngine::Init: silent backend (no output device), command queue %u, speaker mask 0x%x",
				in_pSettings ? in_pSettings->uCommandQueueSize : 0, e.SpeakerConfig );
			return AK_Success;
		}

		void Term()
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( !e.bSoundEngineInit )
			{
				return;
			}
			AkSilent::EndAllPlaying( AK_INVALID_GAME_OBJECT );
			const DishonoredWwise::Stats & stats = DishonoredWwise::GetStats();
			AkSilent::Logf( L"SoundEngine::Term: %u banks loaded, %u prepared, %u missing, %u events posted, %u RTPC / %u switch / %u state sets",
				stats.BanksLoaded, stats.BanksPrepared, stats.BanksMissing, stats.EventsPosted,
				stats.RtpcSets, stats.SwitchSets, stats.StateSets );
			e.Banks.clear();
			e.GameObjects.clear();
			e.bSoundEngineInit = false;
		}

		AKRESULT RenderAudio()
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( !e.bSoundEngineInit )
			{
				return AK_Fail;
			}
			// The behavioural extensions and global callbacks are what retail's DisAkGlobalCallbackFunc
			// rides on, so they run once per RenderAudio like the real engine's audio frame.
			for( size_t i = 0; i < e.BehaviouralExtensions.size(); ++i )
			{
				e.BehaviouralExtensions[ i ]( false );
			}
			for( size_t i = 0; i < e.GlobalCallbacks.size(); ++i )
			{
				e.GlobalCallbacks[ i ]( false );
			}
			return AK_Success;
		}

		// -- plug-ins ------------------------------------------------------------------------------

		AKRESULT RegisterPlugin( AkPluginType in_eType, AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, AkCreatePluginCallback, AkCreateParamCallback )
		{
			// DISHONORED(bringup): recorded, not instantiated. Retail registers eight effect plug-ins and
			// one source plug-in here (UAkAudioDevice::EnsureInitialized, 2013 rva 0x5b1430); without a
			// DSP graph there is nothing for them to process.
			AkSilent::Logf( L"SoundEngine::RegisterPlugin: type %d, company %u, plug-in %u (recorded, no DSP graph)",
				(int)in_eType, in_ulCompanyID, in_ulPluginID );
			return AK_Success;
		}

		AKRESULT RegisterCodec( AkUInt32 in_ulCompanyID, AkUInt32 in_ulCodecID, AkCreateFileSourceCallback, AkCreateBankSourceCallback )
		{
			AkSilent::Logf( L"SoundEngine::RegisterCodec: company %u, codec %u (recorded, no media decoding)", in_ulCompanyID, in_ulCodecID );
			return AK_Success;
		}

		// -- game objects --------------------------------------------------------------------------

		AKRESULT RegisterGameObj( AkGameObjectID in_gameObjectID, const char * in_pszObjName )
		{
			if( in_gameObjectID == AK_INVALID_GAME_OBJECT )
			{
				return AK_InvalidParameter;
			}
			AkSilent::GameObject obj;
			obj.Name = AkSilent::Widen( in_pszObjName );
			memset( &obj.Position, 0, sizeof( obj.Position ) );
			obj.ListenerMask = 0xFFFFFFFF;
			obj.AttenuationScale = 1.f;
			obj.ObstructionLevel = 0.f;
			obj.OcclusionLevel = 0.f;
			obj.DryLevel = 1.f;
			AkSilent::Get().GameObjects[ in_gameObjectID ] = obj;
			return AK_Success;
		}

		AKRESULT UnregisterGameObj( AkGameObjectID in_gameObjectID )
		{
			AkSilent::EndAllPlaying( in_gameObjectID );
			return AkSilent::Get().GameObjects.erase( in_gameObjectID ) ? AK_Success : AK_Fail;
		}

		AKRESULT UnregisterAllGameObj()
		{
			AkSilent::EndAllPlaying( AK_INVALID_GAME_OBJECT );
			AkSilent::Get().GameObjects.clear();
			return AK_Success;
		}

		// -- positioning ---------------------------------------------------------------------------

		AKRESULT SetPosition( AkGameObjectID in_gameObjectID, const AkSoundPosition & in_Position, AkUInt32 )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_gameObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.Position = in_Position;
			return AK_Success;
		}

		AKRESULT SetMultiplePositions( AkGameObjectID in_gameObjectID, const AkSoundPosition * in_pPositions, AkUInt16 in_NumPositions, MultiPositionType )
		{
			return ( in_pPositions && in_NumPositions ) ? SetPosition( in_gameObjectID, in_pPositions[ 0 ] ) : AK_InvalidParameter;
		}

		AKRESULT SetListenerPosition( const AkListenerPosition & in_Position, AkUInt32 in_uIndex )
		{
			if( in_uIndex >= 8 )
			{
				return AK_InvalidParameter;
			}
			AkSilent::Get().Listeners[ in_uIndex ] = in_Position;
			return AK_Success;
		}

		AKRESULT SetActiveListeners( AkGameObjectID in_gameObjectID, AkUInt32 in_uListenerMask )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_gameObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.ListenerMask = in_uListenerMask;
			return AK_Success;
		}

		AKRESULT SetAttenuationScalingFactor( AkGameObjectID in_gameObjectID, AkReal32 in_fAttenuationScalingFactor )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_gameObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.AttenuationScale = in_fAttenuationScalingFactor;
			return AK_Success;
		}

		AKRESULT SetListenerScalingFactor( AkUInt32, AkReal32 ) { return AK_Success; }
		AKRESULT SetListenerSpatialization( AkUInt32, bool, AkSpeakerVolumes * ) { return AK_Success; }
		AKRESULT SetListenerPipeline( AkUInt32, bool, bool ) { return AK_Success; }

		// -- game syncs ----------------------------------------------------------------------------

		AKRESULT SetRTPCValue( AkRtpcID in_rtpcID, AkRtpcValue in_value, AkGameObjectID, AkTimeMs, AkCurveInterpolation )
		{
			AkSilent::Get().GlobalRtpc[ in_rtpcID ] = in_value;
			++AkSilent::MutableStats().RtpcSets;
			return AK_Success;
		}

		AKRESULT SetRTPCValue( const AkOSChar * in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			return SetRTPCValue( AkSilent::RegisterName( in_pszRtpcName ), in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve );
		}

		AKRESULT SetRTPCValue( const char * in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			return SetRTPCValue( AkSilent::RegisterName( in_pszRtpcName ), in_value, in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve );
		}

		AKRESULT ResetRTPCValue( AkRtpcID in_rtpcID, AkGameObjectID, AkTimeMs, AkCurveInterpolation )
		{
			AkSilent::Get().GlobalRtpc.erase( in_rtpcID );
			return AK_Success;
		}

		AKRESULT ResetRTPCValue( const AkOSChar * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			return ResetRTPCValue( AkSilent::RegisterName( in_pszRtpcName ), in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve );
		}

		AKRESULT ResetRTPCValue( const char * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve )
		{
			return ResetRTPCValue( AkSilent::RegisterName( in_pszRtpcName ), in_gameObjectID, in_uValueChangeDuration, in_eFadeCurve );
		}

		AKRESULT SetSwitch( AkSwitchGroupID in_switchGroup, AkSwitchStateID in_switchState, AkGameObjectID in_gameObjectID )
		{
			AkSilent::Engine & e = AkSilent::Get();
			e.Switches[ in_switchGroup ] = in_switchState;
			if( in_gameObjectID != AK_INVALID_GAME_OBJECT )
			{
				e.ObjectSwitches[ ( (AkUInt64)in_gameObjectID << 32 ) | in_switchGroup ] = in_switchState;
			}
			++AkSilent::MutableStats().SwitchSets;
			return AK_Success;
		}

		AKRESULT SetSwitch( const AkOSChar * in_pszSwitchGroup, const AkOSChar * in_pszSwitchState, AkGameObjectID in_gameObjectID )
		{
			const AkSwitchGroupID group = AkSilent::RegisterName( in_pszSwitchGroup );
			const AkSwitchStateID state = AkSilent::RegisterName( in_pszSwitchState );
			AkSilent::Logf( L"SetSwitch %s = %s on game object %u", in_pszSwitchGroup, in_pszSwitchState, (AkUInt32)in_gameObjectID );
			return SetSwitch( group, state, in_gameObjectID );
		}

		AKRESULT SetSwitch( const char * in_pszSwitchGroup, const char * in_pszSwitchState, AkGameObjectID in_gameObjectID )
		{
			return SetSwitch( AkSilent::RegisterName( in_pszSwitchGroup ), AkSilent::RegisterName( in_pszSwitchState ), in_gameObjectID );
		}

		AKRESULT SetState( AkStateGroupID in_stateGroup, AkStateID in_state )
		{
			AkSilent::Get().States[ in_stateGroup ] = in_state;
			++AkSilent::MutableStats().StateSets;
			return AK_Success;
		}

		AKRESULT SetState( const AkOSChar * in_pszStateGroup, const AkOSChar * in_pszState )
		{
			const AkStateGroupID group = AkSilent::RegisterName( in_pszStateGroup );
			const AkStateID state = AkSilent::RegisterName( in_pszState );
			AkSilent::Logf( L"SetState %s = %s", in_pszStateGroup, in_pszState );
			return SetState( group, state );
		}

		AKRESULT SetState( const char * in_pszStateGroup, const char * in_pszState )
		{
			return SetState( AkSilent::RegisterName( in_pszStateGroup ), AkSilent::RegisterName( in_pszState ) );
		}

		AKRESULT PostTrigger( AkTriggerID, AkGameObjectID )
		{
			++AkSilent::MutableStats().TriggersPosted;
			return AK_Success;
		}

		AKRESULT PostTrigger( const AkOSChar * in_pszTrigger, AkGameObjectID in_gameObjectID )
		{
			AkSilent::Logf( L"PostTrigger %s on game object %u", in_pszTrigger, (AkUInt32)in_gameObjectID );
			return PostTrigger( AkSilent::RegisterName( in_pszTrigger ), in_gameObjectID );
		}

		AKRESULT PostTrigger( const char * in_pszTrigger, AkGameObjectID in_gameObjectID )
		{
			return PostTrigger( AkSilent::RegisterName( in_pszTrigger ), in_gameObjectID );
		}

		// -- events --------------------------------------------------------------------------------

		AkPlayingID PostEvent( AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32, AkExternalSourceInfo *, AkPlayingID in_PlayingID )
		{
			return AkSilent::PostEventCommon( in_eventID, in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_PlayingID );
		}

		AkPlayingID PostEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_cExternals, AkExternalSourceInfo * in_pExternalSources, AkPlayingID in_PlayingID )
		{
			return PostEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID );
		}

		AkPlayingID PostEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_cExternals, AkExternalSourceInfo * in_pExternalSources, AkPlayingID in_PlayingID )
		{
			return PostEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_uFlags, in_pfnCallback, in_pCookie, in_cExternals, in_pExternalSources, in_PlayingID );
		}

		AKRESULT ExecuteActionOnEvent( AkUniqueID in_eventID, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID, AkTimeMs, AkCurveInterpolation, AkPlayingID in_PlayingID )
		{
			AkSilent::Engine & e = AkSilent::Get();
			if( in_ActionType != AkActionOnEventType_Stop && in_ActionType != AkActionOnEventType_Break )
			{
				return AK_Success;			// pause / resume have nothing to act on without voices
			}
			std::vector<AkPlayingID> doomed;
			for( std::map<AkPlayingID, AkSilent::Voice>::const_iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
			{
				const bool bMatchesEvent = ( in_eventID == AK_INVALID_UNIQUE_ID ) || ( it->second.EventID == in_eventID );
				const bool bMatchesObject = ( in_gameObjectID == AK_INVALID_GAME_OBJECT ) || ( it->second.GameObject == in_gameObjectID );
				const bool bMatchesPlaying = ( in_PlayingID == AK_INVALID_PLAYING_ID ) || ( it->first == in_PlayingID );
				if( bMatchesEvent && bMatchesObject && bMatchesPlaying )
				{
					doomed.push_back( it->first );
				}
			}
			for( size_t i = 0; i < doomed.size(); ++i )
			{
				AkSilent::EndPlaying( doomed[ i ] );
			}
			return AK_Success;
		}

		AKRESULT ExecuteActionOnEvent( const AkOSChar * in_pszEventName, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve, AkPlayingID in_PlayingID )
		{
			return ExecuteActionOnEvent( AkSilent::RegisterName( in_pszEventName ), in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID );
		}

		AKRESULT ExecuteActionOnEvent( const char * in_pszEventName, AkActionOnEventType in_ActionType, AkGameObjectID in_gameObjectID, AkTimeMs in_uTransitionDuration, AkCurveInterpolation in_eFadeCurve, AkPlayingID in_PlayingID )
		{
			return ExecuteActionOnEvent( AkSilent::RegisterName( in_pszEventName ), in_ActionType, in_gameObjectID, in_uTransitionDuration, in_eFadeCurve, in_PlayingID );
		}

		void StopAll( AkGameObjectID in_gameObjectID )
		{
			AkSilent::EndAllPlaying( in_gameObjectID );
		}

		void StopPlayingID( AkPlayingID in_playingID, AkTimeMs, AkCurveInterpolation )
		{
			AkSilent::EndPlaying( in_playingID );
		}

		AKRESULT SeekOnEvent( AkUniqueID, AkGameObjectID, AkTimeMs, bool ) { return AK_Success; }
		AKRESULT SeekOnEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_iPosition, in_b ); }
		AKRESULT SeekOnEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkTimeMs in_iPosition, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_iPosition, in_b ); }
		AKRESULT SeekOnEvent( AkUniqueID, AkGameObjectID, AkReal32, bool ) { return AK_Success; }
		AKRESULT SeekOnEvent( const AkOSChar * in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_fPercent, in_b ); }
		AKRESULT SeekOnEvent( const char * in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_b ) { return SeekOnEvent( AkSilent::RegisterName( in_pszEventName ), in_gameObjectID, in_fPercent, in_b ); }

		void CancelEventCallbackCookie( void * in_pCookie )
		{
			AkSilent::Engine & e = AkSilent::Get();
			for( std::map<AkPlayingID, AkSilent::Voice>::iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
			{
				if( it->second.Cookie == in_pCookie )
				{
					it->second.Callback = NULL;
				}
			}
		}

		void CancelEventCallback( AkPlayingID in_playingID )
		{
			AkSilent::Engine & e = AkSilent::Get();
			std::map<AkPlayingID, AkSilent::Voice>::iterator it = e.Voices.find( in_playingID );
			if( it != e.Voices.end() )
			{
				it->second.Callback = NULL;
			}
		}

		void CancelBankCallbackCookie( void * ) {}

		AKRESULT GetSourcePlayPosition( AkPlayingID in_PlayingID, AkTimeMs * out_puPosition, bool )
		{
			if( !out_puPosition )
			{
				return AK_InvalidParameter;
			}
			*out_puPosition = 0;
			return AkSilent::Get().Voices.count( in_PlayingID ) ? AK_Success : AK_Fail;
		}

		// -- name hashing --------------------------------------------------------------------------

		AkUInt32 GetIDFromString( const AkOSChar * in_pszString )
		{
			return AkSilent::RegisterName( in_pszString );
		}

		AkUInt32 GetIDFromString( const char * in_pszString )
		{
			return AkSilent::RegisterName( in_pszString );
		}

		AkBankID GetBankIDFromString( const AkOSChar * in_pszString )
		{
			return AkSilent::RegisterName( in_pszString );
		}

		// -- banks ---------------------------------------------------------------------------------

		AKRESULT LoadBank( const AkOSChar * in_pszString, AkMemPoolId, AkBankID & out_bankID )
		{
			return AkSilent::LoadBankCommon( in_pszString, out_bankID, false );
		}

		AKRESULT LoadBank( const char * in_pszString, AkMemPoolId in_memPoolId, AkBankID & out_bankID )
		{
			const AkSilent::String wide = AkSilent::Widen( in_pszString );
			return LoadBank( wide.c_str(), in_memPoolId, out_bankID );
		}

		AKRESULT LoadBank( AkBankID in_bankID, AkMemPoolId )
		{
			const AkOSChar * name = DishonoredWwise::NameForID( in_bankID );
			if( !name )
			{
				return AK_UnknownBankID;
			}
			AkBankID ignored = AK_INVALID_BANK_ID;
			return AkSilent::LoadBankCommon( name, ignored, false );
		}

		AKRESULT LoadBank( const void * in_pInMemoryBankPtr, AkUInt32 in_uInMemoryBankSize, AkBankID & out_bankID )
		{
			// An in-memory bank needs no IO hook: read BKHD straight out of the buffer.
			out_bankID = AK_INVALID_BANK_ID;
			const AkUInt8 * bytes = (const AkUInt8 *)in_pInMemoryBankPtr;
			if( !bytes || in_uInMemoryBankSize < 16 || bytes[0] != 'B' || bytes[1] != 'K' || bytes[2] != 'H' || bytes[3] != 'D' )
			{
				return AK_InvalidFile;
			}
			AkUInt32 version = 0;
			memcpy( &version, bytes + 8, 4 );
			memcpy( &out_bankID, bytes + 12, 4 );
			AkSilent::Bank bank;
			bank.ID = out_bankID;
			bank.HeaderBankID = out_bankID;
			bank.HeaderVersion = version;
			bank.FileSize = (AkInt64)in_uInMemoryBankSize;
			bank.bPrepared = false;
			bank.bFileFound = true;
			AkSilent::Get().Banks[ out_bankID ] = bank;
			++AkSilent::MutableStats().BanksLoaded;
			AkSilent::Logf( L"Loaded in-memory bank id 0x%08x, BKHD version %u, %u bytes", out_bankID, version, in_uInMemoryBankSize );
			return version == AK_BANK_READER_VERSION ? AK_Success : AK_WrongBankVersion;
		}

		AKRESULT LoadBank( const AkOSChar * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkMemPoolId in_memPoolId, AkBankID & out_bankID )
		{
			const AKRESULT result = LoadBank( in_pszString, in_memPoolId, out_bankID );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( out_bankID, result, in_memPoolId, in_pCookie );
			}
			return result;
		}

		AKRESULT LoadBank( const char * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkMemPoolId in_memPoolId, AkBankID & out_bankID )
		{
			const AkSilent::String wide = AkSilent::Widen( in_pszString );
			return LoadBank( wide.c_str(), in_pfnBankCallback, in_pCookie, in_memPoolId, out_bankID );
		}

		AKRESULT LoadBank( AkBankID in_bankID, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkMemPoolId in_memPoolId )
		{
			const AKRESULT result = LoadBank( in_bankID, in_memPoolId );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( in_bankID, result, in_memPoolId, in_pCookie );
			}
			return result;
		}

		AKRESULT LoadBank( const void * in_pInMemoryBankPtr, AkUInt32 in_uInMemoryBankSize, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankID & out_bankID )
		{
			const AKRESULT result = LoadBank( in_pInMemoryBankPtr, in_uInMemoryBankSize, out_bankID );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( out_bankID, result, AK_INVALID_POOL_ID, in_pCookie );
			}
			return result;
		}

		AKRESULT UnloadBank( AkBankID in_bankID, AkMemPoolId * out_pMemPoolId )
		{
			if( out_pMemPoolId )
			{
				*out_pMemPoolId = AK_INVALID_POOL_ID;
			}
			AkSilent::Engine & e = AkSilent::Get();
			if( !e.Banks.erase( in_bankID ) )
			{
				return AK_UnknownBankID;
			}
			const AkOSChar * name = DishonoredWwise::NameForID( in_bankID );
			AkSilent::Logf( L"Unloaded bank %s (id 0x%08x)", name ? name : L"<id only>", in_bankID );
			return AK_Success;
		}

		AKRESULT UnloadBank( const AkOSChar * in_pszString, AkMemPoolId * out_pMemPoolId )
		{
			return UnloadBank( AkSilent::RegisterName( in_pszString ), out_pMemPoolId );
		}

		AKRESULT UnloadBank( const char * in_pszString, AkMemPoolId * out_pMemPoolId )
		{
			return UnloadBank( AkSilent::RegisterName( in_pszString ), out_pMemPoolId );
		}

		AKRESULT UnloadBank( const AkOSChar * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			const AkBankID id = AkSilent::RegisterName( in_pszString );
			const AKRESULT result = UnloadBank( id, (AkMemPoolId *)NULL );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( id, result, AK_INVALID_POOL_ID, in_pCookie );
			}
			return result;
		}

		AKRESULT UnloadBank( const char * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			const AkSilent::String wide = AkSilent::Widen( in_pszString );
			return UnloadBank( wide.c_str(), in_pfnBankCallback, in_pCookie );
		}

		AKRESULT UnloadBank( AkBankID in_bankID, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			const AKRESULT result = UnloadBank( in_bankID, (AkMemPoolId *)NULL );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( in_bankID, result, AK_INVALID_POOL_ID, in_pCookie );
			}
			return result;
		}

		AKRESULT ClearBanks()
		{
			AkSilent::Engine & e = AkSilent::Get();
			AkSilent::Logf( L"SoundEngine::ClearBanks: dropping %u banks", (AkUInt32)e.Banks.size() );
			AkSilent::EndAllPlaying( AK_INVALID_GAME_OBJECT );
			e.Banks.clear();
			return AK_Success;
		}

		AKRESULT SetBankLoadIOSettings( AkReal32, AkPriority ) { return AK_Success; }

		void DefaultBankCallbackFunc( AkUInt32, AKRESULT, AkMemPoolId, void * ) {}

		// -- prepare -------------------------------------------------------------------------------

		AKRESULT PrepareBank( PreparationType in_PreparationType, const AkOSChar * in_pszString, AkBankContent )
		{
			if( in_PreparationType == Preparation_Unload )
			{
				return UnloadBank( in_pszString, (AkMemPoolId *)NULL );
			}
			AkBankID ignored = AK_INVALID_BANK_ID;
			return AkSilent::LoadBankCommon( in_pszString, ignored, true );
		}

		AKRESULT PrepareBank( PreparationType in_PreparationType, const char * in_pszString, AkBankContent in_uFlags )
		{
			const AkSilent::String wide = AkSilent::Widen( in_pszString );
			return PrepareBank( in_PreparationType, wide.c_str(), in_uFlags );
		}

		AKRESULT PrepareBank( PreparationType in_PreparationType, AkBankID in_bankID, AkBankContent in_uFlags )
		{
			const AkOSChar * name = DishonoredWwise::NameForID( in_bankID );
			return name ? PrepareBank( in_PreparationType, name, in_uFlags ) : AK_UnknownBankID;
		}

		AKRESULT PrepareBank( PreparationType in_PreparationType, const AkOSChar * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankContent in_uFlags )
		{
			const AKRESULT result = PrepareBank( in_PreparationType, in_pszString, in_uFlags );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( AkSilent::NameHash( in_pszString ), result, AK_INVALID_POOL_ID, in_pCookie );
			}
			return result;
		}

		AKRESULT PrepareBank( PreparationType in_PreparationType, const char * in_pszString, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankContent in_uFlags )
		{
			const AkSilent::String wide = AkSilent::Widen( in_pszString );
			return PrepareBank( in_PreparationType, wide.c_str(), in_pfnBankCallback, in_pCookie, in_uFlags );
		}

		AKRESULT PrepareBank( PreparationType in_PreparationType, AkBankID in_bankID, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie, AkBankContent in_uFlags )
		{
			const AKRESULT result = PrepareBank( in_PreparationType, in_bankID, in_uFlags );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( in_bankID, result, AK_INVALID_POOL_ID, in_pCookie );
			}
			return result;
		}

		// DISHONORED(bringup): preparing an event means loading just that event's media out of an already
		// prepared bank. Without bank logic there is no media to pull, so the call is recorded and
		// succeeds - which is what the game needs to proceed.
		AKRESULT PrepareEvent( PreparationType, const AkOSChar ** in_ppszString, AkUInt32 in_uNumEvent )
		{
			for( AkUInt32 i = 0; in_ppszString && i < in_uNumEvent; ++i )
			{
				AkSilent::RegisterName( in_ppszString[ i ] );
			}
			return AK_Success;
		}

		AKRESULT PrepareEvent( PreparationType, const char ** in_ppszString, AkUInt32 in_uNumEvent )
		{
			for( AkUInt32 i = 0; in_ppszString && i < in_uNumEvent; ++i )
			{
				AkSilent::RegisterName( in_ppszString[ i ] );
			}
			return AK_Success;
		}

		AKRESULT PrepareEvent( PreparationType, AkUniqueID *, AkUInt32 ) { return AK_Success; }

		AKRESULT PrepareEvent( PreparationType in_PreparationType, const AkOSChar ** in_ppszString, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			const AKRESULT result = PrepareEvent( in_PreparationType, in_ppszString, in_uNumEvent );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( AK_INVALID_BANK_ID, result, AK_INVALID_POOL_ID, in_pCookie );
			}
			return result;
		}

		AKRESULT PrepareEvent( PreparationType in_PreparationType, const char ** in_ppszString, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			const AKRESULT result = PrepareEvent( in_PreparationType, in_ppszString, in_uNumEvent );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( AK_INVALID_BANK_ID, result, AK_INVALID_POOL_ID, in_pCookie );
			}
			return result;
		}

		AKRESULT PrepareEvent( PreparationType in_PreparationType, AkUniqueID * in_pEventID, AkUInt32 in_uNumEvent, AkBankCallbackFunc in_pfnBankCallback, void * in_pCookie )
		{
			const AKRESULT result = PrepareEvent( in_PreparationType, in_pEventID, in_uNumEvent );
			if( in_pfnBankCallback )
			{
				in_pfnBankCallback( AK_INVALID_BANK_ID, result, AK_INVALID_POOL_ID, in_pCookie );
			}
			return result;
		}

		AKRESULT PrepareGameSyncs( PreparationType, AkGroupType, const AkOSChar *, const AkOSChar **, AkUInt32 ) { return AK_Success; }
		AKRESULT PrepareGameSyncs( PreparationType, AkGroupType, const char *, const char **, AkUInt32 ) { return AK_Success; }
		AKRESULT PrepareGameSyncs( PreparationType, AkGroupType, AkUInt32, AkUInt32 *, AkUInt32 ) { return AK_Success; }
		AKRESULT ClearPreparedEvents() { return AK_Success; }

		// -- environments / obstruction ------------------------------------------------------------

		AKRESULT SetEffect( AkUniqueID, AkUInt32, AkUniqueID ) { return AK_Success; }
		AKRESULT SetEnvironmentVolume( AkAuxBusID, AkReal32 ) { return AK_Success; }
		AKRESULT SetEnvironmentVolumes( AkAuxBusID, const AkSpeakerVolumes & ) { return AK_Success; }
		AKRESULT BypassEnvironment( AkAuxBusID, bool ) { return AK_Success; }
		AKRESULT SetGameObjectEnvironmentsValues( AkGameObjectID, AkEnvironmentValue *, AkUInt32 ) { return AK_Success; }

		AKRESULT SetGameObjectDryLevelValue( AkGameObjectID in_gameObjectID, AkReal32 in_fControlValue )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_gameObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.DryLevel = in_fControlValue;
			return AK_Success;
		}

		AKRESULT SetObjectObstructionAndOcclusion( AkGameObjectID in_ObjectID, AkUInt32, AkReal32 in_fObstructionLevel, AkReal32 in_fOcclusionLevel )
		{
			std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
			std::map<AkGameObjectID, AkSilent::GameObject>::iterator it = objs.find( in_ObjectID );
			if( it == objs.end() )
			{
				return AK_Fail;
			}
			it->second.ObstructionLevel = in_fObstructionLevel;
			it->second.OcclusionLevel = in_fOcclusionLevel;
			return AK_Success;
		}

		// -- global settings -----------------------------------------------------------------------

		AkChannelMask GetSpeakerConfiguration() { return AkSilent::Get().SpeakerConfig; }
		AkPanningRule GetPanningRule() { return AkSilent::Get().PanningRule; }

		AKRESULT SetPanningRule( AkPanningRule in_ePanningRule )
		{
			AkSilent::Get().PanningRule = in_ePanningRule;
			return AK_Success;
		}

		AKRESULT SetMaxNumVoicesLimit( AkUInt16 in_maxNumberVoices )
		{
			AkSilent::Get().MaxNumVoices = in_maxNumberVoices;
			return AK_Success;
		}

		AKRESULT SetVolumeThreshold( AkReal32 in_fVolumeThresholdDB )
		{
			AkSilent::Get().VolumeThreshold = in_fVolumeThresholdDB;
			return AK_Success;
		}

		// -- global callbacks ----------------------------------------------------------------------

		AKRESULT RegisterGlobalCallback( AkGlobalCallbackFunc in_pCallback )
		{
			if( !in_pCallback )
			{
				return AK_InvalidParameter;
			}
			return AkSilent::AddCallback( AkSilent::Get().GlobalCallbacks, in_pCallback );
		}

		AKRESULT UnregisterGlobalCallback( AkGlobalCallbackFunc in_pCallback )
		{
			return AkSilent::RemoveCallback( AkSilent::Get().GlobalCallbacks, in_pCallback );
		}

		AKRESULT AddBehavioralExtension( AkGlobalCallbackFunc in_pCallback )
		{
			if( !in_pCallback )
			{
				return AK_InvalidParameter;
			}
			return AkSilent::AddCallback( AkSilent::Get().BehaviouralExtensions, in_pCallback );
		}

		AKRESULT RemoveBehavioralExtension( AkGlobalCallbackFunc in_pCallback )
		{
			return AkSilent::RemoveCallback( AkSilent::Get().BehaviouralExtensions, in_pCallback );
		}

		void AddExternalStateHandler( bool ( * )( AkStateGroupID, AkStateID ) ) {}

		// -- Query ---------------------------------------------------------------------------------

		namespace Query
		{
			AKRESULT GetPosition( AkGameObjectID in_GameObjectID, AkSoundPosition & out_rPosition )
			{
				std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
				std::map<AkGameObjectID, AkSilent::GameObject>::const_iterator it = objs.find( in_GameObjectID );
				if( it == objs.end() )
				{
					return AK_Fail;
				}
				out_rPosition = it->second.Position;
				return AK_Success;
			}

			AKRESULT GetListenerPosition( AkUInt32 in_uIndex, AkListenerPosition & out_rPosition )
			{
				if( in_uIndex >= 8 )
				{
					return AK_InvalidParameter;
				}
				out_rPosition = AkSilent::Get().Listeners[ in_uIndex ];
				return AK_Success;
			}

			AKRESULT GetListenerSpatialization( AkUInt32, bool & out_rbSpatialized, AkSpeakerVolumes & out_rVolumeOffsets )
			{
				out_rbSpatialized = true;
				memset( &out_rVolumeOffsets, 0, sizeof( out_rVolumeOffsets ) );
				return AK_Success;
			}

			AKRESULT GetActiveListeners( AkGameObjectID in_GameObjectID, AkUInt32 & out_ruListenerMask )
			{
				std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
				std::map<AkGameObjectID, AkSilent::GameObject>::const_iterator it = objs.find( in_GameObjectID );
				if( it == objs.end() )
				{
					return AK_Fail;
				}
				out_ruListenerMask = it->second.ListenerMask;
				return AK_Success;
			}

			bool GetIsGameObjectActive( AkGameObjectID in_GameObjectID )
			{
				AkSilent::Engine & e = AkSilent::Get();
				for( std::map<AkPlayingID, AkSilent::Voice>::const_iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
				{
					if( it->second.GameObject == in_GameObjectID )
					{
						return true;
					}
				}
				return false;
			}

			// DISHONORED(bringup): an attenuation radius is bank logic; without it every object reports -1,
			// which is exactly what UAkEvent::GetMaxRadius stores for "no attenuation".
			AkReal32 GetMaxRadius( AkGameObjectID ) { return -1.f; }

			AKRESULT GetRTPCValue( AkRtpcID in_rtpcID, AkGameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType )
			{
				std::map<AkRtpcID, AkRtpcValue> & rtpc = AkSilent::Get().GlobalRtpc;
				std::map<AkRtpcID, AkRtpcValue>::const_iterator it = rtpc.find( in_rtpcID );
				if( it == rtpc.end() )
				{
					out_rValue = 0.f;
					io_rValueType = RTPCValue_Unavailable;
					return AK_Fail;
				}
				out_rValue = it->second;
				io_rValueType = RTPCValue_Global;
				return AK_Success;
			}

			AKRESULT GetRTPCValue( const AkOSChar * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType )
			{
				return GetRTPCValue( AkSilent::NameHash( in_pszRtpcName ), in_gameObjectID, out_rValue, io_rValueType );
			}

			AKRESULT GetRTPCValue( const char * in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue & out_rValue, RTPCValue_type & io_rValueType )
			{
				return GetRTPCValue( AkSilent::NameHash( in_pszRtpcName ), in_gameObjectID, out_rValue, io_rValueType );
			}

			AKRESULT GetSwitch( AkSwitchGroupID in_switchGroup, AkGameObjectID in_gameObjectID, AkSwitchStateID & out_rSwitchState )
			{
				AkSilent::Engine & e = AkSilent::Get();
				std::map<AkUInt64, AkSwitchStateID>::const_iterator perObject = e.ObjectSwitches.find( ( (AkUInt64)in_gameObjectID << 32 ) | in_switchGroup );
				if( perObject != e.ObjectSwitches.end() )
				{
					out_rSwitchState = perObject->second;
					return AK_Success;
				}
				std::map<AkSwitchGroupID, AkSwitchStateID>::const_iterator global = e.Switches.find( in_switchGroup );
				if( global == e.Switches.end() )
				{
					out_rSwitchState = AK_DEFAULT_SWITCH_STATE;
					return AK_Fail;
				}
				out_rSwitchState = global->second;
				return AK_Success;
			}

			AKRESULT GetSwitch( const AkOSChar * in_pstrSwitchGroupName, AkGameObjectID in_GameObj, AkSwitchStateID & out_rSwitchState )
			{
				return GetSwitch( AkSilent::NameHash( in_pstrSwitchGroupName ), in_GameObj, out_rSwitchState );
			}

			AKRESULT GetSwitch( const char * in_pstrSwitchGroupName, AkGameObjectID in_GameObj, AkSwitchStateID & out_rSwitchState )
			{
				return GetSwitch( AkSilent::NameHash( in_pstrSwitchGroupName ), in_GameObj, out_rSwitchState );
			}

			AKRESULT GetState( AkStateGroupID in_stateGroup, AkStateID & out_rState )
			{
				std::map<AkStateGroupID, AkStateID> & states = AkSilent::Get().States;
				std::map<AkStateGroupID, AkStateID>::const_iterator it = states.find( in_stateGroup );
				if( it == states.end() )
				{
					out_rState = 0;
					return AK_Fail;
				}
				out_rState = it->second;
				return AK_Success;
			}

			AKRESULT GetState( const AkOSChar * in_pstrStateGroupName, AkStateID & out_rState )
			{
				return GetState( AkSilent::NameHash( in_pstrStateGroupName ), out_rState );
			}

			AKRESULT GetState( const char * in_pstrStateGroupName, AkStateID & out_rState )
			{
				return GetState( AkSilent::NameHash( in_pstrStateGroupName ), out_rState );
			}

			AKRESULT GetEnvironmentVolumes( AkAuxBusID, AkSpeakerVolumes & out_rVolumes )
			{
				memset( &out_rVolumes, 0, sizeof( out_rVolumes ) );
				return AK_Success;
			}

			AKRESULT GetEnvironmentBypass( AkAuxBusID, bool & out_rbBypassed )
			{
				out_rbBypassed = false;
				return AK_Success;
			}

			AKRESULT GetEnvironmentVolume( AkAuxBusID, AkReal32 & out_rVolume )
			{
				out_rVolume = 0.f;
				return AK_Success;
			}

			AKRESULT GetGameObjectEnvironmentsValues( AkGameObjectID, AkEnvironmentValue *, AkUInt32 & io_ruNumEnvValues )
			{
				io_ruNumEnvValues = 0;
				return AK_Success;
			}

			AKRESULT GetGameObjectDryLevelValue( AkGameObjectID in_gameObjectID, AkReal32 & out_rfControlValue )
			{
				std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
				std::map<AkGameObjectID, AkSilent::GameObject>::const_iterator it = objs.find( in_gameObjectID );
				if( it == objs.end() )
				{
					out_rfControlValue = 0.f;
					return AK_Fail;
				}
				out_rfControlValue = it->second.DryLevel;
				return AK_Success;
			}

			AKRESULT GetObjectObstructionAndOcclusion( AkGameObjectID in_ObjectID, AkUInt32, AkReal32 & out_rfObstructionLevel, AkReal32 & out_rfOcclusionLevel )
			{
				std::map<AkGameObjectID, AkSilent::GameObject> & objs = AkSilent::Get().GameObjects;
				std::map<AkGameObjectID, AkSilent::GameObject>::const_iterator it = objs.find( in_ObjectID );
				if( it == objs.end() )
				{
					out_rfObstructionLevel = 0.f;
					out_rfOcclusionLevel = 0.f;
					return AK_Fail;
				}
				out_rfObstructionLevel = it->second.ObstructionLevel;
				out_rfOcclusionLevel = it->second.OcclusionLevel;
				return AK_Success;
			}

			AkUniqueID GetEventIDFromPlayingID( AkPlayingID in_playingID )
			{
				AkSilent::Engine & e = AkSilent::Get();
				std::map<AkPlayingID, AkSilent::Voice>::const_iterator it = e.Voices.find( in_playingID );
				return it == e.Voices.end() ? AK_INVALID_UNIQUE_ID : it->second.EventID;
			}

			AkGameObjectID GetGameObjectFromPlayingID( AkPlayingID in_playingID )
			{
				AkSilent::Engine & e = AkSilent::Get();
				std::map<AkPlayingID, AkSilent::Voice>::const_iterator it = e.Voices.find( in_playingID );
				return it == e.Voices.end() ? AK_INVALID_GAME_OBJECT : it->second.GameObject;
			}

			AKRESULT GetPlayingIDsFromGameObject( AkGameObjectID in_GameObjId, AkUInt32 & io_ruNumIDs, AkPlayingID * out_aPlayingIDs )
			{
				AkSilent::Engine & e = AkSilent::Get();
				AkUInt32 written = 0;
				for( std::map<AkPlayingID, AkSilent::Voice>::const_iterator it = e.Voices.begin(); it != e.Voices.end(); ++it )
				{
					if( it->second.GameObject != in_GameObjId )
					{
						continue;
					}
					if( out_aPlayingIDs && written < io_ruNumIDs )
					{
						out_aPlayingIDs[ written ] = it->first;
					}
					++written;
				}
				io_ruNumIDs = written;
				return AK_Success;
			}

			// DISHONORED(bringup): the audio-object tree lives in the bank's HIRC section and is only
			// walkable by the real bank reader, so no objects are reported. UAkEvent::ComputeMaxRadius
			// (2013 rva 0xc7aa0) therefore keeps -1, i.e. "this event has no attenuation".
			AKRESULT QueryAudioObjectIDs( AkUniqueID, AkUInt32 & io_ruNumItems, AkObjectInfo * )
			{
				io_ruNumItems = 0;
				return AK_Success;
			}

			AKRESULT QueryAudioObjectIDs( const AkOSChar * in_pszEventName, AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos )
			{
				return QueryAudioObjectIDs( AkSilent::NameHash( in_pszEventName ), io_ruNumItems, out_aObjectInfos );
			}

			AKRESULT QueryAudioObjectIDs( const char * in_pszEventName, AkUInt32 & io_ruNumItems, AkObjectInfo * out_aObjectInfos )
			{
				return QueryAudioObjectIDs( AkSilent::NameHash( in_pszEventName ), io_ruNumItems, out_aObjectInfos );
			}

			AKRESULT GetPositioningInfo( AkUniqueID, AkPositioningInfo & out_rPositioningInfo )
			{
				memset( &out_rPositioningInfo, 0, sizeof( out_rPositioningInfo ) );
				out_rPositioningInfo.positioningType = AkUndefined;
				return AK_Fail;
			}
		}
	}
}
