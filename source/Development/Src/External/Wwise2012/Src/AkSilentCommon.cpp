/*=============================================================================
	AkSilentCommon.cpp - name hash, name registry, log sink and statistics of the silent backend.

	DISHONORED(bringup): see AkSilentInternal.h. The name hash itself is DISHONORED(written) and verified
	against retail content (build/agentAN/bank_stid.py).
=============================================================================*/
#include <AkRegistryMgr.h>
#include <AkPlayingMgr.h>
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

	static bool s_bDiagnostics = false;

	void SetDiagnostics( bool in_bEnabled )
	{
		s_bDiagnostics = in_bEnabled;
	}

	bool IsDiagnostics()
	{
		return s_bDiagnostics;
	}

	const Stats & GetStats()
	{
		AkAutoLock<CAkLock> gate( g_csMain );
		AkSilent::g_Stats.GameObjects = g_pRegistryMgr ? g_pRegistryMgr->NumRegistered() : 0;
		AkSilent::g_Stats.PlayingIDsActive = g_pPlayingMgr ? g_pPlayingMgr->m_PlayingMap.m_uiSize : 0;
		return AkSilent::g_Stats;
	}

	const AkOSChar * NameForID( AkUInt32 in_id )
	{
		std::map<AkUInt32, AkSilent::String>::const_iterator it = AkSilent::Names().find( in_id );
		return it == AkSilent::Names().end() ? NULL : it->second.c_str();
	}
}
