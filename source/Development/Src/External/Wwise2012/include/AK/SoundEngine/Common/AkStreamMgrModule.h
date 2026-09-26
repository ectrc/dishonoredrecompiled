/*=============================================================================
	AkStreamMgrModule.h - Wwise 2012.1 stream-manager creation and the streaming device factory.

	DISHONORED(written): declarations from the demangled 2012 PDB signatures
	  class AK::IAkStreamMgr * __cdecl AK::StreamMgr::Create(struct AkStreamMgrSettings const &)   0x96ee60
	  unsigned long __cdecl AK::StreamMgr::CreateDevice(struct AkDeviceSettings const &,
	      class AK::StreamMgr::IAkLowLevelIOHook *)                                                0x96f290
	  enum AKRESULT __cdecl AK::StreamMgr::SetCurrentLanguage(wchar_t const *)                      0x96ef30
	  enum AKRESULT __cdecl AK::StreamMgr::AddLanguageChangeObserver(void (__cdecl *)(wchar_t const * const,
	      void *), void *)                                                                          0x96f0f0
	UAkAudioDevice::SetBankDirectory (2013 rva 0x5b0b40) is the only SetCurrentLanguage caller and
	CAkFilePackageLowLevelIO installs the language-change observer.
=============================================================================*/
#ifndef _AK_STREAM_MGR_MODULE_H_
#define _AK_STREAM_MGR_MODULE_H_

#include <AK/SoundEngine/Common/IAkStreamMgr.h>

#if defined(_MSC_VER)
#pragma pack (push, 8)
#endif

/** DISHONORED(layout): 4 bytes, a single AkUInt32 (2012 PDB AkStreamMgrSettings). */
struct AkStreamMgrSettings
{
	AkUInt32 uMemorySize;
};

/** DISHONORED(layout): 48 bytes; ePoolAttributes @12 is AkMemPoolAttributes, threadProperties @24. */
struct AkDeviceSettings
{
	void *				pIOMemory;
	AkUInt32			uIOMemorySize;
	AkUInt32			uIOMemoryAlignment;
	AkMemPoolAttributes	ePoolAttributes;
	AkUInt32			uGranularity;
	AkUInt32			uSchedulerTypeFlags;
	AkThreadProperties	threadProperties;
	AkReal32			fTargetAutoStmBufferLength;
	AkUInt32			uMaxConcurrentIO;
	AkReal32			fMaxCacheRatio;
};

#if defined(_MSC_VER)
#pragma pack (pop)
#endif

/** uSchedulerTypeFlags values (SDK contract; retail creates a blocking device, CAkDefaultIOHookBlocking). */
#define AK_SCHEDULER_BLOCKING			0x01
#define AK_SCHEDULER_DEFERRED_LINED_UP	0x02

namespace AK
{
	namespace StreamMgr
	{
		typedef void ( *AkLanguageChangeHandler )( const AkOSChar * const in_pLanguageName, void * in_pCookie );

		void GetDefaultSettings( AkStreamMgrSettings & out_settings );					///< 2012 rva 0x96e3e0
		void GetDefaultDeviceSettings( AkDeviceSettings & out_settings );				///< 2012 rva 0x96e3f0
		IAkStreamMgr * Create( const AkStreamMgrSettings & in_settings );				///< 2012 rva 0x96ee60
		AkMemPoolId GetPoolID();														///< 2012 rva 0x96e470

		AkDeviceID CreateDevice( const AkDeviceSettings & in_settings, IAkLowLevelIOHook * in_pLowLevelHook );	///< 2012 rva 0x96f290
		AKRESULT DestroyDevice( AkDeviceID in_deviceID );								///< 2012 rva 0x96eee0

		void SetFileLocationResolver( IAkFileLocationResolver * in_pFileLocationResolver );	///< 2012 rva 0x96e460
		IAkFileLocationResolver * GetFileLocationResolver();							///< 2012 rva 0x96e450

		const AkOSChar * GetCurrentLanguage();											///< 2012 rva 0x96e480
		AKRESULT SetCurrentLanguage( const AkOSChar * in_pszLanguageName );				///< 2012 rva 0x96ef30
		AKRESULT AddLanguageChangeObserver( AkLanguageChangeHandler in_handler, void * in_pCookie );	///< 2012 rva 0x96f0f0
		void RemoveLanguageChangeObserver( void * in_pCookie );							///< 2012 rva 0x96ef50
	}
}

#if defined(__cplusplus) && !defined(AK_NO_LAYOUT_CHECKS)
static_assert(sizeof(AkStreamMgrSettings) == 4, "AkStreamMgrSettings: 2012 PDB sizeof 4");
static_assert(sizeof(AkDeviceSettings) == 48, "AkDeviceSettings: 2012 PDB sizeof 48");
#endif

#endif // _AK_STREAM_MGR_MODULE_H_
