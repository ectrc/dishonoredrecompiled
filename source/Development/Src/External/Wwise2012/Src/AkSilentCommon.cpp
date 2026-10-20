/*=============================================================================
	AkSilentCommon.cpp - name hash, name registry, log sink and statistics of the silent backend.

	DISHONORED(bringup): see AkSilentInternal.h. The name hash itself is DISHONORED(written) and verified
	against retail content (build/agentAN/bank_stid.py).
	DISHONORED(diag, agent IQ): -akdump=<file> (SetCrashDump) writes a minidump of the first fatal exception of any thread
	(vectored handler + unhandled filter), of a pure virtual call and of abort(), with a <file>.txt beside it. Not retail.
=============================================================================*/
#include <AkRegistryMgr.h>
#include <AkPlayingMgr.h>
#include "AkSilentInternal.h"

#include <cstdarg>
#include <cstdio>
#include <csignal>
#include <cstdlib>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

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

namespace
{
	wchar_t g_szCrashDumpPath[ MAX_PATH ] = { 0 };
	volatile LONG g_lCrashDumped = 0;

	struct AkDumpExceptionInfo
	{
		DWORD ThreadId;
		EXCEPTION_POINTERS * ExceptionPointers;
		BOOL ClientPointers;
	};
	typedef BOOL ( WINAPI * MiniDumpWriteDumpFunc )( HANDLE, DWORD, HANDLE, int, AkDumpExceptionInfo *, void *, void * );

	void WriteCrashDump( EXCEPTION_POINTERS * in_pException, const wchar_t * in_szKind )
	{
		if( !g_szCrashDumpPath[ 0 ] || InterlockedExchange( &g_lCrashDumped, 1 ) )
		{
			return;
		}
		const DWORD uCode = in_pException ? in_pException->ExceptionRecord->ExceptionCode : 0;
		const void * pAddress = in_pException ? in_pException->ExceptionRecord->ExceptionAddress : NULL;
		const ULONG_PTR uAccess = in_pException && in_pException->ExceptionRecord->NumberParameters >= 2 ? in_pException->ExceptionRecord->ExceptionInformation[ 1 ] : 0;
		HMODULE hDbgHelp = LoadLibraryW( L"dbghelp.dll" );
		MiniDumpWriteDumpFunc pWrite = hDbgHelp ? (MiniDumpWriteDumpFunc)GetProcAddress( hDbgHelp, "MiniDumpWriteDump" ) : NULL;
		HANDLE hFile = CreateFileW( g_szCrashDumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		BOOL bWritten = FALSE;
		if( pWrite && hFile != INVALID_HANDLE_VALUE )
		{
			AkDumpExceptionInfo info = { GetCurrentThreadId(), in_pException, FALSE };
			bWritten = pWrite( GetCurrentProcess(), GetCurrentProcessId(), hFile, 0x1 | 0x40 | 0x800, in_pException ? &info : NULL, NULL, NULL );
		}
		if( hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( hFile );
		}
		wchar_t szText[ MAX_PATH + 8 ];
		_snwprintf_s( szText, MAX_PATH + 8, _TRUNCATE, L"%s.txt", g_szCrashDumpPath );
		FILE * pText = NULL;
		if( _wfopen_s( &pText, szText, L"w" ) == 0 && pText )
		{
			fwprintf( pText, L"%s: code 0x%08x at %p, access %p, thread %u, minidump %s\n", in_szKind, uCode, pAddress, (void *)uAccess, GetCurrentThreadId(), bWritten ? L"written" : L"FAILED" );
			fclose( pText );
		}
		AkSilent::Logf( L"akdump: %s: code 0x%08x at %p, access %p, thread %u, minidump %s", in_szKind, uCode, pAddress, (void *)uAccess, GetCurrentThreadId(), bWritten ? L"written" : L"FAILED" );
	}

	bool IsFatalException( DWORD in_uCode )
	{
		return in_uCode == EXCEPTION_ACCESS_VIOLATION || in_uCode == EXCEPTION_STACK_OVERFLOW || in_uCode == EXCEPTION_ILLEGAL_INSTRUCTION
			|| in_uCode == EXCEPTION_PRIV_INSTRUCTION || in_uCode == EXCEPTION_INT_DIVIDE_BY_ZERO || in_uCode == 0xC0000409 || in_uCode == 0xC0000374;
	}

	LONG WINAPI CrashDumpVectoredHandler( EXCEPTION_POINTERS * in_pException )
	{
		if( IsFatalException( in_pException->ExceptionRecord->ExceptionCode ) )
		{
			WriteCrashDump( in_pException, L"first-chance fatal exception" );
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}

	LONG WINAPI CrashDumpUnhandledFilter( EXCEPTION_POINTERS * in_pException )
	{
		WriteCrashDump( in_pException, L"unhandled exception" );
		return EXCEPTION_CONTINUE_SEARCH;
	}

	void DumpHere( const wchar_t * in_szKind )
	{
		__try
		{
			RaiseException( 0xE0414B44, 0, 0, NULL );
		}
		__except( WriteCrashDump( GetExceptionInformation(), in_szKind ), EXCEPTION_EXECUTE_HANDLER )
		{
		}
	}

	void __cdecl CrashDumpPureCall()
	{
		DumpHere( L"pure virtual call" );
		abort();
	}

	void __cdecl CrashDumpAbort( int )
	{
		DumpHere( L"abort" );
	}
}

namespace DishonoredWwise
{
	void SetCrashDump( const wchar_t * in_pszPath )
	{
		if( !in_pszPath || !*in_pszPath )
		{
			return;
		}
		wcsncpy_s( g_szCrashDumpPath, MAX_PATH, in_pszPath, _TRUNCATE );
		AddVectoredExceptionHandler( 1, CrashDumpVectoredHandler );
		SetUnhandledExceptionFilter( CrashDumpUnhandledFilter );
		_set_purecall_handler( CrashDumpPureCall );
		signal( SIGABRT, CrashDumpAbort );
		AkSilent::Logf( L"akdump: armed, %s", g_szCrashDumpPath );
	}
}
