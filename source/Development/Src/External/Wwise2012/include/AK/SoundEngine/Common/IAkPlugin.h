/*=============================================================================
	IAkPlugin.h - the SDK public header include/AK/SoundEngine/Common/IAkPlugin.h (Wwise 2012.1): the interfaces between
	the sound engine and its effect / source plug-ins.

	DISHONORED(port): agent IH4 fork F1 (wave 26). Every interface's virtual order is the 2012 PDB's vtable
	(resources/docs/types/all_types.h *_vtbl; resources/docs/symbols/vtables.csv):
	  IAkRTPCSubscriber          dtor, SetParam                                  (CAkRoomVerbFXParams' base, 0xda26a4)
	  IAkPluginParam             + Clone, Init, Term, SetParamsBlock
	  IAkPluginMemAlloc          dtor, Malloc, Free                              (AkFXMemAlloc 0xd8d980)
	  IAkEffectPluginContext     dtor, IsSendModeEffect, GetStreamMgr, GetPlayingID, GetMaxBufferLength,
	                             PostMonitorData, CanPostMonitorData, GetPluginMedia, GetProcessorFeatures (0xd8dcb4)
	  IAkSourcePluginContext     dtor, GetNumLoops, GetStreamMgr, GetPlayingID, GetMaxBufferLength, GetPluginMedia,
	                             GetProcessorFeatures                            (0xd8dcdc)
	  IAkPlugin                  dtor, Term, Reset, GetPluginInfo                (0xda26b0)
	  IAkEffectPlugin            + Init (allocator, effect context, params, format)
	  IAkInPlaceEffectPlugin     + Execute (buffer), TimeSkip (frames)
	  IAkOutOfPlaceEffectPlugin  + Execute (in, in offset, out), TimeSkip (frames by reference)
	  IAkSourcePlugin            + Init (allocator, source context, params, format), GetDuration, StopLooping, Seek,
	                             TimeSkip (frames by reference), Execute (buffer)
	  IAkProcessorFeatures       dtor, GetSIMDSupport, GetCacheSize, GetCacheLineSize (0xd8db38; moved here from IH2's
	                             AkRuntimeEnvironmentMgr.h, which implements it)
	AkPluginParamID is AkInt16 as the PDB's SetParam(short, ...). IAkRTPCSubscriber moved here from AkRTPCMgr.h (IH3).
	Change this interface only additively: fork F2's plug-ins are written against it.
=============================================================================*/
#ifndef _IAK_PLUGIN_H_
#define _IAK_PLUGIN_H_

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkCommonDefs.h>
#include <new>

#if defined(_MSC_VER)
#pragma pack (push, 8)
#endif

typedef AkInt16 AkPluginParamID;

/** 8 bytes (2012 PDB). */
struct AkPluginInfo
{
	AkPluginType	eType;
	bool			bIsInPlace;
	bool			bIsAsynchronous;
};

namespace AK
{
	class IAkStreamMgr;

	enum AkSIMDProcessorSupport
	{
		AK_SIMD_SSE		= 0x1,
		AK_SIMD_SSE2	= 0x2,
		AK_SIMD_SSE3	= 0x4,
		AK_SIMD_SSSE3	= 0x8
	};

	class IAkProcessorFeatures
	{
	protected:
		virtual ~IAkProcessorFeatures() {}
	public:
		virtual bool GetSIMDSupport( AkSIMDProcessorSupport in_eSIMD ) = 0;
		virtual AkUInt32 GetCacheSize() = 0;
		virtual AkUInt32 GetCacheLineSize() = 0;
	};

	class IAkPluginMemAlloc
	{
	protected:
		virtual ~IAkPluginMemAlloc() {}
	public:
		virtual void * Malloc( size_t in_uSize ) = 0;
		virtual void Free( void * in_pMemAddress ) = 0;
	};

	class IAkEffectPluginContext
	{
	protected:
		virtual ~IAkEffectPluginContext() {}
	public:
		virtual bool IsSendModeEffect() const = 0;
		virtual IAkStreamMgr * GetStreamMgr() const = 0;
		virtual AkPlayingID GetPlayingID() const = 0;
		virtual AkUInt16 GetMaxBufferLength() const = 0;
		virtual AKRESULT PostMonitorData( void * in_pData, AkUInt32 in_uDataSize ) = 0;
		virtual bool CanPostMonitorData() = 0;
		virtual void GetPluginMedia( AkUInt32 in_dataIndex, AkUInt8 * & out_rpData, AkUInt32 & out_rDataSize ) = 0;
		virtual IAkProcessorFeatures * GetProcessorFeatures() = 0;
	};

	class IAkSourcePluginContext
	{
	protected:
		virtual ~IAkSourcePluginContext() {}
	public:
		virtual AkUInt16 GetNumLoops() const = 0;
		virtual IAkStreamMgr * GetStreamMgr() const = 0;
		virtual AkPlayingID GetPlayingID() const = 0;
		virtual AkUInt16 GetMaxBufferLength() const = 0;
		virtual void GetPluginMedia( AkUInt32 in_dataIndex, AkUInt8 * & out_rpData, AkUInt32 & out_rDataSize ) = 0;
		virtual IAkProcessorFeatures * GetProcessorFeatures() = 0;
	};

	class IAkRTPCSubscriber
	{
	protected:
		virtual ~IAkRTPCSubscriber() {}
	public:
		virtual AKRESULT SetParam( AkPluginParamID in_paramID, const void * in_pValue, AkUInt32 in_uParamSize ) = 0;
	};

	class IAkPluginParam : public IAkRTPCSubscriber
	{
	protected:
		virtual ~IAkPluginParam() {}
	public:
		virtual IAkPluginParam * Clone( IAkPluginMemAlloc * in_pAllocator ) = 0;
		virtual AKRESULT Init( IAkPluginMemAlloc * in_pAllocator, const void * in_pParamsBlock, AkUInt32 in_uBlockSize ) = 0;
		virtual AKRESULT Term( IAkPluginMemAlloc * in_pAllocator ) = 0;
		virtual AKRESULT SetParamsBlock( const void * in_pParamsBlock, AkUInt32 in_uBlockSize ) = 0;
	};

	class IAkPlugin
	{
	protected:
		virtual ~IAkPlugin() {}
	public:
		virtual AKRESULT Term( IAkPluginMemAlloc * in_pAllocator ) = 0;
		virtual AKRESULT Reset() = 0;
		virtual AKRESULT GetPluginInfo( AkPluginInfo & out_rPluginInfo ) = 0;
	};

	class IAkEffectPlugin : public IAkPlugin
	{
	protected:
		virtual ~IAkEffectPlugin() {}
	public:
		virtual AKRESULT Init( IAkPluginMemAlloc * in_pAllocator, IAkEffectPluginContext * in_pEffectPluginContext, IAkPluginParam * in_pParams, AkAudioFormat & io_rFormat ) = 0;
	};

	class IAkInPlaceEffectPlugin : public IAkEffectPlugin
	{
	public:
		virtual void Execute( AkAudioBuffer * io_pBuffer ) = 0;
		virtual AKRESULT TimeSkip( AkUInt32 in_uFrames ) = 0;
	};

	class IAkOutOfPlaceEffectPlugin : public IAkEffectPlugin
	{
	public:
		virtual void Execute( AkAudioBuffer * in_pBuffer, AkUInt32 in_uInOffset, AkAudioBuffer * out_pBuffer ) = 0;
		virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames ) = 0;
	};

	class IAkSourcePlugin : public IAkPlugin
	{
	protected:
		virtual ~IAkSourcePlugin() {}
	public:
		virtual AKRESULT Init( IAkPluginMemAlloc * in_pAllocator, IAkSourcePluginContext * in_pSourcePluginContext, IAkPluginParam * in_pParams, AkAudioFormat & io_rFormat ) = 0;
		virtual AkReal32 GetDuration() const = 0;
		virtual AKRESULT StopLooping() = 0;
		virtual AKRESULT Seek( AkUInt32 in_uPosition ) = 0;
		virtual AKRESULT TimeSkip( AkUInt32 & io_uFrames ) = 0;
		virtual void Execute( AkAudioBuffer * io_pBuffer ) = 0;
	};
}

/** The SDK's allocation: operator new on a plug-in allocator (a NULL block skips the constructor and yields NULL),
    AK_PLUGIN_NEW( alloc, CAkThing() ); delete = explicit dtor + Free. */
inline void * operator new( size_t in_uSize, AK::IAkPluginMemAlloc * in_pAllocator ) throw()
{
	return in_pAllocator->Malloc( in_uSize );
}

inline void operator delete( void * in_pPtr, AK::IAkPluginMemAlloc * in_pAllocator ) throw()
{
	in_pAllocator->Free( in_pPtr );
}

#define AK_PLUGIN_NEW( _allocator, _what )		new( _allocator ) _what
#define AK_PLUGIN_ALLOC( _allocator, _size )	( _allocator )->Malloc( ( _size ) )
#define AK_PLUGIN_FREE( _allocator, _pmem )		( _allocator )->Free( ( _pmem ) )

template <class T>
inline void AK_PLUGIN_DELETE( AK::IAkPluginMemAlloc * in_pAllocator, T * in_pObject )
{
	if( in_pObject )
	{
		in_pObject->~T();
		in_pAllocator->Free( in_pObject );
	}
}

/** Plug-in ids: (plug-in id << 16) | (company id << 4) | type, the key CAkEffectsMgr::RegisterPlugin stores. */
#define AKMAKECLASSID( _type, _company, _pluginid )	( (_type) + ( (_company) << 4 ) + ( (_pluginid) << 16 ) )
#define AKGETPLUGINTYPEFROMCLASSID( _classid )		( (_classid) & AkPluginTypeMask )
#define AKGETCOMPANYIDFROMCLASSID( _classid )		( ( (_classid) & 0x0000FFF0 ) >> 4 )
#define AKGETPLUGINIDFROMCLASSID( _classid )		( ( (_classid) & 0xFFFF0000 ) >> 16 )

#if defined(_MSC_VER)
#pragma pack (pop)
#endif

#if defined(__cplusplus) && !defined(AK_NO_LAYOUT_CHECKS)
static_assert( sizeof( AkPluginInfo ) == 8, "AkPluginInfo: 2012 PDB sizeof 8" );
#endif

#endif // _IAK_PLUGIN_H_
