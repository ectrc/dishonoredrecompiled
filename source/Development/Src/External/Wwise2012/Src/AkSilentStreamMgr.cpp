/*=============================================================================
	AkSilentStreamMgr.cpp - AK::StreamMgr and the one AK::IAkStreamMgr instance.

	DISHONORED(bringup): the real stream manager owns an IO thread per device, a buffer pool and the
	std/auto stream objects the lower engine pulls from. The backend has no lower engine, so the streaming
	side is reduced to what the game observes: the singleton exists and can be destroyed (retail's
	UAkAudioDevice::Teardown, 2013 rva 0x5b3de0, calls AK::IAkStreamMgr::m_pStreamMgr slot 1), devices are
	registered with the game's own low-level IO hook, the file-location resolver and the current language
	round-trip, and the language-change observers are called when the language changes (retail's
	CAkFilePackageLowLevelIO installs one, 2013 rva 0x5b0e90).

	The registered IAkIOHookBlocking is kept because the silent sound engine uses it for real: LoadBank
	reads a bank's BKHD chunk through it, which is how the log can report the retail bank version.
=============================================================================*/
#include "AkSilentInternal.h"

namespace AkSilent
{
	struct Device
	{
		AkDeviceID							ID;
		AkDeviceSettings					Settings;
		AK::StreamMgr::IAkLowLevelIOHook *	pHook;
		bool								bBlocking;
	};

	static std::vector<Device> g_Devices;
	static AK::StreamMgr::IAkFileLocationResolver * g_pResolver = NULL;
	static String g_Language;
	static std::vector< std::pair<AK::StreamMgr::AkLanguageChangeHandler, void *> > g_Observers;

	AK::StreamMgr::IAkIOHookBlocking * BlockingHook()
	{
		for( size_t i = 0; i < g_Devices.size(); ++i )
		{
			if( g_Devices[ i ].pHook && g_Devices[ i ].bBlocking )
			{
				return static_cast<AK::StreamMgr::IAkIOHookBlocking *>( g_Devices[ i ].pHook );
			}
		}
		return NULL;
	}

	AK::StreamMgr::IAkFileLocationResolver * Resolver()
	{
		return g_pResolver;
	}

	/** The one stream manager the game holds; no lower engine ever asks it for a stream. */
	class SilentStreamMgr : public AK::IAkStreamMgr
	{
	public:
		virtual ~SilentStreamMgr() {}

		/** m_pStreamMgr is protected in IAkStreamMgr, so the install goes through the derived class. */
		static void Install()
		{
			if( !m_pStreamMgr )
			{
				m_pStreamMgr = new SilentStreamMgr();
			}
		}

		virtual void Destroy()
		{
			Logf( L"StreamMgr::Destroy" );
			m_pStreamMgr = NULL;
			delete this;
		}

		virtual AK::IAkStreamMgrProfile * GetStreamMgrProfile() { return NULL; }

		// DISHONORED(bringup): a caller that wants a real stream gets a clean AK_NotImplemented instead
		// of a crash; nothing on the startup or map path reaches these four slots.
		virtual AKRESULT CreateStd( AkFileID, AkFileSystemFlags *, AkOpenMode, AK::IAkStdStream *& out_pStream, bool )
		{
			out_pStream = NULL;
			return AK_NotImplemented;
		}
		virtual AKRESULT CreateStd( const AkOSChar *, AkFileSystemFlags *, AkOpenMode, AK::IAkStdStream *& out_pStream, bool )
		{
			out_pStream = NULL;
			return AK_NotImplemented;
		}
		virtual AKRESULT CreateAuto( AkFileID, AkFileSystemFlags *, const AkAutoStmHeuristics &, AkAutoStmBufSettings *, AK::IAkAutoStream *& out_pStream, bool )
		{
			out_pStream = NULL;
			return AK_NotImplemented;
		}
		virtual AKRESULT CreateAuto( const AkOSChar *, AkFileSystemFlags *, const AkAutoStmHeuristics &, AkAutoStmBufSettings *, AK::IAkAutoStream *& out_pStream, bool )
		{
			out_pStream = NULL;
			return AK_NotImplemented;
		}
	};
}

// DISHONORED(layout): a plain static with its one definition here, never an inline static in the header:
// IAkStreamMgr.h reaches AkAudio, Engine and DishonoredGame and the Engine link is close to its limit.
AK::IAkStreamMgr * AK::IAkStreamMgr::m_pStreamMgr = NULL;

namespace AK
{
	namespace StreamMgr
	{
		void GetDefaultSettings( AkStreamMgrSettings & out_settings )
		{
			out_settings.uMemorySize = 64 * 1024;
		}

		void GetDefaultDeviceSettings( AkDeviceSettings & out_settings )
		{
			memset( &out_settings, 0, sizeof( out_settings ) );
			out_settings.uIOMemorySize = 2 * 1024 * 1024;
			out_settings.uIOMemoryAlignment = 32;
			out_settings.ePoolAttributes = AkMalloc;
			out_settings.uGranularity = 16 * 1024;
			out_settings.uSchedulerTypeFlags = AK_SCHEDULER_BLOCKING;
			out_settings.threadProperties.nPriority = 0;
			out_settings.threadProperties.uStackSize = 32 * 1024;
			out_settings.fTargetAutoStmBufferLength = 380.f;
			out_settings.uMaxConcurrentIO = 8;
			out_settings.fMaxCacheRatio = 1.f;
		}

		IAkStreamMgr * Create( const AkStreamMgrSettings & in_settings )
		{
			AkSilent::SilentStreamMgr::Install();
			AkSilent::Logf( L"StreamMgr::Create: %u bytes of stream memory (silent backend, no IO thread)", in_settings.uMemorySize );
			return IAkStreamMgr::Get();
		}

		AkMemPoolId GetPoolID()
		{
			return AK_DEFAULT_POOL_ID;
		}

		AkDeviceID CreateDevice( const AkDeviceSettings & in_settings, IAkLowLevelIOHook * in_pLowLevelHook )
		{
			if( !IAkStreamMgr::Get() )
			{
				return AK_INVALID_DEVICE_ID;
			}
			AkSilent::Device device;
			device.ID = (AkDeviceID)AkSilent::g_Devices.size();
			device.Settings = in_settings;
			device.pHook = in_pLowLevelHook;
			device.bBlocking = ( in_settings.uSchedulerTypeFlags & AK_SCHEDULER_BLOCKING ) != 0;
			AkSilent::g_Devices.push_back( device );
			AkSilent::Logf( L"StreamMgr::CreateDevice %u: granularity %u, %s hook", device.ID,
				in_settings.uGranularity, device.bBlocking ? L"blocking" : L"deferred" );
			return device.ID;
		}

		AKRESULT DestroyDevice( AkDeviceID in_deviceID )
		{
			if( in_deviceID >= AkSilent::g_Devices.size() )
			{
				return AK_InvalidParameter;
			}
			AkSilent::g_Devices[ in_deviceID ].pHook = NULL;
			return AK_Success;
		}

		void SetFileLocationResolver( IAkFileLocationResolver * in_pFileLocationResolver )
		{
			AkSilent::g_pResolver = in_pFileLocationResolver;
		}

		IAkFileLocationResolver * GetFileLocationResolver()
		{
			return AkSilent::g_pResolver;
		}

		const AkOSChar * GetCurrentLanguage()
		{
			return AkSilent::g_Language.c_str();
		}

		AKRESULT SetCurrentLanguage( const AkOSChar * in_pszLanguageName )
		{
			AkSilent::g_Language = in_pszLanguageName ? in_pszLanguageName : L"";
			AkSilent::Logf( L"StreamMgr::SetCurrentLanguage: %s", AkSilent::g_Language.c_str() );
			for( size_t i = 0; i < AkSilent::g_Observers.size(); ++i )
			{
				AkSilent::g_Observers[ i ].first( AkSilent::g_Language.c_str(), AkSilent::g_Observers[ i ].second );
			}
			return AK_Success;
		}

		AKRESULT AddLanguageChangeObserver( AkLanguageChangeHandler in_handler, void * in_pCookie )
		{
			if( !in_handler )
			{
				return AK_InvalidParameter;
			}
			AkSilent::g_Observers.push_back( std::make_pair( in_handler, in_pCookie ) );
			return AK_Success;
		}

		void RemoveLanguageChangeObserver( void * in_pCookie )
		{
			for( size_t i = AkSilent::g_Observers.size(); i > 0; --i )
			{
				if( AkSilent::g_Observers[ i - 1 ].second == in_pCookie )
				{
					AkSilent::g_Observers.erase( AkSilent::g_Observers.begin() + ( i - 1 ) );
				}
			}
		}
	}
}
