/*=============================================================================
	AkSilentCommon.cpp - name hash, name registry, log sink and statistics of the silent backend.

	DISHONORED(bringup): see AkSilentInternal.h. The name hash itself is DISHONORED(written) and verified
	against retail content (build/agentAN/bank_stid.py).
=============================================================================*/
#include "AkSilentInternal.h"

#include <cstdarg>
#include <cstdio>

namespace AkSilent
{
	static DishonoredWwise::LogHook g_pLogHook = NULL;
	static std::map<AkUInt32, String> * g_pNames = NULL;
	static DishonoredWwise::Stats g_Stats = {};
	static Engine * g_pEngine = NULL;

	AkUInt32 NameHash( const wchar_t * name )
	{
		AkUInt32 hash = 2166136261u;
		if( name )
		{
			for( const wchar_t * p = name; *p; ++p )
			{
				wchar_t c = *p;
				if( c >= L'A' && c <= L'Z' )
				{
					c = (wchar_t)( c - L'A' + L'a' );
				}
				hash = hash * 16777619u;
				hash = hash ^ (AkUInt32)( c & 0xFF );
			}
		}
		return hash;
	}

	AkUInt32 NameHash( const char * name )
	{
		AkUInt32 hash = 2166136261u;
		if( name )
		{
			for( const char * p = name; *p; ++p )
			{
				char c = *p;
				if( c >= 'A' && c <= 'Z' )
				{
					c = (char)( c - 'A' + 'a' );
				}
				hash = hash * 16777619u;
				hash = hash ^ (AkUInt32)( (unsigned char)c );
			}
		}
		return hash;
	}

	static std::map<AkUInt32, String> & Names()
	{
		if( !g_pNames )
		{
			g_pNames = new std::map<AkUInt32, String>();
		}
		return *g_pNames;
	}

	AkUInt32 RegisterName( const wchar_t * name )
	{
		const AkUInt32 id = NameHash( name );
		if( name && *name )
		{
			Names()[ id ] = name;
		}
		return id;
	}

	AkUInt32 RegisterName( const char * name )
	{
		const AkUInt32 id = NameHash( name );
		if( name && *name )
		{
			String wide;
			for( const char * p = name; *p; ++p )
			{
				wide.push_back( (wchar_t)(unsigned char)*p );
			}
			Names()[ id ] = wide;
		}
		return id;
	}

	void Logf( const wchar_t * fmt, ... )
	{
		if( !g_pLogHook )
		{
			return;
		}
		wchar_t buffer[1024];
		va_list args;
		va_start( args, fmt );
		const int written = _vsnwprintf_s( buffer, 1024, 1023, fmt, args );
		va_end( args );
		if( written < 0 )
		{
			buffer[1023] = L'\0';
		}
		g_pLogHook( buffer );
	}

	DishonoredWwise::Stats & MutableStats()
	{
		return g_Stats;
	}

	Engine & Get()
	{
		if( !g_pEngine )
		{
			g_pEngine = new Engine();
			g_pEngine->bSoundEngineInit = false;
			g_pEngine->bMemoryMgrInit = false;
			g_pEngine->bMusicEngineInit = false;
			g_pEngine->NextPlayingID = 1;
			g_pEngine->PanningRule = AkPanningRule_Speakers;
			g_pEngine->SpeakerConfig = AK_SPEAKER_SETUP_5POINT1;
			g_pEngine->VolumeThreshold = -96.f;
			g_pEngine->MaxNumVoices = 0;
			memset( g_pEngine->Listeners, 0, sizeof( g_pEngine->Listeners ) );
		}
		return *g_pEngine;
	}
}

namespace DishonoredWwise
{
	bool IsSilentBackend()
	{
		return true;
	}

	void SetLogHook( LogHook in_pHook )
	{
		AkSilent::g_pLogHook = in_pHook;
	}

	const Stats & GetStats()
	{
		AkSilent::Engine & e = AkSilent::Get();
		AkSilent::g_Stats.GameObjects = (AkUInt32)e.GameObjects.size();
		AkSilent::g_Stats.PlayingIDsActive = (AkUInt32)e.Voices.size();
		return AkSilent::g_Stats;
	}

	const AkOSChar * NameForID( AkUInt32 in_id )
	{
		std::map<AkUInt32, AkSilent::String>::const_iterator it = AkSilent::Names().find( in_id );
		return it == AkSilent::Names().end() ? NULL : it->second.c_str();
	}
}
